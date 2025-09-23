/// @file context_menu.cpp
/// @brief Advanced Native ContextMenu implementation built on NativePopup.

#include "enki/widgets/context_menu.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/divider.hpp"
#include "enki/state/state.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/rendering/canvas.hpp"

#include <include/core/SkCanvas.h>
#include <include/core/SkPaint.h>
#include <include/core/SkRRect.h>
#include <include/core/SkPath.h>
#include <include/core/SkMaskFilter.h>
#include <include/core/SkBlurTypes.h>
#include <include/effects/SkRuntimeEffect.h>

#include <chrono>
#include <iostream>
#include <cmath>

namespace enki {

// ════════════════════════════════════════════════════════════════
// Custom RenderBox for ContextMenu Body
// ════════════════════════════════════════════════════════════════

class RenderContextMenuBackground : public RenderBox {
public:
    ContextMenuOptions options;

    explicit RenderContextMenuBackground(ContextMenuOptions opt)
        : options(std::move(opt)) {}

    void paint(PaintContext& ctx) override {
        SkCanvas* canvas = static_cast<SkCanvas*>(ctx.canvas.getNativeHandle());
        if (!canvas) return;
        if (size_.width <= 0.0f || size_.height <= 0.0f) return;

        SkRect rect = SkRect::MakeXYWH(ctx.offset.x, ctx.offset.y, size_.width, size_.height);
        SkRRect rrect;
        rrect.setRectXY(rect, options.border_radius, options.border_radius);

        // 1. Draw Drop Shadow
        if (options.elevation > 0.0f) {
            SkPaint shadow_paint;
            shadow_paint.setAntiAlias(true);
            shadow_paint.setColor(options.shadow_color);
            shadow_paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, options.elevation * 0.5f));

            canvas->save();
            canvas->translate(0, options.elevation * 0.3f);
            canvas->drawRRect(rrect, shadow_paint);
            canvas->restore();
        }

        // 2. Draw Background (Shader or Color)
        SkPaint bg_paint;
        bg_paint.setAntiAlias(true);

        if (!options.custom_shader.empty()) {
            auto [effect, err] = SkRuntimeEffect::MakeForShader(SkString(options.custom_shader.c_str()));
            if (effect) {
                struct Uniforms {
                    float time;
                    float resolution[2];
                } uniforms = {
                    0.0f,
                    {size_.width, size_.height}
                };
                sk_sp<SkData> uniform_data = SkData::MakeWithCopy(&uniforms, sizeof(Uniforms));
                bg_paint.setShader(effect->makeShader(uniform_data, nullptr, 0));
            } else {
                bg_paint.setColor(options.background_color);
            }
        } else {
            bg_paint.setColor(options.background_color);
        }

        canvas->drawRRect(rrect, bg_paint);

        // 3. Draw Border Stroke
        if (options.border_width > 0.0f) {
            SkPaint border_paint;
            border_paint.setAntiAlias(true);
            border_paint.setStyle(SkPaint::kStroke_Style);
            border_paint.setStrokeWidth(options.border_width);
            border_paint.setColor(options.border_color);

            canvas->drawRRect(rrect, border_paint);
        }

        // 4. Paint Child Content
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

class ContextMenuBackgroundWidget : public SingleChildRenderObjectWidget {
public:
    ContextMenuOptions options;

    ContextMenuBackgroundWidget(ContextMenuOptions opt, WidgetPtr child)
        : SingleChildRenderObjectWidget(Key::none(), std::move(child)),
          options(std::move(opt)) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override {
        return std::make_unique<RenderContextMenuBackground>(options);
    }

    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override {
        if (auto* rb = dynamic_cast<RenderContextMenuBackground*>(&renderObject)) {
            rb->options = options;
            rb->markNeedsPaint();
        }
    }

    [[nodiscard]] std::string_view typeName() const override { return "ContextMenuBackgroundWidget"; }
};

// ════════════════════════════════════════════════════════════════
// Single Item Row Component
// ════════════════════════════════════════════════════════════════

class ContextMenuItemRow : public StatefulWidget {
public:
    ContextMenuItemPtr item;
    ContextMenuOptions options;
    std::shared_ptr<NativePopup> parent_popup;

    ContextMenuItemRow(ContextMenuItemPtr item,
                       ContextMenuOptions options,
                       std::shared_ptr<NativePopup> parent_popup)
        : item(std::move(item)), options(std::move(options)), parent_popup(std::move(parent_popup)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "ContextMenuItemRow"; }
};

class ContextMenuItemRowState : public State {
private:
    bool is_hovered_ = false;

public:
    WidgetPtr build(BuildContext& ctx) override {
        auto* row_widget = static_cast<const ContextMenuItemRow*>(widget());
        if (!row_widget || !row_widget->item) return nullptr;

        const auto& item_base = row_widget->item;
        const auto& opt = row_widget->options;

        // 1. Divider Item
        if (item_base->itemType() == ContextMenuItemType::Divider) {
            auto div = container(nullptr);
            div->height(1.0f)
               .color(opt.border_color)
               .margin(StyleInsets::symmetric(4.0f, 0.0f));
            return div;
        }

        // 2. Standard Item or SubMenu Item
        bool is_disabled = false;
        bool is_danger = false;
        std::string label_str;
        std::string shortcut_str;
        WidgetPtr icon_w = nullptr;
        std::function<void()> on_selected = nullptr;

        if (item_base->itemType() == ContextMenuItemType::Item) {
            auto* item = static_cast<const ContextMenuItem*>(item_base.get());
            label_str = item->label;
            shortcut_str = item->shortcut_text;
            icon_w = item->icon_widget;
            on_selected = item->on_selected;
            is_disabled = item->disabled;
            is_danger = item->danger;
        } else if (item_base->itemType() == ContextMenuItemType::SubMenu) {
            auto* sub = static_cast<const ContextMenuSubMenu*>(item_base.get());
            label_str = sub->label;
            shortcut_str = "▶";
            icon_w = sub->icon_widget;
            is_disabled = sub->disabled;
        }

        // Foreground color determination
        Color text_col = opt.text_color;
        if (is_disabled) {
            text_col = opt.disabled_color;
        } else if (is_danger) {
            text_col = opt.danger_color;
        }

        std::vector<WidgetPtr> row_elements;

        // Leading Icon
        if (icon_w) {
            row_elements.push_back(icon_w);
        }

        // Label Text
        auto label_text = text({
            .text = label_str,
            .color = text_col,
            .font_size = opt.font_size,
        });
        row_elements.push_back(label_text);

        // Flexible Spacer
        auto spacer = container(nullptr);
        spacer->flexGrow(1.0f);
        row_elements.push_back(spacer);

        // Trailing Shortcut / Submenu arrow
        if (!shortcut_str.empty()) {
            Color shortcut_color = is_disabled ? opt.disabled_color : opt.shortcut_color;
            auto shortcut_text = text({
                .text = shortcut_str,
                .color = shortcut_color,
                .font_size = opt.font_size - 1.0f,
            });
            row_elements.push_back(shortcut_text);
        }

        auto item_row = row(row_elements);
        item_row->alignItems(Align::Center)
                .gap(StyleValue::point(10.0f));

        Color bg_color = 0x00000000;
        if (is_hovered_ && !is_disabled) {
            bg_color = opt.hover_color;
        }

        auto item_container = container(item_row);
        item_container->color(bg_color)
                      .borderRadius(4.0f)
                      .paddingSymmetric(6.0f, 10.0f)
                      .minHeight(StyleValue::point(opt.item_height));

        return gestureDetector({
            .child = item_container,
            .hit_test_behavior = HitTestBehavior::Translucent,
            .on_tap_up = !is_disabled ? GestureTapUpCallback([this, row_widget, on_selected](const TapUpDetails&) {
                if (on_selected) {
                    on_selected();
                }
                // Close parent popup menu
                if (row_widget->parent_popup) {
                    row_widget->parent_popup->close();
                }
            }) : nullptr,
            .on_hover_enter = !is_disabled ? GestureHoverCallback([this](const PointerEvent&) {
                setState([this]() { is_hovered_ = true; });
            }) : nullptr,
            .on_hover_exit = !is_disabled ? GestureHoverCallback([this](const PointerEvent&) {
                setState([this]() { is_hovered_ = false; });
            }) : nullptr,
        });
    }
};

std::unique_ptr<State> ContextMenuItemRow::createState() {
    return std::make_unique<ContextMenuItemRowState>();
}

// ════════════════════════════════════════════════════════════════
// ContextMenu State & NativePopup Triggering
// ════════════════════════════════════════════════════════════════

class ContextMenuState : public State {
private:
    std::shared_ptr<NativePopup> active_popup_ = nullptr;

public:
    void dispose() override {
        hideMenuNow();
        State::dispose();
    }

    void showMenuAt(Point pointer_pos) {
        if (active_popup_) {
            hideMenuNow();
        }

        auto* menu_widget = static_cast<const ContextMenuWidget*>(widget());
        if (!menu_widget || menu_widget->items.empty()) return;

        Element* elem = element();
        if (!elem) return;

        const auto& opt = menu_widget->options;

        // 1. Calculate menu height & width
        int32_t pop_w = static_cast<int32_t>(opt.min_width);
        int32_t pop_h = static_cast<int32_t>(menu_widget->items.size() * opt.item_height + opt.padding.vertical() + 12.0f);

        // 2. Perform screen boundary fitting
        float pop_x = pointer_pos.x;
        float pop_y = pointer_pos.y;

        BuildContext ctx(elem);
        Size screen_sz = ctx.mediaSize();

        if (pop_x + pop_w > screen_sz.width - 10.0f) {
            pop_x = screen_sz.width - pop_w - 10.0f;
        }
        if (pop_y + pop_h > screen_sz.height - 10.0f) {
            pop_y = pointer_pos.y - pop_h;
        }

        if (pop_x < 5.0f) pop_x = 5.0f;
        if (pop_y < 5.0f) pop_y = 5.0f;

        // 3. Configure PopupOptions
        PopupOptions pop_opts;
        pop_opts.position = {pop_x, pop_y};
        pop_opts.width = pop_w;
        pop_opts.height = pop_h;
        pop_opts.auto_dismiss = true;

        // 4. Build Menu Item Column
        active_popup_ = NativePopup::show(ctx, pop_opts, [menu_widget](BuildContext&, std::shared_ptr<NativePopup> popup) {
            std::vector<WidgetPtr> rows;
            rows.reserve(menu_widget->items.size());

            for (const auto& item : menu_widget->items) {
                rows.push_back(std::make_shared<ContextMenuItemRow>(item, menu_widget->options, popup));
            }

            auto menu_col = column(rows);
            menu_col->gap(StyleValue::point(2.0f));

            auto inner = container(menu_col);
            inner->padding(menu_widget->options.padding);

            return std::make_shared<ContextMenuBackgroundWidget>(menu_widget->options, inner);
        });
    }

    void hideMenuNow() {
        if (active_popup_) {
            active_popup_->close();
            active_popup_ = nullptr;
        }
    }

    WidgetPtr build(BuildContext& ctx) override {
        auto* menu_widget = static_cast<const ContextMenuWidget*>(widget());

        return gestureDetector({
            .child = menu_widget->child,
            .hit_test_behavior = HitTestBehavior::Translucent,
            .on_secondary_tap_up = [this](const TapUpDetails& details) {
                showMenuAt(details.global_position);
            },
            .on_long_press = [this]() {
                Element* elem = element();
                if (!elem) return;
                RenderObject* ro = elem->findRenderObject();
                if (!ro) return;
                Rect bounds = ro->globalBounds();
                showMenuAt({bounds.x + bounds.width / 2.0f, bounds.y + bounds.height / 2.0f});
            },
        });
    }
};

std::unique_ptr<State> ContextMenuWidget::createState() {
    return std::make_unique<ContextMenuState>();
}

} // namespace enki
