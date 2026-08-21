#pragma once
/// @file date_picker.hpp
/// @brief Advanced DatePicker & DateRangePicker widget for ENKI Framework (Category 3. Input / Forms).
/// Supports Input Dropdown Popup and Inline modes, Single Date and Range selection,
/// Year/Month fast jump grids, quick presets chips, min/max constraints, and DatePickerController.
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
/// Date Representation
/// ════════════════════════════════════════════════════════════════

struct DateVal {
    int year  = 2026;
    int month = 8;     ///< 1 - 12
    int day   = 19;    ///< 1 - 31

    constexpr DateVal() = default;
    constexpr DateVal(int y, int m, int d) : year(y), month(m), day(d) {}

    auto tie() const { return std::tie(year, month, day); }
    bool operator==(const DateVal& o) const { return tie() == o.tie(); }
    bool operator!=(const DateVal& o) const { return tie() != o.tie(); }
    bool operator<(const DateVal& o) const { return tie() < o.tie(); }
    bool operator<=(const DateVal& o) const { return tie() <= o.tie(); }
    bool operator>(const DateVal& o) const { return tie() > o.tie(); }
    bool operator>=(const DateVal& o) const { return tie() >= o.tie(); }

    [[nodiscard]] std::string formatIso() const;
    [[nodiscard]] std::string formatFormatted() const;
};

struct DateRangeVal {
    std::optional<DateVal> start;
    std::optional<DateVal> end;

    [[nodiscard]] bool isComplete() const { return start.has_value() && end.has_value(); }
    [[nodiscard]] bool contains(const DateVal& d) const {
        if (!start.has_value() || !end.has_value()) return false;
        return d >= *start && d <= *end;
    }
};

/// ════════════════════════════════════════════════════════════════
/// DatePicker Enums & Options
/// ════════════════════════════════════════════════════════════════

enum class DatePickerMode {
    InputPopup,     ///< Input field with click-to-open dropdown popup overlay
    Inline          ///< Self-contained embedded calendar card
};

enum class DatePickerSelectionMode {
    Single,         ///< Pick single date
    Range           ///< Pick start and end dates
};

enum class DatePickerView {
    Days,           ///< 7x6 Day grid
    Months,         ///< 12-Month selector
    Years           ///< Multi-year selector
};

/// ════════════════════════════════════════════════════════════════
/// DatePicker Controller
/// ════════════════════════════════════════════════════════════════

class DatePickerController {
public:
    std::function<void(const DateVal&)> set_date_fn;
    std::function<void(const DateRangeVal&)> set_range_fn;
    std::function<void()> open_fn;
    std::function<void()> close_fn;
    std::function<void()> clear_fn;
    std::function<DateVal()> get_date_fn;
    std::function<DateRangeVal()> get_range_fn;

    void setDate(const DateVal& d) { if (set_date_fn) set_date_fn(d); }
    void setDateRange(const DateRangeVal& r) { if (set_range_fn) set_range_fn(r); }
    void open() { if (open_fn) open_fn(); }
    void close() { if (close_fn) close_fn(); }
    void clear() { if (clear_fn) clear_fn(); }
    [[nodiscard]] DateVal getSelectedDate() const { return get_date_fn ? get_date_fn() : DateVal{}; }
    [[nodiscard]] DateRangeVal getSelectedRange() const { return get_range_fn ? get_range_fn() : DateRangeVal{}; }
};

/// ════════════════════════════════════════════════════════════════
/// DatePicker Options
/// ════════════════════════════════════════════════════════════════

struct DatePickerProps {
    WidgetPtr body = nullptr;
    std::shared_ptr<DatePickerController> controller = nullptr;

    DatePickerMode mode = DatePickerMode::InputPopup;
    DatePickerSelectionMode selection_mode = DatePickerSelectionMode::Single;

    DateVal initial_date = {2026, 8, 19};
    DateRangeVal initial_range;

    std::optional<DateVal> min_date;
    std::optional<DateVal> max_date;
    bool disable_weekends = false;

    std::string placeholder = "Select a date...";
    bool show_quick_presets = true;
    bool show_action_buttons = true;

    // Styling Colors
    Color background_color  = 0xFF1E293B; // Slate 800
    Color border_color      = 0xFF334155; // Slate 700
    Color active_color      = 0xFF0284C7; // Blue 600
    Color highlight_color   = 0xFF38BDF8; // Sky 400
    Color range_fill_color  = 0x440284C7; // Translucent Blue
    Color text_color        = 0xFFFFFFFF;
    Color muted_text_color  = 0xFF94A3B8;

    // Callbacks
    std::function<void(const DateVal& date)> on_date_selected;
    std::function<void(const DateRangeVal& range)> on_range_selected;
    std::function<void()> on_popup_opened;
    std::function<void()> on_popup_closed;
    std::function<bool(const DateVal& date)> is_date_disabled;
};

/// ════════════════════════════════════════════════════════════════
/// DatePicker Widget
/// ════════════════════════════════════════════════════════════════

class DatePicker : public StatefulWidget {
public:
    DatePickerProps props;
    WidgetPtr body; ///< Workspace body layer when in InputPopup mode

    DatePicker() = default;
    explicit DatePicker(DatePickerProps p)
        : props(std::move(p)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "DatePicker"; }
};

inline std::shared_ptr<DatePicker> datePicker(DatePickerProps props) {
    return std::make_shared<DatePicker>(std::move(props));
}

inline std::shared_ptr<DatePicker> dateRangePicker(DatePickerProps props) {
    props.selection_mode = DatePickerSelectionMode::Range;
    return std::make_shared<DatePicker>(std::move(props));
}

} // namespace enki
