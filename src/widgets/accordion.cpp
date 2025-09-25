/// @file accordion.cpp
/// @brief Implementation of Advanced Accordion widget for ENKI Framework.

#include "enki/widgets/accordion.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/state/state.hpp"
#include "enki/tree/build_context.hpp"

#include <algorithm>
#include <iostream>
#include <vector>
#include <set>

namespace enki {

class AccordionState : public State {
private:
    std::set<std::string> expanded_ids_;

public:
    void initState() override {
        State::initState();
        auto* w = static_cast<const AccordionWidget*>(widget());

        for (const auto& it : w->props.items) {
            if (it.is_initially_expanded && !it.is_disabled) {
                expanded_ids_.insert(it.id);
                if (w->props.mode == AccordionMode::Single) {
                    break; // Only 1 expanded in Single mode
                }
            }
        }

        wireController();
    }

    void didUpdateWidget(const Widget& old) override {
        State::didUpdateWidget(old);
        wireController();
    }

    void wireController() {
        auto* w = static_cast<const AccordionWidget*>(widget());
        if (w->props.controller) {
            w->props.controller->expand_fn = [this](const std::string& id) { expandSection(id); };
            w->props.controller->collapse_fn = [this](const std::string& id) { collapseSection(id); };
            w->props.controller->toggle_fn = [this](const std::string& id) { toggleSection(id); };
            w->props.controller->expand_all_fn = [this] { expandAllSections(); };
            w->props.controller->collapse_all_fn = [this] { collapseAllSections(); };
            w->props.controller->is_expanded_fn = [this](const std::string& id) { return expanded_ids_.count(id) > 0; };
            w->props.controller->get_expanded_ids_fn = [this] { return expanded_ids_; };
        }
    }

    void expandSection(const std::string& id) {
        auto* w = static_cast<const AccordionWidget*>(widget());
        if (w->props.mode == AccordionMode::Single) {
            expanded_ids_.clear();
        }
        expanded_ids_.insert(id);
        if (w->props.on_toggle) w->props.on_toggle(id, true);
        if (w->props.on_change) w->props.on_change(expanded_ids_);
        setState([] {});
    }

    void collapseSection(const std::string& id) {
        auto* w = static_cast<const AccordionWidget*>(widget());
        expanded_ids_.erase(id);
        if (w->props.on_toggle) w->props.on_toggle(id, false);
        if (w->props.on_change) w->props.on_change(expanded_ids_);
        setState([] {});
    }

    void toggleSection(const std::string& id) {
        auto* w = static_cast<const AccordionWidget*>(widget());
        if (expanded_ids_.count(id) > 0) {
            if (w->props.mode == AccordionMode::Multiple || w->props.collapsible) {
                collapseSection(id);
            }
        } else {
            expandSection(id);
        }
    }

    void expandAllSections() {
        auto* w = static_cast<const AccordionWidget*>(widget());
        if (w->props.mode == AccordionMode::Multiple) {
            for (const auto& it : w->props.items) {
                if (!it.is_disabled) expanded_ids_.insert(it.id);
            }
            if (w->props.on_change) w->props.on_change(expanded_ids_);
            setState([] {});
        }
    }

    void collapseAllSections() {
        auto* w = static_cast<const AccordionWidget*>(widget());
        expanded_ids_.clear();
        if (w->props.on_change) w->props.on_change(expanded_ids_);
        setState([] {});
    }

    // ── Build Single Accordion Item Section ───────────────────────

    WidgetPtr buildItemSection(const AccordionItem& item, const AccordionProps& opts, bool is_first, bool is_last) {
        bool is_expanded = (expanded_ids_.count(item.id) > 0);

        // ── 1. Header Left: Icon + Title + Subtitle ───────────────────
        std::vector<WidgetPtr> left_items;

        if (!item.icon.empty()) {
            auto ic_box = container(text({
                .text = item.icon,
                .font_size = 14.0f,
            }));
            ic_box->paddingAll(2.0f);
            left_items.push_back(ic_box);
        }

        std::vector<WidgetPtr> title_col_items;
        Color title_col_val = item.is_disabled ? 0xFF64748B : (is_expanded ? 0xFF38BDF8 : opts.title_color);
        auto title_txt = text({
            .text = item.title,
            .color = title_col_val,
            .font_size = 13.5f,
            .font_weight = FontWeight::Bold,
        });
        title_col_items.push_back(title_txt);

        if (!item.subtitle.empty()) {
            auto sub_txt = text({
                .text = item.subtitle,
                .color = item.is_disabled ? 0xFF475569 : opts.subtitle_color,
                .font_size = 11.5f,
            });
            title_col_items.push_back(sub_txt);
        }

        auto title_col = column({
            .flex = 1.0f,
            .gap = StyleValue::point(2.0f),
            .children = std::move(title_col_items),
        });
        left_items.push_back(title_col);

        auto left_row = row({
            .align_items = Align::Center,
            .flex = 1.0f,
            .gap = StyleValue::point(10.0f),
            .children = std::move(left_items),
        });

        // ── 2. Header Right: Badge Pill + Rotating Chevron ────────────
        std::vector<WidgetPtr> right_items;

        if (!item.badge_label.empty()) {
            auto b_txt = text({
                .text = item.badge_label,
                .color = item.badge_fg,
                .font_size = 11.0f,
                .font_weight = FontWeight::Bold,
            });

            auto b_box = container(b_txt);
            b_box->color(item.badge_bg)
                 .borderRadius(4.0f)
                 .paddingSymmetric(2.0f, 6.0f);
            right_items.push_back(b_box);
        }

        if (opts.show_chevron) {
            Color chv_col = item.is_disabled ? 0xFF475569 : (is_expanded ? 0xFF38BDF8 : opts.chevron_color);
            auto chv_txt = text({
                .text = is_expanded ? "⌃" : "⌄",
                .color = chv_col,
                .font_size = 15.0f,
                .font_weight = FontWeight::Bold,
            });

            auto chv_box = container(chv_txt);
            chv_box->paddingAll(2.0f);
            right_items.push_back(chv_box);
        }

        auto right_row = row({
            .align_items = Align::Center,
            .gap = StyleValue::point(8.0f),
            .children = std::move(right_items),
        });

        // Header Row Container
        auto h_row = row({
            .justify_content = Justify::SpaceBetween,
            .align_items = Align::Center,
            .width = StyleValue::percent(100.0f),
            .children = {left_row, right_row},
        });

        auto header_box = container(h_row);
        header_box->paddingSymmetric(12.0f, 16.0f)
                  .width(StyleValue::percent(100.0f));

        if (is_expanded && opts.variant == AccordionVariant::Bordered) {
            header_box->color(0x1A38BDF8); // Subtle active blue tint
        }

        // GestureDetector for Header
        auto item_id = item.id;
        auto header_gd = gestureDetector({
            .child = header_box,
            .cursor_type = !item.is_disabled ? SystemCursor::Pointer : SystemCursor::Default,
            .on_tap_up = !item.is_disabled ? GestureTapUpCallback([this, item_id](const TapUpDetails&) {
                toggleSection(item_id);
            }) : nullptr,
        });

        std::vector<WidgetPtr> section_col_items = {header_gd};

        // ── 3. Expanded Content Body ──────────────────────────────────
        if (is_expanded && item.content) {
            // Divider between header and body
            auto div = container();
            div->color(opts.divider_color).height(1.0f).width(StyleValue::percent(100.0f));
            section_col_items.push_back(div);

            auto body_box = container(item.content);
            body_box->paddingAll(16.0f).width(StyleValue::percent(100.0f));
            section_col_items.push_back(body_box);
        }

        auto section_col = column({
            .width = StyleValue::percent(100.0f),
            .children = std::move(section_col_items),
        });

        // ── 4. Apply Variant Styling ──────────────────────────────────
        if (opts.variant == AccordionVariant::Separated) {
            auto card = container(section_col);
            card->color(opts.background_color)
                .border(is_expanded ? 0xFF0284C7 : opts.border_color, 1.0f)
                .borderRadius(opts.border_radius)
                .width(StyleValue::percent(100.0f))
                .shadow(BoxShadow(0x66000000, {0.0f, 4.0f}, 12.0f));
            return card;
        }

        return section_col;
    }

    WidgetPtr build(BuildContext&) override {
        auto* w = static_cast<const AccordionWidget*>(widget());
        const auto& opts = w->props;

        std::vector<WidgetPtr> items_list;
        size_t n = w->props.items.size();

        for (size_t i = 0; i < n; ++i) {
            bool is_first = (i == 0);
            bool is_last = (i == n - 1);

            auto section = buildItemSection(w->props.items[i], opts, is_first, is_last);
            items_list.push_back(section);

            // Divider between sections (for Bordered & Flush variants)
            if (!is_last && opts.variant != AccordionVariant::Separated) {
                auto div = container();
                div->color(opts.divider_color).height(1.0f).width(StyleValue::percent(100.0f));
                items_list.push_back(div);
            }
        }

        if (opts.variant == AccordionVariant::Separated) {
            return column({
                .gap = StyleValue::point(opts.gap),
                .width = StyleValue::percent(100.0f),
                .children = std::move(items_list),
            });
        }

        auto main_col = column({
            .width = StyleValue::percent(100.0f),
            .children = std::move(items_list),
        });

        if (opts.variant == AccordionVariant::Bordered) {
            auto outer_box = container(main_col);
            outer_box->color(opts.background_color)
                     .border(opts.border_color, 1.0f)
                     .borderRadius(opts.border_radius)
                     .width(StyleValue::percent(100.0f))
                     .shadow(BoxShadow(0x66000000, {0.0f, 4.0f}, 16.0f));
            return outer_box;
        }

        // Flush variant
        return main_col;
    }
};

std::unique_ptr<State> AccordionWidget::createState() {
    return std::make_unique<AccordionState>();
}

} // namespace enki
