/// @file main.cpp
/// @brief Interactive Showcase Demo for ENKI Section 20: Utility / Behavioral.
///
/// Features:
///   1. Visibility — Visible vs Hidden vs Maintain Size
///   2. IgnorePointer — Toggle pointer event passing / click-through

#include "enki/core/types.hpp"
#include "enki/app/app.hpp"
#include "enki/widgets/utility.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/stack.hpp"
#include "enki/widgets/scroll_view.hpp"
#include "enki/state/state.hpp"

#include <iostream>
#include <memory>
#include <string>

using namespace enki;

class UtilityDemoApp : public StatefulWidget {
public:
    UtilityDemoApp() = default;

    std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "UtilityDemoApp"; }
};

class UtilityDemoState : public State {
    // Visibility State
    int  visibility_mode_ = 0; // 0 = Visible, 1 = Hidden (collapse), 2 = Hidden (maintain size)
    int  vis_click_count_ = 0;

    // IgnorePointer State
    bool is_ignoring_ = true;
    int  underlying_click_count_ = 0;
    int  target_button_click_count_ = 0;

public:
    WidgetPtr build(BuildContext&) override {
        // Header
        auto header = container({
            .color = 0xFF0F172A,
            .border_radius = BorderRadius::circular(14.0f),
            .border = Border(0xFF1E293B, 1.0f),
            .width = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(20.0f),
            .child = column({
                .gap = StyleValue::point(6.0f),
                .children = {
                    text("ENKI Section 20: Utility / Behavioral Widgets", {
                        .color = 0xFFF8FAFC,
                        .font_size = 20.0f,
                        .font_weight = FontWeight::Bold,
                    }),
                    text("Declarative C++20 Widgets: Visibility & IgnorePointer (High-Performance HitTesting & Layout Control)", {
                        .color = 0xFF94A3B8,
                        .font_size = 13.0f,
                    }),
                },
            }),
        });

        // ─────────────────────────────────────────────────────────────
        // Section 1: Visibility Widget
        // ─────────────────────────────────────────────────────────────
        auto vis_target_box = container({
            .color = 0xFF0284C7,
            .border_radius = BorderRadius::circular(10.0f),
            .border = Border(0xFF38BDF8, 1.5f),
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::point(70.0f),
            .padding = StyleInsets::all(12.0f),
            .child = row({
                .justify_content = Justify::SpaceBetween,
                .align_items = Align::Center,
                .children = {
                    column({
                        .gap = StyleValue::point(2.0f),
                        .children = {
                            text("🎯 Target Visibility Card", {
                                .color = 0xFFFFFFFF,
                                .font_size = 14.0f,
                                .font_weight = FontWeight::Bold,
                            }),
                            text("This element dynamically toggles visibility and maintains layout space", {
                                .color = 0xFFBAE6FD,
                                .font_size = 12.0f,
                            }),
                        },
                    }),
                    button(text("Click Target (" + std::to_string(vis_click_count_) + ")", {.color = 0xFFFFFFFF, .font_weight = FontWeight::Bold}), [this]() {
                        setState([this]() { vis_click_count_++; });
                    }, { .normal_color = 0xFF0369A1, .border_radius = 6.0f }),
                },
            }),
        });

        auto section1 = container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(12.0f),
            .border = Border(0xFF334155, 1.0f),
            .width = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(16.0f),
            .child = column({
                .gap = StyleValue::point(14.0f),
                .children = {
                    row({
                        .justify_content = Justify::SpaceBetween,
                        .align_items = Align::Center,
                        .children = {
                            row({
                                .align_items = Align::Center,
                                .gap = StyleValue::point(8.0f),
                                .children = {
                                    text("1. Visibility Widget", {
                                        .color = 0xFF38BDF8,
                                        .font_size = 15.0f,
                                        .font_weight = FontWeight::Bold,
                                    }),
                                    text(visibility_mode_ == 0 ? "[State: VISIBLE]" :
                                         visibility_mode_ == 1 ? "[State: HIDDEN (Collapsed)]" :
                                                                 "[State: HIDDEN (Maintains Size)]", {
                                        .color = visibility_mode_ == 0 ? 0xFF34D399 :
                                                 visibility_mode_ == 1 ? 0xFFF87171 : 0xFFFBBF24,
                                        .font_size = 12.0f,
                                        .font_weight = FontWeight::Medium,
                                    }),
                                },
                            }),
                            row({
                                .gap = StyleValue::point(6.0f),
                                .children = {
                                    button(text("Visible", {.color = 0xFFFFFFFF}), [this]() {
                                        setState([this]() { visibility_mode_ = 0; });
                                    }, { .normal_color = visibility_mode_ == 0 ? 0xFF059669 : 0xFF334155, .border_radius = 6.0f }),
                                    button(text("Hide (Collapse)", {.color = 0xFFFFFFFF}), [this]() {
                                        setState([this]() { visibility_mode_ = 1; });
                                    }, { .normal_color = visibility_mode_ == 1 ? 0xFFDC2626 : 0xFF334155, .border_radius = 6.0f }),
                                    button(text("Hide (Maintain Size)", {.color = 0xFFFFFFFF}), [this]() {
                                        setState([this]() { visibility_mode_ = 2; });
                                    }, { .normal_color = visibility_mode_ == 2 ? 0xFFD97706 : 0xFF334155, .border_radius = 6.0f }),
                                },
                            }),
                        },
                    }),
                    // Top Neighbor Card
                    container({
                        .color = 0xFF0F172A,
                        .border_radius = BorderRadius::circular(8.0f),
                        .border = Border(0xFF334155, 1.0f),
                        .width = StyleValue::percent(100.0f),
                        .padding = StyleInsets::all(10.0f),
                        .child = text("⬆️ Upper Neighbor Card — Stays above the target", {.color = 0xFF94A3B8, .font_size = 12.0f}),
                    }),
                    // Visibility Managed Target
                    visibility({
                        .child = vis_target_box,
                        .visible = (visibility_mode_ == 0),
                        .maintain_size = (visibility_mode_ == 2),
                    }),
                    // Bottom Neighbor Card
                    container({
                        .color = 0xFF0F172A,
                        .border_radius = BorderRadius::circular(8.0f),
                        .border = Border(0xFF334155, 1.0f),
                        .width = StyleValue::percent(100.0f),
                        .padding = StyleInsets::all(10.0f),
                        .child = text("⬇️ Lower Neighbor Card — Notice how it shifts up when collapsed, or stays in place when maintaining size", {
                            .color = 0xFF94A3B8,
                            .font_size = 12.0f,
                        }),
                    }),
                },
            }),
        });

        // ─────────────────────────────────────────────────────────────
        // Section 2: IgnorePointer Widget
        // ─────────────────────────────────────────────────────────────
        auto section2 = container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(12.0f),
            .border = Border(0xFF334155, 1.0f),
            .width = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(16.0f),
            .child = column({
                .gap = StyleValue::point(14.0f),
                .children = {
                    row({
                        .justify_content = Justify::SpaceBetween,
                        .align_items = Align::Center,
                        .children = {
                            row({
                                .align_items = Align::Center,
                                .gap = StyleValue::point(8.0f),
                                .children = {
                                    text("2. IgnorePointer Widget", {
                                        .color = 0xFFA78BFA,
                                        .font_size = 15.0f,
                                        .font_weight = FontWeight::Bold,
                                    }),
                                    text(is_ignoring_ ? "[Ignoring: TRUE — Pointer Events Pass Through]" :
                                                        "[Ignoring: FALSE — Normal Interaction]", {
                                        .color = is_ignoring_ ? 0xFFF87171 : 0xFF34D399,
                                        .font_size = 12.0f,
                                        .font_weight = FontWeight::Medium,
                                    }),
                                },
                            }),
                            button(text(is_ignoring_ ? "Enable Clicks (Ignoring: OFF)" : "Disable Clicks (Ignoring: ON)", {.color = 0xFFFFFFFF, .font_weight = FontWeight::Bold}), [this]() {
                                setState([this]() { is_ignoring_ = !is_ignoring_; });
                            }, { .normal_color = is_ignoring_ ? 0xFF059669 : 0xFFE11D48, .border_radius = 8.0f }),
                        },
                    }),
                    // Interactive Stack demonstrating Click-Through
                    stack({
                        // Layer 0: Underlying Background Click Surface
                        container({
                            .color = 0xFF0F172A,
                            .border_radius = BorderRadius::circular(10.0f),
                            .border = Border(0xFF475569, 1.5f),
                            .width = StyleValue::percent(100.0f),
                            .height = StyleValue::point(100.0f),
                            .padding = StyleInsets::all(16.0f),
                            .child = row({
                                .justify_content = Justify::SpaceBetween,
                                .align_items = Align::Center,
                                .children = {
                                    column({
                                        .gap = StyleValue::point(4.0f),
                                        .children = {
                                            text("Underlying Surface Layer (Clickable)", {
                                                .color = 0xFFE2E8F0,
                                                .font_size = 14.0f,
                                                .font_weight = FontWeight::Bold,
                                            }),
                                            text("When Ignoring = TRUE, clicking above passes through directly to this button!", {
                                                .color = 0xFF94A3B8,
                                                .font_size = 12.0f,
                                            }),
                                        },
                                    }),
                                    button(text("Underlying Button (" + std::to_string(underlying_click_count_) + ")", {.color = 0xFFFFFFFF, .font_weight = FontWeight::Bold}), [this]() {
                                        setState([this]() { underlying_click_count_++; });
                                    }, { .normal_color = 0xFF475569, .border_radius = 6.0f }),
                                },
                            }),
                        }),
                        // Layer 1: Foreground Layer wrapped in IgnorePointer
                        ignorePointer({
                            .child = container({
                                .color = is_ignoring_ ? 0x991E1B4B : 0xDD312E81,
                                .border_radius = BorderRadius::circular(10.0f),
                                .border = Border(is_ignoring_ ? 0xFFA78BFA : 0xFFC084FC, 1.5f),
                                .width = StyleValue::percent(100.0f),
                                .height = StyleValue::point(100.0f),
                                .padding = StyleInsets::all(16.0f),
                                .child = row({
                                    .justify_content = Justify::SpaceBetween,
                                    .align_items = Align::Center,
                                    .children = {
                                        column({
                                            .gap = StyleValue::point(4.0f),
                                            .children = {
                                                text(is_ignoring_ ? "🚫 Semi-Transparent Overlay (Ignoring = TRUE)" :
                                                                    "✨ Interactive Overlay (Ignoring = FALSE)", {
                                                    .color = 0xFFF5F3FF,
                                                    .font_size = 14.0f,
                                                    .font_weight = FontWeight::Bold,
                                                }),
                                                text(is_ignoring_ ? "Pointer events pass right through this overlay to the surface below" :
                                                                    "This overlay now intercepts and handles all pointer events directly", {
                                                    .color = 0xFFDDD6FE,
                                                    .font_size = 12.0f,
                                                }),
                                            },
                                        }),
                                        button(text("Overlay Button (" + std::to_string(target_button_click_count_) + ")", {.color = 0xFFFFFFFF, .font_weight = FontWeight::Bold}), [this]() {
                                            setState([this]() { target_button_click_count_++; });
                                        }, { .normal_color = is_ignoring_ ? 0xFF6D28D9 : 0xFF7C3AED, .border_radius = 6.0f }),
                                    },
                                }),
                            }),
                            .ignoring = is_ignoring_,
                        }),
                    }),
                },
            }),
        });

        // Main Layout
        auto main_col = column({
            .align_items = Align::Center,
            .gap = StyleValue::point(16.0f),
            .width = StyleValue::percent(100.0f),
            .children = {
                header,
                section1,
                section2,
            },
        });

        return container({
            .color = 0xFF0B0F17,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(24.0f),
            .child = scrollView(main_col),
        });
    }
};

std::unique_ptr<State> UtilityDemoApp::createState() {
    return std::make_unique<UtilityDemoState>();
}

int main() {
    AppConfig config;
    config.title       = "ENKI Engine — Utility & Behavioral Suite (Section 20)";
    config.width       = 960;
    config.height      = 760;
    config.target_fps  = 0;
    config.vsync       = false;
    config.show_performance_overlay = false;
    config.clear_color = 0xFF0B0F17;
    return runApp(std::make_shared<UtilityDemoApp>(), config);
}
