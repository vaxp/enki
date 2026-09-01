# TextField

> An interactive single-line text input widget with text cursor, drag selection, clipboard cut/copy/paste, and BiDi UTF-8 support.

- **Header File**: `#include "enki/widgets/text_field.hpp"`
- **C++ Class**: `enki::TextFieldWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::TextField` (converts implicitly to `WidgetPtr`)
- **Props Struct**: `enki::TextFieldProps`
- **Controller**: `enki::TextFieldController`
- **Factory Helpers**: `enki::textField(...)`

---

## Overview

`TextField` provides standard interactive text editing. It tracks cursor position, handles mouse drag-selection, processes backspace/delete keys, and interfaces with the system clipboard. It also supports obscure text mode for sensitive data and triggers `on_changed` and `on_submitted` callbacks.

---

## C++ API Definition

### Controller (`TextFieldController`)
```cpp
namespace enki {

struct TextFieldController {
    std::string text;
    size_t      selection_start = 0;
    size_t      selection_end   = 0;

    TextFieldController(std::string initial_text = "");

    void clearSelection();
    void selectAll();
    [[nodiscard]] bool hasSelection() const;
};

} // namespace enki
```

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct TextField {
    Key                                  key             = Key::none();
    std::shared_ptr<TextFieldController> controller      = nullptr;

    TextStyle                            style           = {};
    std::string                          hint_text       = "";
    std::string                          hint            = ""; // Alias for hint_text
    bool                                 obscure_text    = false;
    bool                                 read_only       = false;
    bool                                 auto_focus      = false;
    size_t                               max_lines       = 1;
    
    Color                                cursor_color    = 0xFF0078D7;
    Color                                selection_color = 0x640078D7;

    std::function<void(std::string)>     on_changed      = nullptr;
    std::function<void(std::string)>     on_submitted    = nullptr;

    operator WidgetPtr() const;
};

} // namespace enki
```

### Factory Helper Functions
```cpp
namespace enki {

inline std::shared_ptr<TextFieldWidget> textField(
    std::shared_ptr<TextFieldController> ctrl,
    TextFieldProps options = {});

inline std::shared_ptr<TextFieldWidget> textField(TextFieldProps props);

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `controller` | `std::shared_ptr<TextFieldController>` | `nullptr` | Shared controller holding text buffer and selection indices. |
| `hint` / `hint_text` | `std::string` | `""` | Placeholder text shown when the input buffer is empty. |
| `style` | `TextStyle` | `{}` | Typography configuration for entered text. |
| `obscure_text` | `bool` | `false` | Masks input with bullets (passwords). |
| `read_only` | `bool` | `false` | Disables keyboard input while allowing selection and copy. |
| `auto_focus` | `bool` | `false` | Automatically acquires keyboard focus on initial render. |
| `max_lines` | `size_t` | `1` | Number of text lines. |
| `cursor_color` | `Color` | `0xFF0078D7` | Vertical blinking cursor bar color. |
| `selection_color` | `Color` | `0x640078D7` | Highlight background for selected text ranges. |
| `on_changed` | `std::function<void(std::string)>` | `nullptr` | Callback triggered whenever text content is altered. |
| `on_submitted` | `std::function<void(std::string)>` | `nullptr` | Callback triggered when the user presses Enter/Return. |

---

## Code Examples (From `widgets_demo/form_demo/main.cpp`)

### 1. Basic Text Field with Placeholder
```cpp
#include "enki/widgets/text_field.hpp"

using namespace enki;

auto usernameField = TextField {
    .hint = "Enter your username",
    .on_changed = [](const std::string& val) {
        std::cout << "Typed: " << val << "\n";
    },
    .on_submitted = [](const std::string& val) {
        std::cout << "Submitted: " << val << "\n";
    }
};
```

### 2. Managed Input with External Controller
```cpp
auto emailCtrl = std::make_shared<TextFieldController>("user@domain.com");

auto emailField = TextField {
    .controller = emailCtrl,
    .cursor_color = 0xFF38BDF8, // Sky-blue cursor
    .on_changed = [emailCtrl](const std::string&) {
        // emailCtrl->text is continuously updated
    }
};

// Programmatic selection or clear:
emailCtrl->selectAll();
```

---

## See Also
- [**TextArea**](./text_area.md) — Multi-line text editor with line numbers and undo history.
- [**PasswordField**](./password_field.md) — Dedicated password input with visibility toggle and strength meter.
- [**FormField**](./form_field.md) — Text field wrapped with validator rules.
