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
        auto title = text({
            .text = "RichText Demo",
            .color = 0xFFF1F5F9,
            .font_size = 24.0f,
            .font_weight = FontWeight::Bold,
        });

        // Create interactive spans
        auto interactive_link = span({
            .text = "Clickable Link",
            .style = TextStyle{
                .color = link_hovered ? 0xFF7DD3FC : 0xFF38BDF8,
                .decoration = TextDecoration::Underline,
                .decoration_color = link_hovered ? 0xFF7DD3FC : 0xFF38BDF8,
            },
            .on_click = []() {
                std::cout << "Link was clicked!\n";
            },
            .on_hover = [this](bool hovered) {
                setState([this, hovered]() { link_hovered = hovered; });
            },
        });

        auto interactive_user = span({
            .text = "@vaxp",
            .style = TextStyle{
                .color = user_hovered ? 0xFFFBCFE8 : 0xFFF472B6,
                .font_weight = FontWeight::Bold,
            },
            .on_click = []() {
                std::cout << "User mention clicked!\n";
            },
            .on_hover = [this](bool hovered) {
                setState([this, hovered]() { user_hovered = hovered; });
            },
        });

        auto combined_span = span({
            .text = "",
            .children = {
                span("Welcome to the "),
                span("ENKI Framework", TextStyle{ .font_weight = FontWeight::Bold }),
                span(". This is a demonstration of the "),
                interactive_link,
                span(" feature which allows you to embed interactive and styled text right within a paragraph! "),
                span("Special thanks to "),
                interactive_user,
                span(" for testing this out.")
            }
        });

        auto rt = richText({
            .text_span = combined_span,
            .default_style = TextStyle{
                .color = 0xFF94A3B8,
                .font_size = 16.0f,
                .height = 1.5f,
            },
            .max_lines = 4,
        });

        auto root = column({
            .gap = 20_px,
            .padding = StyleInsets{40.0f, 40.0f, 40.0f, 40.0f},
            .children = { title, rt }
        });
        
        auto root_container = container({
            .color = 0xFF0F172A, // Slate 900
            .child = root,
        });
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
