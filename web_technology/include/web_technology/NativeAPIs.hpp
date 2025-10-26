#pragma once
/// @file NativeAPIs.hpp
/// @brief مجموعة النيتف APIs المدمجة في Enki Web Host.
///
/// كل API ترث من INativeAPI وتُسجّل functions في JS عبر
/// window.enki.<name>.* namespace.
///
/// APIs المدمجة:
///   FileSystemAPI    → window.enki.fs.*
///   DialogAPI        → window.enki.dialog.*
///   WindowAPI        → window.enki.window.*
///   NotificationAPI  → window.enki.notification.*
///   ClipboardAPI     → window.enki.clipboard.*
///   ShellAPI         → window.enki.shell.*
///   SystemAPI        → window.enki.system.*
///
/// @copyright ENKI Framework — MIT License

#include "EnkiWebHost.hpp"
#include <string>
#include <vector>

namespace enki::web {

// ════════════════════════════════════════════════════════════════
// FileSystemAPI — window.enki.fs.*
// ════════════════════════════════════════════════════════════════
/// Permission: "fs.read" و "fs.write"
///
/// JS API:
///   enki.fs.readFile(path, encoding?)  → Promise<string|Uint8Array>
///   enki.fs.writeFile(path, data)      → Promise<void>
///   enki.fs.exists(path)               → Promise<bool>
///   enki.fs.listDir(path)              → Promise<DirEntry[]>
///   enki.fs.mkdir(path, recursive?)    → Promise<void>
///   enki.fs.remove(path, recursive?)   → Promise<void>
///   enki.fs.stat(path)                 → Promise<FileStat>
///   enki.fs.rename(from, to)           → Promise<void>
///   enki.fs.copy(from, to)             → Promise<void>

class FileSystemAPI : public INativeAPI {
public:
    std::string_view name() const override { return "fs"; }
    bool is_permitted(const std::vector<std::string>& perms) const override;
    void register_functions(IWebViewBackend& backend) override;
};

// ════════════════════════════════════════════════════════════════
// DialogAPI — window.enki.dialog.*
// ════════════════════════════════════════════════════════════════
/// Permission: "dialog"
///
/// JS API:
///   enki.dialog.openFile(options?)     → Promise<{path, name}|null>
///   enki.dialog.openFiles(options?)    → Promise<{path,name}[]>
///   enki.dialog.saveFile(options?)     → Promise<string|null>
///   enki.dialog.openDir(options?)      → Promise<string|null>
///   enki.dialog.message(options)       → Promise<"ok"|"cancel"|"yes"|"no">
///   enki.dialog.confirm(message)       → Promise<bool>
///   enki.dialog.prompt(message, def?)  → Promise<string|null>

class DialogAPI : public INativeAPI {
public:
    std::string_view name() const override { return "dialog"; }
    bool is_permitted(const std::vector<std::string>& perms) const override;
    void register_functions(IWebViewBackend& backend) override;
};

// ════════════════════════════════════════════════════════════════
// WindowAPI — window.enki.window.*
// ════════════════════════════════════════════════════════════════
/// Permission: "window" (دائماً متاحة)
///
/// JS API:
///   enki.window.setTitle(title)        → void
///   enki.window.getTitle()             → string
///   enki.window.setSize(w, h)          → void
///   enki.window.getSize()              → {width, height}
///   enki.window.setPosition(x, y)      → void
///   enki.window.getPosition()          → {x, y}
///   enki.window.minimize()             → void
///   enki.window.maximize()             → void
///   enki.window.restore()              → void
///   enki.window.close()                → void
///   enki.window.setFullscreen(bool)    → void
///   enki.window.isFullscreen()         → bool
///   enki.window.center()               → void
///   enki.window.setResizable(bool)     → void
///   enki.window.onClose(callback)      → void  ← intercept close

class WindowAPI : public INativeAPI {
public:
    std::string_view name() const override { return "window"; }
    bool is_permitted(const std::vector<std::string>&) const override { return true; }
    void register_functions(IWebViewBackend& backend) override;
};

// ════════════════════════════════════════════════════════════════
// NotificationAPI — window.enki.notification.*
// ════════════════════════════════════════════════════════════════
/// Permission: "notifications"
///
/// JS API:
///   enki.notification.show(options)    → void
///     options: { title, body, icon?, timeout? }

class NotificationAPI : public INativeAPI {
public:
    std::string_view name() const override { return "notification"; }
    bool is_permitted(const std::vector<std::string>& perms) const override;
    void register_functions(IWebViewBackend& backend) override;
};

// ════════════════════════════════════════════════════════════════
// ClipboardAPI — window.enki.clipboard.*
// ════════════════════════════════════════════════════════════════
/// Permission: "clipboard"
///
/// JS API:
///   enki.clipboard.read()              → Promise<string>
///   enki.clipboard.write(text)         → Promise<void>
///   enki.clipboard.clear()             → Promise<void>

class ClipboardAPI : public INativeAPI {
public:
    std::string_view name() const override { return "clipboard"; }
    bool is_permitted(const std::vector<std::string>& perms) const override;
    void register_functions(IWebViewBackend& backend) override;
};

// ════════════════════════════════════════════════════════════════
// ShellAPI — window.enki.shell.*
// ════════════════════════════════════════════════════════════════
/// Permission: "shell.exec" (خطر — تحتاج تصريحاً صريحاً)
///
/// JS API:
///   enki.shell.exec(cmd, args?)        → Promise<{stdout, stderr, code}>
///   enki.shell.openExternal(url)       → Promise<void>  ← فتح رابط بالمتصفح
///   enki.shell.openPath(path)          → Promise<void>  ← فتح ملف بالتطبيق المناسب
///   enki.shell.revealInFileManager(p)  → Promise<void>

class ShellAPI : public INativeAPI {
public:
    std::string_view name() const override { return "shell"; }
    bool is_permitted(const std::vector<std::string>& perms) const override;
    void register_functions(IWebViewBackend& backend) override;
};

// ════════════════════════════════════════════════════════════════
// SystemAPI — window.enki.system.*
// ════════════════════════════════════════════════════════════════
/// Permission: "system.info"
///
/// JS API:
///   enki.system.platform()            → "linux" | "windows" | "macos"
///   enki.system.arch()                → "x64" | "arm64" | ...
///   enki.system.hostname()            → string
///   enki.system.username()            → string
///   enki.system.homeDir()             → string
///   enki.system.tempDir()             → string
///   enki.system.appDir()              → string  ← مجلد التطبيق
///   enki.system.memory()              → {total, free} (bytes)
///   enki.system.cpuCount()            → number
///   enki.system.env(name)             → string | null
///   enki.system.exit(code?)           → void

class SystemAPI : public INativeAPI {
public:
    std::string_view name() const override { return "system"; }
    bool is_permitted(const std::vector<std::string>& perms) const override;
    void register_functions(IWebViewBackend& backend) override;
};

// ════════════════════════════════════════════════════════════════
// PathAPI — window.enki.path.*
// ════════════════════════════════════════════════════════════════
/// Permission: دائماً متاحة (لا تصل للـ filesystem)
///
/// JS API:
///   enki.path.join(...parts)          → string
///   enki.path.dirname(path)           → string
///   enki.path.basename(path, ext?)    → string
///   enki.path.extname(path)           → string
///   enki.path.isAbsolute(path)        → bool
///   enki.path.resolve(...parts)       → string

class PathAPI : public INativeAPI {
public:
    std::string_view name() const override { return "path"; }
    bool is_permitted(const std::vector<std::string>&) const override { return true; }
    void register_functions(IWebViewBackend& backend) override;
};

} // namespace enki::web
