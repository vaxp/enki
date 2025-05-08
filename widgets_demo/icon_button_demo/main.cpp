#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/icon.hpp"
#include "enki/widgets/icon_button.hpp"
#include "enki/widgets/icons_material.hpp"
#include "enki/widgets/stack.hpp"
#include <iostream>

using namespace enki;

class IconButtonDemoWidget : public StatelessWidget {
public:
    [[nodiscard]] std::string_view typeName() const override { return "IconButtonDemoWidget"; }

    [[nodiscard]] WidgetPtr build(BuildContext& context) override {
        auto title = std::make_shared<Text>("IconButton Demo", TextStyle{.color = 0xFFFFFFFF, .font_size = 24.0f, .font_weight = FontWeight::Bold});
        
        // 1. Standard IconButton
        auto ic1 = icon(IconData::font(0xe5d2, "Material Icons")); // Menu icon
        ic1->color(0xFFE2E8F0)->size(24.0f);
        auto btn1 = iconButton(ic1, []{});
        std::vector<WidgetPtr> r1_items = {
            WidgetPtr(std::make_shared<Text>("Standard Menu:", TextStyle{.color = 0xFFCCCCCC})),
            WidgetPtr(btn1)
        };
        auto row1 = row(r1_items);
        row1->gap(StyleValue::point(16.0f)).alignItems(Align::Center);
        
        // 2. Custom Color IconButton
        auto ic2 = icon(IconData::font(0xe87d, "Material Icons")); // Favorite icon
        ic2->color(0xFFEF4444)->size(24.0f);
        auto btn2 = iconButton(ic2, []{});
        btn2->hoverColor(0x33EF4444); // Red tinted hover
        std::vector<WidgetPtr> r2_items = {
            WidgetPtr(std::make_shared<Text>("Custom Tint Hover:", TextStyle{.color = 0xFFCCCCCC})),
            WidgetPtr(btn2)
        };
        auto row2 = row(r2_items);
        row2->gap(StyleValue::point(16.0f)).alignItems(Align::Center);

        // 3. Large IconButton
        auto ic3 = icon(IconData::font(0xe0b0, "Material Icons")); // Call icon
        ic3->color(0xFF10B981)->size(36.0f);
        auto btn3 = iconButton(ic3, []{});
        btn3->size(64.0f);
        std::vector<WidgetPtr> r3_items = {
            WidgetPtr(std::make_shared<Text>("Large Size:", TextStyle{.color = 0xFFCCCCCC})),
            WidgetPtr(btn3)
        };
        auto row3 = row(r3_items);
        row3->gap(StyleValue::point(16.0f)).alignItems(Align::Center);

        std::vector<WidgetPtr> col_items = {WidgetPtr(title), WidgetPtr(row1), WidgetPtr(row2), WidgetPtr(row3)};
        auto col = column(col_items);
        col->gap(StyleValue::point(32.0f)).padding(StyleInsets::all(40.0f));

        auto bg = container(col);
        bg->color(0xFF1E293B); // Dark slate background

        return bg;
    }
};

int main() {
    std::cout << "Starting IconButton Demo..." << std::endl;
    AppConfig config;
    config.title = "ENKI IconButton Demo";
    config.width = 600;
    config.height = 600;
    config.target_fps = 60;
    
    return runApp(std::make_shared<IconButtonDemoWidget>(), config);
}
