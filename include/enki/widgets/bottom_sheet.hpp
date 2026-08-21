#pragma once
/// @file bottom_sheet.hpp
/// @brief Advanced BottomSheet overlay widget for ENKI Framework.
/// Follows the robust container-wrapping architecture (like Drawer),
/// with AnimationController, Ticker, multi-detent snapping, drag physics,
/// scrim backdrops, and Positioned stack layout.

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
#include <algorithm>

namespace enki {

/// BottomSheet presentation type
enum class BottomSheetType {
    Modal,        ///< Overlaid above a darkened scrim backdrop
    Persistent    ///< Embedded inside the layout (e.g. mini-player / tools tray)
};

/// Snapping detents (height stages)
enum class BottomSheetDetent {
    Hidden,       ///< 0.0 fraction (dismissed)
    Peek,         ///< Mini-player / summary bar (e.g. 100px / ~0.15)
    Half,         ///< 0.50 fraction (half screen)
    Full          ///< 0.88-0.95 fraction (full screen)
};

/// ════════════════════════════════════════════════════════════════
/// BottomSheet Options
/// ════════════════════════════════════════════════════════════════

struct BottomSheetOptions {
    BottomSheetType type = BottomSheetType::Modal;
    BottomSheetDetent initial_detent = BottomSheetDetent::Half;

    bool show_drag_handle = true;
    bool show_close_button = true;
    bool enable_drag = true;
    bool close_on_overlay = true;

    float peek_height = 90.0f;
    float half_fraction = 0.50f;
    float full_fraction = 0.88f;
    float border_radius = 16.0f;

    // Styling Colors
    Color background_color = 0xFF1E293B; // Slate 800
    Color border_color     = 0xFF334155; // Slate 700
    Color handle_color     = 0xFF64748B; // Slate 500
    Color handle_hover_col = 0xFF94A3B8; // Slate 400
    Color overlay_color    = 0x99000000; // Semi-transparent black scrim
    Color title_color      = 0xFFFFFFFF; // White
    Color subtitle_color   = 0xFF94A3B8; // Slate 400

    std::string title = "";
    std::string subtitle = "";

    // Callbacks
    std::function<void()> on_opened;
    std::function<void()> on_closed;
    std::function<void(BottomSheetDetent detent)> on_detent_changed;
    std::function<void(float fraction)> on_drag_progress;
};

/// ════════════════════════════════════════════════════════════════
/// BottomSheet Controller
/// ════════════════════════════════════════════════════════════════

class BottomSheetController {
public:
    std::function<void(BottomSheetDetent)> show_fn;
    std::function<void()> hide_fn;
    std::function<void()> toggle_fn;
    std::function<void(BottomSheetDetent)> snap_fn;
    std::function<bool()> is_open_fn;
    std::function<BottomSheetDetent()> get_detent_fn;

    void show(BottomSheetDetent detent = BottomSheetDetent::Half) { if (show_fn) show_fn(detent); }
    void hide() { if (hide_fn) hide_fn(); }
    void toggle() { if (toggle_fn) toggle_fn(); }
    void snapTo(BottomSheetDetent detent) { if (snap_fn) snap_fn(detent); }
    [[nodiscard]] bool isOpen() const { return is_open_fn ? is_open_fn() : false; }
    [[nodiscard]] BottomSheetDetent getDetent() const { return get_detent_fn ? get_detent_fn() : BottomSheetDetent::Hidden; }
};

struct BottomSheetProps {
    Key key = Key::none();
    WidgetPtr sheet_content;
    WidgetPtr body;
    bool initial_open = false;
    BottomSheetOptions options;
    std::shared_ptr<BottomSheetController> controller;
};

/// ════════════════════════════════════════════════════════════════
/// BottomSheet Widget
/// ════════════════════════════════════════════════════════════════

class BottomSheet : public StatefulWidget {
public:
    WidgetPtr sheet_content;  ///< Content inside the sheet
    WidgetPtr body;           ///< Background body content
    bool initial_open = false;
    BottomSheetOptions options;
    std::shared_ptr<BottomSheetController> controller;

    BottomSheet() = default;
    BottomSheet(WidgetPtr sheet_content, WidgetPtr body, BottomSheetOptions opt = {})
        : sheet_content(std::move(sheet_content)), body(std::move(body)),
          options(std::move(opt)) {}

    // Fluent API
    BottomSheet& type(BottomSheetType t) { options.type = t; return *this; }
    BottomSheet& title(std::string t, std::string sub = "") {
        options.title = std::move(t);
        options.subtitle = std::move(sub);
        return *this;
    }
    BottomSheet& dragHandle(bool enable = true) { options.show_drag_handle = enable; return *this; }
    BottomSheet& closeButton(bool enable = true) { options.show_close_button = enable; return *this; }
    BottomSheet& borderRadius(float r) { options.border_radius = r; return *this; }
    BottomSheet& backgroundColor(Color c) { options.background_color = c; return *this; }
    BottomSheet& overlayColor(Color c) { options.overlay_color = c; return *this; }
    BottomSheet& setController(std::shared_ptr<BottomSheetController> c) {
        controller = std::move(c);
        return *this;
    }
    BottomSheet& onClosed(std::function<void()> cb) { options.on_closed = std::move(cb); return *this; }
    BottomSheet& onOpened(std::function<void()> cb) { options.on_opened = std::move(cb); return *this; }
    BottomSheet& onDetentChanged(std::function<void(BottomSheetDetent)> cb) {
        options.on_detent_changed = std::move(cb);
        return *this;
    }

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "BottomSheet"; }
};

inline std::shared_ptr<BottomSheet> bottomSheet(
    WidgetPtr sheet_content,
    WidgetPtr body,
    BottomSheetOptions options = {}) {
    return std::make_shared<BottomSheet>(std::move(sheet_content), std::move(body), std::move(options));
}

inline std::shared_ptr<BottomSheet> bottomSheet(BottomSheetProps props) {
    auto bs = std::make_shared<BottomSheet>(std::move(props.sheet_content), std::move(props.body), std::move(props.options));
    bs->key = props.key;
    bs->initial_open = props.initial_open;
    if (props.controller) {
        bs->setController(props.controller);
    }
    return bs;
}

} // namespace enki
