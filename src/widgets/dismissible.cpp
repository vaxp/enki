/// @file dismissible.cpp
/// @brief Implementation of Dismissible swipe-to-dismiss widget for ENKI Framework.

#include "enki/widgets/dismissible.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/stack.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/state/state.hpp"
#include "enki/tree/build_context.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>

namespace enki {

class DismissibleState : public State {
private:
    float drag_offset_x_ = 0.0f;
    bool is_dismissed_ = false;

public:
    WidgetPtr build(BuildContext&) override {
        if (is_dismissed_) {
            auto empty = container();
            empty->height(0.0f).width(0.0f);
            return empty;
        }

        auto* w = static_cast<const Dismissible*>(widget());
        const auto& opts = w->options;

        // ── 1. Background Action Layers ───────────────────────────────
        WidgetPtr active_bg = nullptr;
        if (drag_offset_x_ > 0.0f && opts.background) {
            active_bg = opts.background;
        } else if (drag_offset_x_ < 0.0f && opts.secondary_background) {
            active_bg = opts.secondary_background;
        } else if (opts.background) {
            active_bg = opts.background;
        }

        std::vector<WidgetPtr> stack_items;

        if (active_bg) {
            auto bg_box = container(active_bg);
            bg_box->width(StyleValue::percent(100.0f)).height(StyleValue::percent(100.0f));
            stack_items.push_back(Positioned::fill(bg_box));
        }

        // ── 2. Foreground Swiping Child ───────────────────────────────
        auto child_box = container(w->child);
        child_box->width(StyleValue::percent(100.0f));

        auto gd = std::make_shared<GestureDetector>(child_box);
        gd->cursor_type = SystemCursor::Move;

        gd->on_pan_update = [this, opts](const DragUpdateDetails& d) {
            drag_offset_x_ += d.delta.x;

            if (opts.direction == DismissDirection::EndToStart && drag_offset_x_ > 0.0f) {
                drag_offset_x_ = 0.0f;
            } else if (opts.direction == DismissDirection::StartToEnd && drag_offset_x_ < 0.0f) {
                drag_offset_x_ = 0.0f;
            }

            setState([] {});
        };

        gd->on_pan_end = [this, opts](const DragEndDetails&) {
            // Dismiss threshold (e.g. 120px)
            float threshold = 120.0f;

            if (std::abs(drag_offset_x_) >= threshold) {
                is_dismissed_ = true;
                DismissDirection dir = (drag_offset_x_ > 0.0f) ? DismissDirection::StartToEnd
                                                               : DismissDirection::EndToStart;
                if (opts.on_dismissed) opts.on_dismissed(dir);
            } else {
                drag_offset_x_ = 0.0f; // Spring back
            }

            setState([] {});
        };

        auto pos_fg = std::make_shared<Positioned>(gd);
        pos_fg->style.left = StyleValue::point(drag_offset_x_);
        pos_fg->style.right = StyleValue::point(-drag_offset_x_);
        pos_fg->style.top = StyleValue::point(0.0f);
        pos_fg->style.bottom = StyleValue::point(0.0f);

        stack_items.push_back(pos_fg);

        auto root = stack(stack_items);
        root->style.width = StyleValue::percent(100.0f);
        return root;
    }
};

std::unique_ptr<State> Dismissible::createState() {
    return std::make_unique<DismissibleState>();
}

} // namespace enki
