/// @file test_i18n.cpp
/// @brief Unit tests for Locale, PluralRules, and I18n translation system.

#include "enki/i18n/locale.hpp"
#include "enki/i18n/plural_rules.hpp"
#include "enki/i18n/i18n.hpp"
#include <cassert>
#include <iostream>

using namespace enki;

void test_locale_parsing() {
    Locale loc1 = Locale::fromTag("ar-IQ");
    assert(loc1.language == "ar");
    assert(loc1.country == "IQ");
    assert(loc1.isRTL() == true);
    assert(loc1.tag() == "ar-IQ");
    assert(loc1.to_string() == "ar_IQ");

    Locale loc2 = Locale::fromTag("en_US.UTF-8");
    assert(loc2.language == "en");
    assert(loc2.country == "US");
    assert(loc2.isRTL() == false);

    Locale loc3 = Locale::fromTag("fa");
    assert(loc3.language == "fa");
    assert(loc3.isRTL() == true);

    Locale loc4 = Locale::system();
    assert(!loc4.language.empty());

    std::cout << "  [PASS] test_locale_parsing (System locale detected: " << loc4.tag() << ")\n";
}

void test_plural_rules() {
    // Arabic plural rules: 0=Zero, 1=One, 2=Two, 3-10=Few, 11-99=Many, 100+=Other
    assert(resolvePluralCategory("ar", 0) == PluralCategory::Zero);
    assert(resolvePluralCategory("ar", 1) == PluralCategory::One);
    assert(resolvePluralCategory("ar", 2) == PluralCategory::Two);
    assert(resolvePluralCategory("ar", 5) == PluralCategory::Few);
    assert(resolvePluralCategory("ar", 10) == PluralCategory::Few);
    assert(resolvePluralCategory("ar", 11) == PluralCategory::Many);
    assert(resolvePluralCategory("ar", 99) == PluralCategory::Many);
    assert(resolvePluralCategory("ar", 100) == PluralCategory::Other);
    assert(resolvePluralCategory("ar", 102) == PluralCategory::Other);

    // English: 1=One, else Other
    assert(resolvePluralCategory("en", 1) == PluralCategory::One);
    assert(resolvePluralCategory("en", 0) == PluralCategory::Other);
    assert(resolvePluralCategory("en", 2) == PluralCategory::Other);
    assert(resolvePluralCategory("en", 10) == PluralCategory::Other);

    std::cout << "  [PASS] test_plural_rules\n";
}

void test_declarative_i18n() {
    I18n::reset();

    // Declarative master configuration with designated initializers
    I18n::define(I18nConfig{
        .default_locale = Locale("en", "US"),
        .fallback_locale = Locale("en", "US"),
        .translations = {
            { "en", {
                { "app_title", "ENKI Photo Gallery" },
                { "photos",    "Photos" },
                { "favorites", "Favorites" },
                { "welcome",   "Welcome back, {name}!" },
                { "only_in_en", "English Only Key" },
            }},
            { "ar", {
                { "app_title", "معرض صور إنكي" },
                { "photos",    "الصور" },
                { "favorites", "المفضلة" },
                { "welcome",   "مرحباً بعودتك يا {name}!" },
            }}
        },
        .plurals = {
            { "en", {
                { "photo_count", PluralForms{
                    .one   = "1 photo",
                    .other = "{count} photos",
                }}
            }},
            { "ar", {
                { "photo_count", PluralForms{
                    .zero  = "لا توجد صور",
                    .one   = "صورة واحدة",
                    .two   = "صورتان",
                    .few   = "{count} صور",
                    .many  = "{count} صورة",
                    .other = "{count} صورة",
                }}
            }}
        }
    });

    // 1. Check English (default)
    assert(tr("app_title") == "ENKI Photo Gallery");
    assert(tr("photos") == "Photos");
    assert(tr("welcome", {{"name", "Ahmed"}}) == "Welcome back, Ahmed!");
    assert(trPlural("photo_count", 1) == "1 photo");
    assert(trPlural("photo_count", 5) == "5 photos");
    assert(I18n::isRTL() == false);

    // 2. Switch to Arabic dynamically
    bool signal_fired = false;
    auto conn = I18n::onLocaleChanged().connect([&signal_fired](const Locale& loc) {
        assert(loc.language == "ar");
        signal_fired = true;
    });

    I18n::setLocale("ar");
    assert(signal_fired == true);
    assert(I18n::currentLocale().language == "ar");
    assert(I18n::isRTL() == true);

    // 3. Verify Arabic translations
    assert(tr("app_title") == "معرض صور إنكي");
    assert(tr("photos") == "الصور");
    assert(tr("welcome", {{"name", "علي"}}) == "مرحباً بعودتك يا علي!");

    // 4. Verify Arabic Plurals
    assert(trPlural("photo_count", 0) == "لا توجد صور");
    assert(trPlural("photo_count", 1) == "صورة واحدة");
    assert(trPlural("photo_count", 2) == "صورتان");
    assert(trPlural("photo_count", 5) == "5 صور");
    assert(trPlural("photo_count", 25) == "25 صورة");
    assert(trPlural("photo_count", 100) == "100 صورة");

    // 5. Verify Fallback to English when key missing in Arabic
    assert(tr("only_in_en") == "English Only Key");

    I18n::onLocaleChanged().disconnect(conn);
    std::cout << "  [PASS] test_declarative_i18n\n";
}

int main() {
    std::cout << "========================================\n";
    std::cout << " Running ENKI i18n & Locale Unit Tests\n";
    std::cout << "========================================\n";

    test_locale_parsing();
    test_plural_rules();
    test_declarative_i18n();

    std::cout << "========================================\n";
    std::cout << " ALL i18n & Locale Unit Tests Passed!\n";
    std::cout << "========================================\n";
    return 0;
}
