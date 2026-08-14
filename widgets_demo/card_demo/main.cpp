#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/card.hpp"
#include "enki/widgets/button.hpp"

#include <iostream>

using namespace enki;

class CardDemoWidget : public StatelessWidget {
public:
    std::string_view typeName() const override { return "CardDemoWidget"; }

    WidgetPtr build(BuildContext&) override {
        auto title = std::make_shared<Text>("Card Widget Demo", TextStyle{.color = 0xFFFFFFFF, .font_size = 24.0f});
        
        // Simple Card
        auto text1 = std::make_shared<Text>("This is a simple card.", TextStyle{.color = 0xFFCCCCCC});
        auto card1 = card(text1);
        card1->paddingAll(20.0f);
        
        // Elevated Card with Border
        auto text2 = std::make_shared<Text>("Elevated Card with a Border.", TextStyle{.color = 0xFFFFFFFF, .font_size = 18.0f});
        auto card2 = card(text2);
        card2->paddingAll(24.0f).elevation(16.0f).color(0xFF2D3748).border(0xFF4A5568, 1.0f).borderRadius(16.0f);
                        
        // Interactive Card
        auto avatar_placeholder = container(nullptr);
        avatar_placeholder->size(40.0f, 40.0f).color(0xFF4FD1C5).borderRadius(20.0f);
        
        auto name = std::make_shared<Text>("Jane Doe", TextStyle{.color = 0xFFFFFFFF, .font_size = 16.0f, .font_weight = FontWeight::Bold});
        auto desc = std::make_shared<Text>("Software Engineer", TextStyle{.color = 0xFFA0AEC0, .font_size = 12.0f});
        std::vector<WidgetPtr> name_desc = {name, desc};
        auto name_col = column(name_desc);
        
        std::vector<WidgetPtr> header_items = {avatar_placeholder, name_col};
        auto header = row(header_items);
        header->gap(StyleValue::point(12.0f)).alignItems(Align::Center);
        
        auto action_btn = button(std::make_shared<Text>("Follow", TextStyle{.color = 0xFFFFFFFF}), []{});
        std::vector<WidgetPtr> content_items = {header, action_btn};
        auto card_content = column(content_items);
        card_content->gap(StyleValue::point(16.0f));
        
        auto card3 = card(card_content);
        card3->paddingAll(20.0f).color(0xFF1A202C).border(0xFF2D3748, 1.0f);

        std::vector<WidgetPtr> col_items = {title, card1, card2, card3};
        auto col = column(col_items);
        col->gap(StyleValue::point(24.0f)).padding(StyleInsets::all(32.0f));

        auto bg = container(col);
        bg->color(0xFF0F172A);

        return bg;
    }
};

int main() {
    std::cout << "Starting Card Demo...\n";
    AppConfig config;
    config.title = "ENKI Card Demo";
    config.width = 600;
    config.height = 600;
    config.target_fps = 60;
    
    return runApp(std::make_shared<CardDemoWidget>(), config);
}
