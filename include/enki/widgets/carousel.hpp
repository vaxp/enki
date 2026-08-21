#pragma once
/// @file carousel.hpp
/// @brief Advanced Carousel / Swiper Slider widget for ENKI Framework (Category 10. Advanced / Data UI).
/// Supports autoplay with pause-on-hover, infinite looping, swipe gestures,
/// floating navigation arrows, animated dot indicators, and CarouselController.
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

namespace enki {

/// Visual transition style
enum class CarouselEffect {
    Slide,      ///< Standard horizontal sliding
    Fade        ///< Smooth cross-fade
};

/// ════════════════════════════════════════════════════════════════
/// Carousel Options
/// ════════════════════════════════════════════════════════════════

class CarouselController;

struct CarouselProps {
    Key key = Key::none();
    std::vector<WidgetPtr> slides;
    std::shared_ptr<CarouselController> controller;

    CarouselEffect effect = CarouselEffect::Slide;

    int initial_index = 0;
    bool auto_play = true;               ///< Automatically advance slides
    int auto_play_interval_ms = 3500;    ///< Interval between auto advances
    bool pause_on_hover = true;          ///< Freeze auto-play timer on mouse hover
    bool infinite_loop = true;           ///< Loop back to start after last slide

    bool show_arrows = true;             ///< Render floating Previous / Next buttons
    bool show_indicators = true;         ///< Render bottom pagination dots

    float height = 320.0f;               ///< Default carousel height in pixels
    float border_radius = 12.0f;

    // Styling Colors
    Color background_color    = 0xFF1E293B; // Slate 800
    Color border_color        = 0xFF334155; // Slate 700
    Color arrow_bg_color      = 0xCC0F172A; // Dark slate 80% opacity
    Color arrow_fg_color      = 0xFFFFFFFF; // White
    Color arrow_hover_bg      = 0xFF0284C7; // Blue highlight
    Color indicator_active    = 0xFF38BDF8; // Sky 400
    Color indicator_inactive  = 0xFF475569; // Slate 600

    // Callbacks
    std::function<void(int current_index)> on_page_changed;
};

/// ════════════════════════════════════════════════════════════════
/// Carousel Controller
/// ════════════════════════════════════════════════════════════════

class CarouselController {
public:
    std::function<void()> next_page_fn;
    std::function<void()> prev_page_fn;
    std::function<void(int)> jump_to_page_fn;
    std::function<void(bool)> set_auto_play_fn;
    std::function<int()> get_current_page_fn;
    std::function<int()> get_page_count_fn;

    void nextPage() { if (next_page_fn) next_page_fn(); }
    void previousPage() { if (prev_page_fn) prev_page_fn(); }
    void jumpToPage(int idx) { if (jump_to_page_fn) jump_to_page_fn(idx); }
    void setAutoPlay(bool play) { if (set_auto_play_fn) set_auto_play_fn(play); }
    [[nodiscard]] int getCurrentPage() const { return get_current_page_fn ? get_current_page_fn() : 0; }
    [[nodiscard]] int getPageCount() const { return get_page_count_fn ? get_page_count_fn() : 0; }
};

/// ════════════════════════════════════════════════════════════════
/// Carousel Widget
/// ════════════════════════════════════════════════════════════════

class Carousel : public StatefulWidget {
public:
    CarouselProps props;

    Carousel() = default;
    explicit Carousel(CarouselProps p) : props(std::move(p)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "Carousel"; }
};

inline std::shared_ptr<Carousel> carousel(CarouselProps props = {}) {
    return std::make_shared<Carousel>(std::move(props));
}

inline std::shared_ptr<Carousel> carousel(
    std::vector<WidgetPtr> slides) {
    CarouselProps props;
    props.slides = std::move(slides);
    return std::make_shared<Carousel>(std::move(props));
}

} // namespace enki
