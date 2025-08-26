/// @file expansion_panel.cpp
/// @brief Implementation of Advanced ExpansionPanel & ExpansionPanelList for ENKI Framework.

#include "enki/widgets/expansion_panel.hpp"
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

class ExpansionPanelListState : public State {
private:
    std::set<int> expanded_indices_;

public:
    void initState() override {
        State::initState();
        auto* w = static_cast<const ExpansionPanelListWidget*>(widget());

        for (size_t i = 0; i < w->props.panels.size(); ++i) {
            if (w->props.panels[i].is_initially_expanded && !w->props.panels[i].is_disabled) {
                expanded_indices_.insert(static_cast<int>(i));
                if (w->props.is_radio_mode) break;
            }
        }

        wireController();
    }

    void didUpdateWidget(const Widget& old) override {
        State::didUpdateWidget(old);
        wireController();
    }

    void wireController() {
        auto* w = static_cast<const ExpansionPanelListWidget*>(widget());
        if (w->props.controller) {
            w->props.controller->expand_fn = [this](int idx) { expandPanel(idx); };
            w->props.controller->collapse_fn = [this](int idx) { collapsePanel(idx); };
            w->props.controller->toggle_fn = [this](int idx) { togglePanel(idx); };
            w->props.controller->expand_all_fn = [this] { expandAllPanels(); };
            w->props.controller->collapse_all_fn = [this] { collapseAllPanels(); };
            w->props.controller->is_expanded_fn = [this](int idx) { return expanded_indices_.count(idx) > 0; };
            w->props.controller->get_expanded_indices_fn = [this] { return expanded_indices_; };
        }
    }

    void expandPanel(int idx) {
        auto* w = static_cast<const ExpansionPanelListWidget*>(widget());
        if (idx < 0 || idx >= static_cast<int>(w->props.panels.size())) return;
        if (w->props.panels[idx].is_disabled) return;

        if (w->props.is_radio_mode) {
            expanded_indices_.clear();
        }
        expanded_indices_.insert(idx);
        if (w->props.on_panel_toggled) w->props.on_panel_toggled(idx, true);
        if (w->props.on_expansion_changed) w->props.on_expansion_changed(expanded_indices_);
        setState([] {});
    }

    void collapsePanel(int idx) {
        auto* w = static_cast<const ExpansionPanelListWidget*>(widget());
        expanded_indices_.erase(idx);
        if (w->props.on_panel_toggled) w->props.on_panel_toggled(idx, false);
        if (w->props.on_expansion_changed) w->props.on_expansion_changed(expanded_indices_);
        setState([] {});
    }

    void togglePanel(int idx) {
        if (expanded_indices_.count(idx) > 0) {
            collapsePanel(idx);
        } else {
            expandPanel(idx);
        }
    }

    void expandAllPanels() {
        auto* w = static_cast<const ExpansionPanelListWidget*>(widget());
        if (!w->props.is_radio_mode) {
            for (size_t i = 0; i < w->props.panels.size(); ++i) {
                if (!w->props.panels[i].is_disabled) expanded_indices_.insert(static_cast<int>(i));
            }
            if (w->props.on_expansion_changed) w->props.on_expansion_changed(expanded_indices_);
            setState([] {});
        }
    }

    void collapseAllPanels() {
        auto* w = static_cast<const ExpansionPanelListWidget*>(widget());
        expanded_indices_.clear();
        if (w->props.on_expansion_changed) w->props.on_expansion_changed(expanded_indices_);
        setState([] {});
    }

    // ── Build Single Panel Card ───────────────────────────────────

    WidgetPtr buildPanelCard(int index, const ExpansionPanelItem& item, const ExpansionPanelListProps& opts) {
        bool is_expanded = (expanded_indices_.count(index) > 0);

        // ── 1. Header Left: Step Pill / Icon + Title + Subtitle ───────
        std::vector<WidgetPtr> left_items;

        if (!item.icon_or_step.empty()) {
            auto step_txt = text(item.icon_or_step);
            step_txt->fontSize(12.0f).bold().color(is_expanded ? 0xFFFFFFFF : opts.step_pill_fg);

            auto step_box = container(step_txt);
            step_box->color(is_expanded ? 0xFF0284C7 : opts.step_pill_bg)
                    .borderRadius(6.0f)
                    .paddingSymmetric(4.0f, 8.0f);
            left_items.push_back(step_box);
        }

        std::vector<WidgetPtr> title_col_items;
        auto title_txt = text(item.title);
        title_txt->fontSize(14.0f).bold();
        title_txt->color(item.is_disabled ? 0xFF64748B : (is_expanded ? 0xFF38BDF8 : opts.title_color));
        title_col_items.push_back(title_txt);

        if (!item.subtitle.empty()) {
            auto sub_txt = text(item.subtitle);
            sub_txt->fontSize(11.5f).color(item.is_disabled ? 0xFF475569 : opts.subtitle_color);
            title_col_items.push_back(sub_txt);
        }

        auto title_col = column(title_col_items);
        title_col->gap(StyleValue::point(2.0f)).flex(1.0f);
        left_items.push_back(title_col);

        auto left_row = row(left_items);
        left_row->gap(StyleValue::point(10.0f)).alignItems(Align::Center).flex(1.0f);

        // ── 2. Header Right: Badge + Rotating Chevron ─────────────────
        std::vector<WidgetPtr> right_items;

        if (!item.badge_label.empty()) {
            auto b_txt = text(item.badge_label);
            b_txt->fontSize(10.5f).bold().color(item.badge_fg);

            auto b_box = container(b_txt);
            b_box->color(item.badge_bg).borderRadius(4.0f).paddingSymmetric(3.0f, 8.0f);
            right_items.push_back(b_box);
        }

        if (opts.show_chevron) {
            auto chv_txt = text(is_expanded ? "⌃" : "⌄");
            chv_txt->fontSize(15.0f).bold().color(item.is_disabled ? 0xFF475569 : (is_expanded ? 0xFF38BDF8 : opts.chevron_color));

            auto chv_box = container(chv_txt);
            chv_box->paddingAll(2.0f);
            right_items.push_back(chv_box);
        }

        auto right_row = row(right_items);
        right_row->gap(StyleValue::point(8.0f)).alignItems(Align::Center);

        // Header Row Container
        std::vector<WidgetPtr> h_elements = {left_row, right_row};
        auto h_row = row(h_elements);
        h_row->justifyContent(Justify::SpaceBetween)
             .alignItems(Align::Center)
             .width(StyleValue::percent(100.0f));

        auto header_box = container(h_row);
        header_box->paddingSymmetric(12.0f, 16.0f)
                  .width(StyleValue::percent(100.0f));

        if (is_expanded) {
            header_box->color(0x1A38BDF8); // Subtle blue highlight
        }

        // GestureDetector for Header
        auto header_gd = std::make_shared<GestureDetector>(header_box);
        if (item.can_tap_on_header && !item.is_disabled) {
            header_gd->cursor_type = SystemCursor::Pointer;
            header_gd->on_tap_up = [this, index](const TapUpDetails&) {
                togglePanel(index);
            };
        }

        std::vector<WidgetPtr> panel_col_items = {header_gd};

        // ── 3. Expanded Body & Footer Actions ─────────────────────────
        if (is_expanded && item.body) {
            // Divider
            auto div = container();
            div->color(opts.divider_color).height(1.0f).width(StyleValue::percent(100.0f));
            panel_col_items.push_back(div);

            // Body
            auto body_box = container(item.body);
            body_box->paddingAll(16.0f).width(StyleValue::percent(100.0f));
            panel_col_items.push_back(body_box);

            // Optional Footer Action Bar
            if (!item.footer_actions.empty()) {
                auto f_div = container();
                f_div->color(opts.divider_color).height(1.0f).width(StyleValue::percent(100.0f));
                panel_col_items.push_back(f_div);

                auto actions_row = row(item.footer_actions);
                actions_row->justifyContent(Justify::End)
                           .gap(StyleValue::point(10.0f))
                           .alignItems(Align::Center)
                           .width(StyleValue::percent(100.0f));

                auto footer_box = container(actions_row);
                footer_box->paddingSymmetric(10.0f, 16.0f)
                          .color(0x330F172A)
                          .width(StyleValue::percent(100.0f));
                panel_col_items.push_back(footer_box);
            }
        }

        auto panel_col = column(panel_col_items);
        panel_col->width(StyleValue::percent(100.0f));

        // ── 4. Card Styling & Elevation ───────────────────────────────
        auto card = container(panel_col);
        card->color(opts.background_color)
            .border(is_expanded ? opts.expanded_border_col : opts.border_color, 1.0f)
            .borderRadius(opts.border_radius)
            .width(StyleValue::percent(100.0f))
            .shadow(is_expanded ? BoxShadow(0x99000000, {0.0f, 8.0f}, 24.0f)
                                : BoxShadow(0x44000000, {0.0f, 2.0f}, 8.0f));

        return card;
    }

    WidgetPtr build(BuildContext&) override {
        auto* w = static_cast<const ExpansionPanelListWidget*>(widget());
        const auto& opts = w->props;

        std::vector<WidgetPtr> panel_cards;
        for (size_t i = 0; i < w->props.panels.size(); ++i) {
            auto p = buildPanelCard(static_cast<int>(i), w->props.panels[i], opts);
            panel_cards.push_back(p);
        }

        auto main_col = column(panel_cards);
        main_col->gap(StyleValue::point(opts.gap))
                .width(StyleValue::percent(100.0f));

        return main_col;
    }
};

std::unique_ptr<State> ExpansionPanelListWidget::createState() {
    return std::make_unique<ExpansionPanelListState>();
}

} // namespace enki
