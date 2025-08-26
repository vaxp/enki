#pragma once
/// @file number_field.hpp
/// @brief Advanced NumberField widget for ENKI Framework.
/// Supports precision, stepping, mouse scrubbing, expressions, steppers, and prefixes/suffixes.

#include "enki/widgets/text.hpp"
#include "enki/state/state.hpp"
#include "enki/rendering/color.hpp"
#include "enki/core/types.hpp"

#include <string>
#include <functional>
#include <memory>
#include <vector>
#include <limits>
#include <cmath>

namespace enki {

/// Stepper button layout positions
enum class NumberFieldStepperPosition {
    RightVertical,   ///< Stacked ▲ / ▼ arrows on the right
    Sides,           ///< − on the left, + on the right
    RightHorizontal, ///< − + side-by-side on the right
    None             ///< Clean input without stepper buttons
};

/// Predefined visual size presets
enum class NumberFieldSize {
    Small,   ///< Compact height ~32px
    Medium,  ///< Standard height ~40px
    Large    ///< Spacious height ~48px
};

/// Range overflow wrapping behavior
enum class NumberFieldWrapMode {
    Clamp,   ///< Clamp strictly to [min_value, max_value]
    Wrap     ///< Wrap around to min when exceeding max and vice versa
};

/// ════════════════════════════════════════════════════════════════
/// NumberField Controller
/// ════════════════════════════════════════════════════════════════

class NumberFieldController {
private:
    double value_ = 0.0;
    std::vector<double> undo_stack_;
    std::vector<double> redo_stack_;
    static constexpr size_t kMaxHistory = 50;

public:
    NumberFieldController(double initial_val = 0.0) : value_(initial_val) {}

    [[nodiscard]] double getValue() const { return value_; }

    void setValue(double val, bool save_undo = true) {
        if (save_undo && std::abs(value_ - val) > 1e-9) {
            undo_stack_.push_back(value_);
            if (undo_stack_.size() > kMaxHistory) {
                undo_stack_.erase(undo_stack_.begin());
            }
            redo_stack_.clear();
        }
        value_ = val;
    }

    [[nodiscard]] bool canUndo() const { return !undo_stack_.empty(); }
    [[nodiscard]] bool canRedo() const { return !redo_stack_.empty(); }

    bool undo() {
        if (undo_stack_.empty()) return false;
        redo_stack_.push_back(value_);
        value_ = undo_stack_.back();
        undo_stack_.pop_back();
        return true;
    }

    bool redo() {
        if (redo_stack_.empty()) return false;
        undo_stack_.push_back(value_);
        value_ = redo_stack_.back();
        redo_stack_.pop_back();
        return true;
    }
};

/// ════════════════════════════════════════════════════════════════
/// Configuration Options for NumberField
/// ════════════════════════════════════════════════════════════════

struct NumberFieldProps {
    Key key = Key::none();
    std::shared_ptr<NumberFieldController> controller = nullptr;
    double initial_value = 0.0;

    // Value Range & Step
    double min_value = -std::numeric_limits<double>::infinity();
    double max_value = std::numeric_limits<double>::infinity();
    double step = 1.0;
    double large_step = 10.0;  // Used with Shift / PageUp / PageDown
    double fine_step = 0.1;    // Used with Alt or fine drag
    int precision = 0;         // 0 for integer mode, > 0 for decimals, -1 for auto

    bool allow_decimals = true;
    bool allow_negative = true;
    bool allow_expressions = true;   // Evaluate expressions like "100 + 50", "1920 / 2"
    bool enable_scrubbing = true;     // Horizontal drag-to-adjust
    bool enable_auto_repeat = true;   // Auto-repeat stepping on hold
    bool show_thousands_separator = false;

    NumberFieldStepperPosition stepper_position = NumberFieldStepperPosition::RightVertical;
    NumberFieldSize size = NumberFieldSize::Medium;
    NumberFieldWrapMode wrap_mode = NumberFieldWrapMode::Clamp;

    // Prefixes & Suffixes
    std::string prefix_text = ""; // e.g. "$ ", "px "
    std::string suffix_text = ""; // e.g. " px", " %", " MB", " deg"

    // Styling
    TextStyle style;
    Color background_color   = 0xFF0F172A; // Slate 900
    Color border_color       = 0xFF334155; // Slate 700
    Color focus_border_color = 0xFF38BDF8; // Sky 400
    Color button_color       = 0xFF1E293B; // Slate 800
    Color button_hover_color = 0xFF334155; // Slate 700
    Color button_icon_color  = 0xFF94A3B8; // Slate 400
    Color prefix_suffix_color= 0xFF64748B; // Slate 500
    Color cursor_color       = 0xFF38BDF8; // Sky 400
    Color selection_color    = 0x4D38BDF8; // Sky 400 alpha

    float border_radius = 8.0f;
    EdgeInsets padding = EdgeInsets::symmetric(8.0f, 10.0f);

    bool read_only = false;
    bool disabled = false;
    bool auto_focus = false;

    // Callbacks
    std::function<void(double)> on_changed = nullptr;
    std::function<void(double)> on_submitted = nullptr;
    std::function<std::string(double)> custom_formatter = nullptr;
};

/// ════════════════════════════════════════════════════════════════
/// NumberField Widget Implementation
/// ════════════════════════════════════════════════════════════════

class NumberFieldWidget : public StatefulWidget {
public:
    std::shared_ptr<NumberFieldController> controller;
    NumberFieldProps options;

    NumberFieldWidget(std::shared_ptr<NumberFieldController> ctrl, NumberFieldProps opt = {})
        : controller(ctrl ? ctrl : std::make_shared<NumberFieldController>(0.0)),
          options(std::move(opt)) {}

    NumberFieldWidget(double initial_val, NumberFieldProps opt = {})
        : controller(std::make_shared<NumberFieldController>(initial_val)),
          options(std::move(opt)) {}

    NumberFieldWidget(Key key, std::shared_ptr<NumberFieldController> ctrl, double initial_val, NumberFieldProps opt)
        : StatefulWidget(std::move(key)),
          controller(ctrl ? std::move(ctrl) : std::make_shared<NumberFieldController>(initial_val)),
          options(std::move(opt)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "NumberField"; }
};

/// ════════════════════════════════════════════════════════════════
/// Declarative Proxy Struct (C++20 Designated Initializers)
/// ════════════════════════════════════════════════════════════════

struct NumberField {
    Key key = Key::none();
    std::shared_ptr<NumberFieldController> controller = nullptr;
    double initial_value = 0.0;

    double min_value = -std::numeric_limits<double>::infinity();
    double max_value = std::numeric_limits<double>::infinity();
    double step = 1.0;
    double large_step = 10.0;
    double fine_step = 0.1;
    int precision = 0;

    bool allow_decimals = true;
    bool allow_negative = true;
    bool allow_expressions = true;
    bool enable_scrubbing = true;
    bool enable_auto_repeat = true;
    bool show_thousands_separator = false;

    NumberFieldStepperPosition stepper_position = NumberFieldStepperPosition::RightVertical;
    NumberFieldSize size = NumberFieldSize::Medium;
    NumberFieldWrapMode wrap_mode = NumberFieldWrapMode::Clamp;

    std::string prefix_text = "";
    std::string suffix_text = "";

    TextStyle style;
    Color background_color   = 0xFF0F172A;
    Color border_color       = 0xFF334155;
    Color focus_border_color = 0xFF38BDF8;
    Color button_color       = 0xFF1E293B;
    Color button_hover_color = 0xFF334155;
    Color button_icon_color  = 0xFF94A3B8;
    Color prefix_suffix_color= 0xFF64748B;
    Color cursor_color       = 0xFF38BDF8;
    Color selection_color    = 0x4D38BDF8;

    float border_radius = 8.0f;
    EdgeInsets padding = EdgeInsets::symmetric(8.0f, 10.0f);

    bool read_only = false;
    bool disabled = false;
    bool auto_focus = false;

    std::function<void(double)> on_changed = nullptr;
    std::function<void(double)> on_submitted = nullptr;
    std::function<std::string(double)> custom_formatter = nullptr;

    operator WidgetPtr() const {
        NumberFieldProps opts;
        opts.key = key;
        opts.controller = controller;
        opts.initial_value = initial_value;
        opts.min_value = min_value;
        opts.max_value = max_value;
        opts.step = step;
        opts.large_step = large_step;
        opts.fine_step = fine_step;
        opts.precision = precision;
        opts.allow_decimals = allow_decimals;
        opts.allow_negative = allow_negative;
        opts.allow_expressions = allow_expressions;
        opts.enable_scrubbing = enable_scrubbing;
        opts.enable_auto_repeat = enable_auto_repeat;
        opts.show_thousands_separator = show_thousands_separator;
        opts.stepper_position = stepper_position;
        opts.size = size;
        opts.wrap_mode = wrap_mode;
        opts.prefix_text = prefix_text;
        opts.suffix_text = suffix_text;
        opts.style = style;
        opts.background_color = background_color;
        opts.border_color = border_color;
        opts.focus_border_color = focus_border_color;
        opts.button_color = button_color;
        opts.button_hover_color = button_hover_color;
        opts.button_icon_color = button_icon_color;
        opts.prefix_suffix_color = prefix_suffix_color;
        opts.cursor_color = cursor_color;
        opts.selection_color = selection_color;
        opts.border_radius = border_radius;
        opts.padding = padding;
        opts.read_only = read_only;
        opts.disabled = disabled;
        opts.auto_focus = auto_focus;
        opts.on_changed = on_changed;
        opts.on_submitted = on_submitted;
        opts.custom_formatter = custom_formatter;

        return std::make_shared<NumberFieldWidget>(key, controller, initial_value, std::move(opts));
    }
};

} // namespace enki
