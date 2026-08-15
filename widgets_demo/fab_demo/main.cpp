#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/icon.hpp"
#include "enki/widgets/floating_action_button.hpp"
#include "enki/widgets/icons_material.hpp"
#include <iostream>

using namespace enki;

class FABDemoWidget : public StatelessWidget {
public:
    [[nodiscard]] std::string_view typeName() const override { return "FABDemoWidget"; }

    [[nodiscard]] WidgetPtr build(BuildContext& context) override {
        auto title = std::make_shared<Text>("FAB Demo", TextStyle{.color = 0xFFFFFFFF, .font_size = 24.0f, .font_weight = FontWeight::Bold});
        
        // 1. Standard FAB
        auto ic1 = icon(IconData::font(0xe145, "Material Icons")); // Add icon
        ic1->color(0xFFFFFFFF)->size(24.0f);
        auto fab1 = floatingActionButton(ic1, []{});
        std::vector<WidgetPtr> r1_items = {
            WidgetPtr(std::make_shared<Text>("Standard Primary FAB:", TextStyle{.color = 0xFFCCCCCC})),
            WidgetPtr(fab1)
        };
        auto row1 = row(r1_items);
        row1->gap(StyleValue::point(16.0f)).alignItems(Align::Center);
        
        // 2. Custom Color FAB
        auto ic2 = icon(IconData::font(0xe3c9, "Material Icons")); // Edit icon
        ic2->color(0xFFFFFFFF)->size(24.0f);
        auto fab2 = floatingActionButton(ic2, []{});
        fab2->bgColor(0xFF10B981)->hoverColor(0xFF059669); // Emerald Green
        std::vector<WidgetPtr> r2_items = {
            WidgetPtr(std::make_shared<Text>("Custom Color FAB:", TextStyle{.color = 0xFFCCCCCC})),
            WidgetPtr(fab2)
        };
        auto row2 = row(r2_items);
        row2->gap(StyleValue::point(16.0f)).alignItems(Align::Center);

        // 3. Extended / Rounded Rect FAB
        auto ic3 = icon(IconData::font(0xe150, "Material Icons")); // Edit / Write icon
        ic3->color(0xFFFFFFFF)->size(24.0f);
        auto txt3 = std::make_shared<Text>("Compose", TextStyle{.color = 0xFFFFFFFF, .font_size = 16.0f, .font_weight = FontWeight::Bold});
        
        std::vector<WidgetPtr> ext_items = {WidgetPtr(ic3), WidgetPtr(txt3)};
        auto ext_row = row(ext_items);
        ext_row->gap(StyleValue::point(8.0f)).alignItems(Align::Center);
        
        auto fab3 = floatingActionButton(ext_row, []{});
        fab3->bgColor(0xFF8B5CF6)->hoverColor(0xFF7C3AED); // Violet
        fab3->borderRadius(16.0f); // Rounded rect instead of circle
        fab3->size(48.0f); // Adjust min height, width will expand
        
        std::vector<WidgetPtr> r3_items = {
            WidgetPtr(std::make_shared<Text>("Extended FAB:", TextStyle{.color = 0xFFCCCCCC})),
            WidgetPtr(fab3)
        };
        auto row3 = row(r3_items);
        row3->gap(StyleValue::point(16.0f)).alignItems(Align::Center);

        std::vector<WidgetPtr> col_items = {WidgetPtr(title), WidgetPtr(row1), WidgetPtr(row2), WidgetPtr(row3)};
        auto col = column(col_items);
        col->gap(StyleValue::point(40.0f)).padding(StyleInsets::all(40.0f));

        auto bg = container(col);
        bg->color(0xFF1E293B);

        return bg;
    }
};

int main() {
    std::cout << "Starting FloatingActionButton Demo..." << std::endl;
    AppConfig config;
    config.title = "ENKI FloatingActionButton Demo";
    config.width = 600;
    config.height = 600;
    config.target_fps = 60;
    
    return runApp(std::make_shared<FABDemoWidget>(), config);
}
