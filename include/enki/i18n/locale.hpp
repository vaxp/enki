#pragma once
/// @file locale.hpp
/// @brief Locale identification and text directionality for ENKI Framework.
///
/// Represents an IETF BCP 47 language tag (language, country, script) and
/// determines right-to-left (RTL) directionality and native OS language querying.
///
/// @copyright ENKI Framework — MIT License

#include <string>
#include <string_view>
#include <functional>
#include <algorithm>

namespace enki {

/// @brief Represents a standardized Locale identifier.
struct Locale {
    std::string language; ///< ISO 639-1 language code (e.g. "ar", "en", "ja", "fr").
    std::string country;  ///< ISO 3166-1 alpha-2 region code (e.g. "IQ", "US", "EG", "JP").
    std::string script;   ///< ISO 15924 script code (optional, e.g. "Arab", "Latn", "Hans").

    constexpr Locale() : language("en"), country("US") {}

    Locale(std::string_view lang, std::string_view ctry = "", std::string_view scr = "")
        : language(lang), country(ctry), script(scr) {}

    /// Parse from BCP-47 or POSIX tag (e.g. "ar-IQ", "ar_IQ.UTF-8", "en_US", "ja").
    static Locale fromTag(std::string_view tag);

    /// Query native operating system default UI locale (Windows, Linux, Android).
    static Locale system();

    /// Check if this locale requires Right-to-Left (RTL) layout & text shaping.
    [[nodiscard]] bool isRTL() const {
        return language == "ar"  // Arabic
            || language == "fa"  // Persian / Farsi
            || language == "ur"  // Urdu
            || language == "he"  // Hebrew
            || language == "iw"  // Hebrew (legacy code)
            || language == "ps"  // Pashto
            || language == "sd"  // Sindhi
            || language == "ug"  // Uyghur
            || language == "ckb" // Central Kurdish (Sorani)
            || language == "yi"; // Yiddish
    }

    /// Standard BCP-47 tag representation (e.g. "ar-IQ", "en-US", "fr").
    [[nodiscard]] std::string tag() const {
        if (country.empty()) return language;
        return language + "-" + country;
    }

    /// POSIX locale representation (e.g. "ar_IQ", "en_US", "fr").
    [[nodiscard]] std::string to_string() const {
        if (country.empty()) return language;
        return language + "_" + country;
    }

    bool operator==(const Locale& other) const = default;
};

} // namespace enki

namespace std {
template <>
struct hash<enki::Locale> {
    size_t operator()(const enki::Locale& loc) const noexcept {
        size_t h1 = hash<string>{}(loc.language);
        size_t h2 = hash<string>{}(loc.country);
        return h1 ^ (h2 << 1);
    }
};
} // namespace std
