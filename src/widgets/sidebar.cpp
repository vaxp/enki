#include "enki/widgets/sidebar.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
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
// SidebarState
// ════════════════════════════════════════════════════════════════

class SidebarState : public State {
    AnimationController    anim_;    // 0=collapsed 1=expanded
    std::unique_ptr<Ticker> ticker_;
    bool   is_expanded_ = true;
    bool   toggle_hovered_ = false;

public:
    void initState() override {
        State::initState();
        auto* w = static_cast<const SidebarWidget*>(widget());
        is_expanded_ = w->options.initially_expanded;

        anim_.setDuration(std::chrono::milliseconds(220));
        anim_.addListener([this] { setState([] {}); });
        anim_.setValue(is_expanded_ ? 1.0f : 0.0f);

        ticker_ = createTicker([this] {
            if (anim_.isAnimating()) anim_.tick();
        });
        ticker_->start();
    }

    void dispose() override {
        ticker_->stop();
        anim_.dispose();
        State::dispose();
    }

    WidgetPtr build(BuildContext&) override {
        auto* w = static_cast<const SidebarWidget*>(widget());
        const auto& opts = w->options;

        float t     = anim_.value();
        float cur_w = opts.collapsed_width + (opts.expanded_width - opts.collapsed_width) * t;

        // ── Sidebar panel content ──────────────────────────────
        WidgetPtr panel_content;
        {
            std::vector<WidgetPtr> panel_children;

            // Toggle button at top
            if (opts.show_toggle_button && opts.collapsible) {
                std::string icon = is_expanded_ ? "◀" : "▶";
                Color icon_col   = toggle_hovered_ ? opts.toggle_hover_color : opts.toggle_color;

                auto icon_t = text({
                    .text = icon,
                    .color = icon_col,
                    .font_size = 14.0f,
                });
                auto toggle_box = container({
                    .align = Alignment::Center,
                    .width = StyleValue::percent(100.0f),
                    .height = StyleValue::point(opts.toggle_size),
                    .child = icon_t,
                });

                auto toggle_det = gestureDetector({
                    .child = toggle_box,
                    .hit_test_behavior = HitTestBehavior::Opaque,
                    .cursor_type = SystemCursor::Pointer,
                    .on_tap = [this, w] {
                        is_expanded_ = !is_expanded_;
                        if (is_expanded_) anim_.forward();
                        else              anim_.reverse();
                        if (w->on_toggle) w->on_toggle(is_expanded_);
                    },
                    .on_hover_enter = [this](const PointerEvent&) {
                        setState([this] { toggle_hovered_ = true; });
                    },
                    .on_hover_exit = [this](const PointerEvent&) {
                        setState([this] { toggle_hovered_ = false; });
                    },
                });
                panel_children.push_back(toggle_det);
            }

            // User-provided sidebar content
            if (w->sidebar_content) {
                auto content_box = container({
                    .clip_content = true,
                    .flex = 1.0f,
                    .child = w->sidebar_content,
                });
                panel_children.push_back(content_box);
            }

            auto panel_col = column({
                .height = StyleValue::percent(100.0f),
                .children = std::move(panel_children),
            });

            std::optional<Border> panel_border;
            if (opts.border_width > 0.0f) {
                panel_border = Border(opts.border_color, opts.border_width);
            }

            auto panel_box = container({
                .color = opts.background_color,
                .border = panel_border,
                .width = StyleValue::point(cur_w),
                .height = StyleValue::percent(100.0f),
                .child = panel_col,
            });

            panel_content = panel_box;
        }

        // ── Body content ───────────────────────────────────────
        WidgetPtr body_content;
        if (w->body) {
            auto body_box = container({
                .flex = 1.0f,
                .child = w->body,
            });
            body_content = body_box;
        } else {
            auto empty = container({
                .flex = 1.0f,
            });
            body_content = empty;
        }

        // ── Compose Row (sidebar + body) ───────────────────────
        std::vector<WidgetPtr> row_children;
        if (opts.side == SidebarSide::Left) {
            row_children = {panel_content, body_content};
        } else {
            row_children = {body_content, panel_content};
        }

        return row({
            .align_items = Align::Stretch,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .children = std::move(row_children),
        });
    }
};

std::unique_ptr<State> SidebarWidget::createState() {
    return std::make_unique<SidebarState>();
}

} // namespace enki
