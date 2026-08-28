#pragma once
/// @file motion.hpp
/// @brief Animation & Motion widgets (ENKI Roadmap Section 13):
///   - AnimatedOpacity
///   - AnimatedContainer
///   - AnimatedScale
///   - AnimatedRotation
///   - AnimatedSlide
///   - AnimatedSwitcher
///   - SlideTransition
///
/// 100% C++20 Declarative Syntax with designated initializers.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/widgets/container.hpp"
#include "enki/animation/curves.hpp"
#include "enki/animation/animation_controller.hpp"
#include "enki/animation/tween.hpp"
#include <chrono>
#include <functional>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

namespace enki {

// ════════════════════════════════════════════════════════════════
// 1. AnimatedOpacity
// ════════════════════════════════════════════════════════════════

class AnimatedOpacityWidget : public StatefulWidget {
public:
    float                     opacity = 1.0f;
    std::chrono::milliseconds duration = std::chrono::milliseconds(300);
    const Curve*              curve = &Curves::linear;
    std::function<void()>     on_end = nullptr;
    WidgetPtr                 child = nullptr;

    AnimatedOpacityWidget() = default;
    explicit AnimatedOpacityWidget(Key key) : StatefulWidget(std::move(key)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "AnimatedOpacity"; }
};

struct AnimatedOpacity {
    float                     opacity = 1.0f;
    std::chrono::milliseconds duration = std::chrono::milliseconds(300);
    const Curve*              curve = &Curves::linear;
    std::function<void()>     on_end = nullptr;
    WidgetPtr                 child = nullptr;
    Key                       key = Key::none();

    operator WidgetPtr() const;
};

inline std::shared_ptr<AnimatedOpacityWidget> animatedOpacity(AnimatedOpacity props = {}) {
    auto w = std::make_shared<AnimatedOpacityWidget>(props.key);
    w->opacity = props.opacity;
    w->duration = props.duration;
    w->curve = props.curve;
    w->on_end = std::move(props.on_end);
    w->child = std::move(props.child);
    return w;
}

// ════════════════════════════════════════════════════════════════
// 2. AnimatedContainer
// ════════════════════════════════════════════════════════════════

class AnimatedContainerWidget : public StatefulWidget {
public:
    // Decoration
    std::optional<Color>          color;
    std::optional<GradientConfig> gradient;
    std::optional<BorderRadius>   border_radius;
    std::optional<Border>         border;
    std::vector<BoxShadow>        box_shadow;
    std::optional<BoxShape>       shape;
    std::optional<bool>           clip_content;

    // Alignment
    std::optional<Alignment>      align;

    // Dimensions & Constraints
    std::optional<StyleValue>     width;
    std::optional<StyleValue>     height;
    std::optional<StyleValue>     min_width;
    std::optional<StyleValue>     min_height;
    std::optional<StyleValue>     max_width;
    std::optional<StyleValue>     max_height;
    std::optional<float>          aspect_ratio;

    // Insets
    std::optional<StyleInsets>    padding;
    std::optional<StyleInsets>    margin;

    // Animation props
    std::chrono::milliseconds duration = std::chrono::milliseconds(300);
    const Curve*              curve = &Curves::linear;
    std::function<void()>     on_end = nullptr;
    WidgetPtr                 child = nullptr;

    AnimatedContainerWidget() = default;
    explicit AnimatedContainerWidget(Key key) : StatefulWidget(std::move(key)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "AnimatedContainer"; }
};

struct AnimatedContainer {
    // Visual Decoration
    std::optional<Color>          color;
    std::optional<GradientConfig> gradient;
    std::optional<BorderRadius>   border_radius;
    std::optional<Border>         border;
    std::vector<BoxShadow>        box_shadow;
    std::optional<BoxShape>       shape;
    std::optional<bool>           clip_content;

    // Child Alignment
    std::optional<Alignment>      align;

    // Dimensions & Constraints
    std::optional<StyleValue>     width;
    std::optional<StyleValue>     height;
    std::optional<StyleValue>     min_width;
    std::optional<StyleValue>     min_height;
    std::optional<StyleValue>     max_width;
    std::optional<StyleValue>     max_height;
    std::optional<float>          aspect_ratio;

    // Insets
    std::optional<StyleInsets>    padding;
    std::optional<StyleInsets>    margin;

    // Animation
    std::chrono::milliseconds duration = std::chrono::milliseconds(300);
    const Curve*              curve = &Curves::linear;
    std::function<void()>     on_end = nullptr;
    WidgetPtr                 child = nullptr;
    Key                       key = Key::none();

    operator WidgetPtr() const;
};

inline std::shared_ptr<AnimatedContainerWidget> animatedContainer(AnimatedContainer props = {}) {
    auto w = std::make_shared<AnimatedContainerWidget>(props.key);
    w->color = props.color;
    w->gradient = std::move(props.gradient);
    w->border_radius = props.border_radius;
    w->border = props.border;
    w->box_shadow = std::move(props.box_shadow);
    w->shape = props.shape;
    w->clip_content = props.clip_content;
    w->align = props.align;
    w->width = props.width;
    w->height = props.height;
    w->min_width = props.min_width;
    w->min_height = props.min_height;
    w->max_width = props.max_width;
    w->max_height = props.max_height;
    w->aspect_ratio = props.aspect_ratio;
    w->padding = props.padding;
    w->margin = props.margin;
    w->duration = props.duration;
    w->curve = props.curve;
    w->on_end = std::move(props.on_end);
    w->child = std::move(props.child);
    return w;
}

// ════════════════════════════════════════════════════════════════
// 3. AnimatedScale
// ════════════════════════════════════════════════════════════════

class AnimatedScaleWidget : public StatefulWidget {
public:
    float                     scale = 1.0f;
    Alignment                 alignment = Alignment::Center;
    std::chrono::milliseconds duration = std::chrono::milliseconds(300);
    const Curve*              curve = &Curves::linear;
    std::function<void()>     on_end = nullptr;
    WidgetPtr                 child = nullptr;

    AnimatedScaleWidget() = default;
    explicit AnimatedScaleWidget(Key key) : StatefulWidget(std::move(key)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "AnimatedScale"; }
};

struct AnimatedScale {
    float                     scale = 1.0f;
    Alignment                 alignment = Alignment::Center;
    std::chrono::milliseconds duration = std::chrono::milliseconds(300);
    const Curve*              curve = &Curves::linear;
    std::function<void()>     on_end = nullptr;
    WidgetPtr                 child = nullptr;
    Key                       key = Key::none();

    operator WidgetPtr() const;
};

inline std::shared_ptr<AnimatedScaleWidget> animatedScale(AnimatedScale props = {}) {
    auto w = std::make_shared<AnimatedScaleWidget>(props.key);
    w->scale = props.scale;
    w->alignment = props.alignment;
    w->duration = props.duration;
    w->curve = props.curve;
    w->on_end = std::move(props.on_end);
    w->child = std::move(props.child);
    return w;
}

// ════════════════════════════════════════════════════════════════
// 4. AnimatedRotation
// ════════════════════════════════════════════════════════════════

class AnimatedRotationWidget : public StatefulWidget {
public:
    float                     turns = 0.0f;
    Alignment                 alignment = Alignment::Center;
    std::chrono::milliseconds duration = std::chrono::milliseconds(300);
    const Curve*              curve = &Curves::linear;
    std::function<void()>     on_end = nullptr;
    WidgetPtr                 child = nullptr;

    AnimatedRotationWidget() = default;
    explicit AnimatedRotationWidget(Key key) : StatefulWidget(std::move(key)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "AnimatedRotation"; }
};

struct AnimatedRotation {
    float                     turns = 0.0f;
    Alignment                 alignment = Alignment::Center;
    std::chrono::milliseconds duration = std::chrono::milliseconds(300);
    const Curve*              curve = &Curves::linear;
    std::function<void()>     on_end = nullptr;
    WidgetPtr                 child = nullptr;
    Key                       key = Key::none();

    operator WidgetPtr() const;
};

inline std::shared_ptr<AnimatedRotationWidget> animatedRotation(AnimatedRotation props = {}) {
    auto w = std::make_shared<AnimatedRotationWidget>(props.key);
    w->turns = props.turns;
    w->alignment = props.alignment;
    w->duration = props.duration;
    w->curve = props.curve;
    w->on_end = std::move(props.on_end);
    w->child = std::move(props.child);
    return w;
}

// ════════════════════════════════════════════════════════════════
// 5. AnimatedSlide
// ════════════════════════════════════════════════════════════════

class AnimatedSlideWidget : public StatefulWidget {
public:
    Point                     offset = {0.0f, 0.0f};
    std::chrono::milliseconds duration = std::chrono::milliseconds(300);
    const Curve*              curve = &Curves::linear;
    std::function<void()>     on_end = nullptr;
    WidgetPtr                 child = nullptr;

    AnimatedSlideWidget() = default;
    explicit AnimatedSlideWidget(Key key) : StatefulWidget(std::move(key)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "AnimatedSlide"; }
};

struct AnimatedSlide {
    Point                     offset = {0.0f, 0.0f};
    std::chrono::milliseconds duration = std::chrono::milliseconds(300);
    const Curve*              curve = &Curves::linear;
    std::function<void()>     on_end = nullptr;
    WidgetPtr                 child = nullptr;
    Key                       key = Key::none();

    operator WidgetPtr() const;
};

inline std::shared_ptr<AnimatedSlideWidget> animatedSlide(AnimatedSlide props = {}) {
    auto w = std::make_shared<AnimatedSlideWidget>(props.key);
    w->offset = props.offset;
    w->duration = props.duration;
    w->curve = props.curve;
    w->on_end = std::move(props.on_end);
    w->child = std::move(props.child);
    return w;
}

// ════════════════════════════════════════════════════════════════
// 6. AnimatedSwitcher
// ════════════════════════════════════════════════════════════════

using TransitionBuilder = std::function<WidgetPtr(WidgetPtr child, float animation_progress)>;
using LayoutBuilder = std::function<WidgetPtr(WidgetPtr current_child, const std::vector<WidgetPtr>& previous_children)>;

class AnimatedSwitcherWidget : public StatefulWidget {
public:
    WidgetPtr                                child = nullptr;
    std::chrono::milliseconds                duration = std::chrono::milliseconds(300);
    std::optional<std::chrono::milliseconds> reverse_duration;
    const Curve*                             switch_in_curve = &Curves::linear;
    const Curve*                             switch_out_curve = &Curves::linear;
    TransitionBuilder                        transition_builder = nullptr;
    LayoutBuilder                            layout_builder = nullptr;

    AnimatedSwitcherWidget() = default;
    explicit AnimatedSwitcherWidget(Key key) : StatefulWidget(std::move(key)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "AnimatedSwitcher"; }
};

struct AnimatedSwitcher {
    WidgetPtr                                child = nullptr;
    std::chrono::milliseconds                duration = std::chrono::milliseconds(300);
    std::optional<std::chrono::milliseconds> reverse_duration;
    const Curve*                             switch_in_curve = &Curves::linear;
    const Curve*                             switch_out_curve = &Curves::linear;
    TransitionBuilder                        transition_builder = nullptr;
    LayoutBuilder                            layout_builder = nullptr;
    Key                                      key = Key::none();

    operator WidgetPtr() const;
};

inline std::shared_ptr<AnimatedSwitcherWidget> animatedSwitcher(AnimatedSwitcher props = {}) {
    auto w = std::make_shared<AnimatedSwitcherWidget>(props.key);
    w->child = std::move(props.child);
    w->duration = props.duration;
    w->reverse_duration = props.reverse_duration;
    w->switch_in_curve = props.switch_in_curve;
    w->switch_out_curve = props.switch_out_curve;
    w->transition_builder = std::move(props.transition_builder);
    w->layout_builder = std::move(props.layout_builder);
    return w;
}

// ════════════════════════════════════════════════════════════════
// 7. SlideTransition
// ════════════════════════════════════════════════════════════════

class SlideTransitionWidget : public SingleChildRenderObjectWidget {
public:
    std::shared_ptr<AnimationController> position = nullptr;
    Point                                begin = {0.0f, 0.0f};
    Point                                end = {1.0f, 0.0f};
    const Curve*                         curve = &Curves::linear;
    bool                                 transform_hit_tests = true;

    SlideTransitionWidget() = default;
    SlideTransitionWidget(Key key, WidgetPtr child,
                          std::shared_ptr<AnimationController> pos,
                          Point beg = {0.0f, 0.0f}, Point en = {1.0f, 0.0f},
                          const Curve* c = &Curves::linear,
                          bool hit_test = true)
        : SingleChildRenderObjectWidget(std::move(key), std::move(child)),
          position(std::move(pos)), begin(beg), end(en), curve(c),
          transform_hit_tests(hit_test) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
    [[nodiscard]] std::string_view typeName() const override { return "SlideTransition"; }
};

struct SlideTransition {
    std::shared_ptr<AnimationController> position = nullptr;
    Point                                begin = {0.0f, 0.0f};
    Point                                end = {1.0f, 0.0f};
    const Curve*                         curve = &Curves::linear;
    bool                                 transform_hit_tests = true;
    WidgetPtr                            child = nullptr;
    Key                                  key = Key::none();

    operator WidgetPtr() const {
        return std::make_shared<SlideTransitionWidget>(key, child, position, begin, end, curve, transform_hit_tests);
    }
};

inline std::shared_ptr<SlideTransitionWidget> slideTransition(const SlideTransition& props) {
    return std::make_shared<SlideTransitionWidget>(
        props.key, props.child, props.position, props.begin, props.end, props.curve, props.transform_hit_tests
    );
}

// ── Inline operator WidgetPtr() implementations ──────────────────

inline AnimatedOpacity::operator WidgetPtr() const {
    return animatedOpacity(*this);
}

inline AnimatedContainer::operator WidgetPtr() const {
    return animatedContainer(*this);
}

inline AnimatedScale::operator WidgetPtr() const {
    return animatedScale(*this);
}

inline AnimatedRotation::operator WidgetPtr() const {
    return animatedRotation(*this);
}

inline AnimatedSlide::operator WidgetPtr() const {
    return animatedSlide(*this);
}

inline AnimatedSwitcher::operator WidgetPtr() const {
    return animatedSwitcher(*this);
}

} // namespace enki
