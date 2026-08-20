/// @file main.cpp
/// @brief ENKI Advanced NavigationBar Suite Showcase Demo.

#include "enki/app/app.hpp"
#include "enki/widgets/navigation_bar.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/divider.hpp"
#include "enki/state/state.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <memory>

using namespace enki;

// Helper to create a stylized pill button
static WidgetPtr makeBtn(const std::string& label, std::function<void()> onClick, bool active = false) {
    auto t = text(label);
    t->fontSize(11.5f).bold().color(active ? 0xFFFFFFFF : 0xFFCBD5E1);

    ButtonOptions b_opt;
    b_opt.normal_color  = active ? 0xFF0284C7 : 0xFF1E293B;
    b_opt.hover_color   = active ? 0xFF0369A1 : 0xFF334155;
    b_opt.pressed_color = 0xFF075985;
    b_opt.border_radius = 6.0f;
    b_opt.padding       = EdgeInsets::symmetric(6.0f, 12.0f);
    b_opt.min_height    = 28.0f;
    b_opt.min_width     = 50.0f;

    return button(t, std::move(onClick), b_opt);
}

// ── State for Interactive NavigationBar Showcase ────────────────────────────

class NavigationBarDemoState : public State {
    int active_demo_mode_   = 0; // 0: Material 3 Bottom, 1: Floating Dock, 2: Desktop Header, 3: Segmented
    int m3_selected_idx_    = 0;
    int dock_selected_idx_  = 1;
    int header_selected_idx_= 0;
    int seg_selected_idx_   = 0;

    NavIndicatorStyle current_ind_style_ = NavIndicatorStyle::Pill;
    NavItemLayout     current_layout_    = NavItemLayout::Vertical;
    bool              show_badges_       = true;
    bool              glass_effect_      = true;
    std::string       status_hud_        = "Ready. Tap any navigation item to test 600+ FPS sliding indicator.";

public:
    WidgetPtr build(BuildContext&) override {
        // ── Header Title ─────────────────────────────────────────
        auto title = text("Advanced NavigationBar & Nav Suite");
        title->fontSize(22.0f).bold().color(0xFFFFFFFF);
        auto sub = text("600+ FPS Direct Skia Hardware Acceleration · Spring/Lerp Physics · 4 Distinct Styles");
        sub->fontSize(12.5f).color(0xFF94A3B8);
        auto hdr = column(std::vector<WidgetPtr>{title, sub});
        hdr->alignItems(Align::Center).gap(StyleValue::point(4.0f));

        // ── Mode Switcher Tab Bar ────────────────────────────────
        std::vector<NavigationBarItem> mode_items = {
            NavigationBarItem("Material 3 Bottom", Icons::Material::dashboard(), "", false),
            NavigationBarItem("Floating Glass Dock", Icons::Material::layers(), "PRO", false),
            NavigationBarItem("Desktop Top Header", Icons::Material::web(), "", false),
            NavigationBarItem("Segmented Tabs", Icons::Material::tune(), "", false),
        };

        auto mode_bar = segmentedNavBar(mode_items, active_demo_mode_, [this](int idx) {
            setState([this, idx] {
                active_demo_mode_ = idx;
                status_hud_ = "Switched showcase mode to " + std::to_string(idx);
            });
        }, 620.0f);

        // ── Main Stage Area based on active mode ─────────────────
        WidgetPtr stage_content;

        // ─────────────────────────────────────────────────────────
        // MODE 0: Material 3 Bottom Navigation Bar
        // ─────────────────────────────────────────────────────────
        if (active_demo_mode_ == 0) {
            std::vector<NavigationBarItem> m3_items = {
                NavigationBarItem("Home", Icons::Material::home(), Icons::Material::home(), "", false),
                NavigationBarItem("Explore", Icons::Material::search(), Icons::Material::search(), "", false),
                NavigationBarItem("Alerts", Icons::Material::notifications(), Icons::Material::notifications(), show_badges_ ? "12" : "", false),
                NavigationBarItem("Profile", Icons::Material::person(), Icons::Material::person(), "", show_badges_),
            };
            m3_items[0].tooltip = "Go to Home Dashboard";
            m3_items[1].tooltip = "Search & Discover Content";
            m3_items[2].tooltip = "12 New Unread Notifications";
            m3_items[3].tooltip = "User Settings & Preferences";

            NavigationBarOptions opt;
            opt.style           = NavigationBarStyle::BottomStandard;
            opt.indicator_style = current_ind_style_;
            opt.item_layout     = current_layout_;
            opt.background_color= 0xFF0F172A;
            opt.border_color    = 0xFF1E293B;
            opt.active_color    = 0xFF38BDF8;
            opt.inactive_color  = 0xFF64748B;
            opt.indicator_color = 0x2638BDF8;
            opt.height          = 68.0f;

            auto nav = navigationBar(m3_items, m3_selected_idx_, [this](int idx) {
                setState([this, idx] {
                    m3_selected_idx_ = idx;
                    status_hud_ = "Selected Tab #" + std::to_string(idx + 1) + " (Material 3 Bottom)";
                });
            }, opt);
            nav->onReselect([this](int idx) {
                setState([this, idx] {
                    status_hud_ = "⚡ Re-selected Active Tab #" + std::to_string(idx + 1) + " (Scroll to top triggered)";
                });
            });

            // Mock page viewport
            std::string page_title;
            std::string page_desc;
            Color       theme_col = 0xFF38BDF8;
            if (m3_selected_idx_ == 0) {
                page_title = "🏠 Home Feed & Daily Digest";
                page_desc  = "Real-time updates, curated recommendations, and recent activity logs.";
            } else if (m3_selected_idx_ == 1) {
                page_title = "🔍 Global Explore & Search Engine";
                page_desc  = "Discover trending repositories, packages, and design components.";
                theme_col  = 0xFF818CF8;
            } else if (m3_selected_idx_ == 2) {
                page_title = "🔔 Notifications & Alerts (12 unread)";
                page_desc  = "Build pipelines succeeded, pull requests reviewed, and security audit passed.";
                theme_col  = 0xFFF59E0B;
            } else {
                page_title = "👤 User Profile & Account Settings";
                page_desc  = "Signed in as @antigravity-engineer · PRO Member · Hardware acceleration enabled.";
                theme_col  = 0xFF10B981;
            }

            auto pt = text(page_title); pt->fontSize(16.0f).bold().color(theme_col);
            auto pd = text(page_desc);  pd->fontSize(12.5f).color(0xFF94A3B8);
            auto page_card = column(std::vector<WidgetPtr>{pt, pd});
            page_card->gap(StyleValue::point(8.0f)).alignItems(Align::Center);

            auto page_box = container(page_card);
            page_box->color(0xFF1E293B).borderRadius(12.0f).paddingAll(32.0f).width(580.0f);

            auto stage_col = column(std::vector<WidgetPtr>{page_box, nav});
            stage_col->gap(StyleValue::point(16.0f)).alignItems(Align::Center);
            stage_content = stage_col;
        }
        // ─────────────────────────────────────────────────────────
        // MODE 1: macOS / iOS 18 Floating Glass Dock
        // ─────────────────────────────────────────────────────────
        else if (active_demo_mode_ == 1) {
            std::vector<NavigationBarItem> dock_items = {
                NavigationBarItem("Home", Icons::Material::home(), "", false),
                NavigationBarItem("Workspaces", Icons::Material::folder(), "", false),
                NavigationBarItem("Messages", Icons::Material::chat(), show_badges_ ? "3" : "", false),
                NavigationBarItem("Analytics", Icons::Material::analytics(), "", false),
                NavigationBarItem("Config", Icons::Material::settings(), "", show_badges_),
            };
            dock_items[0].tooltip = "macOS Finder / Home";
            dock_items[1].tooltip = "Workspaces & Git Trees";
            dock_items[2].tooltip = "Direct Team Chat (3 unread)";
            dock_items[3].tooltip = "FPS & Performance Telemetry";
            dock_items[4].tooltip = "Settings & Customizations";

            NavigationBarOptions d_opt;
            d_opt.style           = NavigationBarStyle::FloatingPill;
            d_opt.indicator_style = current_ind_style_;
            d_opt.item_layout     = current_layout_;
            d_opt.width           = 520.0f;
            d_opt.height          = 66.0f;
            d_opt.corner_radius   = 33.0f;
            d_opt.background_color= 0xD91E293B;
            d_opt.border_color    = 0x4D38BDF8;
            d_opt.active_color    = 0xFF38BDF8;
            d_opt.indicator_color = 0x3338BDF8;
            d_opt.enable_glassmorphism = glass_effect_;

            auto dock = floatingNavBar(dock_items, dock_selected_idx_, [this](int idx) {
                setState([this, idx] {
                    dock_selected_idx_ = idx;
                    status_hud_ = "Floating Dock Active Item changed to #" + std::to_string(idx + 1);
                });
            }, 520.0f, d_opt);

            auto dock_desc = text("🍏 macOS / iOS 18 Island Dock with Multi-Layer Acrylic Blur & Shadow");
            dock_desc->fontSize(13.0f).color(0xFF38BDF8).bold();

            auto mock_screen_txt = text("💻 Interactive Desktop Canvas Area\nHover over dock icons for tooltips and click for bounce physics");
            mock_screen_txt->fontSize(12.5f).color(0xFF94A3B8);

            auto mock_box = column(std::vector<WidgetPtr>{dock_desc, mock_screen_txt});
            mock_box->gap(StyleValue::point(10.0f)).alignItems(Align::Center);

            auto preview = container(mock_box);
            preview->color(0xFF131C2E).borderRadius(12.0f).border(0xFF1E293B, 1.0f).paddingAll(30.0f).width(580.0f);

            auto stage_col = column(std::vector<WidgetPtr>{preview, dock});
            stage_col->gap(StyleValue::point(18.0f)).alignItems(Align::Center);
            stage_content = stage_col;
        }
        // ─────────────────────────────────────────────────────────
        // MODE 2: Desktop Top Header Bar
        // ─────────────────────────────────────────────────────────
        else if (active_demo_mode_ == 2) {
            std::vector<NavigationBarItem> head_items = {
                NavigationBarItem("Overview", Icons::Material::dashboard(), "", false),
                NavigationBarItem("Builds", Icons::Material::layers(), show_badges_ ? "99+" : "", false),
                NavigationBarItem("Deploy", Icons::Material::public_icon(), "", false),
                NavigationBarItem("Metrics", Icons::Material::analytics(), "", false),
            };

            NavigationBarOptions h_opt;
            h_opt.style           = NavigationBarStyle::TopHeader;
            h_opt.indicator_style = current_ind_style_ == NavIndicatorStyle::Pill ? NavIndicatorStyle::Underline : current_ind_style_;
            h_opt.item_layout     = NavItemLayout::Horizontal;
            h_opt.height          = 58.0f;
            h_opt.background_color= 0xFF0F172A;
            h_opt.border_color    = 0xFF334155;
            h_opt.active_color    = 0xFF38BDF8;
            h_opt.leading_title   = "ENKI Studio";
            h_opt.leading_subtitle= "v2.5 Desktop";
            h_opt.leading_icon    = Icons::Material::bolt();
            h_opt.show_search_placeholder = true;
            h_opt.trailing_actions= {"Docs", "GitHub", "Sign In"};

            auto header_bar = topNavigationBar(head_items, header_selected_idx_, [this](int idx) {
                setState([this, idx] {
                    header_selected_idx_ = idx;
                    status_hud_ = "Desktop Header Nav: Switched view to tab #" + std::to_string(idx + 1);
                });
            }, "ENKI Studio", Icons::Material::bolt(), h_opt);

            header_bar->onAction([this](std::string_view act) {
                setState([this, act] {
                    status_hud_ = "Clicked Header Action Button: [" + std::string(act) + "]";
                });
            });

            auto mock_browser_body = text("🖥️ Desktop Header Navigation with Brand Logo, Search Input, and Action Buttons");
            mock_browser_body->fontSize(13.0f).color(0xFF94A3B8);
            auto m_box = container(mock_browser_body);
            m_box->color(0xFF1E293B).paddingAll(36.0f).width(640.0f).borderRadius(8.0f);

            auto wrap_col = column(std::vector<WidgetPtr>{header_bar, m_box});
            wrap_col->gap(StyleValue::point(14.0f)).alignItems(Align::Center);
            stage_content = wrap_col;
        }
        // ─────────────────────────────────────────────────────────
        // MODE 3: Segmented Capsule Tabs
        // ─────────────────────────────────────────────────────────
        else {
            std::vector<NavigationBarItem> seg_items = {
                NavigationBarItem("All Tasks", Icons::Material::check(), "", false),
                NavigationBarItem("In Progress", Icons::Material::refresh(), show_badges_ ? "4" : "", false),
                NavigationBarItem("Completed", Icons::Material::done_all(), "", false),
                NavigationBarItem("Archived", Icons::Material::inventory(), "NEW", false),
            };

            NavigationBarOptions s_opt;
            s_opt.style           = NavigationBarStyle::SegmentedCapsule;
            s_opt.indicator_style = NavIndicatorStyle::Pill;
            s_opt.item_layout     = NavItemLayout::Horizontal;
            s_opt.width           = 480.0f;
            s_opt.height          = 46.0f;
            s_opt.corner_radius   = 23.0f;
            s_opt.background_color= 0xFF0F172A;
            s_opt.indicator_color = 0xFF0284C7;
            s_opt.active_color    = 0xFFFFFFFF;
            s_opt.inactive_color  = 0xFF94A3B8;

            auto seg = segmentedNavBar(seg_items, seg_selected_idx_, [this](int idx) {
                setState([this, idx] {
                    seg_selected_idx_ = idx;
                    status_hud_ = "Segmented Capsule Filter: Selected #" + std::to_string(idx + 1);
                });
            }, 480.0f, s_opt);

            auto desc = text("⚡ Segmented Filter Capsule with Sliding Pill Background Highlight");
            desc->fontSize(13.0f).color(0xFF38BDF8).bold();

            auto box = container(desc);
            box->color(0xFF1E293B).borderRadius(10.0f).paddingAll(30.0f).width(580.0f);

            auto stage_col = column(std::vector<WidgetPtr>{seg, box});
            stage_col->gap(StyleValue::point(18.0f)).alignItems(Align::Center);
            stage_content = stage_col;
        }

        // ── Controls Deck Panel ──────────────────────────────────
        auto ctrl_hdr = text("🎛️ Live Indicator & Layout Configurator");
        ctrl_hdr->fontSize(13.5f).bold().color(0xFF38BDF8);

        // Indicator Style Buttons
        auto b_pill = makeBtn("Pill Indicator", [this] {
            setState([this] { current_ind_style_ = NavIndicatorStyle::Pill; status_hud_ = "Indicator: Pill"; });
        }, current_ind_style_ == NavIndicatorStyle::Pill);

        auto b_under = makeBtn("Underline", [this] {
            setState([this] { current_ind_style_ = NavIndicatorStyle::Underline; status_hud_ = "Indicator: Underline"; });
        }, current_ind_style_ == NavIndicatorStyle::Underline);

        auto b_dot = makeBtn("Glowing Dot", [this] {
            setState([this] { current_ind_style_ = NavIndicatorStyle::Dot; status_hud_ = "Indicator: Glowing Dot"; });
        }, current_ind_style_ == NavIndicatorStyle::Dot);

        auto b_glow = makeBtn("Ambient Glow", [this] {
            setState([this] { current_ind_style_ = NavIndicatorStyle::Glow; status_hud_ = "Indicator: Ambient Glow"; });
        }, current_ind_style_ == NavIndicatorStyle::Glow);

        auto row_inds = row(std::vector<WidgetPtr>{b_pill, b_under, b_dot, b_glow});
        row_inds->gap(StyleValue::point(8.0f));

        // Layout Buttons
        auto b_vert = makeBtn("Vertical", [this] {
            setState([this] { current_layout_ = NavItemLayout::Vertical; status_hud_ = "Layout: Vertical (Mobile)"; });
        }, current_layout_ == NavItemLayout::Vertical);

        auto b_horiz = makeBtn("Horizontal", [this] {
            setState([this] { current_layout_ = NavItemLayout::Horizontal; status_hud_ = "Layout: Horizontal (Desktop)"; });
        }, current_layout_ == NavItemLayout::Horizontal);

        auto b_icon_only = makeBtn("Icon Only", [this] {
            setState([this] { current_layout_ = NavItemLayout::IconOnly; status_hud_ = "Layout: Icon Only (Dock)"; });
        }, current_layout_ == NavItemLayout::IconOnly);

        auto b_badge = makeBtn(show_badges_ ? "Badges: ON" : "Badges: OFF", [this] {
            setState([this] { show_badges_ = !show_badges_; status_hud_ = "Toggled notification badges"; });
        }, show_badges_);

        auto row_layouts = row(std::vector<WidgetPtr>{b_vert, b_horiz, b_icon_only, b_badge});
        row_layouts->gap(StyleValue::point(8.0f));

        auto ctrl_col = column(std::vector<WidgetPtr>{ctrl_hdr, row_inds, row_layouts});
        ctrl_col->gap(StyleValue::point(10.0f)).alignItems(Align::Center);

        auto ctrl_card = container(ctrl_col);
        ctrl_card->color(0xFF0F172A)
                 .border(0xFF334155, 1.0f)
                 .borderRadius(12.0f)
                 .paddingAll(16.0f)
                 .width(640.0f);

        // ── Real-time Status HUD ─────────────────────────────────
        auto hud_txt = text("💡  " + status_hud_);
        hud_txt->fontSize(12.0f).color(0xFF38BDF8);
        auto hud_card = container(hud_txt);
        hud_card->color(0xFF1E293B)
                .border(0xFF334155, 1.0f)
                .borderRadius(6.0f)
                .paddingSymmetric(8.0f, 16.0f)
                .width(640.0f);

        // ── Page Assembly ────────────────────────────────────────
        auto page = column(std::vector<WidgetPtr>{hdr, mode_bar, stage_content, ctrl_card, hud_card});
        page->gap(StyleValue::point(16.0f)).alignItems(Align::Center);

        auto root = container(page);
        root->color(0xFF0B1120)
            .paddingAll(20.0f)
            .width(StyleValue::percent(100.0f))
            .height(StyleValue::percent(100.0f));

        return root;
    }
};

class NavigationBarDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<NavigationBarDemoState>();
    }
    std::string_view typeName() const override { return "NavigationBarDemoApp"; }
};

int main() {
    std::cout << "====================================================\n";
    std::cout << "  ENKI Engine — Advanced NavigationBar Suite Demo\n";
    std::cout << "====================================================\n";

    AppConfig config;
    config.title       = "Enki — Advanced NavigationBar Suite Showcase";
    config.width       = 740;
    config.height      = 840;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 60;
    config.clear_color = 0xFF0B1120;

    return runApp(std::make_shared<NavigationBarDemoApp>(), config);
}
