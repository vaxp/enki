# TextArea

> An advanced multi-line text editor widget supporting line numbers, undo/redo history, clipboard integration, character/word counters, and smooth scrolling.

- **Header File**: `#include "enki/widgets/text_area.hpp"`
- **C++ Class**: `enki::TextAreaWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::TextArea` (converts implicitly to `WidgetPtr`)
- **Props Struct**: `enki::TextAreaProps`
- **Controller**: `enki::TextAreaController`
- **Underlying Engine**: Skia `SkParagraph` multi-line layout with scroll offset calculation

---

## Overview

`TextArea` extends text input into a full-featured multi-line editor suitable for code snippets, markdown drafting, comments, and long descriptions. It includes an integrated left-side line numbers gutter, a 100-step undo/redo stack (`Ctrl+Z`, `Ctrl+Y`), live line and word counters, and automatic height expansion (`auto_grow`).

---

## C++ API Definition

### Controller (`TextAreaController`)
```cpp
namespace enki {

struct TextAreaController {
    std::string text;
    size_t      selection_start = 0;
    size_t      selection_end   = 0;

    std::vector<std::string> undo_stack;
    std::vector<std::string> redo_stack;

    TextAreaController(std::string initial_text = "");

    void setText(std::string new_text);
    void clearSelection();
    void selectAll();
    [[nodiscard]] bool hasSelection() const;
    [[nodiscard]] std::string getSelectedText() const;

    // Undo / Redo
    bool undo();
    bool redo();
    [[nodiscard]] bool canUndo() const;
    [[nodiscard]] bool canRedo() const;

    // Statistics
    [[nodiscard]] size_t getLineCount() const;
    [[nodiscard]] size_t getWordCount() const;
    void getCursorPosition(size_t& row, size_t& col) const;

    // Clipboard
    bool copyToClipboard();
    bool cutToClipboard();
    bool pasteFromClipboard();
};

} // namespace enki
```

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct TextArea {
    Key                                 key                 = Key::none();
    std::shared_ptr<TextAreaController> controller          = nullptr;

    TextStyle                           style;
    std::string                         hint_text           = "";
    bool                                read_only           = false;
    bool                                auto_focus          = false;

    // Line Bounds & Auto Grow
    size_t                              min_lines           = 4;
    size_t                              max_lines           = 12;
    bool                                auto_grow           = false;

    // Gutter & Counter
    bool                                show_line_numbers   = false;
    bool                                show_counter        = false;
    size_t                              max_characters      = 0;

    // Styling
    Color                               cursor_color        = 0xFF38BDF8;
    Color                               selection_color     = 0x6438BDF8;
    Color                               background_color    = 0xFF1E293B;
    Color                               border_color        = 0xFF334155;
    Color                               focus_border_color  = 0xFF38BDF8;

    Color                               line_number_color   = 0xFF64748B;
    Color                               line_number_bg      = 0xFF0F172A;

    float                               border_radius       = 8.0f;
    EdgeInsets                          padding             = EdgeInsets::all(10.0f);

    // Callbacks
    std::function<void(std::string)>            on_changed       = nullptr;
    std::function<void(std::string)>            on_submitted     = nullptr;
    std::function<void(size_t row, size_t col)> on_cursor_moved  = nullptr;

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `controller` | `std::shared_ptr<TextAreaController>` | `nullptr` | Manages text data, selection, and undo/redo stacks. |
| `min_lines` | `size_t` | `4` | Minimum displayed line height. |
| `max_lines` | `size_t` | `12` | Maximum line height before scrolling activates. |
| `auto_grow` | `bool` | `false` | Automatically grows vertically with new lines up to `max_lines`. |
| `show_line_numbers`| `bool` | `false` | Displays a line number gutter on the left side. |
| `show_counter` | `bool` | `false` | Shows a live character/word count tag at bottom-right. |
| `max_characters` | `size_t` | `0` | Hard character limit (0 disables the limit). |
| `line_number_color`| `Color` | `0xFF64748B` | Text color for numbers in the gutter. |
| `line_number_bg` | `Color` | `0xFF0F172A` | Background strip behind the line number gutter. |
| `on_cursor_moved` | `std::function<void(size_t, size_t)>` | `nullptr` | Callback receiving `(row, col)` on cursor repositioning. |

---

## Code Examples (From `widgets_demo/text_area_demo/main.cpp`)

### 1. Code Editor with Line Numbers Gutter
```cpp
#include "enki/widgets/text_area.hpp"

using namespace enki;

auto codeEditor = TextArea {
    .show_line_numbers = true,
    .min_lines = 8,
    .max_lines = 20,
    .style = TextStyle{
        .font_family = "Fira Code",
        .font_size = 13.0f,
        .color = 0xFFF8FAFC,
    },
    .background_color = 0xFF0F172A, // Deep slate editor background
    .line_number_bg   = 0xFF0B1120, // Dark gutter
    .line_number_color= 0xFF475569,
};
```

### 2. Auto-Growing Feedback Box with Word Limit
```cpp
auto feedbackArea = TextArea {
    .hint_text = "Please leave your comments or feedback...",
    .auto_grow = true,
    .min_lines = 3,
    .max_lines = 8,
    .show_counter = true,
    .max_characters = 500,
    .on_changed = [](const std::string& text) {
        // Handle input changes
    }
};
```

---

## See Also
- [**TextField**](./text_field.md) — Single-line input component.
- [**Card**](../Basic%20UI/card.md) — Surface container often surrounding a `TextArea`.
