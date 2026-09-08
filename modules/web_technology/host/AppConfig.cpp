/// @file AppConfig.cpp
/// @brief تحميل وتحليل ملف enki.json.
///
/// يستخدم مكتبة JSON بسيطة مدمجة (nlohmann/json أو بديل خفيف).
/// لتجنب dependency إضافية نستخدم محلل JSON بسيط مكتوب يدوياً.
///
/// @copyright ENKI Framework — MIT License

#include <web_technology/EnkiWebHost.hpp>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <filesystem>

// ── Simple JSON parser helpers ──────────────────────────────────
// لتجنب dependency خارجية، نستخدم محلل بسيط لـ enki.json.
// في المستقبل يمكن استبداله بـ nlohmann/json.

#include <regex>
#include <map>

namespace fs = std::filesystem;

namespace {

// استخراج قيمة string من JSON بسيط
std::string json_str(const std::string& json, const std::string& key,
                     const std::string& def = "")
{
    std::string pattern = "\"" + key + "\"\\s*:\\s*\"([^\"]+)\"";
    std::regex  rx(pattern);
    std::smatch m;
    if (std::regex_search(json, m, rx)) return m[1].str();
    return def;
}

// استخراج قيمة int
int json_int(const std::string& json, const std::string& key, int def = 0)
{
    std::string pattern = "\"" + key + "\"\\s*:\\s*(-?\\d+)";
    std::regex  rx(pattern);
    std::smatch m;
    if (std::regex_search(json, m, rx)) return std::stoi(m[1].str());
    return def;
}

// استخراج قيمة bool
bool json_bool(const std::string& json, const std::string& key, bool def = false)
{
    std::string pattern = "\"" + key + "\"\\s*:\\s*(true|false)";
    std::regex  rx(pattern);
    std::smatch m;
    if (std::regex_search(json, m, rx)) return m[1].str() == "true";
    return def;
}

// استخراج array of strings
std::vector<std::string> json_str_array(const std::string& json,
                                         const std::string& key)
{
    std::vector<std::string> result;
    // ابحث عن المصفوفة: "key": ["a", "b", ...]
    std::string pat = "\"" + key + "\"\\s*:\\s*\\[([^\\]]+)\\]";
    std::regex  rx(pat);
    std::smatch m;
    if (!std::regex_search(json, m, rx)) return result;

    std::string arr = m[1].str();
    std::regex  item("\"([^\"]+)\"");
    auto begin = std::sregex_iterator(arr.begin(), arr.end(), item);
    auto end   = std::sregex_iterator();
    for (auto it = begin; it != end; ++it) {
        result.push_back((*it)[1].str());
    }
    return result;
}

// استخراج JSON block للـ window object
std::string extract_block(const std::string& json, const std::string& key)
{
    std::string pat = "\"" + key + "\"\\s*:\\s*\\{";
    std::regex  rx(pat);
    std::smatch m;
    if (!std::regex_search(json, m, rx)) return "";

    size_t start = m.position() + m.length() - 1; // موقع القوس المفتوح '{'
    if (json[start] != '{') {
        start = json.find('{', m.position());
    }
    if (start == std::string::npos) return "";

    int depth = 0;
    for (size_t i = start; i < json.size(); ++i) {
        if (json[i] == '{') ++depth;
        else if (json[i] == '}') {
            --depth;
            if (depth == 0) return json.substr(start, i - start + 1);
        }
    }
    return "";
}

} // anonymous namespace

namespace enki::web {

// ── AppConfig::from_file ───────────────────────────────────────

AppConfig AppConfig::from_file(const std::string& path)
{
    std::ifstream f(path);
    if (!f.is_open()) {
        throw std::runtime_error("Cannot open config file: " + path);
    }

    std::ostringstream ss;
    ss << f.rdbuf();
    std::string json = ss.str();

    AppConfig cfg;
    cfg.app_root = fs::path(path).parent_path().string();

    cfg.name    = json_str(json, "name",    "App");
    cfg.version = json_str(json, "version", "0.1.0");
    cfg.entry   = json_str(json, "entry",   "src/index.html");

    cfg.devtools        = json_bool(json, "devtools",        false);
    cfg.single_instance = json_bool(json, "single_instance", true);
    cfg.hardware_accel  = json_bool(json, "hardware_accel",  true);
    cfg.device_scale    = 1.0f;

    cfg.permissions = json_str_array(json, "permissions");

    // ── Window config ──────────────────────────────────────────
    std::string win = extract_block(json, "window");
    if (!win.empty()) {
        cfg.window.title       = json_str (win, "title",      cfg.name);
        cfg.window.width       = json_int (win, "width",      1280);
        cfg.window.height      = json_int (win, "height",     800);
        cfg.window.min_width   = json_int (win, "min_width",  400);
        cfg.window.min_height  = json_int (win, "min_height", 300);
        cfg.window.resizable   = json_bool(win, "resizable",  true);
        cfg.window.frameless   = json_bool(win, "frameless",  false);
        cfg.window.transparent = json_bool(win, "transparent",false);
        cfg.window.center      = json_bool(win, "center",     true);
        cfg.window.maximized   = json_bool(win, "maximized",  false);
        cfg.window.fullscreen  = json_bool(win, "fullscreen", false);
    }

    return cfg;
}

// ── AppConfig::entry_url ──────────────────────────────────────

std::string AppConfig::entry_url() const
{
    if (entry.rfind("http://",  0) == 0) return entry;
    if (entry.rfind("https://", 0) == 0) return entry;
    if (entry.rfind("file://",  0) == 0) return entry;

    // مسار نسبي → file:// URL
    fs::path full = fs::path(app_root) / entry;
    return "file://" + fs::absolute(full).string();
}

} // namespace enki::web
