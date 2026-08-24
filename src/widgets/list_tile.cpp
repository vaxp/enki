#include "enki/widgets/list_tile.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/tree/build_context.hpp"
#include "enki/rendering/canvas.hpp"
#include "enki/state/state.hpp"
#include <cmath>
#include <chrono>

namespace enki {

// ════════════════════════════════════════════════════════════════
// RenderListTile
// ════════════════════════════════════════════════════════════════

RenderListTile::RenderListTile(ListTileProps props) : props_(props) {
    ANUNodeStyleSetWidthPercent(anu_node_, 100.0f);
    ticker_ = createTicker([this]() {
        double now = std::chrono::duration<double>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
        this->tick(now);
    });
}

void RenderListTile::setProps(const ListTileProps& props) {
    props_ = props;
    markNeedsPaint();
}

void RenderListTile::setSelected(bool s) {
    if (selected_ != s) { selected_ = s; markNeedsPaint(); }
}

void RenderListTile::setEnabled(bool e) {
    if (enabled_ != e) { enabled_ = e; markNeedsPaint(); }
}

void RenderListTile::setFocused(bool f) {
    if (focused_ != f) { focused_ = f; markNeedsPaint(); }
}

bool RenderListTile::hitTestSelf(Point localPoint) const {
    return Rect::fromPointSize({0, 0}, size_).contains(localPoint);
}

void RenderListTile::handlePointerDown(const PointerEvent& e) {
    if (!enabled_) return;
    pressed_ = true;
    ripple_active_ = true;
    ripple_origin_ = e.localPosition;
    ripple_start_ = std::chrono::duration<double>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    ripple_radius_ = 0.0f;
    ripple_alpha_ = 1.0f;
    if (ticker_) ticker_->start();
    markNeedsPaint();
}

void RenderListTile::handlePointerUp(const PointerEvent& e) {
    if (!enabled_) return;
    if (pressed_) {
        if (on_tap) on_tap();
    }
    pressed_ = false;
    markNeedsPaint();
}

void RenderListTile::handlePointerEnter(const PointerEvent& e) {
    hovered_ = true;
    markNeedsPaint();
}

void RenderListTile::handlePointerExit(const PointerEvent& e) {
    hovered_ = false;
    pressed_ = false;
    markNeedsPaint();
}

void RenderListTile::tick(double now) {
    if (!ripple_active_) {
        if (ticker_) ticker_->stop();
        return;
    }
    double elapsed = now - ripple_start_;
    float t = static_cast<float>(elapsed / kRippleDuration);
    if (t >= 1.0f) {
        ripple_active_ = false;
        ripple_alpha_ = 0.0f;
        if (ticker_) ticker_->stop();
        markNeedsPaint();
        return;
    }
    ripple_radius_ = kRippleMaxRadius * t;
    ripple_alpha_ = 1.0f - t;
    markNeedsPaint();
}

void RenderListTile::paintBackground(PaintContext& ctx, const Rect& bounds) {
    Color bg = props_.tile_color;

    if (selected_) {
        // Blend selected color over tile color
        bg = props_.selected_color;
    }

    if (hovered_ && enabled_) {
        bg = bg == Colors::Transparent ? props_.hover_color :
             (Color)(((bg & 0xFF000000) | ((uint8_t)((bg & 0xFF) + 13) & 0xFF)));
        bg = props_.hover_color;
    }

    if (pressed_ && enabled_) {
        bg = props_.pressed_color;
    }

    if (!enabled_) {
        bg = props_.disabled_color;
    }

    if (bg != Colors::Transparent) {
        Paint paint;
        paint.setColor(bg);
        paint.setAntiAlias(true);
        if (props_.shape.isUniform() && props_.shape.top_left > 0.0f) {
            ctx.canvas.drawRRect(bounds, props_.shape, paint);
        } else {
            ctx.canvas.drawRect(bounds, paint);
        }
    }
}

void RenderListTile::paintRipple(PaintContext& ctx, const Rect& bounds) {
    if (!ripple_active_ || ripple_alpha_ <= 0.0f) return;

    ctx.canvas.save();
    if (props_.shape.isUniform() && props_.shape.top_left > 0.0f) {
        ctx.canvas.clipRRect(bounds, props_.shape);
    } else {
        ctx.canvas.clipRect(bounds);
    }

    uint8_t alpha = static_cast<uint8_t>(ripple_alpha_ * ((props_.splash_color >> 24) & 0xFF));
    Color ripple_color = (props_.splash_color & 0x00FFFFFF) | (static_cast<uint32_t>(alpha) << 24);

    Paint paint;
    paint.setColor(ripple_color);
    paint.setAntiAlias(true);
    Point center = {bounds.x + ripple_origin_.x, bounds.y + ripple_origin_.y};
    ctx.canvas.drawCircle(center, ripple_radius_, paint);
    ctx.canvas.restore();
}

void RenderListTile::paint(PaintContext& context) {
    Rect bounds = Rect::fromPointSize(context.offset, size_);
    paintBackground(context, bounds);
    paintRipple(context, bounds);

    // Paint children (the row built by ListTileState)
    for (auto* child : children_) {
        if (child) {
            PaintContext child_ctx = context.withOffset(child->offset());
            child->paint(child_ctx);
        }
    }
}

// ════════════════════════════════════════════════════════════════
// ListTileRenderWidget — Internal wrapper to create RenderListTile
// ════════════════════════════════════════════════════════════════

class ListTileRenderWidget : public MultiChildRenderObjectWidget {
public:
    ListTileProps props;
    bool selected;
    bool enabled;
    std::function<void()> on_tap;
    std::function<void()> on_long_press;
    std::function<void()> on_secondary_tap;

    ListTileRenderWidget(std::vector<WidgetPtr> children, ListTileProps opt,
                         bool sel, bool en)
        : MultiChildRenderObjectWidget(Key::none(), std::move(children)),
          props(std::move(opt)), selected(sel), enabled(en) {}

    std::unique_ptr<RenderObject> createRenderObject(BuildContext&) override {
        auto r = std::make_unique<RenderListTile>(props);
        r->setSelected(selected);
        r->setEnabled(enabled);
        r->on_tap = on_tap;
        r->on_long_press = on_long_press;
        r->on_secondary_tap = on_secondary_tap;
        return r;
    }

    void updateRenderObject(BuildContext&, RenderObject& ro) override {
        auto& r = static_cast<RenderListTile&>(ro);
        r.setProps(props);
        r.setSelected(selected);
        r.setEnabled(enabled);
        r.on_tap = on_tap;
        r.on_long_press = on_long_press;
        r.on_secondary_tap = on_secondary_tap;
    }

    std::string_view typeName() const override { return "ListTileRenderWidget"; }
};

// ════════════════════════════════════════════════════════════════
// ListTileState
// ════════════════════════════════════════════════════════════════

class ListTileState : public State {
public:
    WidgetPtr build(BuildContext& ctx) override {
        auto* w = static_cast<const ListTileWidget*>(widget());
        const auto& opts = w->props;

        // ── Build content row ──────────────────────────────────
        // The row: [padding_left] [leading?] [gap] [title/subtitle col] [gap] [trailing?] [padding_right]
        std::vector<WidgetPtr> row_children;

        // Leading
        if (opts.leading_widget) {
            auto lc = container(opts.leading_widget);
            lc->width(opts.leading_width);
            lc->align(Alignment::Center);
            lc->margin(EdgeInsets::only(0, opts.leading_gap, 0, 0));
            row_children.push_back(lc);

            // Gap between leading and title
            auto gap = container();
            gap->width(opts.leading_gap);
            gap->height(StyleValue::percent(100.0f));
            row_children.push_back(gap);
        }

        // Title + subtitle column
        std::vector<WidgetPtr> title_col_children;
        if (opts.title_widget) title_col_children.push_back(opts.title_widget);
        if (opts.subtitle_widget) title_col_children.push_back(opts.subtitle_widget);

        WidgetPtr title_col;
        if (title_col_children.size() == 1) {
            title_col = title_col_children[0];
        } else {
            title_col = column({
                .justify_content = Justify::Center,
                .children = std::move(title_col_children),
            });
        }

        auto title_wrapper = flexItem({
            .flex_grow = 1.0f,
            .flex_shrink = 1.0f,
            .child = title_col,
        });
        row_children.push_back(title_wrapper);

        // Trailing
        if (opts.trailing_widget) {
            auto gap = container();
            gap->width(opts.trailing_gap);
            gap->height(StyleValue::percent(100.0f));
            row_children.push_back(gap);

            auto tc = container(opts.trailing_widget);
            tc->width(opts.trailing_width);
            tc->align(Alignment::Center);
            row_children.push_back(tc);
        }

        // Compute min height based on density + subtitle presence
        float min_h = opts.subtitle_widget
            ? opts.min_height_two_line
            : (opts.visual_density == VisualDensity::Compact ? opts.dense_min_height : opts.min_height);

        auto content_row = row({
            .align_items = Align::Center,
            .children = std::move(row_children),
        });

        // Apply content padding
        auto padded = container(content_row);
        padded->padding(opts.content_padding);
        padded->minHeight(StyleValue::point(min_h));
        padded->width(StyleValue::percent(100.0f));

        // ── Wrap in gesture detector ───────────────────────────
        auto detector = gestureDetector({
            .child = padded,
            .hit_test_behavior = HitTestBehavior::Opaque,
            .cursor_type = opts.enabled ? SystemCursor::Pointer : SystemCursor::Default,
            .on_tap = opts.enabled ? opts.on_tap : nullptr,
            .on_secondary_tap = opts.enabled ? opts.on_secondary_tap : nullptr,
            .on_long_press = opts.enabled ? opts.on_long_press : nullptr,
        });

        // ── Wrap in the custom render widget for hover/press bg ─
        auto render = std::make_shared<ListTileRenderWidget>(
            std::vector<WidgetPtr>{detector}, opts, opts.selected, opts.enabled);
        render->on_tap        = opts.on_tap;
        render->on_long_press = opts.on_long_press;
        render->on_secondary_tap = opts.on_secondary_tap;

        return render;
    }
};

std::unique_ptr<State> ListTileWidget::createState() {
    return std::make_unique<ListTileState>();
}

} // namespace enki
