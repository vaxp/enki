# Form

> A container widget that unifies state tracking, validation orchestration, and reset handling for multiple child `FormField` components.

- **Header File**: `#include "enki/widgets/form.hpp"`
- **C++ Class**: `enki::FormWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::Form` (converts implicitly to `WidgetPtr`)
- **Controller**: `enki::FormState`
- **Enum**: `enki::AutoValidateMode` (`Disabled`, `Always`, `OnUserInteraction`)

---

## Overview

The `Form` widget acts as an umbrella coordinator over input fields. When bound to a `FormState` controller, calling `form->validate()` systematically runs every child field's validator callback, renders red error messages below offending fields, and returns `true` only if every field is valid.

---

## C++ API Definition

### Controller (`FormState`)
```cpp
namespace enki {

class FormState {
public:
    /// Validates all registered fields and returns true if ALL pass.
    bool validate();

    /// Calls on_saved() on all registered fields.
    void save();

    /// Resets all fields to their initial states and clears validation errors.
    void reset();
};

} // namespace enki
```

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

enum class AutoValidateMode {
    Disabled,           ///< Validate only when validate() is explicitly called
    Always,             ///< Continuous live validation on every keystroke/change
    OnUserInteraction   ///< Validate once user has touched/interacted with the field
};

struct Form {
    Key                        key               = Key::none();
    WidgetPtr                  child             = nullptr;
    AutoValidateMode           autovalidate_mode = AutoValidateMode::Disabled;
    std::shared_ptr<FormState> controller        = nullptr;
    std::function<void()>      on_changed        = nullptr;

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `child` | `WidgetPtr` | `nullptr` | Layout widget (such as a `Column`) containing form fields and submit buttons. |
| `controller` | `std::shared_ptr<FormState>` | `nullptr` | Coordinator handling unified validation, saving, and reset actions. |
| `autovalidate_mode`| `AutoValidateMode` | `Disabled` | Determines when fields validate (`Disabled`, `Always`, or `OnUserInteraction`). |
| `on_changed` | `std::function<void()>` | `nullptr` | Invoked whenever any registered child field changes value. |

---

## Code Examples (From `widgets_demo/form_demo/main.cpp`)

### 1. Form Validation on Submit
```cpp
#include "enki/widgets/form.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

class MyFormView : public State {
    std::shared_ptr<FormState> form_state_ = std::make_shared<FormState>();

public:
    WidgetPtr build(BuildContext&) override {
        return Form {
            .controller = form_state_,
            .autovalidate_mode = AutoValidateMode::OnUserInteraction,
            .child = column({
                .gap = 16_px,
                .children = {
                    TextFormField {
                        .label = "Username",
                        .form_state = form_state_,
                        .validator = Validators::required("Username is required"),
                    },
                    row({
                        .gap = 12_px,
                        .children = {
                            Button {
                                .child = text("Reset"),
                                .on_pressed = [this]() {
                                    form_state_->reset();
                                }
                            },
                            Button {
                                .child = text("Submit"),
                                .on_pressed = [this]() {
                                    if (form_state_->validate()) {
                                        form_state_->save();
                                        std::cout << "Form is completely valid!\n";
                                    }
                                }
                            }
                        }
                    })
                }
            })
        };
    }
};
```

---

## See Also
- [**FormField**](./form_field.md) — The individual input fields bound to a form (`TextFormField`, `CheckboxFormField`).
- [**TextField**](./text_field.md) — Raw unmanaged text input.
