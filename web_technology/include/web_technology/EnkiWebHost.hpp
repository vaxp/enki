#pragma once
/// @file EnkiWebHost.hpp
/// @brief Enki Web Host — مشغّل تطبيقات الويب داخل Enki.
///
/// EnkiWebHost يحوّل Enki إلى منصة مشابهة لـ Electron:
///   - يقرأ enki.json للحصول على إعدادات التطبيق
///   - يفتح نافذة Enki أصلية (X11/Wayland)
///   - يُشغّل CEF ليملأ النافذة بالكامل
///   - يُسجّل Native APIs ويعرضها لـ JS عبر window.enki.*
///
/// الاستخدام (main.cpp لمطور التطبيق):
/// @code
///   #include <enki/web_host/EnkiWebHost.hpp>
///
///   int main(int argc, char* argv[]) {
///       enki::web::EnkiWebHost host(argc, argv);
///       host.load_config("enki.json");     // أو يُحدَّد تلقائياً
///       host.register_api<FileSystemAPI>();
///       host.register_api<DialogAPI>();
///       return host.run();
///   }
/// @endcode
///
/// @copyright ENKI Framework — MIT License

#include <string>
#include <memory>
#include <vector>
#include <functional>

// Forward declarations — zero CEF types in this header
namespace enki::web {
    class INativeAPI;
    class IWebViewBackend;
    class NativeAPIRegistry;
    struct AppConfig;
}

namespace enki::web {

// ════════════════════════════════════════════════════════════════
// AppConfig — محتوى enki.json
// ════════════════════════════════════════════════════════════════

struct WindowConfig {
    std::string title      = "Enki App";
    int         width      = 1280;
    int         height     = 800;
    int         min_width  = 400;
    int         min_height = 300;
    bool        resizable  = true;
    bool        frameless  = false;
    bool        transparent= false;
    bool        center     = true;
    bool        maximized  = false;
    bool        fullscreen = false;
};

struct AppConfig {
    std::string   name            = "App";
    std::string   version         = "1.0.0";
    std::string   entry           = "src/index.html"; // relative to app root
    std::string   app_root;                            // مجلد enki.json

    WindowConfig  window;

    std::vector<std::string> permissions;  // ["fs.read", "dialog", ...]

    bool          devtools         = false;
    bool          single_instance  = true;
    bool          hardware_accel   = true;
    float         device_scale     = 1.0f;

    // تحميل من ملف JSON
    static AppConfig from_file(const std::string& path);

    // بناء URL الـ entry point الكامل
    [[nodiscard]] std::string entry_url() const;
};

// ════════════════════════════════════════════════════════════════
// INativeAPI — واجهة API أصلي قابل للتسجيل
// ════════════════════════════════════════════════════════════════

/// كل Native API يرث من هذه الواجهة ويسجّل functions في JS.
class INativeAPI {
public:
    virtual ~INativeAPI() = default;

    /// اسم الـ namespace في JS (مثال: "fs" → window.enki.fs.*)
    [[nodiscard]] virtual std::string_view name() const = 0;

    /// هل هذا الـ API مسموح به بحسب permissions؟
    [[nodiscard]] virtual bool is_permitted(
        const std::vector<std::string>& permissions) const = 0;

    /// تسجيل جميع functions في الـ JSBridge.
    virtual void register_functions(IWebViewBackend& backend) = 0;
};

// ════════════════════════════════════════════════════════════════
// EnkiWebHost — مشغّل تطبيق الويب
// ════════════════════════════════════════════════════════════════

class EnkiWebHost {
public:
    explicit EnkiWebHost(int argc, char* argv[]);
    ~EnkiWebHost();

    // Non-copyable
    EnkiWebHost(const EnkiWebHost&) = delete;
    EnkiWebHost& operator=(const EnkiWebHost&) = delete;

    // ── Configuration ──────────────────────────────────────────

    /// تحميل إعدادات التطبيق من enki.json.
    /// إذا لم يُحدَّد المسار يبحث في مجلد الـ binary.
    bool load_config(const std::string& config_path = "enki.json");

    /// تحديد الإعدادات برمجياً (بديل عن enki.json).
    void set_config(AppConfig config);

    // ── Native APIs ────────────────────────────────────────────

    /// تسجيل Native API وإتاحته لـ JS عبر window.enki.<name>.*
    template <typename T>
    void register_api() {
        static_assert(std::is_base_of_v<INativeAPI, T>,
                      "T must derive from INativeAPI");
        add_api(std::make_unique<T>());
    }

    /// تسجيل API مُنشَأ خارجياً.
    void add_api(std::unique_ptr<INativeAPI> api);

    // ── Run ────────────────────────────────────────────────────

    /// تشغيل الـ host: يفتح النافذة ويُشغّل حلقة الأحداث.
    /// يعود عند إغلاق التطبيق.
    /// @return exit code.
    int run();

    // ── Window control (callable from C++ code) ────────────────

    void set_title(const std::string& title);
    void set_size(int w, int h);
    void minimize();
    void maximize();
    void restore();
    void close();

    // ── JS evaluation ──────────────────────────────────────────

    /// تنفيذ JavaScript في نافذة التطبيق.
    void eval_js(std::string_view script);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;

    void init_cef_subprocess();
    void build_js_bootstrap();   // ينشئ window.enki object في JS
    void register_default_apis();
};

} // namespace enki::web
