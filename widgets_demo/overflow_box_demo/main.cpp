/// @file main.cpp
/// @brief ENKI Engine — OverflowBox Studio Showcase Demo
/// @details Roadmap v0.2.0 | Section 11 Layout — Extended
/// High-performance demo with unique Key on every element to prevent full-tree rebuilds.

#include "enki/app/app.hpp"
#include "enki/widgets/overflow_box.hpp"
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

using namespace enki;

class OverflowBoxDemoApp : public StatefulWidget {
public:
    OverflowBoxDemoApp() = default;
    std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "OverflowBoxDemoApp"; }
};

class OverflowBoxDemoState : public State {
private:
    Alignment alignment_ = Alignment::Center;
    Clip clip_mode_ = Clip::None;
    float child_size_ = 160.0f; // 80.0f (underflow), 160.0f (overflow), 230.0f (extreme)
    std::string last_action_ = "Ready. Interact with the alignment matrix, clip modes, or overhanging badge.";
    int badge_click_count_ = 0;

    [[nodiscard]] std::string alignmentName(Alignment a) const {
        switch (a) {
            case Alignment::TopLeft:      return "TopLeft";
            case Alignment::TopCenter:    return "TopCenter";
            case Alignment::TopRight:     return "TopRight";
            case Alignment::CenterLeft:   return "CenterLeft";
            case Alignment::Center:       return "Center";
            case Alignment::CenterRight:  return "CenterRight";
            case Alignment::BottomLeft:   return "BottomLeft";
            case Alignment::BottomCenter: return "BottomCenter";
            case Alignment::BottomRight:  return "BottomRight";
        }
        return "Unknown";
    }

    [[nodiscard]] std::string clipModeName(Clip c) const {
        switch (c) {
            case Clip::None:                   return "Clip::None (Free Overflow)";
            case Clip::HardEdge:               return "Clip::HardEdge (Hard Boundary Cut)";
            case Clip::AntiAlias:              return "Clip::AntiAlias (Smooth Anti-Aliased Cut)";
            case Clip::AntiAliasWithSaveLayer: return "Clip::AntiAliasWithSaveLayer";
        }
        return "Unknown";
    }

    WidgetPtr makeAlignmentButton(Alignment align, const std::string& key_id, const std::string& label) {
        bool is_selected = (alignment_ == align);
        auto btn_txt = text({
            .text = label,
            .color = is_selected ? 0xFF060911 : 0xFFE2E8F0,
            .font_size = 11.0f,
            .font_weight = is_selected ? FontWeight::Bold : FontWeight::Normal,
            .key = Key::string("txt_align_" + key_id),
        });

        auto btn_box = container({
            .color = is_selected ? 0xFF06B6D4 : 0xFF1E293B,
            .border_radius = BorderRadius::circular(8.0f),
            .border = Border(is_selected ? 0xFF22D3EE : 0xFF334155, 1.0f),
            .padding = StyleInsets::symmetric(8.0f, 10.0f),
            .child = btn_txt,
            .key = Key::string("box_align_" + key_id),
        });

        return gestureDetector({
            .key = Key::string("btn_align_" + key_id),
            .child = btn_box,
            .cursor_type = SystemCursor::Pointer,
            .on_tap = [this, align, label]() {
                setState([this, align, label]() {
                    alignment_ = align;
                    last_action_ = "Alignment shifted to: " + label;
                });
            },
        });
    }

    WidgetPtr makeClipButton(Clip clip, const std::string& key_id, const std::string& label) {
        bool is_selected = (clip_mode_ == clip);
        auto btn_txt = text({
            .text = label,
            .color = is_selected ? 0xFFFFFFFF : 0xFF94A3B8,
            .font_size = 12.0f,
            .font_weight = is_selected ? FontWeight::Bold : FontWeight::Normal,
            .key = Key::string("txt_clip_" + key_id),
        });

        auto btn_box = container({
            .color = is_selected ? 0xFF8B5CF6 : 0xFF1E293B,
            .border_radius = BorderRadius::circular(8.0f),
            .border = Border(is_selected ? 0xFFA78BFA : 0xFF334155, 1.0f),
            .padding = StyleInsets::symmetric(8.0f, 14.0f),
            .child = btn_txt,
            .key = Key::string("box_clip_" + key_id),
        });

        return gestureDetector({
            .key = Key::string("btn_clip_" + key_id),
            .child = btn_box,
            .cursor_type = SystemCursor::Pointer,
            .on_tap = [this, clip, label]() {
                setState([this, clip, label]() {
                    clip_mode_ = clip;
                    last_action_ = "Clipping mode set to: " + label;
                });
            },
        });
    }

    WidgetPtr makeSizeButton(float sz, const std::string& key_id, const std::string& label) {
        bool is_selected = (child_size_ == sz);
        auto btn_txt = text({
            .text = label,
            .color = is_selected ? 0xFF060911 : 0xFF94A3B8,
            .font_size = 12.0f,
            .font_weight = is_selected ? FontWeight::Bold : FontWeight::Normal,
            .key = Key::string("txt_size_" + key_id),
        });

        auto btn_box = container({
            .color = is_selected ? 0xFF10B981 : 0xFF1E293B,
            .border_radius = BorderRadius::circular(8.0f),
            .border = Border(is_selected ? 0xFF34D399 : 0xFF334155, 1.0f),
            .padding = StyleInsets::symmetric(8.0f, 14.0f),
            .child = btn_txt,
            .key = Key::string("box_size_" + key_id),
        });

        return gestureDetector({
            .key = Key::string("btn_size_" + key_id),
            .child = btn_box,
            .cursor_type = SystemCursor::Pointer,
            .on_tap = [this, sz, label]() {
                setState([this, sz, label]() {
                    child_size_ = sz;
                    last_action_ = "Child dimension set to: " + label;
                });
            },
        });
    }

public:
    WidgetPtr build(BuildContext&) override {
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
                            text({
                                .text = "ENKI Engine — OverflowBox Studio Showcase",
                                .color = 0xFFF8FAFC,
                                .font_size = 22.0f,
                                .font_weight = FontWeight::Bold,
                                .key = Key::string("header_title"),
                            }),
                            container({
                                .color = 0xFF0891B2,
                                .border_radius = BorderRadius::circular(6.0f),
                                .padding = StyleInsets::symmetric(3.0f, 8.0f),
                                .child = text({
                                    .text = "ROADMAP v0.2.0 §11",
                                    .color = 0xFFCFFAFE,
                                    .font_size = 11.0f,
                                    .font_weight = FontWeight::Bold,
                                    .key = Key::string("header_tag_txt"),
                                }),
                                .key = Key::string("header_tag"),
                            }),
                        },
                        .key = Key::string("header_row"),
                    }),
                    text({
                        .text = "Demonstrating boundary-independent child constraints, 9-directional overflow alignment, controlled Skia clipping, and outside-bounds hit testing.",
                        .color = 0xFF94A3B8,
                        .font_size = 13.0f,
                        .key = Key::string("header_subtitle"),
                    }),
                },
                .key = Key::string("header_col"),
            }),
            .key = Key::string("header_card"),
        });

        // ── 2. Interactive Control Hub ────────────────────────────────
        auto alignment_matrix = column({
            .gap = StyleValue::point(6.0f),
            .children = {
                row({
                    .gap = StyleValue::point(6.0f),
                    .children = {
                        makeAlignmentButton(Alignment::TopLeft, "tl", "↖ Top-Left"),
                        makeAlignmentButton(Alignment::TopCenter, "tc", "↑ Top-Center"),
                        makeAlignmentButton(Alignment::TopRight, "tr", "↗ Top-Right"),
                    },
                    .key = Key::string("matrix_row_1"),
                }),
                row({
                    .gap = StyleValue::point(6.0f),
                    .children = {
                        makeAlignmentButton(Alignment::CenterLeft, "cl", "← Center-Left"),
                        makeAlignmentButton(Alignment::Center, "cc", "• Center"),
                        makeAlignmentButton(Alignment::CenterRight, "cr", "→ Center-Right"),
                    },
                    .key = Key::string("matrix_row_2"),
                }),
                row({
                    .gap = StyleValue::point(6.0f),
                    .children = {
                        makeAlignmentButton(Alignment::BottomLeft, "bl", "↙ Bottom-Left"),
                        makeAlignmentButton(Alignment::BottomCenter, "bc", "↓ Bottom-Center"),
                        makeAlignmentButton(Alignment::BottomRight, "br", "↘ Bottom-Right"),
                    },
                    .key = Key::string("matrix_row_3"),
                }),
            },
            .key = Key::string("matrix_col"),
        });

        auto controls_panel = container({
            .color = 0xFF0F172A,
            .border_radius = BorderRadius::circular(14.0f),
            .border = Border(0xFF1E293B, 1.0f),
            .width = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(20.0f),
            .child = row({
                .align_items = Align::Start,
                .gap = StyleValue::point(24.0f),
                .children = {
                    // Column A: Alignment Matrix
                    column({
                        .gap = StyleValue::point(10.0f),
                        .children = {
                            text({
                                .text = "ALIGNMENT ANCHOR",
                                .color = 0xFF06B6D4,
                                .font_size = 11.0f,
                                .font_weight = FontWeight::Bold,
                                .key = Key::string("lbl_align_title"),
                            }),
                            alignment_matrix,
                        },
                        .key = Key::string("ctrl_col_align"),
                    }),

                    // Column B: Clipping Mode
                    column({
                        .gap = StyleValue::point(10.0f),
                        .children = {
                            text({
                                .text = "CLIPPING BEHAVIOR",
                                .color = 0xFF8B5CF6,
                                .font_size = 11.0f,
                                .font_weight = FontWeight::Bold,
                                .key = Key::string("lbl_clip_title"),
                            }),
                            row({
                                .gap = StyleValue::point(8.0f),
                                .children = {
                                    makeClipButton(Clip::None, "none", "Clip::None"),
                                    makeClipButton(Clip::HardEdge, "hard", "Clip::HardEdge"),
                                    makeClipButton(Clip::AntiAlias, "aa", "Clip::AntiAlias"),
                                },
                                .key = Key::string("row_clip_btns"),
                            }),
                            text({
                                .text = "Clip::None allows child and touches to extend outside.\nClip::HardEdge and AntiAlias clip paint to container.",
                                .color = 0xFF64748B,
                                .font_size = 11.0f,
                                .key = Key::string("lbl_clip_hint"),
                            }),
                        },
                        .key = Key::string("ctrl_col_clip"),
                    }),

                    // Column C: Child Size Selector
                    column({
                        .gap = StyleValue::point(10.0f),
                        .children = {
                            text({
                                .text = "CHILD DIMENSIONS",
                                .color = 0xFF10B981,
                                .font_size = 11.0f,
                                .font_weight = FontWeight::Bold,
                                .key = Key::string("lbl_size_title"),
                            }),
                            row({
                                .gap = StyleValue::point(8.0f),
                                .children = {
                                    makeSizeButton(80.0f, "80", "80px (Underflow)"),
                                    makeSizeButton(160.0f, "160", "160px (Overflow)"),
                                    makeSizeButton(230.0f, "230", "230px (Extreme)"),
                                },
                                .key = Key::string("row_size_btns"),
                            }),
                            text({
                                .text = "Parent container is locked to exactly 120×120 px.",
                                .color = 0xFF64748B,
                                .font_size = 11.0f,
                                .key = Key::string("lbl_size_hint"),
                            }),
                        },
                        .key = Key::string("ctrl_col_size"),
                    }),
                },
                .key = Key::string("controls_row"),
            }),
            .key = Key::string("controls_card"),
        });

        // ── 3. Showcase Section 1: The Overflow Interactive Sandbox ───
        // Parent container: 120x120 with glowing cyan border
        auto sandbox_child_btn = gestureDetector({
            .key = Key::string("sandbox_click_btn"),
            .child = container({
                .color = 0xFF6D28D9,
                .border_radius = BorderRadius::circular(6.0f),
                .padding = StyleInsets::symmetric(4.0f, 8.0f),
                .child = text({
                    .text = "Click Me",
                    .color = 0xFFFFFFFF,
                    .font_size = 10.0f,
                    .font_weight = FontWeight::Bold,
                    .key = Key::string("sandbox_btn_txt"),
                }),
                .key = Key::string("sandbox_btn_box"),
            }),
            .cursor_type = SystemCursor::Pointer,
            .on_tap = [this]() {
                setState([this]() {
                    last_action_ = "CLICKED inside Overflowing Child! (Hit-Test Success)";
                });
            },
        });

        auto sandbox_child_content = column({
            .justify_content = Justify::Center,
            .align_items = Align::Center,
            .gap = StyleValue::point(4.0f),
            .children = {
                text({
                    .text = "CHILD",
                    .color = 0xFFFFFFFF,
                    .font_size = 12.0f,
                    .font_weight = FontWeight::Bold,
                    .key = Key::string("sandbox_child_lbl"),
                }),
                text({
                    .text = std::to_string(static_cast<int>(child_size_)) + "x" + std::to_string(static_cast<int>(child_size_)),
                    .color = 0xFFE9D5FF,
                    .font_size = 11.0f,
                    .key = Key::string("sandbox_child_dim"),
                }),
                sandbox_child_btn,
            },
            .key = Key::string("sandbox_child_col"),
        });

        auto sandbox_child_card = container({
            .color = 0xD98B5CF6,
            .border_radius = BorderRadius::circular(12.0f),
            .border = Border(0xFFC084FC, 2.0f),
            .width = StyleValue::point(child_size_),
            .height = StyleValue::point(child_size_),
            .padding = StyleInsets::all(10.0f),
            .child = sandbox_child_content,
            .key = Key::string("sandbox_child_card"),
        });

        auto sandbox_overflow = overflowBox({
            .key = Key::string("sandbox_ofb"),
            .alignment = alignment_,
            .min_width = child_size_,
            .max_width = child_size_,
            .min_height = child_size_,
            .max_height = child_size_,
            .clip_behavior = clip_mode_,
            .child = sandbox_child_card,
        });

        auto sandbox_parent = container({
            .color = 0x331E293B,
            .border_radius = BorderRadius::circular(10.0f),
            .border = Border(0xFF38BDF8, 2.0f),
            .width = StyleValue::point(120.0f),
            .height = StyleValue::point(120.0f),
            .child = sandbox_overflow,
            .key = Key::string("sandbox_parent_box"),
        });

        auto sandbox_card = container({
            .color = 0xFF0F172A,
            .border_radius = BorderRadius::circular(14.0f),
            .border = Border(0xFF1E293B, 1.0f),
            .padding = StyleInsets::all(24.0f),
            .flex = 1.0f,
            .flex_grow = 1.0f,
            .flex_shrink = 1.0f,
            .flex_basis = StyleValue::point(0.0f),
            .child = column({
                .gap = StyleValue::point(16.0f),
                .children = {
                    row({
                        .align_items = Align::Center,
                        .gap = StyleValue::point(10.0f),
                        .children = {
                            text({
                                .text = "1. Interactive Overflow Sandbox",
                                .color = 0xFFF8FAFC,
                                .font_size = 16.0f,
                                .font_weight = FontWeight::Bold,
                                .key = Key::string("sandbox_title"),
                            }),
                            container({
                                .color = 0xFF38BDF8,
                                .border_radius = BorderRadius::circular(4.0f),
                                .padding = StyleInsets::symmetric(2.0f, 6.0f),
                                .child = text({
                                    .text = "Cyan Box = Parent (120x120)",
                                    .color = 0xFF060911,
                                    .font_size = 10.0f,
                                    .font_weight = FontWeight::Bold,
                                    .key = Key::string("sandbox_badge_txt"),
                                }),
                                .key = Key::string("sandbox_badge"),
                            }),
                        },
                        .key = Key::string("sandbox_card_header"),
                    }),
                    text({
                        .text = "The Cyan outline is the strict 120x120 boundary of the parent. Notice how the purple child extends freely beyond it when Clip::None is active.",
                        .color = 0xFF94A3B8,
                        .font_size = 12.0f,
                        .key = Key::string("sandbox_desc"),
                    }),
                    container({
                        .color = 0xFF090D1A,
                        .border_radius = BorderRadius::circular(12.0f),
                        .border = Border(0xFF1E293B, 1.0f),
                        .height = StyleValue::point(260.0f),
                        .child = flexbox({
                            .justify_content = Justify::Center,
                            .align_items = Align::Center,
                            .children = { sandbox_parent },
                            .key = Key::string("sandbox_flex"),
                        }),
                        .key = Key::string("sandbox_viewport"),
                    }),
                },
                .key = Key::string("sandbox_card_col"),
            }),
            .key = Key::string("sandbox_card"),
        });

        // ── 4. Showcase Section 2: Real-World Overhanging Badges ───────
        auto avatar_circle = container({
            .color = 0xFF06B6D4,
            .border_radius = BorderRadius::circular(40.0f),
            .border = Border(0xFF0F172A, 4.0f),
            .shape = BoxShape::Circle,
            .width = StyleValue::point(80.0f),
            .height = StyleValue::point(80.0f),
            .child = flexbox({
                .justify_content = Justify::Center,
                .align_items = Align::Center,
                .children = {
                    text({
                        .text = "ENKI",
                        .color = 0xFF060911,
                        .font_size = 14.0f,
                        .font_weight = FontWeight::Bold,
                        .key = Key::string("avatar_txt"),
                    }),
                },
                .key = Key::string("avatar_flex"),
            }),
            .key = Key::string("avatar_circle"),
        });

        auto avatar_box = container({
            .width = StyleValue::point(56.0f),
            .height = StyleValue::point(56.0f),
            .child = overflowBox({
                .key = Key::string("avatar_ofb"),
                .alignment = Alignment::TopCenter,
                .min_width = 80.0f,
                .max_width = 80.0f,
                .min_height = 80.0f,
                .max_height = 80.0f,
                .child = avatar_circle,
            }),
            .key = Key::string("avatar_container_box"),
        });

        auto pill_gd = gestureDetector({
            .key = Key::string("pill_gd_btn"),
            .child = container({
                .color = 0xFFEF4444,
                .border_radius = BorderRadius::circular(18.0f),
                .border = Border(0xFFF87171, 1.0f),
                .padding = StyleInsets::symmetric(6.0f, 12.0f),
                .child = text({
                    .text = "● LIVE VIP (" + std::to_string(badge_click_count_) + ")",
                    .color = 0xFFFFFFFF,
                    .font_size = 11.0f,
                    .font_weight = FontWeight::Bold,
                    .key = Key::string("pill_btn_txt"),
                }),
                .key = Key::string("pill_btn_box"),
            }),
            .cursor_type = SystemCursor::Pointer,
            .on_tap = [this]() {
                setState([this]() {
                    badge_click_count_++;
                    last_action_ = "Clicked Overhanging Pill Badge! Total clicks: " + std::to_string(badge_click_count_);
                });
            },
        });

        auto pill_box = container({
            .width = StyleValue::point(30.0f),
            .height = StyleValue::point(30.0f),
            .child = overflowBox({
                .key = Key::string("pill_ofb"),
                .alignment = Alignment::TopRight,
                .min_width = 110.0f,
                .max_width = 110.0f,
                .min_height = 36.0f,
                .max_height = 36.0f,
                .child = pill_gd,
            }),
            .key = Key::string("pill_container_box"),
        });

        auto profile_top_row = row({
            .justify_content = Justify::SpaceBetween,
            .align_items = Align::Start,
            .children = {
                avatar_box,
                pill_box,
            },
            .key = Key::string("profile_top_row"),
        });

        auto profile_body = column({
            .gap = StyleValue::point(4.0f),
            .children = {
                text({
                    .text = "Alex Mercer",
                    .color = 0xFFF8FAFC,
                    .font_size = 16.0f,
                    .font_weight = FontWeight::Bold,
                    .key = Key::string("profile_name"),
                }),
                text({
                    .text = "Principal Systems Architect",
                    .color = 0xFF38BDF8,
                    .font_size = 12.0f,
                    .key = Key::string("profile_role"),
                }),
                text({
                    .text = "High-performance Skia & Anu layout engine developer.",
                    .color = 0xFF94A3B8,
                    .font_size = 11.0f,
                    .key = Key::string("profile_bio"),
                }),
            },
            .key = Key::string("profile_body_col"),
        });

        auto connect_btn = gestureDetector({
            .key = Key::string("connect_gd_btn"),
            .child = container({
                .color = 0xFF0F172A,
                .border_radius = BorderRadius::circular(8.0f),
                .border = Border(0xFF334155, 1.0f),
                .padding = StyleInsets::symmetric(8.0f, 12.0f),
                .child = flexbox({
                    .justify_content = Justify::Center,
                    .children = {
                        text({
                            .text = "Connect Network",
                            .color = 0xFFE2E8F0,
                            .font_size = 11.0f,
                            .font_weight = FontWeight::Bold,
                            .key = Key::string("connect_btn_txt"),
                        }),
                    },
                    .key = Key::string("connect_btn_flex"),
                }),
                .key = Key::string("connect_btn_box"),
            }),
            .cursor_type = SystemCursor::Pointer,
            .on_tap = [this]() {
                setState([this]() {
                    last_action_ = "Profile card 'Connect' clicked.";
                });
            },
        });

        auto profile_card = container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(16.0f),
            .border = Border(0xFF334155, 1.0f),
            .width = StyleValue::point(260.0f),
            .height = StyleValue::point(220.0f),
            .padding = StyleInsets::all(20.0f),
            .child = column({
                .justify_content = Justify::SpaceBetween,
                .children = {
                    profile_top_row,
                    profile_body,
                    connect_btn,
                },
                .key = Key::string("profile_col"),
            }),
            .key = Key::string("profile_card_box"),
        });

        auto usecases_card = container({
            .color = 0xFF0F172A,
            .border_radius = BorderRadius::circular(14.0f),
            .border = Border(0xFF1E293B, 1.0f),
            .padding = StyleInsets::all(24.0f),
            .flex = 1.0f,
            .flex_grow = 1.0f,
            .flex_shrink = 1.0f,
            .flex_basis = StyleValue::point(0.0f),
            .child = column({
                .gap = StyleValue::point(16.0f),
                .children = {
                    text({
                        .text = "2. Practical Overhanging UI Patterns",
                        .color = 0xFFF8FAFC,
                        .font_size = 16.0f,
                        .font_weight = FontWeight::Bold,
                        .key = Key::string("usecases_title"),
                    }),
                    text({
                        .text = "Real-world pattern: The circular avatar and the VIP notification button both overhang the card boundary while remaining 100% interactive.",
                        .color = 0xFF94A3B8,
                        .font_size = 12.0f,
                        .key = Key::string("usecases_desc"),
                    }),
                    container({
                        .color = 0xFF090D1A,
                        .border_radius = BorderRadius::circular(12.0f),
                        .border = Border(0xFF1E293B, 1.0f),
                        .height = StyleValue::point(260.0f),
                        .child = flexbox({
                            .justify_content = Justify::Center,
                            .align_items = Align::Center,
                            .children = { profile_card },
                            .key = Key::string("usecases_flex"),
                        }),
                        .key = Key::string("usecases_viewport"),
                    }),
                },
                .key = Key::string("usecases_col"),
            }),
            .key = Key::string("usecases_card"),
        });

        auto showcase_row = row({
            .gap = StyleValue::point(20.0f),
            .width = StyleValue::percent(100.0f),
            .children = {
                sandbox_card,
                usecases_card,
            },
            .key = Key::string("showcase_row"),
        });

        // ── 5. Telemetry & Live Status Card ───────────────────────────
        float dx = 120.0f - child_size_;
        float dy = 120.0f - child_size_;
        float rx = 0.5f, ry = 0.5f;
        if (alignment_ == Alignment::TopLeft || alignment_ == Alignment::CenterLeft || alignment_ == Alignment::BottomLeft) rx = 0.0f;
        if (alignment_ == Alignment::TopRight || alignment_ == Alignment::CenterRight || alignment_ == Alignment::BottomRight) rx = 1.0f;
        if (alignment_ == Alignment::TopLeft || alignment_ == Alignment::TopCenter || alignment_ == Alignment::TopRight) ry = 0.0f;
        if (alignment_ == Alignment::BottomLeft || alignment_ == Alignment::BottomCenter || alignment_ == Alignment::BottomRight) ry = 1.0f;

        float computed_offset_x = dx * rx;
        float computed_offset_y = dy * ry;

        auto reset_btn = gestureDetector({
            .key = Key::string("reset_gd_btn"),
            .child = container({
                .color = 0xFF1E293B,
                .border_radius = BorderRadius::circular(8.0f),
                .border = Border(0xFF334155, 1.0f),
                .padding = StyleInsets::symmetric(8.0f, 16.0f),
                .child = text({
                    .text = "Reset Defaults",
                    .color = 0xFF94A3B8,
                    .font_size = 12.0f,
                    .key = Key::string("reset_btn_txt"),
                }),
                .key = Key::string("reset_btn_box"),
            }),
            .cursor_type = SystemCursor::Pointer,
            .on_tap = [this]() {
                setState([this]() {
                    alignment_ = Alignment::Center;
                    clip_mode_ = Clip::None;
                    child_size_ = 160.0f;
                    badge_click_count_ = 0;
                    last_action_ = "Reset to factory defaults.";
                });
            },
        });

        auto telemetry_card = container({
            .color = 0xFF090E17,
            .border_radius = BorderRadius::circular(14.0f),
            .border = Border(0xFF1E293B, 1.0f),
            .width = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(16.0f),
            .child = row({
                .justify_content = Justify::SpaceBetween,
                .align_items = Align::Center,
                .children = {
                    column({
                        .gap = StyleValue::point(4.0f),
                        .children = {
                            text({
                                .text = "LIVE TELEMETRY & DIAGNOSTICS",
                                .color = 0xFF38BDF8,
                                .font_size = 11.0f,
                                .font_weight = FontWeight::Bold,
                                .key = Key::string("telemetry_title"),
                            }),
                            text({
                                .text = "Alignment: " + alignmentName(alignment_) + " | Mode: " + clipModeName(clip_mode_) +
                                     " | Child: " + std::to_string(static_cast<int>(child_size_)) + "px | Offset: (" +
                                     std::to_string(static_cast<int>(computed_offset_x)) + "px, " +
                                     std::to_string(static_cast<int>(computed_offset_y)) + "px)",
                                .color = 0xFFCBD5E1,
                                .font_size = 12.0f,
                                .key = Key::string("telemetry_details"),
                            }),
                            text({
                                .text = "Status: " + last_action_,
                                .color = 0xFF10B981,
                                .font_size = 12.0f,
                                .font_weight = FontWeight::Bold,
                                .key = Key::string("telemetry_status"),
                            }),
                        },
                        .key = Key::string("telemetry_col"),
                    }),
                    reset_btn,
                },
                .key = Key::string("telemetry_row"),
            }),
            .key = Key::string("telemetry_card"),
        });

        // ── 6. Assemble Main Layout ───────────────────────────────────
        auto main_column = column({
            .gap = StyleValue::point(20.0f),
            .width = StyleValue::percent(100.0f),
            .children = {
                header,
                controls_panel,
                showcase_row,
                telemetry_card,
            },
            .key = Key::string("main_column"),
        });

        return container({
            .color = 0xFF060911,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(24.0f),
            .child = scrollView(main_column),
            .key = Key::string("root_container"),
        });
    }
};

std::unique_ptr<State> OverflowBoxDemoApp::createState() {
    return std::make_unique<OverflowBoxDemoState>();
}

int main() {
    std::cout << "====================================================" << std::endl;
    std::cout << "  ENKI Engine — OverflowBox Studio Showcase Demo" << std::endl;
    std::cout << "  Roadmap v0.2.0 | Section 11 Layout — Extended" << std::endl;
    std::cout << "====================================================" << std::endl;

    AppConfig config;
    config.title       = "ENKI Engine — OverflowBox Showcase Demo";
    config.width       = 1180;
    config.height      = 860;
    config.target_fps  = 0;
    config.vsync       = false;
    config.show_performance_overlay = true;
    config.clear_color = 0xFF060911;

    return runApp(std::make_shared<OverflowBoxDemoApp>(), config);
}
