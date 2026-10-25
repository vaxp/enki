# Declarative Internationalization & Localization (i18n)

> A complete engineering guide to the ENKI internationalization and localization subsystem. Covers declarative dictionary definitions, Unicode CLDR plural rules (including 6-form Arabic grammar), native OS locale detection, reactive language switching, and real-world C++20 code from `real_app/gallery`.

- **Header Files**:
  - `#include "enki/i18n/locale.hpp"` — `Locale`, `Locale::system()`, `Locale::isRTL()`.
  - `#include "enki/i18n/plural_rules.hpp"` — `PluralCategory`, `PluralForms`, Unicode CLDR rules.
  - `#include "enki/i18n/i18n.hpp"` — `I18n`, `I18nConfig`, `tr(...)`, `trPlural(...)`, `localizedText(...)`.
- **Primary Classes & Structs**:
  - `enki::Locale` — BCP-47 / POSIX parser, script/region normalization, $O(1)$ RTL detection, native OS locale discovery.
  - `enki::I18n` — Centralized, thread-safe localization manager with fallback locale cascade.
  - `enki::I18nConfig` — Declarative C++20 configuration struct for application dictionaries and plural catalogs.
  - `enki::PluralForms` — Designated initializer struct supporting all 6 Unicode CLDR plural forms (`zero`, `one`, `two`, `few`, `many`, `other`).
- **Primary Functions & DSL Helpers**:
  - `enki::tr(key, params = {})` — Look up translated string with named parameter interpolation.
  - `enki::trPlural(key, count, params = {})` — Select grammatically correct plural string based on language rules.
  - `enki::localizedText(key, params = {}, style = {})` — Declarative text widget with automatic translation.
  - `enki::I18n::setLocale(locale)` — Update active locale and notify all active UI listeners.
  - `enki::I18n::onLocaleChanged()` — Global `Signal<const Locale&>` dispatched on locale changes.

---

## 1. Overview & Architectural Goals

Modern cross-platform applications targeting desktop (Windows, Linux X11/Wayland) and mobile (Android) must deliver seamless native experiences across diverse languages and scripts. In C++ GUI development, localization has historically suffered from rigid compile-time templates, opaque macro hacks, or heavy external runtime frameworks.

The **ENKI Localization Subsystem** is designed from the ground up to embody the framework's core philosophies:

1. **Declarative C++20 Syntax**: Translations and plural rules are defined using designated initializers without macro magic or ugly boilerplate.
2. **Grammar-Accurate Unicode CLDR Pluralization**: Many languages have complex plural systems. For example, Arabic features **6 distinct plural forms** (Zero, One, Two, Few [3–10], Many [11–99], and Other). ENKI implements exact Unicode CLDR rules rather than simple English singular/plural toggles.
3. **Decoupled Architecture**: Storage of translations is memory-efficient and decoupled from asset I/O. Dictionaries can be embedded directly in C++ source or loaded dynamically from external storage.
4. **Reactive Signal-Driven Rebuilding**: When the user switches languages, `I18n::onLocaleChanged()` triggers instantaneous subtree reconciliation across the UI without restarting the process or dropping Cubit/BLoC state.
5. **Zero-Overhead Native RTL & Shaping**: Integrates seamlessly with SkParagraph, HarfBuzz, and ICU for automatic Bidirectional (BiDi) text layout and cursive shaping.

```
┌────────────────────────────────────────────────────────────────────────┐
│                        ENKI i18n Architecture                          │
│                                                                        │
│   [Native OS Platform]                                                 │
│   (Win32: GetUserDefaultLocaleName / Linux: LANG / Android: NDK)       │
│                            │                                           │
│                            ▼                                           │
│                 [enki::Locale::system()]                               │
│                            │                                           │
│   [I18n::define(I18nConfig)] ◄── Declarative C++20 Dictionary Setup    │
│                            │                                           │
│                            ▼                                           │
│                 [enki::I18n Singleton]                                 │
│                   ├── Locale Resolution (Target -> Fallback -> Key)    │
│                   ├── Unicode CLDR Plural Rule Engine                  │
│                   └── Parameter Interpolation ({name} -> value)        │
│                            │                                           │
│            ┌───────────────┴───────────────┐                           │
│            ▼                               ▼                           │
│   [tr() / trPlural()]            [I18n::onLocaleChanged()]             │
│   (Synchronous lookup)           (Signal<const Locale&>)               │
│            │                               │                           │
│            ▼                               ▼                           │
│   [Declarative Widgets]          [Stateful Elements / App Root]        │
│   - text(tr("key"))              - setState() triggers instant         │
│   - localizedText("key")           rebuild of visible text tree        │
└────────────────────────────────────────────────────────────────────────┘
```

---

## 2. Core Subsystems

### A. The `Locale` Struct

`enki::Locale` represents a normalized language, country/region, and optional script identifier according to BCP-47 and POSIX standards.

```cpp
#include "enki/i18n/locale.hpp"

// Explicit construction
enki::Locale ar_iq("ar", "IQ"); // Arabic (Iraq)
enki::Locale en_us("en", "US"); // English (United States)

// BCP-47 and POSIX parsing
auto loc1 = enki::Locale::parse("ar-IQ");      // language="ar", country="IQ"
auto loc2 = enki::Locale::parse("en_GB.UTF-8"); // language="en", country="GB"
auto loc3 = enki::Locale::parse("zh-Hant-TW");  // language="zh", script="Hant", country="TW"

// Stringification
std::string tag = ar_iq.toLanguageTag(); // "ar-IQ"
std::string pos = ar_iq.toPosix();       // "ar_IQ"
```

#### Directionality & RTL Detection
`Locale::isRTL()` performs an $O(1)$ lookup for Right-to-Left writing systems (Arabic, Hebrew, Farsi, Urdu, Pashto, Yiddish, Sindhi, Uighur):

```cpp
if (enki::I18n::isRTL()) {
    // Layout directional adjustments (e.g. alignment, navigation arrows)
}
```

#### Native OS Locale Detection
`Locale::system()` interrogates the host operating system at runtime:
- **Windows**: Calls Win32 `GetUserDefaultLocaleName()`.
- **Linux**: Inspects environment variables `LC_ALL`, `LC_MESSAGES`, and `LANG`.
- **Android**: Retrieves system configuration via JNI / Native Activity.

---

### B. Declarative Dictionaries (`I18n::define`)

Translations are registered using `I18n::define(...)` with an `I18nConfig` struct. Dictionaries are organized by language codes, with fallback cascades:

```cpp
#include "enki/i18n/i18n.hpp"

enki::I18n::define(enki::I18nConfig{
    .default_locale = enki::Locale("en", "US"),
    .fallback_locale = enki::Locale("en", "US"),
    .translations = {
        { "en", {
            { "app.title",       "Media Studio" },
            { "action.save",     "Save Changes" },
            { "msg.welcome",     "Welcome back, {user}!" },
        }},
        { "ar", {
            { "app.title",       "استوديو الوسائط" },
            { "action.save",     "حفظ التغييرات" },
            { "msg.welcome",     "مرحباً بك مجدداً، {user}!" },
        }}
    }
});
```

#### Resolution Fallback Cascade
When `tr(key)` is invoked:
1. Search in `currentLocale.toLanguageTag()` (e.g., `ar-IQ`).
2. Search in `currentLocale.language` (e.g., `ar`).
3. Search in `fallbackLocale.toLanguageTag()` (e.g., `en-US`).
4. Search in `fallbackLocale.language` (e.g., `en`).
5. If no translation exists, return the raw `key` string as a failsafe.

---

### C. Unicode CLDR Pluralization (`trPlural`)

In languages like English, pluralization is a binary choice: *one* vs *other* (`1 photo`, `5 photos`). In languages like Arabic, Slavic languages, or Celtic languages, nouns decline across multiple grammatical categories.

ENKI incorporates the official **Unicode CLDR (Common Locale Data Repository)** plural rules.

#### Arabic 6-Category Rules
| Category | Rule | English Analogy | Arabic Example |
|---|---|---|---|
| `zero` | $n = 0$ | "no items" | `لا توجد صور` |
| `one` | $n = 1$ | "one item" | `صورة واحدة` |
| `two` | $n = 2$ | "two items" (Dual) | `صورتان` |
| `few` | $n \pmod{100} \in [3, 10]$ | Few (Paucal) | `{count} صور` (e.g. 3, 7, 10 صور) |
| `many` | $n \pmod{100} \in [11, 99]$ | Many (Plural) | `{count} صورة` (e.g. 25, 60 صورة) |
| `other` | Everything else (100, 101, 102...) | General | `{count} صورة` |

#### Declarative Plural Definition
```cpp
enki::I18n::define(enki::I18nConfig{
    // ...
    .plurals = {
        { "en", {
            { "photos_count", enki::PluralForms{
                .one   = "1 photo",
                .other = "{count} photos"
            }}
        }},
        { "ar", {
            { "photos_count", enki::PluralForms{
                .zero  = "لا توجد صور",
                .one   = "صورة واحدة",
                .two   = "صورتان",
                .few   = "{count} صور",
                .many  = "{count} صورة",
                .other = "{count} صورة"
            }}
        }}
    }
});
```

#### Usage in Code
```cpp
// Returns "صورة واحدة" for 1, "صورتان" for 2, "5 صور" for 5, "60 صورة" for 60:
std::string text = enki::trPlural("photos_count", photo_list.size());
```

---

### D. Parameter Interpolation

Translations can contain named parameter tokens enclosed in braces `{param}`:

```cpp
// Dictionary definition:
// "greet" -> "Hello, {name}! You have {unread} unread messages."

std::string msg = enki::tr("greet", {
    {"name", "Ali"},
    {"unread", std::to_string(5)}
});
```

---

## 3. Reactive UI Rebuilding on Language Switch

When a user switches the application language, the entire visible widget tree must update instantly without restarting the application or discarding session state.

### Pattern: Root-Level Rebuild with State Preservation

By placing an `I18n::onLocaleChanged()` listener inside the root `StatefulWidget` (e.g., `App` or `WindowFrame`), calling `setState([]{})` triggers reconciliation down through all descendants while preserving Cubits and providers.

```cpp
#include "enki/app/app.hpp"
#include "enki/state/state.hpp"
#include "enki/state/bloc_provider.hpp"
#include "enki/widgets/window_frame.hpp"
#include "enki/i18n/i18n.hpp"

class MainAppState : public enki::State {
    std::shared_ptr<MyCubit> cubit_;
    enki::SlotId locale_sub_ = 0;

public:
    void initState() override {
        State::initState();
        cubit_ = std::make_shared<MyCubit>();

        // 1. Subscribe to language changes
        locale_sub_ = enki::I18n::onLocaleChanged().connect([this](const enki::Locale&) {
            setState([]{}); // Rebuild tree with new language
        });
    }

    void dispose() override {
        // 2. Safe cleanup on unmount
        if (locale_sub_ != 0) {
            enki::I18n::onLocaleChanged().disconnect(locale_sub_);
            locale_sub_ = 0;
        }
        State::dispose();
    }

    enki::WidgetPtr build(enki::BuildContext&) override {
        auto page = enki::bloc_provider_value<MyCubit>(
            cubit_, // Cubit state is retained!
            std::make_shared<MyPage>()
        );

        return enki::windowFrame(enki::WindowFrameProps{
            .content = page,
            .title = std::string(enki::tr("app.window_title")),
            .background_color = 0xFF0B0F19,
        });
    }
};

class MainApp : public enki::StatefulWidget {
public:
    std::unique_ptr<enki::State> createState() override {
        return std::make_unique<MainAppState>();
    }
    std::string_view typeName() const override { return "MainApp"; }
};
```

---

## 4. Real-World Implementation: `real_app/gallery`

The `enki_gallery` application showcases complete end-to-end localization covering Arabic (`ar_IQ`) and English (`en_US`).

### Step 1: Centralized Translation Catalog (`gallery_i18n.hpp`)

File: [`real_app/gallery/src/services/gallery_i18n.hpp`](file:///c:/Users/x/Desktop/enki/real_app/gallery/src/services/gallery_i18n.hpp)

```cpp
#pragma once
#include "enki/i18n/i18n.hpp"
#include "enki/i18n/locale.hpp"

namespace enki::gallery {

inline void initGalleryTranslations() {
    I18n::define(I18nConfig{
        .default_locale = Locale("en", "US"),
        .fallback_locale = Locale("en", "US"),
        .translations = {
            { "en", {
                { "gallery.title",            "Device Gallery" },
                { "gallery.window_title",     "ENKI Gallery • Real Photo Storage" },
                { "gallery.subtitle",         "Loaded: {count} from storage" },
                { "gallery.rescan",           "Rescan Storage" },
                { "folder.all",               "All" },
                { "folder.favorites",         "Favorites" },
                { "folder.pictures",          "Pictures" },
                { "folder.downloads",         "Downloads" },
                { "folder.images",            "Images" },
                { "card.fav",                 "FAV" },
                { "permission.granted",       "Storage Permission: GRANTED" },
                { "permission.desc_granted",  "Read access to local media library is active." },
                { "viewer.back",              "< Back" },
                { "viewer.prev",              "< Prev" },
                { "viewer.next",              "Next >" },
            }},
            { "ar", {
                { "gallery.title",            "معرض الصور" },
                { "gallery.window_title",     "معرض صور إنكي • تخزين حقيقي للصور" },
                { "gallery.subtitle",         "تم تحميل {count} من الذاكرة" },
                { "gallery.rescan",           "إعادة فحص الذاكرة" },
                { "folder.all",               "الكل" },
                { "folder.favorites",         "المفضلة" },
                { "folder.pictures",          "الصور" },
                { "folder.downloads",         "التنزيلات" },
                { "folder.images",            "صور" },
                { "card.fav",                 "مفضلة" },
                { "permission.granted",       "إذن التخزين: مفعّل" },
                { "permission.desc_granted",  "الوصول إلى مكتبة الوسائط المحلية مفعّل وجاهز." },
                { "viewer.back",              "رجوع >" },
                { "viewer.prev",              "السابق" },
                { "viewer.next",              "التالي" },
            }}
        },
        .plurals = {
            { "en", {
                { "photos_count", PluralForms{
                    .one   = "1 photo",
                    .other = "{count} photos"
                }}
            }},
            { "ar", {
                { "photos_count", PluralForms{
                    .zero  = "لا توجد صور",
                    .one   = "صورة واحدة",
                    .two   = "صورتان",
                    .few   = "{count} صور",
                    .many  = "{count} صورة",
                    .other = "{count} صورة"
                }}
            }}
        }
    });
}

/// Helper to map filesystem directories to localized labels while retaining raw filter keys
inline std::string localizeFolderName(std::string_view folder) {
    if (folder == "All") return std::string(tr("folder.all"));
    if (folder == "Favorites") return std::string(tr("folder.favorites"));
    if (folder == "Pictures") return std::string(tr("folder.pictures"));
    if (folder == "Downloads") return std::string(tr("folder.downloads"));
    if (folder == "imges" || folder == "images") return std::string(tr("folder.images"));
    return std::string(folder);
}

} // namespace enki::gallery
```

### Step 2: Interactive Language Switcher in App Bar (`app_bar.hpp`)

File: [`real_app/gallery/src/ui/components/app_bar.hpp`](file:///c:/Users/x/Desktop/enki/real_app/gallery/src/ui/components/app_bar.hpp)

```cpp
// 1. Translated title
auto title = text(std::string(tr("gallery.title")), {
    .color = Palette::text_white,
    .font_size = 18.0f,
    .font_weight = FontWeight::Bold
});

// 2. Arabic/English CLDR plural count in subtitle
std::string count_str = trPlural("photos_count", state.photos.size());
std::string sub_str = tr("gallery.subtitle", {{"count", count_str}});
auto subtitle = text(sub_str, {
    .color = Palette::text_muted,
    .font_size = 12.0f
});

// 3. Interactive Toggle Button: [English / عربي]
bool is_ar = (I18n::currentLocale().language == "ar");
auto lang_btn = makeClickable(
    makeBox(
        text(is_ar ? "🌐 English" : "🌐 عربي", {
            .color = Palette::text_white,
            .font_size = 12.0f,
            .font_weight = FontWeight::SemiBold
        }),
        is_ar ? 0x3038BDF8 : 0x2510B981,
        BorderRadius::circular(8.0f),
        Border(is_ar ? Palette::primary_neon : Palette::emerald, 1.0f),
        StyleInsets::symmetric(6.0f, 10.0f)
    ),
    []() {
        if (I18n::currentLocale().language == "ar") {
            I18n::setLocale(Locale("en", "US"));
        } else {
            I18n::setLocale(Locale("ar", "IQ")); // Arabic (Iraq)
        }
    }
);
```

### Step 3: Decoupled Filter Chips (`filter_chips.hpp`)

File: [`real_app/gallery/src/ui/components/filter_chips.hpp`](file:///c:/Users/x/Desktop/enki/real_app/gallery/src/ui/components/filter_chips.hpp)

To avoid breaking business logic, folder filters keep their original English/system keys (`"All"`, `"Favorites"`, `"Pictures"`) for filtering, while displaying localized strings to the user:

```cpp
std::string display_label = localizeFolderName(folder);

auto chip = makeClickable(
    makeBox(
        text(display_label, {
            .color = active ? Palette::text_white : Palette::text_muted,
            .font_size = 13.0f,
            .font_weight = active ? FontWeight::Bold : FontWeight::Normal
        }),
        active ? Palette::primary : Palette::bg_card,
        BorderRadius::circular(20.0f),
        Border(active ? Palette::primary_neon : Palette::border_subtle, 1.0f),
        StyleInsets::symmetric(6.0f, 14.0f)
    ),
    // Pass immutable internal key to cubit
    [cubit, folder]() { if (cubit) cubit->setFolder(folder); }
);
```

---

## 5. API Reference

### `enki::I18n` Static API

| Method | Parameters | Return Type | Description |
|---|---|---|---|
| `define` | `I18nConfig config` | `void` | Initialize application dictionaries, fallback locale, and plural rules. |
| `setLocale` | `const Locale& locale` | `void` | Switch active runtime locale and dispatch `onLocaleChanged`. |
| `currentLocale` | *none* | `const Locale&` | Retrieve active runtime locale. |
| `fallbackLocale` | *none* | `const Locale&` | Retrieve default fallback locale. |
| `isRTL` | *none* | `bool` | Returns `true` if active locale writes Right-to-Left. |
| `onLocaleChanged` | *none* | `Signal<const Locale&>&` | Global signal emitted on locale change. |
| `translate` | `string_view key, StringMap params = {}` | `std::string` | Look up translation and interpolate `{name}` parameters. |
| `translatePlural` | `string_view key, uint64_t count, StringMap params = {}` | `std::string` | Select grammatical plural category according to CLDR rules and interpolate. |

### Global Helper Functions

| Function | Signature | Description |
|---|---|---|
| `enki::tr` | `std::string tr(string_view key, StringMap params = {})` | Shorthand for `I18n::translate()`. |
| `enki::trPlural` | `std::string trPlural(string_view key, uint64_t count, StringMap params = {})` | Shorthand for `I18n::translatePlural()`. |
| `enki::localizedText`| `WidgetPtr localizedText(string_view key, StringMap params = {}, TextStyle style = {})` | Declarative widget that outputs a `Text` widget with translated content. |

---

## 6. Best Practices & Engineering Guidelines

1. **Namespace Your Translation Keys**:
   Use hierarchical, dot-separated identifiers (e.g., `gallery.title`, `auth.login_btn`, `settings.theme_dark`) to avoid key collisions across large multi-module applications.

2. **Never Hardcode Plural Concatenation**:
   Avoid constructing sentences like `"You have " + count + " item(s)"`. Always use `trPlural("items_count", count)` to guarantee correct grammatical agreement across complex languages (such as Arabic or Polish).

3. **Separate Presentation from State Identifiers**:
   Keep internal database, enum, or filesystem keys in standard canonical ASCII (e.g., `"Pictures"`, `"Favorites"`, `"status_active"`). Map them to localized display strings only at the presentation boundary via helper functions.

4. **Lifecycle-Bound Event Subscriptions**:
   When subscribing to `I18n::onLocaleChanged()`, always store the returned `SlotId` and call `disconnect(slot_id)` inside `State::dispose()` to prevent memory leaks and dangling pointer invocations.

5. **Let Skia Handle BiDi and Cursive Shaping**:
   Do not manually reverse Arabic characters or insert manual layout markers. SkParagraph handles HarfBuzz shaping and BiDi analysis automatically. Ensure the font configured supports the required Unicode ranges (e.g., Arabic, Latin, Cyrillic).

---

## See Also
- [**Cubit State Management**](../state_management/cubit.md) — Reactive subtree management with `Cubit` and `BlocBuilder`.
- [**SelectableText**](../enki/Typography/selectable_text.md) — Interactive typography with clipboard and selection support.
- [**Start Application**](../enki/Start%20Application/start_application.md) — Application bootstrap and `AppConfig`.
