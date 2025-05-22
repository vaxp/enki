#include "enki/widgets/navigation_bar.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/stack.hpp"
#include "enki/state/state.hpp"
#include "enki/rendering/canvas.hpp"
#include "enki/rendering/paint.hpp"
#include "enki/tree/build_context.hpp"
#include <algorithm>

#include "enki/widgets/icon.hpp"

namespace enki {

// ════════════════════════════════════════════════════════════════
// RenderNavigationBar — custom painting
// ════════════════════════════════════════════════════════════════

class RenderNavigationBar : public RenderBox {
public:
    std::vector<NavigationBarItem> items;
    int                            selected_index = 0;
    int                            hovered_index  = -1;
    NavigationBarOptions           options;
    
    std::vector<std::unique_ptr<RenderIcon>> icon_renderers;

    std::function<void(int)>       on_tap;
    std::function<void(int)>       on_hover_change;

    RenderNavigationBar(std::vector<NavigationBarItem> its, int sel,
                        NavigationBarOptions opt)
        : items(std::move(its)), selected_index(sel), options(std::move(opt))
    {
        ANUNodeStyleSetHeight(anu_node_, options.height);
        ANUNodeStyleSetWidthPercent(anu_node_, 100.0f);
        
        for (const auto& item : items) {
            icon_renderers.push_back(std::make_unique<RenderIcon>(
                item.icon, options.icon_font_size, options.inactive_color));
        }
    }

    SystemCursor cursor() const override { return SystemCursor::Pointer; }

    int getIndexFromPoint(Point p) const {
        float w = size_.width;
        int n = static_cast<int>(items.size());
        if (n == 0 || w <= 0.0f) return -1;
        int idx = static_cast<int>((p.x / w) * n);
        return std::clamp(idx, 0, n - 1);
    }

    void handlePointerDown(const PointerEvent& e) override {
        int idx = getIndexFromPoint(e.localPosition);
        if (idx >= 0 && on_tap) on_tap(idx);
    }

    void handlePointerMove(const PointerEvent& e) override {
        int idx = getIndexFromPoint(e.localPosition);
        if (idx != hovered_index && on_hover_change) {
            on_hover_change(idx);
        }
    }

    void handlePointerExit(const PointerEvent& e) override {
        if (on_hover_change) on_hover_change(-1);
    }


    void paint(PaintContext& ctx) override {
        float x = ctx.offset.x;
        float y = ctx.offset.y;
        float w = size_.width;
        float h = options.height;

        if (w <= 0.0f || h <= 0.0f) return;

        // ── Background ─────────────────────────────────────────
        Paint bg;
        bg.setColor(options.background_color);
        ctx.canvas.drawRect(Rect{x, y, w, h}, bg);

        // ── Top border ─────────────────────────────────────────
        if (options.border_top_width > 0.0f) {
            Paint bp;
            bp.setColor(options.border_color);
            bp.setStyle(PaintStyle::Stroke);
            bp.setStrokeWidth(options.border_top_width);
            ctx.canvas.drawLine(Point{x, y}, Point{x + w, y}, bp);
        }

        int n = static_cast<int>(items.size());
        if (n == 0) return;

        float item_w = w / static_cast<float>(n);

        for (int i = 0; i < n; ++i) {
            bool active  = (i == selected_index);
            bool hovered = (i == hovered_index && !active);

            float ix = x + item_w * i;
            float center_x = ix + item_w * 0.5f;

            // ── Hover tint ──────────────────────────────────────
            if (hovered) {
                Paint hp;
                hp.setColor(options.hover_color);
                ctx.canvas.drawRect(Rect{ix, y, item_w, h}, hp);
            }

            // ── Active indicator pill ───────────────────────────
            if (active) {
                Paint ip;
                ip.setColor(options.indicator_color);
                float ind_w = options.indicator_w;
                float ind_h = options.indicator_h;
                float ind_x = center_x - ind_w * 0.5f;
                float ind_y = y + (h - ind_h) * 0.5f - (options.show_labels ? 6.0f : 0.0f);
                ctx.canvas.drawRRect(
                    Rect{ind_x, ind_y, ind_w, ind_h},
                    BorderRadius::circular(options.indicator_radius),
                    ip);
            }

            Color col = active ? options.active_color : options.inactive_color;
            bool show_label = options.show_labels && !items[i].label.empty();
            bool show_icon  = !items[i].icon.empty();

            float content_h = 0.0f;
            if (show_icon)               content_h += options.icon_font_size;
            if (show_icon && show_label) content_h += 2.0f;
            if (show_label)              content_h += options.label_font_size;
            float content_y = y + (h - content_h) * 0.5f;

            // ── Icon ───────────────────────────────────────────
            if (show_icon && i < icon_renderers.size()) {
                auto& ri = icon_renderers[i];
                ri->setColor(col);
                
                PaintContext icon_ctx = ctx;
                icon_ctx.offset = Point{center_x - options.icon_font_size * 0.5f,
                                        content_y};
                ri->paint(icon_ctx);
                
                content_y += options.icon_font_size + 2.0f;
            }

            // ── Label ──────────────────────────────────────────
            if (show_label) {
                Paint labp;
                labp.setColor(col);
                float lw = ctx.canvas.measureText(items[i].label, options.label_font_size,
                                                  nullptr, active);
                ctx.canvas.drawText(items[i].label,
                    Point{center_x - lw * 0.5f,
                          content_y + options.label_font_size},
                    labp, options.label_font_size, nullptr, active);
            }

            // ── Badge ──────────────────────────────────────────
            if (!items[i].badge.empty()) {
                float br  = 7.0f;
                float bx  = ix + item_w * 0.68f;
                float by  = y + 8.0f;

                Paint bkgp;
                bkgp.setColor(options.badge_color);
                ctx.canvas.drawCircle(Point{bx, by + br}, br, bkgp);

                Paint btp;
                btp.setColor(options.badge_text_color);
                float btw = ctx.canvas.measureText(items[i].badge, 9.0f);
                ctx.canvas.drawText(items[i].badge,
                    Point{bx - btw * 0.5f, by + br + 9.0f * 0.35f},
                    btp, 9.0f);
            }
        }
    }

    bool hitTestSelf(Point p) const override {
        return p.x >= 0 && p.x <= size_.width && p.y >= 0 && p.y <= size_.height;
    }
};

// ════════════════════════════════════════════════════════════════
// Internal render widget
// ════════════════════════════════════════════════════════════════

class NavBarRenderWidget : public SingleChildRenderObjectWidget {
public:
    std::vector<NavigationBarItem> items;
    int                            selected_index;
    int                            hovered_index;
    NavigationBarOptions           options;
    std::function<void(int)>       on_tap;
    std::function<void(int)>       on_hover_change;

    NavBarRenderWidget(std::vector<NavigationBarItem> its, int sel, int hov,
                       NavigationBarOptions opt)
        : SingleChildRenderObjectWidget(Key::none(), nullptr),
          items(std::move(its)), selected_index(sel), hovered_index(hov),
          options(std::move(opt)) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext&) override {
        auto ro = std::make_unique<RenderNavigationBar>(items, selected_index, options);
        ro->hovered_index   = hovered_index;
        ro->on_tap          = on_tap;
        ro->on_hover_change = on_hover_change;
        return ro;
    }

    void updateRenderObject(BuildContext&, RenderObject& ro) override {
        auto& r = static_cast<RenderNavigationBar&>(ro);
        
        if (r.items.size() != items.size()) {
            r.icon_renderers.clear();
            for (const auto& item : items) {
                r.icon_renderers.push_back(std::make_unique<RenderIcon>(
                    item.icon, options.icon_font_size, options.inactive_color));
            }
        } else {
            for (size_t i = 0; i < items.size(); ++i) {
                r.icon_renderers[i]->setIconData(items[i].icon);
                r.icon_renderers[i]->setSize(options.icon_font_size);
            }
        }
        
        r.items           = items;
        r.selected_index  = selected_index;
        r.hovered_index   = hovered_index;
        r.options         = options;
        r.on_tap          = on_tap;
        r.on_hover_change = on_hover_change;
        r.markNeedsPaint();
    }

    [[nodiscard]] std::string_view typeName() const override { return "NavBarRenderWidget"; }
};

// ════════════════════════════════════════════════════════════════
// NavigationBarState
// ════════════════════════════════════════════════════════════════

class NavigationBarState : public State {
    int hovered_index_ = -1;

public:
    WidgetPtr build(BuildContext&) override {
        auto* w = static_cast<const NavigationBar*>(widget());

        auto render_widget = std::make_shared<NavBarRenderWidget>(
            w->items, w->selected_index, hovered_index_, w->options);
            
        render_widget->on_tap = [w](int idx) {
            if (w->on_item_selected) w->on_item_selected(idx);
        };
        render_widget->on_hover_change = [this](int idx) {
            setState([this, idx] { hovered_index_ = idx; });
        };

        return render_widget;
    }
};

std::unique_ptr<State> NavigationBar::createState() {
    return std::make_unique<NavigationBarState>();
}

} // namespace enki
