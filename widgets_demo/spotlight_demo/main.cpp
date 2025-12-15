/// @file main.cpp
/// @brief ENKI Advanced Spotlight & Interactive Feature Tour Showcase Demo.

#include "enki/app/app.hpp"
#include "enki/widgets/spotlight.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/stack.hpp"
#include "enki/state/state.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <memory>

using namespace enki;

class SpotlightDemoState : public State {
private:
    std::shared_ptr<SpotlightTourController> tour_ctrl_;
    std::string tour_status_ = "Welcome! Click 'Start Product Tour' to begin guided onboarding.";
    std::vector<SpotlightStep> tour_steps_;

    std::vector<SpotlightStep> buildTourSteps() {
        std::vector<SpotlightStep> steps;

        // Step 1: Navigation Sidebar
        {
            SpotlightStep s;
            s.id = "sidebar_step";
            s.title = "Unified Navigation Sidebar";
            s.description = "Easily jump between Projects, Analytics, Cloud Deployments, and Organization settings.";
            s.target_bounds = Rect{40.0f, 90.0f, 200.0f, 380.0f};
            s.shape = SpotlightShape::RoundedRectangle;
            s.corner_radius = 12.0f;
            s.placement = SpotlightPlacement::Right;
            s.padding = EdgeInsets::all(10.0f);
            steps.push_back(s);
        }

        // Step 2: Live Analytics Metrics
        {
            SpotlightStep s;
            s.id = "metrics_step";
            s.title = "Real-Time System Telemetry";
            s.description = "Monitor live cluster throughput, latency, API consumption, and GPU utilization at a glance.";
            s.target_bounds = Rect{270.0f, 90.0f, 540.0f, 130.0f};
            s.shape = SpotlightShape::RoundedRectangle;
            s.corner_radius = 12.0f;
            s.placement = SpotlightPlacement::Bottom;
            s.padding = EdgeInsets::all(8.0f);
            steps.push_back(s);
        }

        // Step 3: Quick Action Deploy Button
        {
            SpotlightStep s;
            s.id = "action_step";
            s.title = "Instant Cluster Deployment";
            s.description = "One-click deployment to 240 global edge regions with automatic SSL and zero-downtime rollouts.";
            s.target_bounds = Rect{840.0f, 90.0f, 240.0f, 130.0f};
            s.shape = SpotlightShape::RoundedRectangle;
            s.corner_radius = 12.0f;
            s.placement = SpotlightPlacement::Bottom;
            s.padding = EdgeInsets::all(8.0f);
            steps.push_back(s);
        }

        // Step 4: User Profile & Security
        {
            SpotlightStep s;
            s.id = "profile_step";
            s.title = "Account & RBAC Security";
            s.description = "Manage API access tokens, multi-factor authentication, and team member permissions.";
            s.target_bounds = Rect{1015.0f, 22.0f, 44.0f, 44.0f};
            s.shape = SpotlightShape::Circle;
            s.placement = SpotlightPlacement::Bottom;
            s.padding = EdgeInsets::all(6.0f);
            s.next_button_label = "Complete Tour";
            steps.push_back(s);
        }

        return steps;
    }

public:
    void initState() override {
        State::initState();
        tour_ctrl_ = std::make_shared<SpotlightTourController>();
        tour_steps_ = buildTourSteps();
    }

    WidgetPtr build(BuildContext&) override {
        // ── 1. Top Navigation Bar ─────────────────────────────────────
        auto top_navbar = container({
            .color = 0xFF0F172A,
            .border = Border(0xFF1E293B, 1.0f),
            .padding = StyleInsets::symmetric(14.0f, 28.0f),
            .child = row({
                .justify_content = Justify::SpaceBetween,
                .align_items = Align::Center,
                .children = {
                    row({
                        .align_items = Align::Center,
                        .gap = StyleValue::point(12.0f),
                        .children = {
                            text("🌟", { .color = 0xFF38BDF8, .font_size = 20.0f }),
                            text("ENKI WORKBENCH", {
                                .color = 0xFFF8FAFC,
                                .font_size = 16.0f,
                                .font_weight = FontWeight::Bold
                            })
                        }
                    }),
                    row({
                        .align_items = Align::Center,
                        .gap = StyleValue::point(16.0f),
                        .children = {
                            // Start Tour Button
                            gestureDetector({
                                .child = container({
                                    .color = 0xFF0284C7,
                                    .border_radius = BorderRadius::circular(6.0f),
                                    .padding = StyleInsets::symmetric(8.0f, 16.0f),
                                    .child = row({
                                        .align_items = Align::Center,
                                        .gap = StyleValue::point(6.0f),
                                        .children = {
                                            text("✨", { .color = 0xFFFFFFFF, .font_size = 14.0f }),
                                            text("Start Product Tour", {
                                                .color = 0xFFFFFFFF,
                                                .font_size = 13.0f,
                                                .font_weight = FontWeight::Bold
                                            })
                                        }
                                    })
                                }),
                                .cursor_type = SystemCursor::Pointer,
                                .on_tap_up = [this](const TapUpDetails&) {
                                    tour_ctrl_->start();
                                    tour_status_ = "Onboarding tour in progress (Step 1 of 4)...";
                                    setState([] {});
                                }
                            }),
                            // Avatar Icon (Step 4 Target)
                            container({
                                .color = 0x3338BDF8,
                                .border_radius = BorderRadius::circular(22.0f),
                                .border = Border(0xFF38BDF8, 2.0f),
                                .width = StyleValue::point(44.0f),
                                .height = StyleValue::point(44.0f),
                                .child = column({
                                    .justify_content = Justify::Center,
                                    .align_items = Align::Center,
                                    .children = {
                                        text("👤", { .color = 0xFF38BDF8, .font_size = 20.0f })
                                    }
                                })
                            })
                        }
                    })
                }
            })
        });

        // ── 2. Sidebar Navigation (Step 1 Target) ─────────────────────
        auto make_nav_item = [](std::string icon, std::string label, bool active) -> WidgetPtr {
            return container({
                .color = active ? 0x2238BDF8 : 0x00000000,
                .border_radius = BorderRadius::circular(7.0f),
                .border = Border(active ? 0x5538BDF8 : 0x00000000, 1.0f),
                .padding = StyleInsets::symmetric(10.0f, 14.0f),
                .child = row({
                    .align_items = Align::Center,
                    .gap = StyleValue::point(10.0f),
                    .children = {
                        text(icon, { .color = active ? 0xFF38BDF8 : 0xFF94A3B8, .font_size = 16.0f }),
                        text(label, {
                            .color = active ? 0xFFF8FAFC : 0xFFCBD5E1,
                            .font_size = 13.5f,
                            .font_weight = active ? FontWeight::Bold : FontWeight::Medium
                        })
                    }
                })
            });
        };

        auto sidebar = container({
            .color = 0xFF0F172A,
            .border_radius = BorderRadius::circular(12.0f),
            .border = Border(0xFF1E293B, 1.0f),
            .width = StyleValue::point(200.0f),
            .height = StyleValue::point(380.0f),
            .padding = StyleInsets::all(14.0f),
            .child = column({
                .gap = StyleValue::point(6.0f),
                .children = {
                    text("WORKSPACE", { .color = 0xFF64748B, .font_size = 11.0f, .font_weight = FontWeight::Bold }),
                    make_nav_item("📊", "Analytics", true),
                    make_nav_item("📁", "Projects", false),
                    make_nav_item("⚡", "Deployments", false),
                    make_nav_item("👥", "Team Members", false),
                    make_nav_item("⚙️", "Settings", false),
                    make_nav_item("📜", "Audit Logs", false)
                }
            })
        });

        // ── 3. Metrics Card (Step 2 Target) ───────────────────────────
        auto make_metric = [](std::string label, std::string val, std::string change, Color col) -> WidgetPtr {
            return column({
                .gap = StyleValue::point(4.0f),
                .children = {
                    text(label, { .color = 0xFF94A3B8, .font_size = 12.0f }),
                    text(val, { .color = 0xFFF8FAFC, .font_size = 20.0f, .font_weight = FontWeight::Bold }),
                    text(change, { .color = col, .font_size = 11.5f })
                }
            });
        };

        auto metrics_panel = container({
            .color = 0xFF0F172A,
            .border_radius = BorderRadius::circular(12.0f),
            .border = Border(0xFF1E293B, 1.0f),
            .width = StyleValue::point(540.0f),
            .height = StyleValue::point(130.0f),
            .padding = StyleInsets::all(18.0f),
            .child = row({
                .justify_content = Justify::SpaceBetween,
                .align_items = Align::Center,
                .children = {
                    make_metric("Throughput", "4.2M req/s", "↑ 12.4% vs last week", 0xFF10B981),
                    make_metric("Avg Latency", "1.42 ms", "⚡ Sub-millisecond p99", 0xFF38BDF8),
                    make_metric("GPU Compute", "88.4%", "✓ Optimal cluster load", 0xFFF59E0B)
                }
            })
        });

        // ── 4. Action Deploy Card (Step 3 Target) ──────────────────────
        auto deploy_panel = container({
            .color = 0xFF0F172A,
            .border_radius = BorderRadius::circular(12.0f),
            .border = Border(0xFF1E293B, 1.0f),
            .width = StyleValue::point(240.0f),
            .height = StyleValue::point(130.0f),
            .padding = StyleInsets::all(18.0f),
            .child = column({
                .justify_content = Justify::SpaceBetween,
                .children = {
                    text("DEPLOYMENT HUB", { .color = 0xFF64748B, .font_size = 11.0f, .font_weight = FontWeight::Bold }),
                    text("Global Edge Network", { .color = 0xFFF8FAFC, .font_size = 14.0f, .font_weight = FontWeight::Bold }),
                    container({
                        .color = 0xFF10B981,
                        .border_radius = BorderRadius::circular(6.0f),
                        .padding = StyleInsets::symmetric(6.0f, 12.0f),
                        .child = text("🚀 Deploy Cluster", { .color = 0xFF064E3B, .font_size = 12.5f, .font_weight = FontWeight::Bold })
                    })
                }
            })
        });

        // Status Card
        auto status_card = container({
            .color = 0xFF0B0F19,
            .border_radius = BorderRadius::circular(10.0f),
            .border = Border(0xFF1E293B, 1.0f),
            .width = StyleValue::point(800.0f),
            .padding = StyleInsets::all(16.0f),
            .child = row({
                .align_items = Align::Center,
                .gap = StyleValue::point(10.0f),
                .children = {
                    text("ℹ️", { .color = 0xFF38BDF8, .font_size = 16.0f }),
                    text(tour_status_, { .color = 0xFF94A3B8, .font_size = 13.5f })
                }
            })
        });

        // Combine into Dashboard Body
        auto dashboard_body = container({
            .color = 0xFF080C14,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .child = column({
                .gap = StyleValue::point(20.0f),
                .children = {
                    top_navbar,
                    container({
                        .padding = StyleInsets::symmetric(0.0f, 40.0f),
                        .child = row({
                            .gap = StyleValue::point(30.0f),
                            .children = {
                                sidebar,
                                column({
                                    .gap = StyleValue::point(20.0f),
                                    .children = {
                                        row({
                                            .gap = StyleValue::point(20.0f),
                                            .children = {metrics_panel, deploy_panel}
                                        }),
                                        status_card
                                    }
                                })
                            }
                        })
                    })
                }
            })
        });

        // ── Spotlight Overlay Wrap ────────────────────────────────────
        return Spotlight {
            .body = dashboard_body,
            .steps = tour_steps_,
            .options = {
                .overlay_color = 0xCC080C14,
                .pulse_ring_color = 0xFF38BDF8,
                .card_width = 340.0f,
                .on_step_change = [this](size_t idx, const SpotlightStep& step) {
                    tour_status_ = "Viewing Step " + std::to_string(idx + 1) + ": " + step.title;
                    setState([] {});
                },
                .on_finish = [this] {
                    tour_status_ = "🎉 You have completed the product onboarding tour!";
                    setState([] {});
                },
                .on_skip = [this] {
                    tour_status_ = "Tour dismissed. Click 'Start Product Tour' anytime to replay.";
                    setState([] {});
                }
            },
            .controller = tour_ctrl_
        };
    }
};

class SpotlightDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<SpotlightDemoState>();
    }
    [[nodiscard]] std::string_view typeName() const override { return "SpotlightDemoApp"; }
};

int main() {
    std::cout << "====================================================\n";
    std::cout << "  ENKI Engine — Spotlight Feature Tour Showcase Demo\n";
    std::cout << "====================================================\n";

    AppConfig config;
    config.title       = "Enki Engine — Spotlight Feature Tour Showcase Demo";
    config.width       = 1140;
    config.height      = 660;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 0;
    config.show_performance_overlay = true;
    config.clear_color = 0xFF080C14;

    return runApp(std::make_shared<SpotlightDemoApp>(), config);
}
