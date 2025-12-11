# TagInput

> Interactive multi-tag chip input with auto-wrap, inline text entry, and removable chips.

- **Header File**: `#include "enki/widgets/tag_input.hpp"`
- **C++ Class**: `enki::TagInputWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Helper**: `enki::tagInput(TagInputProps props)` (returns `enki::WidgetPtr`)
- **State Object**: `enki::TagInputState`
- **Underlying Mechanism**: Flexbox wrapped chip list with inline text editor, backspace tag deletion, and interactive "×" removal triggers.

---

## Overview

`TagInput` allows users to enter dynamic lists of labels, tags, or keywords. Typing text and pressing `Enter` or `,` commits the input into a standalone chip widget. Tags can be removed either by clicking their delete icon or by pressing `Backspace` when the text input cursor is empty.

---

## C++ API Definition

### Struct Definition (`enki/widgets/tag_input.hpp`)
```cpp
namespace enki {

struct TagInputProps {
    std::vector<std::string>                    tags;
    std::string                                 placeholder = "Add tag and press Enter...";
    int                                         max_tags = 20;
    bool                                        allow_duplicates = false;

    Color                                       chip_background = 0x2600E5FF;
    Color                                       chip_border_color = 0x6600E5FF;
    Color                                       chip_text_color = 0xFF38BDF8;
    Color                                       delete_icon_color = 0xFF94A3B8;
    Color                                       container_background = 0x59000000;
    Color                                       border_color = 0x3300E5FF;
    float                                       border_radius = 10.0f;
    float                                       min_height = 42.0f;

    std::function<void(const std::vector<std::string>&)> on_tags_changed;

    operator WidgetPtr() const;
};

class TagInputWidget : public StatefulWidget {
public:
    TagInputProps props;

    explicit TagInputWidget(TagInputProps p)
        : props(std::move(p)) {}

    [[nodiscard]] std::string_view typeName() const override { return "TagInput"; }
    [[nodiscard]] std::unique_ptr<State> createState() override;
};

inline WidgetPtr tagInput(TagInputProps props) {
    return std::make_shared<TagInputWidget>(std::move(props));
}

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `tags` | `std::vector<std::string>` | `{}` | Initial collection of tag strings. |
| `placeholder` | `std::string` | `"Add tag and press Enter..."` | Ghost placeholder text shown when input is empty. |
| `max_tags` | `int` | `20` | Maximum count of allowable tags in the container. |
| `allow_duplicates` | `bool` | `false` | When `false`, prevents adding identical tag strings. |
| `chip_background` | `Color` | `0x2600E5FF` | Fill color for the chip capsules. |
| `chip_border_color`| `Color` | `0x6600E5FF` | Stroke border color for the chips. |
| `chip_text_color` | `Color` | `0xFF38BDF8` | Font color for the tag label. |
| `delete_icon_color`| `Color` | `0xFF94A3B8` | Color of the "×" removal glyph. |
| `container_background`| `Color` | `0x59000000` | Fill color of the surrounding box. |
| `border_color` | `Color` | `0x3300E5FF` | Border color of the surrounding box. |
| `border_radius`| `float` | `10.0f` | Corner rounding radius. |
| `min_height` | `float` | `42.0f` | Minimum vertical size of the tag field. |
| `on_tags_changed` | `std::function<void(const std::vector<std::string>&)>` | `nullptr` | Callback fired whenever tags are added or removed. |

---

## Code Examples (From `widgets_demo/tag_input_demo/main.cpp`)

### 1. Tokenized Tag Manager
```cpp
auto ti = tagInput({
    .tags = {"VAXP-OS", "ZeroCopy", "Skia", "C++20", "ENKI"},
    .placeholder = "Type tag name and hit Enter...",
    .on_tags_changed = [this](const std::vector<std::string>& t) {
        tags_ = t;
        setState([]{});
    },
});
```
