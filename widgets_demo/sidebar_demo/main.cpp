#include "enki/app/app.hpp"
#include "enki/widgets/sidebar.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/state/state.hpp"

using namespace enki;

class SidebarDemoState : public State {
public:
    WidgetPtr build(BuildContext& ctx) override {
        return sidebar({
            .sidebar_content = container({
                .align = Alignment::TopCenter,
                .padding = StyleInsets::all(20.0f),
                .child = text("Sidebar Area")
            }),
            .body = container({
                .color = 0xFF0F172A,
                .flex_grow = 1.0f,
                .child = centerBox(text("Main Body Area"))
            }),
            .options = {
                .expanded_width = 250.0f,
                .collapsed_width = 60.0f,
                .background_color = 0xFF1E293B,
                .side = SidebarSide::Left
            }
        });
    }
};

class SidebarDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override { return std::make_unique<SidebarDemoState>(); }
    std::string_view typeName() const override { return "SidebarDemoApp"; }
};

int main() {
    AppConfig config;
    config.title       = "Enki Engine — Sidebar Demo";
    config.width       = 1000;
    config.height      = 700;
    config.clear_color = 0xFF0F172A;
    return runApp(std::make_shared<SidebarDemoApp>(), config);
}
