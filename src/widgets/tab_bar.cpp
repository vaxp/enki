#include "enki/widgets/tab_bar.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/stack.hpp"
#include "enki/animation/animation_controller.hpp"
#include "enki/animation/ticker.hpp"
#include "enki/state/state.hpp"
#include "enki/rendering/canvas.hpp"
#include "enki/rendering/paint.hpp"
#include "enki/tree/build_context.hpp"
#include "enki/tree/element.hpp"
#include <cmath>
#include <algorithm>

namespace enki {

// ════════════════════════════════════════════════════════════════
// RenderTabBar Implementation
// ════════════════════════════════════════════════════════════════

RenderTabBar::RenderTabBar(std::vector<TabItem> t, int sel, TabBarOptions opt)
    : tabs(std::move(t)), selected_index(sel), options(std::move(opt))
{
    ANUNodeStyleSetHeight(anu_node_, options.tab_height);
    ANUNodeStyleSetWidthPercent(anu_node_, 100.0f);
    ANUNodeStyleSetFlexDirection(anu_node_, ANUFlexDirectionRow);
    
    for (const auto& tab : tabs) {
        icon_renderers.push_back(std::make_unique<RenderIcon>(
            tab.icon, options.icon_font_size, options.inactive_color));
    }
}

SystemCursor RenderTabBar::cursor() const {
    return SystemCursor::Pointer;
}

int RenderTabBar::getIndexFromPoint(Point p) const {
    float w = size_.width;
    int n = static_cast<int>(tabs.size());
    if (n == 0 || w <= 0.0f) return -1;
    int idx = static_cast<int>((p.x / w) * n);
    return std::clamp(idx, 0, n - 1);
}

void RenderTabBar::handlePointerDown(const PointerEvent& e) {
    int idx = getIndexFromPoint(e.localPosition);
    if (idx >= 0 && on_tap) on_tap(idx);
}

void RenderTabBar::handlePointerMove(const PointerEvent& e) {
    int idx = getIndexFromPoint(e.localPosition);
    if (idx != hovered_index && on_hover_change) {
        on_hover_change(idx);
    }
}

void RenderTabBar::handlePointerExit(const PointerEvent& e) {
    if (on_hover_change) on_hover_change(-1);
}

void RenderTabBar::syncLayout() {
    RenderBox::syncLayout();
    // Recompute indicator geometry whenever layout changes
    float total_w = size_.width;
    if (!tabs.empty() && total_w > 0.0f) {
        float ix, iw;
        tabRect(selected_index, total_w, ix, iw);
        indicator_x = ix;
        indicator_w = iw;
    }
}

float RenderTabBar::tabCenterX(int index, float total_w) const {
    if (tabs.empty()) return 0.0f;
    float tab_w = total_w / static_cast<float>(tabs.size());
    return tab_w * index + tab_w * 0.5f;
}

void RenderTabBar::tabRect(int index, float total_w, float& out_x, float& out_w) const {
    if (tabs.empty()) { out_x = 0; out_w = 0; return; }
    float tab_w = total_w / static_cast<float>(tabs.size());
    out_x = tab_w * index;
    out_w = tab_w;
}

void RenderTabBar::paint(PaintContext& ctx) {
    float x = ctx.offset.x;
    float y = ctx.offset.y;
    float w = size_.width;
    float h = size_.height;

    if (w <= 0.0f || h <= 0.0f) return;

    // ── Background ────────────────────────────────────────────
    Paint bg;
    bg.setColor(options.background_color);
    bg.setAntiAlias(false);
    ctx.canvas.drawRect(Rect{x, y, w, h}, bg);

    int n = static_cast<int>(tabs.size());
    if (n == 0) return;
    float tab_w = w / static_cast<float>(n);

    // ── Hover highlight ───────────────────────────────────────
    if (hovered_index >= 0 && hovered_index < n && hovered_index != selected_index) {
        Paint hover;
        hover.setColor(options.hover_color);
        float hx = x + tab_w * hovered_index;
        ctx.canvas.drawRect(Rect{hx, y, tab_w, h}, hover);
    }

    // ── Tab labels and icons ──────────────────────────────────
    for (int i = 0; i < n; ++i) {
        bool active = (i == selected_index);
        Color col = active ? options.active_color : options.inactive_color;

        float tx = x + tab_w * i;
        float center_x = tx + tab_w * 0.5f;

        bool show_icon  = options.show_icons && !tabs[i].icon.empty();
        bool show_label = options.show_labels && !tabs[i].label.empty();

        float content_h = 0.0f;
        if (show_icon)  content_h += options.icon_font_size;
        if (show_icon && show_label) content_h += options.gap;
        if (show_label) content_h += options.label_font_size;

        float content_y = y + (h - content_h) * 0.5f;

        // ── Icon ──────────────────────────────────────────────
        if (show_icon && i < icon_renderers.size()) {
            auto& ri = icon_renderers[i];
            ri->setColor(col);
            
            PaintContext icon_ctx = ctx;
            icon_ctx.offset = Point{center_x - options.icon_font_size * 0.5f,
                                    content_y};
            ri->paint(icon_ctx);
            
            content_y += options.icon_font_size + 2.0f;
        }

        if (show_label) {
            Paint lp;
            lp.setColor(col);
            float lw = ctx.canvas.measureText(tabs[i].label, options.label_font_size,
                                              nullptr, active);
            ctx.canvas.drawText(tabs[i].label,
                                Point{center_x - lw * 0.5f,
                                      content_y + options.label_font_size},
                                lp, options.label_font_size, nullptr, active);
        }

        // ── Badge ─────────────────────────────────────────────
        if (!tabs[i].badge.empty()) {
            float badge_r = 8.0f;
            float badge_x = tx + tab_w * 0.7f;
            float badge_y = y + 6.0f;

            Paint bp;
            bp.setColor(options.badge_color);
            ctx.canvas.drawCircle(Point{badge_x, badge_y + badge_r}, badge_r, bp);

            Paint btp;
            btp.setColor(options.badge_text_color);
            float btw = ctx.canvas.measureText(tabs[i].badge, options.badge_font_size);
            ctx.canvas.drawText(tabs[i].badge,
                                Point{badge_x - btw * 0.5f,
                                      badge_y + badge_r + options.badge_font_size * 0.35f},
                                btp, options.badge_font_size);
        }
    }

    // ── Indicator (animated) ──────────────────────────────────
    float ind_h = options.indicator_height;
    float ind_y = y + h - ind_h;

    Paint ind;
    ind.setColor(options.indicator_color);
    ctx.canvas.drawRRect(
        Rect{x + indicator_x, ind_y, indicator_w, ind_h},
        BorderRadius{options.indicator_radius, options.indicator_radius, 0.0f, 0.0f},
        ind);
}

bool RenderTabBar::hitTestSelf(Point p) const {
    return p.x >= 0 && p.x <= size_.width && p.y >= 0 && p.y <= size_.height;
}

// ════════════════════════════════════════════════════════════════
// Internal TabBarRenderWidget
// ════════════════════════════════════════════════════════════════

class TabBarRenderWidget : public SingleChildRenderObjectWidget {
public:
    std::vector<TabItem> tabs;
    int                  selected_index;
    float                indicator_x;
    float                indicator_w;
    int                  hovered_index;
    TabBarOptions        options;
    
    std::function<void(int)> on_tap;
    std::function<void(int)> on_hover_change;

    TabBarRenderWidget(std::vector<TabItem> tabs, int sel, float ix, float iw, int hov,
                       TabBarOptions opt)
        : SingleChildRenderObjectWidget(Key::none(), nullptr),
          tabs(std::move(tabs)), selected_index(sel), indicator_x(ix), indicator_w(iw),
          hovered_index(hov), options(std::move(opt)) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext&) override {
        auto ro = std::make_unique<RenderTabBar>(tabs, selected_index, options);
        ro->indicator_x     = indicator_x;
        ro->indicator_w     = indicator_w;
        ro->hovered_index   = hovered_index;
        ro->on_tap          = on_tap;
        ro->on_hover_change = on_hover_change;
        return ro;
    }

    void updateRenderObject(BuildContext&, RenderObject& ro) override {
        auto& r = static_cast<RenderTabBar&>(ro);
        
        if (r.tabs.size() != tabs.size()) {
            r.icon_renderers.clear();
            for (const auto& tab : tabs) {
                r.icon_renderers.push_back(std::make_unique<RenderIcon>(
                    tab.icon, options.icon_font_size, options.inactive_color));
            }
        } else {
            for (size_t i = 0; i < tabs.size(); ++i) {
                r.icon_renderers[i]->setIconData(tabs[i].icon);
                r.icon_renderers[i]->setSize(options.icon_font_size);
            }
        }
        
        r.tabs            = tabs;
        r.selected_index  = selected_index;
        r.indicator_x     = indicator_x;
        r.indicator_w     = indicator_w;
        r.hovered_index   = hovered_index;
        r.options         = options;
        r.on_tap          = on_tap;
        r.on_hover_change = on_hover_change;
        r.markNeedsPaint();
    }

    [[nodiscard]] std::string_view typeName() const override { return "TabBarRenderWidget"; }
};

// ════════════════════════════════════════════════════════════════
// TabBarState
// ════════════════════════════════════════════════════════════════

class TabBarState : public State {
    AnimationController  anim_;          // 0→1 indicator slide
    std::unique_ptr<Ticker> ticker_;
    int    hovered_index_  = -1;
    int    anim_from_      = 0;
    int    anim_to_        = 0;
    float  anim_from_x_   = 0.0f;
    float  anim_from_w_   = 0.0f;
    float  anim_to_x_     = 0.0f;
    float  anim_to_w_     = 0.0f;
    float  current_ind_x_ = 0.0f;
    float  current_ind_w_ = 0.0f;
    bool   layout_done_   = false;

public:
    void initState() override {
        State::initState();
        anim_.setDuration(std::chrono::milliseconds(250));
        anim_.addListener([this] { setState([] {}); });
        ticker_ = createTicker([this] {
            if (anim_.isAnimating()) {
                anim_.tick();
            }
        });
        ticker_->start();
    }

    void dispose() override {
        ticker_->stop();
        anim_.dispose();
        State::dispose();
    }

    WidgetPtr build(BuildContext& ctx) override {
        auto* w = static_cast<const TabBarWidget*>(widget());
        int n = static_cast<int>(w->tabs.size());

        float t = anim_.value();
        float ind_x = anim_from_x_ + (anim_to_x_ - anim_from_x_) * t;
        float ind_w = anim_from_w_ + (anim_to_w_ - anim_from_w_) * t;
        current_ind_x_ = ind_x;
        current_ind_w_ = ind_w;

        auto render_widget = std::make_shared<TabBarRenderWidget>(
            w->tabs, w->selected_index, ind_x, ind_w, hovered_index_, w->options);
            
        render_widget->on_tap = [this, w, n](int idx) {
            if (idx == w->selected_index) return;
            animateTo(idx, n);
            if (w->on_tab_changed) w->on_tab_changed(idx);
        };
        render_widget->on_hover_change = [this](int idx) {
            setState([this, idx] { hovered_index_ = idx; });
        };

        return render_widget;
    }


private:
    void animateTo(int to_index, int n) {
        if (n <= 0) return;
        anim_from_x_ = current_ind_x_;
        anim_from_w_ = current_ind_w_;
        anim_to_x_   = 0.0f;
        anim_to_w_   = 0.0f;

        // Compute from render object if available
        if (auto* ro = element() ? element()->findRenderObject() : nullptr) {
            float total_w = ro->size().width;
            float tab_w = total_w / static_cast<float>(n);
            anim_from_x_ = current_ind_x_;
            anim_from_w_ = current_ind_w_ > 0 ? current_ind_w_ : tab_w;
            anim_to_x_   = tab_w * to_index;
            anim_to_w_   = tab_w;
        }

        anim_to_   = to_index;
        anim_.reset();
        anim_.forward();
    }
};

std::unique_ptr<State> TabBarWidget::createState() {
    return std::make_unique<TabBarState>();
}

// ════════════════════════════════════════════════════════════════
// TabView Implementation
// ════════════════════════════════════════════════════════════════

WidgetPtr TabViewWidget::build(BuildContext&) {
    int idx = std::clamp(selected_index, 0,
                         children.empty() ? 0 : (int)children.size() - 1);
    if (children.empty()) {
        return container();
    }

    auto outer = container(children[idx]);
    outer->flex(1.0f).width(StyleValue::percent(100.0f));
    return outer;
}

} // namespace enki
