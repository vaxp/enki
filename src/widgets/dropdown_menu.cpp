/// @file dropdown_menu.cpp
/// @brief Implementation of Advanced In-Window Overlay DropdownMenu widget.
///
/// Architecture (exactly like Drawer and BottomSheet):
///   Stack (100% x 100%)
///     ├── Positioned::fill → body_widget       (invariant — triggers live inside body)
///     ├── Scrim (transparent click-catcher)     (when open: dismisses on click-outside)
///     └── Positioned(anchor_x, anchor_y)        (floating menu panel above everything)
///
/// NOTE: The trigger button is INSIDE the body content. It calls controller->toggle()
///       or controller->open(). DropdownMenu itself only adds the floating panel layer.

#include "enki/widgets/dropdown_menu.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/stack.hpp"
#include "enki/state/state.hpp"
#include "enki/tree/build_context.hpp"
#include "enki/rendering/canvas.hpp"
#include "enki/rendering/paint.hpp"
#include "enki/platform/platform.hpp"

#include <algorithm>
#include <iostream>
#include <cmath>

namespace enki {

// ════════════════════════════════════════════════════════════════
// RenderDropdownScrim — transparent full-window click-catcher
// ════════════════════════════════════════════════════════════════

class RenderDropdownScrim : public RenderBox {
public:
    std::function<void()> on_tap;

    explicit RenderDropdownScrim(std::function<void()> tap)
        : on_tap(std::move(tap)) {
        ANUNodeStyleSetPositionType(anu_node_, ANUPositionTypeAbsolute);
        ANUNodeStyleSetPosition(anu_node_, ANUEdgeTop,    0.0f);
        ANUNodeStyleSetPosition(anu_node_, ANUEdgeLeft,   0.0f);
        ANUNodeStyleSetPosition(anu_node_, ANUEdgeRight,  0.0f);
        ANUNodeStyleSetPosition(anu_node_, ANUEdgeBottom, 0.0f);
    }

    void paint(PaintContext&) override {}
    bool hitTestSelf(Point) const override { return true; }
    void handlePointerDown(const PointerEvent&) override {
        if (on_tap) on_tap();
    }
};

class DropdownScrimWidget : public SingleChildRenderObjectWidget {
public:
    std::function<void()> on_tap;

    explicit DropdownScrimWidget(std::function<void()> tap)
        : SingleChildRenderObjectWidget(Key::none()), on_tap(std::move(tap)) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext&) override {
        return std::make_unique<RenderDropdownScrim>(on_tap);
    }

    void updateRenderObject(BuildContext&, RenderObject& ro) override {
        auto& r = static_cast<RenderDropdownScrim&>(ro);
        r.on_tap = on_tap;
    }

    [[nodiscard]] std::string_view typeName() const override { return "DropdownScrimWidget"; }
};

// ════════════════════════════════════════════════════════════════
// DropdownMenuState
// ════════════════════════════════════════════════════════════════

class DropdownMenuState : public State {
private:
    bool        is_open_      = false;
    std::string selected_id_  = "";
    int         hovered_idx_  = -1;
    SlotId      key_conn_     = 0;

public:
    void initState() override {
        State::initState();
        auto* w = static_cast<const DropdownMenuWidget*>(widget());
        selected_id_ = w->props.selected_id;
        wireController();

        if (Platform::instance()) {
            key_conn_ = Platform::instance()->onKeyDown().connect([this](int key, int) {
                if (key == 0xff1b && is_open_) closeMenu();
            });
        }
    }

    void didUpdateWidget(const Widget& old) override {
        State::didUpdateWidget(old);
        wireController();
    }

    void dispose() override {
        if (Platform::instance() && key_conn_)
            Platform::instance()->onKeyDown().disconnect(key_conn_);
        State::dispose();
    }

    void wireController() {
        auto* w = static_cast<const DropdownMenuWidget*>(widget());
        if (!w->props.controller) return;
        w->props.controller->open_fn         = [this] { openMenu(); };
        w->props.controller->close_fn        = [this] { closeMenu(); };
        w->props.controller->toggle_fn       = [this] { toggleMenu(); };
        w->props.controller->select_fn       = [this](const std::string& id) { selectItem(id); };
        w->props.controller->is_open_fn      = [this] { return is_open_; };
        w->props.controller->get_selected_fn = [this] { return selected_id_; };
    }

    void openMenu() {
        if (is_open_) return;
        is_open_ = true;
        hovered_idx_ = -1;
        auto* w = static_cast<const DropdownMenuWidget*>(widget());
        if (w->props.on_opened) w->props.on_opened();
        setState([] {});
    }

    void closeMenu() {
        if (!is_open_) return;
        is_open_ = false;
        auto* w = static_cast<const DropdownMenuWidget*>(widget());
        if (w->props.on_closed) w->props.on_closed();
        setState([] {});
    }

    void toggleMenu() { if (is_open_) closeMenu(); else openMenu(); }

    void selectItem(const std::string& id) {
        selected_id_ = id;
        auto* w = static_cast<const DropdownMenuWidget*>(widget());
        for (const auto& item : w->props.items) {
            if (item.id == id && w->props.on_selected) {
                w->props.on_selected(item);
                break;
            }
        }
        if (w->props.close_on_select) closeMenu();
        else setState([] {});
    }

    const DropdownMenuItem* getSelected() const {
        auto* w = static_cast<const DropdownMenuWidget*>(widget());
        for (const auto& it : w->props.items)
            if (it.id == selected_id_) return &it;
        return nullptr;
    }

    // ── Menu Item Row Builder ─────────────────────────────────────

    WidgetPtr buildItemRow(const DropdownMenuItem& item, int idx,
                           const DropdownMenuProps& opts) {
        if (item.type == DropdownMenuItemType::Divider) {
            auto div = container();
            div->color(opts.divider_color).height(1.0f)
               .marginSymmetric(4.0f, 0.0f)
               .width(StyleValue::percent(100.0f));
            return div;
        }

        if (item.type == DropdownMenuItemType::Header) {
            auto hdr = text(item.label);
            hdr->fontSize(10.5f).bold().color(opts.header_color);
            auto hdr_box = container(hdr);
            hdr_box->padding(EdgeInsets(6.0f, 10.0f, 2.0f, 10.0f))
                   .width(StyleValue::percent(100.0f));
            return hdr_box;
        }

        // Left: check/radio + icon + label+subtitle
        std::vector<WidgetPtr> left;
        if (item.type == DropdownMenuItemType::Checkbox) {
            auto c = text(item.is_checked ? "☑" : "☐");
            c->fontSize(14.0f).color(item.is_checked ? 0xFF38BDF8 : 0xFF64748B);
            left.push_back(c);
        } else if (item.type == DropdownMenuItemType::Radio) {
            auto r = text(item.is_checked ? "◉" : "○");
            r->fontSize(13.0f).color(item.is_checked ? 0xFF38BDF8 : 0xFF64748B);
            left.push_back(r);
        }

        if (!item.leading_icon.empty()) {
            auto ic = text(item.leading_icon);
            ic->fontSize(13.0f);
            left.push_back(ic);
        }

        std::vector<WidgetPtr> txt_col_items;
        Color lbl_col = item.is_disabled ? 0xFF64748B
                      : (item.is_danger ? opts.danger_color : opts.text_color);
        auto lbl = text(item.label);
        lbl->fontSize(12.5f).color(lbl_col);
        if (item.id == selected_id_ && !selected_id_.empty()) lbl->bold();
        txt_col_items.push_back(lbl);
        if (!item.subtitle.empty()) {
            auto sub = text(item.subtitle);
            sub->fontSize(10.5f).color(opts.subtitle_color);
            txt_col_items.push_back(sub);
        }
        auto txt_col = column(txt_col_items);
        txt_col->gap(StyleValue::point(1.0f));
        left.push_back(txt_col);

        auto left_row = row(left);
        left_row->gap(StyleValue::point(7.0f)).alignItems(Align::Center).flex(1.0f);

        // Right: badge + shortcut
        std::vector<WidgetPtr> right;
        if (!item.badge_text.empty()) {
            auto bd = text(item.badge_text);
            bd->fontSize(9.5f).bold().color(item.badge_fg);
            auto bd_box = container(bd);
            bd_box->color(item.badge_bg).borderRadius(3.0f).paddingSymmetric(2.0f, 5.0f);
            right.push_back(bd_box);
        }
        if (!item.trailing_shortcut.empty()) {
            auto sc = text(item.trailing_shortcut);
            sc->fontSize(10.5f).color(opts.shortcut_color);
            right.push_back(sc);
        }
        auto right_row = row(right);
        right_row->gap(StyleValue::point(5.0f)).alignItems(Align::Center);

        std::vector<WidgetPtr> full_items = {left_row, right_row};
        auto full_row = row(full_items);
        full_row->justifyContent(Justify::SpaceBetween)
                .alignItems(Align::Center)
                .width(StyleValue::percent(100.0f));

        bool is_hov = (hovered_idx_ == idx);
        bool is_sel = (!selected_id_.empty() && item.id == selected_id_);
        Color bg = is_hov ? opts.hover_color : (is_sel ? 0x1538BDF8 : 0x00000000);

        auto item_box = container(full_row);
        item_box->color(bg).borderRadius(4.0f)
                .paddingSymmetric(6.0f, 10.0f)
                .width(StyleValue::percent(100.0f));

        if (item.is_disabled) return item_box;

        auto gd = std::make_shared<GestureDetector>(item_box);
        gd->cursor_type = SystemCursor::Pointer;
        gd->on_tap_up = [this, item](const TapUpDetails&) {
            auto* w = static_cast<const DropdownMenuWidget*>(widget());
            if ((item.type == DropdownMenuItemType::Checkbox ||
                 item.type == DropdownMenuItemType::Radio) &&
                w->props.on_toggle_checked) {
                w->props.on_toggle_checked(item.id, !item.is_checked);
            }
            selectItem(item.id);
        };
        gd->on_hover_enter = [this, idx](const PointerEvent&) {
            hovered_idx_ = idx; setState([] {});
        };
        gd->on_hover_exit = [this](const PointerEvent&) {
            hovered_idx_ = -1; setState([] {});
        };
        return gd;
    }

    // ── Floating Panel Builder ────────────────────────────────────

    WidgetPtr buildFloatingPanel(const DropdownMenuWidget* w) {
        const auto& opts = w->props;

        std::vector<WidgetPtr> list_items;
        int idx = 0;
        for (const auto& it : w->props.items) {
            list_items.push_back(buildItemRow(it, idx, opts));
            idx++;
        }

        auto items_col = column(list_items);
        items_col->gap(StyleValue::point(1.0f))
                 .width(StyleValue::percent(100.0f));

        auto menu_panel = container(items_col);
        menu_panel->color(opts.background_color)
                  .border(opts.border_color, 1.0f)
                  .borderRadius(opts.border_radius)
                  .paddingAll(6.0f)
                  .width(opts.menu_width)
                  .shadow(BoxShadow(0x99000000, {0.0f, 8.0f}, 20.0f));

        // Floating panel positioned at anchor (below trigger)
        return Positioned {
            .child = menu_panel,
            .top = StyleValue::point(opts.anchor_y + opts.trigger_height + 4.0f),
            .left = StyleValue::point(opts.anchor_x),
            .width = StyleValue::point(opts.menu_width),
        };
    }

    // ── build() ──────────────────────────────────────────────────

    WidgetPtr build(BuildContext&) override {
        auto* w = static_cast<const DropdownMenuWidget*>(widget());

        // ── Body (always Positioned::fill — trigger lives INSIDE body) ──
        WidgetPtr body_fill;
        if (w->props.body) {
            auto bx = container(w->props.body);
            bx->flex(1.0f)
               .width(StyleValue::percent(100.0f))
               .height(StyleValue::percent(100.0f));
            body_fill = Positioned::fill(bx);
        } else {
            auto empty = container();
            empty->flex(1.0f)
                 .width(StyleValue::percent(100.0f))
                 .height(StyleValue::percent(100.0f));
            body_fill = Positioned::fill(empty);
        }

        // Closed: just render the body as-is (trigger is inside body)
        if (!is_open_) {
            return Stack {
                .width  = StyleValue::percent(100.0f),
                .height = StyleValue::percent(100.0f),
                .children = { body_fill },
            };
        }

        // Open: body + transparent scrim + floating panel
        auto scrim = std::make_shared<DropdownScrimWidget>([this] { closeMenu(); });
        auto floating = buildFloatingPanel(w);

        return Stack {
            .width  = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .children = { body_fill, scrim, floating },
        };
    }
};

std::unique_ptr<State> DropdownMenuWidget::createState() {
    return std::make_unique<DropdownMenuState>();
}

} // namespace enki
