#include "enki/widgets/navigation_rail.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
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
// NavigationRailState
// ════════════════════════════════════════════════════════════════

class NavigationRailState : public State {
    AnimationController    expand_anim_;  // 0=collapsed 1=expanded
    std::unique_ptr<Ticker> ticker_;
    bool   is_expanded_    = true;
    int    hovered_index_  = -1;

public:
    void initState() override {
        State::initState();
        auto* w = static_cast<const NavigationRailWidget*>(widget());
        is_expanded_ = w->options.initially_expanded;

        expand_anim_.setDuration(std::chrono::milliseconds(220));
        expand_anim_.addListener([this] { setState([] {}); });
        if (is_expanded_) expand_anim_.setValue(1.0f);
        else              expand_anim_.setValue(0.0f);

        ticker_ = createTicker([this] {
            if (expand_anim_.isAnimating()) expand_anim_.tick();
        });
        ticker_->start();
    }

    void dispose() override {
        ticker_->stop();
        expand_anim_.dispose();
        State::dispose();
    }

    WidgetPtr build(BuildContext&) override {
        auto* w = static_cast<const NavigationRailWidget*>(widget());
        const auto& opts = w->options;
        float t = expand_anim_.value();
        float cur_w = opts.collapsed_width + (opts.expanded_width - opts.collapsed_width) * t;

        std::vector<WidgetPtr> items_col;

        // ── Toggle button ───────────────────────────────────────
        {
            std::string icon = is_expanded_ ? "◀" : "▶";
            auto icon_t = text({
                .text = icon,
                .color = opts.inactive_color,
                .font_size = 14.0f,
            });
            auto toggle_box = container(icon_t);
            toggle_box->height(opts.header_height)
                       .width(StyleValue::percent(100.0f))
                       .align(Alignment::Center);

            auto toggle_det = gestureDetector({
                .child = toggle_box,
                .hit_test_behavior = HitTestBehavior::Opaque,
                .cursor_type = SystemCursor::Pointer,
                .on_tap = [this] {
                    is_expanded_ = !is_expanded_;
                    if (is_expanded_) expand_anim_.forward();
                    else              expand_anim_.reverse();
                },
            });
            items_col.push_back(toggle_det);
        }

        // ── Rail items ─────────────────────────────────────────
        int n = static_cast<int>(w->items.size());
        for (int i = 0; i < n; ++i) {
            bool active  = (i == w->selected_index);
            bool hovered = (i == hovered_index_);
            const auto& item = w->items[i];

            Color text_col = active ? opts.active_color : opts.inactive_color;

            // Icon
            WidgetPtr icon_node = Icon { .data = item.icon, .size = opts.icon_font_size, .color = text_col };

            WidgetPtr row_content;

            // When expanded, show label alongside icon
            if (t > 0.01f && !item.label.empty()) {
                // Alpha-fade the label based on expansion progress
                uint8_t alpha = static_cast<uint8_t>(std::clamp(t * 2.0f - 0.4f, 0.0f, 1.0f) * 255);
                auto faded_label = text({
                    .text = item.label,
                    .color = (text_col & 0x00FFFFFF) | (static_cast<uint32_t>(alpha) << 24),
                    .font_size = opts.label_font_size,
                    .font_weight = active ? FontWeight::Bold : FontWeight::Normal,
                });

                row_content = row({
                    .align_items = Align::Center,
                    .gap = StyleValue::point(12.0f),
                    .padding = StyleInsets::only(0, 0, 0, 16.0f),
                    .children = { icon_node, faded_label },
                });

            } else {
                row_content = row({
                    .align_items = Align::Center,
                    .padding = StyleInsets::only(0, 0, 0, 16.0f),
                    .children = { icon_node },
                });
            }

            // Item container with indicator background
            Color bg_col = active ? opts.indicator_color : (hovered ? opts.hover_color : Colors::Transparent);
            auto item_box = container(row_content);
            item_box->color(bg_col)
                     .borderRadius(opts.indicator_radius)
                     .height(opts.item_height)
                     .width(StyleValue::percent(100.0f))
                     .paddingSymmetric(0.0f, 8.0f)
                     .align(Alignment::CenterLeft);

            // Badge — shown inline (no absolute positioning needed here)
            WidgetPtr item_widget = item_box;
            if (!item.badge.empty()) {
                auto badge_t = text({
                    .text = item.badge,
                    .color = opts.badge_text_color,
                    .font_size = 9.0f,
                });
                auto badge_box = container(badge_t);
                badge_box->color(opts.badge_color)
                          .borderRadius(8.0f)
                          .paddingSymmetric(1.0f, 4.0f);

                // Show badge next to item in a row
                item_widget = row({
                    .align_items = Align::Center,
                    .children = { item_box, badge_box },
                });
            }

            int idx = i;
            auto on_sel = w->on_item_selected;
            auto det = gestureDetector({
                .child = item_widget,
                .hit_test_behavior = HitTestBehavior::Opaque,
                .cursor_type = SystemCursor::Pointer,
                .on_tap = [on_sel, idx] {
                    if (on_sel) on_sel(idx);
                },
                .on_hover_enter = [this, idx](const PointerEvent&) {
                    setState([this, idx] { hovered_index_ = idx; });
                },
                .on_hover_exit = [this](const PointerEvent&) {
                    setState([this] { hovered_index_ = -1; });
                },
            });

            items_col.push_back(det);
        }

        auto items_column = column({
            .gap = StyleValue::point(4.0f),
            .padding = StyleInsets{opts.padding_v, 0, opts.padding_v, 0},
            .children = std::move(items_col),
        });

        // Rail container
        auto rail = container(items_column);
        rail->color(opts.background_color)
             .width(cur_w)
             .height(StyleValue::percent(100.0f))
             .border(opts.border_color, opts.border_right_width);

        return rail;
    }
};

std::unique_ptr<State> NavigationRailWidget::createState() {
    return std::make_unique<NavigationRailState>();
}

} // namespace enki
