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
        auto* w = static_cast<const DialogWidget*>(widget());
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
                    auto* w = static_cast<const DialogWidget*>(widget());
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
        auto* w = static_cast<const DialogWidget*>(widget());
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
        auto* w = static_cast<const DialogWidget*>(widget());
        if (w->options.on_opened) w->options.on_opened();
    }

    void closeDialog() {
        if (!is_open_) return;
        is_open_ = false;
        anim_.reverse();
        auto* w = static_cast<const DialogWidget*>(widget());
        if (w->options.on_closed) w->options.on_closed();
    }

    void toggleDialog() {
        if (is_open_) closeDialog();
        else openDialog();
    }

    // ── Build Dialog Card ─────────────────────────────────────────

    WidgetPtr buildDialogCard(const DialogWidget* w, float t) {
        const auto& opts = w->options;

        std::vector<WidgetPtr> card_elements;

        // ── 1. Header (Icon, Title, Subtitle, Close Button) ───────────
        bool has_header = !opts.title.empty() || !opts.icon.empty() || opts.show_close_button;
        if (has_header) {
            std::vector<WidgetPtr> header_left_items;

            // Icon Badge
            if (!opts.icon.empty()) {
                Color badge_bg = opts.icon_badge_bg;
                if (opts.type == DialogType::Danger)  badge_bg = 0x2EEF4444;
                if (opts.type == DialogType::Warning) badge_bg = 0x2EF59E0B;
                if (opts.type == DialogType::Success) badge_bg = 0x2E10B981;

                header_left_items.push_back(container({
                    .color = badge_bg,
                    .border_radius = BorderRadius::circular(8.0f),
                    .padding = StyleInsets::all(8.0f),
                    .child = text(opts.icon, { .font_size = 18.0f })
                }));
            }

            // Title + Subtitle Column
            std::vector<WidgetPtr> titles;
            if (!opts.title.empty()) {
                titles.push_back(text(opts.title, { .color = opts.title_color, .font_size = 16.0f, .font_weight = FontWeight::Bold }));
            }
            if (!opts.subtitle.empty()) {
                titles.push_back(text(opts.subtitle, { .color = opts.subtitle_color, .font_size = 12.5f }));
            }

            header_left_items.push_back(column({
                .flex = 1.0f,
                .gap = StyleValue::point(2.0f),
                .children = titles
            }));

            std::vector<WidgetPtr> header_items = {
                row({
                    .align_items = Align::Center,
                    .flex = 1.0f,
                    .gap = StyleValue::point(12.0f),
                    .children = header_left_items
                })
            };

            // Close button ✕
            if (opts.show_close_button) {
                auto close_btn = std::make_shared<GestureDetector>(
                    container({
                        .padding = StyleInsets::all(6.0f),
                        .child = text("✕", { .color = 0xFF94A3B8, .font_size = 14.0f, .font_weight = FontWeight::Bold })
                    })
                );
                close_btn->cursor_type = SystemCursor::Pointer;
                close_btn->on_tap_up = [this](const TapUpDetails&) {
                    closeDialog();
                };
                header_items.push_back(close_btn);
            }

            card_elements.push_back(row({
                .justify_content = Justify::SpaceBetween,
                .align_items = Align::Center,
                .width = StyleValue::percent(100.0f),
                .children = header_items
            }));

            // Divider below header
            card_elements.push_back(container({
                .color = 0xFF334155,
                .width = StyleValue::percent(100.0f),
                .height = StyleValue::point(1.0f)
            }));
        }

        // ── 2. Content Body ───────────────────────────────────────────
        if (w->dialog_content) {
            card_elements.push_back(container({
                .width = StyleValue::percent(100.0f),
                .child = w->dialog_content
            }));
        }

        // ── 3. Footer Action Buttons ──────────────────────────────────
        if (!opts.actions.empty()) {
            // Divider above actions
            card_elements.push_back(container({
                .color = 0xFF334155,
                .width = StyleValue::percent(100.0f),
                .height = StyleValue::point(1.0f)
            }));

            std::vector<WidgetPtr> action_buttons;
            for (const auto& act : opts.actions) {
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

                auto btn_gd = std::make_shared<GestureDetector>(
                    container({
                        .color = bg_col,
                        .border_radius = BorderRadius::circular(6.0f),
                        .border = Border(border_col, 1.0f),
                        .padding = StyleInsets::symmetric(8.0f, 18.0f),
                        .child = text(act.label, { .color = txt_col, .font_size = 13.0f, .font_weight = FontWeight::Bold })
                    })
                );
                btn_gd->cursor_type = SystemCursor::Pointer;
                btn_gd->on_tap_up = [this, act](const TapUpDetails&) {
                    if (act.on_click) act.on_click();
                    closeDialog();
                };
                action_buttons.push_back(btn_gd);
            }

            card_elements.push_back(row({
                .justify_content = Justify::End,
                .align_items = Align::Center,
                .gap = StyleValue::point(10.0f),
                .width = StyleValue::percent(100.0f),
                .children = action_buttons
            }));
        }

        // Outer dialog panel box with elevation shadow
        return container({
            .color = opts.background_color,
            .border_radius = BorderRadius::circular(opts.border_radius),
            .border = Border(opts.border_color, 1.0f),
            .box_shadow = {BoxShadow(0x99000000, {0.0f, 12.0f}, 32.0f)},
            .width = StyleValue::point(opts.width),
            .padding = StyleInsets::all(20.0f),
            .child = column({
                .gap = StyleValue::point(14.0f),
                .width = StyleValue::percent(100.0f),
                .children = card_elements
            })
        });
    }

    WidgetPtr build(BuildContext&) override {
        auto* w = static_cast<const DialogWidget*>(widget());
        const auto& opts = w->options;
        float t = anim_.value();

        // ── 1. Page Body (Invariant 100% full-screen stack layer) ─────
        WidgetPtr body_widget;
        if (w->body) {
            body_widget = Positioned::fill(container({
                .width = StyleValue::percent(100.0f),
                .height = StyleValue::percent(100.0f),
                .child = w->body
            }));
        } else {
            body_widget = Positioned::fill(container({
                .width = StyleValue::percent(100.0f),
                .height = StyleValue::percent(100.0f)
            }));
        }

        // When closed and animation finished: render only body
        if (t <= 0.001f) {
            return stack({
                .children = {body_widget},
                .width = StyleValue::percent(100.0f),
                .height = StyleValue::percent(100.0f)
            });
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
        auto pos_center = Positioned::fill(column({
            .justify_content = Justify::Center,
            .align_items = Align::Center,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .children = {dialog_card}
        }));

        // ── 4. Stack Composition: Body + Scrim + Centered Modal ───────
        return stack({
            .children = {
                body_widget,
                scrim,
                pos_center
            },
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f)
        });
    }
};

std::unique_ptr<State> DialogWidget::createState() {
    return std::make_unique<DialogState>();
}

} // namespace enki
