#include "enki/widgets/breadcrumb.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/container.hpp"
#include "enki/state/state.hpp"
#include "enki/tree/build_context.hpp"

namespace enki {

// ════════════════════════════════════════════════════════════════
// BreadcrumbItemWidget — single interactive item
// ════════════════════════════════════════════════════════════════

class BreadcrumbItemWidget : public StatefulWidget {
public:
    std::string            label;
    std::function<void()>  on_tap;
    bool                   active;      ///< true = last item (non-clickable, brighter)
    BreadcrumbProps        options;

    BreadcrumbItemWidget(std::string label, std::function<void()> on_tap,
                         bool active, BreadcrumbProps opt)
        : label(std::move(label)), on_tap(std::move(on_tap)),
          active(active), options(std::move(opt)) {}

    std::string_view typeName() const override { return "BreadcrumbItemWidget"; }
    std::unique_ptr<State> createState() override;
};

class BreadcrumbItemState : public State {
    bool hovered_ = false;
public:
    WidgetPtr build(BuildContext&) override {
        auto* w = static_cast<const BreadcrumbItemWidget*>(widget());

        Color col;
        if (w->active) {
            col = w->options.active_color;
        } else if (hovered_) {
            col = w->options.hover_color;
        } else {
            col = w->options.inactive_color;
        }

        bool bold = w->active && w->options.bold_active;
        auto t = text({
            .text = w->label,
            .color = col,
            .font_size = w->options.font_size,
            .font_weight = bold ? FontWeight::Bold : FontWeight::Normal,
        });

        if (w->active || !w->on_tap) {
            return t;
        }

        auto fn = w->on_tap;
        return gestureDetector({
            .child = t,
            .hit_test_behavior = HitTestBehavior::Opaque,
            .cursor_type = SystemCursor::Pointer,
            .on_tap = [fn] { if (fn) fn(); },
            .on_hover_enter = [this](const PointerEvent&) {
                setState([this] { hovered_ = true; });
            },
            .on_hover_exit = [this](const PointerEvent&) {
                setState([this] { hovered_ = false; });
            },
        });
    }
};

std::unique_ptr<State> BreadcrumbItemWidget::createState() {
    return std::make_unique<BreadcrumbItemState>();
}

// ════════════════════════════════════════════════════════════════
// BreadcrumbWidget::build
// ════════════════════════════════════════════════════════════════

WidgetPtr BreadcrumbWidget::build(BuildContext&) {
    if (options.items.empty()) {
        return container();
    }

    std::vector<WidgetPtr> children;
    int n = static_cast<int>(options.items.size());

    for (int i = 0; i < n; ++i) {
        bool is_active = (i == n - 1);

        auto item_widget = std::make_shared<BreadcrumbItemWidget>(
            options.items[i].label,
            options.items[i].on_tap,
            is_active,
            options);
        children.push_back(item_widget);

        // Separator between items
        if (i < n - 1) {
            auto sep = text({
                .text = options.separator,
                .color = options.separator_color,
                .font_size = options.separator_font_size,
            });

            auto sep_box = container(sep);
            sep_box->paddingSymmetric(0.0f, options.separator_spacing);
            children.push_back(sep_box);
        }
    }

    auto row = std::make_shared<Row>(std::move(children));
    row->style.align_items = Align::Center;
    row->style.gap         = StyleValue::point(options.item_spacing);

    return row;
}

} // namespace enki
