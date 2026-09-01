# ComboBox

> A rich searchable select dropdown widget supporting single-selection, multi-select tag chips, live search filtering, option grouping, and keyboard navigation.

- **Header File**: `#include "enki/widgets/combo_box.hpp"`
- **C++ Class**: `enki::ComboBoxWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::ComboBox` (converts implicitly to `WidgetPtr`)
- **Props Struct**: `enki::ComboBoxProps`
- **Controller**: `enki::ComboBoxController`
- **Item Struct**: `enki::ComboBoxItem`
- **Enums**: `enki::ComboBoxMode` (`Single`, `Multi`, `Custom`)

---

## Overview

`ComboBox` combines a text input with a floating dropdown menu. It allows users to quickly filter through large lists of options, categorize items into groups, attach icon/badge metadata, and select either a single item or multiple tags.

---

## C++ API Definition

### Item Model (`ComboBoxItem`)
```cpp
namespace enki {

struct ComboBoxItem {
    std::string id          = "";
    std::string label       = "";
    std::string subtitle    = "";
    std::string icon        = "";            // Emoji or icon glyph (e.g. 🦀, 🌐, 🔒)
    std::string group       = "";            // Categorical group header
    std::string badge       = "";            // Trailing badge (e.g. "FAST", "PRO")
    Color       badge_color = 0xFF38BDF8;
    bool        is_disabled = false;

    ComboBoxItem() = default;
    ComboBoxItem(std::string id_, std::string label_, std::string icon_ = "",
                 std::string subtitle_ = "", std::string group_ = "");

    ComboBoxItem& setBadge(std::string b, Color c = 0xFF38BDF8);
    ComboBoxItem& setDisabled(bool d);
};

} // namespace enki
```

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

enum class ComboBoxMode {
    Single,     ///< Single selection; choice fills the input text
    Multi,      ///< Multi-selection; choices render as removable tag chips
    Custom      ///< Allows typing arbitrary custom values
};

struct ComboBox {
    Key                                 key                = Key::none();
    std::vector<ComboBoxItem>           items;
    WidgetPtr                           body               = nullptr;
    std::shared_ptr<ComboBoxController> controller         = nullptr;

    ComboBoxMode                        mode               = ComboBoxMode::Single;
    std::string                         placeholder        = "Search or select option...";

    float                               width              = 320.0f;
    float                               input_height       = 42.0f;
    float                               max_menu_height    = 260.0f;
    float                               border_radius      = 8.0f;

    bool                                allow_clear        = true;
    bool                                allow_custom_value = false;
    bool                                show_search_icon   = true;

    Color                               background_color   = 0xFF0F172A;
    Color                               border_color       = 0xFF334155;
    Color                               border_focus_color = 0xFF0284C7;
    Color                               text_color         = 0xFFFFFFFF;
    Color                               menu_bg_color      = 0xFF1E293B;

    std::function<void(const ComboBoxItem&)>              on_selected     = nullptr;
    std::function<void(const std::vector<ComboBoxItem>&)> on_multi_changed= nullptr;
    std::function<void(const std::string&)>               on_custom_value = nullptr;

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `items` | `std::vector<ComboBoxItem>` | `{}` | List of selectable choices. |
| `mode` | `ComboBoxMode` | `Single` | Selection behavior (`Single`, `Multi`, `Custom`). |
| `placeholder` | `std::string` | `"Search or select..."` | Watermark text when no selection is made. |
| `width` | `float` | `320.0f` | Total width of the input field and dropdown menu. |
| `max_menu_height` | `float` | `260.0f` | Maximum height of the scrollable dropdown list. |
| `allow_clear` | `bool` | `true` | Displays a trailing `✕` button to clear the selection. |
| `show_search_icon` | `bool` | `true` | Displays a leading `🔍` search icon inside the field. |
| `on_selected` | `std::function<void(const ComboBoxItem&)>` | `nullptr` | Callback for single selection. |
| `on_multi_changed` | `std::function<void(const std::vector<ComboBoxItem>&)>` | `nullptr` | Callback when tags change in multi-select mode. |

---

## Code Examples (From `widgets_demo/combo_box_demo/main.cpp`)

### 1. Grouped Cloud Region Dropdown
```cpp
#include "enki/widgets/combo_box.hpp"

using namespace enki;

auto regionDropdown = ComboBox {
    .placeholder = "Select cloud deployment region...",
    .items = {
        ComboBoxItem("us-east-1", "US East (N. Virginia)", "🇺🇸", "Low Latency", "North America")
            .setBadge("FAST", 0xFF10B981),
        ComboBoxItem("us-west-2", "US West (Oregon)", "🇺🇸", "", "North America"),
        ComboBoxItem("eu-central-1", "Frankfurt", "🇩🇪", "GDPR Ready", "Europe")
            .setBadge("EU", 0xFF38BDF8),
        ComboBoxItem("ap-northeast-1", "Tokyo", "🇯🇵", "", "Asia Pacific"),
    },
    .on_selected = [](const ComboBoxItem& item) {
        std::cout << "Selected: " << item.id << " (" << item.label << ")\n";
    }
};
```

### 2. Multi-Select Tags Mode
```cpp
auto skillsPicker = ComboBox {
    .mode = ComboBoxMode::Multi,
    .placeholder = "Choose technologies...",
    .items = {
        ComboBoxItem("cpp", "C++20", "⚡"),
        ComboBoxItem("skia", "Skia Graphics", "🎨"),
        ComboBoxItem("wayland", "Wayland Protocol", "🐧"),
    },
    .on_multi_changed = [](const std::vector<ComboBoxItem>& tags) {
        std::cout << "Selected " << tags.size() << " tags\n";
    }
};
```

---

## See Also
- [**TextField**](./text_field.md) — Simple unconstrained text input.
- [**SearchField**](./search_field.md) — Command palette and application-wide search.
- [**Chip**](../Basic%20UI/chip.md) — Tags used in multi-selection.
