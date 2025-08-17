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
            auto item_text = text("Item " + std::to_string(i));
            item_text->fontSize(16.0f).color(0xFFE2E8F0);

            auto btn_text = text("Action " + std::to_string(i));
            btn_text->color(0xFFFFFFFF);
            ButtonProps btn_opts;
            btn_opts.normal_color = 0xFF3B82F6;
            auto item_btn = button(std::static_pointer_cast<Widget>(btn_text), [i]() {
                std::cout << "Clicked Action " << i << std::endl;
            }, btn_opts);

            std::vector<WidgetPtr> row_children = { 
                std::static_pointer_cast<Widget>(item_text), 
                std::static_pointer_cast<Widget>(item_btn) 
            };
            auto item_row = row(row_children);
            item_row->alignItems(Align::Center).justifyContent(Justify::SpaceBetween);
            
            auto item_container = container(item_row);
            item_container->padding(StyleInsets::symmetric(10.0f, 16.0f))
                          .color(i % 2 == 0 ? 0xFF1E293B : 0xFF0F172A)
                          .border(0xFF334155, 1.0f) // Removed StyleBorders::only
                          .width(StyleValue::percent(100.0f));

            list_items.push_back(std::static_pointer_cast<Widget>(item_container));
        }

        auto vertical_col = column(list_items);
        vertical_col->width(StyleValue::percent(100.0f)).flexShrink(0.0f);

        auto vertical_scroll = scrollView(
            ScrollOptions{.direction = Axis::Vertical, .show_scrollbar = true},
            std::static_pointer_cast<Widget>(vertical_col)
        );

        auto vs_container = container(std::static_pointer_cast<Widget>(vertical_scroll));
        vs_container->flex(1.0f).minHeight(0.0f); // Takes up remaining height but can shrink

        // Horizontal list of cards
        std::vector<WidgetPtr> horizontal_cards;
        for (int i = 1; i <= 10; ++i) {
            auto card_text = text("Card " + std::to_string(i));
            card_text->fontSize(18.0f).bold().color(0xFFFFFFFF);

            auto card_container = container(std::static_pointer_cast<Widget>(card_text));
            card_container->width(150.0f).height(100.0f)
                          .color(0xFF8B5CF6)
                          .borderRadius(8.0f)
                          .margin(StyleInsets::symmetric(0.0f, 8.0f))
                          .align(Alignment::Center)
                          .flexShrink(0.0f);

            horizontal_cards.push_back(std::static_pointer_cast<Widget>(card_container));
        }

        auto horizontal_row = row(horizontal_cards);
        horizontal_row->flexShrink(0.0f);
        auto horizontal_scroll = scrollView(
            ScrollOptions{.direction = Axis::Horizontal, .show_scrollbar = true},
            std::static_pointer_cast<Widget>(horizontal_row)
        );
        auto hs_container = container(std::static_pointer_cast<Widget>(horizontal_scroll));
        hs_container->height(120.0f).minWidth(0.0f);

        // Build the root layout
        std::vector<WidgetPtr> root_children;
        
        // Header
        auto header_title = text("Advanced ScrollView Demo");
        header_title->fontSize(28.0f).bold().color(0xFFFFFFFF);

        auto header_subtitle = text("Featuring 100% Anu Layout, Nested Scrolling, and Interactive Children");
        header_subtitle->fontSize(14.0f).color(0xFF94A3B8);

        std::vector<WidgetPtr> header_texts = { 
            std::static_pointer_cast<Widget>(header_title), 
            std::static_pointer_cast<Widget>(header_subtitle) 
        };
        auto header_col = column(header_texts);
        header_col->alignItems(Align::Center);
        
        auto header_container = container(std::static_pointer_cast<Widget>(header_col));
        header_container->padding(StyleInsets::all(20.0f))
                        .width(StyleValue::percent(100.0f))
                        .color(0xFF0F172A)
                        .border(0xFF334155, 1.0f);

        root_children.push_back(std::static_pointer_cast<Widget>(header_container));
         
        // Horizontal scroll area
        auto h_label = text("Horizontal Scroll");
        h_label->fontSize(16.0f).bold().color(0xFFE2E8F0);
        
        auto h_label_container = container(std::static_pointer_cast<Widget>(h_label));
        h_label_container->padding(StyleInsets::all(8.0f));

        std::vector<WidgetPtr> hs_area_children = { 
            std::static_pointer_cast<Widget>(h_label_container), 
            std::static_pointer_cast<Widget>(hs_container) 
        };
        auto hs_area_col = column(hs_area_children);
        
        auto hs_area_container = container(std::static_pointer_cast<Widget>(hs_area_col));
        hs_area_container->padding(StyleInsets::all(16.0f));

        root_children.push_back(std::static_pointer_cast<Widget>(hs_area_container));
        
        // Vertical scroll area label
        auto v_label = text("Vertical Scroll List");
        v_label->fontSize(16.0f).bold().color(0xFFE2E8F0);

        auto v_label_container = container(std::static_pointer_cast<Widget>(v_label));
        v_label_container->padding(StyleInsets::symmetric(8.0f, 24.0f));

        root_children.push_back(std::static_pointer_cast<Widget>(v_label_container));
        
        // The vertical list itself
        root_children.push_back(std::static_pointer_cast<Widget>(vs_container));

        auto main_col = column(root_children);
        main_col->width(StyleValue::percent(100.0f)).height(StyleValue::percent(100.0f));

        auto root = container(std::static_pointer_cast<Widget>(main_col));
        root->color(0xFF1E293B)
            .width(StyleValue::percent(100.0f))
            .height(StyleValue::percent(100.0f));

        return root;
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
