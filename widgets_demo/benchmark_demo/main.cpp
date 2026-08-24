/// @file main.cpp
/// @brief ENKI Extreme Performance & Benchmark Demo (Zero-Blur Fast Path)

#include "enki/app/app.hpp"
#include "enki/state/state.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/gesture_detector.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>

using namespace enki;

// ════════════════════════════════════════════════════════════════
// Design System & Colors
// ════════════════════════════════════════════════════════════════

namespace Style {
    constexpr Color bg_dark       = 0xFF0B0F19;
    constexpr Color bg_card       = 0xFF161D2F;
    constexpr Color bg_card_hover = 0xFF1E283E;
    constexpr Color border_subtle = 0x3038BDF8;
    constexpr Color border_bright = 0x8038BDF8;
    constexpr Color primary       = 0xFF6366F1;
    constexpr Color primary_light = 0xFF818CF8;
    constexpr Color cyan_neon     = 0xFF00E5FF;
    constexpr Color emerald       = 0xFF10B981;
    constexpr Color amber         = 0xFFF59E0B;
    constexpr Color text_white    = 0xFFFFFFFF;
    constexpr Color text_muted    = 0xFF94A3B8;
}

// ════════════════════════════════════════════════════════════════
// Benchmark State & App
// ════════════════════════════════════════════════════════════════

class BenchmarkDemoApp : public StatefulWidget {
public:
    std::string_view typeName() const override { return "BenchmarkDemoApp"; }
    std::unique_ptr<State> createState() override;
};

class BenchmarkDemoState : public State {
public:
    int click_counter = 0;
    int active_tab = 0;
    int stress_item_count = 24;
    bool is_hovered = false;

    WidgetPtr build(BuildContext& context) override {
        auto root = container(column({
            .children = {
                buildHeader(),
                sizedBox(0, 16.0f),
                buildStatsRow(),
                sizedBox(0, 16.0f),
                buildControls(),
                sizedBox(0, 16.0f),
                buildGrid(),
            }
        }));
        root->paddingAll(24.0f).color(Style::bg_dark);
        return root;
    }

private:
    WidgetPtr buildHeader() {
        auto title = text({
            .text = "ENKI — Extreme Performance Showcase",
            .color = Style::text_white,
            .font_size = 22.0f,
            .font_weight = FontWeight::Bold,
        });

        auto subtitle = text({
            .text = "Zero-Blur Fast Path: Pure GPU Rasterization, Zero Offscreen FBO Passes",
            .color = Style::cyan_neon,
            .font_size = 12.5f,
        });

        return column({
            .children = {
                title,
                sizedBox(0, 4.0f),
                subtitle,
            }
        });
    }

    WidgetPtr buildStatsRow() {
        auto card1 = buildMetricCard("Total Interactions", std::to_string(click_counter), "Clicks", Style::primary_light);
        auto card2 = buildMetricCard("Render Mode", "Zero-Blur", "Pure Vector", Style::emerald);
        auto card3 = buildMetricCard("Active Nodes", std::to_string(stress_item_count + 18), "Elements", Style::amber);
        auto card4 = buildMetricCard("Engine Target", "Uncapped", "0ms Throttle", Style::cyan_neon);

        return row({
            .justify_content = Justify::SpaceBetween,
            .align_items = Align::Center,
            .children = {
                card1,
                card2,
                card3,
                card4,
            }
        });
    }

    WidgetPtr buildMetricCard(const std::string& label, const std::string& val, const std::string& unit, Color accent) {
        auto lbl = text({
            .text = label,
            .color = Style::text_muted,
            .font_size = 11.0f,
        });

        auto v = text({
            .text = val,
            .color = accent,
            .font_size = 20.0f,
            .font_weight = FontWeight::Bold,
        });

        auto u = text({
            .text = unit,
            .color = Style::text_muted,
            .font_size = 10.0f,
        });

        auto content = column({
            .children = {
                lbl,
                sizedBox(0, 4.0f),
                row({
                    .justify_content = Justify::Start,
                    .align_items = Align::Baseline,
                    .children = {
                        v,
                        sizedBox(6.0f, 0),
                        u,
                    }
                }),
            }
        });

        auto c = container(content);
        c->width(230.0f)
         .paddingAll(14.0f)
         .color(Style::bg_card)
         .borderRadius(10.0f)
         .border(Style::border_subtle, 1.0f);
        return c;
    }

    WidgetPtr buildControls() {
        auto btn_inc_text = text({
            .text = "⚡ Click to Mutate Tree (+" + std::to_string(click_counter) + ")",
            .color = Style::text_white,
            .font_size = 13.0f,
            .font_weight = FontWeight::Bold,
        });

        auto btn_inc_box = container(btn_inc_text);
        btn_inc_box->paddingSymmetric(18.0f, 10.0f)
                   .color(is_hovered ? Style::primary_light : Style::primary)
                   .borderRadius(8.0f)
                   .border(Style::cyan_neon, is_hovered ? 1.5f : 0.0f);

        auto btn_inc = gestureDetector({
            .key = Key::string("btn_inc"),
            .child = btn_inc_box,
            .cursor_type = SystemCursor::Pointer,
            .on_tap = [this]() {
                setState([this]() {
                    click_counter++;
                });
            },
            .on_hover_enter = [this](const auto&) {
                setState([this]() { is_hovered = true; });
            },
            .on_hover_exit = [this](const auto&) {
                setState([this]() { is_hovered = false; });
            },
        });

        auto btn_add_text = text({
            .text = "+ Add 12 Items",
            .color = Style::text_white,
            .font_size = 12.0f,
        });
        auto btn_add_box = container(btn_add_text);
        btn_add_box->paddingSymmetric(14.0f, 10.0f)
                   .color(Style::bg_card)
                   .borderRadius(8.0f)
                   .border(Style::border_subtle, 1.0f);
        auto btn_add = gestureDetector({
            .key = Key::string("btn_add"),
            .child = btn_add_box,
            .cursor_type = SystemCursor::Pointer,
            .on_tap = [this]() {
                setState([this]() {
                    stress_item_count = std::min(stress_item_count + 12, 120);
                });
            },
        });

        auto btn_reset_text = text({
            .text = "Reset Grid",
            .color = Style::text_muted,
            .font_size = 12.0f,
        });
        auto btn_reset_box = container(btn_reset_text);
        btn_reset_box->paddingSymmetric(14.0f, 10.0f)
                     .color(Style::bg_card)
                     .borderRadius(8.0f)
                     .border(Style::border_subtle, 1.0f);
        auto btn_reset = gestureDetector({
            .key = Key::string("btn_reset"),
            .child = btn_reset_box,
            .cursor_type = SystemCursor::Pointer,
            .on_tap = [this]() {
                setState([this]() {
                    stress_item_count = 24;
                    click_counter = 0;
                });
            },
        });

        return row({
            .justify_content = Justify::Start,
            .align_items = Align::Center,
            .children = {
                btn_inc,
                sizedBox(12.0f, 0),
                btn_add,
                sizedBox(12.0f, 0),
                btn_reset,
            }
        });
    }

    WidgetPtr buildGrid() {
        std::vector<WidgetPtr> items;
        items.reserve(stress_item_count);

        for (int i = 0; i < stress_item_count; ++i) {
            auto item_title = text({
                .text = "Node #" + std::to_string(i + 1),
                .color = Style::text_white,
                .font_size = 12.0f,
                .font_weight = FontWeight::Bold,
            });

            auto item_sub = text({
                .text = "Val: " + std::to_string((click_counter * 7 + i * 13) % 997),
                .color = Style::cyan_neon,
                .font_size = 11.0f,
            });

            auto col = column({
                .children = {
                    item_title,
                    sizedBox(0, 4.0f),
                    item_sub,
                }
            });

            auto card = container(col);
            card->width(140.0f)
                .height(68.0f)
                .paddingAll(10.0f)
                .color(Style::bg_card)
                .borderRadius(8.0f)
                .border(Style::border_subtle, 1.0f);

            items.push_back(card);
        }

        // Wrap in rows of 6 items
        std::vector<WidgetPtr> rows;
        for (size_t i = 0; i < items.size(); i += 6) {
            std::vector<WidgetPtr> row_items;
            for (size_t j = i; j < std::min(i + 6, items.size()); ++j) {
                row_items.push_back(items[j]);
                if (j + 1 < std::min(i + 6, items.size())) {
                    row_items.push_back(sizedBox(12.0f, 0));
                }
            }
            rows.push_back(row({
                .children = std::move(row_items)
            }));
            rows.push_back(sizedBox(0, 10.0f));
        }

        return column({
            .children = std::move(rows)
        });
    }
};

std::unique_ptr<State> BenchmarkDemoApp::createState() {
    return std::make_unique<BenchmarkDemoState>();
}

// ════════════════════════════════════════════════════════════════
// Main Entry
// ════════════════════════════════════════════════════════════════

int main(int argc, char** argv) {
    AppConfig config;
    config.title                    = "ENKI — Extreme Performance Benchmark (Zero-Blur)";
    config.width                    = 1080;
    config.height                   = 620;
    config.window_mode              = WindowMode::Normal;
    config.vsync                    = false; // Uncapped
    config.target_fps               = 0;     // Maximum rate
    config.show_performance_overlay = true;  // Live HUD
    config.clear_color              = Style::bg_dark;

    return runApp(std::make_shared<BenchmarkDemoApp>(), config);
}
