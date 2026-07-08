#pragma once
/// @file i18n.hpp
/// @brief Master Internationalization (i18n) Engine and Declarative Translation API.
///
/// Features:
///   - 100% Declarative configuration with designated initializers (I18n::define).
///   - Ultra-ergonomic inline translation via tr("key") and tr("key", {{"name", "Ali"}}).
///   - Unicode CLDR Pluralization via trPlural("photos_count", count).
///   - Reactive runtime language switching with Signal notifications.
///   - Declarative localizedText() widget with automatic locale re-binding.
///   - RTL layout auto-detection.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/core/signal.hpp"
#include "enki/i18n/locale.hpp"
#include "enki/i18n/plural_rules.hpp"
#include "enki/widgets/text.hpp"
#include <string>
#include <string_view>
#include <unordered_map>
#include <memory>
#include <vector>

namespace enki {

/// Arguments map for dynamic text parameter interpolation: {{"name", "Ali"}, {"count", "5"}}
using TranslationArgs = std::unordered_map<std::string, std::string>;

/// @brief Declarative master configuration structure for I18n.
struct I18nConfig {
    Locale default_locale  = Locale("en", "US");
    Locale fallback_locale = Locale("en", "US");

    /// Multi-language string table: [lang_code -> [key -> localized_text]]
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> translations;

    /// Multi-language plural table: [lang_code -> [key -> PluralForms]]
    std::unordered_map<std::string, std::unordered_map<std::string, PluralForms>> plurals;
};

/// @brief Central Internationalization & Translation Manager.
class I18n {
public:
    /// Declaratively initialize or bulk-register the i18n configuration.
    static void define(const I18nConfig& config);

    /// Add or update a translation map for a specific language code (e.g. "ar", "en").
    static void addTranslations(std::string_view lang,
                                const std::unordered_map<std::string, std::string>& strings);

    /// Add a declarative plural form definition for a specific language and key.
    static void addPlural(std::string_view lang,
                          std::string_view key,
                          const PluralForms& forms);

    /// Set the active locale (triggers onLocaleChanged signal).
    static void setLocale(const Locale& locale);

    /// Convenience overload to set locale by language code (e.g. "ar", "en", "ja").
    static void setLocale(std::string_view lang);

    /// Get the current active locale.
    [[nodiscard]] static const Locale& currentLocale();

    /// Get the fallback locale used when a key is missing in the current locale.
    [[nodiscard]] static const Locale& fallbackLocale();

    /// Set the fallback locale.
    static void setFallbackLocale(const Locale& locale);

    /// Check if current locale has Right-to-Left (RTL) directionality.
    [[nodiscard]] static bool isRTL();

    /// Signal emitted immediately when active locale changes.
    static Signal<const Locale&>& onLocaleChanged();

    /// Translate a key into the active locale, with optional parameter interpolation.
    [[nodiscard]] static std::string translate(std::string_view key,
                                               const TranslationArgs& args = {});

    /// Translate a pluralized key based on count and active locale rules.
    [[nodiscard]] static std::string translatePlural(std::string_view key,
                                                     int64_t count,
                                                     const TranslationArgs& args = {});

    /// Clear all loaded translation stores and reset to default.
    static void reset();
};

// ════════════════════════════════════════════════════════════════
// Ergonomic Free Functions
// ════════════════════════════════════════════════════════════════

/// @brief Translate a key into the current active locale.
///
/// Usage:
/// @code
///   text(tr("photos"));
///   text(tr("welcome_user", {{"name", user.name}}));
/// @endcode
inline std::string tr(std::string_view key, const TranslationArgs& args = {}) {
    return I18n::translate(key, args);
}

/// @brief Translate a plural key based on count into the active locale.
///
/// Usage:
/// @code
///   text(trPlural("photos_count", total_photos));
/// @endcode
inline std::string trPlural(std::string_view key, int64_t count, const TranslationArgs& args = {}) {
    return I18n::translatePlural(key, count, args);
}

// ════════════════════════════════════════════════════════════════
// Declarative LocalizedText Widget
// ════════════════════════════════════════════════════════════════

/// @brief A declarative Text widget that automatically subscribes to locale changes.
/// When I18n::setLocale() is called, this widget automatically re-evaluates its translation.
class LocalizedTextWidget : public StatefulWidget {
public:
    std::string key;
    TextStyle style;
    TranslationArgs args;

    LocalizedTextWidget(std::string_view translation_key,
                        TextStyle text_style = {},
                        TranslationArgs translation_args = {})
        : key(translation_key), style(std::move(text_style)), args(std::move(translation_args)) {}

    std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "LocalizedTextWidget"; }
};

/// @brief Declarative factory for a self-updating localized text widget.
///
/// Usage:
/// @code
///   localizedText("app_title", TextStyle{ .color = Palette::text_white, .font_size = 18.0f });
/// @endcode
inline WidgetPtr localizedText(std::string_view key,
                              const TextStyle& style = {},
                              const TranslationArgs& args = {}) {
    return std::make_shared<LocalizedTextWidget>(key, style, args);
}

} // namespace enki
