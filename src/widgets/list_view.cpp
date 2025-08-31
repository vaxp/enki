#include "enki/widgets/list_view.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/scroll_view.hpp"
#include "enki/widgets/divider.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/tree/build_context.hpp"
#include "enki/state/state.hpp"

namespace enki {

class ListViewState : public State {
public:
    WidgetPtr build(BuildContext& ctx) override {
        auto* w = static_cast<const ListViewWidget*>(widget());

        // ── Gather items ───────────────────────────────────────
        std::vector<WidgetPtr> children;

        auto add_item = [&](int idx, WidgetPtr item) {
            // Separator before this item (not before first)
            if (idx > 0 && w->props.separator_builder) {
                if (auto sep = w->props.separator_builder(idx - 1)) {
                    children.push_back(sep);
                }
            }

            // Selection: wrap with gesture detection if callback present
            if (w->props.on_item_selected) {
                auto on_sel = w->props.on_item_selected;
                auto det = std::make_shared<GestureDetector>(item);
                det->hit_test_behavior = HitTestBehavior::Opaque;
                det->cursor_type = SystemCursor::Pointer;
                det->on_tap = [on_sel, idx]{ on_sel(idx); };
                children.push_back(det);
            } else {
                children.push_back(item);
            }
        };

        if (w->props.item_builder) {
            for (int i = 0; i < w->props.item_count; ++i)
                add_item(i, w->props.item_builder(i));
        } else {
            for (int i = 0; i < (int)w->props.items.size(); ++i)
                add_item(i, w->props.items[i]);
        }

        // ── Build flex column/row ──────────────────────────────
        auto flex = std::make_shared<Flexbox>(std::move(children));
        flex->flexDirection(w->props.direction == Axis::Vertical
                            ? FlexDirection::Column : FlexDirection::Row);
        flex->flexShrink(0.0f);

        if (w->props.direction == Axis::Vertical)
            flex->width(StyleValue::percent(100.0f));
        else
            flex->height(StyleValue::percent(100.0f));

        // Apply padding via a wrapper
        WidgetPtr content;
        if (w->props.list_padding != EdgeInsets{}) {
            auto pc = container(flex);
            pc->padding(w->props.list_padding);
            content = pc;
        } else {
            content = flex;
        }

        // ── Wrap in ScrollView ─────────────────────────────────
        ScrollOptions scroll_opts;
        scroll_opts.direction    = w->props.direction;
        scroll_opts.scroll_speed = w->props.scroll_speed;
        scroll_opts.show_scrollbar = true;
        scroll_opts.clamp_overscroll = (w->props.scroll_physics == ScrollPhysics::Clamped);

        if (w->props.shrink_wrap) {
            // No scroll, just return the column directly
            return content;
        }

        return scrollView(scroll_opts, content);
    }
};

std::unique_ptr<State> ListViewWidget::createState() {
    return std::make_unique<ListViewState>();
}

} // namespace enki
