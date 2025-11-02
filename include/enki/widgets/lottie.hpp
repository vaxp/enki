#pragma once
/// @file lottie.hpp
/// @brief Declarative Lottie animation widget for ENKI (C++20 designated initializers).
///
/// Features:
///   - Automatic caching via LottieCache for high-efficiency rendering.
///   - Full BoxFit & Alignment integration (Contain, Cover, FitWidth, etc.).
///   - Intrinsic sizing & aspect-ratio measurement with Anu Flexbox.
///   - Built-in & custom LottieController lifecycle management.
///   - Dynamic marker & segment playback.
///   - Interactive hover and tap animation triggers.
///   - Geometric clipping (BorderRadius & Circle).
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/rendering/lottie_composition.hpp"
#include "enki/animation/lottie_controller.hpp"
#include <string>
#include <string_view>
#include <memory>
#include <optional>
#include <functional>

namespace enki {

// ════════════════════════════════════════════════════════════════
// LottieStyle — Configuration for Lottie widget & RenderLottie
// ════════════════════════════════════════════════════════════════

struct LottieStyle {
    std::shared_ptr<LottieComposition> composition;
    std::string                        asset_path;
    std::string                        json_data;
    std::shared_ptr<LottieController>  controller;

    std::optional<StyleValue>          width;
    std::optional<StyleValue>          height;
    std::optional<StyleValue>          min_width;
    std::optional<StyleValue>          min_height;
    std::optional<StyleValue>          max_width;
    std::optional<StyleValue>          max_height;

    BoxFit                             fit            = BoxFit::Contain;
    Alignment                          alignment      = Alignment::Center;
    BorderRadius                       border_radius  = BorderRadius::zero();
    BoxShape                           shape          = BoxShape::Rectangle;

    float                              opacity        = 1.0f;
    bool                               clip_content   = true;
    bool                               auto_play      = true;
    bool                               repeat         = true;
    float                              speed          = 1.0f;
    std::string                        marker;

    bool                               animate_on_hover = false;
    bool                               animate_on_tap   = false;

    std::function<void()>              on_end;
    std::function<void(int)>           on_loop;
};

// ════════════════════════════════════════════════════════════════
// LottieWidget — Stateful Widget for Declarative Lottie Rendering
// ════════════════════════════════════════════════════════════════

class LottieWidget : public StatefulWidget {
public:
    LottieStyle style;

    LottieWidget() = default;
    explicit LottieWidget(Key key) : StatefulWidget(std::move(key)) {}
    explicit LottieWidget(LottieStyle style, Key key = Key::none())
        : StatefulWidget(std::move(key)), style(std::move(style)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "Lottie"; }
};

// ════════════════════════════════════════════════════════════════
// LottieProps & Declarative Factory
// ════════════════════════════════════════════════════════════════

struct LottieProps {
    Key                                key = Key::none();
    std::shared_ptr<LottieComposition> composition = nullptr;
    std::string                        asset;
    std::string                        json_data;
    std::shared_ptr<LottieController>  controller = nullptr;

    std::optional<StyleValue>          width = std::nullopt;
    std::optional<StyleValue>          height = std::nullopt;
    std::optional<StyleValue>          min_width = std::nullopt;
    std::optional<StyleValue>          min_height = std::nullopt;
    std::optional<StyleValue>          max_width = std::nullopt;
    std::optional<StyleValue>          max_height = std::nullopt;

    BoxFit                             fit = BoxFit::Contain;
    Alignment                          alignment = Alignment::Center;
    BorderRadius                       border_radius = BorderRadius::zero();
    BoxShape                           shape = BoxShape::Rectangle;

    float                              opacity = 1.0f;
    bool                               clip_content = true;
    bool                               auto_play = true;
    bool                               repeat = true;
    float                              speed = 1.0f;
    std::string                        marker;

    bool                               animate_on_hover = false;
    bool                               animate_on_tap = false;

    std::function<void()>              on_end = nullptr;
    std::function<void(int)>           on_loop = nullptr;

    operator WidgetPtr() const;
};

inline WidgetPtr lottie(const LottieProps& props = {}) {
    return static_cast<WidgetPtr>(props);
}

} // namespace enki
