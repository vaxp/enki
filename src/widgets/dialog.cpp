/// @file dialog.cpp
/// @brief Implementation of Advanced In-Window Overlay Modal Dialog widget.

#include "enki/widgets/dialog.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/stack.hpp"
#include "enki/animation/animation_controller.hpp"
#include "enki/animation/ticker.hpp"
#include "enki/state/state.hpp"
#include "enki/rendering/canvas.hpp"
#include "enki/rendering/paint.hpp"
#include "enki/tree/build_context.hpp"
#include "enki/platform/platform.hpp"

#include <algorithm>
#include <iostream>
#include <cmath>

namespace enki {

// ════════════════════════════════════════════════════════════════
// RenderDialogScrim — Semi-transparent backdrop overlay
// ════════════════════════════════════════════════════════════════

class RenderDialogScrim : public RenderBox {
public:
    float alpha;
    Color base_color;
    std::function<void()> on_tap;

    RenderDialogScrim(float a, Color c, std::function<void()> tap)
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
        uint8_t eff_a  = static_cast<uint8_t>(base_a * std::clamp(alpha, 0.0f, 1.0f));
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

class DialogScrimWidget : public SingleChildRenderObjectWidget {
public:
    float alpha;
    Color base_color;
    std::function<void()> on_tap;

    DialogScrimWidget(float a, Color c, std::function<void()> tap)
        : SingleChildRenderObjectWidget(Key::none()),
          alpha(a), base_color(c), on_tap(std::move(tap)) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext&) override {
        return std::make_unique<RenderDialogScrim>(alpha, base_color, on_tap);
    }

    void updateRenderObject(BuildContext&, RenderObject& ro) override {
        auto& r = static_cast<RenderDialogScrim&>(ro);
        r.alpha = alpha;
        r.base_color = base_color;
        r.on_tap = on_tap;
        r.markNeedsPaint();
    }

    [[nodiscard]] std::string_view typeName() const override { return "DialogScrimWidget"; }
};

// ════════════════════════════════════════════════════════════════
// DialogState
// ════════════════════════════════════════════════════════════════

class DialogState : public State {
private:
    AnimationController anim_;
    std::unique_ptr<Ticker> ticker_;
    bool is_open_ = false;
    SlotId key_down_conn_ = 0;

public:
    void initState() override {
        State::initState();
        auto* w = static_cast<const Dialog*>(widget());
        is_open_ = w->initial_open;

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
                    auto* w = static_cast<const Dialog*>(widget());
                    if (w->options.escape_to_close && is_open_) {
                        closeDialog();
                    }
                }
            });
        }
    }

    void didUpdateWidget(const Widget& old) override {
        State::didUpdateWidget(old);
        wireController();
    }

    void dispose() override {
        if (Platform::instance() && key_down_conn_) {
            Platform::instance()->onKeyDown().disconnect(key_down_conn_);
        }
        if (ticker_) ticker_->stop();
        anim_.dispose();
        State::dispose();
    }

    void wireController() {
        auto* w = static_cast<const Dialog*>(widget());
        if (w->controller) {
            w->controller->show_fn = [this] { openDialog(); };
            w->controller->hide_fn = [this] { closeDialog(); };
            w->controller->toggle_fn = [this] { toggleDialog(); };
            w->controller->is_open_fn = [this] { return is_open_; };
        }
    }

    void openDialog() {
        if (is_open_) return;
        is_open_ = true;
        anim_.forward();
        auto* w = static_cast<const Dialog*>(widget());
        if (w->options.on_opened) w->options.on_opened();
    }

    void closeDialog() {
        if (!is_open_) return;
        is_open_ = false;
        anim_.reverse();
        auto* w = static_cast<const Dialog*>(widget());
        if (w->options.on_closed) w->options.on_closed();
    }

    void toggleDialog() {
        if (is_open_) closeDialog();
        else openDialog();
    }

    // ── Build Dialog Card ─────────────────────────────────────────

    WidgetPtr buildDialogCard(const Dialog* w, float t) {
        const auto& opts = w->options;

        std::vector<WidgetPtr> card_elements;

        // ── 1. Header (Icon, Title, Subtitle, Close Button) ───────────
        bool has_header = !opts.title.empty() || !opts.icon.empty() || opts.show_close_button;
        if (has_header) {
            std::vector<WidgetPtr> header_left_items;

            // Icon Badge
            if (!opts.icon.empty()) {
                auto ic_txt = text(opts.icon);
                ic_txt->fontSize(18.0f);

                Color badge_bg = opts.icon_badge_bg;
                if (opts.type == DialogType::Danger)  badge_bg = 0x2EEF4444;
                if (opts.type == DialogType::Warning) badge_bg = 0x2EF59E0B;
                if (opts.type == DialogType::Success) badge_bg = 0x2E10B981;

                auto badge_box = container(ic_txt);
                badge_box->color(badge_bg)
                         .borderRadius(8.0f)
                         .paddingAll(8.0f);
                header_left_items.push_back(badge_box);
            }

            // Title + Subtitle Column
            std::vector<WidgetPtr> titles;
            if (!opts.title.empty()) {
                auto t_txt = text(opts.title);
                t_txt->fontSize(16.0f).bold().color(opts.title_color);
                titles.push_back(t_txt);
            }
            if (!opts.subtitle.empty()) {
                auto s_txt = text(opts.subtitle);
                s_txt->fontSize(12.5f).color(opts.subtitle_color);
                titles.push_back(s_txt);
            }

            auto titles_col = column(titles);
            titles_col->gap(StyleValue::point(2.0f)).flex(1.0f);
            header_left_items.push_back(titles_col);

            auto header_left = row(header_left_items);
            header_left->gap(StyleValue::point(12.0f)).alignItems(Align::Center).flex(1.0f);

            std::vector<WidgetPtr> header_items = {header_left};

            // Close button ✕
            if (opts.show_close_button) {
                auto close_lbl = text("✕");
                close_lbl->fontSize(14.0f).bold().color(0xFF94A3B8);

                auto close_box = container(close_lbl);
                close_box->paddingAll(6.0f);

                auto close_btn = std::make_shared<GestureDetector>(close_box);
                close_btn->cursor_type = SystemCursor::Pointer;
                close_btn->on_tap_up = [this](const TapUpDetails&) {
                    closeDialog();
                };
                header_items.push_back(close_btn);
            }

            auto header_row = row(header_items);
            header_row->justifyContent(Justify::SpaceBetween)
                      .alignItems(Align::Center)
                      .width(StyleValue::percent(100.0f));

            card_elements.push_back(header_row);

            // Divider below header
            auto div = container();
            div->color(0xFF334155).height(1.0f).width(StyleValue::percent(100.0f));
            card_elements.push_back(div);
        }

        // ── 2. Content Body ───────────────────────────────────────────
        if (w->dialog_content) {
            auto body_box = container(w->dialog_content);
            body_box->width(StyleValue::percent(100.0f));
            card_elements.push_back(body_box);
        }

        // ── 3. Footer Action Buttons ──────────────────────────────────
        if (!opts.actions.empty()) {
            // Divider above actions
            auto div2 = container();
            div2->color(0xFF334155).height(1.0f).width(StyleValue::percent(100.0f));
            card_elements.push_back(div2);

            std::vector<WidgetPtr> action_buttons;
            for (const auto& act : opts.actions) {
                auto btn_txt = text(act.label);
                btn_txt->fontSize(13.0f).bold();

                Color bg_col = 0xFF334155;
                Color txt_col = 0xFFF1F5F9;
                Color border_col = 0xFF475569;

                if (act.is_danger) {
                    bg_col = 0xFFDC2626;
                    border_col = 0xFFEF4444;
                    txt_col = 0xFFFFFFFF;
                } else if (act.is_primary) {
                    bg_col = 0xFF0284C7;
                    border_col = 0xFF38BDF8;
                    txt_col = 0xFFFFFFFF;
                } else if (act.is_cancel) {
                    bg_col = 0xFF0F172A;
                    border_col = 0xFF334155;
                    txt_col = 0xFF94A3B8;
                }

                btn_txt->color(txt_col);

                auto btn_box = container(btn_txt);
                btn_box->color(bg_col)
                       .border(border_col, 1.0f)
                       .borderRadius(6.0f)
                       .paddingSymmetric(8.0f, 18.0f);

                auto btn_gd = std::make_shared<GestureDetector>(btn_box);
                btn_gd->cursor_type = SystemCursor::Pointer;
                btn_gd->on_tap_up = [this, act](const TapUpDetails&) {
                    if (act.on_click) act.on_click();
                    closeDialog();
                };
                action_buttons.push_back(btn_gd);
            }

            auto actions_row = row(action_buttons);
            actions_row->gap(StyleValue::point(10.0f))
                       .justifyContent(Justify::End)
                       .alignItems(Align::Center)
                       .width(StyleValue::percent(100.0f));

            card_elements.push_back(actions_row);
        }

        auto card_col = column(card_elements);
        card_col->gap(StyleValue::point(14.0f))
                .width(StyleValue::percent(100.0f));

        // Outer dialog panel box with elevation shadow
        auto dialog_box = container(card_col);
        dialog_box->color(opts.background_color)
                  .border(opts.border_color, 1.0f)
                  .borderRadius(opts.border_radius)
                  .paddingAll(20.0f)
                  .width(opts.width)
                  .shadow(BoxShadow(0x99000000, {0.0f, 12.0f}, 32.0f));

        return dialog_box;
    }

    WidgetPtr build(BuildContext&) override {
        auto* w = static_cast<const Dialog*>(widget());
        const auto& opts = w->options;
        float t = anim_.value();

        // ── 1. Page Body (Invariant 100% full-screen stack layer) ─────
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

        // When closed and animation finished: render only body
        if (t <= 0.001f) {
            std::vector<WidgetPtr> stack_items = {body_widget};
            auto root = stack(stack_items);
            root->style.width = StyleValue::percent(100.0f);
            root->style.height = StyleValue::percent(100.0f);
            return root;
        }

        // ── 2. Scrim Backdrop Overlay (Dismiss on tap) ────────────────
        std::function<void()> tap_cb = nullptr;
        if (opts.barrier_dismissible) {
            tap_cb = [this] { closeDialog(); };
        }
        auto scrim = std::make_shared<DialogScrimWidget>(t, opts.overlay_color, tap_cb);

        // ── 3. Centered Modal Dialog Card ─────────────────────────────
        auto dialog_card = buildDialogCard(w, t);

        // Perfect centering layout in full viewport
        std::vector<WidgetPtr> center_items = {dialog_card};
        auto center_col = column(center_items);
        center_col->justifyContent(Justify::Center)
                  .alignItems(Align::Center)
                  .width(StyleValue::percent(100.0f))
                  .height(StyleValue::percent(100.0f));

        auto pos_center = Positioned::fill(center_col);

        // ── 4. Stack Composition: Body + Scrim + Centered Modal ───────
        std::vector<WidgetPtr> stack_items = {
            body_widget,
            scrim,
            pos_center
        };

        auto root = stack(stack_items);
        root->style.width = StyleValue::percent(100.0f);
        root->style.height = StyleValue::percent(100.0f);
        return root;
    }
};

std::unique_ptr<State> Dialog::createState() {
    return std::make_unique<DialogState>();
}

} // namespace enki
