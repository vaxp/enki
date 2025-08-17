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
/// Form Widget
/// ════════════════════════════════════════════════════════════════

struct FormProps {
    AutoValidateMode autovalidate_mode = AutoValidateMode::Disabled;
    std::shared_ptr<FormState> controller;
    std::function<void()> on_changed;
};

class Form : public StatefulWidget {
public:
    WidgetPtr child;
    FormProps props;

    Form(WidgetPtr child_, FormProps opts = {})
        : child(std::move(child_)), props(std::move(opts)) {}

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
/// TextFormField Widget
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

class TextFormField : public StatefulWidget {
public:
    TextFormFieldProps props;

    TextFormField() = default;
    explicit TextFormField(TextFormFieldProps opts) : props(std::move(opts)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "TextFormField"; }
};

/// ════════════════════════════════════════════════════════════════
/// CheckboxFormField Widget
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

class CheckboxFormField : public StatefulWidget {
public:
    CheckboxFormFieldProps props;

    CheckboxFormField() = default;
    explicit CheckboxFormField(CheckboxFormFieldProps opts) : props(std::move(opts)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "CheckboxFormField"; }
};

/// ════════════════════════════════════════════════════════════════
/// Convenience Factory Helpers
/// ════════════════════════════════════════════════════════════════

inline std::shared_ptr<Form> form(WidgetPtr child, FormProps props = {}) {
    return std::make_shared<Form>(std::move(child), std::move(props));
}

inline std::shared_ptr<TextFormField> textFormField(TextFormFieldProps props) {
    return std::make_shared<TextFormField>(std::move(props));
}

inline std::shared_ptr<CheckboxFormField> checkboxFormField(CheckboxFormFieldProps props) {
    return std::make_shared<CheckboxFormField>(std::move(props));
}

} // namespace enki
