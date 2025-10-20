/// @file main.cpp
/// @brief Extended Scrolling & Slivers Visual Showcase for ENKI Framework.
/// Demonstrates CustomScrollView, SliverAppBar, SliverGrid, SliverList, SliverToBoxAdapter, and SliverPadding.

#include "enki/app/app.hpp"
#include "enki/widgets/sliver.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/divider.hpp"
#include "enki/state/state.hpp"
#include <iostream>
#include <string>
#include <vector>

using namespace enki;

static const Color kThemeGradients[] = {
    0xFF3B82F6, 0xFF8B5CF6, 0xFFEC4899, 0xFF10B981, 0xFFF59E0B, 0xFF06B6D4
};

struct ShowcaseCardData {
    std::string title;
    std::string category;
    std::string metric;
    Color accent;
};

static const std::vector<ShowcaseCardData> kGridData = {
    {"CustomScrollView", "Viewport", "100%", 0xFF3B82F6},
    {"SliverAppBar", "Collapsible", "60 FPS", 0xFF8B5CF6},
    {"SliverGrid", "2D Layout", "Responsive", 0xFFEC4899},
    {"SliverList", "Lazy 1D", "Virtual", 0xFF10B981},
    {"SliverToBox", "Adapter", "Universal", 0xFFF59E0B},
    {"SliverPadding", "Spacing", "Adaptive", 0xFF06B6D4},
};

static WidgetPtr pillBadge(const std::string& label, Color bg, Color fg) {
    return container({
        .color = bg,
        .border_radius = BorderRadius::circular(12.0f),
        .padding = StyleInsets::symmetric(3.0f, 8.0f),
        .child = text(label, {
            .color = fg,
            .font_size = 11.0f,
            .font_weight = FontWeight::Bold,
        }),
    });
}

class SliverDemoState : public State {
    int active_filter_ = 0; // 0=All, 1=Grid, 2=List
    bool pinned_bar_ = true;

    WidgetPtr filterButton(const std::string& label, int idx) {
        bool active = (active_filter_ == idx);
        auto lbl = text(label, {
            .color = active ? 0xFFFFFFFF : 0xFF94A3B8,
            .font_size = 13.0f,
            .font_weight = active ? FontWeight::Bold : FontWeight::Normal,
        });

        ButtonProps opts;
        opts.normal_color  = active ? 0xFF6366F1 : 0xFF1E293B;
        opts.hover_color   = active ? 0xFF4F46E5 : 0xFF334155;
        opts.pressed_color = active ? 0xFF4338CA : 0xFF0F172A;
        opts.border_radius = 20.0f;
        opts.padding       = EdgeInsets::symmetric(8.0f, 16.0f);
        opts.enable_ripple = true;

        return button(lbl, [this, idx](){
            setState([this, idx]{ active_filter_ = idx; });
        }, opts);
    }

public:
    WidgetPtr build(BuildContext& ctx) override {
        // ── 1. Collapsible Flexible Space ─────────────────────
        auto flexible_header = container({
            .color = 0xFF1E1B4B,
            .align = Alignment::BottomLeft,
            .padding = StyleInsets::only(0.0f, 20.0f, 20.0f, 20.0f),
            .child = column({
                .gap = StyleValue::point(6.0f),
                .children = {
                    text("Sliver Subsystem Showcase", {
                        .color = 0xFFFFFFFF,
                        .font_size = 24.0f,
                        .font_weight = FontWeight::Bold,
                    }),
                    text("Coordinated scroll architecture with collapsible pinned header", {
                        .color = 0xFFA5B4FC,
                        .font_size = 13.0f,
                    }),
                },
            }),
        });

        // ── 2. SliverAppBar ───────────────────────────────────
        WidgetPtr app_bar = SliverAppBar {
            .title = text("ENKI — Extended Scrolling", {
                .color = 0xFFFFFFFF,
                .font_size = 16.0f,
                .font_weight = FontWeight::Bold,
            }),
            .leading = container({
                .align = Alignment::Center,
                .child = text("◄", { .color = 0xFFFFFFFF, .font_size = 16.0f }),
            }),
            .actions = {
                button(text(pinned_bar_ ? "Pinned: ON" : "Pinned: OFF", { .color = 0xFFFFFFFF, .font_size = 11.0f }), [this](){
                    setState([this]{ pinned_bar_ = !pinned_bar_; });
                }, ButtonProps{
                    .normal_color = 0xFF4338CA,
                    .border_radius = 6.0f,
                    .padding = EdgeInsets::symmetric(4.0f, 10.0f),
                }),
            },
            .flexible_space = flexible_header,
            .expanded_height = 180.0f,
            .collapsed_height = 56.0f,
            .pinned = pinned_bar_,
            .background_color = 0xFF1E1B4B,
            .elevation = 6.0f,
        };

        // ── 3. Filter Bar (SliverToBoxAdapter) ────────────────
        WidgetPtr filter_bar = SliverToBoxAdapter {
            .child = container({
                .color = 0xFF0F172A,
                .padding = StyleInsets::symmetric(14.0f, 16.0f),
                .child = row({
                    .align_items = Align::Center,
                    .gap = StyleValue::point(8.0f),
                    .children = {
                        filterButton("All Components", 0),
                        filterButton("SliverGrid Only", 1),
                        filterButton("SliverList Only", 2),
                    },
                }),
            }),
        };

        // ── 4. Section 1: SliverGrid ──────────────────────────
        WidgetPtr grid_section_title = SliverToBoxAdapter {
            .child = container({
                .color = 0xFF0F172A,
                .padding = StyleInsets::symmetric(12.0f, 18.0f),
                .child = row({
                    .align_items = Align::Center,
                    .gap = StyleValue::point(10.0f),
                    .children = {
                        text("Featured Slivers", {
                            .color = 0xFFF1F5F9,
                            .font_size = 18.0f,
                            .font_weight = FontWeight::Bold,
                        }),
                        pillBadge("6 items", 0xFF6366F1, 0xFFFFFFFF),
                    },
                }),
            }),
        };

        WidgetPtr grid_sliver = SliverPadding {
            .padding = EdgeInsets::symmetric(8.0f, 16.0f),
            .sliver = SliverGrid {
                .item_count = static_cast<int>(kGridData.size()),
                .item_builder = [](int idx) -> WidgetPtr {
                    const auto& d = kGridData[idx % kGridData.size()];
                    return container({
                        .color = 0xFF1E293B,
                        .border_radius = BorderRadius::circular(10.0f),
                        .border = Border(d.accent, 1.5f),
                        .padding = StyleInsets::all(14.0f),
                        .child = column({
                            .justify_content = Justify::SpaceBetween,
                            .height = StyleValue::percent(100.0f),
                            .children = {
                                row({
                                    .justify_content = Justify::SpaceBetween,
                                    .children = {
                                        pillBadge(d.category, d.accent, 0xFFFFFFFF),
                                        text(d.metric, {
                                            .color = 0xFF38BDF8,
                                            .font_size = 12.0f,
                                            .font_weight = FontWeight::Bold,
                                        }),
                                    },
                                }),
                                text(d.title, {
                                    .color = 0xFFFFFFFF,
                                    .font_size = 15.0f,
                                    .font_weight = FontWeight::Bold,
                                }),
                            },
                        }),
                    });
                },
                .fixed_delegate = SliverGridDelegateFixedCount(3, 10.0f, 10.0f, 1.25f),
            },
        };

        // ── 5. Section 2: SliverList ──────────────────────────
        WidgetPtr list_section_title = SliverToBoxAdapter {
            .child = container({
                .color = 0xFF0F172A,
                .padding = StyleInsets::symmetric(14.0f, 18.0f),
                .child = row({
                    .align_items = Align::Center,
                    .gap = StyleValue::point(10.0f),
                    .children = {
                        text("Virtual Activity Stream", {
                            .color = 0xFFF1F5F9,
                            .font_size = 18.0f,
                            .font_weight = FontWeight::Bold,
                        }),
                        pillBadge("50 entries", 0xFF10B981, 0xFFFFFFFF),
                    },
                }),
            }),
        };

        WidgetPtr list_sliver = SliverPadding {
            .padding = EdgeInsets::symmetric(4.0f, 16.0f),
            .sliver = SliverList {
                .item_count = 50,
                .item_builder = [](int idx) -> WidgetPtr {
                    Color dot_color = kThemeGradients[idx % 6];
                    return container({
                        .color = 0xFF1E293B,
                        .border_radius = BorderRadius::circular(8.0f),
                        .padding = StyleInsets::symmetric(12.0f, 16.0f),
                        .child = row({
                            .align_items = Align::Center,
                            .gap = StyleValue::point(12.0f),
                            .children = {
                                container({
                                    .color = dot_color,
                                    .border_radius = BorderRadius::circular(18.0f),
                                    .align = Alignment::Center,
                                    .width = StyleValue::point(36.0f),
                                    .height = StyleValue::point(36.0f),
                                    .child = text(std::to_string(idx + 1), {
                                        .color = 0xFFFFFFFF,
                                        .font_size = 12.0f,
                                        .font_weight = FontWeight::Bold,
                                    }),
                                }),
                                column({
                                    .flex_grow = 1.0f,
                                    .gap = StyleValue::point(3.0f),
                                    .children = {
                                        text("Sliver Feed Item #" + std::to_string(idx + 1), {
                                            .color = 0xFFFFFFFF,
                                            .font_size = 14.0f,
                                            .font_weight = FontWeight::Bold,
                                        }),
                                        text("High performance coordinated layout render block", {
                                            .color = 0xFF94A3B8,
                                            .font_size = 12.0f,
                                        }),
                                    },
                                }),
                                pillBadge("Active", 0xFF065F46, 0xFF34D399),
                            },
                        }),
                    });
                },
                .separator_builder = [](int idx) -> WidgetPtr {
                    return sizedBox(0.0f, 8.0f);
                },
            },
        };

        // ── Assemble Slivers ──────────────────────────────────
        std::vector<WidgetPtr> all_slivers;
        all_slivers.push_back(app_bar);
        all_slivers.push_back(filter_bar);

        if (active_filter_ == 0 || active_filter_ == 1) {
            all_slivers.push_back(grid_section_title);
            all_slivers.push_back(grid_sliver);
        }

        if (active_filter_ == 0 || active_filter_ == 2) {
            all_slivers.push_back(list_section_title);
            all_slivers.push_back(list_sliver);
        }

        all_slivers.push_back(SliverToBoxAdapter {
            .child = container({
                .color = 0xFF0F172A,
                .align = Alignment::Center,
                .padding = StyleInsets::all(24.0f),
                .child = text("— End of CustomScrollView Stream —", {
                    .color = 0xFF64748B,
                    .font_size = 12.0f,
                }),
            }),
        });

        return CustomScrollView {
            .slivers = std::move(all_slivers),
            .direction = Axis::Vertical,
            .scroll_physics = ScrollPhysics::Clamped,
            .scroll_speed = 50.0f,
            .show_scrollbar = true,
        };
    }
};

class SliverDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override { return std::make_unique<SliverDemoState>(); }
    std::string_view typeName() const override { return "SliverDemoApp"; }
};

int main() {
    AppConfig cfg;
    cfg.title      = "ENKI — Extended Scrolling (Slivers) Demo";
    cfg.width      = 720;
    cfg.height     = 780;
    cfg.resizable  = true;
    cfg.vsync      = false;
    cfg.target_fps = 0;
    cfg.show_performance_overlay = true;
    cfg.clear_color = 0xFF0F172A;
    return runApp(std::make_shared<SliverDemoApp>(), cfg);
}
