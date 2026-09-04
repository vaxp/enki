/// @file main.cpp
/// @brief ENKI Engine — CustomMultiChildLayout Studio Showcase Demo
/// @details Roadmap v0.2.0 | Section 11 Layout — Extended
/// High-performance demo with unique Key on every element for + FPS reconciliation.

#include "enki/app/app.hpp"
#include "enki/widgets/custom_multi_child_layout.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/divider.hpp"
#include "enki/widgets/scroll_view.hpp"
#include "enki/state/state.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <cmath>

using namespace enki;

// ════════════════════════════════════════════════════════════════
// Custom Layout Delegates for the Showcase
// ════════════════════════════════════════════════════════════════

/// @brief Delegate 1: FAB Anchor Layout (FAB halfway overlapping header & content)
class FabAnchorLayoutDelegate : public MultiChildLayoutDelegate {
public:
    float header_height = 140.0f;
    float fab_margin_right = 40.0f;
    float seam_gap = 14.0f;

    FabAnchorLayoutDelegate(float h_height, float fab_mr)
        : header_height(h_height), fab_margin_right(fab_mr) {}

    Size getSize(const BoxConstraints& constraints) override {
        return Size{
            constraints.hasBoundedWidth() ? constraints.max_width : 640.0f,
            380.0f
        };
    }

    void performLayout(Size size) override {
        // 1. Header card
        if (hasChild("header")) {
            layoutChild("header", BoxConstraints::tightFor(size.width, header_height));
            positionChild("header", Point{0.0f, 0.0f});
        }

        // 2. Body / Content below header with a clean gap
        float body_y = header_height + seam_gap;
        float body_h = size.height - body_y;
        if (body_h < 60.0f) body_h = 60.0f;

        if (hasChild("body")) {
            layoutChild("body", BoxConstraints::tightFor(size.width, body_h));
            positionChild("body", Point{0.0f, body_y});
        }

        // 3. Floating Action Button (FAB) anchored over the seam line
        if (hasChild("fab")) {
            Size fab_sz = layoutChild("fab", BoxConstraints::tight(Size{56.0f, 56.0f}));
            Point fab_pos = {
                size.width - fab_sz.width - fab_margin_right,
                header_height + (seam_gap / 2.0f) - (fab_sz.height / 2.0f)
            };
            positionChild("fab", fab_pos);
        }
    }

    bool shouldRelayout(const MultiChildLayoutDelegate& oldDelegate) const override {
        const auto* old = dynamic_cast<const FabAnchorLayoutDelegate*>(&oldDelegate);
        return !old || old->header_height != header_height || old->fab_margin_right != fab_margin_right;
    }
};

/// @brief Delegate 2: Radial Orbit Hub (Satellites positioned in circle around central node)
class RadialOrbitLayoutDelegate : public MultiChildLayoutDelegate {
public:
    float radius = 120.0f;
    float start_angle_rad = 0.0f;

    RadialOrbitLayoutDelegate(float r, float angle_deg)
        : radius(r), start_angle_rad(angle_deg * 3.14159265f / 180.0f) {}

    Size getSize(const BoxConstraints& constraints) override {
        return Size{
            constraints.hasBoundedWidth() ? constraints.max_width : 640.0f,
            380.0f
        };
    }

    void performLayout(Size size) override {
        Point center = { size.width / 2.0f, size.height / 2.0f };

        // 1. Center Hub Node
        if (hasChild("center_hub")) {
            Size hub_sz = layoutChild("center_hub", BoxConstraints::tight(Size{90.0f, 90.0f}));
            positionChild("center_hub", Point{ center.x - hub_sz.width / 2.0f, center.y - hub_sz.height / 2.0f });
        }

        // 2. Satellites (4 nodes)
        const std::vector<std::string> sat_ids = {"sat_1", "sat_2", "sat_3", "sat_4"};
        float step = (2.0f * 3.14159265f) / 4.0f;

        for (size_t i = 0; i < sat_ids.size(); ++i) {
            const auto& sid = sat_ids[i];
            if (hasChild(sid)) {
                Size sat_sz = layoutChild(sid, BoxConstraints::tight(Size{72.0f, 72.0f}));
                float theta = start_angle_rad + static_cast<float>(i) * step;
                float px = center.x + radius * std::cos(theta) - (sat_sz.width / 2.0f);
                float py = center.y + radius * std::sin(theta) - (sat_sz.height / 2.0f);
                positionChild(sid, Point{ px, py });
            }
        }
    }

    bool shouldRelayout(const MultiChildLayoutDelegate& oldDelegate) const override {
        const auto* old = dynamic_cast<const RadialOrbitLayoutDelegate*>(&oldDelegate);
        return !old || old->radius != radius || old->start_angle_rad != start_angle_rad;
    }
};

/// @brief Delegate 3: Dependent Card Layout (Avatar with dependent badge and side content)
class DependentCardLayoutDelegate : public MultiChildLayoutDelegate {
public:
    float avatar_size = 80.0f;
    std::string badge_corner = "top_right"; // "top_right", "bottom_right", "top_left"

    DependentCardLayoutDelegate(float a_size, std::string corner)
        : avatar_size(a_size), badge_corner(std::move(corner)) {}

    Size getSize(const BoxConstraints& constraints) override {
        return Size{
            constraints.hasBoundedWidth() ? constraints.max_width : 640.0f,
            380.0f
        };
    }

    void performLayout(Size size) override {
        float start_x = 30.0f;
        float start_y = 60.0f;

        // 1. Avatar
        Size actual_avatar = Size{0.0f, 0.0f};
        if (hasChild("avatar")) {
            actual_avatar = layoutChild("avatar", BoxConstraints::tight(Size{avatar_size, avatar_size}));
            positionChild("avatar", Point{start_x, start_y});
        }

        // 2. Badge overlapping the avatar corner
        if (hasChild("badge")) {
            Size badge_sz = layoutChild("badge", BoxConstraints::tight(Size{26.0f, 26.0f}));
            Point badge_pos = {0.0f, 0.0f};
            if (badge_corner == "top_right") {
                badge_pos = { start_x + actual_avatar.width - (badge_sz.width / 2.0f), start_y - (badge_sz.height / 2.0f) };
            } else if (badge_corner == "bottom_right") {
                badge_pos = { start_x + actual_avatar.width - (badge_sz.width / 2.0f), start_y + actual_avatar.height - (badge_sz.height / 2.0f) };
            } else {
                badge_pos = { start_x - (badge_sz.width / 2.0f), start_y - (badge_sz.height / 2.0f) };
            }
            positionChild("badge", badge_pos);
        }

        // 3. User Info Card placed precisely to the right of avatar
        if (hasChild("info_card")) {
            float info_x = start_x + actual_avatar.width + 24.0f;
            float info_w = size.width - info_x - 30.0f;
            if (info_w < 100.0f) info_w = 100.0f;
            Size info_sz = layoutChild("info_card", BoxConstraints::tightFor(info_w, 160.0f));
            (void)info_sz;
            positionChild("info_card", Point{info_x, start_y});
        }

        // 4. Secondary Action Bar placed below both
        if (hasChild("action_bar")) {
            float bottom_y = start_y + std::max(actual_avatar.height, 160.0f) + 24.0f;
            float bar_w = size.width - (start_x * 2.0f);
            Size bar_sz = layoutChild("action_bar", BoxConstraints::tightFor(bar_w, 54.0f));
            (void)bar_sz;
            positionChild("action_bar", Point{start_x, bottom_y});
        }
    }

    bool shouldRelayout(const MultiChildLayoutDelegate& oldDelegate) const override {
        const auto* old = dynamic_cast<const DependentCardLayoutDelegate*>(&oldDelegate);
        return !old || old->avatar_size != avatar_size || old->badge_corner != badge_corner;
    }
};

// ════════════════════════════════════════════════════════════════
// Showcase Application State
// ════════════════════════════════════════════════════════════════

enum class ShowcaseMode {
    FabAnchor,
    RadialOrbit,
    DependentCard
};

class CustomMultiChildLayoutDemoApp : public StatefulWidget {
public:
    CustomMultiChildLayoutDemoApp() = default;
    std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "CustomMultiChildLayoutDemoApp"; }
};

class CustomMultiChildLayoutDemoState : public State {
private:
    ShowcaseMode current_mode_ = ShowcaseMode::FabAnchor;

    // Mode 1 settings
    float header_height_ = 140.0f;
    float fab_margin_ = 36.0f;

    // Mode 2 settings
    float orbit_radius_ = 120.0f;
    float orbit_angle_deg_ = 0.0f;

    // Mode 3 settings
    float avatar_size_ = 80.0f;
    std::string badge_corner_ = "top_right";

    // Interaction stats
    std::string status_log_ = "Ready. Select a layout mode or tune layout parameters.";
    int interaction_count_ = 0;

    WidgetPtr makeTabButton(ShowcaseMode mode, const std::string& label, const std::string& key_id) {
        bool is_active = (current_mode_ == mode);
        auto txt = text({
            .text = label,
            .color = is_active ? 0xFF060911 : 0xFFCBD5E1,
            .font_size = 12.0f,
            .font_weight = is_active ? FontWeight::Bold : FontWeight::Normal,
            .key = Key::string("txt_tab_" + key_id),
        });

        auto box = container({
            .color = is_active ? 0xFF06B6D4 : 0xFF1E293B,
            .border_radius = BorderRadius::circular(8.0f),
            .border = Border(is_active ? 0xFF22D3EE : 0xFF334155, 1.0f),
            .padding = StyleInsets::symmetric(8.0f, 16.0f),
            .child = txt,
            .key = Key::string("box_tab_" + key_id),
        });

        return gestureDetector({
            .key = Key::string("btn_tab_" + key_id),
            .child = box,
            .cursor_type = SystemCursor::Pointer,
            .on_tap = [this, mode, label]() {
                setState([this, mode, label]() {
                    current_mode_ = mode;
                    status_log_ = "Switched to layout pattern: " + label;
                    interaction_count_++;
                });
            },
        });
    }

    WidgetPtr makeActionButton(const std::string& label, const std::string& key_id, uint32_t color, std::function<void()> on_click) {
        auto txt = text({
            .text = label,
            .color = 0xFFFFFFFF,
            .font_size = 11.0f,
            .font_weight = FontWeight::SemiBold,
            .key = Key::string("txt_act_" + key_id),
        });

        auto box = container({
            .color = color,
            .border_radius = BorderRadius::circular(6.0f),
            .border = Border(0x33FFFFFF, 1.0f),
            .padding = StyleInsets::symmetric(6.0f, 12.0f),
            .child = txt,
            .key = Key::string("box_act_" + key_id),
        });

        return gestureDetector({
            .key = Key::string("btn_act_" + key_id),
            .child = box,
            .cursor_type = SystemCursor::Pointer,
            .on_tap = [this, on_click]() {
                setState([this, on_click]() {
                    on_click();
                    interaction_count_++;
                });
            },
        });
    }

    WidgetPtr buildFabAnchorScene() {
        auto delegate = std::make_shared<FabAnchorLayoutDelegate>(header_height_, fab_margin_);

        return customMultiChildLayout({
            .key = Key::string("cmcl_fab_anchor"),
            .delegate = delegate,
            .children = {
                // Header
                layoutId({
                    .key = Key::string("lid_header"),
                    .id = "header",
                    .child = container({
                        .color = 0xFF1E1B4B, // Deep indigo
                        .border_radius = BorderRadius::circular(14.0f),
                        .border = Border(0xFF4338CA, 1.0f),
                        .padding = StyleInsets::all(20.0f),
                        .child = column({
                            .children = {
                                row({
                                    .justify_content = Justify::SpaceBetween,
                                    .align_items = Align::Center,
                                    .children = {
                                        text({
                                            .text = "CLOUD INFRASTRUCTURE CONSOLE",
                                            .color = 0xFF818CF8,
                                            .font_size = 11.0f,
                                            .font_weight = FontWeight::Bold,
                                            .key = Key::string("fab_h_subtitle"),
                                        }),
                                        gestureDetector({
                                            .key = Key::string("btn_pill"),
                                            .child = container({
                                                .color = 0xFF059669,
                                                .border_radius = BorderRadius::circular(12.0f),
                                                .border = Border(0xFF34D399, 1.0f),
                                                .padding = StyleInsets::symmetric(4.0f, 10.0f),
                                                .child = text({
                                                    .text = "● ALL SYSTEMS OPERATIONAL",
                                                    .color = 0xFFFFFFFF,
                                                    .font_size = 10.0f,
                                                    .font_weight = FontWeight::Bold,
                                                    .key = Key::string("pill_txt"),
                                                }),
                                                .key = Key::string("box_pill"),
                                            }),
                                            .on_tap = [this]() {
                                                setState([this]() {
                                                    status_log_ = "Clicked Status Pill: All microservices healthy.";
                                                    interaction_count_++;
                                                });
                                            },
                                        }),
                                    },
                                    .key = Key::string("fab_h_top_row"),
                                }),
                                container({ .height = StyleValue::point(8.0f), .key = Key::string("fab_h_sp1") }),
                                text({
                                    .text = "Region: eu-west-1 • 8 Microservices Active",
                                    .color = 0xFFFFFFFF,
                                    .font_size = 16.0f,
                                    .font_weight = FontWeight::Bold,
                                    .key = Key::string("fab_h_title"),
                                }),
                            },
                            .key = Key::string("fab_h_col"),
                        }),
                        .key = Key::string("box_header"),
                    }),
                }),

                // 2. Body Section (painted BEFORE fab so fab floats on top of the seam)
                layoutId({
                    .key = Key::string("lid_body"),
                    .id = "body",
                    .child = container({
                        .color = 0xFF0F172A,
                        .border_radius = BorderRadius::circular(14.0f),
                        .border = Border(0xFF334155, 1.0f),
                        .padding = StyleInsets::all(24.0f),
                        .child = column({
                            .children = {
                                text({
                                    .text = "Deployment Stream & Pod Metrics",
                                    .color = 0xFFF1F5F9,
                                    .font_size = 15.0f,
                                    .font_weight = FontWeight::SemiBold,
                                    .key = Key::string("body_title"),
                                }),
                                container({ .height = StyleValue::point(8.0f), .key = Key::string("body_sp1") }),
                                text({
                                    .text = "The FAB dynamically anchors exactly over the seam line, bridging header and body seamlessly.",
                                    .color = 0xFF94A3B8,
                                    .font_size = 12.0f,
                                    .key = Key::string("body_desc"),
                                }),
                                container({ .height = StyleValue::point(14.0f), .key = Key::string("body_sp2") }),
                                row({
                                    .gap = StyleValue::point(12.0f),
                                    .children = {
                                        container({
                                            .color = 0xFF1E293B,
                                            .border_radius = BorderRadius::circular(8.0f),
                                            .border = Border(0xFF475569, 1.0f),
                                            .padding = StyleInsets::symmetric(8.0f, 14.0f),
                                            .child = text({
                                                .text = "Throughput: 4,820 req/s",
                                                .color = 0xFF38BDF8,
                                                .font_size = 11.0f,
                                                .key = Key::string("metric_1"),
                                            }),
                                            .key = Key::string("box_m1"),
                                        }),
                                        container({
                                            .color = 0xFF1E293B,
                                            .border_radius = BorderRadius::circular(8.0f),
                                            .border = Border(0xFF475569, 1.0f),
                                            .padding = StyleInsets::symmetric(8.0f, 14.0f),
                                            .child = text({
                                                .text = "Latency: 2.1 ms",
                                                .color = 0xFF4ADE80,
                                                .font_size = 11.0f,
                                                .key = Key::string("metric_2"),
                                            }),
                                            .key = Key::string("box_m2"),
                                        }),
                                    },
                                    .key = Key::string("metrics_row"),
                                }),
                            },
                            .key = Key::string("body_col"),
                        }),
                        .key = Key::string("box_body"),
                    }),
                }),

                // 3. Floating Action Button (FAB) (painted AFTER body so it is on TOP!)
                layoutId({
                    .key = Key::string("lid_fab"),
                    .id = "fab",
                    .child = gestureDetector({
                        .key = Key::string("btn_fab"),
                        .child = container({
                            .color = 0xFFEC4899,
                            .border_radius = BorderRadius::circular(28.0f),
                            .border = Border(0xFFF472B6, 2.0f),
                            .align = Alignment::Center,
                            .width = StyleValue::point(56.0f),
                            .height = StyleValue::point(56.0f),
                            .child = text({
                                .text = "+",
                                .color = 0xFFFFFFFF,
                                .font_size = 30.0f,
                                .font_weight = FontWeight::Bold,
                                .key = Key::string("fab_plus_txt"),
                            }),
                            .key = Key::string("box_fab"),
                        }),
                        .cursor_type = SystemCursor::Pointer,
                        .on_tap = [this]() {
                            setState([this]() {
                                status_log_ = "Clicked FAB (+)! Spawned cluster deployment task.";
                                interaction_count_++;
                            });
                        },
                    }),
                }),
            },
        });
    }

    WidgetPtr buildRadialOrbitScene() {
        auto delegate = std::make_shared<RadialOrbitLayoutDelegate>(orbit_radius_, orbit_angle_deg_);

        return customMultiChildLayout({
            .key = Key::string("cmcl_radial_orbit"),
            .delegate = delegate,
            .children = {
                // Central Node
                layoutId({
                    .key = Key::string("lid_hub"),
                    .id = "center_hub",
                    .child = gestureDetector({
                        .key = Key::string("btn_hub"),
                        .child = container({
                            .color = 0xFF4F46E5, // Indigo
                            .border_radius = BorderRadius::circular(45.0f),
                            .border = Border(0xFF818CF8, 3.0f),
                            .align = Alignment::Center,
                            .width = StyleValue::point(90.0f),
                            .height = StyleValue::point(90.0f),
                            .child = column({
                                .justify_content = Justify::Center,
                                .align_items = Align::Center,
                                .children = {
                                    text({ .text = "CORE", .color = 0xFFFFFFFF, .font_size = 12.0f, .font_weight = FontWeight::Bold, .key = Key::string("hub_t1") }),
                                    text({ .text = "CLUSTER", .color = 0xFFA5B4FC, .font_size = 9.0f, .key = Key::string("hub_t2") }),
                                },
                                .key = Key::string("hub_col"),
                            }),
                            .key = Key::string("box_hub"),
                        }),
                        .cursor_type = SystemCursor::Pointer,
                        .on_tap = [this]() {
                            setState([this]() {
                                status_log_ = "Clicked Core Cluster: Primary orchestrator active.";
                                interaction_count_++;
                            });
                        },
                    }),
                }),

                // Satellite 1
                layoutId({
                    .key = Key::string("lid_sat1"),
                    .id = "sat_1",
                    .child = gestureDetector({
                        .key = Key::string("btn_sat1"),
                        .child = container({
                            .color = 0xFF0284C7, // Sky
                            .border_radius = BorderRadius::circular(36.0f),
                            .border = Border(0xFF38BDF8, 2.0f),
                            .align = Alignment::Center,
                            .width = StyleValue::point(72.0f),
                            .height = StyleValue::point(72.0f),
                            .child = text({ .text = "AUTH", .color = 0xFFFFFFFF, .font_size = 12.0f, .font_weight = FontWeight::Bold, .key = Key::string("sat1_txt") }),
                            .key = Key::string("box_sat1"),
                        }),
                        .on_tap = [this]() {
                            setState([this]() {
                                status_log_ = "Clicked Satellite 1 [AUTH]: JWT service synced.";
                                interaction_count_++;
                            });
                        },
                    }),
                }),

                // Satellite 2
                layoutId({
                    .key = Key::string("lid_sat2"),
                    .id = "sat_2",
                    .child = gestureDetector({
                        .key = Key::string("btn_sat2"),
                        .child = container({
                            .color = 0xFF059669, // Emerald
                            .border_radius = BorderRadius::circular(36.0f),
                            .border = Border(0xFF34D399, 2.0f),
                            .align = Alignment::Center,
                            .width = StyleValue::point(72.0f),
                            .height = StyleValue::point(72.0f),
                            .child = text({ .text = "DB POOL", .color = 0xFFFFFFFF, .font_size = 11.0f, .font_weight = FontWeight::Bold, .key = Key::string("sat2_txt") }),
                            .key = Key::string("box_sat2"),
                        }),
                        .on_tap = [this]() {
                            setState([this]() {
                                status_log_ = "Clicked Satellite 2 [DB POOL]: Connection pool 98% idle.";
                                interaction_count_++;
                            });
                        },
                    }),
                }),

                // Satellite 3
                layoutId({
                    .key = Key::string("lid_sat3"),
                    .id = "sat_3",
                    .child = gestureDetector({
                        .key = Key::string("btn_sat3"),
                        .child = container({
                            .color = 0xFFD97706, // Amber
                            .border_radius = BorderRadius::circular(36.0f),
                            .border = Border(0xFFFBBF24, 2.0f),
                            .align = Alignment::Center,
                            .width = StyleValue::point(72.0f),
                            .height = StyleValue::point(72.0f),
                            .child = text({ .text = "QUEUE", .color = 0xFFFFFFFF, .font_size = 12.0f, .font_weight = FontWeight::Bold, .key = Key::string("sat3_txt") }),
                            .key = Key::string("box_sat3"),
                        }),
                        .on_tap = [this]() {
                            setState([this]() {
                                status_log_ = "Clicked Satellite 3 [QUEUE]: RabbitMQ backlog empty.";
                                interaction_count_++;
                            });
                        },
                    }),
                }),

                // Satellite 4
                layoutId({
                    .key = Key::string("lid_sat4"),
                    .id = "sat_4",
                    .child = gestureDetector({
                        .key = Key::string("btn_sat4"),
                        .child = container({
                            .color = 0xFF9333EA, // Purple
                            .border_radius = BorderRadius::circular(36.0f),
                            .border = Border(0xFFC084FC, 2.0f),
                            .align = Alignment::Center,
                            .width = StyleValue::point(72.0f),
                            .height = StyleValue::point(72.0f),
                            .child = text({ .text = "EDGE", .color = 0xFFFFFFFF, .font_size = 12.0f, .font_weight = FontWeight::Bold, .key = Key::string("sat4_txt") }),
                            .key = Key::string("box_sat4"),
                        }),
                        .on_tap = [this]() {
                            setState([this]() {
                                status_log_ = "Clicked Satellite 4 [EDGE]: Envoy proxy active.";
                                interaction_count_++;
                            });
                        },
                    }),
                }),
            },
        });
    }

    WidgetPtr buildDependentCardScene() {
        auto delegate = std::make_shared<DependentCardLayoutDelegate>(avatar_size_, badge_corner_);

        return customMultiChildLayout({
            .key = Key::string("cmcl_dependent_card"),
            .delegate = delegate,
            .children = {
                // Avatar
                layoutId({
                    .key = Key::string("lid_dep_avatar"),
                    .id = "avatar",
                    .child = gestureDetector({
                        .key = Key::string("btn_avatar"),
                        .child = container({
                            .color = 0xFF312E81,
                            .border_radius = BorderRadius::circular(avatar_size_ / 2.0f),
                            .border = Border(0xFF6366F1, 2.5f),
                            .align = Alignment::Center,
                            .width = StyleValue::point(avatar_size_),
                            .height = StyleValue::point(avatar_size_),
                            .child = text({
                                .text = "ENKI",
                                .color = 0xFFE0E7FF,
                                .font_size = avatar_size_ * 0.24f,
                                .font_weight = FontWeight::Bold,
                                .key = Key::string("avatar_txt"),
                            }),
                            .key = Key::string("box_avatar"),
                        }),
                        .on_tap = [this]() {
                            setState([this]() {
                                status_log_ = "Clicked Avatar: User profile inspected.";
                                interaction_count_++;
                            });
                        },
                    }),
                }),

                // Dependent Badge pinned to corner
                layoutId({
                    .key = Key::string("lid_dep_badge"),
                    .id = "badge",
                    .child = container({
                        .color = 0xFFEF4444, // Red
                        .border_radius = BorderRadius::circular(13.0f),
                        .border = Border(0xFFFFFFFF, 2.0f),
                        .align = Alignment::Center,
                        .width = StyleValue::point(26.0f),
                        .height = StyleValue::point(26.0f),
                        .child = text({
                            .text = "9+",
                            .color = 0xFFFFFFFF,
                            .font_size = 10.0f,
                            .font_weight = FontWeight::Bold,
                            .key = Key::string("badge_txt"),
                        }),
                        .key = Key::string("box_badge"),
                    }),
                }),

                // User Info Card positioned dynamically to the right
                layoutId({
                    .key = Key::string("lid_dep_info"),
                    .id = "info_card",
                    .child = container({
                        .color = 0xFF1E293B,
                        .border_radius = BorderRadius::circular(12.0f),
                        .border = Border(0xFF334155, 1.0f),
                        .padding = StyleInsets::all(18.0f),
                        .child = column({
                            .children = {
                                text({
                                    .text = "Alex Thorne • Principal Architect",
                                    .color = 0xFFF8FAFC,
                                    .font_size = 15.0f,
                                    .font_weight = FontWeight::Bold,
                                    .key = Key::string("info_name"),
                                }),
                                container({ .height = StyleValue::point(4.0f), .key = Key::string("info_sp1") }),
                                text({
                                    .text = "Department: Core Systems & High-Performance UI",
                                    .color = 0xFF94A3B8,
                                    .font_size = 12.0f,
                                    .key = Key::string("info_dept"),
                                }),
                                container({ .height = StyleValue::point(14.0f), .key = Key::string("info_sp2") }),
                                text({
                                    .text = "This card's left coordinate automatically tracks: (avatar.x + avatar.width + 24px) regardless of avatar resizing.",
                                    .color = 0xFF64748B,
                                    .font_size = 11.0f,
                                    .key = Key::string("info_desc"),
                                }),
                            },
                            .key = Key::string("info_col"),
                        }),
                        .key = Key::string("box_info"),
                    }),
                }),

                // Action Bar below both
                layoutId({
                    .key = Key::string("lid_dep_actions"),
                    .id = "action_bar",
                    .child = container({
                        .color = 0xFF0F172A,
                        .border_radius = BorderRadius::circular(10.0f),
                        .border = Border(0xFF1E293B, 1.0f),
                        .padding = StyleInsets::symmetric(10.0f, 16.0f),
                        .child = row({
                            .justify_content = Justify::SpaceBetween,
                            .align_items = Align::Center,
                            .children = {
                                text({
                                    .text = "Security Token: ACTIVE (TTL 2h 45m)",
                                    .color = 0xFF22C55E,
                                    .font_size = 11.0f,
                                    .font_weight = FontWeight::SemiBold,
                                    .key = Key::string("bar_sec_txt"),
                                }),
                                makeActionButton("REVOKE ACCESS", "revoke", 0xFFDC2626, [this]() {
                                    status_log_ = "Action triggered: Revoke access key requested.";
                                }),
                            },
                            .key = Key::string("bar_row"),
                        }),
                        .key = Key::string("box_actions"),
                    }),
                }),
            },
        });
    }

    WidgetPtr buildControls() {
        if (current_mode_ == ShowcaseMode::FabAnchor) {
            return row({
                .align_items = Align::Center,
                .gap = StyleValue::point(10.0f),
                .children = {
                    text({ .text = "Header Height:", .color = 0xFF94A3B8, .font_size = 12.0f, .key = Key::string("ctrl_h_lbl") }),
                    makeActionButton("110px", "h110", header_height_ == 110.0f ? 0xFF6366F1 : 0xFF1E293B, [this]() {
                        header_height_ = 110.0f;
                        status_log_ = "Header height set to 110px (FAB tracked automatically).";
                    }),
                    makeActionButton("150px", "h150", header_height_ == 150.0f ? 0xFF6366F1 : 0xFF1E293B, [this]() {
                        header_height_ = 150.0f;
                        status_log_ = "Header height set to 150px (FAB tracked automatically).";
                    }),
                    makeActionButton("190px", "h190", header_height_ == 190.0f ? 0xFF6366F1 : 0xFF1E293B, [this]() {
                        header_height_ = 190.0f;
                        status_log_ = "Header height set to 190px (FAB tracked automatically).";
                    }),
                    container({ .width = StyleValue::point(16.0f), .key = Key::string("sp_ctrl_1") }),
                    text({ .text = "FAB Margin:", .color = 0xFF94A3B8, .font_size = 12.0f, .key = Key::string("ctrl_m_lbl") }),
                    makeActionButton("24px", "m24", fab_margin_ == 24.0f ? 0xFFEC4899 : 0xFF1E293B, [this]() {
                        fab_margin_ = 24.0f;
                        status_log_ = "FAB right margin set to 24px.";
                    }),
                    makeActionButton("48px", "m48", fab_margin_ == 48.0f ? 0xFFEC4899 : 0xFF1E293B, [this]() {
                        fab_margin_ = 48.0f;
                        status_log_ = "FAB right margin set to 48px.";
                    }),
                },
                .key = Key::string("row_fab_ctrls"),
            });
        } else if (current_mode_ == ShowcaseMode::RadialOrbit) {
            return row({
                .align_items = Align::Center,
                .gap = StyleValue::point(10.0f),
                .children = {
                    text({ .text = "Orbit Radius:", .color = 0xFF94A3B8, .font_size = 12.0f, .key = Key::string("ctrl_r_lbl") }),
                    makeActionButton("90px", "r90", orbit_radius_ == 90.0f ? 0xFF0284C7 : 0xFF1E293B, [this]() {
                        orbit_radius_ = 90.0f;
                        status_log_ = "Orbit radius set to 90px.";
                    }),
                    makeActionButton("125px", "r125", orbit_radius_ == 125.0f ? 0xFF0284C7 : 0xFF1E293B, [this]() {
                        orbit_radius_ = 125.0f;
                        status_log_ = "Orbit radius set to 125px.";
                    }),
                    makeActionButton("155px", "r155", orbit_radius_ == 155.0f ? 0xFF0284C7 : 0xFF1E293B, [this]() {
                        orbit_radius_ = 155.0f;
                        status_log_ = "Orbit radius set to 155px.";
                    }),
                    container({ .width = StyleValue::point(16.0f), .key = Key::string("sp_ctrl_2") }),
                    text({ .text = "Rotation:", .color = 0xFF94A3B8, .font_size = 12.0f, .key = Key::string("ctrl_rot_lbl") }),
                    makeActionButton("Rotate +45°", "rot_plus", 0xFF059669, [this]() {
                        orbit_angle_deg_ += 45.0f;
                        status_log_ = "Orbit rotated +45 deg (New angle: " + std::to_string(static_cast<int>(orbit_angle_deg_)) + "°).";
                    }),
                    makeActionButton("Reset (0°)", "rot_reset", 0xFF334155, [this]() {
                        orbit_angle_deg_ = 0.0f;
                        status_log_ = "Orbit rotation reset to 0°.";
                    }),
                },
                .key = Key::string("row_orbit_ctrls"),
            });
        } else {
            return row({
                .align_items = Align::Center,
                .gap = StyleValue::point(10.0f),
                .children = {
                    text({ .text = "Avatar Size:", .color = 0xFF94A3B8, .font_size = 12.0f, .key = Key::string("ctrl_av_lbl") }),
                    makeActionButton("64px", "av64", avatar_size_ == 64.0f ? 0xFF6366F1 : 0xFF1E293B, [this]() {
                        avatar_size_ = 64.0f;
                        status_log_ = "Avatar size set to 64px (Info card and badge re-anchored).";
                    }),
                    makeActionButton("84px", "av84", avatar_size_ == 84.0f ? 0xFF6366F1 : 0xFF1E293B, [this]() {
                        avatar_size_ = 84.0f;
                        status_log_ = "Avatar size set to 84px (Info card and badge re-anchored).";
                    }),
                    makeActionButton("104px", "av104", avatar_size_ == 104.0f ? 0xFF6366F1 : 0xFF1E293B, [this]() {
                        avatar_size_ = 104.0f;
                        status_log_ = "Avatar size set to 104px (Info card and badge re-anchored).";
                    }),
                    container({ .width = StyleValue::point(16.0f), .key = Key::string("sp_ctrl_3") }),
                    text({ .text = "Badge Corner:", .color = 0xFF94A3B8, .font_size = 12.0f, .key = Key::string("ctrl_bc_lbl") }),
                    makeActionButton("Top-Right", "c_tr", badge_corner_ == "top_right" ? 0xFFEF4444 : 0xFF1E293B, [this]() {
                        badge_corner_ = "top_right";
                        status_log_ = "Badge corner anchored to Top-Right.";
                    }),
                    makeActionButton("Bottom-Right", "c_br", badge_corner_ == "bottom_right" ? 0xFFEF4444 : 0xFF1E293B, [this]() {
                        badge_corner_ = "bottom_right";
                        status_log_ = "Badge corner anchored to Bottom-Right.";
                    }),
                    makeActionButton("Top-Left", "c_tl", badge_corner_ == "top_left" ? 0xFFEF4444 : 0xFF1E293B, [this]() {
                        badge_corner_ = "top_left";
                        status_log_ = "Badge corner anchored to Top-Left.";
                    }),
                },
                .key = Key::string("row_dep_ctrls"),
            });
        }
    }

public:
    WidgetPtr build(BuildContext& /*ctx*/) override {
        return container({
            .color = 0xFF060911, // Darkest base
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(24.0f),
            .child = column({
                .children = {
                    // Header Bar
                    row({
                        .justify_content = Justify::SpaceBetween,
                        .align_items = Align::Center,
                        .children = {
                            column({
                                .children = {
                                    text({
                                        .text = "CUSTOM MULTI-CHILD LAYOUT STUDIO",
                                        .color = 0xFF06B6D4,
                                        .font_size = 11.0f,
                                        .font_weight = FontWeight::Bold,
                                        .key = Key::string("header_tag"),
                                    }),
                                    container({ .height = StyleValue::point(4.0f), .key = Key::string("h_sp0") }),
                                    text({
                                        .text = "Declarative C++20 Layout Delegates & Dynamic Constraints",
                                        .color = 0xFFFFFFFF,
                                        .font_size = 18.0f,
                                        .font_weight = FontWeight::Bold,
                                        .key = Key::string("header_title"),
                                    }),
                                },
                                .key = Key::string("h_text_col"),
                            }),
                            container({
                                .color = 0xFF111827,
                                .border_radius = BorderRadius::circular(20.0f),
                                .border = Border(0xFF374151, 1.0f),
                                .padding = StyleInsets::symmetric(8.0f, 16.0f),
                                .child = text({
                                    .text = "ENKI v0.2.0 • + FPS",
                                    .color = 0xFF10B981,
                                    .font_size = 11.0f,
                                    .font_weight = FontWeight::Bold,
                                    .key = Key::string("fps_badge_txt"),
                                }),
                                .key = Key::string("fps_badge_box"),
                            }),
                        },
                        .key = Key::string("top_header_row"),
                    }),

                    container({ .height = StyleValue::point(18.0f), .key = Key::string("sp_top") }),

                    // Navigation Tabs
                    row({
                        .gap = StyleValue::point(12.0f),
                        .children = {
                            makeTabButton(ShowcaseMode::FabAnchor, "1. FAB Seam Anchor", "fab"),
                            makeTabButton(ShowcaseMode::RadialOrbit, "2. Radial Orbit Hub", "orbit"),
                            makeTabButton(ShowcaseMode::DependentCard, "3. Dependent Profile Card", "card"),
                        },
                        .key = Key::string("tabs_row"),
                    }),

                    container({ .height = StyleValue::point(14.0f), .key = Key::string("sp_mid1") }),

                    // Parameter Controls Bar
                    container({
                        .color = 0xFF0B1120,
                        .border_radius = BorderRadius::circular(10.0f),
                        .border = Border(0xFF1E293B, 1.0f),
                        .padding = StyleInsets::symmetric(12.0f, 16.0f),
                        .child = buildControls(),
                        .key = Key::string("ctrls_box"),
                    }),

                    container({ .height = StyleValue::point(16.0f), .key = Key::string("sp_mid2") }),

                    // Main Layout Stage
                    container({
                        .color = 0xFF070B14,
                        .border_radius = BorderRadius::circular(14.0f),
                        .border = Border(0xFF1F2937, 1.0f),
                        .height = StyleValue::point(380.0f),
                        .child = (current_mode_ == ShowcaseMode::FabAnchor)
                            ? buildFabAnchorScene()
                            : (current_mode_ == ShowcaseMode::RadialOrbit)
                                ? buildRadialOrbitScene()
                                : buildDependentCardScene(),
                        .key = Key::string("stage_box"),
                    }),

                    container({ .height = StyleValue::point(14.0f), .key = Key::string("sp_mid3") }),

                    // Live Status & Architecture Bar
                    container({
                        .color = 0xFF0D1526,
                        .border_radius = BorderRadius::circular(10.0f),
                        .border = Border(0xFF1E293B, 1.0f),
                        .padding = StyleInsets::symmetric(10.0f, 16.0f),
                        .child = row({
                            .justify_content = Justify::SpaceBetween,
                            .align_items = Align::Center,
                            .children = {
                                row({
                                    .align_items = Align::Center,
                                    .gap = StyleValue::point(8.0f),
                                    .children = {
                                        text({ .text = "STATUS:", .color = 0xFF06B6D4, .font_size = 11.0f, .font_weight = FontWeight::Bold, .key = Key::string("st_lbl") }),
                                        text({ .text = status_log_, .color = 0xFFE2E8F0, .font_size = 11.0f, .key = Key::string("st_val") }),
                                    },
                                    .key = Key::string("st_left_row"),
                                }),
                                text({
                                    .text = "Events Dispatched: " + std::to_string(interaction_count_),
                                    .color = 0xFF94A3B8,
                                    .font_size = 11.0f,
                                    .key = Key::string("st_cnt"),
                                }),
                            },
                            .key = Key::string("status_row"),
                        }),
                        .key = Key::string("box_status_bar"),
                    }),
                },
                .key = Key::string("root_col"),
            }),
            .key = Key::string("root_container"),
        });
    }
};

std::unique_ptr<State> CustomMultiChildLayoutDemoApp::createState() {
    return std::make_unique<CustomMultiChildLayoutDemoState>();
}

int main() {
    std::cout << "====================================================" << std::endl;
    std::cout << "  ENKI Engine — CustomMultiChildLayout Showcase Demo" << std::endl;
    std::cout << "  Roadmap v0.2.0 | Section 11 Layout — Extended" << std::endl;
    std::cout << "====================================================" << std::endl;

    AppConfig config;
    config.title       = "ENKI Engine — CustomMultiChildLayout Showcase Demo";
    config.width       = 1140;
    config.height      = 820;
    config.target_fps  = 0;
    config.vsync       = false;
    config.show_performance_overlay = true;
    config.clear_color = 0xFF060911;

    return runApp(std::make_shared<CustomMultiChildLayoutDemoApp>(), config);
}
