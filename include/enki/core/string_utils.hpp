#pragma once
/// @file string_utils.hpp
/// @brief String utilities and interned string support.

#include <string>
#include <string_view>
#include <unordered_set>
#include <mutex>
#include <cstdarg>
#include <memory>

namespace enki {

// ============================================================
// String Interning
// ============================================================
/// An interning table that ensures each unique string is stored
/// only once in memory. Useful for widget type names and property keys.
class StringInterner {
public:
    /// Get the singleton interner.
    static StringInterner& instance();

    /// Intern a string — returns a persistent string_view that is
    /// valid for the lifetime of the interner (i.e., the program).
    std::string_view intern(std::string_view str);

    /// @return Number of interned strings.
    [[nodiscard]] size_t size() const;

    /// Clear all interned strings.
    void clear();

private:
    StringInterner() = default;
    mutable std::mutex mutex_;
    std::unordered_set<std::string> strings_;
};

// ============================================================
// String Formatting
// ============================================================

/// Format a string using printf-style formatting.
/// @return The formatted string.
std::string formatString(const char* fmt, ...);

/// Format a string using printf-style formatting (va_list version).
std::string vformatString(const char* fmt, va_list args);

// ============================================================
// String Utilities
// ============================================================

/// Trim whitespace from both ends of a string_view.
std::string_view trim(std::string_view str);

/// Check if a string starts with a prefix.
bool startsWith(std::string_view str, std::string_view prefix);

/// Check if a string ends with a suffix.
bool endsWith(std::string_view str, std::string_view suffix);

/// Convert a string to lowercase (ASCII only).
std::string toLower(std::string_view str);

/// Convert a string to uppercase (ASCII only).
std::string toUpper(std::string_view str);

}  // namespace enki
