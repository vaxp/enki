/// @file EnkiWebHost.cpp
/// @brief تنفيذ EnkiWebHost — مشغّل تطبيقات الويب في Enki.
///
/// @copyright ENKI Framework — MIT License

#include <web_technology/EnkiWebHost.hpp>
#include <web_technology/NativeAPIs.hpp>
#include "../cef/CefBridge.hpp"

#include <filesystem>
#include <iostream>
#include <sstream>
#include <vector>

namespace fs = std::filesystem;

namespace enki::web {

// ════════════════════════════════════════════════════════════════
// EnkiWebHost::Impl — التفاصيل الداخلية
// ════════════════════════════════════════════════════════════════

struct EnkiWebHost::Impl {
    int    argc;
    char** argv;

    AppConfig                          config;
    bool                               config_loaded = false;
    std::vector<std::unique_ptr<INativeAPI>> apis;
    std::unique_ptr<IWebViewBackend>   backend;

    // ── native platform window handle (X11 / Wayland) ─────────
    // يُملَأ بعد فتح النافذة الأصلية
    void* native_window_handle = nullptr;

    // ── JS bootstrap snippet ───────────────────────────────────
    std::string js_bootstrap; // ينشئ window.enki object
};

// ════════════════════════════════════════════════════════════════
// Constructor / Destructor
// ════════════════════════════════════════════════════════════════

EnkiWebHost::EnkiWebHost(int argc, char* argv[])
    : impl_(std::make_unique<Impl>())
{
    impl_->argc = argc;
    impl_->argv = argv;

    // تهيئة CEF subprocess إذا كنا في subprocess mode
    // (CEF يُشغّل هذا البرنامج نفسه كـ renderer/gpu process)
    init_cef_subprocess();
}

EnkiWebHost::~EnkiWebHost()
{
    if (impl_->backend) {
        impl_->backend->shutdown();
    }
    CefGlobal::instance().shutdown();
}

// ════════════════════════════════════════════════════════════════
// Configuration
// ════════════════════════════════════════════════════════════════

bool EnkiWebHost::load_config(const std::string& config_path)
{
    std::string path = config_path;

    // إذا لم يُحدَّد مسار كامل، ابحث بجانب الـ binary
    if (!fs::exists(path)) {
        fs::path bin_dir = fs::path(impl_->argv[0]).parent_path();
        path = (bin_dir / config_path).string();
    }

    if (!fs::exists(path)) {
        std::cerr << "[EnkiWebHost] Warning: enki.json not found at: "
                  << path << "\nUsing defaults.\n";
        impl_->config_loaded = true;
        return false;
    }

    try {
        impl_->config = AppConfig::from_file(path);
        impl_->config_loaded = true;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[EnkiWebHost] Failed to load config: " << e.what() << "\n";
        return false;
    }
}

void EnkiWebHost::set_config(AppConfig config)
{
    impl_->config = std::move(config);
    impl_->config_loaded = true;
}

// ════════════════════════════════════════════════════════════════
// Native APIs
// ════════════════════════════════════════════════════════════════

void EnkiWebHost::add_api(std::unique_ptr<INativeAPI> api)
{
    impl_->apis.push_back(std::move(api));
}

// ════════════════════════════════════════════════════════════════
// run()
// ════════════════════════════════════════════════════════════════

int EnkiWebHost::run()
{
    if (!impl_->config_loaded) {
        load_config("enki.json");  // محاولة تلقائية
    }

    const auto& cfg = impl_->config;

    // ── 1. تسجيل Default Native APIs ──────────────────────────
    register_default_apis();

    // ── 2. إنشاء CEF Backend ───────────────────────────────────
    impl_->backend = IWebViewBackend::create();
    if (!impl_->backend) {
        std::cerr << "[EnkiWebHost] CEF backend not available. "
                     "Was ENKI_HAS_WEBVIEW defined?\n";
        return 1;
    }

    // ── 3. تسجيل Native API functions في JS ───────────────────
    build_js_bootstrap();

    // ── 4. إعداد Backend ───────────────────────────────────────
    BackendConfig bcfg;
    bcfg.width          = cfg.window.width;
    bcfg.height         = cfg.window.height;
    bcfg.device_scale   = cfg.device_scale;
    bcfg.enable_js      = true;
    bcfg.enable_webgl   = true;
    bcfg.background_color = 0xFF0F0F1A;  // يطابق --bg في CSS
    bcfg.windowed_mode  = true;           
    bcfg.window_title   = cfg.window.title.empty() ? cfg.name : cfg.window.title;

    if (cfg.devtools) {
        bcfg.devtools_port = 9222;
    }

    // Ensure CEF is initialized with real argc / argv
    if (!CefGlobal::instance().ensure_initialized(impl_->argc, impl_->argv,
                                                  /*subprocess_path=*/"",
                                                  /*windowed_mode=*/true)) {
        std::cerr << "[EnkiWebHost] Failed to initialize CEF global context.\n";
        return 1;
    }

    if (!impl_->backend->initialize(bcfg)) {
        std::cerr << "[EnkiWebHost] Failed to initialize CEF backend.\n";
        return 1;
    }

    // ── 5. تسجيل جميع native functions في الـ backend ─────────
    for (auto& api : impl_->apis) {
        if (api->is_permitted(cfg.permissions)) {
            api->register_functions(*impl_->backend);
        }
    }

    // ── 6. حقن window.enki bootstrap قبل أي كود JS ────────────
    impl_->backend->set_on_load_start([this](std::string /*url*/) {
        // نحقن window.enki قبل تنفيذ JS الصفحة
        impl_->backend->eval_js(impl_->js_bootstrap);
    });

    // ── 7. تحميل صفحة التطبيق ──────────────────────────────────
    impl_->backend->load_url(cfg.entry_url());

    // ── 8. حلقة الأحداث الرئيسية ──────────────────────────────
    // TODO: دمج مع نافذة Enki الأصلية (X11/Wayland event loop)
    // في المرحلة الحالية نستخدم CefRunMessageLoop كـ standalone
    CefRunMessageLoop();

    return 0;
}

// ════════════════════════════════════════════════════════════════
// build_js_bootstrap
// ════════════════════════════════════════════════════════════════

void EnkiWebHost::build_js_bootstrap()
{
    // ينشئ window.enki object فارغاً — كل API تُضيف namespace خاصتها
    std::ostringstream js;
    js << R"js(
(function() {
    if (window.__enki_initialized) return;
    window.__enki_initialized = true;

    // ── window.enki namespace ─────────────────────────────────
    window.enki = window.enki || {};

    // ── Internal IPC helper ──────────────────────────────────
    // كل native function مُسجَّلة باسم "__enki_<api>_<fn>"
    // هذا الـ helper يستدعيها ويُرجع Promise
    window.enki.__call = function(funcName, argsObj) {
        return new Promise(function(resolve, reject) {
            try {
                var result = window[funcName](JSON.stringify(argsObj || {}));
                resolve(typeof result === 'string' ? JSON.parse(result) : result);
            } catch(e) {
                reject(e);
            }
        });
    };

    console.log('[Enki] Web Host initialized. Platform: ' +
        (window.enki.system ? window.enki.system.platform() : 'linux'));
})();
)js";

    impl_->js_bootstrap = js.str();
}

// ════════════════════════════════════════════════════════════════
// register_default_apis
// ════════════════════════════════════════════════════════════════

void EnkiWebHost::register_default_apis()
{
    // APIs المدمجة — تُضاف دائماً، يُصفّاها الـ permissions لاحقاً
    add_api(std::make_unique<FileSystemAPI>());
    add_api(std::make_unique<DialogAPI>());
    add_api(std::make_unique<WindowAPI>());
    add_api(std::make_unique<NotificationAPI>());
    add_api(std::make_unique<ClipboardAPI>());
    add_api(std::make_unique<ShellAPI>());
    add_api(std::make_unique<SystemAPI>());
    add_api(std::make_unique<PathAPI>());
}

// ════════════════════════════════════════════════════════════════
// Window control
// ════════════════════════════════════════════════════════════════

void EnkiWebHost::set_title(const std::string& title)
{
    if (impl_->backend) {
        impl_->backend->eval_js("document.title = '" + title + "';");
    }
}

void EnkiWebHost::eval_js(std::string_view script)
{
    if (impl_->backend) impl_->backend->eval_js(script);
}

// ── Window controls delegate to WindowAPI via JS ───────────────
void EnkiWebHost::minimize() { eval_js("enki.window.minimize()"); }
void EnkiWebHost::maximize() { eval_js("enki.window.maximize()"); }
void EnkiWebHost::restore()  { eval_js("enki.window.restore()");  }
void EnkiWebHost::close()    { eval_js("enki.window.close()");    }
void EnkiWebHost::set_size(int w, int h)
{
    eval_js("enki.window.setSize(" + std::to_string(w) + "," +
                                     std::to_string(h) + ")");
}

// ════════════════════════════════════════════════════════════════
// init_cef_subprocess
// ════════════════════════════════════════════════════════════════

void EnkiWebHost::init_cef_subprocess()
{
    // Check if this process was launched as a CEF child subprocess (has --type= switch)
    bool is_subprocess = false;
    for (int i = 1; i < impl_->argc; ++i) {
        if (impl_->argv[i] && std::string_view(impl_->argv[i]).starts_with("--type=")) {
            is_subprocess = true;
            break;
        }
    }

    if (is_subprocess) {
        CefMainArgs main_args(impl_->argc, impl_->argv);
        auto app = CefRefPtr<EnkiSubprocessApp>(new EnkiSubprocessApp());
        int exit_code = CefExecuteProcess(main_args, app.get(), nullptr);
        if (exit_code >= 0) {
            ::exit(exit_code);
        }
    }
}

} // namespace enki::web
