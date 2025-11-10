# Enki Input & Form Widgets

> High-performance, accessible, and reactive input controls and form validation primitives for desktop applications.

The **Input / Forms** category provides a comprehensive suite of graphical user input widgets in Enki. Every component features C++20 designated initializer support, dedicated state controllers (`TextFieldController`, `FormState`, `DatePickerController`, etc.), two-way data flow callbacks (`on_changed`, `on_submitted`), and hardware-accelerated Skia 2D rendering.

---

## Widget Catalog (Input / Forms)

| # | Widget | Primary Type / Helper | Header | Description |
|---|---|---|---|---|
| 1 | [**TextField**](./text_field.md) | `struct TextField`, `textField(...)` | `<enki/widgets/text_field.hpp>` | Single-line interactive text input with cursor, selection, clipboard, and BiDi UTF-8. |
| 2 | [**TextArea**](./text_area.md) | `struct TextArea`, `TextAreaWidget` | `<enki/widgets/text_area.hpp>` | Multi-line text editor with line numbers, undo/redo history, and word counters. |
| 3 | [**PasswordField**](./password_field.md) | `struct PasswordField`, `PasswordFieldWidget` | `<enki/widgets/password_field.hpp>` | Password input with eye visibility toggle, live strength meter, and CapsLock warning. |
| 4 | [**NumberField**](./number_field.md) | `struct NumberField`, `NumberFieldWidget` | `<enki/widgets/number_field.hpp>` | Numeric input with precision, horizontal mouse scrubbing, steppers, and unit prefixes/suffixes. |
| 5 | [**Checkbox**](./checkbox.md) | `struct Checkbox`, `class CheckboxWidget` | `<enki/widgets/checkbox.hpp>` | Binary toggle box with checkmark animation and disabled states. |
| 6 | [**Radio**](./radio.md) | `struct Radio`, `class RadioWidget` | `<enki/widgets/radio.hpp>` | Mutually exclusive radio selector bound to an integer `group_value`. |
| 7 | [**Switch**](./switch.md) | `struct Switch`, `class SwitchWidget` | `<enki/widgets/switch.hpp>` | Modern sliding pill toggle for on/off boolean settings. |
| 8 | [**Slider**](./slider.md) | `struct Slider`, `class SliderWidget` | `<enki/widgets/slider.hpp>` | Continuous horizontal track slider for selecting a numeric value in a range. |
| 9 | [**RangeSlider**](./range_slider.md) | `struct RangeSlider`, `RangeSliderWidget` | `<enki/widgets/range_slider.hpp>` | Dual-thumb track slider for selecting both a start and end value without overlap. |
| 10 | [**ComboBox**](./combo_box.md) | `struct ComboBox`, `ComboBoxWidget` | `<enki/widgets/combo_box.hpp>` | Searchable select dropdown supporting single choice, multi-tag chips, and groups. |
| 11 | [**DatePicker**](./date_picker.md) | `struct DatePicker`, `DateRangePicker` | `<enki/widgets/date_picker.hpp>` | Calendar picker supporting popup/inline modes, date ranges, and quick preset chips. |
| 12 | [**TimePicker**](./time_picker.md) | `struct TimePicker`, `TimePickerWidget` | `<enki/widgets/time_picker.hpp>` | Clock picker supporting 12h AM/PM and 24h military time with second precision. |
| 13 | [**ColorPicker**](./color_picker.md) | `struct ColorPicker`, `ColorPickerWidget` | `<enki/widgets/color_picker.hpp>` | 2D saturation-value canvas picker with hue/alpha sliders, HEX/RGBA formats, and palettes. |
| 14 | [**Form**](./form.md) | `struct Form`, `class FormState` | `<enki/widgets/form.hpp>` | Form orchestrator providing unified `validate()`, `save()`, and `reset()` workflows. |
| 15 | [**FormField**](./form_field.md) | `TextFormField`, `CheckboxFormField` | `<enki/widgets/form.hpp>` | Form-bound fields integrated with the composable `Validators` library. |
| 16 | [**SearchField**](./search_field.md) | `struct SearchField`, `SearchFieldWidget` | `<enki/widgets/search_field.hpp>` | Command palette / search input with debounced query provider and shortcut badges. |

---

## Form Architecture & Validation Workflow

Forms in Enki follow an orchestrated controller architecture:
1. **`FormState` Controller**: Holds references to all registered `FormField` elements.
2. **`Form` Widget**: Encapsulates form fields and manages automatic validation triggers (`AutoValidateMode`).
3. **`Validators` Combinators**: Composable rules (`required`, `email`, `minLength`, `match`, `compose`).

```
          ┌───────────────────────────────────┐
          │             FormWidget            │
          │      (controller: FormState)      │
          └─────────────────┬─────────────────┘
                            │
         ┌──────────────────┴──────────────────┐
         ▼                                     ▼
┌──────────────────┐                  ┌──────────────────┐
│  TextFormField   │                  │ CheckboxFormField│
│(Validators::email│                  │(Validators::req) │
└──────────────────┘                  └──────────────────┘
         │                                     │
         └──────────────────┬──────────────────┘
                            ▼
          [ FormState::validate() == true ]
                            ▼
                  [ FormState::save() ]
```

---

## Quick Example (Unified Form)

```cpp
#include "enki/app/app.hpp"
#include "enki/widgets/form.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/flexbox.hpp"

using namespace enki;

class RegistrationForm : public State {
    std::shared_ptr<FormState> form = std::make_shared<FormState>();
    std::string email_val;

public:
    WidgetPtr build(BuildContext&) override {
        return Form {
            .controller = form,
            .autovalidate_mode = AutoValidateMode::OnUserInteraction,
            .child = column({
                .gap = 14_px,
                .children = {
                    TextFormField {
                        .label = "Email Address",
                        .hint = "user@example.com",
                        .validator = Validators::compose({
                            Validators::required("Email cannot be empty"),
                            Validators::email("Enter a valid corporate email")
                        }),
                        .on_saved = [this](const std::string& v) { email_val = v; }
                    },
                    CheckboxFormField {
                        .label = "I accept terms and conditions",
                        .required = true,
                        .validator = [](bool v) -> std::optional<std::string> {
                            if (!v) return "You must agree to continue";
                            return std::nullopt;
                        }
                    },
                    Button {
                        .child = text("Create Account"),
                        .on_pressed = [this]() {
                            if (form->validate()) {
                                form->save();
                                std::cout << "Registered: " << email_val << "\n";
                            }
                        }
                    }
                }
            })
        };
    }
};
```
