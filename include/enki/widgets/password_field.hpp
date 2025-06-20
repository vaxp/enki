#pragma once
/// @file password_field.hpp
/// @brief Advanced PasswordField widget for ENKI Framework.
/// Supports obscurity masking, eye reveal/peek, live strength meter, CapsLock detection,
/// criteria checklist validation, and cryptographically strong password generation.

#include "enki/widgets/text.hpp"
#include "enki/state/state.hpp"
#include "enki/rendering/color.hpp"
#include "enki/core/types.hpp"

#include <string>
#include <string_view>
#include <functional>
#include <memory>
#include <vector>
#include <random>

namespace enki {

/// Password strength rating level
enum class PasswordStrengthLevel {
    Empty,       ///< No text entered
    VeryWeak,    ///< Red: very low entropy (< 25)
    Weak,        ///< Orange: low entropy (25-45)
    Medium,      ///< Yellow: moderate entropy (45-65)
    Strong,      ///< Green: good entropy (65-85)
    VeryStrong   ///< Emerald: excellent entropy (> 85)
};

/// Validation rules criteria checklist
struct PasswordValidationRules {
    size_t min_length = 8;
    bool require_uppercase = true;
    bool require_lowercase = true;
    bool require_digit = true;
    bool require_special = true;

    [[nodiscard]] bool checkMinLength(std::string_view p) const { return p.length() >= min_length; }
    [[nodiscard]] bool checkUppercase(std::string_view p) const;
    [[nodiscard]] bool checkLowercase(std::string_view p) const;
    [[nodiscard]] bool checkDigit(std::string_view p) const;
    [[nodiscard]] bool checkSpecial(std::string_view p) const;
    [[nodiscard]] bool meetsAll(std::string_view p) const;
};

/// ════════════════════════════════════════════════════════════════
/// PasswordField Controller
/// ════════════════════════════════════════════════════════════════

class PasswordFieldController {
private:
    std::string password_ = "";
    bool is_obscured_ = true;
    PasswordValidationRules rules_;

public:
    PasswordFieldController(std::string initial_password = "", PasswordValidationRules rules = {})
        : password_(std::move(initial_password)), rules_(rules) {}

    [[nodiscard]] const std::string& getPassword() const { return password_; }
    void setPassword(std::string_view p) { password_ = std::string(p); }

    [[nodiscard]] bool isObscured() const { return is_obscured_; }
    void setObscured(bool obscured) { is_obscured_ = obscured; }
    void toggleObscured() { is_obscured_ = !is_obscured_; }

    [[nodiscard]] const PasswordValidationRules& getRules() const { return rules_; }
    void setRules(const PasswordValidationRules& rules) { rules_ = rules; }

    [[nodiscard]] PasswordStrengthLevel calculateStrength() const;
    [[nodiscard]] double calculateEntropy() const;
    [[nodiscard]] bool meetsAllRules() const { return rules_.meetsAll(password_); }

    void generateStrongPassword(size_t length = 16, bool use_symbols = true);
    void clear() { password_.clear(); }
};

/// ════════════════════════════════════════════════════════════════
/// Configuration Options for PasswordField
/// ════════════════════════════════════════════════════════════════

struct PasswordFieldOptions {
    std::string placeholder = "Enter password...";
    std::string mask_char = "•"; // Default bullet glyph

    bool show_lock_icon = true;
    bool show_visibility_toggle = true;
    bool show_clear_button = false;
    bool show_generator_button = false;
    bool show_strength_meter = false;
    bool show_rules_checklist = false;
    bool show_capslock_warning = true;
    bool hold_to_peek = false; // Reveal while mouse is pressed on eye

    bool auto_focus = false;
    bool read_only = false;

    // Styling
    TextStyle style;
    Color background_color   = 0xFF0F172A; // Slate 900
    Color border_color       = 0xFF334155; // Slate 700
    Color focus_border_color = 0xFF38BDF8; // Sky 400
    Color icon_color         = 0xFF94A3B8; // Slate 400
    Color placeholder_color  = 0xFF64748B; // Slate 500
    Color cursor_color       = 0xFF38BDF8; // Sky 400
    Color selection_color    = 0x4D38BDF8; // Sky 400 alpha
    Color warning_color      = 0xFFF59E0B; // Amber 500 for CapsLock

    float border_radius = 8.0f;
    EdgeInsets padding = EdgeInsets::symmetric(8.0f, 12.0f);

    // Callbacks
    std::function<void(std::string_view password)> on_changed;
    std::function<void(std::string_view password)> on_submitted;
    std::function<void(PasswordStrengthLevel strength)> on_strength_changed;
};

/// ════════════════════════════════════════════════════════════════
/// PasswordField Widget
/// ════════════════════════════════════════════════════════════════

class PasswordField : public StatefulWidget {
public:
    std::shared_ptr<PasswordFieldController> controller;
    PasswordFieldOptions options;

    PasswordField(std::shared_ptr<PasswordFieldController> ctrl, PasswordFieldOptions opt = {})
        : controller(ctrl ? ctrl : std::make_shared<PasswordFieldController>()),
          options(std::move(opt)) {}

    PasswordField(PasswordFieldOptions opt = {})
        : controller(std::make_shared<PasswordFieldController>()),
          options(std::move(opt)) {}

    // Fluent API Chaining
    PasswordField* placeholder(std::string p) { options.placeholder = std::move(p); return this; }
    PasswordField* strengthMeter(bool show = true) { options.show_strength_meter = show; return this; }
    PasswordField* rulesChecklist(bool show = true) { options.show_rules_checklist = show; return this; }
    PasswordField* generatorButton(bool show = true) { options.show_generator_button = show; return this; }
    PasswordField* clearButton(bool show = true) { options.show_clear_button = show; return this; }
    PasswordField* holdToPeek(bool enable = true) { options.hold_to_peek = enable; return this; }
    PasswordField* capslockWarning(bool enable = true) { options.show_capslock_warning = enable; return this; }
    PasswordField* rules(PasswordValidationRules r) { controller->setRules(r); return this; }
    PasswordField* onChanged(std::function<void(std::string_view)> cb) { options.on_changed = std::move(cb); return this; }
    PasswordField* onSubmitted(std::function<void(std::string_view)> cb) { options.on_submitted = std::move(cb); return this; }

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "PasswordField"; }
};

inline std::shared_ptr<PasswordField> passwordField(
    std::shared_ptr<PasswordFieldController> ctrl,
    PasswordFieldOptions options = {}) {
    return std::make_shared<PasswordField>(std::move(ctrl), std::move(options));
}

inline std::shared_ptr<PasswordField> passwordField(
    PasswordFieldOptions options = {}) {
    return std::make_shared<PasswordField>(std::move(options));
}

} // namespace enki
