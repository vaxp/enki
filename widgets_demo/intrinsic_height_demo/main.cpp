/// @file main.cpp
/// @brief ENKI Engine — IntrinsicHeight Showcase Demo
/// @details Roadmap v0.2.0 | Section 11 Layout — Extended

#include "enki/app/app.hpp"
#include "enki/widgets/intrinsic_height.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/divider.hpp"
#include "enki/widgets/scroll_view.hpp"
#include "enki/state/state.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <memory>

using namespace enki;

class IntrinsicHeightDemoApp : public StatefulWidget {
public:
    IntrinsicHeightDemoApp() = default;
    std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "IntrinsicHeightDemoApp"; }
};

class IntrinsicHeightDemoState : public State {
private:
    int extra_enterprise_features_ = 0;
    int step_mode_ = 0; // 0 = None, 1 = 25px, 2 = 50px, 3 = 80px
    std::string last_action_ = "Ready. Click 'Add Feature' or select a plan to test synchronization.";

    [[nodiscard]] std::optional<float> currentStepHeight() const {
        switch (step_mode_) {
            case 1: return 25.0f;
            case 2: return 50.0f;
            case 3: return 80.0f;
            default: return std::nullopt;
        }
    }

    [[nodiscard]] std::vector<std::string> getStarterFeatures() const {
        return {
            "✓ 5 Microservice Nodes",
            "✓ 10 GB Encrypted Storage",
            "✓ Community Discord Support",
        };
    }

    [[nodiscard]] std::vector<std::string> getProFeatures() const {
        return {
            "✓ 25 Microservice Nodes",
            "✓ 100 GB NVMe Storage",
            "✓ Zero-Downtime Rollouts",
            "✓ Priority 24/7 SLA Support",
            "✓ Automated Nightly Snapshots",
        };
    }

    [[nodiscard]] std::vector<std::string> getEnterpriseFeatures() const {
        std::vector<std::string> f = {
            "✓ Unlimited Distributed Nodes",
            "✓ 2 TB Dedicated NVMe Storage",
            "✓ Custom VPC & Dedicated Gateway",
            "✓ Real-Time Threat Intelligence",
            "✓ 99.999% High Availability SLA",
        };
        for (int i = 1; i <= extra_enterprise_features_; ++i) {
            f.push_back("✓ Dynamic Cluster Extension #" + std::to_string(i));
        }
        return f;
    }

public:
    WidgetPtr build(BuildContext&) override {
        auto starter_features = getStarterFeatures();
        auto pro_features = getProFeatures();
        auto ent_features = getEnterpriseFeatures();
        auto step = currentStepHeight();

        // ── 1. Header ─────────────────────────────────────────────────
        auto header = container({
            .color = 0xFF0F172A,
            .border_radius = BorderRadius::circular(14.0f),
            .border = Border(0xFF1E293B, 1.0f),
            .width = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(20.0f),
            .child = column({
                .gap = StyleValue::point(6.0f),
                .children = {
                    row({
                        .align_items = Align::Center,
                        .gap = StyleValue::point(10.0f),
                        .children = {
                            text("ENKI Engine — IntrinsicHeight Studio Showcase", {
                                .color = 0xFFF8FAFC,
                                .font_size = 22.0f,
                                .font_weight = FontWeight::Bold,
                            }),
                            container({
                                .color = 0xFF059669,
                                .border_radius = BorderRadius::circular(6.0f),
                                .padding = StyleInsets::symmetric(3.0f, 8.0f),
                                .child = text("ROADMAP v0.2.0 §11", {
                                    .color = 0xFFD1FAE5,
                                    .font_size = 11.0f,
                                    .font_weight = FontWeight::Bold,
                                }),
                            }),
                        },
                    }),
                    text("Demonstrating equal-height card layouts, full-span vertical dividers, and step height snapping without manual pixel heights.", {
                        .color = 0xFF94A3B8,
                        .font_size = 13.0f,
                    }),
                },
            }),
        });

        // ── 2. Interactive Controls Toolbar ────────────────────────────
        auto step_btn = [this](int id, std::string label) {
            bool active = (step_mode_ == id);
            return button(
                text(label, {
                    .color = active ? 0xFFFFFFFF : 0xFF94A3B8,
                    .font_size = 13.0f,
                    .font_weight = active ? FontWeight::Bold : FontWeight::Normal,
                }),
                [this, id]() {
                    setState([this, id]() { step_mode_ = id; });
                },
                {
                    .normal_color = active ? 0xFF10B981 : 0xFF1E293B,
                    .hover_color  = active ? 0xFF059669 : 0xFF334155,
                    .border_radius = 8.0f,
                    .padding = EdgeInsets::symmetric(8.0f, 14.0f),
                }
            );
        };

        auto controls_panel = container({
            .color = 0xFF131B2E,
            .border_radius = BorderRadius::circular(12.0f),
            .border = Border(0xFF1E293B, 1.0f),
            .width = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(16.0f),
            .child = row({
                .justify_content = Justify::SpaceBetween,
                .align_items = Align::Center,
                .children = {
                    column({
                        .gap = StyleValue::point(8.0f),
                        .children = {
                            text("Dynamic Content Mutator:", {
                                .color = 0xFFCBD5E1,
                                .font_size = 12.0f,
                                .font_weight = FontWeight::Bold,
                            }),
                            row({
                                .gap = StyleValue::point(10.0f),
                                .children = {
                                    button(
                                        text("Add Enterprise Feature (+)", {.color = 0xFFFFFFFF, .font_size = 13.0f, .font_weight = FontWeight::Bold}),
                                        [this]() {
                                            setState([this]() {
                                                extra_enterprise_features_++;
                                                last_action_ = "Added feature to Enterprise card. All sibling cards expanded in lockstep!";
                                            });
                                        },
                                        {
                                            .normal_color = 0xFF6366F1,
                                            .hover_color = 0xFF4F46E5,
                                            .border_radius = 8.0f,
                                            .padding = EdgeInsets::symmetric(8.0f, 14.0f),
                                        }
                                    ),
                                    button(
                                        text("Reset Content", {.color = 0xFFE2E8F0, .font_size = 13.0f}),
                                        [this]() {
                                            setState([this]() {
                                                extra_enterprise_features_ = 0;
                                                last_action_ = "Reset Enterprise card to default size.";
                                            });
                                        },
                                        {
                                            .normal_color = 0xFF334155,
                                            .hover_color = 0xFF475569,
                                            .border_radius = 8.0f,
                                            .padding = EdgeInsets::symmetric(8.0f, 14.0f),
                                        }
                                    ),
                                },
                            }),
                        },
                    }),
                    column({
                        .gap = StyleValue::point(8.0f),
                        .children = {
                            text("Step Height Quantization:", {
                                .color = 0xFFCBD5E1,
                                .font_size = 12.0f,
                                .font_weight = FontWeight::Bold,
                            }),
                            row({
                                .gap = StyleValue::point(8.0f),
                                .children = {
                                    step_btn(0, "Natural (0px)"),
                                    step_btn(1, "Snap 25px"),
                                    step_btn(2, "Snap 50px"),
                                    step_btn(3, "Snap 80px"),
                                },
                            }),
                        },
                    }),
                },
            }),
        });

        // ── 3. Helper to Build a Pricing Card ──────────────────────────
        auto build_tier_card = [this](std::string tier, std::string price, uint32_t header_color,
                                      const std::vector<std::string>& features, bool unified) {
            std::vector<WidgetPtr> feature_widgets;
            for (const auto& f : features) {
                feature_widgets.push_back(text(f, {
                    .color = 0xFF94A3B8,
                    .font_size = 12.0f,
                }));
            }

            auto features_col = column({
                .gap = StyleValue::point(6.0f),
                .children = feature_widgets,
            });

            auto action_btn = button(
                text("Select " + tier, {.color = 0xFFFFFFFF, .font_size = 13.0f, .font_weight = FontWeight::Bold}),
                [this, tier]() {
                    setState([this, tier]() {
                        last_action_ = "Selected plan: [" + tier + "]";
                    });
                },
                {
                    .normal_color = header_color,
                    .hover_color  = header_color + 0x00111111,
                    .border_radius = 6.0f,
                    .padding = EdgeInsets::symmetric(10.0f, 16.0f),
                }
            );

            std::vector<WidgetPtr> card_children;
            card_children.push_back(row({
                .justify_content = Justify::SpaceBetween,
                .align_items = Align::Center,
                .children = {
                    text(tier, {.color = 0xFFF1F5F9, .font_size = 16.0f, .font_weight = FontWeight::Bold}),
                    text(price, {.color = header_color, .font_size = 15.0f, .font_weight = FontWeight::Bold}),
                },
            }));
            card_children.push_back(divider({.thickness = 1.0f, .color = 0xFF1E293B}));
            card_children.push_back(features_col);
            card_children.push_back(action_btn);

            return container({
                .color = 0xFF111827,
                .border_radius = BorderRadius::circular(12.0f),
                .border = Border(0xFF1F2937, 1.0f),
                .padding = StyleInsets::all(16.0f),
                .flex = 1.0f,
                .flex_grow = 1.0f,
                .flex_shrink = 1.0f,
                .flex_basis = StyleValue::point(0.0f),
                .child = column({
                    .justify_content = Justify::SpaceBetween,
                    .flex_grow = unified ? std::optional<float>(1.0f) : std::nullopt,
                    .gap = StyleValue::point(12.0f),
                    .children = card_children,
                }),
            });
        };

        // ── 4. Standard Row Section (BEFORE: Uneven Heights) ───────────
        auto standard_row = row({
            .justify_content = Justify::SpaceBetween,
            .align_items = Align::Start,
            .gap = StyleValue::point(16.0f),
            .children = {
                build_tier_card("Starter", "$29/mo", 0xFF6366F1, starter_features, false),
                verticalDivider({.thickness = 1.0f, .color = 0xFF334155}),
                build_tier_card("Professional", "$99/mo", 0xFF0284C7, pro_features, false),
                verticalDivider({.thickness = 1.0f, .color = 0xFF334155}),
                build_tier_card("Enterprise", "$499/mo", 0xFF10B981, ent_features, false),
            },
        });

        auto standard_section = container({
            .color = 0xFF0F172A,
            .border_radius = BorderRadius::circular(12.0f),
            .border = Border(0xFF334155, 1.0f),
            .padding = StyleInsets::all(20.0f),
            .child = column({
                .gap = StyleValue::point(14.0f),
                .children = {
                    row({
                        .align_items = Align::Center,
                        .gap = StyleValue::point(8.0f),
                        .children = {
                            container({
                                .color = 0xFFEF4444,
                                .border_radius = BorderRadius::circular(4.0f),
                                .padding = StyleInsets::symmetric(3.0f, 6.0f),
                                .child = text("BEFORE", {.color = 0xFFFFFFFF, .font_size = 10.0f, .font_weight = FontWeight::Bold}),
                            }),
                            text("Standard Row (Uneven Ragged Heights)", {
                                .color = 0xFFF1F5F9,
                                .font_size = 15.0f,
                                .font_weight = FontWeight::Bold,
                            }),
                        },
                    }),
                    text("Cards stop at their own independent content height. Action buttons are jagged at different vertical positions, and vertical dividers fail to span.", {
                        .color = 0xFF64748B,
                        .font_size = 12.0f,
                    }),
                    standard_row,
                },
            }),
        });

        // ── 5. IntrinsicHeight Section (AFTER: Equal Heights) ──────────
        auto unified_inner_row = row({
            .justify_content = Justify::SpaceBetween,
            .align_items = Align::Stretch,
            .gap = StyleValue::point(16.0f),
            .width = StyleValue::percent(100.0f),
            .children = {
                build_tier_card("Starter", "$29/mo", 0xFF6366F1, starter_features, true),
                verticalDivider({.thickness = 1.5f, .color = 0xFF475569}),
                build_tier_card("Professional", "$99/mo", 0xFF0284C7, pro_features, true),
                verticalDivider({.thickness = 1.5f, .color = 0xFF475569}),
                build_tier_card("Enterprise", "$499/mo", 0xFF10B981, ent_features, true),
            },
        });

        auto unified_row_wrapped = intrinsicHeight({
            .step_height = step,
            .child = unified_inner_row,
        });

        auto unified_section = container({
            .color = 0xFF0F172A,
            .border_radius = BorderRadius::circular(12.0f),
            .border = Border(0xFF10B981, 1.5f),
            .padding = StyleInsets::all(20.0f),
            .child = column({
                .gap = StyleValue::point(14.0f),
                .children = {
                    row({
                        .align_items = Align::Center,
                        .gap = StyleValue::point(8.0f),
                        .children = {
                            container({
                                .color = 0xFF10B981,
                                .border_radius = BorderRadius::circular(4.0f),
                                .padding = StyleInsets::symmetric(3.0f, 6.0f),
                                .child = text("AFTER", {.color = 0xFFFFFFFF, .font_size = 10.0f, .font_weight = FontWeight::Bold}),
                            }),
                            text("IntrinsicHeight (Unified Heights & Clean Baseline)", {
                                .color = 0xFFF1F5F9,
                                .font_size = 15.0f,
                                .font_weight = FontWeight::Bold,
                            }),
                        },
                    }),
                    text("All 3 cards and vertical dividers automatically stretch to match the tallest card in the row. Action buttons align on a crisp bottom baseline.", {
                        .color = 0xFF64748B,
                        .font_size = 12.0f,
                    }),
                    unified_row_wrapped,
                },
            }),
        });

        // ── 6. Live Telemetry Card ─────────────────────────────────────
        std::string step_str = step ? (std::to_string(static_cast<int>(*step)) + " px") : "None (Natural)";
        auto telemetry_card = container({
            .color = 0xFF0B1220,
            .border_radius = BorderRadius::circular(10.0f),
            .border = Border(0xFF1E293B, 1.0f),
            .padding = StyleInsets::symmetric(14.0f, 18.0f),
            .child = row({
                .justify_content = Justify::SpaceBetween,
                .align_items = Align::Center,
                .children = {
                    row({
                        .gap = StyleValue::point(16.0f),
                        .children = {
                            text("Active Step: " + step_str, {
                                .color = 0xFF34D399,
                                .font_size = 13.0f,
                                .font_weight = FontWeight::Bold,
                            }),
                            text("Enterprise Features: " + std::to_string(ent_features.size()), {
                                .color = 0xFF38BDF8,
                                .font_size = 13.0f,
                                .font_weight = FontWeight::Bold,
                            }),
                        },
                    }),
                    text(last_action_, {
                        .color = 0xFF94A3B8,
                        .font_size = 12.0f,
                    }),
                },
            }),
        });

        // ── Page Assembly ──────────────────────────────────────────────
        auto main_column = column({
            .gap = StyleValue::point(24.0f),
            .width = StyleValue::percent(100.0f),
            .children = {
                header,
                controls_panel,
                standard_section,
                unified_section,
                telemetry_card,
            },
        });

        return container({
            .color = 0xFF060911,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(24.0f),
            .child = scrollView(main_column),
        });
    }
};

std::unique_ptr<State> IntrinsicHeightDemoApp::createState() {
    return std::make_unique<IntrinsicHeightDemoState>();
}

int main() {
    std::cout << "====================================================" << std::endl;
    std::cout << "  ENKI Engine — IntrinsicHeight Studio Showcase Demo" << std::endl;
    std::cout << "  Roadmap v0.2.0 | Section 11 Layout — Extended" << std::endl;
    std::cout << "====================================================" << std::endl;

    AppConfig config;
    config.title       = "ENKI Engine — IntrinsicHeight Showcase Demo";
    config.width       = 1140;
    config.height      = 840;
    config.target_fps  = 0;
    config.vsync       = false;
    config.clear_color = 0xFF060911;

    return runApp(std::make_shared<IntrinsicHeightDemoApp>(), config);
}
