/// @file main.cpp
/// @brief ENKI Engine — IntrinsicWidth Showcase Demo
/// @details Roadmap v0.2.0 | Section 11 Layout — Extended

#include "enki/app/app.hpp"
#include "enki/widgets/intrinsic_width.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/scroll_view.hpp"
#include "enki/widgets/badge.hpp"
#include "enki/state/state.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <memory>

using namespace enki;

class IntrinsicWidthDemoApp : public StatefulWidget {
public:
    IntrinsicWidthDemoApp() = default;
    std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "IntrinsicWidthDemoApp"; }
};

class IntrinsicWidthDemoState : public State {
private:
    int text_preset_ = 1; // 0 = Short, 1 = Balanced, 2 = Ultra-Wide
    int step_mode_   = 0; // 0 = None, 1 = 20px, 2 = 50px, 3 = 100px
    std::string last_action_ = "Ready. Click any action button to test hit-testing and event bubbling.";

    [[nodiscard]] std::optional<float> currentStepWidth() const {
        switch (step_mode_) {
            case 1: return 20.0f;
            case 2: return 50.0f;
            case 3: return 100.0f;
            default: return std::nullopt;
        }
    }

    [[nodiscard]] std::vector<std::string> getLabels() const {
        if (text_preset_ == 0) {
            return {"Save", "Deploy", "Cancel", "Docs"};
        } else if (text_preset_ == 1) {
            return {
                "Quick Save",
                "Deploy Release Container",
                "Revert Staging Changes",
                "View API Documentation"
            };
        } else {
            return {
                "Save Local Draft",
                "Execute Multi-Region Kubernetes Zero-Downtime Rollout",
                "Abort Transaction",
                "Generate Telemetry Report"
            };
        }
    }

public:
    WidgetPtr build(BuildContext&) override {
        auto labels = getLabels();
        auto step = currentStepWidth();

        // ── 1. Top Header ─────────────────────────────────────────────
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
                            text("ENKI Engine — IntrinsicWidth Studio Showcase", {
                                .color = 0xFFF8FAFC,
                                .font_size = 22.0f,
                                .font_weight = FontWeight::Bold,
                            }),
                            container({
                                .color = 0xFF4338CA,
                                .border_radius = BorderRadius::circular(6.0f),
                                .padding = StyleInsets::symmetric(3.0f, 8.0f),
                                .child = text("ROADMAP v0.2.0 §11", {
                                    .color = 0xFFE0E7FF,
                                    .font_size = 11.0f,
                                    .font_weight = FontWeight::Bold,
                                }),
                            }),
                        },
                    }),
                    text("Demonstrating natural intrinsic width measurement, multi-child column synchronization, and step quantization.", {
                        .color = 0xFF94A3B8,
                        .font_size = 13.0f,
                    }),
                },
            }),
        });

        // ── 2. Interactive Control Bar ─────────────────────────────────
        auto preset_btn = [this](int id, std::string label) {
            bool active = (text_preset_ == id);
            return button(
                text(label, {
                    .color = active ? 0xFFFFFFFF : 0xFF94A3B8,
                    .font_size = 13.0f,
                    .font_weight = active ? FontWeight::Bold : FontWeight::Normal,
                }),
                [this, id]() {
                    setState([this, id]() { text_preset_ = id; });
                },
                {
                    .normal_color = active ? 0xFF6366F1 : 0xFF1E293B,
                    .hover_color  = active ? 0xFF4F46E5 : 0xFF334155,
                    .border_radius = 8.0f,
                    .padding = EdgeInsets::symmetric(8.0f, 14.0f),
                }
            );
        };

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
                    .normal_color = active ? 0xFF0EA5E9 : 0xFF1E293B,
                    .hover_color  = active ? 0xFF0284C7 : 0xFF334155,
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
                            text("Text Content Preset:", {
                                .color = 0xFFCBD5E1,
                                .font_size = 12.0f,
                                .font_weight = FontWeight::Bold,
                            }),
                            row({
                                .gap = StyleValue::point(8.0f),
                                .children = {
                                    preset_btn(0, "Short Labels"),
                                    preset_btn(1, "Medium Labels"),
                                    preset_btn(2, "Ultra-Wide Dynamic"),
                                },
                            }),
                        },
                    }),
                    column({
                        .gap = StyleValue::point(8.0f),
                        .children = {
                            text("Step Width Quantization:", {
                                .color = 0xFFCBD5E1,
                                .font_size = 12.0f,
                                .font_weight = FontWeight::Bold,
                            }),
                            row({
                                .gap = StyleValue::point(8.0f),
                                .children = {
                                    step_btn(0, "Natural (0px)"),
                                    step_btn(1, "Snap 20px"),
                                    step_btn(2, "Snap 50px"),
                                    step_btn(3, "Snap 100px"),
                                },
                            }),
                        },
                    }),
                },
            }),
        });

        // ── 3. Direct Side-by-Side Comparison ───────────────────────────
        // Left Column: Standard Column (Ragged / Independent Widths)
        std::vector<WidgetPtr> raw_buttons;
        for (const auto& lbl : labels) {
            raw_buttons.push_back(button(
                text(lbl, {.color = 0xFFFFFFFF, .font_size = 13.0f, .font_weight = FontWeight::Medium}),
                [this, lbl]() {
                    setState([this, lbl]() {
                        last_action_ = "Standard button clicked: [" + lbl + "]";
                    });
                },
                {
                    .normal_color = 0xFF334155,
                    .hover_color  = 0xFF475569,
                    .border_radius = 6.0f,
                    .padding = EdgeInsets::symmetric(10.0f, 16.0f),
                }
            ));
        }

        auto standard_column = column({
            .align_items = Align::Start,
            .gap = StyleValue::point(10.0f),
            .children = raw_buttons,
        });

        auto standard_card = container({
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
                            text("Standard Column (Ragged Widths)", {
                                .color = 0xFFF1F5F9,
                                .font_size = 15.0f,
                                .font_weight = FontWeight::Bold,
                            }),
                        },
                    }),
                    text("Each button calculates its own bounding width independently, resulting in an inconsistent and unbalanced UI hierarchy.", {
                        .color = 0xFF64748B,
                        .font_size = 12.0f,
                    }),
                    standard_column,
                },
            }),
        });

        // Right Column: Wrapped in IntrinsicWidth (Unified Synchronized Widths)
        std::vector<WidgetPtr> unified_buttons;
        uint32_t colors[] = {0xFF4338CA, 0xFF0284C7, 0xFF059669, 0xFFD97706};
        for (size_t i = 0; i < labels.size(); ++i) {
            const auto& lbl = labels[i];
            uint32_t c = colors[i % 4];
            unified_buttons.push_back(button(
                text(lbl, {.color = 0xFFFFFFFF, .font_size = 13.0f, .font_weight = FontWeight::Medium}),
                [this, lbl]() {
                    setState([this, lbl]() {
                        last_action_ = "IntrinsicWidth button clicked: [" + lbl + "]";
                    });
                },
                {
                    .normal_color = c,
                    .hover_color  = c + 0x00111111,
                    .border_radius = 6.0f,
                    .padding = EdgeInsets::symmetric(10.0f, 16.0f),
                }
            ));
        }

        auto intrinsic_inner_column = column({
            .align_items = Align::Stretch,
            .gap = StyleValue::point(10.0f),
            .children = unified_buttons,
        });

        auto intrinsic_wrapped = intrinsicWidth({
            .step_width = step,
            .child = intrinsic_inner_column,
        });

        auto intrinsic_card = container({
            .color = 0xFF0F172A,
            .border_radius = BorderRadius::circular(12.0f),
            .border = Border(0xFF6366F1, 1.5f),
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
                            text("IntrinsicWidth (Unified Widths)", {
                                .color = 0xFFF1F5F9,
                                .font_size = 15.0f,
                                .font_weight = FontWeight::Bold,
                            }),
                        },
                    }),
                    text("All children automatically synchronize to the widest element in the column without hardcoding any manual pixel widths.", {
                        .color = 0xFF64748B,
                        .font_size = 12.0f,
                    }),
                    intrinsic_wrapped,
                },
            }),
        });

        auto comparison_row = row({
            .justify_content = Justify::SpaceBetween,
            .align_items = Align::Start,
            .gap = StyleValue::point(24.0f),
            .children = {
                expanded({.flex = 1.0f, .child = standard_card}),
                expanded({.flex = 1.0f, .child = intrinsic_card}),
            },
        });

        // ── 4. Live Telemetry & Event Status Card ───────────────────────
        std::string step_str = step ? (std::to_string(static_cast<int>(*step)) + " px") : "None (Free)";
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
                                .color = 0xFF38BDF8,
                                .font_size = 13.0f,
                                .font_weight = FontWeight::Bold,
                            }),
                            text("Synchronized Items: " + std::to_string(labels.size()), {
                                .color = 0xFF34D399,
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

        // ── Main Page Layout ───────────────────────────────────────────
        auto main_column = column({
            .gap = StyleValue::point(20.0f),
            .width = StyleValue::percent(100.0f),
            .children = {
                header,
                controls_panel,
                comparison_row,
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

std::unique_ptr<State> IntrinsicWidthDemoApp::createState() {
    return std::make_unique<IntrinsicWidthDemoState>();
}

int main() {
    std::cout << "====================================================" << std::endl;
    std::cout << "  ENKI Engine — IntrinsicWidth Studio Showcase Demo" << std::endl;
    std::cout << "  Roadmap v0.2.0 | Section 11 Layout — Extended" << std::endl;
    std::cout << "====================================================" << std::endl;

    AppConfig config;
    config.title       = "ENKI Engine — IntrinsicWidth Showcase Demo";
    config.width       = 1080;
    config.height      = 780;
    config.target_fps  = 0;
    config.vsync       = false;
    config.show_performance_overlay = true;
    config.clear_color = 0xFF060911;

    return runApp(std::make_shared<IntrinsicWidthDemoApp>(), config);
}
