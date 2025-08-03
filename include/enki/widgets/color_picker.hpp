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

struct ColorPickerOptions {
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
/// ColorPicker Widget
/// ════════════════════════════════════════════════════════════════

class ColorPicker : public StatefulWidget {
public:
    ColorPickerOptions options;
    std::shared_ptr<ColorPickerController> controller;

    ColorPicker() = default;
    explicit ColorPicker(ColorPickerOptions opts = {},
                         std::shared_ptr<ColorPickerController> ctrl = nullptr)
        : options(std::move(opts)), controller(std::move(ctrl)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "ColorPicker"; }
};

inline std::shared_ptr<ColorPicker> colorPicker(
    ColorPickerOptions options = {},
    std::shared_ptr<ColorPickerController> controller = nullptr) {
    return std::make_shared<ColorPicker>(std::move(options), std::move(controller));
}

} // namespace enki
