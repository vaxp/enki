#pragma once
/// @file color_picker.hpp
/// @brief Advanced ColorPicker widget for ENKI Framework (Category 3. Input / Forms).
/// Supports 2D Saturation-Value Canvas, Hue & Alpha sliders, HEX/RGBA/HSV formats,
/// Curated & Recent Palettes, Old/New preview, Input Dropdown and Inline modes, and ColorPickerController.
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

namespace enki {

/// ════════════════════════════════════════════════════════════════
/// ColorPicker Enums & Options
/// ════════════════════════════════════════════════════════════════

enum class ColorPickerMode {
    InputPopup,     ///< Clickable color well with dropdown popup
    Inline          ///< Self-contained embedded color inspector card
};

enum class ColorFormat {
    HEX,
    RGBA,
    HSV
};

/// ════════════════════════════════════════════════════════════════
/// ColorPicker Controller
/// ════════════════════════════════════════════════════════════════

class ColorPickerController {
public:
    std::function<void(Color)> set_color_fn;
    std::function<void()> open_fn;
    std::function<void()> close_fn;
    std::function<Color()> get_color_fn;
    std::function<std::string()> get_hex_fn;

    void setColor(Color c) { if (set_color_fn) set_color_fn(c); }
    void open() { if (open_fn) open_fn(); }
    void close() { if (close_fn) close_fn(); }
    [[nodiscard]] Color getColor() const { return get_color_fn ? get_color_fn() : 0xFFFFFFFF; }
    [[nodiscard]] std::string getHex() const { return get_hex_fn ? get_hex_fn() : "#FFFFFFFF"; }
};

/// ════════════════════════════════════════════════════════════════
/// ColorPicker Options
/// ════════════════════════════════════════════════════════════════

struct ColorPickerProps {
    std::shared_ptr<ColorPickerController> controller = nullptr;
    WidgetPtr body = nullptr;

    ColorPickerMode mode = ColorPickerMode::InputPopup;
    ColorFormat default_format = ColorFormat::HEX;

    Color initial_color = 0xFF38BDF8; // Sky 400
    bool enable_alpha = true;
    bool show_palette = true;
    bool show_comparison = true;

    std::vector<Color> palette = {
        0xFFEF4444, // Red 500
        0xFFF97316, // Orange 500
        0xFFF59E0B, // Amber 500
        0xFF10B981, // Emerald 500
        0xFF06B6D4, // Cyan 500
        0xFF38BDF8, // Sky 400
        0xFF3B82F6, // Blue 500
        0xFF6366F1, // Indigo 500
        0xFFF43F5E, // Rose 500
        0xFF000000  // Black
    };

    // Styling Colors
    Color background_color  = 0xFF1E293B; // Slate 800
    Color border_color      = 0xFF334155; // Slate 700
    Color active_color      = 0xFF0284C7; // Blue 600
    Color text_color        = 0xFFFFFFFF;
    Color muted_text_color  = 0xFF94A3B8;

    // Callbacks
    std::function<void(Color color)> on_color_changed;
    std::function<void(Color color)> on_color_submitted;
    std::function<void()> on_popup_opened;
    std::function<void()> on_popup_closed;
};

/// ════════════════════════════════════════════════════════════════
/// ColorPicker Widget Implementation
/// ════════════════════════════════════════════════════════════════

class ColorPickerWidget : public StatefulWidget {
public:
    ColorPickerProps props;
    ColorPickerWidget() = default;
    explicit ColorPickerWidget(ColorPickerProps p)
        : props(std::move(p)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "ColorPicker"; }
};

/// ════════════════════════════════════════════════════════════════
/// Declarative Proxy Struct (C++20 Designated Initializers)
/// ════════════════════════════════════════════════════════════════

struct ColorPicker {
    std::shared_ptr<ColorPickerController> controller = nullptr;
    WidgetPtr body = nullptr;
    ColorPickerMode mode = ColorPickerMode::InputPopup;
    ColorFormat default_format = ColorFormat::HEX;
    Color initial_color = 0xFF38BDF8;
    bool enable_alpha = true;
    bool show_palette = true;
    bool show_comparison = true;
    std::vector<Color> palette = {
        0xFFEF4444, 0xFFF97316, 0xFFF59E0B, 0xFF10B981, 0xFF06B6D4,
        0xFF38BDF8, 0xFF3B82F6, 0xFF6366F1, 0xFFF43F5E, 0xFF000000
    };
    Color background_color = 0xFF1E293B;
    Color border_color = 0xFF334155;
    Color active_color = 0xFF0284C7;
    Color text_color = 0xFFFFFFFF;
    Color muted_text_color = 0xFF94A3B8;
    std::function<void(Color color)> on_color_changed;
    std::function<void(Color color)> on_color_submitted;
    std::function<void()> on_popup_opened;
    std::function<void()> on_popup_closed;

    operator WidgetPtr() const {
        ColorPickerProps p;
        p.controller = controller;
        p.body = body;
        p.mode = mode;
        p.default_format = default_format;
        p.initial_color = initial_color;
        p.enable_alpha = enable_alpha;
        p.show_palette = show_palette;
        p.show_comparison = show_comparison;
        p.palette = palette;
        p.background_color = background_color;
        p.border_color = border_color;
        p.active_color = active_color;
        p.text_color = text_color;
        p.muted_text_color = muted_text_color;
        p.on_color_changed = on_color_changed;
        p.on_color_submitted = on_color_submitted;
        p.on_popup_opened = on_popup_opened;
        p.on_popup_closed = on_popup_closed;
        return std::make_shared<ColorPickerWidget>(std::move(p));
    }
};

} // namespace enki
