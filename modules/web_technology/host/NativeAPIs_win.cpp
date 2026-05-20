/// @file NativeAPIs_win.cpp
/// @brief Windows-specific Native APIs for Enki Web Host (Clipboard, Shell, Dialog, SystemInfo).
/// @copyright ENKI Framework — MIT License

#if defined(_WIN32)

#include <web_technology/NativeAPIs.hpp>
#include <web_technology/IWebViewBackend.hpp>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <commdlg.h>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>
#include <regex>
#include <vector>
#include <string>

namespace fs = std::filesystem;

namespace {

std::string json_escape(std::string_view s) {
    std::string out;
    out.reserve(s.size() + 16);
    for (char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\b': out += "\\b";  break;
            case '\f': out += "\\f";  break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
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

    pat = "\"" + key + "\"\\s*:\\s*([^,}\\]\\s]+)";
    std::regex rx2(pat);
    if (std::regex_search(json, m, rx2)) return m[1].str();
    return "";
}

bool has_permission(const std::vector<std::string>& perms, std::string_view req) {
    for (const auto& p : perms) {
        if (p == req || p == "*") return true;
        if (p.ends_with(".*") && req.starts_with(p.substr(0, p.size() - 2))) {
            return true;
        }
    }
    return false;
}

std::wstring utf8_to_wide(std::string_view s) {
    if (s.empty()) return L"";
    int len = MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), nullptr, 0);
    if (len <= 0) return L"";
    std::wstring out(len, 0);
    MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), out.data(), len);
    return out;
}

std::string wide_to_utf8(std::wstring_view ws) {
    if (ws.empty()) return "";
    int len = WideCharToMultiByte(CP_UTF8, 0, ws.data(), static_cast<int>(ws.size()), nullptr, 0, nullptr, nullptr);
    if (len <= 0) return "";
    std::string out(len, 0);
    WideCharToMultiByte(CP_UTF8, 0, ws.data(), static_cast<int>(ws.size()), out.data(), len, nullptr, nullptr);
    return out;
}

} // anonymous namespace

namespace enki::web {

// ════════════════════════════════════════════════════════════════
// DialogAPI (Windows GetOpenFileNameW / GetSaveFileNameW)
// ════════════════════════════════════════════════════════════════

bool DialogAPI::is_permitted(const std::vector<std::string>& perms) const {
    return has_permission(perms, "dialog");
}

void DialogAPI::register_functions(IWebViewBackend& backend) {
    backend.bind_function("__enki_dialog_openFile", [](std::string_view args_json) -> std::string {
        std::string title = get_json_field(std::string(args_json), "title");
        if (title.empty()) title = "Open File";
        std::wstring wtitle = utf8_to_wide(title);

        wchar_t szFile[MAX_PATH] = {0};
        OPENFILENAMEW ofn{};
        ofn.lStructSize = sizeof(ofn);
        ofn.lpstrFile   = szFile;
        ofn.nMaxFile    = MAX_PATH;
        ofn.lpstrTitle  = wtitle.c_str();
        ofn.lpstrFilter = L"All Files (*.*)\0*.*\0";
        ofn.nFilterIndex = 1;
        ofn.Flags       = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

        if (GetOpenFileNameW(&ofn)) {
            std::string result = wide_to_utf8(szFile);
            std::string fname  = fs::path(szFile).filename().string();
            return "{\"path\":\"" + json_escape(result) + "\",\"name\":\"" + json_escape(fname) + "\"}";
        }
        return "{\"path\":null}";
    });

    backend.bind_function("__enki_dialog_saveFile", [](std::string_view args_json) -> std::string {
        std::string title = get_json_field(std::string(args_json), "title");
        if (title.empty()) title = "Save File";
        std::wstring wtitle = utf8_to_wide(title);

        wchar_t szFile[MAX_PATH] = {0};
        OPENFILENAMEW ofn{};
        ofn.lStructSize = sizeof(ofn);
        ofn.lpstrFile   = szFile;
        ofn.nMaxFile    = MAX_PATH;
        ofn.lpstrTitle  = wtitle.c_str();
        ofn.lpstrFilter = L"All Files (*.*)\0*.*\0";
        ofn.nFilterIndex = 1;
        ofn.Flags       = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

        if (GetSaveFileNameW(&ofn)) {
            std::string result = wide_to_utf8(szFile);
            return "{\"path\":\"" + json_escape(result) + "\"}";
        }
        return "{\"path\":null}";
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
// NotificationAPI (Windows Native / PowerShell Notification)
// ════════════════════════════════════════════════════════════════

bool NotificationAPI::is_permitted(const std::vector<std::string>& perms) const {
    return has_permission(perms, "notifications");
}

void NotificationAPI::register_functions(IWebViewBackend& backend) {
    backend.bind_function("__enki_notification_show", [](std::string_view args_json) -> std::string {
        std::string title = get_json_field(std::string(args_json), "title");
        std::string body  = get_json_field(std::string(args_json), "body");
        if (title.empty()) title = "Notification";

        std::string ps_cmd = "powershell -NoProfile -WindowStyle Hidden -Command \"[void][System.Reflection.Assembly]::LoadWithPartialName('System.Windows.Forms'); $n = New-Object System.Windows.Forms.NotifyIcon; $n.Icon = [System.Drawing.SystemIcons]::Information; $n.Visible = $true; $n.ShowBalloonTip(3000, '" +
                             title + "', '" + body + "', [System.Windows.Forms.ToolTipIcon]::Info)\"";
        
        STARTUPINFOA si{};
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;
        PROCESS_INFORMATION pi{};
        CreateProcessA(nullptr, ps_cmd.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi);
        if (pi.hProcess) { CloseHandle(pi.hProcess); CloseHandle(pi.hThread); }

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
// ClipboardAPI (Win32 Clipboard)
// ════════════════════════════════════════════════════════════════

bool ClipboardAPI::is_permitted(const std::vector<std::string>& perms) const {
    return has_permission(perms, "clipboard");
}

void ClipboardAPI::register_functions(IWebViewBackend& backend) {
    backend.bind_function("__enki_clipboard_write", [](std::string_view args_json) -> std::string {
        std::string text = get_json_field(std::string(args_json), "text");
        std::wstring wtext = utf8_to_wide(text);

        if (OpenClipboard(nullptr)) {
            EmptyClipboard();
            size_t bytes = (wtext.size() + 1) * sizeof(wchar_t);
            HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, bytes);
            if (hMem) {
                memcpy(GlobalLock(hMem), wtext.c_str(), bytes);
                GlobalUnlock(hMem);
                SetClipboardData(CF_UNICODETEXT, hMem);
            }
            CloseClipboard();
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
// ShellAPI (Win32 ShellExecute)
// ════════════════════════════════════════════════════════════════

bool ShellAPI::is_permitted(const std::vector<std::string>& perms) const {
    return has_permission(perms, "shell.exec") || has_permission(perms, "shell.open_external");
}

void ShellAPI::register_functions(IWebViewBackend& backend) {
    backend.bind_function("__enki_shell_openExternal", [](std::string_view args_json) -> std::string {
        std::string url = get_json_field(std::string(args_json), "url");
        if (!url.empty()) {
            ShellExecuteA(nullptr, "open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
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
// SystemAPI (Win32 System Information)
// ════════════════════════════════════════════════════════════════

bool SystemAPI::is_permitted(const std::vector<std::string>& perms) const {
    return has_permission(perms, "system.info");
}

void SystemAPI::register_functions(IWebViewBackend& backend) {
    backend.bind_function("__enki_system_getInfo", [](std::string_view) -> std::string {
        char hostname[MAX_COMPUTERNAME_LENGTH + 1] = "localhost";
        DWORD host_len = sizeof(hostname);
        GetComputerNameA(hostname, &host_len);

        char username[256] = "user";
        DWORD user_len = sizeof(username);
        GetUserNameA(username, &user_len);

        const char* userprof = getenv("USERPROFILE");
        std::string home_dir = userprof ? userprof : "C:\\";

        MEMORYSTATUSEX mem{};
        mem.dwLength = sizeof(mem);
        GlobalMemoryStatusEx(&mem);

        SYSTEM_INFO sys{};
        GetSystemInfo(&sys);

        std::ostringstream out;
        out << "{"
            << "\"platform\":\"windows\","
            << "\"arch\":\"x64\","
            << "\"kernel\":\"Windows NT\","
            << "\"hostname\":\"" << json_escape(hostname) << "\","
            << "\"username\":\"" << json_escape(username) << "\","
            << "\"homeDir\":\"" << json_escape(home_dir) << "\","
            << "\"memory\":{"
            <<   "\"total\":" << mem.ullTotalPhys << ","
            <<   "\"free\":" << mem.ullAvailPhys
            << "},"
            << "\"cpus\":" << sys.dwNumberOfProcessors
            << "}";
        return out.str();
    });

    backend.eval_js(R"js(
        window.enki = window.enki || {};
        window.enki.system = {
            platform: function() { return "windows"; },
            arch: function() { return "x64"; },
            hostname: function() { return location.hostname || "localhost"; },
            username: function() { return "user"; },
            homeDir: function() { return "C:\\"; },
            appDir: function() { return location.pathname; },
            memory: function() { return window.enki.__call('__enki_system_getInfo').then(r => r.memory); },
            cpuCount: function() { return navigator.hardwareConcurrency || 4; }
        };
    )js");
}

} // namespace enki::web

#endif // _WIN32
