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

class CarouselWidget : public StatefulWidget {
public:
    std::vector<WidgetPtr> slides;
    std::shared_ptr<CarouselController> controller;

    CarouselEffect effect = CarouselEffect::Slide;

    int initial_index = 0;
    bool auto_play = true;               
    int auto_play_interval_ms = 3500;    
    bool pause_on_hover = true;          
    bool infinite_loop = true;           

    bool show_arrows = true;             
    bool show_indicators = true;         

    float height = 320.0f;               
    float border_radius = 12.0f;

    Color background_color    = 0xFF1E293B; 
    Color border_color        = 0xFF334155; 
    Color arrow_bg_color      = 0xCC0F172A; 
    Color arrow_fg_color      = 0xFFFFFFFF; 
    Color arrow_hover_bg      = 0xFF0284C7; 
    Color indicator_active    = 0xFF38BDF8; 
    Color indicator_inactive  = 0xFF475569; 

    std::function<void(int current_index)> on_page_changed;

    CarouselWidget(Key key) : StatefulWidget(std::move(key)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "Carousel"; }
};

struct Carousel {
    std::vector<WidgetPtr> slides;
    std::shared_ptr<CarouselController> controller = nullptr;

    CarouselEffect effect = CarouselEffect::Slide;

    int initial_index = 0;
    bool auto_play = true;               
    int auto_play_interval_ms = 3500;    
    bool pause_on_hover = true;          
    bool infinite_loop = true;           

    bool show_arrows = true;             
    bool show_indicators = true;         

    float height = 320.0f;               
    float border_radius = 12.0f;

    Color background_color    = 0xFF1E293B; 
    Color border_color        = 0xFF334155; 
    Color arrow_bg_color      = 0xCC0F172A; 
    Color arrow_fg_color      = 0xFFFFFFFF; 
    Color arrow_hover_bg      = 0xFF0284C7; 
    Color indicator_active    = 0xFF38BDF8; 
    Color indicator_inactive  = 0xFF475569; 

    std::function<void(int current_index)> on_page_changed = nullptr;
    Key key = Key::none();

    operator WidgetPtr() const {
        auto w = std::make_shared<CarouselWidget>(key);
        w->slides = slides;
        w->controller = controller;
        w->effect = effect;
        w->initial_index = initial_index;
        w->auto_play = auto_play;
        w->auto_play_interval_ms = auto_play_interval_ms;
        w->pause_on_hover = pause_on_hover;
        w->infinite_loop = infinite_loop;
        w->show_arrows = show_arrows;
        w->show_indicators = show_indicators;
        w->height = height;
        w->border_radius = border_radius;
        w->background_color = background_color;
        w->border_color = border_color;
        w->arrow_bg_color = arrow_bg_color;
        w->arrow_fg_color = arrow_fg_color;
        w->arrow_hover_bg = arrow_hover_bg;
        w->indicator_active = indicator_active;
        w->indicator_inactive = indicator_inactive;
        w->on_page_changed = on_page_changed;
        return w;
    }
};

} // namespace enki
