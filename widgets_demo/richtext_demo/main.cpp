#include <enki/app/app.hpp>
#include <enki/widgets/flexbox.hpp>
#include <enki/widgets/container.hpp>
#include <enki/widgets/text.hpp>
#include <enki/state/state.hpp>
#include <iostream>

using namespace enki;

class RichTextDemoState : public State {
    bool link_hovered = false;
    bool user_hovered = false;

public:
    WidgetPtr build(BuildContext&) override {
        auto title = text("RichText Demo");
        title->fontSize(24.0f).bold().color(0xFFF1F5F9);

        // Create interactive spans
        auto interactive_link = span("Clickable Link");
        interactive_link->style = TextStyle().setColor(link_hovered ? 0xFF7DD3FC : 0xFF38BDF8).underline(link_hovered ? 0xFF7DD3FC : 0xFF38BDF8);
        interactive_link->onClick([]() {
            std::cout << "Link was clicked!\n";
        }).onHover([this](bool hovered) {
            setState([this, hovered]() { link_hovered = hovered; });
        });

        auto interactive_user = span("@vaxp");
        interactive_user->style = TextStyle().setColor(user_hovered ? 0xFFFBCFE8 : 0xFFF472B6).bold();
        interactive_user->onClick([]() {
            std::cout << "User mention clicked!\n";
        }).onHover([this](bool hovered) {
            setState([this, hovered]() { user_hovered = hovered; });
        });

        auto combined_span = span("", std::nullopt, {
            span("Welcome to the "),
            span("ENKI Framework", TextStyle().bold()),
            span(". This is a demonstration of the "),
            interactive_link,
            span(" feature which allows you to embed interactive and styled text right within a paragraph! "),
            span("Special thanks to "),
            interactive_user,
            span(" for testing this out.")
        });

        auto rt = richText(combined_span);
        rt->defaultStyle(TextStyle().setColor(0xFF94A3B8).setFontSize(16.0f).setHeight(1.5f));
        rt->maxLines(4);

        auto root = column({title, rt});
        root->padding(StyleInsets{40.0f, 40.0f, 40.0f, 40.0f}).gap(20.0f);
        
        auto root_container = container(root);
        root_container->color(0xFF0F172A); // Slate 900
        return root_container;
    }
};

class RichTextDemo : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<RichTextDemoState>();
    }
    std::string_view typeName() const override { return "RichTextDemo"; }
};

int main(int argc, char** argv) {
    std::cout << "Starting RichText Demo...\n"; 
    AppConfig config;
    config.title = "ENKI RichText Demo";
    config.width = 600;
    config.height = 400;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 0; // Uncapped max speed
    config.show_performance_overlay = true; // Display real-time FPS & Frame Time HUD
    config.clear_color = 0xFF0B0F19;
    
    return runApp(std::make_shared<RichTextDemo>(), config);
}
