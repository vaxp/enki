/// @file NativeAPIs.cpp
/// @brief Implementation of Built-in Native APIs for Enki Web Host.
///
/// @copyright ENKI Framework — MIT License

#include <web_technology/NativeAPIs.hpp>
#include <web_technology/IWebViewBackend.hpp>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <pwd.h>
#include <regex>

namespace fs = std::filesystem;

namespace {

// ── Simple JSON string escaping & parsing helpers ───────────────
std::string json_escape(std::string_view s) {
    std::string out;
    out.reserve(s.size() + 16);
    for (char c : s) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\b': out += "\\b"; break;
            case '\f': out += "\\f"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    char buf[8];
                    snprintf(buf, sizeof(buf), "\\u%04x", c);
                    out += buf;
                } else {
                    out += c;
                }
        }
    }
    return out;
}

std::string get_json_field(const std::string& json, const std::string& key) {
    std::string pat = "\"" + key + "\"\\s*:\\s*\"([^\"]*)\"";
    std::regex rx(pat);
    std::smatch m;
    if (std::regex_search(json, m, rx)) return m[1].str();

    // try non-quoted (numbers/booleans)
    pat = "\"" + key + "\"\\s*:\\s*([^,}\\]\\s]+)";
    std::regex rx2(pat);
    if (std::regex_search(json, m, rx2)) return m[1].str();
    return "";
}

bool has_permission(const std::vector<std::string>& perms, std::string_view req) {
    for (const auto& p : perms) {
        if (p == req || p == "*") return true;
        // e.g. "fs.*" matches "fs.read"
        if (p.ends_with(".*") && req.starts_with(p.substr(0, p.size() - 2))) {
            return true;
        }
    }
    return false;
}

} // anonymous namespace

namespace enki::web {

// ════════════════════════════════════════════════════════════════
// FileSystemAPI
// ════════════════════════════════════════════════════════════════

bool FileSystemAPI::is_permitted(const std::vector<std::string>& perms) const {
    return has_permission(perms, "fs.read") || has_permission(perms, "fs.write");
}

void FileSystemAPI::register_functions(IWebViewBackend& backend) {
    // ── readFile ────────────────────────────────────────────────
    backend.bind_function("__enki_fs_readFile", [](std::string_view args_json) -> std::string {
        std::string path = get_json_field(std::string(args_json), "path");
        if (path.empty()) return "{\"error\":\"Path required\"}";
        try {
            std::ifstream file(path, std::ios::binary);
            if (!file) return "{\"error\":\"File not found or cannot be opened\"}";
            std::ostringstream ss;
            ss << file.rdbuf();
            std::string content = ss.str();
            return "{\"content\":\"" + json_escape(content) + "\"}";
        } catch (const std::exception& e) {
            return "{\"error\":\"" + json_escape(e.what()) + "\"}";
        }
    });

    // ── writeFile ───────────────────────────────────────────────
    backend.bind_function("__enki_fs_writeFile", [](std::string_view args_json) -> std::string {
        std::string path = get_json_field(std::string(args_json), "path");
        std::string data = get_json_field(std::string(args_json), "data");
        if (path.empty()) return "{\"error\":\"Path required\"}";
        try {
            std::ofstream file(path, std::ios::binary | std::ios::trunc);
            if (!file) return "{\"error\":\"Cannot open file for writing\"}";
            file << data;
            return "{\"success\":true}";
        } catch (const std::exception& e) {
            return "{\"error\":\"" + json_escape(e.what()) + "\"}";
        }
    });

    // ── exists ──────────────────────────────────────────────────
    backend.bind_function("__enki_fs_exists", [](std::string_view args_json) -> std::string {
        std::string path = get_json_field(std::string(args_json), "path");
        bool ex = !path.empty() && fs::exists(path);
        return ex ? "{\"exists\":true}" : "{\"exists\":false}";
    });

    // ── listDir ─────────────────────────────────────────────────
    backend.bind_function("__enki_fs_listDir", [](std::string_view args_json) -> std::string {
        std::string path = get_json_field(std::string(args_json), "path");
        if (path.empty()) return "{\"error\":\"Path required\"}";
        try {
            std::ostringstream out;
            out << "{\"entries\":[";
            bool first = true;
            for (const auto& entry : fs::directory_iterator(path)) {
                if (!first) out << ",";
                first = false;
                out << "{\"name\":\"" << json_escape(entry.path().filename().string()) << "\","
                    << "\"is_dir\":" << (entry.is_directory() ? "true" : "false") << ","
                    << "\"size\":" << (entry.is_regular_file() ? entry.file_size() : 0) << "}";
            }
            out << "]}";
            return out.str();
        } catch (const std::exception& e) {
            return "{\"error\":\"" + json_escape(e.what()) + "\"}";
        }
    });

    // ── mkdir ───────────────────────────────────────────────────
    backend.bind_function("__enki_fs_mkdir", [](std::string_view args_json) -> std::string {
        std::string path = get_json_field(std::string(args_json), "path");
        if (path.empty()) return "{\"error\":\"Path required\"}";
        try {
            fs::create_directories(path);
            return "{\"success\":true}";
        } catch (const std::exception& e) {
            return "{\"error\":\"" + json_escape(e.what()) + "\"}";
        }
    });

    // ── remove ──────────────────────────────────────────────────
    backend.bind_function("__enki_fs_remove", [](std::string_view args_json) -> std::string {
        std::string path = get_json_field(std::string(args_json), "path");
        if (path.empty()) return "{\"error\":\"Path required\"}";
        try {
            fs::remove_all(path);
            return "{\"success\":true}";
        } catch (const std::exception& e) {
            return "{\"error\":\"" + json_escape(e.what()) + "\"}";
        }
    });

    // Inject JS helper object
    backend.eval_js(R"js(
        window.enki = window.enki || {};
        window.enki.fs = {
            readFile: function(path, enc) { return window.enki.__call('__enki_fs_readFile', {path: path, encoding: enc}).then(r => r.content || Promise.reject(r.error)); },
            writeFile: function(path, data) { return window.enki.__call('__enki_fs_writeFile', {path: path, data: data}).then(r => r.success ? true : Promise.reject(r.error)); },
            exists: function(path) { return window.enki.__call('__enki_fs_exists', {path: path}).then(r => r.exists); },
            listDir: function(path) { return window.enki.__call('__enki_fs_listDir', {path: path}).then(r => r.entries || Promise.reject(r.error)); },
            mkdir: function(path) { return window.enki.__call('__enki_fs_mkdir', {path: path}).then(r => r.success ? true : Promise.reject(r.error)); },
            remove: function(path) { return window.enki.__call('__enki_fs_remove', {path: path}).then(r => r.success ? true : Promise.reject(r.error)); }
        };
    )js");
}

// ════════════════════════════════════════════════════════════════
// DialogAPI
// ════════════════════════════════════════════════════════════════

bool DialogAPI::is_permitted(const std::vector<std::string>& perms) const {
    return has_permission(perms, "dialog");
}

void DialogAPI::register_functions(IWebViewBackend& backend) {
    backend.bind_function("__enki_dialog_openFile", [](std::string_view args_json) -> std::string {
        std::string title = get_json_field(std::string(args_json), "title");
        if (title.empty()) title = "Open File";

        // Try zenity file chooser
        std::string cmd = "zenity --file-selection --title=\"" + json_escape(title) + "\" 2>/dev/null";
        FILE* pipe = popen(cmd.c_str(), "r");
        if (!pipe) return "{\"path\":null}";

        char buffer[1024];
        std::string result = "";
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            result += buffer;
        }
        pclose(pipe);

        while (!result.empty() && (result.back() == '\n' || result.back() == '\r')) {
            result.pop_back();
        }

        if (result.empty()) return "{\"path\":null}";
        std::string fname = fs::path(result).filename().string();
        return "{\"path\":\"" + json_escape(result) + "\",\"name\":\"" + json_escape(fname) + "\"}";
    });

    backend.bind_function("__enki_dialog_saveFile", [](std::string_view args_json) -> std::string {
        std::string title = get_json_field(std::string(args_json), "title");
        if (title.empty()) title = "Save File";

        std::string cmd = "zenity --file-selection --save --confirm-overwrite --title=\"" + json_escape(title) + "\" 2>/dev/null";
        FILE* pipe = popen(cmd.c_str(), "r");
        if (!pipe) return "{\"path\":null}";

        char buffer[1024];
        std::string result = "";
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            result += buffer;
        }
        pclose(pipe);

        while (!result.empty() && (result.back() == '\n' || result.back() == '\r')) {
            result.pop_back();
        }

        if (result.empty()) return "{\"path\":null}";
        return "{\"path\":\"" + json_escape(result) + "\"}";
    });

    backend.eval_js(R"js(
        window.enki = window.enki || {};
        window.enki.dialog = {
            openFile: function(opts) { return window.enki.__call('__enki_dialog_openFile', opts || {}); },
            saveFile: function(opts) { return window.enki.__call('__enki_dialog_saveFile', opts || {}).then(r => r.path); }
        };
    )js");
}

// ════════════════════════════════════════════════════════════════
// WindowAPI
// ════════════════════════════════════════════════════════════════

void WindowAPI::register_functions(IWebViewBackend& backend) {
    backend.bind_function("__enki_window_setTitle", [](std::string_view args_json) -> std::string {
        std::string title = get_json_field(std::string(args_json), "title");
        // Title change
        return "{\"success\":true}";
    });

    backend.eval_js(R"js(
        window.enki = window.enki || {};
        window.enki.window = {
            setTitle: function(t) { document.title = t; return window.enki.__call('__enki_window_setTitle', {title: t}); },
            minimize: function() { console.log('window.minimize called'); },
            maximize: function() { console.log('window.maximize called'); },
            restore:  function() { console.log('window.restore called'); },
            close:    function() { window.close(); }
        };
    )js");
}

// ════════════════════════════════════════════════════════════════
// NotificationAPI
// ════════════════════════════════════════════════════════════════

bool NotificationAPI::is_permitted(const std::vector<std::string>& perms) const {
    return has_permission(perms, "notifications");
}

void NotificationAPI::register_functions(IWebViewBackend& backend) {
    backend.bind_function("__enki_notification_show", [](std::string_view args_json) -> std::string {
        std::string title = get_json_field(std::string(args_json), "title");
        std::string body  = get_json_field(std::string(args_json), "body");
        if (title.empty()) title = "Notification";

        std::string cmd = "notify-send \"" + json_escape(title) + "\" \"" + json_escape(body) + "\" 2>/dev/null &";
        int res = system(cmd.c_str());
        (void)res;
        return "{\"success\":true}";
    });

    backend.eval_js(R"js(
        window.enki = window.enki || {};
        window.enki.notification = {
            show: function(opts) { return window.enki.__call('__enki_notification_show', opts || {}); }
        };
    )js");
}

// ════════════════════════════════════════════════════════════════
// ClipboardAPI
// ════════════════════════════════════════════════════════════════

bool ClipboardAPI::is_permitted(const std::vector<std::string>& perms) const {
    return has_permission(perms, "clipboard");
}

void ClipboardAPI::register_functions(IWebViewBackend& backend) {
    backend.bind_function("__enki_clipboard_write", [](std::string_view args_json) -> std::string {
        std::string text = get_json_field(std::string(args_json), "text");
        // write to xclip or wl-copy
        std::string cmd = "wl-copy 2>/dev/null || xclip -selection clipboard 2>/dev/null";
        FILE* pipe = popen(cmd.c_str(), "w");
        if (pipe) {
            fputs(text.c_str(), pipe);
            pclose(pipe);
        }
        return "{\"success\":true}";
    });

    backend.eval_js(R"js(
        window.enki = window.enki || {};
        window.enki.clipboard = {
            write: function(text) { return window.enki.__call('__enki_clipboard_write', {text: text}); }
        };
    )js");
}

// ════════════════════════════════════════════════════════════════
// ShellAPI
// ════════════════════════════════════════════════════════════════

bool ShellAPI::is_permitted(const std::vector<std::string>& perms) const {
    return has_permission(perms, "shell.exec") || has_permission(perms, "shell.open_external");
}

void ShellAPI::register_functions(IWebViewBackend& backend) {
    backend.bind_function("__enki_shell_openExternal", [](std::string_view args_json) -> std::string {
        std::string url = get_json_field(std::string(args_json), "url");
        if (!url.empty()) {
            std::string cmd = "xdg-open \"" + json_escape(url) + "\" 2>/dev/null &";
            int res = system(cmd.c_str());
            (void)res;
        }
        return "{\"success\":true}";
    });

    backend.eval_js(R"js(
        window.enki = window.enki || {};
        window.enki.shell = {
            openExternal: function(url) { return window.enki.__call('__enki_shell_openExternal', {url: url}); }
        };
    )js");
}

// ════════════════════════════════════════════════════════════════
// SystemAPI
// ════════════════════════════════════════════════════════════════

bool SystemAPI::is_permitted(const std::vector<std::string>& perms) const {
    return has_permission(perms, "system.info");
}

void SystemAPI::register_functions(IWebViewBackend& backend) {
    backend.bind_function("__enki_system_getInfo", [](std::string_view) -> std::string {
        struct utsname ut;
        uname(&ut);

        struct sysinfo si;
        sysinfo(&si);

        const char* user = getenv("USER");
        const char* home = getenv("HOME");
        char hostname[256] = "localhost";
        gethostname(hostname, sizeof(hostname));

        std::ostringstream out;
        out << "{"
            << "\"platform\":\"linux\","
            << "\"arch\":\"" << json_escape(ut.machine) << "\","
            << "\"kernel\":\"" << json_escape(ut.release) << "\","
            << "\"hostname\":\"" << json_escape(hostname) << "\","
            << "\"username\":\"" << (user ? json_escape(user) : "user") << "\","
            << "\"homeDir\":\"" << (home ? json_escape(home) : "/home") << "\","
            << "\"memory\":{"
            <<   "\"total\":" << (uint64_t)si.totalram * si.mem_unit << ","
            <<   "\"free\":" << (uint64_t)si.freeram * si.mem_unit
            << "},"
            << "\"cpus\":" << sysconf(_SC_NPROCESSORS_ONLN)
            << "}";
        return out.str();
    });

    backend.eval_js(R"js(
        window.enki = window.enki || {};
        window.enki.system = {
            platform: function() { return "linux"; },
            arch: function() { return "x64"; },
            hostname: function() { return location.hostname || "localhost"; },
            username: function() { return "user"; },
            homeDir: function() { return "/home"; },
            appDir: function() { return location.pathname; },
            memory: function() { return window.enki.__call('__enki_system_getInfo').then(r => r.memory); },
            cpuCount: function() { return navigator.hardwareConcurrency || 4; }
        };
    )js");
}

// ════════════════════════════════════════════════════════════════
// PathAPI
// ════════════════════════════════════════════════════════════════

void PathAPI::register_functions(IWebViewBackend& backend) {
    backend.eval_js(R"js(
        window.enki = window.enki || {};
        window.enki.path = {
            join: function() {
                var args = Array.prototype.slice.call(arguments);
                return args.join('/').replace(/\/+/g, '/');
            },
            dirname: function(p) {
                var idx = p.lastIndexOf('/');
                return idx === -1 ? '.' : p.substr(0, idx) || '/';
            },
            basename: function(p, ext) {
                var base = p.substr(p.lastIndexOf('/') + 1);
                if (ext && base.endsWith(ext)) base = base.substr(0, base.length - ext.length);
                return base;
            },
            extname: function(p) {
                var idx = p.lastIndexOf('.');
                return idx === -1 ? '' : p.substr(idx);
            },
            isAbsolute: function(p) { return p.startsWith('/'); }
        };
    )js");
}

} // namespace enki::web
