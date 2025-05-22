#include "enki/widgets/drawer.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/stack.hpp"
#include "enki/animation/animation_controller.hpp"
#include "enki/animation/ticker.hpp"
#include "enki/state/state.hpp"
#include "enki/rendering/canvas.hpp"
#include "enki/rendering/paint.hpp"
#include "enki/tree/build_context.hpp"
#include <algorithm>
#include <cmath>

namespace enki {

// ════════════════════════════════════════════════════════════════
// RenderDrawerScrim — draws a semi-transparent overlay
// ════════════════════════════════════════════════════════════════

class RenderDrawerScrim : public RenderBox {
public:
    float  alpha;
    Color  base_color;
    std::function<void()> on_tap;

    RenderDrawerScrim(float a, Color c, std::function<void()> tap) 
        : alpha(a), base_color(c), on_tap(std::move(tap)) {
        ANUNodeStyleSetPositionType(anu_node_, ANUPositionTypeAbsolute);
        ANUNodeStyleSetPosition(anu_node_, ANUEdgeTop, 0.0f);
        ANUNodeStyleSetPosition(anu_node_, ANUEdgeLeft, 0.0f);
        ANUNodeStyleSetPosition(anu_node_, ANUEdgeRight, 0.0f);
        ANUNodeStyleSetPosition(anu_node_, ANUEdgeBottom, 0.0f);
    }

    void paint(PaintContext& ctx) override {
        if (alpha <= 0.0f) return;
        float x = ctx.offset.x;
        float y = ctx.offset.y;
        float w = size_.width;
        float h = size_.height;

        uint8_t base_a = (base_color >> 24) & 0xFF;
        uint8_t eff_a  = static_cast<uint8_t>(base_a * alpha);
        Color col = (static_cast<uint32_t>(eff_a) << 24) | (base_color & 0x00FFFFFF);

        Paint p;
        p.setColor(col);
        ctx.canvas.drawRect(Rect{x, y, w, h}, p);
    }

    bool hitTestSelf(Point p) const override {
        return alpha > 0.0f && p.x >= 0 && p.x <= size_.width &&
               p.y >= 0 && p.y <= size_.height;
    }

    void handlePointerDown(const PointerEvent& e) override {
        if (alpha > 0.0f && on_tap) {
            on_tap();
        }
    }
};

class DrawerScrimWidget : public SingleChildRenderObjectWidget {
public:
    float alpha;
    Color base_color;
    std::function<void()> on_tap;

    DrawerScrimWidget(float a, Color c, std::function<void()> tap) 
        : alpha(a), base_color(c), on_tap(std::move(tap)) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext&) override {
        return std::make_unique<RenderDrawerScrim>(alpha, base_color, on_tap);
    }

    void updateRenderObject(BuildContext&, RenderObject& ro) override {
        auto& r = static_cast<RenderDrawerScrim&>(ro);
        r.alpha      = alpha;
        r.base_color = base_color;
        r.on_tap     = on_tap;
        r.markNeedsPaint();
    }

    [[nodiscard]] std::string_view typeName() const override { return "DrawerScrimWidget"; }
};

// ════════════════════════════════════════════════════════════════
// DrawerState
// ════════════════════════════════════════════════════════════════

class DrawerState : public State {
    AnimationController     anim_;    // 0=closed 1=open
    std::unique_ptr<Ticker> ticker_;
    bool is_open_ = false;

public:
    void initState() override {
        State::initState();
        auto* w = static_cast<const Drawer*>(widget());
        is_open_ = w->initial_open;

        anim_.setDuration(std::chrono::milliseconds(280));
        anim_.addListener([this] { setState([] {}); });
        anim_.setValue(is_open_ ? 1.0f : 0.0f);

        ticker_ = createTicker([this] {
            if (anim_.isAnimating()) anim_.tick();
        });
        ticker_->start();

        // Wire up external controller
        if (w->controller) {
            w->controller->open_fn = [this] { openDrawer(); };
            w->controller->close_fn = [this] { closeDrawer(); };
            w->controller->is_open_fn = [this] { return is_open_; };
        }
    }

    void dispose() override {
        ticker_->stop();
        anim_.dispose();
        State::dispose();
    }

    void openDrawer() {
        if (is_open_) return;
        is_open_ = true;
        anim_.forward();
        auto* w = static_cast<const Drawer*>(widget());
        if (w->on_open) w->on_open();
    }

    void closeDrawer() {
        if (!is_open_) return;
        is_open_ = false;
        anim_.reverse();
        auto* w = static_cast<const Drawer*>(widget());
        if (w->on_close) w->on_close();
    }

    WidgetPtr build(BuildContext&) override {
        auto* w = static_cast<const Drawer*>(widget());
        const auto& opts = w->options;
        float t = anim_.value();

        // The drawer slides in/out: when t=0 it's fully off-screen,
        // when t=1 it's fully visible.
        float drawer_width = opts.width;

        // ── Body ───────────────────────────────────────────────
        WidgetPtr body_widget;
        if (w->body) {
            auto bx = container(w->body);
            bx->flex(1.0f).height(StyleValue::percent(100.0f));
            body_widget = bx;
        } else {
            auto empty = container();
            empty->flex(1.0f).height(StyleValue::percent(100.0f));
            body_widget = empty;
        }

        // Only render overlay + drawer when partially or fully open
        if (t <= 0.001f) {
            return body_widget;
        }

        // ── Scrim overlay (dismiss on tap) ─────────────────────
        WidgetPtr scrim;
        {
            std::function<void()> tap_cb = nullptr;
            if (opts.close_on_overlay) {
                tap_cb = [this] { closeDrawer(); };
            }
            scrim = std::make_shared<DrawerScrimWidget>(t, opts.overlay_color, tap_cb);
        }

        // ── Drawer panel ───────────────────────────────────────
        // Offset: t=0 → fully off-screen, t=1 → fully on-screen
        float offset_x;
        if (opts.side == DrawerSide::Left) {
            // slides from left: starts at -drawer_width, ends at 0
            offset_x = drawer_width * (t - 1.0f);
        } else {
            // slides from right; Positioned via right=0, so we use left = (1-t)*width
            // We'll use absolute left positioning
            offset_x = 0.0f; // handled by Positioned
        }

        // Build drawer content
        std::vector<WidgetPtr> drawer_kids;
        if (w->drawer_content) {
            auto dc = container(w->drawer_content);
            dc->flex(1.0f);
            drawer_kids.push_back(dc);
        }
        auto drawer_col = std::make_shared<Column>(std::move(drawer_kids));
        drawer_col->style.height = StyleValue::percent(100.0f);

        auto drawer_box = container(drawer_col);
        drawer_box->color(opts.background_color)
                   .width(drawer_width)
                   .height(StyleValue::percent(100.0f))
                   .clip(true);

        if (opts.border_radius > 0.0f) {
            drawer_box->borderRadius(BorderRadius::only(
                opts.side == DrawerSide::Left ? 0.0f : opts.border_radius,
                opts.side == DrawerSide::Left ? opts.border_radius : 0.0f,
                opts.side == DrawerSide::Left ? opts.border_radius : 0.0f,
                opts.side == DrawerSide::Left ? 0.0f : opts.border_radius
            ));
        }

        // Shadow on drawer panel
        if (opts.shadow_blur > 0.0f) {
            float sdx = opts.side == DrawerSide::Left ? 4.0f : -4.0f;
            drawer_box->shadow(BoxShadow(opts.shadow_color, {sdx, 0.0f}, opts.shadow_blur));
        }

        // Position the drawer absolutely within the stack
        WidgetPtr positioned_drawer;
        {
            auto pos = std::make_shared<Positioned>(drawer_box);
            pos->style.top    = StyleValue::point(0.0f);
            pos->style.bottom = StyleValue::point(0.0f);

            if (opts.side == DrawerSide::Left) {
                float left_val = drawer_width * (t - 1.0f);
                pos->style.left  = StyleValue::point(left_val);
                pos->style.width = StyleValue::point(drawer_width);
            } else {
                float right_off = drawer_width * (t - 1.0f);
                pos->style.right = StyleValue::point(right_off);
                pos->style.width = StyleValue::point(drawer_width);
            }
            positioned_drawer = pos;
        }

        // ── Stack: body + scrim + drawer ───────────────────────
        auto root_stack = std::make_shared<Stack>(std::vector<WidgetPtr>{
            body_widget,
            scrim,
            positioned_drawer,
        });
        root_stack->style.width  = StyleValue::percent(100.0f);
        root_stack->style.height = StyleValue::percent(100.0f);

        return root_stack;
    }
};

std::unique_ptr<State> Drawer::createState() {
    return std::make_unique<DrawerState>();
}

} // namespace enki
