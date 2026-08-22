/// @file menu.cpp
/// @brief Implementation of Menu and MenuBar widgets with cascading submenus on NativePopup.

#include "enki/widgets/menu.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/state/state.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/rendering/canvas.hpp"

#include <include/core/SkCanvas.h>
#include <include/core/SkPaint.h>
#include <include/core/SkRRect.h>
#include <include/core/SkMaskFilter.h>
#include <include/core/SkBlurTypes.h>

#include <iostream>
#include <cmath>

namespace enki {

// ════════════════════════════════════════════════════════════════
// Custom RenderBox for Menu Surface Background
// ════════════════════════════════════════════════════════════════

class RenderMenuBackground : public RenderBox {
public:
    MenuOptions options;

    explicit RenderMenuBackground(MenuOptions opt)
        : options(std::move(opt)) {}

    void paint(PaintContext& ctx) override {
        SkCanvas* canvas = static_cast<SkCanvas*>(ctx.canvas.getNativeHandle());
        if (!canvas) return;
        if (size_.width <= 0.0f || size_.height <= 0.0f) return;

        SkRect rect = SkRect::MakeXYWH(ctx.offset.x, ctx.offset.y, size_.width, size_.height);
        SkRRect rrect;
        rrect.setRectXY(rect, options.border_radius, options.border_radius);

        // 1. Drop Shadow
        if (options.elevation > 0.0f) {
            SkPaint shadow_paint;
            shadow_paint.setAntiAlias(true);
            shadow_paint.setColor(0x70000000);
            shadow_paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, options.elevation * 0.5f));

            canvas->save();
            canvas->translate(0, options.elevation * 0.3f);
            canvas->drawRRect(rrect, shadow_paint);
            canvas->restore();
        }

        // 2. Background Fill
        SkPaint bg_paint;
        bg_paint.setAntiAlias(true);
        bg_paint.setColor(options.background_color);
        canvas->drawRRect(rrect, bg_paint);

        // 3. Border Stroke
        if (options.border_width > 0.0f) {
            SkPaint border_paint;
            border_paint.setAntiAlias(true);
            border_paint.setStyle(SkPaint::kStroke_Style);
            border_paint.setStrokeWidth(options.border_width);
            border_paint.setColor(options.border_color);
            canvas->drawRRect(rrect, border_paint);
        }

        // 4. Paint Children
        if (!children().empty()) {
            RenderBox* child = static_cast<RenderBox*>(children()[0]);
            PaintContext child_ctx = ctx.withOffset(child->offset());
            child->paint(child_ctx);
        }
    }

    [[nodiscard]] bool hitTestSelf(Point localPoint) const override {
        return localPoint.x >= 0 && localPoint.x <= size_.width &&
               localPoint.y >= 0 && localPoint.y <= size_.height;
    }
};

class MenuBackgroundWidget : public SingleChildRenderObjectWidget {
public:
    MenuOptions options;

    MenuBackgroundWidget(MenuOptions opt, WidgetPtr child)
        : SingleChildRenderObjectWidget(Key::none(), std::move(child)),
          options(std::move(opt)) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext&) override {
        return std::make_unique<RenderMenuBackground>(options);
    }

    void updateRenderObject(BuildContext&, RenderObject& renderObject) override {
        if (auto* rb = dynamic_cast<RenderMenuBackground*>(&renderObject)) {
            rb->options = options;
            rb->markNeedsPaint();
        }
    }

    [[nodiscard]] std::string_view typeName() const override { return "MenuBackgroundWidget"; }
};

// ════════════════════════════════════════════════════════════════
// MenuItem Widget & Cascading Submenu Management
// ════════════════════════════════════════════════════════════════

class MenuItemWidget : public StatefulWidget {
public:
    MenuItem item;
    MenuOptions options;
    std::shared_ptr<NativePopup> parent_popup;
    std::function<void()> close_all_callback;
    std::function<void(std::shared_ptr<NativePopup>)> on_submenu_opened;

    MenuItemWidget(MenuItem item, MenuOptions options,
                   std::shared_ptr<NativePopup> parent_popup,
                   std::function<void()> close_all_callback,
                   std::function<void(std::shared_ptr<NativePopup>)> on_submenu_opened)
        : item(std::move(item)), options(std::move(options)),
          parent_popup(std::move(parent_popup)),
          close_all_callback(std::move(close_all_callback)),
          on_submenu_opened(std::move(on_submenu_opened)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "MenuItemWidget"; }
};

// Forward declaration of MenuContentWidget for submenus
class MenuContentWidget;

class MenuItemWidgetState : public State {
private:
    bool hovered_ = false;
    std::shared_ptr<NativePopup> active_submenu_ = nullptr;

    void openSubmenu(Rect anchor_bounds) {
        auto* w = static_cast<const MenuItemWidget*>(widget());
        if (w->item.type != MenuItemType::Submenu || w->item.submenu_items.empty()) return;

        BuildContext ctx(element());
        Size screen_sz = ctx.mediaSize();

        int pop_w = static_cast<int>(w->options.min_width);
        int pop_h = static_cast<int>(w->item.submenu_items.size() * 32.0f + 16.0f);

        float sub_x = anchor_bounds.x + anchor_bounds.width + 2.0f;
        float sub_y = anchor_bounds.y - 4.0f;

        // Reposition to left if overflowing screen
        if (sub_x + pop_w > screen_sz.width - 10.0f) {
            sub_x = anchor_bounds.x - pop_w - 2.0f;
        }
        if (sub_y + pop_h > screen_sz.height - 10.0f) {
            sub_y = screen_sz.height - pop_h - 10.0f;
        }
        if (sub_x < 5.0f) sub_x = 5.0f;
        if (sub_y < 5.0f) sub_y = 5.0f;

        PopupOptions opts;
        opts.position = {sub_x, sub_y};
        opts.width = pop_w;
        opts.height = pop_h;
        opts.auto_dismiss = true;

        auto items = w->item.submenu_items;
        auto opt = w->options;
        auto close_all = w->close_all_callback;

        active_submenu_ = NativePopup::show(ctx, opts, [items, opt, close_all](BuildContext&, std::shared_ptr<NativePopup> sub_popup) {
            std::vector<WidgetPtr> item_widgets;
            for (const auto& itm : items) {
                if (itm.type == MenuItemType::Divider) {
                    auto div = container();
                    div->height(1.0f).color(opt.border_color).marginSymmetric(3.0f, 0);
                    item_widgets.push_back(div);
                } else {
                    item_widgets.push_back(std::make_shared<MenuItemWidget>(itm, opt, sub_popup, close_all, nullptr));
                }
            }

            auto col = column(std::move(item_widgets));
            col->gap(StyleValue::point(2.0f));

            auto inner = container(col);
            inner->padding(opt.padding);

            return std::make_shared<MenuBackgroundWidget>(opt, inner);
        });

        if (w->on_submenu_opened) {
            w->on_submenu_opened(active_submenu_);
        }
    }

public:
    WidgetPtr build(BuildContext&) override {
        auto* w = static_cast<const MenuItemWidget*>(widget());
        const auto& itm = w->item;
        const auto& opt = w->options;

        // 1. Leading Icon / State indicator
        std::string lead_text = "";
        Color lead_color = itm.enabled ? opt.accent_color : opt.disabled_color;

        if (itm.type == MenuItemType::Checkbox) {
            lead_text = itm.checked ? "✓" : "  ";
        } else if (itm.type == MenuItemType::Radio) {
            lead_text = itm.checked ? "●" : "○";
        } else if (!itm.icon.empty()) {
            lead_text = itm.icon;
        }

        auto lead_w = text(lead_text);
        lead_w->fontSize(12.0f).bold().color(lead_color);

        auto lead_box = container(lead_w);
        lead_box->width(20.0f).align(Alignment::Center);

        // 2. Label Text
        auto label_w = text(itm.label);
        label_w->fontSize(13.0f)
               .color(itm.enabled ? (hovered_ ? 0xFFFFFFFF : opt.text_color) : opt.disabled_color);

        // 3. Trailing Shortcut / Submenu arrow
        std::string trail_text = "";
        if (itm.type == MenuItemType::Submenu) {
            trail_text = "▶";
        } else if (!itm.shortcut.empty()) {
            trail_text = itm.shortcut;
        }

        auto trail_w = text(trail_text);
        trail_w->fontSize(11.0f).color(opt.text_sec_color);

        // Build item Row
        auto item_row = row({lead_box, label_w});
        item_row->gap(StyleValue::point(8.0f))
                .alignItems(Align::Center);

        auto full_row = row({item_row, trail_w});
        full_row->justifyContent(Justify::SpaceBetween)
                .alignItems(Align::Center);

        auto box = container(full_row);
        Color item_bg = (hovered_ && itm.enabled) ? opt.item_hover_color : 0x00000000;
        box->color(item_bg)
           .borderRadius(5.0f)
           .paddingSymmetric(5.0f, 8.0f);

        auto detector = std::make_shared<GestureDetector>();
        detector->hit_test_behavior = HitTestBehavior::Opaque;
        detector->cursor_type       = itm.enabled ? SystemCursor::Pointer : SystemCursor::Default;

        detector->on_hover_enter = [this, w](const PointerEvent&) {
            if (!w->item.enabled) return;
            setState([this] { hovered_ = true; });

            if (w->item.type == MenuItemType::Submenu) {
                Rect bounds{0, 0, 180.0f, 30.0f};
                if (auto* ro = context().element()->findRenderObject()) {
                    bounds = ro->globalBounds();
                }
                openSubmenu(bounds);
            }
        };

        detector->on_hover_exit = [this](const PointerEvent&) {
            setState([this] { hovered_ = false; });
        };

        detector->on_tap = [this, w] {
            if (!w->item.enabled) return;

            if (w->item.type == MenuItemType::Action) {
                if (w->item.on_selected) w->item.on_selected();
                if (w->close_all_callback) w->close_all_callback();
                else if (w->parent_popup) w->parent_popup->close();
            } else if (w->item.type == MenuItemType::Checkbox) {
                if (w->item.on_toggle) w->item.on_toggle(!w->item.checked);
                if (w->close_all_callback) w->close_all_callback();
                else if (w->parent_popup) w->parent_popup->close();
            } else if (w->item.type == MenuItemType::Radio) {
                if (w->item.on_selected) w->item.on_selected();
                if (w->close_all_callback) w->close_all_callback();
                else if (w->parent_popup) w->parent_popup->close();
            }
        };

        detector->child = box;
        return detector;
    }
};

std::unique_ptr<State> MenuItemWidget::createState() {
    return std::make_unique<MenuItemWidgetState>();
}

// ════════════════════════════════════════════════════════════════
// MenuBar State Implementation
// ════════════════════════════════════════════════════════════════

class MenuBarButton : public StatefulWidget {
public:
    std::string label;
    bool is_active;
    MenuOptions options;
    std::function<void(Rect)> on_open;
    std::function<void(Rect)> on_hover_active;

    MenuBarButton(std::string label, bool active, MenuOptions opt,
                  std::function<void(Rect)> on_open,
                  std::function<void(Rect)> on_hover_active)
        : label(std::move(label)), is_active(active), options(std::move(opt)),
          on_open(std::move(on_open)), on_hover_active(std::move(on_hover_active)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "MenuBarButton"; }
};

class MenuBarButtonState : public State {
private:
    bool hovered_ = false;

public:
    WidgetPtr build(BuildContext&) override {
        auto* btn = static_cast<const MenuBarButton*>(widget());

        auto label_w = text(btn->label);
        label_w->fontSize(13.0f).bold()
               .color((hovered_ || btn->is_active) ? 0xFFFFFFFF : btn->options.text_color);

        auto box = container(label_w);
        Color bg = btn->is_active ? btn->options.item_hover_color : (hovered_ ? 0x30FFFFFF : 0x00000000);
        box->color(bg)
           .borderRadius(6.0f)
           .paddingSymmetric(5.0f, 10.0f);

        auto detector = std::make_shared<GestureDetector>();
        detector->hit_test_behavior = HitTestBehavior::Opaque;
        detector->cursor_type       = SystemCursor::Pointer;

        detector->on_hover_enter = [this, btn](const PointerEvent&) {
            setState([this] { hovered_ = true; });
            if (btn->on_hover_active) {
                Rect b{0, 0, 80.0f, 30.0f};
                if (auto* ro = context().element()->findRenderObject()) b = ro->globalBounds();
                btn->on_hover_active(b);
            }
        };

        detector->on_hover_exit = [this](const PointerEvent&) {
            setState([this] { hovered_ = false; });
        };

        detector->on_tap = [this, btn] {
            if (btn->on_open) {
                Rect b{0, 0, 80.0f, 30.0f};
                if (auto* ro = context().element()->findRenderObject()) b = ro->globalBounds();
                btn->on_open(b);
            }
        };

        detector->child = box;
        return detector;
    }
};

std::unique_ptr<State> MenuBarButton::createState() {
    return std::make_unique<MenuBarButtonState>();
}

class MenuBarState : public State {
private:
    int active_index_ = -1;
    std::shared_ptr<NativePopup> active_popup_ = nullptr;
    std::shared_ptr<NativePopup> active_child_submenu_ = nullptr;

    void closeAll() {
        if (active_child_submenu_) {
            active_child_submenu_->close();
            active_child_submenu_ = nullptr;
        }
        if (active_popup_) {
            active_popup_->close();
            active_popup_ = nullptr;
        }
        setState([this] { active_index_ = -1; });
    }

    void openMenu(int index, Rect anchor_bounds) {
        auto* bar = static_cast<const MenuBarWidget*>(widget());
        if (index < 0 || index >= static_cast<int>(bar->entries.size())) return;

        if (active_index_ == index && active_popup_) {
            closeAll();
            return;
        }

        if (active_child_submenu_) {
            active_child_submenu_->close();
            active_child_submenu_ = nullptr;
        }
        if (active_popup_) {
            active_popup_->close();
            active_popup_ = nullptr;
        }

        active_index_ = index;
        setState([this] {});

        BuildContext ctx(element());
        Size screen_sz = ctx.mediaSize();

        const auto& entry = bar->entries[index];
        const auto& opt = bar->options;

        int pop_w = static_cast<int>(opt.min_width);
        int pop_h = static_cast<int>(entry.items.size() * 32.0f + 16.0f);

        float pop_x = anchor_bounds.x;
        float pop_y = anchor_bounds.y + anchor_bounds.height + 4.0f;

        if (pop_x + pop_w > screen_sz.width - 10.0f) {
            pop_x = screen_sz.width - pop_w - 10.0f;
        }
        if (pop_x < 5.0f) pop_x = 5.0f;

        PopupOptions pop_opts;
        pop_opts.position = {pop_x, pop_y};
        pop_opts.width = pop_w;
        pop_opts.height = pop_h;
        pop_opts.auto_dismiss = opt.auto_dismiss;
        pop_opts.on_close = [this]() {
            active_popup_ = nullptr;
            setState([this] { active_index_ = -1; });
        };

        auto items = entry.items;

        active_popup_ = NativePopup::show(ctx, pop_opts, [this, items, opt](BuildContext&, std::shared_ptr<NativePopup> parent_popup) {
            std::vector<WidgetPtr> item_widgets;
            for (const auto& itm : items) {
                if (itm.type == MenuItemType::Divider) {
                    auto div = container();
                    div->height(1.0f).color(opt.border_color).marginSymmetric(3.0f, 0);
                    item_widgets.push_back(div);
                } else {
                    item_widgets.push_back(std::make_shared<MenuItemWidget>(
                        itm, opt, parent_popup,
                        [this]() { closeAll(); },
                        [this](std::shared_ptr<NativePopup> sub) { active_child_submenu_ = sub; }
                    ));
                }
            }

            auto col = column(std::move(item_widgets));
            col->gap(StyleValue::point(2.0f));

            auto inner = container(col);
            inner->padding(opt.padding);

            return std::make_shared<MenuBackgroundWidget>(opt, inner);
        });
    }

public:
    void dispose() override {
        closeAll();
        State::dispose();
    }

    WidgetPtr build(BuildContext&) override {
        auto* bar = static_cast<const MenuBarWidget*>(widget());

        std::vector<WidgetPtr> btn_widgets;
        for (size_t i = 0; i < bar->entries.size(); ++i) {
            int idx = static_cast<int>(i);
            const auto& ent = bar->entries[i];

            auto btn = std::make_shared<MenuBarButton>(
                ent.label,
                active_index_ == idx,
                bar->options,
                [this, idx](Rect b) { openMenu(idx, b); },
                [this, idx](Rect b) {
                    if (active_index_ >= 0 && active_index_ != idx) {
                        openMenu(idx, b);
                    }
                }
            );
            btn_widgets.push_back(btn);
        }

        auto bar_row = row(std::move(btn_widgets));
        bar_row->gap(StyleValue::point(4.0f))
               .alignItems(Align::Center);

        auto bar_box = container(bar_row);
        bar_box->color(bar->options.background_color)
               .borderRadius(bar->options.border_radius)
               .border(bar->options.border_color, bar->options.border_width)
               .paddingSymmetric(4.0f, 8.0f);

        return bar_box;
    }
};

std::unique_ptr<State> MenuBarWidget::createState() {
    return std::make_unique<MenuBarState>();
}

// ════════════════════════════════════════════════════════════════
// Standalone Menu State Implementation
// ════════════════════════════════════════════════════════════════

class MenuState : public State {
private:
    std::shared_ptr<NativePopup> active_popup_ = nullptr;

    void openMenu(Rect anchor_bounds) {
        if (active_popup_) {
            active_popup_->close();
            active_popup_ = nullptr;
            return;
        }

        auto* menu_widget = static_cast<const MenuWidget*>(widget());
        BuildContext ctx(element());
        Size screen_sz = ctx.mediaSize();

        const auto& opt = menu_widget->options;
        int pop_w = static_cast<int>(opt.min_width);
        int pop_h = static_cast<int>(menu_widget->items.size() * 32.0f + 16.0f);

        float pop_x = anchor_bounds.x;
        float pop_y = anchor_bounds.y + anchor_bounds.height + 4.0f;

        if (pop_x + pop_w > screen_sz.width - 10.0f) {
            pop_x = screen_sz.width - pop_w - 10.0f;
        }
        if (pop_x < 5.0f) pop_x = 5.0f;

        PopupOptions pop_opts;
        pop_opts.position = {pop_x, pop_y};
        pop_opts.width = pop_w;
        pop_opts.height = pop_h;
        pop_opts.auto_dismiss = opt.auto_dismiss;
        pop_opts.on_close = [this]() {
            active_popup_ = nullptr;
        };

        auto items = menu_widget->items;

        active_popup_ = NativePopup::show(ctx, pop_opts, [this, items, opt](BuildContext&, std::shared_ptr<NativePopup> parent_popup) {
            std::vector<WidgetPtr> item_widgets;
            for (const auto& itm : items) {
                if (itm.type == MenuItemType::Divider) {
                    auto div = container();
                    div->height(1.0f).color(opt.border_color).marginSymmetric(3.0f, 0);
                    item_widgets.push_back(div);
                } else {
                    item_widgets.push_back(std::make_shared<MenuItemWidget>(
                        itm, opt, parent_popup,
                        [this]() {
                            if (active_popup_) {
                                active_popup_->close();
                                active_popup_ = nullptr;
                            }
                        },
                        nullptr
                    ));
                }
            }

            auto col = column(std::move(item_widgets));
            col->gap(StyleValue::point(2.0f));

            auto inner = container(col);
            inner->padding(opt.padding);

            return std::make_shared<MenuBackgroundWidget>(opt, inner);
        });
    }

public:
    void dispose() override {
        if (active_popup_) {
            active_popup_->close();
            active_popup_ = nullptr;
        }
        State::dispose();
    }

    WidgetPtr build(BuildContext&) override {
        auto* menu_widget = static_cast<const MenuWidget*>(widget());

        auto detector = std::make_shared<GestureDetector>();
        detector->hit_test_behavior = HitTestBehavior::Opaque;
        detector->cursor_type       = SystemCursor::Pointer;

        detector->on_tap = [this] {
            Rect b{0, 0, 100.0f, 32.0f};
            if (auto* ro = context().element()->findRenderObject()) b = ro->globalBounds();
            openMenu(b);
        };

        detector->child = menu_widget->child;
        return detector;
    }
};

std::unique_ptr<State> MenuWidget::createState() {
    return std::make_unique<MenuState>();
}

} // namespace enki
