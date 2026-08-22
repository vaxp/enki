#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/scroll_view.hpp"
#include "enki/state/state.hpp"
#include <iostream>

using namespace enki;

class ScrollDemoState : public State {
public:
    WidgetPtr build(BuildContext& ctx) override {
        // Generate a long list of items
        std::vector<WidgetPtr> list_items;
        for (int i = 1; i <= 50; ++i) {
            auto item_text = text("Item " + std::to_string(i), { .color = 0xFFE2E8F0, .font_size = 16.0f });

            auto btn_text = text("Action " + std::to_string(i), { .color = 0xFFFFFFFF, .font_size = 13.0f });
            ButtonProps btn_opts;
            btn_opts.normal_color = 0xFF3B82F6;
            auto item_btn = button(btn_text, [i]() {
                std::cout << "Clicked Action " << i << std::endl;
            }, btn_opts);

            auto item_row = row({
                .justify_content = Justify::SpaceBetween,
                .align_items = Align::Center,
                .children = { item_text, item_btn }
            });

            auto item_container = container({
                .color = (i % 2 == 0 ? 0xFF1E293B : 0xFF0F172A),
                .border = Border(0xFF334155, 1.0f),
                .width = StyleValue::percent(100.0f),
                .padding = StyleInsets::symmetric(10.0f, 16.0f),
                .child = item_row
            });

            list_items.push_back(item_container);
        }

        auto vertical_col = column({
            .flex_shrink = 0.0f,
            .width = StyleValue::percent(100.0f),
            .children = list_items
        });

        auto vertical_scroll = ScrollView {
            .child = vertical_col,
            .direction = Axis::Vertical,
            .show_scrollbar = true,
        };

        auto vs_container = container({
            .min_height = StyleValue::point(0.0f),
            .flex = 1.0f,
            .child = vertical_scroll
        });

        // Horizontal list of cards
        std::vector<WidgetPtr> horizontal_cards;
        for (int i = 1; i <= 10; ++i) {
            auto card_text = text("Card " + std::to_string(i), {
                .color = 0xFFFFFFFF,
                .font_size = 18.0f,
                .font_weight = FontWeight::Bold
            });

            auto card_container = container({
                .color = 0xFF8B5CF6,
                .border_radius = BorderRadius::circular(8.0f),
                .align = Alignment::Center,
                .width = StyleValue::point(150.0f),
                .height = StyleValue::point(100.0f),
                .margin = StyleInsets::symmetric(0.0f, 8.0f),
                .flex_shrink = 0.0f,
                .child = card_text
            });

            horizontal_cards.push_back(card_container);
        }

        auto horizontal_row = row({
            .flex_shrink = 0.0f,
            .children = horizontal_cards
        });

        auto horizontal_scroll = ScrollView {
            .child = horizontal_row,
            .direction = Axis::Horizontal,
            .show_scrollbar = true,
        };

        auto hs_container = container({
            .height = StyleValue::point(120.0f),
            .min_width = StyleValue::point(0.0f),
            .child = horizontal_scroll
        });

        // Header
        auto header_title = text("Advanced ScrollView Demo", {
            .color = 0xFFFFFFFF,
            .font_size = 28.0f,
            .font_weight = FontWeight::Bold
        });

        auto header_subtitle = text("Featuring 100% Anu Layout, Nested Scrolling, and Interactive Children", {
            .color = 0xFF94A3B8,
            .font_size = 14.0f
        });

        auto header_container = container({
            .color = 0xFF0F172A,
            .border = Border(0xFF334155, 1.0f),
            .width = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(20.0f),
            .child = column({
                .align_items = Align::Center,
                .children = { header_title, header_subtitle }
            })
        });

        // Horizontal scroll area
        auto h_label = text("Horizontal Scroll", {
            .color = 0xFFE2E8F0,
            .font_size = 16.0f,
            .font_weight = FontWeight::Bold
        });

        auto hs_area_container = container({
            .padding = StyleInsets::all(16.0f),
            .child = column({
                .children = {
                    container({
                        .padding = StyleInsets::all(8.0f),
                        .child = h_label
                    }),
                    hs_container
                }
            })
        });

        // Vertical scroll area label
        auto v_label = text("Vertical Scroll List", {
            .color = 0xFFE2E8F0,
            .font_size = 16.0f,
            .font_weight = FontWeight::Bold
        });

        auto v_label_container = container({
            .padding = StyleInsets::symmetric(8.0f, 24.0f),
            .child = v_label
        });

        // Main Layout
        auto main_col = column({
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .children = {
                header_container,
                hs_area_container,
                v_label_container,
                vs_container
            }
        });

        return container({
            .color = 0xFF1E293B,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .child = main_col
        });
    }
};

class ScrollDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<ScrollDemoState>();
    }
    std::string_view typeName() const override { return "ScrollDemoApp"; }
};

int main() {
    std::cout << "================================================\n";
    std::cout << "  ENKI Engine — ScrollView Demo   \n";
    std::cout << "================================================\n";

    AppConfig config;
    config.title       = "Enki Engine — ScrollView Demo";
    config.width       = 800;
    config.height      = 600;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 0;
    config.show_performance_overlay = true;
    config.clear_color = 0xFF0F172A;

    return runApp(std::make_shared<ScrollDemoApp>(), config);
}
