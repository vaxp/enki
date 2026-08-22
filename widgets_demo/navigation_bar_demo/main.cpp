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
    auto t = text(label, {
        .color = active ? 0xFFFFFFFF : 0xFFCBD5E1,
        .font_size = 11.5f,
        .font_weight = FontWeight::Bold,
    });

    ButtonProps b_opt;
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
    WidgetPtr build(BuildContext& ctx) override {
        WidgetPtr stage_content;

        if (active_demo_mode_ == 0) {
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

            stage_content = column({
                .align_items = Align::Center,
                .gap = StyleValue::point(16.0f),
                .children = {
                    container({
                        .color = 0xFF1E293B,
                        .border_radius = BorderRadius::circular(12.0f),
                        .width = StyleValue::point(580.0f),
                        .padding = StyleInsets::all(32.0f),
                        .child = column({
                            .align_items = Align::Center,
                            .gap = StyleValue::point(8.0f),
                            .children = {
                                text(page_title, { .color = theme_col, .font_size = 16.0f, .font_weight = FontWeight::Bold }),
                                text(page_desc, { .color = 0xFF94A3B8, .font_size = 12.5f })
                            }
                        })
                    }),
                    NavigationBar {
                        .items = {
                            {"Home", Icons::Material::home(), Icons::Material::home(), "", false},
                            {"Explore", Icons::Material::search(), Icons::Material::search(), "", false},
                            {"Alerts", Icons::Material::notifications(), Icons::Material::notifications(), show_badges_ ? "12" : "", false},
                            {"Profile", Icons::Material::person(), Icons::Material::person(), "", show_badges_}
                        },
                        .selected_index = m3_selected_idx_,
                        .on_item_selected = [this](int idx) {
                            setState([this, idx] {
                                m3_selected_idx_ = idx;
                                status_hud_ = "Selected Tab #" + std::to_string(idx + 1) + " (Material 3 Bottom)";
                            });
                        },
                        .on_item_reselect = [this](int idx) {
                            setState([this, idx] {
                                status_hud_ = "⚡ Re-selected Active Tab #" + std::to_string(idx + 1) + " (Scroll to top triggered)";
                            });
                        },
                        .options = {
                            .style = NavigationBarStyle::BottomStandard,
                            .indicator_style = current_ind_style_,
                            .item_layout = current_layout_,
                            .background_color = 0xFF0F172A,
                            .border_color = 0xFF1E293B,
                            .active_color = 0xFF38BDF8,
                            .inactive_color = 0xFF64748B,
                            .indicator_color = 0x2638BDF8,
                            .height = 68.0f
                        }
                    }
                }
            });
        }
        else if (active_demo_mode_ == 1) {
            stage_content = column({
                .align_items = Align::Center,
                .gap = StyleValue::point(18.0f),
                .children = {
                    container({
                        .color = 0xFF131C2E,
                        .border_radius = BorderRadius::circular(12.0f),
                        .border = Border(0xFF1E293B, 1.0f),
                        .width = StyleValue::point(580.0f),
                        .padding = StyleInsets::all(30.0f),
                        .child = column({
                            .align_items = Align::Center,
                            .gap = StyleValue::point(10.0f),
                            .children = {
                                text("🍏 macOS / iOS 18 Island Dock with Multi-Layer Acrylic Blur & Shadow", { .color = 0xFF38BDF8, .font_size = 13.0f, .font_weight = FontWeight::Bold }),
                                text("💻 Interactive Desktop Canvas Area\nHover over dock icons for tooltips and click for bounce physics", { .color = 0xFF94A3B8, .font_size = 12.5f })
                            }
                        })
                    }),
                    NavigationBar {
                        .items = {
                            {"Home", Icons::Material::home(), "", false},
                            {"Workspaces", Icons::Material::folder(), "", false},
                            {"Messages", Icons::Material::chat(), show_badges_ ? "3" : "", false},
                            {"Analytics", Icons::Material::analytics(), "", false},
                            {"Config", Icons::Material::settings(), "", show_badges_}
                        },
                        .selected_index = dock_selected_idx_,
                        .on_item_selected = [this](int idx) {
                            setState([this, idx] {
                                dock_selected_idx_ = idx;
                                status_hud_ = "Floating Dock Active Item changed to #" + std::to_string(idx + 1);
                            });
                        },
                        .options = {
                            .style = NavigationBarStyle::FloatingPill,
                            .indicator_style = current_ind_style_,
                            .item_layout = current_layout_,
                            .background_color = 0xEE1E293B,
                            .border_color = 0x3338BDF8,
                            .indicator_color = 0x3338BDF8,
                            .height = 64.0f,
                            .width = 520.0f,
                            .indicator_radius = 20.0f,
                            .corner_radius = 32.0f,
                            .enable_glassmorphism = glass_effect_
                        }
                    }
                }
            });
        }
        else if (active_demo_mode_ == 2) {
            stage_content = column({
                .align_items = Align::Center,
                .gap = StyleValue::point(14.0f),
                .children = {
                    NavigationBar {
                        .items = {
                            {"Overview", Icons::Material::dashboard(), "", false},
                            {"Builds", Icons::Material::layers(), show_badges_ ? "99+" : "", false},
                            {"Deploy", Icons::Material::public_icon(), "", false},
                            {"Metrics", Icons::Material::analytics(), "", false}
                        },
                        .selected_index = header_selected_idx_,
                        .on_item_selected = [this](int idx) {
                            setState([this, idx] {
                                header_selected_idx_ = idx;
                                status_hud_ = "Desktop Header Nav: Switched view to tab #" + std::to_string(idx + 1);
                            });
                        },
                        .on_action_clicked = [this](std::string_view act) {
                            setState([this, act] {
                                status_hud_ = "Clicked Header Action Button: [" + std::string(act) + "]";
                            });
                        },
                        .options = {
                            .style = NavigationBarStyle::TopHeader,
                            .indicator_style = current_ind_style_ == NavIndicatorStyle::Pill ? NavIndicatorStyle::Underline : current_ind_style_,
                            .item_layout = NavItemLayout::Horizontal,
                            .background_color = 0xFF0F172A,
                            .border_color = 0xFF1E293B,
                            .height = 60.0f,
                            .leading_title = "ENKI Studio",
                            .leading_icon = Icons::Material::bolt(),
                            .show_search_placeholder = true,
                            .trailing_actions = {"Docs", "GitHub", "Sign In"}
                        }
                    },
                    container({
                        .color = 0xFF1E293B,
                        .border_radius = BorderRadius::circular(8.0f),
                        .width = StyleValue::point(640.0f),
                        .padding = StyleInsets::all(36.0f),
                        .child = text("🖥️ Desktop Header Navigation with Brand Logo, Search Input, and Action Buttons", { .color = 0xFF94A3B8, .font_size = 13.0f })
                    })
                }
            });
        }
        else {
            stage_content = column({
                .align_items = Align::Center,
                .gap = StyleValue::point(18.0f),
                .children = {
                    NavigationBar {
                        .items = {
                            {"All Tasks", Icons::Material::check(), "", false},
                            {"In Progress", Icons::Material::refresh(), show_badges_ ? "4" : "", false},
                            {"Completed", Icons::Material::done_all(), "", false},
                            {"Archived", Icons::Material::inventory(), "NEW", false}
                        },
                        .selected_index = seg_selected_idx_,
                        .on_item_selected = [this](int idx) {
                            setState([this, idx] {
                                seg_selected_idx_ = idx;
                                status_hud_ = "Segmented Capsule Filter: Selected #" + std::to_string(idx + 1);
                            });
                        },
                        .options = {
                            .style = NavigationBarStyle::SegmentedCapsule,
                            .indicator_style = NavIndicatorStyle::Pill,
                            .item_layout = NavItemLayout::Horizontal,
                            .background_color = 0xFF0F172A,
                            .active_color = 0xFFFFFFFF,
                            .inactive_color = 0xFF94A3B8,
                            .indicator_color = 0xFF0284C7,
                            .height = 44.0f,
                            .width = 480.0f,
                            .indicator_radius = 18.0f,
                            .indicator_w = 0.0f,
                            .indicator_h = 36.0f,
                            .corner_radius = 22.0f
                        }
                    },
                    container({
                        .color = 0xFF1E293B,
                        .border_radius = BorderRadius::circular(10.0f),
                        .width = StyleValue::point(580.0f),
                        .padding = StyleInsets::all(30.0f),
                        .child = text("⚡ Segmented Filter Capsule with Sliding Pill Background Highlight", { .color = 0xFF38BDF8, .font_size = 13.0f, .font_weight = FontWeight::Bold })
                    })
                }
            });
        }

        return container({
            .color = 0xFF0B1120,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(20.0f),
            .child = column({
                .align_items = Align::Center,
                .gap = StyleValue::point(16.0f),
                .children = {
                    column({
                        .align_items = Align::Center,
                        .gap = StyleValue::point(4.0f),
                        .children = {
                            text("Advanced NavigationBar & Nav Suite", { .color = 0xFFFFFFFF, .font_size = 22.0f, .font_weight = FontWeight::Bold }),
                            text("600+ FPS Direct Skia Hardware Acceleration · Spring/Lerp Physics · 4 Distinct Styles", { .color = 0xFF94A3B8, .font_size = 12.5f })
                        }
                    }),
                    NavigationBar {
                        .items = {
                            {"Material 3 Bottom", Icons::Material::dashboard(), "", false},
                            {"Floating Glass Dock", Icons::Material::layers(), "PRO", false},
                            {"Desktop Top Header", Icons::Material::web(), "", false},
                            {"Segmented Tabs", Icons::Material::tune(), "", false}
                        },
                        .selected_index = active_demo_mode_,
                        .on_item_selected = [this](int idx) {
                            setState([this, idx] {
                                active_demo_mode_ = idx;
                                status_hud_ = "Switched showcase mode to " + std::to_string(idx);
                            });
                        },
                        .options = {
                            .style = NavigationBarStyle::SegmentedCapsule,
                            .indicator_style = NavIndicatorStyle::Pill,
                            .item_layout = NavItemLayout::Horizontal,
                            .background_color = 0xFF0F172A,
                            .active_color = 0xFFFFFFFF,
                            .inactive_color = 0xFF94A3B8,
                            .indicator_color = 0xFF0284C7,
                            .height = 44.0f,
                            .width = 620.0f,
                            .indicator_radius = 18.0f,
                            .indicator_w = 0.0f,
                            .indicator_h = 36.0f,
                            .corner_radius = 22.0f
                        }
                    },
                    stage_content,
                    container({
                        .color = 0xFF0F172A,
                        .border_radius = BorderRadius::circular(12.0f),
                        .border = Border(0xFF334155, 1.0f),
                        .width = StyleValue::point(640.0f),
                        .padding = StyleInsets::all(16.0f),
                        .child = column({
                            .align_items = Align::Center,
                            .gap = StyleValue::point(10.0f),
                            .children = {
                                text("🎛️ Live Indicator & Layout Configurator", { .color = 0xFF38BDF8, .font_size = 13.5f, .font_weight = FontWeight::Bold }),
                                row({
                                    .gap = StyleValue::point(8.0f),
                                    .children = {
                                        makeBtn("Pill Indicator", [this] { setState([this] { current_ind_style_ = NavIndicatorStyle::Pill; status_hud_ = "Indicator: Pill"; }); }, current_ind_style_ == NavIndicatorStyle::Pill),
                                        makeBtn("Underline", [this] { setState([this] { current_ind_style_ = NavIndicatorStyle::Underline; status_hud_ = "Indicator: Underline"; }); }, current_ind_style_ == NavIndicatorStyle::Underline),
                                        makeBtn("Glowing Dot", [this] { setState([this] { current_ind_style_ = NavIndicatorStyle::Dot; status_hud_ = "Indicator: Glowing Dot"; }); }, current_ind_style_ == NavIndicatorStyle::Dot),
                                        makeBtn("Ambient Glow", [this] { setState([this] { current_ind_style_ = NavIndicatorStyle::Glow; status_hud_ = "Indicator: Ambient Glow"; }); }, current_ind_style_ == NavIndicatorStyle::Glow)
                                    }
                                }),
                                row({
                                    .gap = StyleValue::point(8.0f),
                                    .children = {
                                        makeBtn("Vertical", [this] { setState([this] { current_layout_ = NavItemLayout::Vertical; status_hud_ = "Layout: Vertical (Mobile)"; }); }, current_layout_ == NavItemLayout::Vertical),
                                        makeBtn("Horizontal", [this] { setState([this] { current_layout_ = NavItemLayout::Horizontal; status_hud_ = "Layout: Horizontal (Desktop)"; }); }, current_layout_ == NavItemLayout::Horizontal),
                                        makeBtn("Icon Only", [this] { setState([this] { current_layout_ = NavItemLayout::IconOnly; status_hud_ = "Layout: Icon Only (Dock)"; }); }, current_layout_ == NavItemLayout::IconOnly),
                                        makeBtn(show_badges_ ? "Badges: ON" : "Badges: OFF", [this] { setState([this] { show_badges_ = !show_badges_; status_hud_ = "Toggled notification badges"; }); }, show_badges_)
                                    }
                                })
                            }
                        })
                    }),
                    container({
                        .color = 0xFF1E293B,
                        .border_radius = BorderRadius::circular(6.0f),
                        .border = Border(0xFF334155, 1.0f),
                        .width = StyleValue::point(640.0f),
                        .padding = StyleInsets::symmetric(8.0f, 16.0f),
                        .child = text("💡  " + status_hud_, { .color = 0xFF38BDF8, .font_size = 12.0f })
                    })
                }
            })
        });
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
    config.show_performance_overlay = true;
    config.clear_color = 0xFF0B1120;

    return runApp(std::make_shared<NavigationBarDemoApp>(), config);
}
