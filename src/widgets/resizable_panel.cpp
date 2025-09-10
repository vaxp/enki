/// @file resizable_panel.cpp
/// @brief Implementation of Advanced ResizablePanel & Floating Window for ENKI Framework.

#include "enki/widgets/resizable_panel.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/stack.hpp"
#include "enki/state/state.hpp"
#include "enki/tree/build_context.hpp"

#include <algorithm>
#include <iostream>
#include <vector>

namespace enki {

class ResizablePanelState : public State {
private:
    float current_x_ = 240.0f;
    float current_y_ = 120.0f;
    float current_width_ = 460.0f;
    float current_height_ = 320.0f;

    float saved_x_ = 240.0f;
    float saved_y_ = 120.0f;
    float saved_w_ = 460.0f;
    float saved_h_ = 320.0f;

    bool is_minimized_ = false;
    bool is_maximized_ = false;
    bool is_closed_ = false;

public:
    void initState() override {
        State::initState();
        auto* w = static_cast<const ResizablePanelWidget*>(widget());
        current_x_ = w->options.initial_x;
        current_y_ = w->options.initial_y;
        current_width_ = w->options.initial_width;
        current_height_ = w->options.initial_height;

        wireController();
    }

    void didUpdateWidget(const Widget& old) override {
        State::didUpdateWidget(old);
        wireController();
    }

    void wireController() {
        auto* w = static_cast<const ResizablePanelWidget*>(widget());
        if (w->controller) {
            w->controller->set_size_fn = [this](float width, float height) {
                auto* sw = static_cast<const ResizablePanelWidget*>(widget());
                current_width_ = std::clamp(width, sw->options.min_width, sw->options.max_width);
                current_height_ = std::clamp(height, sw->options.min_height, sw->options.max_height);
                setState([] {});
            };
            w->controller->set_position_fn = [this](float x, float y) {
                current_x_ = std::max(0.0f, x);
                current_y_ = std::max(0.0f, y);
                setState([] {});
            };
            w->controller->set_minimized_fn = [this](bool min) {
                is_minimized_ = min;
                setState([] {});
            };
            w->controller->set_maximized_fn = [this](bool max) {
                toggleMaximize();
            };
            w->controller->close_fn = [this] {
                is_closed_ = true;
                setState([] {});
            };
            w->controller->reset_fn = [this] {
                auto* sw = static_cast<const ResizablePanelWidget*>(widget());
                current_x_ = sw->options.initial_x;
                current_y_ = sw->options.initial_y;
                current_width_ = sw->options.initial_width;
                current_height_ = sw->options.initial_height;
                is_minimized_ = false;
                is_maximized_ = false;
                is_closed_ = false;
                setState([] {});
            };
            w->controller->get_position_fn = [this] { return Point{current_x_, current_y_}; };
            w->controller->get_size_fn = [this] { return Size{current_width_, current_height_}; };
        }
    }

    void toggleMaximize() {
        if (!is_maximized_) {
            saved_x_ = current_x_;
            saved_y_ = current_y_;
            saved_w_ = current_width_;
            saved_h_ = current_height_;
            current_x_ = 16.0f;
            current_y_ = 16.0f;
            current_width_ = 980.0f;
            current_height_ = 580.0f;
            is_maximized_ = true;
        } else {
            current_x_ = saved_x_;
            current_y_ = saved_y_;
            current_width_ = saved_w_;
            current_height_ = saved_h_;
            is_maximized_ = false;
        }
        setState([] {});
    }

    // ── Build Window Header Bar ───────────────────────────────────

    WidgetPtr buildHeaderBar(const ResizablePanelWidget* w) {
        const auto& opts = w->options;

        // Left: Icon + Title
        std::vector<WidgetPtr> left_items;
        if (!opts.icon.empty()) {
            auto ic = text(opts.icon);
            ic->fontSize(14.0f);
            left_items.push_back(ic);
        }

        auto tit = text(opts.title);
        tit->fontSize(13.0f).bold().color(opts.title_color);
        left_items.push_back(tit);

        auto left_row = row(left_items);
        left_row->gap(StyleValue::point(8.0f)).alignItems(Align::Center).flex(1.0f);

        // Right: Window Action Buttons (— ◻ ✕)
        auto makeActionBtn = [](std::string sym, Color fg, std::function<void()> cb) -> WidgetPtr {
            auto t = text(sym);
            t->fontSize(11.5f).bold().color(fg);

            auto b = container(t);
            b->paddingSymmetric(2.0f, 6.0f);

            auto gd = std::make_shared<GestureDetector>(b);
            gd->cursor_type = SystemCursor::Pointer;
            gd->on_tap_up = [cb](const TapUpDetails&) {
                if (cb) cb();
            };
            return gd;
        };

        std::vector<WidgetPtr> right_items;
        if (opts.allow_minimize) {
            right_items.push_back(makeActionBtn("—", 0xFF94A3B8, [this] {
                is_minimized_ = !is_minimized_;
                setState([] {});
            }));
        }
        if (opts.allow_maximize) {
            right_items.push_back(makeActionBtn(is_maximized_ ? "🗗" : "◻", 0xFF94A3B8, [this] {
                toggleMaximize();
            }));
        }
        if (opts.allow_close) {
            right_items.push_back(makeActionBtn("✕", 0xFFEF4444, [this] {
                is_closed_ = true;
                auto* sw = static_cast<const ResizablePanelWidget*>(widget());
                if (sw->options.on_closed) sw->options.on_closed();
                setState([] {});
            }));
        }

        auto right_row = row(right_items);
        right_row->gap(StyleValue::point(6.0f)).alignItems(Align::Center);

        std::vector<WidgetPtr> h_elements = {left_row, right_row};
        auto h_row = row(h_elements);
        h_row->justifyContent(Justify::SpaceBetween)
             .alignItems(Align::Center)
             .width(StyleValue::percent(100.0f));

        auto header_box = container(h_row);
        header_box->color(opts.header_bg_color)
                  .paddingSymmetric(8.0f, 12.0f)
                  .width(StyleValue::percent(100.0f));

        // Drag to move header gesture
        auto header_gd = std::make_shared<GestureDetector>(header_box);
        if (opts.allow_drag_move && !is_maximized_) {
            header_gd->cursor_type = SystemCursor::Move;
            header_gd->on_pan_update = [this](const DragUpdateDetails& d) {
                current_x_ = std::max(0.0f, current_x_ + d.delta.x);
                current_y_ = std::max(0.0f, current_y_ + d.delta.y);
                auto* sw = static_cast<const ResizablePanelWidget*>(widget());
                if (sw->options.on_moved) sw->options.on_moved(current_x_, current_y_);
                setState([] {});
            };
        }

        return header_gd;
    }

    WidgetPtr build(BuildContext&) override {
        auto* w = static_cast<const ResizablePanelWidget*>(widget());
        const auto& opts = w->options;

        // ── 1. Page Body Invariant Layer ──────────────────────────────
        WidgetPtr body_widget;
        if (w->body) {
            auto bx = container(w->body);
            bx->width(StyleValue::percent(100.0f)).height(StyleValue::percent(100.0f));
            body_widget = Positioned::fill(bx);
        } else {
            auto empty = container();
            empty->width(StyleValue::percent(100.0f)).height(StyleValue::percent(100.0f));
            body_widget = Positioned::fill(empty);
        }

        std::vector<WidgetPtr> stack_items = {body_widget};

        // ── 2. Floating Resizable Window Layer ────────────────────────
        if (!is_closed_) {
            std::vector<WidgetPtr> window_col_items;

            // Header
            if (opts.show_header) {
                window_col_items.push_back(buildHeaderBar(w));
            }

            // Body Content & Resize Handles
            if (!is_minimized_ && w->child) {
                auto div = container();
                div->color(opts.border_color).height(1.0f).width(StyleValue::percent(100.0f));
                window_col_items.push_back(div);

                auto body_container = container(w->child);
                body_container->paddingAll(14.0f)
                              .flex(1.0f)
                              .width(StyleValue::percent(100.0f));

                // Bottom-right corner resize grip (◢)
                auto grip_txt = text("◢");
                grip_txt->fontSize(12.0f).color(opts.grip_color);
                auto grip_box = container(grip_txt);
                grip_box->paddingAll(4.0f);

                auto grip_gd = std::make_shared<GestureDetector>(grip_box);
                grip_gd->cursor_type = SystemCursor::ResizeHorizontal;
                grip_gd->on_pan_update = [this, opts](const DragUpdateDetails& d) {
                    current_width_ = std::clamp(current_width_ + d.delta.x, opts.min_width, opts.max_width);
                    current_height_ = std::clamp(current_height_ + d.delta.y, opts.min_height, opts.max_height);
                    auto* sw = static_cast<const ResizablePanelWidget*>(widget());
                    if (sw->options.on_resized) sw->options.on_resized(current_width_, current_height_);
                    setState([] {});
                };

                auto grip_row = row(std::vector<WidgetPtr>{grip_gd});
                grip_row->justifyContent(Justify::End).width(StyleValue::percent(100.0f));

                window_col_items.push_back(body_container);
                window_col_items.push_back(grip_row);
            }

            auto window_col = column(window_col_items);
            window_col->width(StyleValue::percent(100.0f))
                      .height(StyleValue::percent(100.0f));

            auto window_card = container(window_col);
            window_card->color(opts.background_color)
                       .border(opts.active_border_col, 1.0f)
                       .borderRadius(opts.border_radius)
                       .width(current_width_)
                       .height(is_minimized_ ? 42.0f : current_height_)
                       .shadow(BoxShadow(0xCC000000, {0.0f, 10.0f}, 30.0f));

            stack_items.push_back(Positioned {
                .child = window_card,
                .top = StyleValue::point(current_y_),
                .left = StyleValue::point(current_x_),
            });
        }

        return Stack {
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .children = std::move(stack_items),
        };
    }
};

std::unique_ptr<State> ResizablePanelWidget::createState() {
    return std::make_unique<ResizablePanelState>();
}

} // namespace enki
