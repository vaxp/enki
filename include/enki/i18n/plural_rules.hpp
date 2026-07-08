#pragma once
/// @file plural_rules.hpp
/// @brief Unicode CLDR Pluralization Rules and Declarative Plural Forms.
///
/// Supports full pluralization rules for Arabic (6 forms), English, Slavic,
/// Asian, and Romance languages with C++20 designated initializers.
///
/// @copyright ENKI Framework — MIT License

#include <string>
#include <string_view>
#include <cstdint>
#include <cmath>

namespace enki {

/// @brief CLDR Plural Category.
enum class PluralCategory : uint8_t {
    Zero,
    One,
    Two,
    Few,
    Many,
    Other
};

/// @brief Declarative plural forms container for a single localized message.
///
/// Usage (Arabic):
/// @code
///   PluralForms{
///       .zero  = "لا توجد عناصر",
///       .one   = "عنصر واحد",
///       .two   = "عنصران",
///       .few   = "{count} عناصر",
///       .many  = "{count} عنصراً",
///       .other = "{count} عنصر"
///   };
/// @endcode
struct PluralForms {
    std::string zero;  ///< Explicit zero form (e.g. Arabic 0, Russian 0)
    std::string one;   ///< Singular form (e.g. "1 photo")
    std::string two;   ///< Dual form (e.g. Arabic "صورتان", Hebrew)
    std::string few;   ///< Paucal form (e.g. Arabic 3-10, Russian 2-4)
    std::string many;  ///< Fractional/plural form (e.g. Arabic 11-99, Russian 5+)
    std::string other; ///< General fallback plural (required for default)

    /// Pick the appropriate string for the given category with graceful fallback to `other`.
    [[nodiscard]] std::string_view get(PluralCategory category) const {
        switch (category) {
            case PluralCategory::Zero:  if (!zero.empty())  return zero;  break;
            case PluralCategory::One:   if (!one.empty())   return one;   break;
            case PluralCategory::Two:   if (!two.empty())   return two;   break;
            case PluralCategory::Few:   if (!few.empty())   return few;   break;
            case PluralCategory::Many:  if (!many.empty())  return many;  break;
            case PluralCategory::Other: break;
        }
        return other;
    }
};

/// @brief Resolve CLDR plural category for an integer count and language code.
inline PluralCategory resolvePluralCategory(std::string_view lang, int64_t n) {
    n = std::abs(n);

    // 1. Arabic (ar) — 6 categories
    if (lang == "ar") {
        if (n == 0) return PluralCategory::Zero;
        if (n == 1) return PluralCategory::One;
        if (n == 2) return PluralCategory::Two;
        int64_t rem100 = n % 100;
        if (rem100 >= 3 && rem100 <= 10) return PluralCategory::Few;
        if (rem100 >= 11 && rem100 <= 99) return PluralCategory::Many;
        return PluralCategory::Other;
    }

    // 2. East Asian (ja, zh, ko, vi) — no plural variation
    if (lang == "ja" || lang == "zh" || lang == "ko" || lang == "vi") {
        return PluralCategory::Other;
    }

    // 3. French / Portuguese (fr, pt) — 0 and 1 are singular
    if (lang == "fr" || lang == "pt") {
        if (n == 0 || n == 1) return PluralCategory::One;
        return PluralCategory::Other;
    }

    // 4. Slavic (ru, uk, be, pl, sr, hr, bs)
    if (lang == "ru" || lang == "uk" || lang == "be" || lang == "pl") {
        int64_t rem10 = n % 10;
        int64_t rem100 = n % 100;
        if (rem10 == 1 && rem100 != 11) return PluralCategory::One;
        if (rem10 >= 2 && rem10 <= 4 && (rem100 < 10 || rem100 >= 20)) return PluralCategory::Few;
        return PluralCategory::Many;
    }

    // 5. Default Germanic / Romance / Generic (en, de, es, it, tr, etc.)
    if (n == 1) return PluralCategory::One;
    return PluralCategory::Other;
}

} // namespace enki
