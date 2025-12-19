/// @file navigation_bar.cpp
/// @brief  Direct Skia hardware-accelerated Advanced NavigationBar Suite for ENKI.

#include "enki/widgets/navigation_bar.hpp"
#include "enki/animation/animation_controller.hpp"
#include "enki/animation/ticker.hpp"
#include "enki/tree/build_context.hpp"
#include "enki/rendering/canvas.hpp"
#include "enki/rendering/paint.hpp"
#include "enki/widgets/icon.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>

namespace enki {

// ════════════════════════════════════════════════════════════════
// Item Geometry Helper
// ════════════════════════════════════════════════════════════════

struct ItemGeometry {
    Rect  bounds;
    Point center;
    Rect  target_indicator;
    Point icon_pos;
    Point label_pos;
    Point badge_pos;
};

static void computeGeometry(float bar_w, float bar_h,
                            const NavigationBarOptions& options,
                            const std::vector<NavigationBarItem>& items,
                            std::vector<ItemGeometry>& out_geoms,
                            Rect& out_leading,
                            Rect& out_search,
                            std::vector<Rect>& out_actions)
{
    int n = static_cast<int>(items.size());
    out_geoms.resize(n);
    out_actions.clear();
    out_leading = Rect{};
    out_search = Rect{};

    if (n == 0 || bar_w <= 0.0f || bar_h <= 0.0f) return;

    if (options.style == NavigationBarStyle::TopHeader) {
        // ── Desktop Header Layout ──────────────────────────
        float left_cursor = options.padding_horizontal;
        float right_cursor = bar_w - options.padding_horizontal;

        // 1. Leading brand
        if (!options.leading_title.empty() || !options.leading_icon.empty()) {
            float lead_w = 32.0f + (options.leading_title.empty() ? 0.0f : options.leading_title.length() * 9.0f + 20.0f);
            out_leading = Rect::fromLTWH(left_cursor, 0.0f, lead_w, bar_h);
            left_cursor += lead_w + 24.0f;
        }

        // 2. Trailing actions
        for (int a = static_cast<int>(options.trailing_actions.size()) - 1; a >= 0; --a) {
            float act_w = options.trailing_actions[a].length() * 8.0f + 24.0f;
            right_cursor -= act_w;
            out_actions.insert(out_actions.begin(), Rect::fromLTWH(right_cursor, (bar_h - 32.0f) * 0.5f, act_w, 32.0f));
            right_cursor -= 8.0f;
        }

        // 3. Search input
        if (options.show_search_placeholder) {
            float sw = 180.0f;
            right_cursor -= sw;
            out_search = Rect::fromLTWH(right_cursor, (bar_h - 34.0f) * 0.5f, sw, 34.0f);
            right_cursor -= 16.0f;
        }

        // 4. Middle navigation links
        float available_items_w = std::max(60.0f, right_cursor - left_cursor);
        float item_w = std::min(130.0f, available_items_w / std::max(1, n));

        for (int i = 0; i < n; ++i) {
            float ix = left_cursor + i * (item_w + options.item_gap);
            Rect ib{ix, 0.0f, item_w, bar_h};
            Point center{ix + item_w * 0.5f, bar_h * 0.5f};

            Rect ind_rect;
            if (options.indicator_style == NavIndicatorStyle::Underline) {
                float uw = item_w - 16.0f;
                ind_rect = Rect::fromLTWH(center.x - uw * 0.5f, bar_h - options.indicator_thickness - 2.0f,
                                          uw, options.indicator_thickness);
            } else {
                float pw = options.indicator_w > 0.0f ? options.indicator_w : item_w - 12.0f;
                float ph = options.indicator_h > 0.0f ? options.indicator_h : 36.0f;
                ind_rect = Rect::fromLTWH(center.x - pw * 0.5f, center.y - ph * 0.5f, pw, ph);
            }

            out_geoms[i] = ItemGeometry{
                .bounds = ib,
                .center = center,
                .target_indicator = ind_rect,
                .icon_pos = Point{center.x - 28.0f, center.y - options.icon_font_size * 0.5f},
                .label_pos = Point{center.x - 4.0f, center.y + options.label_font_size * 0.35f},
                .badge_pos = Point{center.x + item_w * 0.35f, center.y - 10.0f}
            };
        }
    } else {
        // ── Standard Bottom / Floating Pill / Segmented ────
        float content_w = bar_w - options.padding_horizontal * 2.0f;
        float item_w = content_w / static_cast<float>(n);

        for (int i = 0; i < n; ++i) {
            float ix = options.padding_horizontal + item_w * i;
            Rect ib{ix, 0.0f, item_w, bar_h};
            Point center{ix + item_w * 0.5f, bar_h * 0.5f};

            Rect ind_rect;
            if (options.indicator_style == NavIndicatorStyle::Underline) {
                float uw = options.indicator_w > 0.0f ? options.indicator_w : item_w * 0.6f;
                ind_rect = Rect::fromLTWH(center.x - uw * 0.5f, bar_h - options.indicator_thickness - 3.0f,
                                          uw, options.indicator_thickness);
            } else if (options.indicator_style == NavIndicatorStyle::Dot) {
                ind_rect = Rect::fromLTWH(center.x - 3.5f, bar_h - 10.0f, 7.0f, 7.0f);
            } else {
                float pw = options.indicator_w > 0.0f ? options.indicator_w : (item_w - options.item_gap * 2.0f);
                float ph = options.indicator_h > 0.0f ? options.indicator_h : 36.0f;
                float py = options.show_labels && options.item_layout == NavItemLayout::Vertical
                         ? (center.y - ph * 0.5f - 6.0f)
                         : (center.y - ph * 0.5f);
                ind_rect = Rect::fromLTWH(center.x - pw * 0.5f, py, pw, ph);
            }

            out_geoms[i] = ItemGeometry{
                .bounds = ib,
                .center = center,
                .target_indicator = ind_rect,
                .icon_pos = Point{center.x - options.icon_font_size * 0.5f, center.y - options.icon_font_size * 0.5f - (options.show_labels ? 7.0f : 0.0f)},
                .label_pos = Point{center.x, center.y + options.icon_font_size * 0.5f + 4.0f},
                .badge_pos = Point{center.x + (options.item_layout == NavItemLayout::Vertical ? 12.0f : item_w * 0.35f),
                                   options.item_layout == NavItemLayout::Vertical ? (center.y - 16.0f) : (center.y - 10.0f)}
            };
        }
    }
}

// ════════════════════════════════════════════════════════════════
// RenderNavigationBar — Custom Direct Skia Painting
// ════════════════════════════════════════════════════════════════

class RenderNavigationBar : public RenderBox {
public:
    std::vector<NavigationBarItem> items;
    int                            selected_index = 0;
    int                            hovered_index  = -1;
    int                            pressed_index  = -1;
    int                            hovered_action = -1;
    NavigationBarOptions           options;

    // Animated indicator position passed from State
    Rect                           indicator_rect;
    bool                           has_indicator_rect = false;

    std::vector<std::unique_ptr<RenderIcon>> icon_renderers;
    std::vector<std::unique_ptr<RenderIcon>> selected_icon_renderers;
    std::unique_ptr<RenderIcon>              leading_icon_renderer;

    std::function<void(int)>                 on_tap;
    std::function<void(int)>                 on_reselect;
    std::function<void(int)>                 on_hover_change;
    std::function<void(std::string_view)>   on_action_clicked;

    std::vector<ItemGeometry> item_geoms;
    Rect leading_rect;
    Rect search_rect;
    std::vector<Rect> action_rects;

    RenderNavigationBar(std::vector<NavigationBarItem> its, int sel,
                        Rect ind_rect, bool has_ind,
                        NavigationBarOptions opt)
        : items(std::move(its)), selected_index(sel), options(std::move(opt)),
          indicator_rect(ind_rect), has_indicator_rect(has_ind)
    {
        updateLayoutConstraints();
        initRenderers();
    }

    void updateLayoutConstraints() {
        ANUNodeStyleSetHeight(anu_node_, options.height);
        if (options.width > 0.0f) {
            ANUNodeStyleSetWidth(anu_node_, options.width);
        } else {
            ANUNodeStyleSetWidthPercent(anu_node_, 100.0f);
        }
    }

    void initRenderers() {
        icon_renderers.clear();
        selected_icon_renderers.clear();

        for (const auto& item : items) {
            icon_renderers.push_back(std::make_unique<RenderIcon>(
                item.icon, options.icon_font_size, options.inactive_color));
            
            IconData sel_ic = item.selected_icon.empty() ? item.icon : item.selected_icon;
            selected_icon_renderers.push_back(std::make_unique<RenderIcon>(
                sel_ic, options.icon_font_size, options.active_color));
        }

        if (!options.leading_icon.empty()) {
            leading_icon_renderer = std::make_unique<RenderIcon>(
                options.leading_icon, 24.0f, options.active_color);
        } else {
            leading_icon_renderer.reset();
        }
    }

    void updateOptions(const NavigationBarOptions& opt,
                       const std::vector<NavigationBarItem>& its,
                       int sel,
                       Rect ind_rect,
                       bool has_ind)
    {
        bool items_changed = (items.size() != its.size());
        items = its;
        selected_index = sel;
        options = opt;
        indicator_rect = ind_rect;
        has_indicator_rect = has_ind;
        updateLayoutConstraints();

        if (items_changed) {
            initRenderers();
        } else {
            for (size_t i = 0; i < items.size(); ++i) {
                icon_renderers[i]->setIconData(items[i].icon);
                icon_renderers[i]->setSize(options.icon_font_size);
                IconData sel_ic = items[i].selected_icon.empty() ? items[i].icon : items[i].selected_icon;
                selected_icon_renderers[i]->setIconData(sel_ic);
                selected_icon_renderers[i]->setSize(options.icon_font_size);
            }
        }
        markNeedsLayout();
        markNeedsPaint();
    }

    SystemCursor cursor() const override {
        return (hovered_index >= 0 || hovered_action >= 0) ? SystemCursor::Pointer : SystemCursor::Default;
    }

    void syncLayout() override {
        RenderBox::syncLayout();
        ensureGeometry();
    }

    void ensureGeometry() {
        float w = size_.width > 0.0f ? size_.width : (options.width > 0.0f ? options.width : 500.0f);
        float h = size_.height > 0.0f ? size_.height : options.height;
        computeGeometry(w, h, options, items, item_geoms, leading_rect, search_rect, action_rects);
    }

    int getItemAtPoint(Point p) {
        ensureGeometry();
        for (int i = 0; i < static_cast<int>(item_geoms.size()); ++i) {
            if (item_geoms[i].bounds.contains(p)) {
                return i;
            }
        }
        return -1;
    }

    int getActionAtPoint(Point p) {
        ensureGeometry();
        for (int a = 0; a < static_cast<int>(action_rects.size()); ++a) {
            if (action_rects[a].contains(p)) {
                return a;
            }
        }
        return -1;
    }

    bool hitTestSelf(Point p) const override {
        return (p.x >= 0 && p.x <= size_.width && p.y >= 0 && p.y <= size_.height);
    }

    void handlePointerDown(const PointerEvent& e) override {
        if (e.button != MouseButton::Left) return;
        int idx = getItemAtPoint(e.localPosition);
        if (idx >= 0 && items[idx].enabled) {
            pressed_index = idx;
            if (idx == selected_index) {
                if (on_reselect) on_reselect(idx);
            } else {
                if (on_tap) on_tap(idx);
            }
            markNeedsPaint();
        }
        int act = getActionAtPoint(e.localPosition);
        if (act >= 0 && act < static_cast<int>(options.trailing_actions.size())) {
            if (on_action_clicked) {
                on_action_clicked(options.trailing_actions[act]);
            }
        }
    }

    void handlePointerUp(const PointerEvent&) override {
        pressed_index = -1;
        markNeedsPaint();
    }

    void handlePointerMove(const PointerEvent& e) override {
        int idx = getItemAtPoint(e.localPosition);
        int act = getActionAtPoint(e.localPosition);
        bool changed = (idx != hovered_index || act != hovered_action);
        hovered_index = idx;
        hovered_action = act;
        if (changed) {
            if (on_hover_change) on_hover_change(idx);
            markNeedsPaint();
        }
    }

    void handlePointerExit(const PointerEvent&) override {
        hovered_index = -1;
        pressed_index = -1;
        hovered_action = -1;
        if (on_hover_change) on_hover_change(-1);
        markNeedsPaint();
    }

    // ════════════════════════════════════════════════════════════
    // Paint Implementation (Multi-layer Direct Skia)
    // ════════════════════════════════════════════════════════════

    void paint(PaintContext& ctx) override {
        float x = ctx.offset.x;
        float y = ctx.offset.y;
        float w = size_.width;
        float h = options.height;

        if (w <= 0.0f || h <= 0.0f) return;

        ensureGeometry();

        Rect bar_rect{x, y, w, h};
        BorderRadius bar_radius = BorderRadius::circular(options.corner_radius);

        // ── 1. Floating Shadow & Backdrop Glow ──────────────────
        if (options.style == NavigationBarStyle::FloatingPill || options.enable_glassmorphism) {
            Rect shadow_rect{x, y + options.shadow_offset_y, w, h};
            Paint sp;
            sp.setColor(options.shadow_color);
            sp.setImageFilter(ImageFilter::blur(options.shadow_blur, options.shadow_blur));
            ctx.canvas.drawRRect(shadow_rect, bar_radius, sp);

            Paint gp;
            gp.setColor(0x1A38BDF8);
            gp.setImageFilter(ImageFilter::blur(8.0f, 8.0f));
            ctx.canvas.drawRRect(bar_rect, bar_radius, gp);
        }

        // ── 2. Primary Surface Background ───────────────────────
        Paint bg;
        bg.setColor(options.background_color);
        bg.setAntiAlias(true);
        if (options.corner_radius > 0.0f) {
            ctx.canvas.drawRRect(bar_rect, bar_radius, bg);
        } else {
            ctx.canvas.drawRect(bar_rect, bg);
        }

        // ── 3. Borders & Separators ─────────────────────────────
        if (options.border_width > 0.0f) {
            Paint bp;
            bp.setColor(options.border_color);
            bp.setStyle(PaintStyle::Stroke);
            bp.setStrokeWidth(options.border_width);
            bp.setAntiAlias(true);

            if (options.corner_radius > 0.0f) {
                ctx.canvas.drawRRect(bar_rect, bar_radius, bp);
            } else if (options.style == NavigationBarStyle::BottomStandard) {
                ctx.canvas.drawLine(Point{x, y}, Point{x + w, y}, bp);
            } else if (options.style == NavigationBarStyle::TopHeader) {
                ctx.canvas.drawLine(Point{x, y + h}, Point{x + w, y + h}, bp);
            }
        }

        // ── 4. Dynamic Hover Highlights Per Item ────────────────
        int n = static_cast<int>(items.size());
        for (int i = 0; i < n; ++i) {
            if (i == hovered_index && i != selected_index) {
                const auto& geom = item_geoms[i];
                Paint hp;
                hp.setColor(options.hover_color);
                hp.setAntiAlias(true);

                Rect h_rect = (options.indicator_style == NavIndicatorStyle::Pill)
                            ? Rect{x + geom.target_indicator.x, y + geom.target_indicator.y, geom.target_indicator.width, geom.target_indicator.height}
                            : Rect{x + geom.bounds.x + 4.0f, y + 6.0f, geom.bounds.width - 8.0f, h - 12.0f};
                ctx.canvas.drawRRect(h_rect,
                                     BorderRadius::circular(options.indicator_radius > 0 ? options.indicator_radius : 8.0f), hp);
            }
        }

        // ── 5. Active Sliding Indicator (Smooth Animated) ────────
        if (options.indicator_style != NavIndicatorStyle::None) {
            Rect ind_target = (has_indicator_rect && indicator_rect.width > 0.0f)
                            ? indicator_rect
                            : (selected_index >= 0 && selected_index < n ? item_geoms[selected_index].target_indicator : Rect{});

            if (ind_target.width > 0.0f) {
                Rect ind_world{x + ind_target.x, y + ind_target.y, ind_target.width, ind_target.height};

                if (options.indicator_style == NavIndicatorStyle::Pill) {
                    Paint gp;
                    gp.setColor(options.glow_color);
                    gp.setImageFilter(ImageFilter::blur(6.0f, 6.0f));
                    ctx.canvas.drawRRect(ind_world, BorderRadius::circular(options.indicator_radius), gp);

                    Paint ip;
                    ip.setColor(options.indicator_color);
                    ip.setAntiAlias(true);
                    ctx.canvas.drawRRect(ind_world, BorderRadius::circular(options.indicator_radius), ip);

                    if ((options.indicator_border & 0xFF000000) != 0) {
                        Paint ibp;
                        ibp.setColor(options.indicator_border);
                        ibp.setStyle(PaintStyle::Stroke);
                        ibp.setStrokeWidth(1.0f);
                        ibp.setAntiAlias(true);
                        ctx.canvas.drawRRect(ind_world, BorderRadius::circular(options.indicator_radius), ibp);
                    }
                } else if (options.indicator_style == NavIndicatorStyle::Underline) {
                    Paint gp;
                    gp.setColor(options.active_color);
                    gp.setImageFilter(ImageFilter::blur(4.0f, 4.0f));
                    ctx.canvas.drawRRect(ind_world, BorderRadius::circular(options.indicator_thickness * 0.5f), gp);

                    Paint lp;
                    lp.setColor(options.active_color);
                    lp.setAntiAlias(true);
                    ctx.canvas.drawRRect(ind_world, BorderRadius::circular(options.indicator_thickness * 0.5f), lp);
                } else if (options.indicator_style == NavIndicatorStyle::Dot) {
                    Point dot_c{ind_world.x + ind_world.width * 0.5f, ind_world.y + ind_world.height * 0.5f};
                    Paint gp;
                    gp.setColor(options.glow_color);
                    gp.setImageFilter(ImageFilter::blur(4.0f, 4.0f));
                    ctx.canvas.drawCircle(dot_c, 4.0f, gp);

                    Paint dp;
                    dp.setColor(options.active_color);
                    dp.setAntiAlias(true);
                    ctx.canvas.drawCircle(dot_c, 3.0f, dp);
                } else if (options.indicator_style == NavIndicatorStyle::Glow) {
                    Paint gp;
                    gp.setShader(Gradient::radial(
                        {ind_world.x + ind_world.width * 0.5f, ind_world.y + ind_world.height * 0.5f},
                        ind_world.width * 0.8f,
                        {options.glow_color, 0x00000000}
                    ));
                    ctx.canvas.drawRect(ind_world, gp);
                }
            }
        }

        // ── 6. Navigation Items (Icons, Labels, Badges) ──────────
        for (int i = 0; i < n; ++i) {
            const auto& item = items[i];
            const auto& geom = item_geoms[i];
            bool active = (i == selected_index);
            bool hovered = (i == hovered_index);
            bool pressed = (i == pressed_index);

            Color item_color = !item.enabled ? 0xFF475569
                             : active ? options.active_color
                             : hovered ? 0xFFE2E8F0
                             : options.inactive_color;

            ctx.canvas.save();
            if (pressed) {
                ctx.canvas.translate(x + geom.center.x, y + geom.center.y);
                ctx.canvas.scale(0.92f, 0.92f);
                ctx.canvas.translate(-(x + geom.center.x), -(y + geom.center.y));
            }

            // ── A. Vertical Layout (Mobile/Bottom Bar) ───────────
            if (options.item_layout == NavItemLayout::Vertical) {
                if (!item.icon.empty() && i < static_cast<int>(icon_renderers.size())) {
                    auto& ri = active ? selected_icon_renderers[i] : icon_renderers[i];
                    ri->setColor(item_color);
                    PaintContext icon_ctx = ctx;
                    icon_ctx.offset = Point{x + geom.icon_pos.x, y + geom.icon_pos.y};
                    ri->paint(icon_ctx);
                }

                if (options.show_labels && !item.label.empty()) {
                    Paint labp;
                    labp.setColor(item_color);
                    labp.setAntiAlias(true);
                    float lw = ctx.canvas.measureText(item.label, options.label_font_size, nullptr, active);
                    ctx.canvas.drawText(item.label,
                        Point{x + geom.label_pos.x - lw * 0.5f, y + geom.label_pos.y},
                        labp, options.label_font_size, nullptr, active);
                }
            }
            // ── B. Horizontal Layout (Desktop/Top Header) ────────
            else if (options.item_layout == NavItemLayout::Horizontal) {
                float total_w = 0.0f;
                bool has_ic = !item.icon.empty();
                bool has_lb = options.show_labels && !item.label.empty();
                if (has_ic) total_w += options.icon_font_size;
                if (has_ic && has_lb) total_w += 6.0f;
                float lw = has_lb ? ctx.canvas.measureText(item.label, options.label_font_size, nullptr, active) : 0.0f;
                total_w += lw;

                float start_x = geom.center.x - total_w * 0.5f;

                if (has_ic && i < static_cast<int>(icon_renderers.size())) {
                    auto& ri = active ? selected_icon_renderers[i] : icon_renderers[i];
                    ri->setColor(item_color);
                    PaintContext icon_ctx = ctx;
                    icon_ctx.offset = Point{x + start_x, y + geom.center.y - options.icon_font_size * 0.5f};
                    ri->paint(icon_ctx);
                    start_x += options.icon_font_size + 6.0f;
                }

                if (has_lb) {
                    Paint labp;
                    labp.setColor(item_color);
                    labp.setAntiAlias(true);
                    ctx.canvas.drawText(item.label,
                        Point{x + start_x, y + geom.center.y + options.label_font_size * 0.35f},
                        labp, options.label_font_size, nullptr, active);
                }
            }
            // ── C. IconOnly ─────────────────────────────────────
            else if (options.item_layout == NavItemLayout::IconOnly) {
                if (!item.icon.empty() && i < static_cast<int>(icon_renderers.size())) {
                    auto& ri = active ? selected_icon_renderers[i] : icon_renderers[i];
                    ri->setColor(item_color);
                    PaintContext icon_ctx = ctx;
                    icon_ctx.offset = Point{x + geom.center.x - options.icon_font_size * 0.5f,
                                            y + geom.center.y - options.icon_font_size * 0.5f};
                    ri->paint(icon_ctx);
                }
            }
            // ── D. LabelOnly ────────────────────────────────────
            else if (options.item_layout == NavItemLayout::LabelOnly) {
                if (!item.label.empty()) {
                    Paint labp;
                    labp.setColor(item_color);
                    labp.setAntiAlias(true);
                    float lw = ctx.canvas.measureText(item.label, options.label_font_size, nullptr, active);
                    ctx.canvas.drawText(item.label,
                        Point{x + geom.center.x - lw * 0.5f, y + geom.center.y + options.label_font_size * 0.35f},
                        labp, options.label_font_size, nullptr, active);
                }
            }

            // ── Badge Rendering ─────────────────────────────────
            if (item.dot_badge) {
                Point dot_p{x + geom.badge_pos.x, y + geom.badge_pos.y};
                Paint gp;
                gp.setColor(options.badge_color);
                gp.setImageFilter(ImageFilter::blur(3.0f, 3.0f));
                ctx.canvas.drawCircle(dot_p, 4.0f, gp);

                Paint bp;
                bp.setColor(options.badge_color);
                bp.setAntiAlias(true);
                ctx.canvas.drawCircle(dot_p, 3.5f, bp);

                Paint op;
                op.setColor(options.background_color);
                op.setStyle(PaintStyle::Stroke);
                op.setStrokeWidth(1.5f);
                ctx.canvas.drawCircle(dot_p, 3.5f, op);
            } else if (!item.badge.empty()) {
                float bw = std::max(16.0f, ctx.canvas.measureText(item.badge, 9.5f) + 8.0f);
                float bh = 15.0f;
                Rect b_rect{x + geom.badge_pos.x - bw * 0.5f, y + geom.badge_pos.y - bh * 0.5f, bw, bh};

                Paint bgp;
                bgp.setColor(0x66EF4444);
                bgp.setImageFilter(ImageFilter::blur(3.0f, 3.0f));
                ctx.canvas.drawRRect(b_rect, BorderRadius::circular(bh * 0.5f), bgp);

                Paint bp;
                bp.setColor(options.badge_color);
                bp.setAntiAlias(true);
                ctx.canvas.drawRRect(b_rect, BorderRadius::circular(bh * 0.5f), bp);

                Paint btp;
                btp.setColor(options.badge_text_color);
                btp.setAntiAlias(true);
                float btw = ctx.canvas.measureText(item.badge, 9.5f, nullptr, true);
                ctx.canvas.drawText(item.badge,
                    Point{b_rect.x + (bw - btw) * 0.5f, b_rect.y + bh * 0.72f},
                    btp, 9.5f, nullptr, true);
            }

            ctx.canvas.restore();
        }

        // ── 7. Desktop Top Header Brand, Search, & Actions ──────
        if (options.style == NavigationBarStyle::TopHeader) {
            if (leading_icon_renderer || !options.leading_title.empty()) {
                float lx = x + leading_rect.x;
                if (leading_icon_renderer) {
                    PaintContext l_ctx = ctx;
                    l_ctx.offset = Point{lx, y + (h - 24.0f) * 0.5f};
                    leading_icon_renderer->paint(l_ctx);
                    lx += 30.0f;
                }
                if (!options.leading_title.empty()) {
                    Paint tp;
                    tp.setColor(0xFFFFFFFF);
                    tp.setAntiAlias(true);
                    ctx.canvas.drawText(options.leading_title,
                        Point{lx, y + (options.leading_subtitle.empty() ? (h * 0.5f + 5.0f) : (h * 0.5f - 2.0f))},
                        tp, 15.0f, nullptr, true);

                    if (!options.leading_subtitle.empty()) {
                        Paint stp;
                        stp.setColor(0xFF94A3B8);
                        stp.setAntiAlias(true);
                        ctx.canvas.drawText(options.leading_subtitle,
                            Point{lx, y + h * 0.5f + 12.0f},
                            stp, 10.0f);
                    }
                }
            }

            if (options.show_search_placeholder && search_rect.width > 0.0f) {
                Rect sw_rect{x + search_rect.x, y + search_rect.y, search_rect.width, search_rect.height};
                Paint sbp;
                sbp.setColor(0xFF1E293B);
                sbp.setAntiAlias(true);
                ctx.canvas.drawRRect(sw_rect, BorderRadius::circular(8.0f), sbp);

                Paint sbrdp;
                sbrdp.setColor(0xFF334155);
                sbrdp.setStyle(PaintStyle::Stroke);
                sbrdp.setStrokeWidth(1.0f);
                ctx.canvas.drawRRect(sw_rect, BorderRadius::circular(8.0f), sbrdp);

                Paint stp;
                stp.setColor(0xFF64748B);
                stp.setAntiAlias(true);
                ctx.canvas.drawText("🔍 " + options.search_hint,
                    Point{sw_rect.x + 10.0f, sw_rect.y + 21.0f},
                    stp, 11.5f);
            }

            for (size_t a = 0; a < action_rects.size() && a < options.trailing_actions.size(); ++a) {
                Rect act_r{x + action_rects[a].x, y + action_rects[a].y, action_rects[a].width, action_rects[a].height};
                bool act_hov = (static_cast<int>(a) == hovered_action);

                Paint abp;
                abp.setColor(act_hov ? 0xFF334155 : 0xFF1E293B);
                abp.setAntiAlias(true);
                ctx.canvas.drawRRect(act_r, BorderRadius::circular(6.0f), abp);

                Paint abrd;
                abrd.setColor(act_hov ? 0xFF38BDF8 : 0xFF334155);
                abrd.setStyle(PaintStyle::Stroke);
                abrd.setStrokeWidth(1.0f);
                ctx.canvas.drawRRect(act_r, BorderRadius::circular(6.0f), abrd);

                Paint atp;
                atp.setColor(act_hov ? 0xFF38BDF8 : 0xFFE2E8F0);
                atp.setAntiAlias(true);
                float atw = ctx.canvas.measureText(options.trailing_actions[a], 11.5f, nullptr, true);
                ctx.canvas.drawText(options.trailing_actions[a],
                    Point{act_r.x + (act_r.width - atw) * 0.5f, act_r.y + 20.0f},
                    atp, 11.5f, nullptr, true);
            }
        }

        // ── 8. Hover Tooltip Overlay ─────────────────────────────
        if (options.show_tooltips && hovered_index >= 0 && hovered_index < n) {
            const auto& item = items[hovered_index];
            std::string tip = !item.tooltip.empty() ? item.tooltip : item.label;
            if (!tip.empty()) {
                const auto& geom = item_geoms[hovered_index];
                float tw = ctx.canvas.measureText(tip, 10.5f) + 14.0f;
                float th = 22.0f;
                float tx = x + geom.center.x - tw * 0.5f;
                float ty = (options.style == NavigationBarStyle::TopHeader)
                         ? (y + h + 6.0f)
                         : (y - th - 6.0f);

                Rect tip_r{tx, ty, tw, th};

                Paint tip_shadow;
                tip_shadow.setColor(0x88000000);
                tip_shadow.setImageFilter(ImageFilter::blur(4.0f, 4.0f));
                ctx.canvas.drawRRect(tip_r, BorderRadius::circular(4.0f), tip_shadow);

                Paint tip_bg;
                tip_bg.setColor(0xFF0F172A);
                tip_bg.setAntiAlias(true);
                ctx.canvas.drawRRect(tip_r, BorderRadius::circular(4.0f), tip_bg);

                Paint tip_border;
                tip_border.setColor(0xFF38BDF8);
                tip_border.setStyle(PaintStyle::Stroke);
                tip_border.setStrokeWidth(1.0f);
                ctx.canvas.drawRRect(tip_r, BorderRadius::circular(4.0f), tip_border);

                Paint tip_txt;
                tip_txt.setColor(0xFFFFFFFF);
                tip_txt.setAntiAlias(true);
                float ttw = ctx.canvas.measureText(tip, 10.5f);
                ctx.canvas.drawText(tip, Point{tx + (tw - ttw) * 0.5f, ty + 15.0f}, tip_txt, 10.5f);
            }
        }
    }
};

// ════════════════════════════════════════════════════════════════
// Internal Render Widget
// ════════════════════════════════════════════════════════════════

class NavBarRenderWidget : public SingleChildRenderObjectWidget {
public:
    std::vector<NavigationBarItem> items;
    int                            selected_index;
    Rect                           indicator_rect;
    bool                           has_indicator_rect;
    NavigationBarOptions           options;
    std::function<void(int)>       on_tap;
    std::function<void(int)>       on_reselect;
    std::function<void(std::string_view)> on_action_clicked;

    NavBarRenderWidget(std::vector<NavigationBarItem> its, int sel,
                       Rect ind_rect, bool has_ind,
                       NavigationBarOptions opt)
        : SingleChildRenderObjectWidget(Key::none(), nullptr),
          items(std::move(its)), selected_index(sel),
          indicator_rect(ind_rect), has_indicator_rect(has_ind),
          options(std::move(opt)) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext&) override {
        auto ro = std::make_unique<RenderNavigationBar>(
            items, selected_index, indicator_rect, has_indicator_rect, options);
        ro->on_tap            = on_tap;
        ro->on_reselect       = on_reselect;
        ro->on_action_clicked = on_action_clicked;
        return ro;
    }

    void updateRenderObject(BuildContext&, RenderObject& ro) override {
        auto& r = static_cast<RenderNavigationBar&>(ro);
        r.on_tap            = on_tap;
        r.on_reselect       = on_reselect;
        r.on_action_clicked = on_action_clicked;
        r.updateOptions(options, items, selected_index, indicator_rect, has_indicator_rect);
    }

    [[nodiscard]] std::string_view typeName() const override { return "NavBarRenderWidget"; }
};

// ════════════════════════════════════════════════════════════════
// NavigationBarState — Smooth 60FPS Ticker-Driven Animation
// ════════════════════════════════════════════════════════════════

class NavigationBarState : public State {
    AnimationController     anim_;
    std::unique_ptr<Ticker> ticker_;

    int  last_selected_idx_ = -1;
    Rect anim_from_rect_;
    Rect anim_to_rect_;
    Rect current_indicator_rect_;
    bool initialized_ = false;

public:
    void initState() override {
        State::initState();
        anim_.setDuration(std::chrono::milliseconds(220));
        anim_.addListener([this] {
            setState([] {});
        });
        ticker_ = createTicker([this] {
            if (anim_.isAnimating()) {
                anim_.tick();
            }
        });
        ticker_->start();
    }

    void dispose() override {
        if (ticker_) ticker_->stop();
        anim_.dispose();
        State::dispose();
    }

    WidgetPtr build(BuildContext&) override {
        auto* w = static_cast<const NavigationBarWidget*>(widget());
        int sel = w->selected_index;

        // Calculate target geometry
        float bar_w = w->options.width > 0.0f ? w->options.width : (w->options.style == NavigationBarStyle::TopHeader ? 800.0f : 500.0f);
        float bar_h = w->options.height;
        std::vector<ItemGeometry> geoms;
        Rect dummy_lead, dummy_search;
        std::vector<Rect> dummy_acts;
        computeGeometry(bar_w, bar_h, w->options, w->items, geoms, dummy_lead, dummy_search, dummy_acts);

        Rect target_rect;
        if (sel >= 0 && sel < static_cast<int>(geoms.size())) {
            target_rect = geoms[sel].target_indicator;
        }

        if (!initialized_) {
            anim_from_rect_ = target_rect;
            anim_to_rect_   = target_rect;
            current_indicator_rect_ = target_rect;
            last_selected_idx_ = sel;
            initialized_ = true;
        } else if (sel != last_selected_idx_) {
            anim_from_rect_ = current_indicator_rect_;
            anim_to_rect_   = target_rect;
            last_selected_idx_ = sel;
            anim_.reset();
            anim_.forward();
        }

        if (anim_.isAnimating()) {
            float t = Curves::easeOut.evaluateF(anim_.value());
            current_indicator_rect_ = Rect{
                anim_from_rect_.x + (anim_to_rect_.x - anim_from_rect_.x) * t,
                anim_from_rect_.y + (anim_to_rect_.y - anim_from_rect_.y) * t,
                anim_from_rect_.width + (anim_to_rect_.width - anim_from_rect_.width) * t,
                anim_from_rect_.height + (anim_to_rect_.height - anim_from_rect_.height) * t
            };
        } else {
            current_indicator_rect_ = target_rect;
        }

        auto render_widget = std::make_shared<NavBarRenderWidget>(
            w->items, w->selected_index, current_indicator_rect_, initialized_, w->options);

        render_widget->on_tap = [w, this](int idx) {
            if (w->on_item_selected) w->on_item_selected(idx);
        };
        render_widget->on_reselect = [w](int idx) {
            if (w->on_item_reselect) w->on_item_reselect(idx);
        };
        render_widget->on_action_clicked = [w](std::string_view act) {
            if (w->on_action_clicked) w->on_action_clicked(act);
        };

        return render_widget;
    }
};

std::unique_ptr<State> NavigationBarWidget::createState() {
    return std::make_unique<NavigationBarState>();
}

} // namespace enki
