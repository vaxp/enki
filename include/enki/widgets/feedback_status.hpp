#pragma once
/// @file feedback_status.hpp
/// @brief ENKI Section 18: Feedback & Status Extended widgets (C++20 Declarative API).
///
/// Widgets:
///   1. Skeleton — Shimmer loading placeholder with gradient sweep
///   2. Ripple — Material ink-ripple effect expanding on tap
///   3. Pulse — Concentric looping status beacon animation
///   4. CountBadge — Animated numeric badge with number transition & overflow formatting
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/rendering/color.hpp"
#include "enki/animation/curves.hpp"
#include "enki/animation/animation_controller.hpp"
#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <optional>
#include <vector>

namespace enki {

// ════════════════════════════════════════════════════════════════
// 1. Skeleton (Shimmer Loading Placeholder)
// ════════════════════════════════════════════════════════════════

enum class SkeletonShape {
    Rectangle,
    Circle
};

class SkeletonWidget : public StatefulWidget {
public:
    WidgetPtr                     child;
    bool                          enabled = true;
    Color                         base_color = 0xFF1E293B;
    Color                         highlight_color = 0xFF334155;
    std::chrono::milliseconds     duration = std::chrono::milliseconds(1200);
    std::optional<StyleValue>     width = std::nullopt;
    std::optional<StyleValue>     height = std::nullopt;
    BorderRadius                  border_radius = BorderRadius::circular(4.0f);
    SkeletonShape                 shape = SkeletonShape::Rectangle;

    SkeletonWidget(Key key = Key::none()) : StatefulWidget(std::move(key)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "Skeleton"; }
};

struct SkeletonProps {
    Key                           key = Key::none();
    WidgetPtr                     child = nullptr;
    bool                          enabled = true;
    Color                         base_color = 0xFF1E293B;
    Color                         highlight_color = 0xFF334155;
    std::chrono::milliseconds     duration = std::chrono::milliseconds(1200);
    std::optional<StyleValue>     width = std::nullopt;
    std::optional<StyleValue>     height = std::nullopt;
    BorderRadius                  border_radius = BorderRadius::circular(4.0f);
    SkeletonShape                 shape = SkeletonShape::Rectangle;
};

struct Skeleton {
    Key                           key = Key::none();
    WidgetPtr                     child = nullptr;
    bool                          enabled = true;
    Color                         base_color = 0xFF1E293B;
    Color                         highlight_color = 0xFF334155;
    std::chrono::milliseconds     duration = std::chrono::milliseconds(1200);
    std::optional<StyleValue>     width = std::nullopt;
    std::optional<StyleValue>     height = std::nullopt;
    BorderRadius                  border_radius = BorderRadius::circular(4.0f);
    SkeletonShape                 shape = SkeletonShape::Rectangle;

    operator WidgetPtr() const {
        auto w = std::make_shared<SkeletonWidget>(key);
        w->child = child;
        w->enabled = enabled;
        w->base_color = base_color;
        w->highlight_color = highlight_color;
        w->duration = duration;
        w->width = width;
        w->height = height;
        w->border_radius = border_radius;
        w->shape = shape;
        return w;
    }
};

inline WidgetPtr skeleton(const SkeletonProps& props = {}) {
    auto w = std::make_shared<SkeletonWidget>(props.key);
    w->child = props.child;
    w->enabled = props.enabled;
    w->base_color = props.base_color;
    w->highlight_color = props.highlight_color;
    w->duration = props.duration;
    w->width = props.width;
    w->height = props.height;
    w->border_radius = props.border_radius;
    w->shape = props.shape;
    return w;
}

inline WidgetPtr skeletonRect(float width, float height, float border_radius = 4.0f,
                              Color base = 0xFF1E293B, Color highlight = 0xFF334155) {
    return skeleton({
        .base_color = base,
        .highlight_color = highlight,
        .width = StyleValue::point(width),
        .height = StyleValue::point(height),
        .border_radius = BorderRadius::circular(border_radius),
        .shape = SkeletonShape::Rectangle,
    });
}

inline WidgetPtr skeletonCircle(float diameter, Color base = 0xFF1E293B, Color highlight = 0xFF334155) {
    return skeleton({
        .base_color = base,
        .highlight_color = highlight,
        .width = StyleValue::point(diameter),
        .height = StyleValue::point(diameter),
        .border_radius = BorderRadius::circular(diameter * 0.5f),
        .shape = SkeletonShape::Circle,
    });
}

inline WidgetPtr skeletonText(float width = 120.0f, float height = 14.0f,
                              Color base = 0xFF1E293B, Color highlight = 0xFF334155) {
    return skeletonRect(width, height, 4.0f, base, highlight);
}

// ════════════════════════════════════════════════════════════════
// 2. Ripple (Material Ink-Ripple Effect)
// ════════════════════════════════════════════════════════════════

class RippleWidget : public StatefulWidget {
public:
    WidgetPtr                 child;
    Color                     color = 0x33FFFFFF;
    BorderRadius              border_radius = BorderRadius::zero();
    bool                      clip_ripple = true;
    std::chrono::milliseconds duration = std::chrono::milliseconds(350);
    std::function<void()>     on_tap = nullptr;

    RippleWidget(Key key = Key::none()) : StatefulWidget(std::move(key)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "Ripple"; }
};

struct RippleProps {
    Key                       key = Key::none();
    WidgetPtr                 child = nullptr;
    Color                     color = 0x33FFFFFF;
    BorderRadius              border_radius = BorderRadius::zero();
    bool                      clip_ripple = true;
    std::chrono::milliseconds duration = std::chrono::milliseconds(350);
    std::function<void()>     on_tap = nullptr;
};

struct Ripple {
    Key                       key = Key::none();
    WidgetPtr                 child = nullptr;
    Color                     color = 0x33FFFFFF;
    BorderRadius              border_radius = BorderRadius::zero();
    bool                      clip_ripple = true;
    std::chrono::milliseconds duration = std::chrono::milliseconds(350);
    std::function<void()>     on_tap = nullptr;

    operator WidgetPtr() const {
        auto w = std::make_shared<RippleWidget>(key);
        w->child = child;
        w->color = color;
        w->border_radius = border_radius;
        w->clip_ripple = clip_ripple;
        w->duration = duration;
        w->on_tap = on_tap;
        return w;
    }
};

inline WidgetPtr ripple(const RippleProps& props) {
    auto w = std::make_shared<RippleWidget>(props.key);
    w->child = props.child;
    w->color = props.color;
    w->border_radius = props.border_radius;
    w->clip_ripple = props.clip_ripple;
    w->duration = props.duration;
    w->on_tap = props.on_tap;
    return w;
}

inline WidgetPtr ripple(WidgetPtr child, std::function<void()> on_tap = nullptr,
                        Color color = 0x33FFFFFF, BorderRadius radius = BorderRadius::zero()) {
    return ripple({
        .child = std::move(child),
        .color = color,
        .border_radius = radius,
        .on_tap = std::move(on_tap),
    });
}

// ════════════════════════════════════════════════════════════════
// 3. Pulse (Concentric Beacon / Radar Status Animation)
// ════════════════════════════════════════════════════════════════

class PulseWidget : public StatefulWidget {
public:
    WidgetPtr                 child = nullptr;
    Color                     color = 0xFF10B981;  // Default Emerald
    size_t                    ring_count = 2;
    float                     max_radius = 24.0f;
    float                     dot_radius = 6.0f;
    bool                      center_dot = true;
    std::chrono::milliseconds duration = std::chrono::milliseconds(1500);
    const Curve*              curve = &Curves::easeOut;

    PulseWidget(Key key = Key::none()) : StatefulWidget(std::move(key)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "Pulse"; }
};

struct PulseProps {
    Key                       key = Key::none();
    WidgetPtr                 child = nullptr;
    Color                     color = 0xFF10B981;
    size_t                    ring_count = 2;
    float                     max_radius = 24.0f;
    float                     dot_radius = 6.0f;
    bool                      center_dot = true;
    std::chrono::milliseconds duration = std::chrono::milliseconds(1500);
    const Curve*              curve = &Curves::easeOut;
};

struct Pulse {
    Key                       key = Key::none();
    WidgetPtr                 child = nullptr;
    Color                     color = 0xFF10B981;
    size_t                    ring_count = 2;
    float                     max_radius = 24.0f;
    float                     dot_radius = 6.0f;
    bool                      center_dot = true;
    std::chrono::milliseconds duration = std::chrono::milliseconds(1500);
    const Curve*              curve = &Curves::easeOut;

    operator WidgetPtr() const {
        auto w = std::make_shared<PulseWidget>(key);
        w->child = child;
        w->color = color;
        w->ring_count = ring_count;
        w->max_radius = max_radius;
        w->dot_radius = dot_radius;
        w->center_dot = center_dot;
        w->duration = duration;
        w->curve = curve;
        return w;
    }
};

inline WidgetPtr pulse(const PulseProps& props = {}) {
    auto w = std::make_shared<PulseWidget>(props.key);
    w->child = props.child;
    w->color = props.color;
    w->ring_count = props.ring_count;
    w->max_radius = props.max_radius;
    w->dot_radius = props.dot_radius;
    w->center_dot = props.center_dot;
    w->duration = props.duration;
    w->curve = props.curve;
    return w;
}

// ════════════════════════════════════════════════════════════════
// 4. CountBadge (Animated Numeric Badge with Spring / Overflow)
// ════════════════════════════════════════════════════════════════

class CountBadgeWidget : public StatefulWidget {
public:
    WidgetPtr                 child = nullptr;
    int                       count = 0;
    std::optional<int>        max_count = 99;
    bool                      show_zero = false;
    Color                     bg_color = 0xFFEF4444;   // Default Red
    Color                     text_color = 0xFFFFFFFF;
    float                     font_size = 11.0f;
    Alignment                 alignment = Alignment::TopRight;
    Point                     offset = {0.0f, 0.0f};
    std::chrono::milliseconds animation_duration = std::chrono::milliseconds(300);

    CountBadgeWidget(Key key = Key::none()) : StatefulWidget(std::move(key)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "CountBadge"; }
};

struct CountBadgeProps {
    Key                       key = Key::none();
    WidgetPtr                 child = nullptr;
    int                       count = 0;
    std::optional<int>        max_count = 99;
    bool                      show_zero = false;
    Color                     bg_color = 0xFFEF4444;
    Color                     text_color = 0xFFFFFFFF;
    float                     font_size = 11.0f;
    Alignment                 alignment = Alignment::TopRight;
    Point                     offset = {0.0f, 0.0f};
    std::chrono::milliseconds animation_duration = std::chrono::milliseconds(300);
};

struct CountBadge {
    Key                       key = Key::none();
    WidgetPtr                 child = nullptr;
    int                       count = 0;
    std::optional<int>        max_count = 99;
    bool                      show_zero = false;
    Color                     bg_color = 0xFFEF4444;
    Color                     text_color = 0xFFFFFFFF;
    float                     font_size = 11.0f;
    Alignment                 alignment = Alignment::TopRight;
    Point                     offset = {0.0f, 0.0f};
    std::chrono::milliseconds animation_duration = std::chrono::milliseconds(300);

    operator WidgetPtr() const {
        auto w = std::make_shared<CountBadgeWidget>(key);
        w->child = child;
        w->count = count;
        w->max_count = max_count;
        w->show_zero = show_zero;
        w->bg_color = bg_color;
        w->text_color = text_color;
        w->font_size = font_size;
        w->alignment = alignment;
        w->offset = offset;
        w->animation_duration = animation_duration;
        return w;
    }
};

inline WidgetPtr countBadge(const CountBadgeProps& props) {
    auto w = std::make_shared<CountBadgeWidget>(props.key);
    w->child = props.child;
    w->count = props.count;
    w->max_count = props.max_count;
    w->show_zero = props.show_zero;
    w->bg_color = props.bg_color;
    w->text_color = props.text_color;
    w->font_size = props.font_size;
    w->alignment = props.alignment;
    w->offset = props.offset;
    w->animation_duration = props.animation_duration;
    return w;
}

inline WidgetPtr countBadge(WidgetPtr child, int count, std::optional<int> max_count = 99,
                            Color bg_color = 0xFFEF4444) {
    return countBadge({
        .child = std::move(child),
        .count = count,
        .max_count = max_count,
        .bg_color = bg_color,
    });
}

} // namespace enki
