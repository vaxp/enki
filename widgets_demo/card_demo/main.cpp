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
        auto title = text("Card Widget Demo", { .color = 0xFFFFFFFF, .font_size = 24.0f });
        
        // Simple Card
        auto card1 = Card {
            .padding = StyleInsets::all(20.0f),
            .child = text("This is a simple card.", { .color = 0xFFCCCCCC })
        };
        
        // Elevated Card with Border
        auto card2 = Card {
            .color = 0xFF2D3748,
            .elevation = 16.0f,
            .border_radius = BorderRadius::circular(16.0f),
            .border = Border(0xFF4A5568, 1.0f),
            .padding = StyleInsets::all(24.0f),
            .child = text("Elevated Card with a Border.", { .color = 0xFFFFFFFF, .font_size = 18.0f })
        };
                        
        // Interactive Card
        auto avatar_placeholder = container({
            .color = 0xFF4FD1C5,
            .border_radius = BorderRadius::circular(20.0f),
            .width = StyleValue::point(40.0f),
            .height = StyleValue::point(40.0f)
        });
        
        auto name = text("Jane Doe", { .color = 0xFFFFFFFF, .font_size = 16.0f, .font_weight = FontWeight::Bold });
        auto desc = text("Software Engineer", { .color = 0xFFA0AEC0, .font_size = 12.0f });
        
        auto header = row({
            .align_items = Align::Center,
            .gap = StyleValue::point(12.0f),
            .children = {
                avatar_placeholder,
                column({
                    .children = {name, desc}
                })
            }
        });
        
        auto action_btn = button(text("Follow", { .color = 0xFFFFFFFF }), []{});
        
        auto card3 = Card {
            .color = 0xFF1A202C,
            .border = Border(0xFF2D3748, 1.0f),
            .padding = StyleInsets::all(20.0f),
            .child = column({
                .gap = StyleValue::point(16.0f),
                .children = {header, action_btn}
            })
        };

        return container({
            .color = 0xFF0F172A,
            .child = column({
                .gap = StyleValue::point(24.0f),
                .padding = StyleInsets::all(32.0f),
                .children = {title, card1, card2, card3}
            })
        });
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
