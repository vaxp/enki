/// @file split_view.cpp
/// @brief Implementation of Advanced SplitView widget for ENKI Framework.

#include "enki/widgets/split_view.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/stack.hpp"
#include "enki/state/state.hpp"
#include "enki/tree/build_context.hpp"

#include <algorithm>
#include <iostream>
#include <vector>

namespace enki {

class SplitViewState : public State {
private:
    float current_ratio_ = 0.50f;
    bool is_dragging_ = false;
    bool is_hovered_ = false;

public:
    void initState() override {
        State::initState();
        auto* w = static_cast<const SplitView*>(widget());
        current_ratio_ = std::clamp(w->options.initial_ratio, 0.0f, 1.0f);
        wireController();
    }

    void didUpdateWidget(const Widget& old) override {
        State::didUpdateWidget(old);
        wireController();
    }

    void wireController() {
        auto* w = static_cast<const SplitView*>(widget());
        if (w->controller) {
            w->controller->set_ratio_fn = [this](float r) {
                current_ratio_ = std::clamp(r, 0.0f, 1.0f);
                auto* sw = static_cast<const SplitView*>(widget());
                if (sw->options.on_split_changed) sw->options.on_split_changed(current_ratio_);
                setState([] {});
            };
            w->controller->get_ratio_fn = [this] { return current_ratio_; };
            w->controller->collapse_leading_fn = [this] {
                current_ratio_ = 0.0f;
                auto* sw = static_cast<const SplitView*>(widget());
                if (sw->options.on_pane_collapsed) sw->options.on_pane_collapsed(true);
                setState([] {});
            };
            w->controller->collapse_trailing_fn = [this] {
                current_ratio_ = 1.0f;
                auto* sw = static_cast<const SplitView*>(widget());
                if (sw->options.on_pane_collapsed) sw->options.on_pane_collapsed(false);
                setState([] {});
            };
            w->controller->reset_fn = [this] {
                auto* sw = static_cast<const SplitView*>(widget());
                current_ratio_ = sw->options.initial_ratio;
                if (sw->options.on_split_changed) sw->options.on_split_changed(current_ratio_);
                setState([] {});
            };
        }
    }

    WidgetPtr buildHandle(const SplitView* w) {
        const auto& opts = w->options;
        bool is_horizontal = (opts.orientation == SplitOrientation::Horizontal);

        Color bg_col = is_dragging_ ? opts.handle_active_color
                                    : (is_hovered_ ? opts.handle_hover_color : opts.handle_color);

        WidgetPtr grip_widget = nullptr;
        if (opts.show_handle_grip) {
            auto grip_txt = text(is_horizontal ? "⋮" : "⋯");
            grip_txt->fontSize(11.0f).color(is_hovered_ ? 0xFFFFFFFF : 0xFF64748B);
            grip_widget = grip_txt;
        }

        std::vector<WidgetPtr> grip_items;
        if (grip_widget) grip_items.push_back(grip_widget);

        auto grip_col = column(grip_items);
        grip_col->justifyContent(Justify::Center).alignItems(Align::Center);

        auto handle_box = container(grip_col);
        handle_box->color(bg_col);

        if (is_horizontal) {
            handle_box->width(opts.handle_thickness)
                      .height(StyleValue::percent(100.0f));
        } else {
            handle_box->height(opts.handle_thickness)
                      .width(StyleValue::percent(100.0f));
        }

        auto handle_gd = std::make_shared<GestureDetector>(handle_box);
        handle_gd->cursor_type = is_horizontal ? SystemCursor::ResizeHorizontal
                                               : SystemCursor::ResizeVertical;

        handle_gd->on_hover_enter = [this](const PointerEvent&) {
            is_hovered_ = true;
            setState([] {});
        };

        handle_gd->on_hover_exit = [this](const PointerEvent&) {
            is_hovered_ = false;
            setState([] {});
        };

        handle_gd->on_pan_start = [this](const DragStartDetails&) {
            is_dragging_ = true;
            setState([] {});
        };

        handle_gd->on_pan_update = [this, is_horizontal](const DragUpdateDetails& d) {
            float delta = is_horizontal ? d.delta.x : d.delta.y;
            // Approximate fractional displacement (~800px standard container)
            float fraction_delta = delta / 800.0f;
            current_ratio_ = std::clamp(current_ratio_ + fraction_delta, 0.05f, 0.95f);

            auto* sw = static_cast<const SplitView*>(widget());
            if (sw->options.on_split_changed) sw->options.on_split_changed(current_ratio_);
            setState([] {});
        };

        handle_gd->on_pan_end = [this](const DragEndDetails&) {
            is_dragging_ = false;
            setState([] {});
        };

        handle_gd->on_double_tap = [this] {
            auto* sw = static_cast<const SplitView*>(widget());
            current_ratio_ = sw->options.initial_ratio;
            if (sw->options.on_split_changed) sw->options.on_split_changed(current_ratio_);
            setState([] {});
        };

        return handle_gd;
    }

    WidgetPtr build(BuildContext&) override {
        auto* w = static_cast<const SplitView*>(widget());
        const auto& opts = w->options;
        bool is_horizontal = (opts.orientation == SplitOrientation::Horizontal);

        // ── 1. Leading Pane ───────────────────────────────────────────
        WidgetPtr leading_pane;
        if (w->leading) {
            auto l_box = container(w->leading);
            if (is_horizontal) {
                l_box->width(StyleValue::percent(current_ratio_ * 100.0f))
                     .height(StyleValue::percent(100.0f));
            } else {
                l_box->height(StyleValue::percent(current_ratio_ * 100.0f))
                     .width(StyleValue::percent(100.0f));
            }
            leading_pane = l_box;
        } else {
            auto empty = container();
            leading_pane = empty;
        }

        // ── 2. Draggable Handle Divider ───────────────────────────────
        auto handle = buildHandle(w);

        // ── 3. Trailing Pane ──────────────────────────────────────────
        WidgetPtr trailing_pane;
        if (w->trailing) {
            auto t_box = container(w->trailing);
            t_box->flex(1.0f);
            if (is_horizontal) {
                t_box->height(StyleValue::percent(100.0f));
            } else {
                t_box->width(StyleValue::percent(100.0f));
            }
            trailing_pane = t_box;
        } else {
            auto empty = container();
            empty->flex(1.0f);
            trailing_pane = empty;
        }

        // ── 4. Assemble Root Layout (Row for Horizontal, Column for Vertical)
        std::vector<WidgetPtr> split_items = {leading_pane, handle, trailing_pane};

        if (is_horizontal) {
            auto root_row = row(split_items);
            root_row->width(StyleValue::percent(100.0f))
                    .height(StyleValue::percent(100.0f));
            return root_row;
        } else {
            auto root_col = column(split_items);
            root_col->width(StyleValue::percent(100.0f))
                    .height(StyleValue::percent(100.0f));
            return root_col;
        }
    }
};

std::unique_ptr<State> SplitView::createState() {
    return std::make_unique<SplitViewState>();
}

} // namespace enki
