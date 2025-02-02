/// @file string_utils.cpp
/// @brief Implementation of string utilities and interning.

#include "enki/core/string_utils.hpp"
#include <algorithm>
#include <cctype>
#include <cstring>
#include <cstdio>

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

}  // namespace enki
