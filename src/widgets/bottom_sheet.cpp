/// @file bottom_sheet.cpp
/// @brief Implementation of Advanced BottomSheet widget.

#include "enki/widgets/bottom_sheet.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/stack.hpp"
#include "enki/animation/animation_controller.hpp"
#include "enki/animation/ticker.hpp"
#include "enki/platform/platform.hpp"
#include "enki/state/state.hpp"
#include "enki/tree/element.hpp"
#include "enki/tree/build_context.hpp"
#include "enki/rendering/canvas.hpp"
#include "enki/rendering/paint.hpp"
#include "enki/app/app.hpp"

#include <algorithm>
#include <iostream>
#include <cmath>

namespace enki {

// ════════════════════════════════════════════════════════════════
// RenderBottomSheetScrim — Scrim overlay that dismisses on tap
// ════════════════════════════════════════════════════════════════

class RenderBottomSheetScrim : public RenderBox {
public:
    float alpha;
    Color base_color;
    std::function<void()> on_tap;

    RenderBottomSheetScrim(float a, Color c, std::function<void()> tap)
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

    void handlePointerDown(const PointerEvent&) override {
        if (alpha > 0.0f && on_tap) {
            on_tap();
        }
    }
};

class BottomSheetScrimWidget : public SingleChildRenderObjectWidget {
public:
    float alpha;
    Color base_color;
    std::function<void()> on_tap;

    BottomSheetScrimWidget(float a, Color c, std::function<void()> tap)
        : SingleChildRenderObjectWidget(Key::none()), alpha(a), base_color(c), on_tap(std::move(tap)) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext&) override {
        return std::make_unique<RenderBottomSheetScrim>(alpha, base_color, on_tap);
    }

    void updateRenderObject(BuildContext&, RenderObject& ro) override {
        auto& r = static_cast<RenderBottomSheetScrim&>(ro);
        r.alpha      = alpha;
        r.base_color = base_color;
        r.on_tap     = on_tap;
        r.markNeedsPaint();
    }

    [[nodiscard]] std::string_view typeName() const override { return "BottomSheetScrimWidget"; }
};

// ════════════════════════════════════════════════════════════════
// BottomSheet State Implementation
// ════════════════════════════════════════════════════════════════

class BottomSheetState : public State {
private:
    AnimationController anim_;
    std::unique_ptr<Ticker> ticker_;

    bool is_open_ = false;
    BottomSheetDetent current_detent_ = BottomSheetDetent::Half;
    float current_fraction_ = 0.50f;

    bool is_dragging_ = false;
    float drag_start_y_ = 0.0f;
    float drag_start_fraction_ = 0.0f;

    SlotId key_down_conn_ = 0;

public:
    void initState() override {
        State::initState();
        auto* w = static_cast<const BottomSheetWidget*>(widget());
        is_open_ = w->initial_open;
        current_detent_ = w->options.initial_detent;
        updateFractionFromDetent();

        anim_.setDuration(std::chrono::milliseconds(200));
        anim_.addListener([this] { setState([] {}); });
        anim_.setValue(is_open_ ? 1.0f : 0.0f);

        ticker_ = createTicker([this] {
            if (anim_.isAnimating()) anim_.tick();
        });
        ticker_->start();

        wireController();

        if (Platform::instance()) {
            key_down_conn_ = Platform::instance()->onKeyDown().connect([this](int key, int) {
                if (key == 0xff1b) { // Escape key
                    if (is_open_) {
                        closeSheet();
                    }
                }
            });
        }
    }

    void didUpdateWidget(const Widget& old_widget) override {
        State::didUpdateWidget(old_widget);
        wireController();
    }

    void dispose() override {
        if (Platform::instance() && key_down_conn_) {
            Platform::instance()->onKeyDown().disconnect(key_down_conn_);
        }
        ticker_->stop();
        anim_.dispose();
        State::dispose();
    }

    void wireController() {
        auto* w = static_cast<const BottomSheetWidget*>(widget());
        if (w->controller) {
            w->controller->show_fn = [this](BottomSheetDetent d) { openSheet(d); };
            w->controller->hide_fn = [this] { closeSheet(); };
            w->controller->toggle_fn = [this] {
                if (is_open_) closeSheet();
                else openSheet(BottomSheetDetent::Half);
            };
            w->controller->snap_fn = [this](BottomSheetDetent d) { setDetent(d); };
            w->controller->is_open_fn = [this] { return is_open_; };
            w->controller->get_detent_fn = [this] { return current_detent_; };
        }
    }

    void openSheet(BottomSheetDetent detent = BottomSheetDetent::Half) {
        current_detent_ = detent;
        updateFractionFromDetent();
        is_open_ = true;
        anim_.forward();

        auto* w = static_cast<const BottomSheetWidget*>(widget());
        if (w->options.on_opened) w->options.on_opened();
        if (w->options.on_detent_changed) w->options.on_detent_changed(current_detent_);
        setState([] {});
    }

    void closeSheet() {
        if (!is_open_) return;
        is_open_ = false;
        anim_.reverse();

        auto* w = static_cast<const BottomSheetWidget*>(widget());
        if (w->options.on_closed) w->options.on_closed();
        setState([] {});
    }

    void setDetent(BottomSheetDetent detent) {
        current_detent_ = detent;
        if (detent == BottomSheetDetent::Hidden) {
            closeSheet();
            return;
        }
        updateFractionFromDetent();
        if (!is_open_) {
            openSheet(detent);
        } else {
            auto* w = static_cast<const BottomSheetWidget*>(widget());
            if (w->options.on_detent_changed) w->options.on_detent_changed(current_detent_);
            setState([] {});
        }
    }

    void updateFractionFromDetent() {
        auto* w = static_cast<const BottomSheetWidget*>(widget());
        const auto& opt = w->options;
        switch (current_detent_) {
            case BottomSheetDetent::Hidden: current_fraction_ = 0.0f; break;
            case BottomSheetDetent::Peek:   current_fraction_ = opt.peek_height / 700.0f; break;
            case BottomSheetDetent::Half:   current_fraction_ = opt.half_fraction; break;
            case BottomSheetDetent::Full:   current_fraction_ = opt.full_fraction; break;
        }
    }

    void snapToNearest(float frac) {
        auto* w = static_cast<const BottomSheetWidget*>(widget());
        const auto& opt = w->options;

        float peek_f = opt.peek_height / 700.0f;
        float half_f = opt.half_fraction;
        float full_f = opt.full_fraction;

        if (frac < peek_f * 0.5f) {
            closeSheet();
        } else if (std::abs(frac - peek_f) <= std::abs(frac - half_f)) {
            setDetent(BottomSheetDetent::Peek);
        } else if (std::abs(frac - half_f) <= std::abs(frac - full_f)) {
            setDetent(BottomSheetDetent::Half);
        } else {
            setDetent(BottomSheetDetent::Full);
        }
    }

    WidgetPtr build(BuildContext&) override {
        auto* w = static_cast<const BottomSheetWidget*>(widget());
        const auto& opts = w->options;
        float t = anim_.value();

        // ── 1. Stable Background Body (Always 100% width & height) ─────
        WidgetPtr body_widget;
        if (w->body) {
            auto bx = container(w->body);
            bx->width(StyleValue::percent(100.0f))
              .height(StyleValue::percent(100.0f));
            body_widget = Positioned::fill(bx);
        } else {
            auto empty = container();
            empty->width(StyleValue::percent(100.0f))
                 .height(StyleValue::percent(100.0f));
            body_widget = Positioned::fill(empty);
        }

        // When closed and animation finished, render just the stack with body
        if (t <= 0.001f && !is_open_) {
            auto root_stack = stack({body_widget});
            root_stack->style.width  = StyleValue::percent(100.0f);
            root_stack->style.height = StyleValue::percent(100.0f);
            return root_stack;
        }

        // ── 2. Scrim Overlay ──────────────────────────────────────────
        WidgetPtr scrim = nullptr;
        if (opts.type == BottomSheetType::Modal) {
            std::function<void()> tap_cb = nullptr;
            if (opts.close_on_overlay) {
                tap_cb = [this] { closeSheet(); };
            }
            scrim = std::make_shared<BottomSheetScrimWidget>(t, opts.overlay_color, tap_cb);
        }

        // ── 3. BottomSheet Panel Construction ─────────────────────────
        float height_pct = std::clamp(current_fraction_ * 100.0f, 12.0f, 96.0f);

        // A. Drag Handle Pill
        std::vector<WidgetPtr> handle_row_items;
        if (opts.show_drag_handle) {
            auto pill = container();
            pill->color(opts.handle_color)
                .borderRadius(2.5f)
                .width(44.0f)
                .height(5.0f);
            handle_row_items.push_back(pill);
        }
        auto handle_row = row(handle_row_items);
        handle_row->justifyContent(Justify::Center)
                  .width(StyleValue::percent(100.0f));

        // B. Title & Subtitle + Close Button
        std::vector<WidgetPtr> title_items;
        if (!opts.title.empty()) {
            auto t_lbl = text(opts.title);
            t_lbl->fontSize(16.0f).bold().color(opts.title_color);
            title_items.push_back(t_lbl);

            if (!opts.subtitle.empty()) {
                auto sub_lbl = text(opts.subtitle);
                sub_lbl->fontSize(12.0f).color(opts.subtitle_color);
                title_items.push_back(sub_lbl);
            }
        }
        auto title_col = column(title_items);
        title_col->gap(StyleValue::point(2.0f)).flex(1.0f);

        std::vector<WidgetPtr> top_bar_items = {title_col};
        if (opts.show_close_button) {
            auto btn_close = button(text("✕"), [this] {
                closeSheet();
            });
            top_bar_items.push_back(btn_close);
        }

        auto top_bar_row = row(top_bar_items);
        top_bar_row->justifyContent(Justify::SpaceBetween)
                   .alignItems(Align::Center)
                   .width(StyleValue::percent(100.0f));

        // C. Header GestureDetector for Dragging
        std::vector<WidgetPtr> hdr_stack_items;
        if (opts.show_drag_handle) {
            hdr_stack_items.push_back(handle_row);
        }
        if (!opts.title.empty() || opts.show_close_button) {
            hdr_stack_items.push_back(top_bar_row);
        }

        auto hdr_col = column(hdr_stack_items);
        hdr_col->gap(StyleValue::point(8.0f))
               .width(StyleValue::percent(100.0f));

        auto hdr_detector = std::make_shared<GestureDetector>(hdr_col);
        hdr_detector->cursor_type = opts.show_drag_handle ? SystemCursor::ResizeVertical : SystemCursor::Default;

        hdr_detector->on_pan_start = [this, opts](const DragStartDetails& e) {
            if (!opts.enable_drag) return;
            is_dragging_ = true;
            drag_start_y_ = e.global_position.y;
            drag_start_fraction_ = current_fraction_;
        };

        hdr_detector->on_pan_update = [this, opts](const DragUpdateDetails& e) {
            if (!is_dragging_) return;
            float vh = 720.0f;
            float delta_y = drag_start_y_ - e.global_position.y;
            float new_frac = std::clamp(drag_start_fraction_ + (delta_y / vh), 0.05f, 0.95f);
            current_fraction_ = new_frac;
            if (opts.on_drag_progress) {
                opts.on_drag_progress(current_fraction_);
            }
            setState([] {});
        };

        hdr_detector->on_pan_end = [this](const DragEndDetails&) {
            if (!is_dragging_) return;
            is_dragging_ = false;
            snapToNearest(current_fraction_);
        };

        // D. Combine Panel
        std::vector<WidgetPtr> sheet_kids = {hdr_detector};
        if (w->sheet_content) {
            auto content_box = container(w->sheet_content);
            content_box->flex(1.0f).width(StyleValue::percent(100.0f));
            sheet_kids.push_back(content_box);
        }

        auto sheet_col = column(sheet_kids);
        sheet_col->gap(StyleValue::point(10.0f))
                 .width(StyleValue::percent(100.0f))
                 .height(StyleValue::percent(100.0f));

        auto sheet_box = container(sheet_col);
        sheet_box->color(opts.background_color)
                 .border(opts.border_color, 1.0f)
                 .borderRadius(BorderRadius::only(opts.border_radius, opts.border_radius, 0.0f, 0.0f))
                 .paddingAll(16.0f)
                 .width(StyleValue::percent(100.0f))
                 .height(StyleValue::percent(100.0f))
                 .clip(true);

        // Positioned sheet panel sliding up from bottom
        auto pos_sheet = std::make_shared<Positioned>(sheet_box);
        pos_sheet->style.left   = StyleValue::point(0.0f);
        pos_sheet->style.right  = StyleValue::point(0.0f);

        // Slide up offset strictly proportional to sheet height:
        pos_sheet->style.bottom = StyleValue::percent((t - 1.0f) * height_pct);
        pos_sheet->style.height = StyleValue::percent(height_pct);

        // ── 4. Stack Composition: Body + Scrim + Sheet ────────────────
        std::vector<WidgetPtr> stack_items;
        stack_items.push_back(body_widget);
        if (scrim) {
            stack_items.push_back(scrim);
        }
        stack_items.push_back(pos_sheet);

        auto root_stack = stack(stack_items);
        root_stack->style.width  = StyleValue::percent(100.0f);
        root_stack->style.height = StyleValue::percent(100.0f);

        return root_stack;
    }
};

std::unique_ptr<State> BottomSheetWidget::createState() {
    return std::make_unique<BottomSheetState>();
}

} // namespace enki
