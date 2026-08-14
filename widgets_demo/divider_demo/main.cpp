#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/divider.hpp"
#include "enki/state/state.hpp"

#include <iostream>

using namespace enki;

class DividerDemoWidget : public StatelessWidget {
public:
    std::string_view typeName() const override { return "DividerDemoWidget"; }

    WidgetPtr build(BuildContext&) override {
        auto title = std::make_shared<Text>("Divider Widget Demo", TextStyle{.color = 0xFFFFFFFF, .font_size = 24.0f});
        
        DividerOptions opt1;
        opt1.color = 0xFFFF0000; // Red
        opt1.thickness = 2.0f;
        auto div1 = divider(opt1);

        auto text1 = std::make_shared<Text>("Above Red Divider", TextStyle{.color = 0xFFCCCCCC});
        auto text2 = std::make_shared<Text>("Below Red Divider", TextStyle{.color = 0xFFCCCCCC});
        
        DividerOptions opt2;
        opt2.color = 0xFF00FF00; // Green
        opt2.thickness = 4.0f;
        opt2.indent = 50.0f;
        opt2.end_indent = 50.0f;
        auto div2 = divider(opt2);

        auto col = column({title, text1, div1, text2, div2});
        col->gap(StyleValue::point(16.0f));
        col->padding(StyleInsets::all(24.0f));

        auto bg = container(col);
        bg->color(0xFF1E1E1E);

        return bg;
    }
};

int main() {
    std::cout << "Starting Divider Demo...\n";
    AppConfig config;
    config.title = "ENKI Divider Demo";
    config.width = 600;
    config.height = 400;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 0; // Uncapped max speed
    config.show_performance_overlay = true; // Display real-time FPS & Frame Time HUD
    config.clear_color = 0xFF0B0F19;
    
    return runApp(std::make_shared<DividerDemoWidget>(), config);
}
