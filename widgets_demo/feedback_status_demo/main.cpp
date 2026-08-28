/// @file main.cpp
/// @brief Interactive Showcase Demo for ENKI Section 18: Feedback & Status Extended.
///
/// Features:
///   1. Skeleton — Shimmer loading placeholder vs loaded state card
///   2. Ripple — Material ink-ripple interactive click cards
///   3. Pulse — Live status beacon radar animations (Online / Busy / Live)
///   4. CountBadge — Interactive live spring/scale counter with overflow ("99+")

#include "enki/core/types.hpp"
#include "enki/app/app.hpp"
#include "enki/widgets/feedback_status.hpp"
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

class FeedbackStatusDemoApp : public StatefulWidget {
public:
    FeedbackStatusDemoApp() = default;

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "FeedbackStatusDemoApp"; }
};

class FeedbackStatusDemoState : public State {
    bool is_loading_ = true;
    int  notif_count_ = 3;
    int  cart_count_ = 12;
    int  inbox_count_ = 98;
    int  ripple_click_count_ = 0;

public:
    WidgetPtr build(BuildContext&) override {
        // Top Header
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
                            pulse({
                                .color = 0xFF38BDF8,
                                .ring_count = 2,
                                .max_radius = 16.0f,
                                .dot_radius = 5.0f,
                            }),
                            text("ENKI Section 18: Feedback & Status Extended", {
                                .color = 0xFFF8FAFC,
                                .font_size = 20.0f,
                                .font_weight = FontWeight::Bold,
                            }),
                        },
                    }),
                    text("Declarative C++20 Widgets: Skeleton, Ripple, Pulse, CountBadge (60+ FPS Shaders)", {
                        .color = 0xFF94A3B8,
                        .font_size = 13.0f,
                    }),
                },
            }),
        });

        // ─────────────────────────────────────────────────────────────
        // Section 1: Skeleton Shimmer Loading
        // ─────────────────────────────────────────────────────────────
        auto skeleton_card = is_loading_ ?
            // Loading State (Shimmer Skeleton)
            container({
                .color = 0xFF0F172A,
                .border_radius = BorderRadius::circular(12.0f),
                .border = Border(0xFF334155, 1.0f),
                .width = StyleValue::percent(100.0f),
                .padding = StyleInsets::all(16.0f),
                .child = row({
                    .align_items = Align::Center,
                    .gap = StyleValue::point(16.0f),
                    .children = {
                        skeletonCircle(56.0f, 0xFF1E293B, 0xFF475569),
                        expanded(column({
                            .gap = StyleValue::point(10.0f),
                            .children = {
                                skeletonRect(180.0f, 16.0f, 4.0f, 0xFF1E293B, 0xFF475569),
                                skeletonRect(280.0f, 12.0f, 4.0f, 0xFF1E293B, 0xFF475569),
                                row({
                                    .gap = StyleValue::point(8.0f),
                                    .children = {
                                        skeletonRect(70.0f, 22.0f, 6.0f, 0xFF1E293B, 0xFF475569),
                                        skeletonRect(90.0f, 22.0f, 6.0f, 0xFF1E293B, 0xFF475569),
                                    },
                                }),
                            },
                        })),
                    },
                }),
            }) :
            // Loaded State (Real Content)
            container({
                .color = 0xFF0F172A,
                .border_radius = BorderRadius::circular(12.0f),
                .border = Border(0xFF10B981, 1.5f),
                .width = StyleValue::percent(100.0f),
                .padding = StyleInsets::all(16.0f),
                .child = row({
                    .align_items = Align::Center,
                    .gap = StyleValue::point(16.0f),
                    .children = {
                        container({
                            .color = 0xFF0284C7,
                            .border_radius = BorderRadius::circular(28.0f),
                            .border = Border(0xFF38BDF8, 2.0f),
                            .align = Alignment::Center,
                            .width = StyleValue::point(56.0f),
                            .height = StyleValue::point(56.0f),
                            .child = text("🚀", {.font_size = 24.0f}),
                        }),
                        expanded(column({
                            .gap = StyleValue::point(6.0f),
                            .children = {
                                row({
                                    .align_items = Align::Center,
                                    .gap = StyleValue::point(8.0f),
                                    .children = {
                                        text("Alex Mercer — Core Engineer", {
                                            .color = 0xFFF8FAFC,
                                            .font_size = 15.0f,
                                            .font_weight = FontWeight::Bold,
                                        }),
                                        container({
                                            .color = 0xFF065F46,
                                            .border_radius = BorderRadius::circular(6.0f),
                                            .padding = StyleInsets::symmetric(StyleValue::point(2.0f), StyleValue::point(6.0f)),
                                            .child = text("VERIFIED", {
                                                .color = 0xFF34D399,
                                                .font_size = 10.0f,
                                                .font_weight = FontWeight::Bold,
                                            }),
                                        }),
                                    },
                                }),
                                text("Lead Systems Architect @ ENKI High-Performance Engine", {
                                    .color = 0xFF94A3B8,
                                    .font_size = 13.0f,
                                }),
                                row({
                                    .gap = StyleValue::point(8.0f),
                                    .children = {
                                        container({
                                            .color = 0xFF1E293B,
                                            .border_radius = BorderRadius::circular(6.0f),
                                            .padding = StyleInsets::symmetric(StyleValue::point(3.0f), StyleValue::point(8.0f)),
                                            .child = text("C++20", {.color = 0xFF60A5FA, .font_size = 11.0f}),
                                        }),
                                        container({
                                            .color = 0xFF1E293B,
                                            .border_radius = BorderRadius::circular(6.0f),
                                            .padding = StyleInsets::symmetric(StyleValue::point(3.0f), StyleValue::point(8.0f)),
                                            .child = text("Skia 60 FPS", {.color = 0xFFA78BFA, .font_size = 11.0f}),
                                        }),
                                    },
                                }),
                            },
                        })),
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
                                    text("1. Skeleton (Shimmer Loading Placeholder)", {
                                        .color = 0xFF38BDF8,
                                        .font_size = 15.0f,
                                        .font_weight = FontWeight::Bold,
                                    }),
                                    text(is_loading_ ? "[State: SHIMMER LOADING]" : "[State: LOADED CONTENT]", {
                                        .color = is_loading_ ? 0xFFFBBF24 : 0xFF34D399,
                                        .font_size = 12.0f,
                                        .font_weight = FontWeight::Medium,
                                    }),
                                },
                            }),
                            button(text(is_loading_ ? "Show Loaded Profile" : "Simulate Shimmer", {.color = 0xFFFFFFFF}), [this]() {
                                setState([this]() { is_loading_ = !is_loading_; });
                            }, { .normal_color = is_loading_ ? 0xFF059669 : 0xFFD97706, .border_radius = 8.0f }),
                        },
                    }),
                    skeleton_card,
                },
            }),
        });

        // ─────────────────────────────────────────────────────────────
        // Section 2: Material Ink Ripple
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
                            text("2. Ripple (Material Ink Waves on Click / Touch)", {
                                .color = 0xFF818CF8,
                                .font_size = 15.0f,
                                .font_weight = FontWeight::Bold,
                            }),
                            text("Total Clicks: " + std::to_string(ripple_click_count_), {
                                .color = 0xFFA5B4FC,
                                .font_size = 13.0f,
                            }),
                        },
                    }),
                    row({
                        .gap = StyleValue::point(12.0f),
                        .children = {
                            expanded(ripple({
                                .child = container({
                                    .color = 0xFF0284C7,
                                    .border_radius = BorderRadius::circular(10.0f),
                                    .align = Alignment::Center,
                                    .height = StyleValue::point(64.0f),
                                    .child = text("Tap Me (Cyan Ripple)", {
                                        .color = 0xFFFFFFFF,
                                        .font_size = 13.0f,
                                        .font_weight = FontWeight::Bold,
                                    }),
                                }),
                                .color = 0x55FFFFFF,
                                .border_radius = BorderRadius::circular(10.0f),
                                .on_tap = [this]() {
                                    setState([this]() { ripple_click_count_++; });
                                },
                            })),
                            expanded(ripple({
                                .child = container({
                                    .color = 0xFF059669,
                                    .border_radius = BorderRadius::circular(10.0f),
                                    .align = Alignment::Center,
                                    .height = StyleValue::point(64.0f),
                                    .child = text("Tap Me (Emerald Ripple)", {
                                        .color = 0xFFFFFFFF,
                                        .font_size = 13.0f,
                                        .font_weight = FontWeight::Bold,
                                    }),
                                }),
                                .color = 0x55FFFFFF,
                                .border_radius = BorderRadius::circular(10.0f),
                                .on_tap = [this]() {
                                    setState([this]() { ripple_click_count_++; });
                                },
                            })),
                            expanded(ripple({
                                .child = container({
                                    .color = 0xFF7C3AED,
                                    .border_radius = BorderRadius::circular(10.0f),
                                    .align = Alignment::Center,
                                    .height = StyleValue::point(64.0f),
                                    .child = text("Tap Me (Violet Ripple)", {
                                        .color = 0xFFFFFFFF,
                                        .font_size = 13.0f,
                                        .font_weight = FontWeight::Bold,
                                    }),
                                }),
                                .color = 0x55FFFFFF,
                                .border_radius = BorderRadius::circular(10.0f),
                                .on_tap = [this]() {
                                    setState([this]() { ripple_click_count_++; });
                                },
                            })),
                        },
                    }),
                },
            }),
        });

        // ─────────────────────────────────────────────────────────────
        // Section 3: Pulse Status Indicators
        // ─────────────────────────────────────────────────────────────
        auto section3 = container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(12.0f),
            .border = Border(0xFF334155, 1.0f),
            .width = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(16.0f),
            .child = column({
                .gap = StyleValue::point(14.0f),
                .children = {
                    text("3. Pulse (Concentric Radar / Live Beacon Indicators)", {
                        .color = 0xFF34D399,
                        .font_size = 15.0f,
                        .font_weight = FontWeight::Bold,
                    }),
                    row({
                        .justify_content = Justify::SpaceBetween,
                        .children = {
                            // Online Badge
                            container({
                                .color = 0xFF0F172A,
                                .border_radius = BorderRadius::circular(10.0f),
                                .border = Border(0xFF334155, 1.0f),
                                .padding = StyleInsets::symmetric(StyleValue::point(10.0f), StyleValue::point(14.0f)),
                                .child = row({
                                    .align_items = Align::Center,
                                    .gap = StyleValue::point(10.0f),
                                    .children = {
                                        pulse({
                                            .color = 0xFF10B981,
                                            .ring_count = 3,
                                            .max_radius = 18.0f,
                                            .dot_radius = 5.0f,
                                        }),
                                        column({
                                            .gap = StyleValue::point(2.0f),
                                            .children = {
                                                text("Operational", {.color = 0xFF34D399, .font_size = 13.0f, .font_weight = FontWeight::Bold}),
                                                text("All nodes healthy", {.color = 0xFF64748B, .font_size = 11.0f}),
                                            },
                                        }),
                                    },
                                }),
                            }),
                            // Recording Badge
                            container({
                                .color = 0xFF0F172A,
                                .border_radius = BorderRadius::circular(10.0f),
                                .border = Border(0xFF334155, 1.0f),
                                .padding = StyleInsets::symmetric(StyleValue::point(10.0f), StyleValue::point(14.0f)),
                                .child = row({
                                    .align_items = Align::Center,
                                    .gap = StyleValue::point(10.0f),
                                    .children = {
                                        pulse({
                                            .color = 0xFFEF4444,
                                            .ring_count = 2,
                                            .max_radius = 18.0f,
                                            .dot_radius = 5.0f,
                                            .duration = std::chrono::milliseconds(1000),
                                        }),
                                        column({
                                            .gap = StyleValue::point(2.0f),
                                            .children = {
                                                text("Recording Live", {.color = 0xFFF87171, .font_size = 13.0f, .font_weight = FontWeight::Bold}),
                                                text("4K Stream 60 FPS", {.color = 0xFF64748B, .font_size = 11.0f}),
                                            },
                                        }),
                                    },
                                }),
                            }),
                            // High Load Badge
                            container({
                                .color = 0xFF0F172A,
                                .border_radius = BorderRadius::circular(10.0f),
                                .border = Border(0xFF334155, 1.0f),
                                .padding = StyleInsets::symmetric(StyleValue::point(10.0f), StyleValue::point(14.0f)),
                                .child = row({
                                    .align_items = Align::Center,
                                    .gap = StyleValue::point(10.0f),
                                    .children = {
                                        pulse({
                                            .color = 0xFFF59E0B,
                                            .ring_count = 2,
                                            .max_radius = 18.0f,
                                            .dot_radius = 5.0f,
                                        }),
                                        column({
                                            .gap = StyleValue::point(2.0f),
                                            .children = {
                                                text("High Load", {.color = 0xFFFBBF24, .font_size = 13.0f, .font_weight = FontWeight::Bold}),
                                                text("Auto-scaling active", {.color = 0xFF64748B, .font_size = 11.0f}),
                                            },
                                        }),
                                    },
                                }),
                            }),
                        },
                    }),
                },
            }),
        });

        // ─────────────────────────────────────────────────────────────
        // Section 4: Animated CountBadge
        // ─────────────────────────────────────────────────────────────
        auto section4 = container({
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
                            text("4. CountBadge (Spring / Pop Animation & 99+ Overflow)", {
                                .color = 0xFFF43F5E,
                                .font_size = 15.0f,
                                .font_weight = FontWeight::Bold,
                            }),
                            row({
                                .gap = StyleValue::point(6.0f),
                                .children = {
                                    button(text("+1", {.color = 0xFFFFFFFF}), [this]() {
                                        setState([this]() {
                                            notif_count_++;
                                            inbox_count_++;
                                        });
                                    }, { .normal_color = 0xFF2563EB, .border_radius = 6.0f }),
                                    button(text("-1", {.color = 0xFFFFFFFF}), [this]() {
                                        setState([this]() {
                                            if (notif_count_ > 0) notif_count_--;
                                            if (inbox_count_ > 0) inbox_count_--;
                                        });
                                    }, { .normal_color = 0xFF475569, .border_radius = 6.0f }),
                                    button(text("+50 (Overflow)", {.color = 0xFFFFFFFF}), [this]() {
                                        setState([this]() {
                                            inbox_count_ += 50;
                                        });
                                    }, { .normal_color = 0xFFE11D48, .border_radius = 6.0f }),
                                    button(text("Reset", {.color = 0xFFFFFFFF}), [this]() {
                                        setState([this]() {
                                            notif_count_ = 0;
                                            inbox_count_ = 0;
                                        });
                                    }, { .normal_color = 0xFF334155, .border_radius = 6.0f }),
                                },
                            }),
                        },
                    }),
                    row({
                        .justify_content = Justify::SpaceAround,
                        .align_items = Align::Center,
                        .children = {
                            // Notifications Badge
                            countBadge({
                                .child = container({
                                    .color = 0xFF0F172A,
                                    .border_radius = BorderRadius::circular(10.0f),
                                    .border = Border(0xFF334155, 1.0f),
                                    .align = Alignment::Center,
                                    .width = StyleValue::point(120.0f),
                                    .height = StyleValue::point(48.0f),
                                    .child = text("🔔 Alerts", {.color = 0xFFF8FAFC, .font_weight = FontWeight::Bold}),
                                }),
                                .count = notif_count_,
                                .max_count = 99,
                                .bg_color = 0xFFEF4444,
                            }),
                            // Shopping Cart Badge
                            countBadge({
                                .child = container({
                                    .color = 0xFF0F172A,
                                    .border_radius = BorderRadius::circular(10.0f),
                                    .border = Border(0xFF334155, 1.0f),
                                    .align = Alignment::Center,
                                    .width = StyleValue::point(120.0f),
                                    .height = StyleValue::point(48.0f),
                                    .child = text("🛒 Cart", {.color = 0xFFF8FAFC, .font_weight = FontWeight::Bold}),
                                }),
                                .count = cart_count_,
                                .max_count = 99,
                                .bg_color = 0xFF0284C7,
                            }),
                            // Messages Inbox Badge (Testing Overflow)
                            countBadge({
                                .child = container({
                                    .color = 0xFF0F172A,
                                    .border_radius = BorderRadius::circular(10.0f),
                                    .border = Border(0xFF334155, 1.0f),
                                    .align = Alignment::Center,
                                    .width = StyleValue::point(140.0f),
                                    .height = StyleValue::point(48.0f),
                                    .child = text("✉️ Inbox (" + std::to_string(inbox_count_) + ")", {.color = 0xFFF8FAFC, .font_weight = FontWeight::Bold}),
                                }),
                                .count = inbox_count_,
                                .max_count = 99,
                                .bg_color = 0xFF9333EA,
                            }),
                        },
                    }),
                },
            }),
        });

        // Main Layout Scroll View
        auto main_col = column({
            .align_items = Align::Center,
            .gap = StyleValue::point(16.0f),
            .width = StyleValue::percent(100.0f),
            .children = {
                header,
                section1,
                section2,
                section3,
                section4,
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

std::unique_ptr<State> FeedbackStatusDemoApp::createState() {
    return std::make_unique<FeedbackStatusDemoState>();
}

int main() {
    AppConfig config;
    config.title       = "ENKI Engine — Feedback & Status Suite (Section 18)";
    config.width       = 960;
    config.height      = 840;
    config.target_fps  = 0;
    config.vsync       = false;
    config.show_performance_overlay = false;
    config.clear_color = 0xFF0B0F17;
    return runApp(std::make_shared<FeedbackStatusDemoApp>(), config);
}
