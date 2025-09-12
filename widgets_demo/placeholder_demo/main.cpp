/// @file main.cpp
/// @brief ENKI Advanced Placeholder & Skeleton Shimmer Interactive Showcase Demo.

#include "enki/app/app.hpp"
#include "enki/widgets/placeholder.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/state/state.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <memory>

using namespace enki;

class PlaceholderDemoState : public State {
private:
    std::string hud_msg_ = "Notice the live dimension badges in Blueprint mode, smooth 60fps Shimmer wave in Skeleton mode, and interactive Media Slots.";
    bool is_loading_mode_ = true;

public:
    WidgetPtr build(BuildContext&) override {
        // ── Main Page Header ──────────────────────────────────────────
        auto title = text("Advanced Placeholder & Skeleton Suite", {
            .color = 0xFFFFFFFF,
            .font_size = 22.0f,
            .font_weight = FontWeight::Bold,
        });

        auto sub = text("Enterprise scaffolding (Category 2. Basic UI), Blueprint wireframes with dimension badges, 60fps Shimmer loaders, and Media Slots", {
            .color = 0xFF94A3B8,
            .font_size = 13.0f,
        });

        auto title_col = column({
            .align_items = Align::Center,
            .gap = StyleValue::point(6.0f),
            .children = {title, sub}
        });

        // ── Left Column: Blueprint Wireframe Scaffolding ──────────────
        auto bp_title = text("📐 Blueprint Wireframes & Layout Slots", {
            .color = 0xFF38BDF8,
            .font_size = 15.0f,
            .font_weight = FontWeight::Bold,
        });

        auto slot_hero = Placeholder {
            .style = PlaceholderStyle::Blueprint,
            .width = 320.0f,
            .height = 100.0f,
            .label = "Hero Banner"
        };
        auto slot_chart = Placeholder {
            .style = PlaceholderStyle::Blueprint,
            .width = 320.0f,
            .height = 130.0f,
            .label = "Analytics Chart Area"
        };

        auto media_slot = Placeholder {
            .style = PlaceholderStyle::MediaSlot,
            .width = 320.0f,
            .height = 90.0f,
            .label = "Drop Avatar or Image File Here",
            .icon = "🖼️"
        };

        auto bp_col = column({
            .gap = StyleValue::point(12.0f),
            .children = {bp_title, slot_hero, slot_chart, media_slot}
        });

        auto bp_card = container({
            .color = 0xFF0F172A,
            .border_radius = BorderRadius::circular(12.0f),
            .border = Border(0xFF334155, 1.0f),
            .width = StyleValue::point(360.0f),
            .padding = StyleInsets::all(20.0f),
            .child = bp_col
        });

        // ── Right Column: Skeleton Shimmer Loading Screens ────────────
        auto sk_title = text("⚡ Animated Shimmer Skeletons", {
            .color = 0xFF10B981,
            .font_size = 15.0f,
            .font_weight = FontWeight::Bold,
        });

        // Toggle Loading Button
        auto t_btn = text(is_loading_mode_ ? "Switch to: Loaded UI" : "Switch to: Skeleton", {
            .color = 0xFFFFFFFF,
            .font_size = 11.5f,
            .font_weight = FontWeight::Bold,
        });
        auto b_btn = container({
            .color = is_loading_mode_ ? 0xFF0284C7 : 0xFF059669,
            .border_radius = BorderRadius::circular(6.0f),
            .padding = StyleInsets::symmetric(6.0f, 14.0f),
            .child = t_btn
        });

        auto gd_btn = std::make_shared<GestureDetector>(b_btn);
        gd_btn->cursor_type = SystemCursor::Pointer;
        gd_btn->on_tap_up = [this](const TapUpDetails&) {
            is_loading_mode_ = !is_loading_mode_;
            hud_msg_ = is_loading_mode_ ? "Showing animated skeleton loading state" : "Showing simulated loaded content";
            setState([] {});
        };

        auto head_row = row({
            .justify_content = Justify::SpaceBetween,
            .align_items = Align::Center,
            .width = StyleValue::percent(100.0f),
            .children = {sk_title, gd_btn}
        });

        WidgetPtr display_content;

        if (is_loading_mode_) {
            // Skeleton Card & List
            auto sk_card = placeholderCardSkeleton(380.0f);
            auto sk_list = placeholderListSkeleton(3, 380.0f);

            display_content = column({
                .gap = StyleValue::point(14.0f),
                .children = {head_row, sk_card, sk_list}
            });
        } else {
            // Actual Loaded Content Card
            auto t1 = text("Alex Morgan (Lead Engineer)", { .color = 0xFFFFFFFF, .font_size = 14.0f, .font_weight = FontWeight::Bold });
            auto t2 = text("Senior Distributed Systems Specialist • Tokyo, Japan", { .color = 0xFF38BDF8, .font_size = 11.5f });
            auto t3 = text("Successfully orchestrated high-throughput rendering pipelines with 600+ FPS benchmark throughput on Linux Wayland/X11.", {
                .color = 0xFFCBD5E1,
                .font_size = 12.0f,
            });

            auto lc_col = column({
                .gap = StyleValue::point(8.0f),
                .children = {t1, t2, t3}
            });

            auto lc_box = container({
                .color = 0xFF1E293B,
                .border_radius = BorderRadius::circular(12.0f),
                .border = Border(0xFF0284C7, 1.0f),
                .width = StyleValue::point(380.0f),
                .padding = StyleInsets::all(16.0f),
                .child = lc_col
            });

            // List of items
            auto makeItem = [](std::string icon, std::string it_title, std::string badge) -> WidgetPtr {
                auto ic = text(icon, { .font_size = 14.0f });
                auto tt = text(it_title, { .color = 0xFFFFFFFF, .font_size = 12.5f, .font_weight = FontWeight::Bold });
                auto bg = text(badge, { .color = 0xFF38BDF8, .font_size = 10.5f, .font_weight = FontWeight::Bold });

                auto bg_box = container({
                    .color = 0x330284C7,
                    .border_radius = BorderRadius::circular(4.0f),
                    .padding = StyleInsets::symmetric(2.0f, 6.0f),
                    .child = bg
                });

                auto it_row = row({
                    .justify_content = Justify::SpaceBetween,
                    .align_items = Align::Center,
                    .width = StyleValue::percent(100.0f),
                    .children = {ic, tt, bg_box}
                });

                return container({
                    .color = 0xFF1E293B,
                    .border_radius = BorderRadius::circular(8.0f),
                    .padding = StyleInsets::symmetric(10.0f, 12.0f),
                    .child = it_row
                });
            };

            auto item1 = makeItem("📊", "Performance Pipeline", "ACTIVE");
            auto item2 = makeItem("🚀", "Wayland Buffer Sync", "SYNCD");
            auto item3 = makeItem("🎨", "Skia Direct Shaders", "OPTIMAL");

            auto list_col = column({
                .gap = StyleValue::point(8.0f),
                .width = StyleValue::point(380.0f),
                .children = {item1, item2, item3}
            });

            display_content = column({
                .gap = StyleValue::point(14.0f),
                .children = {head_row, lc_box, list_col}
            });
        }

        auto right_card = container({
            .color = 0xFF0F172A,
            .border_radius = BorderRadius::circular(12.0f),
            .border = Border(0xFF334155, 1.0f),
            .width = StyleValue::point(420.0f),
            .padding = StyleInsets::all(20.0f),
            .child = display_content
        });

        // ── Side-by-Side Main Sections ────────────────────────────────
        auto sections_row = row({
            .justify_content = Justify::Center,
            .align_items = Align::Start,
            .gap = StyleValue::point(24.0f),
            .children = {bp_card, right_card}
        });

        // ── HUD / Status Box ──────────────────────────────────────────
        auto hud_box = container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(6.0f),
            .border = Border(0xFF334155, 1.0f),
            .width = StyleValue::point(804.0f),
            .padding = StyleInsets::symmetric(8.0f, 16.0f),
            .child = row({
                .children = { text("💡 " + hud_msg_, { .color = 0xFF38BDF8, .font_size = 12.5f }) }
            })
        });

        // ── Assemble Page Body ────────────────────────────────────────
        auto page_col = column({
            .align_items = Align::Center,
            .gap = StyleValue::point(20.0f),
            .children = {title_col, sections_row, hud_box}
        });

        return container({
            .color = 0xFF0B1120,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(24.0f),
            .child = page_col
        });
    }
};

class PlaceholderDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<PlaceholderDemoState>();
    }
    std::string_view typeName() const override { return "PlaceholderDemoApp"; }
};

int main() {
    std::cout << "====================================================\n";
    std::cout << "  ENKI Engine — Advanced Placeholder Demo\n";
    std::cout << "====================================================\n";

    AppConfig config;
    config.title       = "Enki Engine — Advanced Placeholder Demo";
    config.width       = 1180;
    config.height      = 720;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 0;
    config.show_performance_overlay = true;
    config.clear_color = 0xFF0B1120;

    return runApp(std::make_shared<PlaceholderDemoApp>(), config);
}
