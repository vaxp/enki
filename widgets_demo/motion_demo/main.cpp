/// @file main.cpp
/// @brief ENKI Section 13 Animation & Motion Showcase Demo App.
/// Interactive visual demonstration of all 7 animation & motion widgets:
///   1. AnimatedOpacity
///   2. AnimatedContainer
///   3. AnimatedScale
///   4. AnimatedRotation
///   5. AnimatedSlide
///   6. AnimatedSwitcher
///   7. SlideTransition

#include "enki/app/app.hpp"
#include "enki/widgets/motion.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/stack.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/scroll_view.hpp"
#include "enki/state/state.hpp"
#include "enki/animation/ticker.hpp"
#include "enki/animation/animation_controller.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <memory>

using namespace enki;

// ════════════════════════════════════════════════════════════════
// Interactive Motion Demo App State
// ════════════════════════════════════════════════════════════════

class MotionDemoState : public State {
    // 1. AnimatedOpacity
    bool is_visible_ = true;

    // 2. AnimatedContainer
    bool is_expanded_ = false;

    // 3. AnimatedScale
    bool is_scaled_up_ = false;

    // 4. AnimatedRotation
    float rotation_turns_ = 0.0f;

    // 5. AnimatedSlide
    bool is_slid_in_ = true;

    // 6. AnimatedSwitcher
    int switcher_index_ = 0;

    // 7. SlideTransition / Toast Slide
    bool is_toast_visible_ = true;

public:
    WidgetPtr build(BuildContext&) override {
        // Header
        auto header = column({
            .align_items = Align::Center,
            .gap = StyleValue::point(6.0f),
            .children = {
                text("ENKI Motion & Animation Suite (Section 13)", {
                    .color = 0xFF38BDF8,
                    .font_size = 22.0f,
                    .font_weight = FontWeight::Bold,
                }),
                text("Interactive showcase of 7 high-performance C++20 animation primitives", {
                    .color = 0xFF94A3B8,
                    .font_size = 14.0f,
                }),
            },
        });

        // ─────────────────────────────────────────────────────────────
        // Section 1: AnimatedOpacity
        // ─────────────────────────────────────────────────────────────
        auto opacity_demo = container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(12.0f),
            .border = Border(0xFF334155, 1.0f),
            .width = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(16.0f),
            .child = column({
                .gap = StyleValue::point(12.0f),
                .children = {
                    row({
                        .justify_content = Justify::SpaceBetween,
                        .align_items = Align::Center,
                        .children = {
                            text("1. AnimatedOpacity (Smooth alpha fading)", {
                                .color = 0xFFF43F5E,
                                .font_size = 16.0f,
                                .font_weight = FontWeight::Bold,
                            }),
                            button(text(is_visible_ ? "Fade Out" : "Fade In", {.color = 0xFFFFFFFF}), [this]() {
                                setState([this]() { is_visible_ = !is_visible_; });
                            }, { .normal_color = 0xFFF43F5E, .border_radius = 8.0f }),
                        },
                    }),
                    animatedOpacity({
                        .opacity = is_visible_ ? 1.0f : 0.1f,
                        .duration = std::chrono::milliseconds(400),
                        .curve = &Curves::easeInOut,
                        .child = container({
                            .color = 0xFF881337,
                            .border_radius = BorderRadius::circular(10.0f),
                            .border = Border(0xFFFB7185, 1.5f),
                            .align = Alignment::Center,
                            .padding = StyleInsets::all(14.0f),
                            .child = text("Dynamic Opacity Content Box (saveLayerAlpha GPU rendered)", {
                                .color = 0xFFFEE2E2,
                                .font_size = 13.0f,
                                .font_weight = FontWeight::Medium,
                            }),
                        }),
                    }),
                },
            }),
        });

        // ─────────────────────────────────────────────────────────────
        // Section 2: AnimatedContainer
        // ─────────────────────────────────────────────────────────────
        auto container_demo = container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(12.0f),
            .border = Border(0xFF334155, 1.0f),
            .width = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(16.0f),
            .child = column({
                .gap = StyleValue::point(12.0f),
                .children = {
                    row({
                        .justify_content = Justify::SpaceBetween,
                        .align_items = Align::Center,
                        .children = {
                            text("2. AnimatedContainer (Morph size, color, radius, insets)", {
                                .color = 0xFF3B82F6,
                                .font_size = 16.0f,
                                .font_weight = FontWeight::Bold,
                            }),
                            button(text(is_expanded_ ? "Shrink" : "Expand & Morph", {.color = 0xFFFFFFFF}), [this]() {
                                setState([this]() { is_expanded_ = !is_expanded_; });
                            }, { .normal_color = 0xFF2563EB, .border_radius = 8.0f }),
                        },
                    }),
                    animatedContainer({
                        .color = is_expanded_ ? 0xFF059669 : 0xFF1D4ED8,
                        .border_radius = is_expanded_ ? BorderRadius::circular(24.0f) : BorderRadius::circular(6.0f),
                        .border = is_expanded_ ? Border(0xFF34D399, 2.0f) : Border(0xFF60A5FA, 1.0f),
                        .align = Alignment::Center,
                        .width = is_expanded_ ? StyleValue::percent(100.0f) : StyleValue::point(260.0f),
                        .height = is_expanded_ ? StyleValue::point(90.0f) : StyleValue::point(55.0f),
                        .padding = is_expanded_ ? StyleInsets::all(18.0f) : StyleInsets::all(10.0f),
                        .duration = std::chrono::milliseconds(500),
                        .curve = &Curves::fastOutSlowIn,
                        .child = text(is_expanded_ ? "Morphed to 100% width, Emerald Color, 24px radius!" : "Initial Blue Box State", {
                            .color = 0xFFFFFFFF,
                            .font_size = 13.0f,
                            .font_weight = FontWeight::Bold,
                        }),
                    }),
                },
            }),
        });

        // ─────────────────────────────────────────────────────────────
        // Section 3: AnimatedScale & AnimatedRotation
        // ─────────────────────────────────────────────────────────────
        auto scale_rotation_row = row({
            .justify_content = Justify::SpaceBetween,
            .gap = StyleValue::point(16.0f),
            .width = StyleValue::percent(100.0f),
            .children = {
                // Scale card
                container({
                    .color = 0xFF1E293B,
                    .border_radius = BorderRadius::circular(12.0f),
                    .border = Border(0xFF334155, 1.0f),
                    .padding = StyleInsets::all(16.0f),
                    .flex = 1.0f,
                    .child = column({
                        .align_items = Align::Center,
                        .gap = StyleValue::point(12.0f),
                        .children = {
                            row({
                                .justify_content = Justify::SpaceBetween,
                                .align_items = Align::Center,
                                .width = StyleValue::percent(100.0f),
                                .children = {
                                    text("3. AnimatedScale", {
                                        .color = 0xFFA855F7,
                                        .font_size = 15.0f,
                                        .font_weight = FontWeight::Bold,
                                    }),
                                    button(text(is_scaled_up_ ? "Normal" : "Scale x1.35", {.color = 0xFFFFFFFF}), [this]() {
                                        setState([this]() { is_scaled_up_ = !is_scaled_up_; });
                                    }, { .normal_color = 0xFF9333EA, .border_radius = 6.0f }),
                                },
                            }),
                            container({
                                .align = Alignment::Center,
                                .height = StyleValue::point(80.0f),
                                .child = animatedScale({
                                    .scale = is_scaled_up_ ? 1.35f : 1.0f,
                                    .alignment = Alignment::Center,
                                    .duration = std::chrono::milliseconds(350),
                                    .curve = &Curves::elasticOut,
                                    .child = container({
                                        .color = 0xFF581C87,
                                        .border_radius = BorderRadius::circular(12.0f),
                                        .border = Border(0xFFC084FC, 1.5f),
                                        .padding = StyleInsets::all(12.0f),
                                        .child = text("Elastic Scale Box", { .color = 0xFFF3E8FF, .font_size = 12.0f }),
                                    }),
                                }),
                            }),
                        },
                    }),
                }),

                // Rotation card
                container({
                    .color = 0xFF1E293B,
                    .border_radius = BorderRadius::circular(12.0f),
                    .border = Border(0xFF334155, 1.0f),
                    .padding = StyleInsets::all(16.0f),
                    .flex = 1.0f,
                    .child = column({
                        .align_items = Align::Center,
                        .gap = StyleValue::point(12.0f),
                        .children = {
                            row({
                                .justify_content = Justify::SpaceBetween,
                                .align_items = Align::Center,
                                .width = StyleValue::percent(100.0f),
                                .children = {
                                    text("4. AnimatedRotation", {
                                        .color = 0xFFF59E0B,
                                        .font_size = 15.0f,
                                        .font_weight = FontWeight::Bold,
                                    }),
                                    button(text("Rotate +90°", {.color = 0xFFFFFFFF}), [this]() {
                                        setState([this]() { rotation_turns_ += 0.25f; });
                                    }, { .normal_color = 0xFFD97706, .border_radius = 6.0f }),
                                },
                            }),
                            container({
                                .align = Alignment::Center,
                                .height = StyleValue::point(80.0f),
                                .child = animatedRotation({
                                    .turns = rotation_turns_,
                                    .alignment = Alignment::Center,
                                    .duration = std::chrono::milliseconds(450),
                                    .curve = &Curves::bounceOut,
                                    .child = container({
                                        .color = 0xFF78350F,
                                        .border_radius = BorderRadius::circular(12.0f),
                                        .border = Border(0xFFFCD34D, 1.5f),
                                        .padding = StyleInsets::all(12.0f),
                                        .child = text("Bounce Rotate", { .color = 0xFFFEF3C7, .font_size = 12.0f }),
                                    }),
                                }),
                            }),
                        },
                    }),
                }),
            },
        });

        // ─────────────────────────────────────────────────────────────
        // Section 4: AnimatedSlide
        // ─────────────────────────────────────────────────────────────
        auto slide_demo = container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(12.0f),
            .border = Border(0xFF334155, 1.0f),
            .width = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(16.0f),
            .child = column({
                .gap = StyleValue::point(12.0f),
                .children = {
                    row({
                        .justify_content = Justify::SpaceBetween,
                        .align_items = Align::Center,
                        .children = {
                            text("5. AnimatedSlide (Fractional translation relative to bounds)", {
                                .color = 0xFF10B981,
                                .font_size = 16.0f,
                                .font_weight = FontWeight::Bold,
                            }),
                            button(text(is_slid_in_ ? "Slide Offset Right" : "Slide Back to 0", {.color = 0xFFFFFFFF}), [this]() {
                                setState([this]() { is_slid_in_ = !is_slid_in_; });
                            }, { .normal_color = 0xFF059669, .border_radius = 8.0f }),
                        },
                    }),
                    animatedSlide({
                        .offset = is_slid_in_ ? Point{0.0f, 0.0f} : Point{0.35f, 0.0f},
                        .duration = std::chrono::milliseconds(400),
                        .curve = &Curves::easeOut,
                        .child = container({
                            .color = 0xFF064E3B,
                            .border_radius = BorderRadius::circular(8.0f),
                            .border = Border(0xFF34D399, 1.5f),
                            .padding = StyleInsets::all(12.0f),
                            .child = text("Sliding Content Banner (Relative DX translation)", {
                                .color = 0xFFD1FAE5,
                                .font_size = 13.0f,
                            }),
                        }),
                    }),
                },
            }),
        });

        // ─────────────────────────────────────────────────────────────
        // Section 5: AnimatedSwitcher
        // ─────────────────────────────────────────────────────────────
        WidgetPtr switcher_child = nullptr;
        if (switcher_index_ % 3 == 0) {
            switcher_child = container({
                .color = 0xFF1E1B4B,
                .border_radius = BorderRadius::circular(10.0f),
                .border = Border(0xFF818CF8, 1.5f),
                .align = Alignment::Center,
                .width = StyleValue::percent(100.0f),
                .padding = StyleInsets::all(14.0f),
                .child = text("Indigo Card — Theme Variant A", { .color = 0xFFE0E7FF, .font_weight = FontWeight::Bold }),
                .key = Key::string("card_indigo"),
            });
        } else if (switcher_index_ % 3 == 1) {
            switcher_child = container({
                .color = 0xFF064E3B,
                .border_radius = BorderRadius::circular(10.0f),
                .border = Border(0xFF34D399, 1.5f),
                .align = Alignment::Center,
                .width = StyleValue::percent(100.0f),
                .padding = StyleInsets::all(14.0f),
                .child = text("Emerald Card — Theme Variant B", { .color = 0xFFD1FAE5, .font_weight = FontWeight::Bold }),
                .key = Key::string("card_emerald"),
            });
        } else {
            switcher_child = container({
                .color = 0xFF701A75,
                .border_radius = BorderRadius::circular(10.0f),
                .border = Border(0xFFF472B6, 1.5f),
                .align = Alignment::Center,
                .width = StyleValue::percent(100.0f),
                .padding = StyleInsets::all(14.0f),
                .child = text("Fuchsia Card — Theme Variant C", { .color = 0xFFFCE7F3, .font_weight = FontWeight::Bold }),
                .key = Key::string("card_fuchsia"),
            });
        }

        auto switcher_demo = container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(12.0f),
            .border = Border(0xFF334155, 1.0f),
            .width = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(16.0f),
            .child = column({
                .gap = StyleValue::point(12.0f),
                .children = {
                    row({
                        .justify_content = Justify::SpaceBetween,
                        .align_items = Align::Center,
                        .children = {
                            text("6. AnimatedSwitcher (Cross-fade between dynamic widgets)", {
                                .color = 0xFF6366F1,
                                .font_size = 16.0f,
                                .font_weight = FontWeight::Bold,
                            }),
                            button(text("Switch Next Child", {.color = 0xFFFFFFFF}), [this]() {
                                setState([this]() { switcher_index_++; });
                            }, { .normal_color = 0xFF4F46E5, .border_radius = 8.0f }),
                        },
                    }),
                    animatedSwitcher({
                        .child = switcher_child,
                        .duration = std::chrono::milliseconds(350),
                    }),
                },
            }),
        });

        // ─────────────────────────────────────────────────────────────
        // Section 6: SlideTransition (Clipped Entrance / Exit Drawer)
        // ─────────────────────────────────────────────────────────────

        auto transition_demo = container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(12.0f),
            .border = Border(0xFF334155, 1.0f),
            .width = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(16.0f),
            .child = column({
                .gap = StyleValue::point(12.0f),
                .children = {
                    row({
                        .justify_content = Justify::SpaceBetween,
                        .align_items = Align::Center,
                        .children = {
                            row({
                                .align_items = Align::Center,
                                .gap = StyleValue::point(8.0f),
                                .children = {
                                    text("7. Clipped Toast / Banner Slide", {
                                        .color = 0xFF06B6D4,
                                        .font_size = 16.0f,
                                        .font_weight = FontWeight::Bold,
                                    }),
                                    text(is_toast_visible_ ? "[State: VISIBLE]" : "[State: HIDDEN]", {
                                        .color = is_toast_visible_ ? 0xFF34D399 : 0xFF94A3B8,
                                        .font_size = 12.0f,
                                        .font_weight = FontWeight::Medium,
                                    }),
                                },
                            }),
                            button(text(is_toast_visible_ ? "Slide Out (Hide)" : "Slide In (Show)", {.color = 0xFFFFFFFF, .font_weight = FontWeight::Bold}), [this]() {
                                setState([this]() { is_toast_visible_ = !is_toast_visible_; });
                            }, { .normal_color = is_toast_visible_ ? 0xFFBE123C : 0xFF059669, .border_radius = 8.0f }),
                        },
                    }),
                    // Clipped Viewport Box
                    container({
                        .color = 0xFF0F172A,
                        .border_radius = BorderRadius::circular(10.0f),
                        .border = Border(0xFF334155, 1.0f),
                        .clip_content = true,
                        .width = StyleValue::percent(100.0f),
                        .height = StyleValue::point(52.0f),
                        .child = animatedSlide({
                            .offset = is_toast_visible_ ? Point{0.0f, 0.0f} : Point{0.0f, -1.5f},
                            .duration = std::chrono::milliseconds(350),
                            .curve = &Curves::easeInOut,
                            .child = container({
                                .color = 0xFF0E7490,
                                .border_radius = BorderRadius::circular(8.0f),
                                .border = Border(0xFF22D3EE, 1.5f),
                                .align = Alignment::Center,
                                .width = StyleValue::percent(100.0f),
                                .height = StyleValue::percent(100.0f),
                                .padding = StyleInsets::symmetric(StyleValue::point(10.0f), StyleValue::point(14.0f)),
                                .child = text("🚀 Clipped Notification Banner — Enters & Exits 100% offscreen", {
                                    .color = 0xFFCFFAFE,
                                    .font_size = 13.0f,
                                    .font_weight = FontWeight::Bold,
                                }),
                            }),
                        }),
                    }),
                },
            }),
        });

        // Main layout
        auto main_col = column({
            .align_items = Align::Center,
            .gap = StyleValue::point(16.0f),
            .width = StyleValue::percent(100.0f),
            .children = {
                header,
                opacity_demo,
                container_demo,
                scale_rotation_row,
                slide_demo,
                switcher_demo,
                transition_demo,
            },
        });

        return container({
            .color = 0xFF0B1120,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(20.0f),
            .child = scrollView(main_col),
        });
    }
};

class MotionDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override { return std::make_unique<MotionDemoState>(); }
    [[nodiscard]] std::string_view typeName() const override { return "MotionDemoApp"; }
};

int main() {
    AppConfig config;
    config.title       = "ENKI Engine — Motion & Animation Suite (Section 13)";
    config.width       = 960;
    config.height      = 840;
    config.target_fps  = 0;
    config.vsync       = false;
    config.show_performance_overlay = true;
    config.clear_color = 0xFF0B1120;
    return runApp(std::make_shared<MotionDemoApp>(), config);
}
