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
        SidebarOptions opts;
        opts.background_color = 0xFF1E293B;
        opts.expanded_width = 250.0f;
        opts.collapsed_width = 60.0f;
        opts.side = SidebarSide::Left;
        
        auto sb_content = container(text("Sidebar Area"));
        sb_content->paddingAll(20.0f).align(Alignment::TopCenter);

        auto body_content = centerBox(text("Main Body Area"));
        auto body_container = container(body_content);
        body_container->color(0xFF0F172A).flex(1.0f);

        auto sb = sidebar(sb_content, body_container, opts);
        
        return sb;
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
