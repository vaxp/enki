/// @file combo_box.cpp
/// @brief Implementation of Advanced ComboBox widget for ENKI Framework.

#include "enki/widgets/combo_box.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/stack.hpp"
#include "enki/rendering/canvas.hpp"
#include "enki/rendering/paint.hpp"
#include "enki/state/state.hpp"
#include "enki/tree/build_context.hpp"
#include "enki/platform/platform.hpp"

#include <algorithm>
#include <iostream>
#include <vector>
#include <set>
#include <cctype>

namespace enki {

// ════════════════════════════════════════════════════════════════
// RenderComboBoxScrim — Transparent backdrop to close on outside click
// ════════════════════════════════════════════════════════════════

class RenderComboBoxScrim : public RenderBox {
public:
    std::function<void()> on_tap;

    explicit RenderComboBoxScrim(std::function<void()> tap) : on_tap(std::move(tap)) {
        ANUNodeStyleSetPositionType(anu_node_, ANUPositionTypeAbsolute);
        ANUNodeStyleSetPosition(anu_node_, ANUEdgeTop, 0.0f);
        ANUNodeStyleSetPosition(anu_node_, ANUEdgeLeft, 0.0f);
        ANUNodeStyleSetPosition(anu_node_, ANUEdgeRight, 0.0f);
        ANUNodeStyleSetPosition(anu_node_, ANUEdgeBottom, 0.0f);
    }

    void paint(PaintContext&) override {} // Transparent
    bool hitTestSelf(Point) const override { return true; }
    void handlePointerDown(const PointerEvent&) override {
        if (on_tap) on_tap();
    }
};

class ComboBoxScrimWidget : public SingleChildRenderObjectWidget {
public:
    std::function<void()> on_tap;
    explicit ComboBoxScrimWidget(std::function<void()> tap)
        : SingleChildRenderObjectWidget(Key::none()), on_tap(std::move(tap)) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext&) override {
        return std::make_unique<RenderComboBoxScrim>(on_tap);
    }
    void updateRenderObject(BuildContext&, RenderObject& ro) override {
        auto& r = static_cast<RenderComboBoxScrim&>(ro);
        r.on_tap = on_tap;
    }
    [[nodiscard]] std::string_view typeName() const override { return "ComboBoxScrimWidget"; }
};

// ════════════════════════════════════════════════════════════════
// ComboBox State
// ════════════════════════════════════════════════════════════════

class ComboBoxState : public State {
private:
    bool is_open_ = false;
    std::string search_query_ = "";
    std::string selected_id_ = "";
    std::set<std::string> multi_selected_ids_;
    SlotId key_conn_ = 0;

public:
    void initState() override {
        State::initState();
        wireController();

        if (Platform::instance()) {
            key_conn_ = Platform::instance()->onKeyDown().connect([this](int key, int) {
                if (key == 0xff1b && is_open_) { // Escape
                    closeDropdown();
                }
            });
        }
    }

    void didUpdateWidget(const Widget& old) override {
        State::didUpdateWidget(old);
        wireController();
    }

    void dispose() override {
        if (Platform::instance() && key_conn_) {
            Platform::instance()->onKeyDown().disconnect(key_conn_);
        }
        State::dispose();
    }

    void wireController() {
        auto* w = static_cast<const ComboBoxWidget*>(widget());
        if (w->props.controller) {
            w->props.controller->select_fn = [this](const std::string& id) {
                selected_id_ = id;
                setState([] {});
            };
            w->props.controller->select_multi_fn = [this](const std::vector<std::string>& ids) {
                multi_selected_ids_.clear();
                for (const auto& id : ids) multi_selected_ids_.insert(id);
                setState([] {});
            };
            w->props.controller->clear_fn = [this] { clearSelection(); };
            w->props.controller->open_fn = [this] { openDropdown(); };
            w->props.controller->close_fn = [this] { closeDropdown(); };
            w->props.controller->toggle_fn = [this] { toggleDropdown(); };
            w->props.controller->get_value_fn = [this] { return selected_id_; };
            w->props.controller->get_multi_values_fn = [this] {
                return std::vector<std::string>(multi_selected_ids_.begin(), multi_selected_ids_.end());
            };
            w->props.controller->is_open_fn = [this] { return is_open_; };
        }
    }

    void openDropdown() {
        if (is_open_) return;
        is_open_ = true;
        setState([] {});
    }

    void closeDropdown() {
        if (!is_open_) return;
        is_open_ = false;
        setState([] {});
    }

    void toggleDropdown() {
        is_open_ = !is_open_;
        setState([] {});
    }

    void selectItem(const ComboBoxItem& it) {
        if (it.is_disabled) return;
        auto* w = static_cast<const ComboBoxWidget*>(widget());

        if (w->props.mode == ComboBoxMode::Single) {
            selected_id_ = it.id;
            closeDropdown();
            if (w->props.on_selected) w->props.on_selected(it);
        } else { // Multi
            if (multi_selected_ids_.count(it.id) > 0) {
                multi_selected_ids_.erase(it.id);
            } else {
                multi_selected_ids_.insert(it.id);
            }
            if (w->props.on_multi_changed) {
                std::vector<ComboBoxItem> selected_items;
                for (const auto& item : w->props.items) {
                    if (multi_selected_ids_.count(item.id) > 0) selected_items.push_back(item);
                }
                w->props.on_multi_changed(selected_items);
            }
        }
        setState([] {});
    }

    void removeMultiTag(const std::string& id) {
        auto* w = static_cast<const ComboBoxWidget*>(widget());
        multi_selected_ids_.erase(id);
        if (w->props.on_multi_changed) {
            std::vector<ComboBoxItem> selected_items;
            for (const auto& item : w->props.items) {
                if (multi_selected_ids_.count(item.id) > 0) selected_items.push_back(item);
            }
            w->props.on_multi_changed(selected_items);
        }
        setState([] {});
    }

    void clearSelection() {
        selected_id_.clear();
        multi_selected_ids_.clear();
        search_query_.clear();
        setState([] {});
    }

    // ── Build Floating Dropdown Menu Panel ────────────────────────

    WidgetPtr buildFloatingMenu(const ComboBoxWidget* w) {
        const auto& opts = w->props;

        std::vector<WidgetPtr> menu_rows;
        std::string last_group = "";

        for (const auto& item : w->props.items) {
            // Group Header (if new group category)
            if (!item.group.empty() && item.group != last_group) {
                last_group = item.group;
                auto g_txt = text(item.group);
                g_txt->fontSize(10.5f).bold().color(0xFF94A3B8);

                auto g_box = container(g_txt);
                g_box->paddingSymmetric(4.0f, 10.0f)
                     .color(0x330F172A)
                     .width(StyleValue::percent(100.0f));
                menu_rows.push_back(g_box);
            }

            // Item Row Left: Icon + Label + Subtitle
            std::vector<WidgetPtr> left_items;
            if (!item.icon.empty()) {
                auto ic = text(item.icon);
                ic->fontSize(14.0f);
                left_items.push_back(ic);
            }

            std::vector<WidgetPtr> txt_col_items;
            auto lbl = text(item.label);
            lbl->fontSize(12.5f).bold();

            bool is_selected = (opts.mode == ComboBoxMode::Single && selected_id_ == item.id) ||
                               (opts.mode == ComboBoxMode::Multi && multi_selected_ids_.count(item.id) > 0);

            lbl->color(item.is_disabled ? 0xFF64748B : (is_selected ? 0xFF38BDF8 : opts.text_color));
            txt_col_items.push_back(lbl);

            if (!item.subtitle.empty()) {
                auto sub = text(item.subtitle);
                sub->fontSize(11.0f).color(item.is_disabled ? 0xFF475569 : 0xFF94A3B8);
                txt_col_items.push_back(sub);
            }

            auto txt_col = column(txt_col_items);
            txt_col->gap(StyleValue::point(2.0f)).flex(1.0f);
            left_items.push_back(txt_col);

            auto left_row = row(left_items);
            left_row->gap(StyleValue::point(8.0f)).alignItems(Align::Center).flex(1.0f);

            // Item Row Right: Badge or Checkmark ✓
            std::vector<WidgetPtr> right_items;
            if (!item.badge.empty()) {
                auto b_txt = text(item.badge);
                b_txt->fontSize(10.0f).bold().color(item.badge_color);

                auto b_box = container(b_txt);
                b_box->color(0x2238BDF8).borderRadius(4.0f).paddingSymmetric(2.0f, 6.0f);
                right_items.push_back(b_box);
            }

            if (is_selected) {
                auto chk = text("✓");
                chk->fontSize(13.0f).bold().color(0xFF38BDF8);
                right_items.push_back(chk);
            }

            auto right_row = row(right_items);
            right_row->gap(StyleValue::point(6.0f)).alignItems(Align::Center);

            std::vector<WidgetPtr> row_items = {left_row, right_row};
            auto row_wrap = row(row_items);
            row_wrap->justifyContent(Justify::SpaceBetween)
                    .alignItems(Align::Center)
                    .width(StyleValue::percent(100.0f));

            auto item_box = container(row_wrap);
            item_box->color(is_selected ? opts.item_selected_col : 0x00000000)
                    .borderRadius(6.0f)
                    .paddingSymmetric(8.0f, 10.0f)
                    .width(StyleValue::percent(100.0f));

            auto item_gd = std::make_shared<GestureDetector>(item_box);
            if (!item.is_disabled) {
                item_gd->cursor_type = SystemCursor::Pointer;
                auto it_copy = item;
                item_gd->on_tap_up = [this, it_copy](const TapUpDetails&) {
                    selectItem(it_copy);
                };
            }
            menu_rows.push_back(item_gd);
        }

        auto menu_col = column(menu_rows);
        menu_col->gap(StyleValue::point(4.0f)).width(StyleValue::percent(100.0f));

        auto menu_panel = container(menu_col);
        menu_panel->color(opts.menu_bg_color)
                  .border(opts.border_color, 1.0f)
                  .borderRadius(opts.border_radius)
                  .paddingAll(8.0f)
                  .width(opts.width)
                  .shadow(BoxShadow(0x99000000, {0.0f, 10.0f}, 24.0f));

        return menu_panel;
    }

    WidgetPtr build(BuildContext&) override {
        auto* w = static_cast<const ComboBoxWidget*>(widget());
        const auto& opts = w->props;

        // ── 1. Invariant Page Body (100% dimensions) ──────────────────
        WidgetPtr body_widget;
        if (w->props.body) {
            auto bx = container(w->props.body);
            bx->width(StyleValue::percent(100.0f)).height(StyleValue::percent(100.0f));
            body_widget = Positioned::fill(bx);
        } else {
            auto empty = container();
            empty->width(StyleValue::percent(100.0f)).height(StyleValue::percent(100.0f));
            body_widget = Positioned::fill(empty);
        }

        std::vector<WidgetPtr> stack_items = {body_widget};

        // ── 2. Floating Dropdown Menu Panel (when open) ───────────────
        if (is_open_) {
            // Backdrop scrim to close on tap outside
            auto scrim = std::make_shared<ComboBoxScrimWidget>([this] {
                closeDropdown();
            });
            stack_items.push_back(scrim);

            auto menu_panel = buildFloatingMenu(w);
            stack_items.push_back(Positioned {
                .child = menu_panel,
                .top = StyleValue::point(opts.anchor_y + opts.input_height + 4.0f),
                .left = StyleValue::point(opts.anchor_x),
            });
        }

        return Stack {
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .children = std::move(stack_items),
        };
    }
};

std::unique_ptr<State> ComboBoxWidget::createState() {
    return std::make_unique<ComboBoxState>();
}

} // namespace enki
