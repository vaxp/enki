#pragma once
/// @file time_picker.hpp
/// @brief Advanced TimePicker widget for ENKI Framework (Category 3. Input / Forms).
/// Supports Digital Stepper & Columns, Clock Dial, 12h/24h formats, Seconds support,
/// Quick Presets, Input Dropdown and Inline modes, and TimePickerController.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/state/state.hpp"
#include "enki/rendering/color.hpp"

#include <string>
#include <string_view>
#include <functional>
#include <memory>
#include <vector>
#include <optional>
#include <tuple>

namespace enki {

/// ════════════════════════════════════════════════════════════════
/// Time Value Representation
/// ════════════════════════════════════════════════════════════════

struct TimeVal {
    int hour   = 8;     ///< 0 - 23 or 1 - 12
    int minute = 30;    ///< 0 - 59
    int second = 0;     ///< 0 - 59
    bool is_pm = true;  ///< For 12-hour format

    constexpr TimeVal() = default;
    constexpr TimeVal(int h, int m, int s = 0, bool pm = false)
        : hour(h), minute(m), second(s), is_pm(pm) {}

    auto tie() const { return std::tie(hour, minute, second, is_pm); }
    bool operator==(const TimeVal& o) const { return tie() == o.tie(); }
    bool operator!=(const TimeVal& o) const { return tie() != o.tie(); }

    [[nodiscard]] std::string format12h(bool show_seconds = false) const;
    [[nodiscard]] std::string format24h(bool show_seconds = false) const;
};

/// ════════════════════════════════════════════════════════════════
/// TimePicker Enums & Options
/// ════════════════════════════════════════════════════════════════

enum class TimePickerMode {
    InputPopup,     ///< Input field with dropdown popup
    Inline          ///< Self-contained embedded card
};

enum class TimeFormat {
    TwelveHour,     ///< 12-hour format with AM/PM
    TwentyFourHour  ///< 24-hour military format
};

struct TimePickerOptions {
    TimePickerMode mode = TimePickerMode::InputPopup;
    TimeFormat format = TimeFormat::TwelveHour;

    TimeVal initial_time = {8, 30, 0, true};
    bool show_seconds = false;
    int minute_step = 1;

    bool show_quick_presets = true;
    std::string placeholder = "Select time...";

    // Styling Colors
    Color background_color  = 0xFF1E293B; // Slate 800
    Color border_color      = 0xFF334155; // Slate 700
    Color active_color      = 0xFF0284C7; // Blue 600
    Color highlight_color   = 0xFF38BDF8; // Sky 400
    Color text_color        = 0xFFFFFFFF;
    Color muted_text_color  = 0xFF94A3B8;

    // Callbacks
    std::function<void(const TimeVal& time)> on_time_selected;
    std::function<void()> on_popup_opened;
    std::function<void()> on_popup_closed;
};

/// ════════════════════════════════════════════════════════════════
/// TimePicker Controller
/// ════════════════════════════════════════════════════════════════

class TimePickerController {
public:
    std::function<void(const TimeVal&)> set_time_fn;
    std::function<void()> open_fn;
    std::function<void()> close_fn;
    std::function<TimeVal()> get_time_fn;

    void setTime(const TimeVal& t) { if (set_time_fn) set_time_fn(t); }
    void open() { if (open_fn) open_fn(); }
    void close() { if (close_fn) close_fn(); }
    [[nodiscard]] TimeVal getTime() const { return get_time_fn ? get_time_fn() : TimeVal{}; }
};

/// ════════════════════════════════════════════════════════════════
/// TimePicker Widget
/// ════════════════════════════════════════════════════════════════

class TimePicker : public StatefulWidget {
public:
    TimePickerOptions options;
    std::shared_ptr<TimePickerController> controller;

    TimePicker() = default;
    explicit TimePicker(TimePickerOptions opts = {},
                        std::shared_ptr<TimePickerController> ctrl = nullptr)
        : options(std::move(opts)), controller(std::move(ctrl)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "TimePicker"; }
};

inline std::shared_ptr<TimePicker> timePicker(
    TimePickerOptions options = {},
    std::shared_ptr<TimePickerController> controller = nullptr) {
    return std::make_shared<TimePicker>(std::move(options), std::move(controller));
}

} // namespace enki
