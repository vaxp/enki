/// @file main.cpp
/// @brief ENKI Engine — LimitedBox Studio Showcase Demo
/// @details Roadmap v0.2.0 | Section 11 Layout — Extended
/// High-performance demo with unique Key on every element for + FPS reconciliation.

#include "enki/app/app.hpp"
#include "enki/widgets/limited_box.hpp"
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

class LimitedBoxDemoApp : public StatefulWidget {
public:
    LimitedBoxDemoApp() = default;
    std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "LimitedBoxDemoApp"; }
};

class LimitedBoxDemoState : public State {
private:
    float active_max_height_ = 140.0f;
    float active_max_width_  = 220.0f;
    std::string last_action_ = "Ready. Switch max dimensions or scroll the feeds.";
    int card_click_count_ = 0;

    WidgetPtr makeHeightButton(float h, const std::string& key_id, const std::string& label) {
        bool is_selected = (active_max_height_ == h);
        auto btn_txt = text({
            .text = label,
            .color = is_selected ? 0xFF060911 : 0xFFE2E8F0,
            .font_size = 11.0f,
            .font_weight = is_selected ? FontWeight::Bold : FontWeight::Normal,
            .key = Key::string("txt_h_" + key_id),
        });

        auto btn_box = container({
            .color = is_selected ? 0xFF06B6D4 : 0xFF1E293B,
            .border_radius = BorderRadius::circular(8.0f),
            .border = Border(is_selected ? 0xFF22D3EE : 0xFF334155, 1.0f),
            .padding = StyleInsets::symmetric(8.0f, 12.0f),
            .child = btn_txt,
            .key = Key::string("box_h_" + key_id),
        });

        return gestureDetector({
            .key = Key::string("btn_h_" + key_id),
            .child = btn_box,
            .cursor_type = SystemCursor::Pointer,
            .on_tap = [this, h, label]() {
                setState([this, h, label]() {
                    active_max_height_ = h;
                    last_action_ = "Max height limit set to: " + label;
                });
            },
        });
    }

    WidgetPtr makeWidthButton(float w, const std::string& key_id, const std::string& label) {
        bool is_selected = (active_max_width_ == w);
        auto btn_txt = text({
            .text = label,
            .color = is_selected ? 0xFFFFFFFF : 0xFF94A3B8,
            .font_size = 11.0f,
            .font_weight = is_selected ? FontWeight::Bold : FontWeight::Normal,
            .key = Key::string("txt_w_" + key_id),
        });

        auto btn_box = container({
            .color = is_selected ? 0xFF8B5CF6 : 0xFF1E293B,
            .border_radius = BorderRadius::circular(8.0f),
            .border = Border(is_selected ? 0xFFA78BFA : 0xFF334155, 1.0f),
            .padding = StyleInsets::symmetric(8.0f, 12.0f),
            .child = btn_txt,
            .key = Key::string("box_w_" + key_id),
        });

        return gestureDetector({
            .key = Key::string("btn_w_" + key_id),
            .child = btn_box,
            .cursor_type = SystemCursor::Pointer,
            .on_tap = [this, w, label]() {
                setState([this, w, label]() {
                    active_max_width_ = w;
                    last_action_ = "Max width limit set to: " + label;
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
                                .text = "ENKI Engine — LimitedBox Studio Showcase",
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
                        .text = "Applies max-width and max-height ONLY when incoming constraints are unconstrained (e.g. inside ScrollViews). Has zero limiting effect in bounded containers.",
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
        auto controls_panel = container({
            .color = 0xFF0F172A,
            .border_radius = BorderRadius::circular(14.0f),
            .border = Border(0xFF1E293B, 1.0f),
            .width = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(20.0f),
            .child = row({
                .justify_content = Justify::SpaceBetween,
                .align_items = Align::Center,
                .children = {
                    // Control Group 1: Max Height Limits
                    column({
                        .gap = StyleValue::point(8.0f),
                        .children = {
                            text({
                                .text = "MAX HEIGHT LIMIT (FOR VERTICAL UNCONSTRAINED SCROLL)",
                                .color = 0xFF06B6D4,
                                .font_size = 11.0f,
                                .font_weight = FontWeight::Bold,
                                .key = Key::string("lbl_h_title"),
                            }),
                            row({
                                .gap = StyleValue::point(8.0f),
                                .children = {
                                    makeHeightButton(90.0f, "90", "90 px (Compact)"),
                                    makeHeightButton(140.0f, "140", "140 px (Standard)"),
                                    makeHeightButton(200.0f, "200", "200 px (Expanded)"),
                                },
                                .key = Key::string("row_h_btns"),
                            }),
                        },
                        .key = Key::string("col_ctrl_h"),
                    }),

                    // Control Group 2: Max Width Limits
                    column({
                        .gap = StyleValue::point(8.0f),
                        .children = {
                            text({
                                .text = "MAX WIDTH LIMIT (FOR HORIZONTAL UNCONSTRAINED SCROLL)",
                                .color = 0xFF8B5CF6,
                                .font_size = 11.0f,
                                .font_weight = FontWeight::Bold,
                                .key = Key::string("lbl_w_title"),
                            }),
                            row({
                                .gap = StyleValue::point(8.0f),
                                .children = {
                                    makeWidthButton(150.0f, "150", "150 px"),
                                    makeWidthButton(220.0f, "220", "220 px"),
                                    makeWidthButton(300.0f, "300", "300 px"),
                                },
                                .key = Key::string("row_w_btns"),
                            }),
                        },
                        .key = Key::string("col_ctrl_w"),
                    }),
                },
                .key = Key::string("controls_row"),
            }),
            .key = Key::string("controls_card"),
        });

        // ── 3. Showcase Section 1: Unconstrained Scroll Comparison ─────
        // Card 1A: Unconstrained WITHOUT LimitedBox (tries to take massive height 320px)
        auto card_unlimited_content = container({
            .color = 0xFF331E29,
            .border_radius = BorderRadius::circular(10.0f),
            .border = Border(0xFFEF4444, 1.5f),
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::point(320.0f),
            .padding = StyleInsets::all(14.0f),
            .child = column({
                .gap = StyleValue::point(6.0f),
                .children = {
                    text({
                        .text = "WITHOUT LIMITEDBOX",
                        .color = 0xFFF87171,
                        .font_size = 11.0f,
                        .font_weight = FontWeight::Bold,
                        .key = Key::string("txt_unlim_title"),
                    }),
                    text({
                        .text = "Height: 320 px (Full Unrestricted)",
                        .color = 0xFFFECACA,
                        .font_size = 13.0f,
                        .font_weight = FontWeight::Bold,
                        .key = Key::string("txt_unlim_h"),
                    }),
                    text({
                        .text = "Without LimitedBox, the child consumes excessive height in the unconstrained scroll axis, pushing other content off-screen.",
                        .color = 0xFF94A3B8,
                        .font_size = 11.0f,
                        .key = Key::string("txt_unlim_desc"),
                    }),
                },
                .key = Key::string("col_unlim"),
            }),
            .key = Key::string("box_unlim_content"),
        });

        auto feed_without_limitedbox = container({
            .color = 0xFF090D1A,
            .border_radius = BorderRadius::circular(12.0f),
            .border = Border(0xFF1E293B, 1.0f),
            .height = StyleValue::point(250.0f),
            .padding = StyleInsets::all(12.0f),
            .child = scrollView(
                ScrollOptions{ .direction = Axis::Vertical },
                column({
                    .gap = StyleValue::point(10.0f),
                    .children = {
                        card_unlimited_content,
                    },
                    .key = Key::string("col_feed_unlim"),
                })
            ),
            .key = Key::string("viewport_unlim"),
        });

        auto showcase_card_unlimited = container({
            .color = 0xFF0F172A,
            .border_radius = BorderRadius::circular(14.0f),
            .border = Border(0xFF1E293B, 1.0f),
            .padding = StyleInsets::all(20.0f),
            .flex = 1.0f,
            .flex_grow = 1.0f,
            .flex_shrink = 1.0f,
            .flex_basis = StyleValue::point(0.0f),
            .child = column({
                .gap = StyleValue::point(12.0f),
                .children = {
                    row({
                        .align_items = Align::Center,
                        .gap = StyleValue::point(8.0f),
                        .children = {
                            container({
                                .color = 0xFFEF4444,
                                .border_radius = BorderRadius::circular(4.0f),
                                .padding = StyleInsets::symmetric(2.0f, 6.0f),
                                .child = text({
                                    .text = "UNBOUNDED",
                                    .color = 0xFFFFFFFF,
                                    .font_size = 10.0f,
                                    .font_weight = FontWeight::Bold,
                                    .key = Key::string("badge_unlim_txt"),
                                }),
                                .key = Key::string("badge_unlim"),
                            }),
                            text({
                                .text = "1. Standard Unconstrained Feed",
                                .color = 0xFFF8FAFC,
                                .font_size = 15.0f,
                                .font_weight = FontWeight::Bold,
                                .key = Key::string("title_card_unlim"),
                            }),
                        },
                        .key = Key::string("row_head_unlim"),
                    }),
                    text({
                        .text = "Child requests 320px in an unconstrained vertical scroll view and takes all of it.",
                        .color = 0xFF94A3B8,
                        .font_size = 11.5f,
                        .key = Key::string("desc_card_unlim"),
                    }),
                    feed_without_limitedbox,
                },
                .key = Key::string("col_card_unlim"),
            }),
            .key = Key::string("card_unlim"),
        });

        // Card 1B: With LimitedBox (height clamped to active_max_height_)
        auto card_limited_item = [this](int idx, Color accent) {
            std::string id_str = std::to_string(idx);
            return limitedBox({
                .key = Key::string("lbox_item_" + id_str),
                .max_height = active_max_height_,
                .child = container({
                    .color = 0xFF172033,
                    .border_radius = BorderRadius::circular(10.0f),
                    .border = Border(accent, 1.5f),
                    .width = StyleValue::percent(100.0f),
                    .height = StyleValue::point(320.0f), // would be 320px, but LimitedBox clamps it!
                    .padding = StyleInsets::all(12.0f),
                    .child = column({
                        .justify_content = Justify::SpaceBetween,
                        .children = {
                            row({
                                .justify_content = Justify::SpaceBetween,
                                .align_items = Align::Center,
                                .children = {
                                    text({
                                        .text = "LIMITED ITEM #" + id_str,
                                        .color = accent,
                                        .font_size = 11.0f,
                                        .font_weight = FontWeight::Bold,
                                        .key = Key::string("txt_lim_title_" + id_str),
                                    }),
                                    container({
                                        .color = accent,
                                        .border_radius = BorderRadius::circular(4.0f),
                                        .padding = StyleInsets::symmetric(2.0f, 6.0f),
                                        .child = text({
                                            .text = "Clamped: " + std::to_string(static_cast<int>(active_max_height_)) + "px",
                                            .color = 0xFF060911,
                                            .font_size = 10.0f,
                                            .font_weight = FontWeight::Bold,
                                            .key = Key::string("txt_lim_clamp_" + id_str),
                                        }),
                                        .key = Key::string("badge_lim_clamp_" + id_str),
                                    }),
                                },
                                .key = Key::string("row_lim_head_" + id_str),
                            }),
                            text({
                                .text = "Child requests 320px height, but LimitedBox clamps it to exactly " +
                                        std::to_string(static_cast<int>(active_max_height_)) + "px because the parent scroll is unconstrained.",
                                .color = 0xFFCBD5E1,
                                .font_size = 11.0f,
                                .key = Key::string("txt_lim_body_" + id_str),
                            }),
                            gestureDetector({
                                .key = Key::string("gd_lim_click_" + id_str),
                                .child = container({
                                    .color = 0xFF0F172A,
                                    .border_radius = BorderRadius::circular(6.0f),
                                    .border = Border(0xFF334155, 1.0f),
                                    .padding = StyleInsets::symmetric(4.0f, 10.0f),
                                    .child = text({
                                        .text = "Interactive Click (" + std::to_string(card_click_count_) + ")",
                                        .color = 0xFF38BDF8,
                                        .font_size = 10.0f,
                                        .font_weight = FontWeight::Bold,
                                        .key = Key::string("txt_lim_btn_" + id_str),
                                    }),
                                    .key = Key::string("box_lim_btn_" + id_str),
                                }),
                                .cursor_type = SystemCursor::Pointer,
                                .on_tap = [this, idx]() {
                                    setState([this, idx]() {
                                        card_click_count_++;
                                        last_action_ = "Clicked LimitedBox Item #" + std::to_string(idx) + "! Hit-testing works flawlessly.";
                                    });
                                },
                            }),
                        },
                        .key = Key::string("col_lim_inner_" + id_str),
                    }),
                    .key = Key::string("box_lim_inner_" + id_str),
                }),
            });
        };

        auto feed_with_limitedbox = container({
            .color = 0xFF090D1A,
            .border_radius = BorderRadius::circular(12.0f),
            .border = Border(0xFF1E293B, 1.0f),
            .height = StyleValue::point(250.0f),
            .padding = StyleInsets::all(12.0f),
            .child = scrollView(
                ScrollOptions{ .direction = Axis::Vertical },
                column({
                    .gap = StyleValue::point(10.0f),
                    .children = {
                        card_limited_item(1, 0xFF06B6D4),
                        card_limited_item(2, 0xFF10B981),
                        card_limited_item(3, 0xFF8B5CF6),
                    },
                    .key = Key::string("col_feed_lim"),
                })
            ),
            .key = Key::string("viewport_lim"),
        });

        auto showcase_card_limited = container({
            .color = 0xFF0F172A,
            .border_radius = BorderRadius::circular(14.0f),
            .border = Border(0xFF1E293B, 1.0f),
            .padding = StyleInsets::all(20.0f),
            .flex = 1.0f,
            .flex_grow = 1.0f,
            .flex_shrink = 1.0f,
            .flex_basis = StyleValue::point(0.0f),
            .child = column({
                .gap = StyleValue::point(12.0f),
                .children = {
                    row({
                        .align_items = Align::Center,
                        .gap = StyleValue::point(8.0f),
                        .children = {
                            container({
                                .color = 0xFF10B981,
                                .border_radius = BorderRadius::circular(4.0f),
                                .padding = StyleInsets::symmetric(2.0f, 6.0f),
                                .child = text({
                                    .text = "LIMITED",
                                    .color = 0xFFFFFFFF,
                                    .font_size = 10.0f,
                                    .font_weight = FontWeight::Bold,
                                    .key = Key::string("badge_lim_txt"),
                                }),
                                .key = Key::string("badge_lim"),
                            }),
                            text({
                                .text = "2. Protected by LimitedBox",
                                .color = 0xFFF8FAFC,
                                .font_size = 15.0f,
                                .font_weight = FontWeight::Bold,
                                .key = Key::string("title_card_lim"),
                            }),
                        },
                        .key = Key::string("row_head_lim"),
                    }),
                    text({
                        .text = "Same 320px child inside LimitedBox: cleanly clamped to " +
                                std::to_string(static_cast<int>(active_max_height_)) + "px. Multiple items fit smoothly.",
                        .color = 0xFF94A3B8,
                        .font_size = 11.5f,
                        .key = Key::string("desc_card_lim"),
                    }),
                    feed_with_limitedbox,
                },
                .key = Key::string("col_card_lim"),
            }),
            .key = Key::string("card_lim"),
        });

        // ── 4. Showcase Section 2: Bounded Parent Demonstration ────────
        // Same LimitedBox placed inside a bounded 280x160 container.
        // Even though LimitedBox has maxHeight=140px, because parent is strictly 180px high,
        // LimitedBox MUST NOT limit the child! Child fills 100% of the 180px container!
        auto bounded_box_child = limitedBox({
            .key = Key::string("bounded_demo_lbox"),
            .max_width = 120.0f,
            .max_height = 90.0f, // smaller than parent 180px, but will be ignored!
            .child = container({
                .color = 0xFF1E293B,
                .border_radius = BorderRadius::circular(10.0f),
                .border = Border(0xFF38BDF8, 1.5f),
                .width = StyleValue::percent(100.0f),
                .height = StyleValue::percent(100.0f),
                .padding = StyleInsets::all(16.0f),
                .child = column({
                    .justify_content = Justify::SpaceBetween,
                    .children = {
                        row({
                            .justify_content = Justify::SpaceBetween,
                            .align_items = Align::Center,
                            .children = {
                                text({
                                    .text = "BOUNDED PARENT TEST",
                                    .color = 0xFF38BDF8,
                                    .font_size = 11.0f,
                                    .font_weight = FontWeight::Bold,
                                    .key = Key::string("txt_bnd_title"),
                                }),
                                container({
                                    .color = 0xFF0284C7,
                                    .border_radius = BorderRadius::circular(4.0f),
                                    .padding = StyleInsets::symmetric(2.0f, 6.0f),
                                    .child = text({
                                        .text = "Ignored Limits",
                                        .color = 0xFFFFFFFF,
                                        .font_size = 10.0f,
                                        .font_weight = FontWeight::Bold,
                                        .key = Key::string("txt_bnd_ign_badge"),
                                    }),
                                    .key = Key::string("box_bnd_ign_badge"),
                                }),
                            },
                            .key = Key::string("row_bnd_head"),
                        }),
                        text({
                            .text = "This card sits inside a container with fixed 180px height. LimitedBox has max_height=90px, but because the parent is BOUNDED, LimitedBox does not interfere and allows 100% full height.",
                            .color = 0xFFCBD5E1,
                            .font_size = 11.5f,
                            .key = Key::string("txt_bnd_desc"),
                        }),
                        text({
                            .text = "Actual Height: 180px (Matches Bounded Parent 100%)",
                            .color = 0xFF10B981,
                            .font_size = 11.0f,
                            .font_weight = FontWeight::Bold,
                            .key = Key::string("txt_bnd_actual"),
                        }),
                    },
                    .key = Key::string("col_bnd_inner"),
                }),
                .key = Key::string("box_bnd_inner"),
            }),
        });

        auto showcase_card_bounded = container({
            .color = 0xFF0F172A,
            .border_radius = BorderRadius::circular(14.0f),
            .border = Border(0xFF1E293B, 1.0f),
            .padding = StyleInsets::all(20.0f),
            .flex = 1.0f,
            .flex_grow = 1.0f,
            .flex_shrink = 1.0f,
            .flex_basis = StyleValue::point(0.0f),
            .child = column({
                .gap = StyleValue::point(12.0f),
                .children = {
                    row({
                        .align_items = Align::Center,
                        .gap = StyleValue::point(8.0f),
                        .children = {
                            container({
                                .color = 0xFF0284C7,
                                .border_radius = BorderRadius::circular(4.0f),
                                .padding = StyleInsets::symmetric(2.0f, 6.0f),
                                .child = text({
                                    .text = "BOUNDED",
                                    .color = 0xFFFFFFFF,
                                    .font_size = 10.0f,
                                    .font_weight = FontWeight::Bold,
                                    .key = Key::string("badge_bnd_txt"),
                                }),
                                .key = Key::string("badge_bnd"),
                            }),
                            text({
                                .text = "3. Zero Effect in Bounded Containers",
                                .color = 0xFFF8FAFC,
                                .font_size = 15.0f,
                                .font_weight = FontWeight::Bold,
                                .key = Key::string("title_card_bnd"),
                            }),
                        },
                        .key = Key::string("row_head_bnd"),
                    }),
                    text({
                        .text = "Proving the core contract: LimitedBox is dormant unless incoming constraints are unconstrained.",
                        .color = 0xFF94A3B8,
                        .font_size = 11.5f,
                        .key = Key::string("desc_card_bnd"),
                    }),
                    container({
                        .color = 0xFF090D1A,
                        .border_radius = BorderRadius::circular(12.0f),
                        .border = Border(0xFF1E293B, 1.0f),
                        .height = StyleValue::point(250.0f),
                        .padding = StyleInsets::all(12.0f),
                        .child = container({
                            .color = 0x221E293B,
                            .border_radius = BorderRadius::circular(10.0f),
                            .border = Border(0xFF334155, 1.0f),
                            .width = StyleValue::percent(100.0f),
                            .height = StyleValue::point(180.0f),
                            .child = bounded_box_child,
                            .key = Key::string("box_bnd_fixed_parent"),
                        }),
                        .key = Key::string("viewport_bnd"),
                    }),
                },
                .key = Key::string("col_card_bnd"),
            }),
            .key = Key::string("card_bnd"),
        });

        auto comparison_row = row({
            .gap = StyleValue::point(16.0f),
            .width = StyleValue::percent(100.0f),
            .children = {
                showcase_card_unlimited,
                showcase_card_limited,
                showcase_card_bounded,
            },
            .key = Key::string("comparison_row"),
        });

        // ── 5. Telemetry & Live Status Card ───────────────────────────
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
                    active_max_height_ = 140.0f;
                    active_max_width_ = 220.0f;
                    card_click_count_ = 0;
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
                                .text = "Active Max Height: " + std::to_string(static_cast<int>(active_max_height_)) + "px | " +
                                       "Active Max Width: " + std::to_string(static_cast<int>(active_max_width_)) + "px | " +
                                       "Interactions: " + std::to_string(card_click_count_),
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
                comparison_row,
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

std::unique_ptr<State> LimitedBoxDemoApp::createState() {
    return std::make_unique<LimitedBoxDemoState>();
}

int main() {
    std::cout << "====================================================" << std::endl;
    std::cout << "  ENKI Engine — LimitedBox Studio Showcase Demo" << std::endl;
    std::cout << "  Roadmap v0.2.0 | Section 11 Layout — Extended" << std::endl;
    std::cout << "====================================================" << std::endl;

    AppConfig config;
    config.title       = "ENKI Engine — LimitedBox Showcase Demo";
    config.width       = 1240;
    config.height      = 860;
    config.target_fps  = 0;
    config.vsync       = false;
    config.show_performance_overlay = true;
    config.clear_color = 0xFF060911;

    return runApp(std::make_shared<LimitedBoxDemoApp>(), config);
}
