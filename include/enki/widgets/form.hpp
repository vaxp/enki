#pragma once
/// @file form.hpp
/// @brief Advanced Form & FormField validation and state management suite for ENKI Framework (Category 3. Input / Forms).
/// Supports FormState with unified validate/save/reset, FormField<T>, TextFormField, CheckboxFormField,
/// built-in composable Validators library, and live error decoration.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/state/state.hpp"
#include "enki/widgets/text_field.hpp"
#include "enki/rendering/color.hpp"

#include <string>
#include <string_view>
#include <functional>
#include <memory>
#include <vector>
#include <optional>
#include <regex>
#include <algorithm>

namespace enki {

/// ════════════════════════════════════════════════════════════════
/// AutoValidateMode Enum
/// ════════════════════════════════════════════════════════════════

enum class AutoValidateMode {
    Disabled,           ///< Validate only when validate() is explicitly called
    Always,             ///< Continuous live validation on every keystroke/change
    OnUserInteraction   ///< Validate once user has touched/interacted with the field
};

/// ════════════════════════════════════════════════════════════════
/// FormField State Interface
/// ════════════════════════════════════════════════════════════════

class FormFieldStateBase {
public:
    virtual ~FormFieldStateBase() = default;
    virtual bool validate() = 0;
    virtual void save() = 0;
    virtual void reset() = 0;
};

/// ════════════════════════════════════════════════════════════════
/// FormController / FormState
/// ════════════════════════════════════════════════════════════════

class FormState {
public:
    std::vector<FormFieldStateBase*> fields;

    void registerField(FormFieldStateBase* field) {
        if (field) fields.push_back(field);
    }

    void unregisterField(FormFieldStateBase* field) {
        fields.erase(std::remove(fields.begin(), fields.end(), field), fields.end());
    }

    /// Validates all registered fields and returns true if ALL are valid.
    bool validate() {
        bool all_valid = true;
        for (auto* f : fields) {
            if (f && !f->validate()) {
                all_valid = false;
            }
        }
        return all_valid;
    }

    /// Calls save() on all fields.
    void save() {
        for (auto* f : fields) {
            if (f) f->save();
        }
    }

    /// Resets all fields to their initial states and clears validation errors.
    void reset() {
        for (auto* f : fields) {
            if (f) f->reset();
        }
    }
};

/// ════════════════════════════════════════════════════════════════
/// Form Widget Implementation
/// ════════════════════════════════════════════════════════════════

struct FormProps {
    AutoValidateMode autovalidate_mode = AutoValidateMode::Disabled;
    std::shared_ptr<FormState> controller;
    std::function<void()> on_changed;
};

class FormWidget : public StatefulWidget {
public:
    WidgetPtr child;
    FormProps props;

    FormWidget(WidgetPtr child_, FormProps opts = {})
        : child(std::move(child_)), props(std::move(opts)) {}

    FormWidget(Key key, WidgetPtr child_, FormProps opts = {})
        : StatefulWidget(std::move(key)), child(std::move(child_)), props(std::move(opts)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "Form"; }
};

/// ════════════════════════════════════════════════════════════════
/// Standard Reusable Validator Combinators
/// ════════════════════════════════════════════════════════════════

using ValidatorFn = std::function<std::optional<std::string>(const std::string& value)>;

struct Validators {
    /// Field must not be empty / whitespace only
    static ValidatorFn required(std::string error_msg = "This field is required") {
        return [msg = std::move(error_msg)](const std::string& val) -> std::optional<std::string> {
            auto s = val;
            s.erase(0, s.find_first_not_of(" \t\n\r"));
            s.erase(s.find_last_not_of(" \t\n\r") + 1);
            if (s.empty()) return msg;
            return std::nullopt;
        };
    }

    /// Validates email address format
    static ValidatorFn email(std::string error_msg = "Please enter a valid email address") {
        return [msg = std::move(error_msg)](const std::string& val) -> std::optional<std::string> {
            if (val.empty()) return std::nullopt; // Combine with required() for mandatory check
            std::regex pattern(R"([a-zA-Z0-9_.+-]+@[a-zA-Z0-9-]+\.[a-zA-Z0-9-.]+)");
            if (!std::regex_match(val, pattern)) return msg;
            return std::nullopt;
        };
    }

    /// Minimum string length
    static ValidatorFn minLength(size_t min_len, std::string error_msg = "") {
        return [min_len, msg = std::move(error_msg)](const std::string& val) -> std::optional<std::string> {
            if (val.size() < min_len) {
                if (!msg.empty()) return msg;
                return "Must be at least " + std::to_string(min_len) + " characters";
            }
            return std::nullopt;
        };
    }

    /// Values match (e.g. Confirm Password match)
    static ValidatorFn match(std::function<std::string()> get_target, std::string error_msg = "Values do not match") {
        return [get_target = std::move(get_target), msg = std::move(error_msg)](const std::string& val) -> std::optional<std::string> {
            if (val != get_target()) return msg;
            return std::nullopt;
        };
    }

    /// Compose multiple validators in sequence (stops on first error)
    static ValidatorFn compose(std::vector<ValidatorFn> validators) {
        return [rules = std::move(validators)](const std::string& val) -> std::optional<std::string> {
            for (const auto& rule : rules) {
                if (rule) {
                    auto err = rule(val);
                    if (err.has_value()) return err;
                }
            }
            return std::nullopt;
        };
    }
};

/// ════════════════════════════════════════════════════════════════
/// TextFormField Widget Implementation
/// ════════════════════════════════════════════════════════════════

struct TextFormFieldProps {
    std::string label;
    std::string hint;
    std::string initial_value;
    std::string helper_text;
    bool required = false;
    bool obscure_text = false;
    float width = 340.0f;

    std::shared_ptr<FormState> form_state;
    std::shared_ptr<TextFieldController> controller;
    ValidatorFn validator;
    std::function<void(const std::string&)> on_saved;
    std::function<void(const std::string&)> on_changed;
};

class TextFormFieldWidget : public StatefulWidget {
public:
    TextFormFieldProps props;

    TextFormFieldWidget() = default;
    explicit TextFormFieldWidget(TextFormFieldProps opts) : props(std::move(opts)) {}
    TextFormFieldWidget(Key key, TextFormFieldProps opts) : StatefulWidget(std::move(key)), props(std::move(opts)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "TextFormField"; }
};

/// ════════════════════════════════════════════════════════════════
/// CheckboxFormField Widget Implementation
/// ════════════════════════════════════════════════════════════════

struct CheckboxFormFieldProps {
    std::string label;
    bool initial_value = false;
    bool required = false;

    std::shared_ptr<FormState> form_state;
    std::function<std::optional<std::string>(bool value)> validator;
    std::function<void(bool)> on_saved;
    std::function<void(bool)> on_changed;
};

class CheckboxFormFieldWidget : public StatefulWidget {
public:
    CheckboxFormFieldProps props;

    CheckboxFormFieldWidget() = default;
    explicit CheckboxFormFieldWidget(CheckboxFormFieldProps opts) : props(std::move(opts)) {}
    CheckboxFormFieldWidget(Key key, CheckboxFormFieldProps opts) : StatefulWidget(std::move(key)), props(std::move(opts)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "CheckboxFormField"; }
};

/// ════════════════════════════════════════════════════════════════
/// Declarative Proxy Structs (C++20 Designated Initializers)
/// ════════════════════════════════════════════════════════════════

struct Form {
    Key key = Key::none();
    WidgetPtr child = nullptr;
    AutoValidateMode autovalidate_mode = AutoValidateMode::Disabled;
    std::shared_ptr<FormState> controller = nullptr;
    std::function<void()> on_changed = nullptr;

    operator WidgetPtr() const {
        FormProps p;
        p.autovalidate_mode = autovalidate_mode;
        p.controller = controller;
        p.on_changed = on_changed;
        return std::make_shared<FormWidget>(key, child, std::move(p));
    }
};

struct TextFormField {
    Key key = Key::none();
    std::string label;
    std::string hint;
    std::string initial_value;
    std::string helper_text;
    bool required = false;
    bool obscure_text = false;
    float width = 340.0f;

    std::shared_ptr<FormState> form_state = nullptr;
    std::shared_ptr<TextFieldController> controller = nullptr;
    ValidatorFn validator = nullptr;
    std::function<void(const std::string&)> on_saved = nullptr;
    std::function<void(const std::string&)> on_changed = nullptr;

    operator WidgetPtr() const {
        TextFormFieldProps p;
        p.label = label;
        p.hint = hint;
        p.initial_value = initial_value;
        p.helper_text = helper_text;
        p.required = required;
        p.obscure_text = obscure_text;
        p.width = width;
        p.form_state = form_state;
        p.controller = controller;
        p.validator = validator;
        p.on_saved = on_saved;
        p.on_changed = on_changed;
        return std::make_shared<TextFormFieldWidget>(key, std::move(p));
    }
};

struct CheckboxFormField {
    Key key = Key::none();
    std::string label;
    bool initial_value = false;
    bool required = false;

    std::shared_ptr<FormState> form_state = nullptr;
    std::function<std::optional<std::string>(bool value)> validator = nullptr;
    std::function<void(bool)> on_saved = nullptr;
    std::function<void(bool)> on_changed = nullptr;

    operator WidgetPtr() const {
        CheckboxFormFieldProps p;
        p.label = label;
        p.initial_value = initial_value;
        p.required = required;
        p.form_state = form_state;
        p.validator = validator;
        p.on_saved = on_saved;
        p.on_changed = on_changed;
        return std::make_shared<CheckboxFormFieldWidget>(key, std::move(p));
    }
};

} // namespace enki
