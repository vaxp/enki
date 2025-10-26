/// @file CefBridge.cpp
/// @brief CefBridge — IWebViewBackend implementation + CefGlobal singleton.
///
/// @copyright ENKI Framework — MIT License

#include "CefBridge.hpp"
#include "InputForwarder.hpp"
#include <include/cef_app.h>
#include <include/cef_base.h>
#include <include/cef_browser.h>
#include <include/cef_command_line.h>
#include <cassert>
#include <filesystem>
#include <iostream>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

namespace enki::web {
namespace fs = ::std::filesystem;

// ════════════════════════════════════════════════════════════════
// IWebViewBackend::create()  — factory (defined in this file)
// ════════════════════════════════════════════════════════════════

std::unique_ptr<IWebViewBackend> IWebViewBackend::create()
{
    return std::make_unique<CefBridge>();
}

// ════════════════════════════════════════════════════════════════
// CefGlobal
// ════════════════════════════════════════════════════════════════

CefGlobal& CefGlobal::instance()
{
    static CefGlobal inst;
    return inst;
}

bool CefGlobal::ensure_initialized(int argc, char* argv[], const std::string& subprocess_path,
                                   bool windowed_mode)
{
    if (initialized_.load(std::memory_order_acquire)) return true;
    if (shutdown_called_.load()) return false;

    // Pass real argc / argv to Chromium
    CefMainArgs main_args(argc, argv);

    CefSettings settings;
    settings.no_sandbox              = true;
    // OSR mode requires windowless_rendering_enabled = true.
    // Windowed mode requires it to be false.
    settings.windowless_rendering_enabled = windowed_mode ? false : true;
    settings.multi_threaded_message_loop  = false;
    settings.command_line_args_disabled   = false;

    // ── Locate CEF Resources and Locales ──
    std::vector<fs::path> res_candidates = {
        "web_technology/cef_binary/cef_binary_144.0.34+g8fc21c8+chromium-144.0.7559.261_linux64_minimal/Resources",
        "../web_technology/cef_binary/cef_binary_144.0.34+g8fc21c8+chromium-144.0.7559.261_linux64_minimal/Resources",
        "/home/x/Work/enki/web_technology/cef_binary/cef_binary_144.0.34+g8fc21c8+chromium-144.0.7559.261_linux64_minimal/Resources"
    };

    for (const auto& p : res_candidates) {
        if (fs::exists(p / "icudtl.dat")) {
            CefString(&settings.resources_dir_path) = fs::absolute(p).string();
            if (fs::exists(p / "locales")) {
                CefString(&settings.locales_dir_path) = fs::absolute(p / "locales").string();
            }
            break;
        }
    }

    // ── Locate Subprocess binary ──
    if (!subprocess_path.empty()) {
        CefString(&settings.browser_subprocess_path) = subprocess_path;
    } else {
        std::vector<fs::path> sub_candidates = {
            "build_web_test/web_technology/subprocess/enki_cef_subprocess",
            "../build_web_test/web_technology/subprocess/enki_cef_subprocess",
            "web_technology/subprocess/enki_cef_subprocess",
            "enki_cef_subprocess"
        };
        for (const auto& sp : sub_candidates) {
            if (fs::exists(sp)) {
                CefString(&settings.browser_subprocess_path) = fs::absolute(sp).string();
                break;
            }
        }
    }

    // ── Cache Path ──
    const char* home = getenv("HOME");
    std::string cache_dir = home ? (std::string(home) + "/.cache/enki_web") : "/tmp/enki_web_cache";
    try { fs::create_directories(cache_dir); } catch (...) {}
    CefString(&settings.root_cache_path) = cache_dir;

    // Log to stderr by default; change to a file path for production.
    settings.log_severity = LOGSEVERITY_WARNING;

    // Create the app object (nullptr = use internal defaults).
    auto app = CefRefPtr<EnkiCefApp>(new EnkiCefApp());

    bool ok = CefInitialize(main_args, settings, app.get(), nullptr);
    if (ok) {
        initialized_.store(true, std::memory_order_release);
    }
    return ok;
}

void CefGlobal::shutdown()
{
    if (!initialized_.load()) return;
    if (shutdown_called_.exchange(true)) return;

    CefShutdown();
    initialized_.store(false, std::memory_order_release);
}

void CefGlobal::do_message_loop_work()
{
    if (instance().is_initialized()) {
        CefDoMessageLoopWork();
    }
}

// ════════════════════════════════════════════════════════════════
// CefBridge
// ════════════════════════════════════════════════════════════════

CefBridge::CefBridge()
    : js_bridge_(std::make_unique<JSBridge>())
{}

CefBridge::~CefBridge()
{
    shutdown();
}

// ── initialize ─────────────────────────────────────────────────

bool CefBridge::initialize(const BackendConfig& cfg)
{
    config_ = cfg;

    // Ensure CEF is initialised (idempotent).
    if (!CefGlobal::instance().ensure_initialized()) {
        return false;
    }

    // Create the offscreen browser asynchronously.
    // OnAfterCreated() will be called when it is ready.
    create_browser(cfg);
    return true;
}

void CefBridge::create_browser(const BackendConfig& cfg)
{
    // ── RenderHandler ──────────────────────────────────────────
    render_handler_ = CefRefPtr<EnkiRenderHandler>(
        new EnkiRenderHandler([this](WebViewFrame frame) {
            OnPaintCallback cb;
            {
                std::lock_guard<std::mutex> lk(mutex_);
                cb = on_paint_;
            }
            if (cb) cb(std::move(frame));
        }));
    render_handler_->setSize(cfg.width, cfg.height);
    render_handler_->setDeviceScale(cfg.device_scale);

    // ── LifeSpanHandler ────────────────────────────────────────
    auto life_span = CefRefPtr<EnkiLifeSpanHandler>(
        new EnkiLifeSpanHandler(
            [this](CefRefPtr<CefBrowser> b) { on_browser_created(b); },
            [this]()                         { on_browser_closed();   },
            [this](std::string url) -> bool {
                OnNewWindowCallback cb;
                { std::lock_guard<std::mutex> lk(mutex_); cb = on_new_window_; }
                return cb ? cb(url) : false;
            }
        ));

    // ── CefClient ──────────────────────────────────────────────
    client_ = CefRefPtr<EnkiCefClient>(
        new EnkiCefClient(
            render_handler_,
            life_span,
            [this](std::string t) {
                { std::lock_guard<std::mutex> lk(state_mutex_); current_title_ = t; }
                OnTitleCallback cb; { std::lock_guard<std::mutex> lk(mutex_); cb = on_title_; }
                if (cb) cb(std::move(t));
            },
            [this](std::string u) {
                { std::lock_guard<std::mutex> lk(state_mutex_); current_url_ = u; }
                OnURLCallback cb; { std::lock_guard<std::mutex> lk(mutex_); cb = on_url_; }
                if (cb) cb(std::move(u));
            },
            [this](bool ok, int code) {
                is_loading_.store(false);
                can_go_back_.store(browser_ ? browser_->CanGoBack()    : false);
                can_go_forward_.store(browser_ ? browser_->CanGoForward() : false);
                OnLoadCallback cb; { std::lock_guard<std::mutex> lk(mutex_); cb = on_load_; }
                if (cb) cb(ok, code);
            },
            [this](std::string u) {
                is_loading_.store(true);
                OnLoadStartCallback cb; { std::lock_guard<std::mutex> lk(mutex_); cb = on_load_start_; }
                if (cb) cb(std::move(u));
            },
            [this](std::string msg, int line, std::string src) {
                OnConsoleCallback cb; { std::lock_guard<std::mutex> lk(mutex_); cb = on_console_; }
                if (cb) cb(std::move(msg), line, std::move(src));
            }
        ));

    // ── Window / Browser settings ──────────────────────────────
    CefWindowInfo window_info;

    if (cfg.windowed_mode) {
        // ── Create an X11 window and embed CEF into it ─────────
        // We open a temporary Display connection just to create the window,
        // then close it immediately. CEF will open its own Display connection
        // internally to avoid dual-connection conflicts (stack smashing).
        Display* dpy = XOpenDisplay(nullptr);
        if (!dpy) {
            std::cerr << "[CefBridge] Cannot open X11 display.\n";
            return;
        }
        int scr    = DefaultScreen(dpy);
        Window win = XCreateSimpleWindow(
            dpy,
            RootWindow(dpy, scr),
            0, 0,                                       // position
            static_cast<unsigned>(cfg.width),           // width
            static_cast<unsigned>(cfg.height),          // height
            0,                                          // border width
            BlackPixel(dpy, scr),                       // border colour
            BlackPixel(dpy, scr)                        // background colour
        );

        // Set title and delete-window atom before handing off to CEF
        XStoreName(dpy, win, cfg.window_title.c_str());
        Atom wm_delete = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
        XSetWMProtocols(dpy, win, &wm_delete, 1);

        // Size hints so the WM positions it properly
        XSizeHints* hints = XAllocSizeHints();
        if (hints) {
            hints->flags      = PSize;
            hints->width      = cfg.width;
            hints->height     = cfg.height;
            XSetNormalHints(dpy, win, hints);
            XFree(hints);
        }

        XMapWindow(dpy, win);
        XFlush(dpy);
        XSync(dpy, False);

        // Store the X11 Window ID (opaque unsigned long — doesn't need Display*)
        unsigned long win_id = static_cast<unsigned long>(win);

        // Close OUR Display connection — CEF will open its own
        XCloseDisplay(dpy);

        // Give CEF the window handle to embed into
        CefRect cef_rect(0, 0, cfg.width, cfg.height);
        window_info.SetAsChild(static_cast<CefWindowHandle>(win_id), cef_rect);
    } else {
        // ── Offscreen (OSR) — for Enki Canvas embedding ───────
        window_info.SetAsWindowless(0);
    }

    CefBrowserSettings browser_settings;
    if (cfg.windowed_mode) {
        browser_settings.windowless_frame_rate = 0;
    } else {
        browser_settings.windowless_frame_rate = 60;
    }
    browser_settings.javascript    = cfg.enable_js     ? STATE_ENABLED  : STATE_DISABLED;
    browser_settings.webgl         = cfg.enable_webgl  ? STATE_ENABLED  : STATE_DISABLED;
    browser_settings.image_loading = cfg.enable_images ? STATE_ENABLED  : STATE_DISABLED;
    browser_settings.background_color = cfg.background_color;

    CefBrowserHost::CreateBrowser(
        window_info,
        client_.get(),
        pending_url_.empty() ? "about:blank" : pending_url_,
        browser_settings,
        nullptr,   // extra_info
        nullptr    // request_context
    );
}

// ── on_browser_created ─────────────────────────────────────────

void CefBridge::on_browser_created(CefRefPtr<CefBrowser> browser)
{
    {
        std::lock_guard<std::mutex> lk(mutex_);
        browser_ = browser;
        browser_ready_ = true;

        js_bridge_->set_browser(browser);
    }
    browser_ready_cv_.notify_all();

    // Load any pending URL / HTML.
    if (!pending_url_.empty()) {
        browser->GetMainFrame()->LoadURL(pending_url_);
        pending_url_.clear();
    } else if (!pending_html_.empty()) {
        browser->GetMainFrame()->LoadURL(
            "data:text/html;charset=utf-8," + pending_html_);
        pending_html_.clear();
    }
}

// ── on_browser_closed ──────────────────────────────────────────

void CefBridge::on_browser_closed()
{
    std::lock_guard<std::mutex> lk(mutex_);
    js_bridge_->clear_browser();
    browser_ = nullptr;
    browser_ready_ = false;
}

// ── shutdown ───────────────────────────────────────────────────

void CefBridge::shutdown()
{
    {
        std::lock_guard<std::mutex> lk(mutex_);
        if (shutdown_called_) return;
        shutdown_called_ = true;
    }

    if (browser_) {
        browser_->GetHost()->CloseBrowser(true);
        browser_ = nullptr;
    }

    client_          = nullptr;
    render_handler_  = nullptr;
}

// ── Navigation ─────────────────────────────────────────────────

void CefBridge::load_url(std::string_view url)
{
    std::lock_guard<std::mutex> lk(mutex_);
    if (browser_) {
        browser_->GetMainFrame()->LoadURL(
            CefString(std::string(url)));
    } else {
        pending_url_ = std::string(url);
    }
}

void CefBridge::load_html(std::string_view html, std::string_view base_url)
{
    std::lock_guard<std::mutex> lk(mutex_);
    if (browser_) {
        // Use a data URI or CefStreamReader for larger documents.
        std::string uri = "data:text/html;charset=utf-8,";
        uri += std::string(html);
        browser_->GetMainFrame()->LoadURL(uri);
    } else {
        pending_html_          = std::string(html);
        pending_html_base_url_ = std::string(base_url);
    }
}

void CefBridge::reload(bool ignore_cache)
{
    std::lock_guard<std::mutex> lk(mutex_);
    if (!browser_) return;
    if (ignore_cache)
        browser_->ReloadIgnoreCache();
    else
        browser_->Reload();
}

void CefBridge::stop_loading()
{
    std::lock_guard<std::mutex> lk(mutex_);
    if (browser_) browser_->StopLoad();
}

void CefBridge::go_back()
{
    std::lock_guard<std::mutex> lk(mutex_);
    if (browser_) browser_->GoBack();
}

void CefBridge::go_forward()
{
    std::lock_guard<std::mutex> lk(mutex_);
    if (browser_) browser_->GoForward();
}

// ── Geometry ───────────────────────────────────────────────────

void CefBridge::resize(int w, int h)
{
    if (render_handler_) render_handler_->setSize(w, h);
    std::lock_guard<std::mutex> lk(mutex_);
    if (browser_) browser_->GetHost()->WasResized();
}

void CefBridge::set_device_scale(float scale)
{
    if (render_handler_) render_handler_->setDeviceScale(scale);
    std::lock_guard<std::mutex> lk(mutex_);
    if (browser_) browser_->GetHost()->NotifyScreenInfoChanged();
}

// ── Focus ──────────────────────────────────────────────────────

void CefBridge::set_focus(bool focused)
{
    std::lock_guard<std::mutex> lk(mutex_);
    if (browser_) InputForwarder::sendFocus(browser_, focused);
}

// ── Input ──────────────────────────────────────────────────────

void CefBridge::send_mouse_move(const WebMouseMoveEvent& e)
{
    std::lock_guard<std::mutex> lk(mutex_);
    if (browser_) InputForwarder::sendMouseMove(browser_, e);
}

void CefBridge::send_mouse_click(const WebMouseClickEvent& e)
{
    std::lock_guard<std::mutex> lk(mutex_);
    if (browser_) InputForwarder::sendMouseClick(browser_, e);
}

void CefBridge::send_mouse_wheel(const WebMouseWheelEvent& e)
{
    std::lock_guard<std::mutex> lk(mutex_);
    if (browser_) InputForwarder::sendMouseWheel(browser_, e);
}

void CefBridge::send_key(const WebKeyEvent& e)
{
    std::lock_guard<std::mutex> lk(mutex_);
    if (browser_) InputForwarder::sendKey(browser_, e);
}

void CefBridge::send_text(const WebTextInputEvent& e)
{
    std::lock_guard<std::mutex> lk(mutex_);
    if (browser_) InputForwarder::sendText(browser_, e);
}

// ── JavaScript ─────────────────────────────────────────────────

void CefBridge::eval_js(std::string_view script)
{
    js_bridge_->eval_js(script);
}

void CefBridge::eval_js_in_frame(std::string_view script,
                                  std::string_view frame_name)
{
    js_bridge_->eval_js_in_frame(script, frame_name);
}

void CefBridge::bind_function(
    std::string_view name,
    std::function<std::string(std::string_view)> fn)
{
    if (client_) {
        client_->register_bound_function(std::string(name), fn);
    }
    js_bridge_->bind_function(name, std::move(fn));
}

// ── Queries ────────────────────────────────────────────────────

bool CefBridge::is_loading()     const { return is_loading_.load(); }
bool CefBridge::can_go_back()    const { return can_go_back_.load(); }
bool CefBridge::can_go_forward() const { return can_go_forward_.load(); }

std::string CefBridge::get_url() const {
    std::lock_guard<std::mutex> lk(state_mutex_);
    return current_url_;
}
std::string CefBridge::get_title() const {
    std::lock_guard<std::mutex> lk(state_mutex_);
    return current_title_;
}

// ── Callbacks ──────────────────────────────────────────────────

void CefBridge::set_on_paint      (OnPaintCallback    cb) { std::lock_guard<std::mutex> lk(mutex_); on_paint_      = std::move(cb); }
void CefBridge::set_on_title      (OnTitleCallback    cb) { std::lock_guard<std::mutex> lk(mutex_); on_title_      = std::move(cb); }
void CefBridge::set_on_url        (OnURLCallback      cb) { std::lock_guard<std::mutex> lk(mutex_); on_url_        = std::move(cb); }
void CefBridge::set_on_load       (OnLoadCallback     cb) { std::lock_guard<std::mutex> lk(mutex_); on_load_       = std::move(cb); }
void CefBridge::set_on_load_start (OnLoadStartCallback cb){ std::lock_guard<std::mutex> lk(mutex_); on_load_start_ = std::move(cb); }
void CefBridge::set_on_console    (OnConsoleCallback  cb) { std::lock_guard<std::mutex> lk(mutex_); on_console_    = std::move(cb); }
void CefBridge::set_on_new_window (OnNewWindowCallback cb){ std::lock_guard<std::mutex> lk(mutex_); on_new_window_ = std::move(cb); }

} // namespace enki::web
