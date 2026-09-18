#pragma once
/// @file env.hpp
/// @brief Environment queries, path discovery, and filesystem utilities.

#include <string>
#include <vector>
#include <filesystem>
#include <cstdlib>
#include <optional>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace fs = std::filesystem;

namespace enki::cli {

class Env {
public:
    static std::string get(const std::string& var_name) {
#if defined(_WIN32)
        char* val = nullptr;
        size_t len = 0;
        if (_dupenv_s(&val, &len, var_name.c_str()) == 0 && val != nullptr) {
            std::string res(val);
            free(val);
            return res;
        }
        return "";
#else
        const char* val = std::getenv(var_name.c_str());
        return val ? std::string(val) : "";
#endif
    }

    static void set(const std::string& key, const std::string& val) {
#if defined(_WIN32)
        _putenv_s(key.c_str(), val.c_str());
#else
        setenv(key.c_str(), val.c_str(), 1);
#endif
    }

    static std::string pathToUtf8(const fs::path& p) {
#if defined(_WIN32)
        const std::wstring& wstr = p.native();
        if (wstr.empty()) return "";
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), static_cast<int>(wstr.size()), NULL, 0, NULL, NULL);
        if (size_needed <= 0) return "";
        std::string str(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, wstr.data(), static_cast<int>(wstr.size()), &str[0], size_needed, NULL, NULL);
        return str;
#else
        return p.string();
#endif
    }

    static fs::path stringToPath(const std::string& s) {
#if defined(_WIN32)
        if (s.empty()) return fs::path();
        int wlen = MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), NULL, 0);
        if (wlen <= 0) return fs::path(s);
        std::wstring wstr(wlen, 0);
        MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), &wstr[0], wlen);
        return fs::path(wstr);
#else
        return fs::path(s);
#endif
    }

    /// @brief Search for executable in PATH
    static std::optional<fs::path> which(const std::string& binary_name) {
        std::string path_env = get("PATH");
        if (path_env.empty()) return std::nullopt;

#if defined(_WIN32)
        char delimiter = ';';
        std::vector<std::string> exts = { "", ".exe", ".cmd", ".bat" };
#else
        char delimiter = ':';
        std::vector<std::string> exts = { "" };
#endif

        size_t start = 0;
        size_t end = path_env.find(delimiter);
        while (start < path_env.size()) {
            std::string dir = path_env.substr(start, end - start);
            if (!dir.empty()) {
                for (const auto& ext : exts) {
                    std::error_code ec;
                    fs::path candidate = stringToPath(dir) / (binary_name + ext);
                    if (fs::exists(candidate, ec) && !fs::is_directory(candidate, ec)) {
                        return candidate;
                    }
                }
            }
            if (end == std::string::npos) break;
            start = end + 1;
            end = path_env.find(delimiter, start);
        }
        return std::nullopt;
    }

    static std::optional<fs::path> findNinja() {
        auto in_path = which("ninja");
        if (in_path) return in_path;
#if defined(_WIN32)
        std::vector<std::string> candidates = {
            "C:\\msys64\\ucrt64\\bin\\ninja.exe",
            "C:\\Program Files (x86)\\Microsoft Visual Studio\\2022\\BuildTools\\Common7\\IDE\\CommonExtensions\\Microsoft\\CMake\\Ninja\\ninja.exe",
            "C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\Common7\\IDE\\CommonExtensions\\Microsoft\\CMake\\Ninja\\ninja.exe",
            "C:\\Program Files\\Microsoft Visual Studio\\2022\\Professional\\Common7\\IDE\\CommonExtensions\\Microsoft\\CMake\\Ninja\\ninja.exe",
            "C:\\Program Files\\Microsoft Visual Studio\\2022\\Enterprise\\Common7\\IDE\\CommonExtensions\\Microsoft\\CMake\\Ninja\\ninja.exe",
            "C:\\msys64\\mingw64\\bin\\ninja.exe"
        };
        for (const auto& c : candidates) {
            std::error_code ec;
            fs::path p = stringToPath(c);
            if (fs::exists(p, ec)) return p;
        }
#endif
        return std::nullopt;
    }

    static std::optional<fs::path> findMeson() {
        auto in_path = which("meson");
        if (in_path) return in_path;
#if defined(_WIN32)
        std::vector<std::string> candidates = {
            "C:\\msys64\\ucrt64\\bin\\meson.exe",
            "C:\\msys64\\mingw64\\bin\\meson.exe"
        };
        for (const auto& c : candidates) {
            std::error_code ec;
            fs::path p = stringToPath(c);
            if (fs::exists(p, ec)) return p;
        }
#endif
        return std::nullopt;
    }

    /// @brief Find the root of the ENKI repository by searching upwards for root meson.build
    static fs::path findEnkiRepoRoot(fs::path start_dir = fs::current_path()) {
        fs::path cur = start_dir;
        std::error_code ec;
        for (int i = 0; i < 10; ++i) {
            fs::path m = cur / "meson.build";
            fs::path inc = cur / "include" / "enki";
            if (fs::exists(m, ec) && fs::exists(inc, ec)) {
                return cur;
            }
            if (!cur.has_parent_path() || cur.parent_path() == cur) break;
            cur = cur.parent_path();
        }
        return start_dir;
    }
};

} // namespace enki::cli
