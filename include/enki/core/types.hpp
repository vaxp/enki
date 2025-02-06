#include <layout_engine/Anu.h>
#pragma once
/// @file types.hpp
/// @brief Core type definitions for the ENKI framework.

#include <cstdint>
#include <cstddef>
#include <cmath>
#include <string_view>
#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <optional>
#include <variant>
#include <concepts>
#include <type_traits>
#include <cassert>
#include <limits>

namespace enki {

// ============================================================
// Forward declarations
// ============================================================
class Widget;
class Element;
class RenderObject;
class State;
class BuildContext;
class Canvas;
class LayoutNode;

using WidgetPtr = std::shared_ptr<Widget>;

// ============================================================
// Geometric Types
// ============================================================

/// A 2D point with float coordinates.
struct Point {
    float x = 0.0f;
    float y = 0.0f;

    constexpr Point() = default;
    constexpr Point(float x, float y) : x(x), y(y) {}

    constexpr Point operator+(const Point& o) const { return {x + o.x, y + o.y}; }
    constexpr Point operator-(const Point& o) const { return {x - o.x, y - o.y}; }
    constexpr Point operator*(float s) const { return {x * s, y * s}; }
    constexpr bool  operator==(const Point&) const = default;

    [[nodiscard]] float distance(const Point& o) const {
        float dx = x - o.x, dy = y - o.y;
        return std::sqrt(dx * dx + dy * dy);
    }
};

/// A 2D size (width x height).
struct Size {
    float width  = 0.0f;
    float height = 0.0f;

    constexpr Size() = default;
    constexpr Size(float w, float h) : width(w), height(h) {}
    constexpr bool operator==(const Size&) const = default;

    [[nodiscard]] constexpr bool isEmpty() const {
        return width <= 0.0f || height <= 0.0f;
    }
    [[nodiscard]] constexpr float area() const { return width * height; }

    static constexpr Size zero() { return {0, 0}; }
    static constexpr Size infinite() {
        return {std::numeric_limits<float>::infinity(),
                std::numeric_limits<float>::infinity()};
    }
};

/// A 2D axis-aligned rectangle.
struct Rect {
    float x = 0.0f;
    float y = 0.0f;
    float width  = 0.0f;
    float height = 0.0f;

    constexpr Rect() = default;
    constexpr Rect(float x, float y, float w, float h)
        : x(x), y(y), width(w), height(h) {}

    static constexpr Rect fromLTWH(float l, float t, float w, float h) {
        return {l, t, w, h};
    }
    static constexpr Rect fromLTRB(float l, float t, float r, float b) {
        return {l, t, r - l, b - t};
    }
    static constexpr Rect fromPointSize(Point origin, Size size) {
        return {origin.x, origin.y, size.width, size.height};
    }

    [[nodiscard]] constexpr float left()   const { return x; }
    [[nodiscard]] constexpr float top()    const { return y; }
    [[nodiscard]] constexpr float right()  const { return x + width; }
    [[nodiscard]] constexpr float bottom() const { return y + height; }
    [[nodiscard]] constexpr Point topLeft()  const { return {x, y}; }
    [[nodiscard]] constexpr Point center()   const { return {x + width / 2, y + height / 2}; }
    [[nodiscard]] constexpr Size  size()     const { return {width, height}; }

    [[nodiscard]] constexpr bool contains(Point p) const {
        return p.x >= x && p.x < x + width &&
               p.y >= y && p.y < y + height;
    }

    [[nodiscard]] constexpr bool intersects(const Rect& o) const {
        return !(o.x >= right() || o.right() <= x ||
                 o.y >= bottom() || o.bottom() <= y);
    }

    [[nodiscard]] constexpr Rect intersect(const Rect& o) const {
        float l = std::max(x, o.x);
        float t = std::max(y, o.y);
        float r = std::min(right(), o.right());
        float b = std::min(bottom(), o.bottom());
        if (r <= l || b <= t) return {};
        return Rect::fromLTRB(l, t, r, b);
    }

    [[nodiscard]] constexpr Rect unite(const Rect& o) const {
        if (width <= 0 || height <= 0) return o;
        if (o.width <= 0 || o.height <= 0) return *this;
        float l = std::min(x, o.x);
        float t = std::min(y, o.y);
        float r = std::max(right(), o.right());
        float b = std::max(bottom(), o.bottom());
        return Rect::fromLTRB(l, t, r, b);
    }

    [[nodiscard]] constexpr Rect inflate(float d) const {
        return {x - d, y - d, width + 2 * d, height + 2 * d};
    }

    [[nodiscard]] constexpr Rect translate(float dx, float dy) const {
        return {x + dx, y + dy, width, height};
    }

    constexpr bool operator==(const Rect&) const = default;

    static constexpr Rect zero() { return {}; }
};

// ============================================================
// Color (ARGB 32-bit)
// ============================================================
using Color = uint32_t;

// ============================================================
// ID Types
// ============================================================
using Id = uint64_t;

// ============================================================
// Edge Insets (Padding / Margin)
// ============================================================
struct EdgeInsets {
    float top    = 0.0f;
    float right  = 0.0f;
    float bottom = 0.0f;
    float left   = 0.0f;
    float start  = 0.0f;
    float end    = 0.0f;

    constexpr EdgeInsets() = default;
    constexpr EdgeInsets(float top, float right, float bottom, float left)
        : top(top), right(right), bottom(bottom), left(left) {}
        
    constexpr EdgeInsets(float top, float right, float bottom, float left, float start, float end)
        : top(top), right(right), bottom(bottom), left(left), start(start), end(end) {}

    static constexpr EdgeInsets all(float v) { return {v, v, v, v}; }
    static constexpr EdgeInsets symmetric(float vertical, float horizontal) {
        return {vertical, horizontal, vertical, horizontal};
    }
    static constexpr EdgeInsets only(float t = 0, float r = 0, float b = 0, float l = 0) {
        return {t, r, b, l};
    }
    static constexpr EdgeInsets directional(float top, float bottom, float start, float end) {
        return {top, 0.0f, bottom, 0.0f, start, end};
    }

    [[nodiscard]] constexpr float horizontal() const { return left + right + start + end; }
    [[nodiscard]] constexpr float vertical()   const { return top + bottom; }

    constexpr bool operator==(const EdgeInsets&) const = default;
};

// ============================================================
// Border Radius
// ============================================================
struct BorderRadius {
    float top_left     = 0.0f;
    float top_right    = 0.0f;
    float bottom_right = 0.0f;
    float bottom_left  = 0.0f;

    constexpr BorderRadius() = default;
    explicit constexpr BorderRadius(float radius)
        : top_left(radius), top_right(radius),
          bottom_right(radius), bottom_left(radius) {}
    constexpr BorderRadius(float tl, float tr, float br, float bl)
        : top_left(tl), top_right(tr), bottom_right(br), bottom_left(bl) {}

    static constexpr BorderRadius circular(float radius) {
        return BorderRadius(radius);
    }
    static constexpr BorderRadius zero() { return {}; }

    [[nodiscard]] constexpr bool isUniform() const {
        return top_left == top_right && top_right == bottom_right &&
               bottom_right == bottom_left;
    }

    constexpr bool operator==(const BorderRadius&) const = default;
};

// ============================================================
// Border
// ============================================================
struct Border {
    Color color = 0xFF000000;
    float width = 0.0f;

    constexpr Border() = default;
    constexpr Border(Color c, float w) : color(c), width(w) {}
    constexpr bool operator==(const Border&) const = default;
};

// ============================================================
// Style Values and Units (Anu Flexbox Dimension Support)
// ============================================================

/// @brief Represents a style dimension value (Point/Pixel, Percent, Auto, Undefined).
struct StyleValue {
    enum class Unit : uint8_t {
        Undefined = 0,
        Point,
        Percent,
        Auto
    };

    Unit  unit  = Unit::Undefined;
    float value = 0.0f;

    constexpr StyleValue() = default;
    constexpr StyleValue(float val) : unit(Unit::Point), value(val) {}
    constexpr StyleValue(Unit u, float val) : unit(u), value(val) {}

    static constexpr StyleValue point(float val) { return {Unit::Point, val}; }
    static constexpr StyleValue percent(float val) { return {Unit::Percent, val}; }
    static constexpr StyleValue autoValue() { return {Unit::Auto, 0.0f}; }
    static constexpr StyleValue undefined() { return {Unit::Undefined, 0.0f}; }

    [[nodiscard]] constexpr bool isDefined() const { return unit != Unit::Undefined; }
    [[nodiscard]] constexpr bool isPoint() const { return unit == Unit::Point; }
    [[nodiscard]] constexpr bool isPercent() const { return unit == Unit::Percent; }
    [[nodiscard]] constexpr bool isAuto() const { return unit == Unit::Auto; }

    constexpr bool operator==(const StyleValue&) const = default;
};

// Literals for StyleValue
inline namespace literals {
    constexpr StyleValue operator""_px(long double val) {
        return StyleValue::point(static_cast<float>(val));
    }
    constexpr StyleValue operator""_px(unsigned long long val) {
        return StyleValue::point(static_cast<float>(val));
    }
    constexpr StyleValue operator""_pct(long double val) {
        return StyleValue::percent(static_cast<float>(val));
    }
    constexpr StyleValue operator""_pct(unsigned long long val) {
        return StyleValue::percent(static_cast<float>(val));
    }
    inline constexpr StyleValue auto_val = StyleValue::autoValue();
    inline constexpr StyleValue undefined_val = StyleValue::undefined();
}

/// @brief Style Insets using StyleValue for padding, margin, position.
struct StyleInsets {
    StyleValue top;
    StyleValue right;
    StyleValue bottom;
    StyleValue left;
    StyleValue start;
    StyleValue end;

    constexpr StyleInsets() = default;
    constexpr StyleInsets(StyleValue all)
        : top(all), right(all), bottom(all), left(all) {}
    constexpr StyleInsets(StyleValue vertical, StyleValue horizontal)
        : top(vertical), right(horizontal), bottom(vertical), left(horizontal) {}
    constexpr StyleInsets(StyleValue t, StyleValue r, StyleValue b, StyleValue l)
        : top(t), right(r), bottom(b), left(l) {}
    constexpr StyleInsets(StyleValue t, StyleValue r, StyleValue b, StyleValue l, StyleValue s, StyleValue e)
        : top(t), right(r), bottom(b), left(l), start(s), end(e) {}

    static constexpr StyleInsets all(StyleValue v) { return {v}; }
    static constexpr StyleInsets symmetric(StyleValue vertical, StyleValue horizontal) {
        return {vertical, horizontal};
    }
    static constexpr StyleInsets only(StyleValue t = {}, StyleValue r = {}, StyleValue b = {}, StyleValue l = {}) {
        return {t, r, b, l};
    }
    static constexpr StyleInsets directional(StyleValue t, StyleValue b, StyleValue s, StyleValue e) {
        return {t, {}, b, {}, s, e};
    }

    constexpr bool operator==(const StyleInsets&) const = default;
};

// ============================================================
// Enums
// ============================================================

/// Main axis alignment.
enum class MainAxisAlign { Start, Center, End, SpaceBetween, SpaceAround, SpaceEvenly };

/// Cross axis alignment.
enum class CrossAxisAlign { Auto, Start, Center, End, Stretch, Baseline };

/// Main axis sizing behavior.
enum class MainAxisSize { Min, Max };

/// Layout axis.
enum class Axis { Horizontal, Vertical };

/// Content alignment within a Stack.
enum class Alignment {
    TopLeft, TopCenter, TopRight,
    CenterLeft, Center, CenterRight,
    BottomLeft, BottomCenter, BottomRight,
};

/// Layout Direction (RTL / LTR)
enum class LayoutDirection { Inherit, LTR, RTL };

// ============================================================
// C++20 Concepts
// ============================================================

/// Constrains a type to be a valid State for Cubit/BLoC.
template<typename T>
concept StateType = std::is_copy_constructible_v<T> &&
                    std::is_default_constructible_v<T> &&
                    std::equality_comparable<T>;

/// Constrains a type to be a valid Widget.
template<typename T>
concept WidgetConcept = std::derived_from<T, Widget>;

// ============================================================
// Anu Flexbox Enums (Direct 1:1 Mapping)
// ============================================================

enum class Direction {
    Inherit = ANUDirectionInherit,
    LTR     = ANUDirectionLTR,
    RTL     = ANUDirectionRTL
};

enum class Justify {
    Start        = ANUJustifyFlexStart,
    Center       = ANUJustifyCenter,
    End          = ANUJustifyFlexEnd,
    SpaceBetween = ANUJustifySpaceBetween,
    SpaceAround  = ANUJustifySpaceAround,
    SpaceEvenly  = ANUJustifySpaceEvenly
};

enum class Align {
    Auto         = ANUAlignAuto,
    Start        = ANUAlignFlexStart,
    Center       = ANUAlignCenter,
    End          = ANUAlignFlexEnd,
    Stretch      = ANUAlignStretch,
    Baseline     = ANUAlignBaseline,
    SpaceBetween = ANUAlignSpaceBetween,
    SpaceAround  = ANUAlignSpaceAround,
    SpaceEvenly  = ANUAlignSpaceEvenly
};

enum class FlexDirection {
    Column        = ANUFlexDirectionColumn,
    ColumnReverse = ANUFlexDirectionColumnReverse,
    Row           = ANUFlexDirectionRow,
    RowReverse    = ANUFlexDirectionRowReverse
};

enum class FlexWrap {
    NoWrap      = ANUWrapNoWrap,
    Wrap        = ANUWrapWrap,
    WrapReverse = ANUWrapWrapReverse
};

enum class Edge {
    Left       = ANUEdgeLeft,
    Top        = ANUEdgeTop,
    Right      = ANUEdgeRight,
    Bottom     = ANUEdgeBottom,
    Start      = ANUEdgeStart,
    End        = ANUEdgeEnd,
    Horizontal = ANUEdgeHorizontal,
    Vertical   = ANUEdgeVertical,
    All        = ANUEdgeAll
};

enum class PositionType {
    Static   = ANUPositionTypeStatic,
    Relative = ANUPositionTypeRelative,
    Absolute = ANUPositionTypeAbsolute
};

enum class Overflow {
    Visible = ANUOverflowVisible,
    Hidden  = ANUOverflowHidden,
    Scroll  = ANUOverflowScroll
};

enum class Display {
    Flex     = ANUDisplayFlex,
    None     = ANUDisplayNone,
    Contents = ANUDisplayContents
};

enum class BoxSizing {
    BorderBox  = ANUBoxSizingBorderBox,
    ContentBox = ANUBoxSizingContentBox
};

enum class Gutter {
    Column = ANUGutterColumn,
    Row    = ANUGutterRow,
    All    = ANUGutterAll
};

}  // namespace enki

