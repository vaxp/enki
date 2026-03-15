/// @file string_utils.cpp
/// @brief Implementation of string utilities and interning.

#include "enki/core/string_utils.hpp"
#include <algorithm>
#include <cctype>
#include <cstring>
#include <cstdio>
#include <filesystem>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#elif defined(__linux__)
#include <unistd.h>
#endif


namespace enki {

// ============================================================
// StringInterner
// ============================================================

StringInterner& StringInterner::instance() {
    static StringInterner s_instance;
    return s_instance;
}

std::string_view StringInterner::intern(std::string_view str) {
    std::lock_guard lock(mutex_);
    auto [it, _] = strings_.emplace(str);
    return *it;
}

size_t StringInterner::size() const {
    std::lock_guard lock(mutex_);
    return strings_.size();
}

void StringInterner::clear() {
    std::lock_guard lock(mutex_);
    strings_.clear();
}

// ============================================================
// String Formatting
// ============================================================

std::string formatString(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    auto result = vformatString(fmt, args);
    va_end(args);
    return result;
}

std::string vformatString(const char* fmt, va_list args) {
    // First pass: determine required size
    va_list args_copy;
    va_copy(args_copy, args);
    int size = std::vsnprintf(nullptr, 0, fmt, args_copy);
    va_end(args_copy);

    if (size < 0) return "";

    std::string result(static_cast<size_t>(size), '\0');
    std::vsnprintf(result.data(), result.size() + 1, fmt, args);
    return result;
}

// ============================================================
// String Utilities
// ============================================================

std::string_view trim(std::string_view str) {
    size_t start = 0;
    while (start < str.size() && std::isspace(static_cast<unsigned char>(str[start]))) {
        ++start;
    }
    size_t end = str.size();
    while (end > start && std::isspace(static_cast<unsigned char>(str[end - 1]))) {
        --end;
    }
    return str.substr(start, end - start);
}

bool startsWith(std::string_view str, std::string_view prefix) {
    if (prefix.size() > str.size()) return false;
    return str.substr(0, prefix.size()) == prefix;
}

bool endsWith(std::string_view str, std::string_view suffix) {
    if (suffix.size() > str.size()) return false;
    return str.substr(str.size() - suffix.size()) == suffix;
}

std::string toLower(std::string_view str) {
    std::string result(str);
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

std::string toUpper(std::string_view str) {
    std::string result(str);
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return result;
}

// ============================================================
// Asset Path Resolution
// ============================================================

std::string resolveAssetPath(std::string_view relative_path) {
    if (relative_path.empty()) return {};

    std::filesystem::path p(relative_path);
    std::error_code ec;

    // 1. Direct match (current working directory or absolute path)
    if (std::filesystem::exists(p, ec)) {
        auto canon = std::filesystem::canonical(p, ec);
        return ec ? p.string() : canon.string();
    }

    // 2. Executable location hierarchy
    std::filesystem::path exe_dir;
#if defined(_WIN32)
    wchar_t buf[MAX_PATH] = {0};
    if (GetModuleFileNameW(nullptr, buf, MAX_PATH)) {
        exe_dir = std::filesystem::path(buf).parent_path();
    }
#elif defined(__linux__)
    char buf[1024] = {0};
    ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (len > 0) {
        exe_dir = std::filesystem::path(std::string(buf, len)).parent_path();
    }
#endif

    if (!exe_dir.empty()) {
        std::filesystem::path cur = exe_dir;
        for (int i = 0; i < 6; ++i) {
            std::filesystem::path candidate = cur / p;
            if (std::filesystem::exists(candidate, ec)) {
                auto canon = std::filesystem::canonical(candidate, ec);
                return ec ? candidate.string() : canon.string();
            }
            if (!cur.has_parent_path() || cur == cur.parent_path()) break;
            cur = cur.parent_path();
        }
    }

    // 3. Fallback: traverse up from current working directory
    std::filesystem::path cur_cwd = std::filesystem::current_path(ec);
    if (!ec) {
        for (int i = 0; i < 5; ++i) {
            std::filesystem::path candidate = cur_cwd / p;
            if (std::filesystem::exists(candidate, ec)) {
                auto canon = std::filesystem::canonical(candidate, ec);
                return ec ? candidate.string() : canon.string();
            }
            if (!cur_cwd.has_parent_path() || cur_cwd == cur_cwd.parent_path()) break;
            cur_cwd = cur_cwd.parent_path();
        }
    }

    return std::string(relative_path);
}

}  // namespace enki

