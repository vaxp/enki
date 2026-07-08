/// @file i18n.cpp
/// @brief Master Internationalization (i18n) implementation.

#include "enki/i18n/i18n.hpp"
#include "enki/state/state.hpp"
#include <mutex>

namespace enki {

namespace {

struct I18nStore {
    std::mutex mutex;
    Locale current_locale{"en", "US"};
    Locale fallback_locale{"en", "US"};

    // [language/tag -> [key -> translation]]
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> translations;

    // [language/tag -> [key -> PluralForms]]
    std::unordered_map<std::string, std::unordered_map<std::string, PluralForms>> plurals;

    Signal<const Locale&> on_locale_changed;
};

I18nStore& store() {
    static I18nStore s_store;
    return s_store;
}

std::string interpolate(std::string_view text, const TranslationArgs& args) {
    if (args.empty() || text.empty()) {
        return std::string(text);
    }
    std::string result;
    result.reserve(text.size() + 32);
    size_t i = 0;
    while (i < text.size()) {
        if (text[i] == '{') {
            auto close_pos = text.find('}', i + 1);
            if (close_pos != std::string_view::npos) {
                std::string placeholder(text.substr(i + 1, close_pos - i - 1));
                auto it = args.find(placeholder);
                if (it != args.end()) {
                    result.append(it->second);
                } else {
                    result.append(text.substr(i, close_pos - i + 1));
                }
                i = close_pos + 1;
                continue;
            }
        }
        result.push_back(text[i]);
        ++i;
    }
    return result;
}

} // anonymous namespace

void I18n::define(const I18nConfig& config) {
    auto& s = store();
    std::lock_guard lock(s.mutex);

    s.current_locale  = config.default_locale;
    s.fallback_locale = config.fallback_locale;

    for (const auto& [lang, map] : config.translations) {
        auto& target = s.translations[lang];
        for (const auto& [k, v] : map) {
            target[k] = v;
        }
    }

    for (const auto& [lang, map] : config.plurals) {
        auto& target = s.plurals[lang];
        for (const auto& [k, v] : map) {
            target[k] = v;
        }
    }

    s.on_locale_changed.emit(s.current_locale);
}

void I18n::addTranslations(std::string_view lang,
                           const std::unordered_map<std::string, std::string>& strings) {
    auto& s = store();
    std::lock_guard lock(s.mutex);
    auto& target = s.translations[std::string(lang)];
    for (const auto& [k, v] : strings) {
        target[k] = v;
    }
}

void I18n::addPlural(std::string_view lang,
                     std::string_view key,
                     const PluralForms& forms) {
    auto& s = store();
    std::lock_guard lock(s.mutex);
    s.plurals[std::string(lang)][std::string(key)] = forms;
}

void I18n::setLocale(const Locale& locale) {
    auto& s = store();
    {
        std::lock_guard lock(s.mutex);
        if (s.current_locale == locale) {
            return;
        }
        s.current_locale = locale;
    }
    s.on_locale_changed.emit(locale);
}

void I18n::setLocale(std::string_view lang) {
    setLocale(Locale::fromTag(lang));
}

const Locale& I18n::currentLocale() {
    auto& s = store();
    std::lock_guard lock(s.mutex);
    return s.current_locale;
}

const Locale& I18n::fallbackLocale() {
    auto& s = store();
    std::lock_guard lock(s.mutex);
    return s.fallback_locale;
}

void I18n::setFallbackLocale(const Locale& locale) {
    auto& s = store();
    std::lock_guard lock(s.mutex);
    s.fallback_locale = locale;
}

bool I18n::isRTL() {
    return currentLocale().isRTL();
}

Signal<const Locale&>& I18n::onLocaleChanged() {
    return store().on_locale_changed;
}

std::string I18n::translate(std::string_view key, const TranslationArgs& args) {
    auto& s = store();
    std::lock_guard lock(s.mutex);

    std::string k(key);
    std::string current_tag  = s.current_locale.tag();
    std::string current_lang = s.current_locale.language;
    std::string fallback_tag = s.fallback_locale.tag();
    std::string fallback_lang= s.fallback_locale.language;

    // 1. Check current locale full tag (e.g. "ar-IQ")
    auto it_lang = s.translations.find(current_tag);
    if (it_lang != s.translations.end()) {
        auto it_str = it_lang->second.find(k);
        if (it_str != it_lang->second.end()) {
            return interpolate(it_str->second, args);
        }
    }

    // 2. Check current locale language only (e.g. "ar")
    if (current_lang != current_tag) {
        it_lang = s.translations.find(current_lang);
        if (it_lang != s.translations.end()) {
            auto it_str = it_lang->second.find(k);
            if (it_str != it_lang->second.end()) {
                return interpolate(it_str->second, args);
            }
        }
    }

    // 3. Check fallback locale full tag (e.g. "en-US")
    it_lang = s.translations.find(fallback_tag);
    if (it_lang != s.translations.end()) {
        auto it_str = it_lang->second.find(k);
        if (it_str != it_lang->second.end()) {
            return interpolate(it_str->second, args);
        }
    }

    // 4. Check fallback locale language only (e.g. "en")
    if (fallback_lang != fallback_tag) {
        it_lang = s.translations.find(fallback_lang);
        if (it_lang != s.translations.end()) {
            auto it_str = it_lang->second.find(k);
            if (it_str != it_lang->second.end()) {
                return interpolate(it_str->second, args);
            }
        }
    }

    // 5. Fallback: return raw key interpolated with args
    return interpolate(key, args);
}

std::string I18n::translatePlural(std::string_view key,
                                 int64_t count,
                                 const TranslationArgs& user_args) {
    auto& s = store();
    std::lock_guard lock(s.mutex);

    std::string k(key);
    std::string current_tag  = s.current_locale.tag();
    std::string current_lang = s.current_locale.language;
    std::string fallback_tag = s.fallback_locale.tag();
    std::string fallback_lang= s.fallback_locale.language;

    PluralCategory category = resolvePluralCategory(current_lang, count);

    // Merge count into arguments
    TranslationArgs args = user_args;
    if (args.find("count") == args.end()) {
        args["count"] = std::to_string(count);
    }

    // 1. Check current full tag plurals
    auto it_pl = s.plurals.find(current_tag);
    if (it_pl != s.plurals.end()) {
        auto it_f = it_pl->second.find(k);
        if (it_f != it_pl->second.end()) {
            return interpolate(it_f->second.get(category), args);
        }
    }

    // 2. Check current language plurals
    if (current_lang != current_tag) {
        it_pl = s.plurals.find(current_lang);
        if (it_pl != s.plurals.end()) {
            auto it_f = it_pl->second.find(k);
            if (it_f != it_pl->second.end()) {
                return interpolate(it_f->second.get(category), args);
            }
        }
    }

    // 3. Fallback locale plurals
    it_pl = s.plurals.find(fallback_tag);
    if (it_pl != s.plurals.end()) {
        auto it_f = it_pl->second.find(k);
        if (it_f != it_pl->second.end()) {
            PluralCategory fallback_cat = resolvePluralCategory(fallback_lang, count);
            return interpolate(it_f->second.get(fallback_cat), args);
        }
    }

    if (fallback_lang != fallback_tag) {
        it_pl = s.plurals.find(fallback_lang);
        if (it_pl != s.plurals.end()) {
            auto it_f = it_pl->second.find(k);
            if (it_f != it_pl->second.end()) {
                PluralCategory fallback_cat = resolvePluralCategory(fallback_lang, count);
                return interpolate(it_f->second.get(fallback_cat), args);
            }
        }
    }

    // 4. Default: attempt regular translate
    return interpolate(k + " (" + std::to_string(count) + ")", args);
}

void I18n::reset() {
    auto& s = store();
    std::lock_guard lock(s.mutex);
    s.translations.clear();
    s.plurals.clear();
    s.current_locale  = Locale("en", "US");
    s.fallback_locale = Locale("en", "US");
}

// ════════════════════════════════════════════════════════════════
// LocalizedTextWidget Implementation
// ════════════════════════════════════════════════════════════════

class LocalizedTextState : public State {
public:
    void initState() override {
        State::initState();
        conn_id_ = I18n::onLocaleChanged().connect([this](const Locale&) {
            setState([] {});
        });
    }

    void dispose() override {
        if (conn_id_ != InvalidSlotId) {
            I18n::onLocaleChanged().disconnect(conn_id_);
            conn_id_ = InvalidSlotId;
        }
        State::dispose();
    }

    WidgetPtr build(BuildContext&) override {
        const auto* w = static_cast<const LocalizedTextWidget*>(widget());
        if (!w) return sizedBox(0, 0);
        return text(tr(w->key, w->args), w->style);
    }

private:
    SlotId conn_id_ = InvalidSlotId;
};

std::unique_ptr<State> LocalizedTextWidget::createState() {
    return std::make_unique<LocalizedTextState>();
}

} // namespace enki
