/// @file main.cpp
/// @brief ENKI Stack & Positioned Interactive Showcase Demo
///
/// Features demonstrated:
///   1. Notification badges on Avatars & Action icons with absolute negative/positive offsets.
///   2. Hero Card with Layered Gradients, Floating Status Chips, and absolute text overlays.
///   3. Staggered overlapping cards with Z-index hit-testing (clicking layers).
///   4. Interactive Fullscreen Modal Dialog overlay created via Positioned::fill().
///   5. Dynamic coordinate controls and interactive toggles.

#include "enki/app/app.hpp"
#include "enki/state/state.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/stack.hpp"
#include "enki/widgets/button.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <sstream>

using namespace enki;

// ════════════════════════════════════════════════════════════════
// Design System & Palette
// ════════════════════════════════════════════════════════════════

namespace Style {
    constexpr Color bg_dark       = 0xFF0B0F19;
    constexpr Color bg_card       = 0xFF161D2F;
    constexpr Color bg_card_light = 0xFF1F293D;
    constexpr Color border_subtle = 0x3038BDF8;
    constexpr Color border_bright = 0x8038BDF8;
    constexpr Color primary       = 0xFF6366F1;
    constexpr Color primary_light = 0xFF818CF8;
    constexpr Color cyan_neon     = 0xFF00E5FF;
    constexpr Color emerald       = 0xFF10B981;
    constexpr Color rose          = 0xFFF43F5E;
    constexpr Color amber         = 0xFFF59E0B;
    constexpr Color text_white    = 0xFFFFFFFF;
    constexpr Color text_muted    = 0xFF94A3B8;
}

// ════════════════════════════════════════════════════════════════
// Stack Demo State & App
// ════════════════════════════════════════════════════════════════

class StackDemoApp : public StatefulWidget {
public:
    std::string_view typeName() const override { return "StackDemoApp"; }
    std::unique_ptr<State> createState() override;
};

class StackDemoState : public State {
public:
    int badge_count = 3;
    bool is_online = true;
    int active_layer_index = 2; // 0, 1, or 2
    bool show_modal = false;
    float badge_offset_x = -4.0f;
    float badge_offset_y = -4.0f;

    WidgetPtr build(BuildContext&) override {
        // Main content tree
        auto content = container({
            .color = Style::bg_dark,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(24.0f),
            .child = column({
                .justify_content = Justify::Center,
                .align_items = Align::Center,
                .gap = StyleValue::point(18.0f),
                .children = {
                    buildHeader(),
                    buildCardsGrid(),
                    buildControlsRow(),
                }
            })
        });

        // Root stack allows overlaying modal on top of entire window
        std::vector<WidgetPtr> root_children;
        root_children.push_back(Positioned::fill(content));

        if (show_modal) {
            root_children.push_back(buildModalOverlay());
        }

        return Stack {
            .fit = StackFit::Expand,
            .children = std::move(root_children),
        };
    }

private:
    WidgetPtr buildHeader() {
        return column({
            .align_items = Align::Center,
            .gap = StyleValue::point(4.0f),
            .children = {
                text("ENKI — Stack & Positioned Layout Architecture", {
                    .color = Style::text_white,
                    .font_size = 22.0f,
                    .font_weight = FontWeight::Bold,
                }),
                text("Multi-layered 2.5D visual hierarchy with absolute offsets, alignment & reverse hit-testing", {
                    .color = Style::cyan_neon,
                    .font_size = 12.5f,
                }),
            }
        });
    }

    WidgetPtr buildCardsGrid() {
        auto card1 = buildAvatarBadgeCard();
        auto card2 = buildHeroGradientCard();
        auto card3 = buildLayeredStackCard();

        return row({
            .justify_content = Justify::Center,
            .align_items = Align::Start,
            .gap = StyleValue::point(20.0f),
            .children = {
                card1,
                card2,
                card3,
            }
        });
    }

    // ── Card 1: Avatar with Positioned Online & Counter Badges ──

    WidgetPtr buildAvatarBadgeCard() {
        auto avatar_icon = text("👤", { .font_size = 32.0f });

        auto avatar = container({
            .color = 0xFF312E81,
            .border_radius = BorderRadius::circular(36.0f),
            .border = Border(Style::primary_light, 2.0f),
            .align = Alignment::Center,
            .width = StyleValue::point(72.0f),
            .height = StyleValue::point(72.0f),
            .child = avatar_icon,
        });

        // Online dot positioned bottom-right
        auto online_dot = container({
            .color = is_online ? Style::emerald : Style::amber,
            .border_radius = BorderRadius::circular(8.0f),
            .border = Border(Style::bg_card, 2.5f),
            .width = StyleValue::point(16.0f),
            .height = StyleValue::point(16.0f),
        });

        auto pos_online = Positioned {
            .child = online_dot,
            .right = StyleValue::point(2.0f),
            .bottom = StyleValue::point(2.0f),
        };

        // Count badge positioned top-right
        auto count_text = text(std::to_string(badge_count), {
            .color = Style::text_white,
            .font_size = 10.0f,
            .font_weight = FontWeight::Bold,
        });

        auto count_pill = container({
            .color = Style::rose,
            .border_radius = BorderRadius::circular(10.0f),
            .border = Border(Style::bg_card, 2.0f),
            .padding = StyleInsets::symmetric(2.0f, 6.0f),
            .child = count_text,
        });

        auto pos_count = Positioned {
            .child = count_pill,
            .top = StyleValue::point(badge_offset_y),
            .right = StyleValue::point(badge_offset_x),
        };

        // The Stack wrapping the avatar + badges
        auto avatar_stack = Stack {
            .clip_behavior = Clip::None,
            .width = StyleValue::point(76.0f),
            .height = StyleValue::point(76.0f),
            .children = {
                avatar,
                pos_online,
                pos_count,
            }
        };

        // Action button to increment badge
        auto btn_add = Button {
            .child = text("+ Badge Count", {
                .color = Style::text_white,
                .font_size = 11.5f,
                .font_weight = FontWeight::Bold,
            }),
            .on_pressed = [this]() {
                setState([this]() {
                    badge_count = (badge_count % 99) + 1;
                });
            },
            .normal_color = Style::primary,
            .border_radius = 6.0f,
            .padding = EdgeInsets::symmetric(7.0f, 12.0f),
        };

        // Toggle online status
        auto btn_toggle = Button {
            .child = text(is_online ? "Set Away" : "Set Online", {
                .color = Style::cyan_neon,
                .font_size = 11.5f,
            }),
            .on_pressed = [this]() {
                setState([this]() {
                    is_online = !is_online;
                });
            },
            .normal_color = Style::bg_card_light,
            .border_radius = 6.0f,
            .padding = EdgeInsets::symmetric(6.0f, 10.0f),
        };

        return container({
            .color = Style::bg_card,
            .border_radius = BorderRadius::circular(12.0f),
            .border = Border(Style::border_subtle, 1.0f),
            .width = StyleValue::point(310.0f),
            .height = StyleValue::point(280.0f),
            .padding = StyleInsets::all(16.0f),
            .child = column({
                .align_items = Align::Center,
                .gap = StyleValue::point(12.0f),
                .children = {
                    text("1. Floating Badges", {
                        .color = Style::text_white,
                        .font_size = 14.0f,
                        .font_weight = FontWeight::Bold,
                    }),
                    text("Badges positioned at absolute coordinates with zero parent flow disruption.", {
                        .color = Style::text_muted,
                        .font_size = 11.0f,
                    }),
                    container({
                        .align = Alignment::Center,
                        .child = avatar_stack,
                    }),
                    row({
                        .justify_content = Justify::Center,
                        .align_items = Align::Center,
                        .gap = StyleValue::point(8.0f),
                        .children = {
                            btn_add,
                            btn_toggle,
                        }
                    }),
                }
            })
        });
    }

    // ── Card 2: Hero Feature Card with Layered Gradient & Chips ──

    WidgetPtr buildHeroGradientCard() {
        // Base background gradient layer
        auto base_bg = container({
            .gradient = GradientConfig::linear({0xFF1E1B4B, 0xFF4338CA, 0xFF0F172A}),
            .border_radius = BorderRadius::circular(8.0f),
        });

        // PRO status chip positioned top-right
        auto pro_chip = container({
            .color = Style::cyan_neon,
            .border_radius = BorderRadius::circular(4.0f),
            .padding = StyleInsets::symmetric(3.0f, 8.0f),
            .child = text("PRO FEATURE", {
                .color = Style::bg_dark,
                .font_size = 9.5f,
                .font_weight = FontWeight::Bold,
            }),
        });

        auto pos_pro = Positioned {
            .child = pro_chip,
            .top = StyleValue::point(10.0f),
            .right = StyleValue::point(10.0f),
        };

        // Rating pill positioned top-left
        auto star_pill = container({
            .color = 0xCC000000,
            .border_radius = BorderRadius::circular(4.0f),
            .padding = StyleInsets::symmetric(3.0f, 8.0f),
            .child = text("★ 4.98", {
                .color = Style::amber,
                .font_size = 10.0f,
                .font_weight = FontWeight::Bold,
            }),
        });

        auto pos_star = Positioned {
            .child = star_pill,
            .top = StyleValue::point(10.0f),
            .left = StyleValue::point(10.0f),
        };

        // Bottom text overlay positioned bottom-left
        auto bottom_box = container({
            .gradient = GradientConfig::linear({0x00000000, 0xE60B0F19}),
            .padding = StyleInsets::all(10.0f),
            .child = column({
                .children = {
                    text("Hyper-Vector Pipeline", {
                        .color = Style::text_white,
                        .font_size = 13.0f,
                        .font_weight = FontWeight::Bold,
                    }),
                    text("0.3ms frame latency", {
                        .color = Style::emerald,
                        .font_size = 10.5f,
                    }),
                }
            })
        });

        auto pos_bottom = Positioned {
            .child = bottom_box,
            .right = StyleValue::point(0.0f),
            .bottom = StyleValue::point(0.0f),
            .left = StyleValue::point(0.0f),
        };

        // Hero Stack
        auto hero_stack = Stack {
            .clip_behavior = Clip::HardEdge,
            .width = StyleValue::point(278.0f),
            .height = StyleValue::point(160.0f),
            .children = {
                Positioned::fill(base_bg),
                pos_pro,
                pos_star,
                pos_bottom,
            }
        };

        return container({
            .color = Style::bg_card,
            .border_radius = BorderRadius::circular(12.0f),
            .border = Border(Style::border_subtle, 1.0f),
            .width = StyleValue::point(310.0f),
            .height = StyleValue::point(280.0f),
            .padding = StyleInsets::all(16.0f),
            .child = column({
                .align_items = Align::Center,
                .gap = StyleValue::point(8.0f),
                .children = {
                    text("2. Layered Hero Card", {
                        .color = Style::text_white,
                        .font_size = 14.0f,
                        .font_weight = FontWeight::Bold,
                    }),
                    text("Multi-layer composition: Base image + dark gradient overlay + status tags.", {
                        .color = Style::text_muted,
                        .font_size = 11.0f,
                    }),
                    hero_stack,
                }
            })
        });
    }

    // ── Card 3: Overlapping Z-Index Hit-Testing Layers ──

    WidgetPtr buildLayeredStackCard() {
        // Layer 0 (Bottom)
        auto l0_box = container({
            .color = active_layer_index == 0 ? Style::primary : Style::bg_card_light,
            .border_radius = BorderRadius::circular(8.0f),
            .border = Border(Style::border_subtle, 1.0f),
            .padding = StyleInsets::all(8.0f),
            .child = text("Layer 0 (Base)", {
                .color = Style::text_muted,
                .font_size = 10.5f,
                .font_weight = FontWeight::Bold,
            })
        });
        auto l0_gesture = gestureDetector({
            .key = Key::string("layer_0"),
            .child = l0_box,
            .cursor_type = SystemCursor::Pointer,
            .on_tap_up = [this](const TapUpDetails&) {
                setState([this]() { active_layer_index = 0; });
            },
        });
        auto pos_l0 = Positioned {
            .child = l0_gesture,
            .top = StyleValue::point(0.0f),
            .left = StyleValue::point(0.0f),
            .width = StyleValue::point(160.0f),
            .height = StyleValue::point(70.0f),
        };

        // Layer 1 (Middle)
        auto l1_box = container({
            .color = active_layer_index == 1 ? Style::primary : 0xFF2D3748,
            .border_radius = BorderRadius::circular(8.0f),
            .border = Border(Style::cyan_neon, active_layer_index == 1 ? 1.5f : 0.5f),
            .padding = StyleInsets::all(8.0f),
            .child = text("Layer 1 (Mid)", {
                .color = Style::text_white,
                .font_size = 10.5f,
                .font_weight = FontWeight::Bold,
            })
        });
        auto l1_gesture = gestureDetector({
            .key = Key::string("layer_1"),
            .child = l1_box,
            .cursor_type = SystemCursor::Pointer,
            .on_tap_up = [this](const TapUpDetails&) {
                setState([this]() { active_layer_index = 1; });
            },
        });
        auto pos_l1 = Positioned {
            .child = l1_gesture,
            .top = StyleValue::point(30.0f),
            .left = StyleValue::point(40.0f),
            .width = StyleValue::point(160.0f),
            .height = StyleValue::point(70.0f),
        };

        // Layer 2 (Top)
        auto l2_box = container({
            .color = active_layer_index == 2 ? Style::primary : 0xFF4A5568,
            .border_radius = BorderRadius::circular(8.0f),
            .border = Border(Style::cyan_neon, active_layer_index == 2 ? 1.5f : 0.5f),
            .padding = StyleInsets::all(8.0f),
            .child = text("Layer 2 (Front)", {
                .color = Style::text_white,
                .font_size = 10.5f,
                .font_weight = FontWeight::Bold,
            })
        });
        auto l2_gesture = gestureDetector({
            .key = Key::string("layer_2"),
            .child = l2_box,
            .cursor_type = SystemCursor::Pointer,
            .on_tap_up = [this](const TapUpDetails&) {
                setState([this]() { active_layer_index = 2; });
            },
        });
        auto pos_l2 = Positioned {
            .child = l2_gesture,
            .top = StyleValue::point(60.0f),
            .left = StyleValue::point(80.0f),
            .width = StyleValue::point(160.0f),
            .height = StyleValue::point(70.0f),
        };

        auto overlap_stack = Stack {
            .clip_behavior = Clip::None,
            .width = StyleValue::point(250.0f),
            .height = StyleValue::point(140.0f),
            .children = {
                pos_l0,
                pos_l1,
                pos_l2,
            }
        };

        return container({
            .color = Style::bg_card,
            .border_radius = BorderRadius::circular(12.0f),
            .border = Border(Style::border_subtle, 1.0f),
            .width = StyleValue::point(310.0f),
            .height = StyleValue::point(280.0f),
            .padding = StyleInsets::all(16.0f),
            .child = column({
                .align_items = Align::Center,
                .gap = StyleValue::point(6.0f),
                .children = {
                    text("3. Z-Index Hit-Testing", {
                        .color = Style::text_white,
                        .font_size = 14.0f,
                        .font_weight = FontWeight::Bold,
                    }),
                    text("Click overlapping layers: Topmost layer receives events first.", {
                        .color = Style::text_muted,
                        .font_size = 11.0f,
                    }),
                    overlap_stack,
                    text("Active Clicked: Layer #" + std::to_string(active_layer_index), {
                        .color = Style::cyan_neon,
                        .font_size = 11.0f,
                        .font_weight = FontWeight::Bold,
                    }),
                }
            })
        });
    }

    // ── Controls Row (Modal Trigger & Offset Sliders) ──

    WidgetPtr buildControlsRow() {
        auto btn_modal = Button {
            .child = text("🚀 Open Stack Modal Overlay", {
                .color = Style::text_white,
                .font_size = 13.0f,
                .font_weight = FontWeight::Bold,
            }),
            .on_pressed = [this]() {
                setState([this]() {
                    show_modal = true;
                });
            },
            .normal_color = Style::primary,
            .border_radius = 8.0f,
            .padding = EdgeInsets::symmetric(10.0f, 18.0f),
        };

        auto reset_btn = Button {
            .child = text("Reset State", {
                .color = Style::text_muted,
                .font_size = 12.0f,
            }),
            .on_pressed = [this]() {
                setState([this]() {
                    badge_count = 3;
                    is_online = true;
                    active_layer_index = 2;
                    show_modal = false;
                });
            },
            .normal_color = Style::bg_card,
            .border_radius = 8.0f,
            .padding = EdgeInsets::symmetric(10.0f, 14.0f),
        };

        return row({
            .justify_content = Justify::Center,
            .align_items = Align::Center,
            .gap = StyleValue::point(12.0f),
            .children = {
                btn_modal,
                reset_btn,
            }
        });
    }

    // ── Fullscreen Stack Modal Overlay (Positioned::fill) ──

    WidgetPtr buildModalOverlay() {
        auto backdrop = container({
            .color = 0xB3000000, // 70% black backdrop
        });

        auto backdrop_gesture = gestureDetector({
            .key = Key::string("modal_backdrop"),
            .child = backdrop,
            .on_tap_up = [this](const TapUpDetails&) {
                setState([this]() { show_modal = false; });
            },
        });

        auto close_btn = Button {
            .child = text("Close Modal", {
                .color = Style::text_white,
                .font_size = 12.0f,
                .font_weight = FontWeight::Bold,
            }),
            .on_pressed = [this]() {
                setState([this]() { show_modal = false; });
            },
            .normal_color = Style::rose,
            .border_radius = 6.0f,
            .padding = EdgeInsets::symmetric(8.0f, 16.0f),
        };

        auto dialog_card = container({
            .color = Style::bg_card,
            .border_radius = BorderRadius::circular(14.0f),
            .border = Border(Style::cyan_neon, 1.5f),
            .width = StyleValue::point(460.0f),
            .padding = StyleInsets::all(20.0f),
            .child = column({
                .gap = StyleValue::point(12.0f),
                .children = {
                    text("🌟 Stack Overlay Modal Dialog", {
                        .color = Style::text_white,
                        .font_size = 18.0f,
                        .font_weight = FontWeight::Bold,
                    }),
                    text("This entire overlay is rendered via Positioned::fill() on the root Stack.\n"
                         "It intercepts all pointer clicks and blocks background interaction seamlessly.", {
                        .color = Style::text_muted,
                        .font_size = 12.0f,
                    }),
                    row({
                        .justify_content = Justify::End,
                        .align_items = Align::Center,
                        .children = { close_btn }
                    }),
                }
            })
        });

        // Center the dialog card in the modal overlay stack
        auto modal_stack = Stack {
            .alignment = Alignment::Center,
            .fit = StackFit::Expand,
            .children = {
                Positioned::fill(backdrop_gesture),
                dialog_card,
            }
        };

        return Positioned::fill(modal_stack);
    }
};

std::unique_ptr<State> StackDemoApp::createState() {
    return std::make_unique<StackDemoState>();
}

// ════════════════════════════════════════════════════════════════
// Main Entry
// ════════════════════════════════════════════════════════════════

int main(int argc, char** argv) {
    AppConfig config;
    config.title                    = "ENKI — Stack & Positioned Showcase";
    config.width                    = 1060;
    config.height                   = 450;
    config.window_mode              = WindowMode::Normal;
    config.vsync                    = false;
    config.target_fps               = 0;
    config.show_performance_overlay = true;
    config.clear_color              = Style::bg_dark;

    return runApp(std::make_shared<StackDemoApp>(), config);
}
