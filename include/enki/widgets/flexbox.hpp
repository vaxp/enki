#pragma once
/// @file flexbox.hpp
/// @brief Comprehensive Flexbox widget supporting 100% of Anu Layout Engine properties.
///
/// Features:
///   - 100% direct delegation to Anu Layout Engine (Zero Calculation Tampering).
///   - All container and item flex properties (Direction, Wrap, Justify, Align, Gap, Insets, Constraints).
///   - Row, Column, FlexItem, Expanded, Flexible, Spacer semantic helpers.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/tree/render_object.hpp"
#include <layout_engine/Anu.h>
#include <optional>
#include <vector>
#include <memory>
#include <string_view>

namespace enki {

// ════════════════════════════════════════════════════════════════
// FlexboxStyle — Complete layout configuration struct
// ════════════════════════════════════════════════════════════════

/// @brief Border width specification per edge.
struct StyleBorders {
    float left       = 0.0f;
    float top        = 0.0f;
    float right      = 0.0f;
    float bottom     = 0.0f;
    float start      = 0.0f;
    float end        = 0.0f;
    float horizontal = 0.0f;
    float vertical   = 0.0f;
    float all        = 0.0f;

    constexpr StyleBorders() = default;
    static constexpr StyleBorders uniform(float width) {
        StyleBorders b;
        b.all = width;
        return b;
    }
    static constexpr StyleBorders symmetric(float vertical, float horizontal) {
        StyleBorders b;
        b.vertical = vertical;
        b.horizontal = horizontal;
        return b;
    }
    static constexpr StyleBorders only(float t = 0, float r = 0, float b = 0, float l = 0) {
        StyleBorders borders;
        borders.top = t;
        borders.right = r;
        borders.bottom = b;
        borders.left = l;
        return borders;
    }

    constexpr bool operator==(const StyleBorders&) const = default;
};

/// @brief Comprehensive style definition mapping 1:1 to Anu Layout Engine.
struct FlexboxStyle {
    // ── Direction & Flow ───────────────────────────────────────
    std::optional<Direction>     direction;
    std::optional<FlexDirection> flex_direction;
    std::optional<FlexWrap>      flex_wrap;
    std::optional<Display>       display;
    std::optional<Overflow>      overflow;
    std::optional<BoxSizing>     box_sizing;
    std::optional<PositionType>  position_type;

    // ── Alignment ──────────────────────────────────────────────
    std::optional<Justify>       justify_content;
    std::optional<Align>         align_items;
    std::optional<Align>         align_self;
    std::optional<Align>         align_content;

    // ── Flex Factors ───────────────────────────────────────────
    std::optional<float>         flex;
    std::optional<float>         flex_grow;
    std::optional<float>         flex_shrink;
    StyleValue                   flex_basis;

    // ── Gaps / Gutters ─────────────────────────────────────────
    StyleValue                   gap;
    StyleValue                   row_gap;
    StyleValue                   column_gap;

    // ── Dimensions & Constraints ───────────────────────────────
    StyleValue                   width;
    StyleValue                   height;
    StyleValue                   min_width;
    StyleValue                   min_height;
    StyleValue                   max_width;
    StyleValue                   max_height;
    std::optional<float>         aspect_ratio;

    // ── Insets (Padding, Margin, Position) ─────────────────────
    StyleInsets                  padding;
    StyleInsets                  margin;
    StyleInsets                  position;

    // ── Border Widths ──────────────────────────────────────────
    StyleBorders                 border;

    constexpr bool operator==(const FlexboxStyle&) const = default;
};

/// @brief Applies all fields of FlexboxStyle directly to an ANUNodeRef without modification.
void applyFlexboxStyle(ANUNodeRef node, const FlexboxStyle& style);

// ════════════════════════════════════════════════════════════════
// RenderFlex — Flexbox Container Render Object
// ════════════════════════════════════════════════════════════════

class RenderFlex : public RenderBox {
public:
    RenderFlex() = default;
    explicit RenderFlex(FlexboxStyle style);
    ~RenderFlex() override = default;

    void setStyle(const FlexboxStyle& style);
    [[nodiscard]] const FlexboxStyle& style() const { return style_; }

    void paint(PaintContext& context) override;
    bool hitTestChildren(HitTestResult& result, Point localPoint) override;

private:
    FlexboxStyle style_;
};

// ════════════════════════════════════════════════════════════════
// RenderFlexItem — Individual Flex Item Render Object
// ════════════════════════════════════════════════════════════════

class RenderFlexItem : public RenderBox {
public:
    RenderFlexItem() = default;
    explicit RenderFlexItem(FlexboxStyle style);
    ~RenderFlexItem() override = default;

    void setStyle(const FlexboxStyle& style);
    [[nodiscard]] const FlexboxStyle& style() const { return style_; }

    void paint(PaintContext& context) override;
    bool hitTestChildren(HitTestResult& result, Point localPoint) override;

private:
    FlexboxStyle style_;
};

// ════════════════════════════════════════════════════════════════
// Declarative Props (C++20 Designated Initializers Support)
// ════════════════════════════════════════════════════════════════

struct FlexboxProps {
    std::optional<Direction>     direction;
    std::optional<FlexDirection> flex_direction;
    std::optional<FlexWrap>      flex_wrap;
    std::optional<Display>       display;
    std::optional<Overflow>      overflow;
    std::optional<BoxSizing>     box_sizing;
    std::optional<PositionType>  position_type;
    std::optional<Justify>       justify_content;
    std::optional<Align>         align_items;
    std::optional<Align>         align_self;
    std::optional<Align>         align_content;
    std::optional<float>         flex;
    std::optional<float>         flex_grow;
    std::optional<float>         flex_shrink;
    std::optional<StyleValue>    flex_basis;
    std::optional<StyleValue>    gap;
    std::optional<StyleValue>    row_gap;
    std::optional<StyleValue>    column_gap;
    std::optional<StyleValue>    width;
    std::optional<StyleValue>    height;
    std::optional<StyleValue>    min_width;
    std::optional<StyleValue>    min_height;
    std::optional<StyleValue>    max_width;
    std::optional<StyleValue>    max_height;
    std::optional<float>         aspect_ratio;
    std::optional<StyleInsets>   padding;
    std::optional<StyleInsets>   margin;
    std::optional<StyleInsets>   position;
    std::optional<StyleBorders>  border;

    std::vector<WidgetPtr>       children;
    Key                          key = Key::none();

    FlexboxStyle extractStyle() const {
        FlexboxStyle s;
        if (direction) s.direction = direction;
        if (flex_direction) s.flex_direction = flex_direction;
        if (flex_wrap) s.flex_wrap = flex_wrap;
        if (display) s.display = display;
        if (overflow) s.overflow = overflow;
        if (box_sizing) s.box_sizing = box_sizing;
        if (position_type) s.position_type = position_type;
        if (justify_content) s.justify_content = justify_content;
        if (align_items) s.align_items = align_items;
        if (align_self) s.align_self = align_self;
        if (align_content) s.align_content = align_content;
        if (flex) s.flex = flex;
        if (flex_grow) s.flex_grow = flex_grow;
        if (flex_shrink) s.flex_shrink = flex_shrink;
        if (flex_basis) s.flex_basis = *flex_basis;
        if (gap) s.gap = *gap;
        if (row_gap) s.row_gap = *row_gap;
        if (column_gap) s.column_gap = *column_gap;
        if (width) s.width = *width;
        if (height) s.height = *height;
        if (min_width) s.min_width = *min_width;
        if (min_height) s.min_height = *min_height;
        if (max_width) s.max_width = *max_width;
        if (max_height) s.max_height = *max_height;
        if (aspect_ratio) s.aspect_ratio = aspect_ratio;
        if (padding) s.padding = *padding;
        if (margin) s.margin = *margin;
        if (position) s.position = *position;
        if (border) s.border = *border;
        return s;
    }
};

using RowProps = FlexboxProps;
using ColumnProps = FlexboxProps;
using WrapProps = FlexboxProps;

struct FlexItemProps {
    std::optional<Align>         align_self;
    std::optional<float>         flex;
    std::optional<float>         flex_grow;
    std::optional<float>         flex_shrink;
    std::optional<StyleValue>    flex_basis;
    std::optional<PositionType>  position_type;
    std::optional<StyleValue>    width;
    std::optional<StyleValue>    height;
    std::optional<StyleValue>    min_width;
    std::optional<StyleValue>    min_height;
    std::optional<StyleValue>    max_width;
    std::optional<StyleValue>    max_height;
    std::optional<float>         aspect_ratio;
    std::optional<StyleInsets>   padding;
    std::optional<StyleInsets>   margin;
    std::optional<StyleInsets>   position;

    WidgetPtr child = nullptr;
    Key key = Key::none();

    FlexboxStyle extractStyle() const {
        FlexboxStyle s;
        if (align_self) s.align_self = align_self;
        if (flex) s.flex = flex;
        if (flex_grow) s.flex_grow = flex_grow;
        if (flex_shrink) s.flex_shrink = flex_shrink;
        if (flex_basis) s.flex_basis = *flex_basis;
        if (position_type) s.position_type = position_type;
        if (width) s.width = *width;
        if (height) s.height = *height;
        if (min_width) s.min_width = *min_width;
        if (min_height) s.min_height = *min_height;
        if (max_width) s.max_width = *max_width;
        if (max_height) s.max_height = *max_height;
        if (aspect_ratio) s.aspect_ratio = aspect_ratio;
        if (padding) s.padding = *padding;
        if (margin) s.margin = *margin;
        if (position) s.position = *position;
        return s;
    }
};

struct ExpandedProps {
    float flex = 1.0f;
    WidgetPtr child = nullptr;
};

// ════════════════════════════════════════════════════════════════
// Flexbox Widget — Pure Declarative Multi-child Container
// ════════════════════════════════════════════════════════════════

class Flexbox : public MultiChildRenderObjectWidget {
public:
    FlexboxStyle style;

    Flexbox() = default;
    explicit Flexbox(const FlexboxProps& props)
        : MultiChildRenderObjectWidget(props.key, props.children), style(props.extractStyle()) {}
    explicit Flexbox(FlexboxProps&& props)
        : MultiChildRenderObjectWidget(props.key, std::move(props.children)), style(props.extractStyle()) {}
    Flexbox(Key k, FlexboxStyle s, std::vector<WidgetPtr> children)
        : MultiChildRenderObjectWidget(std::move(k), std::move(children)), style(std::move(s)) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
    [[nodiscard]] std::string_view typeName() const override { return "Flexbox"; }
};

// ════════════════════════════════════════════════════════════════
// Row & Column Semantic Aliases
// ════════════════════════════════════════════════════════════════

class Row : public Flexbox {
public:
    Row() { style.flex_direction = FlexDirection::Row; }
    explicit Row(const FlexboxProps& props) : Flexbox(props) {
        style.flex_direction = FlexDirection::Row;
    }
    explicit Row(FlexboxProps&& props) : Flexbox(std::move(props)) {
        style.flex_direction = FlexDirection::Row;
    }
    Row(Key k, FlexboxStyle s, std::vector<WidgetPtr> children)
        : Flexbox(std::move(k), std::move(s), std::move(children)) {
        style.flex_direction = FlexDirection::Row;
    }
    [[nodiscard]] std::string_view typeName() const override { return "Row"; }
};

class Column : public Flexbox {
public:
    Column() { style.flex_direction = FlexDirection::Column; }
    explicit Column(const FlexboxProps& props) : Flexbox(props) {
        style.flex_direction = FlexDirection::Column;
    }
    explicit Column(FlexboxProps&& props) : Flexbox(std::move(props)) {
        style.flex_direction = FlexDirection::Column;
    }
    Column(Key k, FlexboxStyle s, std::vector<WidgetPtr> children)
        : Flexbox(std::move(k), std::move(s), std::move(children)) {
        style.flex_direction = FlexDirection::Column;
    }
    [[nodiscard]] std::string_view typeName() const override { return "Column"; }
};

class Wrap : public Flexbox {
public:
    Wrap() {
        style.flex_direction = FlexDirection::Row;
        style.flex_wrap = FlexWrap::Wrap;
    }
    explicit Wrap(const FlexboxProps& props) : Flexbox(props) {
        style.flex_direction = FlexDirection::Row;
        style.flex_wrap = FlexWrap::Wrap;
    }
    explicit Wrap(FlexboxProps&& props) : Flexbox(std::move(props)) {
        style.flex_direction = FlexDirection::Row;
        style.flex_wrap = FlexWrap::Wrap;
    }
    Wrap(Key k, FlexboxStyle s, std::vector<WidgetPtr> children)
        : Flexbox(std::move(k), std::move(s), std::move(children)) {
        style.flex_direction = FlexDirection::Row;
        style.flex_wrap = FlexWrap::Wrap;
    }
    [[nodiscard]] std::string_view typeName() const override { return "Wrap"; }
};

// ════════════════════════════════════════════════════════════════
// FlexItem — Single child flex modifier
// ════════════════════════════════════════════════════════════════

class FlexItem : public SingleChildRenderObjectWidget {
public:
    FlexboxStyle style;

    FlexItem() = default;
    explicit FlexItem(const FlexItemProps& props)
        : SingleChildRenderObjectWidget(props.key, props.child), style(props.extractStyle()) {}
    explicit FlexItem(FlexItemProps&& props)
        : SingleChildRenderObjectWidget(props.key, std::move(props.child)), style(props.extractStyle()) {}
    FlexItem(Key k, FlexboxStyle s, WidgetPtr child)
        : SingleChildRenderObjectWidget(std::move(k), std::move(child)), style(std::move(s)) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
    [[nodiscard]] std::string_view typeName() const override { return "FlexItem"; }
};

// ════════════════════════════════════════════════════════════════
// Factory Functions
// ════════════════════════════════════════════════════════════════

inline std::shared_ptr<Flexbox> flexbox(const FlexboxProps& props = {}) {
    return std::make_shared<Flexbox>(props);
}
inline std::shared_ptr<Flexbox> flexbox(FlexboxProps&& props) {
    return std::make_shared<Flexbox>(std::move(props));
}
inline std::shared_ptr<Flexbox> flexbox(Key key, FlexboxProps props) {
    props.key = std::move(key);
    return std::make_shared<Flexbox>(std::move(props));
}

inline std::shared_ptr<Row> row(const FlexboxProps& props = {}) {
    return std::make_shared<Row>(props);
}
inline std::shared_ptr<Row> row(FlexboxProps&& props) {
    return std::make_shared<Row>(std::move(props));
}
inline std::shared_ptr<Row> row(Key key, FlexboxProps props) {
    props.key = std::move(key);
    return std::make_shared<Row>(std::move(props));
}

inline std::shared_ptr<Column> column(const FlexboxProps& props = {}) {
    return std::make_shared<Column>(props);
}
inline std::shared_ptr<Column> column(FlexboxProps&& props) {
    return std::make_shared<Column>(std::move(props));
}
inline std::shared_ptr<Column> column(Key key, FlexboxProps props) {
    props.key = std::move(key);
    return std::make_shared<Column>(std::move(props));
}

inline std::shared_ptr<Wrap> wrap(const FlexboxProps& props = {}) {
    return std::make_shared<Wrap>(props);
}
inline std::shared_ptr<Wrap> wrap(FlexboxProps&& props) {
    return std::make_shared<Wrap>(std::move(props));
}
inline std::shared_ptr<Wrap> wrap(Key key, FlexboxProps props) {
    props.key = std::move(key);
    return std::make_shared<Wrap>(std::move(props));
}

inline std::shared_ptr<FlexItem> flexItem(const FlexItemProps& props = {}) {
    return std::make_shared<FlexItem>(props);
}
inline std::shared_ptr<FlexItem> flexItem(FlexItemProps&& props) {
    return std::make_shared<FlexItem>(std::move(props));
}
inline std::shared_ptr<FlexItem> flexItem(Key key, FlexItemProps props) {
    props.key = std::move(key);
    return std::make_shared<FlexItem>(std::move(props));
}

inline std::shared_ptr<FlexItem> expanded(const ExpandedProps& props = {}) {
    FlexboxStyle s;
    s.flex_grow = props.flex;
    s.flex_shrink = 1.0f;
    s.flex_basis = StyleValue::point(0.0f);
    return std::make_shared<FlexItem>(Key::none(), s, props.child);
}
inline std::shared_ptr<FlexItem> expanded(ExpandedProps&& props) {
    FlexboxStyle s;
    s.flex_grow = props.flex;
    s.flex_shrink = 1.0f;
    s.flex_basis = StyleValue::point(0.0f);
    return std::make_shared<FlexItem>(Key::none(), s, std::move(props.child));
}
inline std::shared_ptr<FlexItem> expanded(WidgetPtr child, float flex = 1.0f) {
    return expanded(ExpandedProps{ .flex = flex, .child = std::move(child) });
}

inline std::shared_ptr<FlexItem> flexible(const ExpandedProps& props = {}) {
    FlexboxStyle s;
    s.flex_grow = props.flex;
    s.flex_shrink = 1.0f;
    s.flex_basis = StyleValue::autoValue();
    return std::make_shared<FlexItem>(Key::none(), s, props.child);
}
inline std::shared_ptr<FlexItem> flexible(ExpandedProps&& props) {
    FlexboxStyle s;
    s.flex_grow = props.flex;
    s.flex_shrink = 1.0f;
    s.flex_basis = StyleValue::autoValue();
    return std::make_shared<FlexItem>(Key::none(), s, std::move(props.child));
}
inline std::shared_ptr<FlexItem> flexible(WidgetPtr child, float flex = 1.0f) {
    return flexible(ExpandedProps{ .flex = flex, .child = std::move(child) });
}

inline std::shared_ptr<FlexItem> spacer(float flex = 1.0f) {
    FlexboxStyle s;
    s.flex_grow = flex;
    s.flex_shrink = 1.0f;
    s.flex_basis = StyleValue::point(0.0f);
    return std::make_shared<FlexItem>(Key::none(), s, nullptr);
}

} // namespace enki

