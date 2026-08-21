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
// Flexbox Widget — Declarative Container
// ════════════════════════════════════════════════════════════════

class Flexbox : public MultiChildRenderObjectWidget {
public:
    FlexboxStyle style;

    Flexbox() = default;
    explicit Flexbox(std::vector<WidgetPtr> children)
        : MultiChildRenderObjectWidget(Key::none(), std::move(children)) {}
    Flexbox(FlexboxStyle s, std::vector<WidgetPtr> children)
        : MultiChildRenderObjectWidget(Key::none(), std::move(children)), style(std::move(s)) {}
    Flexbox(Key k, FlexboxStyle s, std::vector<WidgetPtr> children)
        : MultiChildRenderObjectWidget(std::move(k), std::move(children)), style(std::move(s)) {}

    // ── Fluent Builder API ─────────────────────────────────────
    Flexbox& direction(Direction dir) { style.direction = dir; return *this; }
    Flexbox& flexDirection(FlexDirection fd) { style.flex_direction = fd; return *this; }
    Flexbox& justifyContent(Justify jc) { style.justify_content = jc; return *this; }
    Flexbox& alignItems(Align ai) { style.align_items = ai; return *this; }
    Flexbox& alignSelf(Align as) { style.align_self = as; return *this; }
    Flexbox& alignContent(Align ac) { style.align_content = ac; return *this; }
    Flexbox& flexWrap(FlexWrap fw) { style.flex_wrap = fw; return *this; }
    Flexbox& display(Display d) { style.display = d; return *this; }
    Flexbox& overflow(Overflow o) { style.overflow = o; return *this; }
    Flexbox& boxSizing(BoxSizing bs) { style.box_sizing = bs; return *this; }
    Flexbox& positionType(PositionType pt) { style.position_type = pt; return *this; }

    Flexbox& flex(float f) { style.flex = f; return *this; }
    Flexbox& flexGrow(float fg) { style.flex_grow = fg; return *this; }
    Flexbox& flexShrink(float fs) { style.flex_shrink = fs; return *this; }
    Flexbox& flexBasis(StyleValue fb) { style.flex_basis = fb; return *this; }

    Flexbox& gap(StyleValue g) { style.gap = g; return *this; }
    Flexbox& rowGap(StyleValue rg) { style.row_gap = rg; return *this; }
    Flexbox& columnGap(StyleValue cg) { style.column_gap = cg; return *this; }

    Flexbox& width(StyleValue w) { style.width = w; return *this; }
    Flexbox& height(StyleValue h) { style.height = h; return *this; }
    Flexbox& minWidth(StyleValue mw) { style.min_width = mw; return *this; }
    Flexbox& minHeight(StyleValue mh) { style.min_height = mh; return *this; }
    Flexbox& maxWidth(StyleValue mw) { style.max_width = mw; return *this; }
    Flexbox& maxHeight(StyleValue mh) { style.max_height = mh; return *this; }
    Flexbox& aspectRatio(float ar) { style.aspect_ratio = ar; return *this; }

    Flexbox& padding(StyleInsets p) { style.padding = p; return *this; }
    Flexbox& margin(StyleInsets m) { style.margin = m; return *this; }
    Flexbox& position(StyleInsets p) { style.position = p; return *this; }
    Flexbox& border(StyleBorders b) { style.border = b; return *this; }

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
    explicit Row(std::vector<WidgetPtr> children) : Flexbox(std::move(children)) {
        style.flex_direction = FlexDirection::Row;
    }
    Row(Justify justify, Align align, std::vector<WidgetPtr> children)
        : Flexbox(std::move(children)) {
        style.flex_direction = FlexDirection::Row;
        style.justify_content = justify;
        style.align_items = align;
    }
    Row(Key k, std::vector<WidgetPtr> children) : Flexbox(std::move(k), {}, std::move(children)) {
        style.flex_direction = FlexDirection::Row;
    }
    [[nodiscard]] std::string_view typeName() const override { return "Row"; }
};

class Column : public Flexbox {
public:
    Column() { style.flex_direction = FlexDirection::Column; }
    explicit Column(std::vector<WidgetPtr> children) : Flexbox(std::move(children)) {
        style.flex_direction = FlexDirection::Column;
    }
    Column(Justify justify, Align align, std::vector<WidgetPtr> children)
        : Flexbox(std::move(children)) {
        style.flex_direction = FlexDirection::Column;
        style.justify_content = justify;
        style.align_items = align;
    }
    Column(Key k, std::vector<WidgetPtr> children) : Flexbox(std::move(k), {}, std::move(children)) {
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
    explicit Wrap(std::vector<WidgetPtr> children) : Flexbox(std::move(children)) {
        style.flex_direction = FlexDirection::Row;
        style.flex_wrap = FlexWrap::Wrap;
    }
    Wrap(Justify justify, Align align, std::vector<WidgetPtr> children)
        : Flexbox(std::move(children)) {
        style.flex_direction = FlexDirection::Row;
        style.flex_wrap = FlexWrap::Wrap;
        style.justify_content = justify;
        style.align_items = align;
    }
    Wrap(Key k, std::vector<WidgetPtr> children) : Flexbox(std::move(k), {}, std::move(children)) {
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
    explicit FlexItem(WidgetPtr child) : SingleChildRenderObjectWidget(Key::none(), std::move(child)) {}
    FlexItem(FlexboxStyle s, WidgetPtr child)
        : SingleChildRenderObjectWidget(Key::none(), std::move(child)), style(std::move(s)) {}
    FlexItem(Key k, FlexboxStyle s, WidgetPtr child)
        : SingleChildRenderObjectWidget(std::move(k), std::move(child)), style(std::move(s)) {}

    // ── Fluent Builder API ─────────────────────────────────────
    FlexItem& flex(float f) { style.flex = f; return *this; }
    FlexItem& flexGrow(float fg) { style.flex_grow = fg; return *this; }
    FlexItem& flexShrink(float fs) { style.flex_shrink = fs; return *this; }
    FlexItem& flexBasis(StyleValue fb) { style.flex_basis = fb; return *this; }
    FlexItem& alignSelf(Align as) { style.align_self = as; return *this; }
    FlexItem& positionType(PositionType pt) { style.position_type = pt; return *this; }

    FlexItem& width(StyleValue w) { style.width = w; return *this; }
    FlexItem& height(StyleValue h) { style.height = h; return *this; }
    FlexItem& minWidth(StyleValue mw) { style.min_width = mw; return *this; }
    FlexItem& minHeight(StyleValue mh) { style.min_height = mh; return *this; }
    FlexItem& maxWidth(StyleValue mw) { style.max_width = mw; return *this; }
    FlexItem& maxHeight(StyleValue mh) { style.max_height = mh; return *this; }
    FlexItem& aspectRatio(float ar) { style.aspect_ratio = ar; return *this; }

    FlexItem& margin(StyleInsets m) { style.margin = m; return *this; }
    FlexItem& padding(StyleInsets p) { style.padding = p; return *this; }
    FlexItem& position(StyleInsets p) { style.position = p; return *this; }

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
    [[nodiscard]] std::string_view typeName() const override { return "FlexItem"; }
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
// Factory Functions
// ════════════════════════════════════════════════════════════════

inline std::shared_ptr<Flexbox> flexbox(std::vector<WidgetPtr> children) {
    return std::make_shared<Flexbox>(std::move(children));
}

inline std::shared_ptr<Flexbox> flexbox(std::initializer_list<WidgetPtr> children) {
    return std::make_shared<Flexbox>(std::vector<WidgetPtr>(children));
}

inline std::shared_ptr<Flexbox> flexbox(FlexboxStyle style, std::vector<WidgetPtr> children) {
    return std::make_shared<Flexbox>(std::move(style), std::move(children));
}

inline std::shared_ptr<Flexbox> flexbox(FlexboxStyle style, std::initializer_list<WidgetPtr> children) {
    return std::make_shared<Flexbox>(std::move(style), std::vector<WidgetPtr>(children));
}

inline std::shared_ptr<Flexbox> flexbox(FlexboxProps props) {
    return std::make_shared<Flexbox>(props.key, props.extractStyle(), std::move(props.children));
}

inline std::shared_ptr<Row> row(std::vector<WidgetPtr> children) {
    return std::make_shared<Row>(std::move(children));
}

inline std::shared_ptr<Row> row(std::initializer_list<WidgetPtr> children) {
    return std::make_shared<Row>(std::vector<WidgetPtr>(children));
}

inline std::shared_ptr<Row> row(Key key, std::vector<WidgetPtr> children) {
    return std::make_shared<Row>(std::move(key), std::move(children));
}

inline std::shared_ptr<Row> row(Key key, std::initializer_list<WidgetPtr> children) {
    return std::make_shared<Row>(std::move(key), std::vector<WidgetPtr>(children));
}

inline std::shared_ptr<Row> row(Justify justify, Align align, std::vector<WidgetPtr> children) {
    return std::make_shared<Row>(justify, align, std::move(children));
}

inline std::shared_ptr<Row> row(Justify justify, Align align, std::initializer_list<WidgetPtr> children) {
    return std::make_shared<Row>(justify, align, std::vector<WidgetPtr>(children));
}

inline std::shared_ptr<Row> row(FlexboxProps props) {
    auto r = std::make_shared<Row>(props.key, std::move(props.children));
    r->style = props.extractStyle();
    r->style.flex_direction = FlexDirection::Row;
    return r;
}

inline std::shared_ptr<Column> column(std::vector<WidgetPtr> children) {
    return std::make_shared<Column>(std::move(children));
}

inline std::shared_ptr<Column> column(std::initializer_list<WidgetPtr> children) {
    return std::make_shared<Column>(std::vector<WidgetPtr>(children));
}

inline std::shared_ptr<Column> column(Key key, std::vector<WidgetPtr> children) {
    return std::make_shared<Column>(std::move(key), std::move(children));
}

inline std::shared_ptr<Column> column(Key key, std::initializer_list<WidgetPtr> children) {
    return std::make_shared<Column>(std::move(key), std::vector<WidgetPtr>(children));
}

inline std::shared_ptr<Column> column(Justify justify, Align align, std::vector<WidgetPtr> children) {
    return std::make_shared<Column>(justify, align, std::move(children));
}

inline std::shared_ptr<Column> column(Justify justify, Align align, std::initializer_list<WidgetPtr> children) {
    return std::make_shared<Column>(justify, align, std::vector<WidgetPtr>(children));
}

inline std::shared_ptr<Column> column(FlexboxProps props) {
    auto c = std::make_shared<Column>(props.key, std::move(props.children));
    c->style = props.extractStyle();
    c->style.flex_direction = FlexDirection::Column;
    return c;
}

inline std::shared_ptr<Wrap> wrap(std::vector<WidgetPtr> children) {
    return std::make_shared<Wrap>(std::move(children));
}

inline std::shared_ptr<Wrap> wrap(std::initializer_list<WidgetPtr> children) {
    return std::make_shared<Wrap>(std::vector<WidgetPtr>(children));
}

inline std::shared_ptr<Wrap> wrap(Key key, std::vector<WidgetPtr> children) {
    return std::make_shared<Wrap>(std::move(key), std::move(children));
}

inline std::shared_ptr<Wrap> wrap(Justify justify, Align align, std::vector<WidgetPtr> children) {
    return std::make_shared<Wrap>(justify, align, std::move(children));
}

inline std::shared_ptr<Wrap> wrap(FlexboxProps props) {
    auto w = std::make_shared<Wrap>(props.key, std::move(props.children));
    w->style = props.extractStyle();
    w->style.flex_direction = FlexDirection::Row;
    w->style.flex_wrap = FlexWrap::Wrap;
    return w;
}

inline std::shared_ptr<FlexItem> flexItem(FlexboxStyle style, WidgetPtr child) {
    return std::make_shared<FlexItem>(std::move(style), std::move(child));
}

inline std::shared_ptr<FlexItem> flexItem(FlexItemProps props) {
    return std::make_shared<FlexItem>(props.key, props.extractStyle(), std::move(props.child));
}

inline std::shared_ptr<FlexItem> expanded(WidgetPtr child, float flex = 1.0f) {
    FlexboxStyle s;
    s.flex_grow = flex;
    s.flex_shrink = 1.0f;
    s.flex_basis = StyleValue::point(0.0f);
    return std::make_shared<FlexItem>(s, std::move(child));
}

inline std::shared_ptr<FlexItem> expanded(ExpandedProps props) {
    FlexboxStyle s;
    s.flex_grow = props.flex;
    s.flex_shrink = 1.0f;
    s.flex_basis = StyleValue::point(0.0f);
    return std::make_shared<FlexItem>(s, std::move(props.child));
}

inline std::shared_ptr<FlexItem> flexible(WidgetPtr child, float flex = 1.0f) {
    FlexboxStyle s;
    s.flex_grow = flex;
    s.flex_shrink = 1.0f;
    s.flex_basis = StyleValue::autoValue();
    return std::make_shared<FlexItem>(s, std::move(child));
}

inline std::shared_ptr<FlexItem> flexible(ExpandedProps props) {
    FlexboxStyle s;
    s.flex_grow = props.flex;
    s.flex_shrink = 1.0f;
    s.flex_basis = StyleValue::autoValue();
    return std::make_shared<FlexItem>(s, std::move(props.child));
}

inline std::shared_ptr<FlexItem> spacer(float flex = 1.0f) {
    FlexboxStyle s;
    s.flex_grow = flex;
    s.flex_shrink = 1.0f;
    s.flex_basis = StyleValue::point(0.0f);
    return std::make_shared<FlexItem>(s, nullptr);
}

} // namespace enki
