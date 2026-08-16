#include "enki/app/app.hpp"
#include "enki/widgets/drawer.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/state/state.hpp"

using namespace enki;

class DrawerDemoState : public State {
    std::shared_ptr<DrawerController> drawer_ctrl;
public:
    void initState() override {
        State::initState();
        drawer_ctrl = std::make_shared<DrawerController>();
    }

    WidgetPtr build(BuildContext& ctx) override {
        DrawerOptions opts;
        opts.background_color = 0xFF1E293B;
        opts.width = 300.0f;
        
        auto drawer_content = container(text("Drawer Menu"));
        drawer_content->paddingAll(20.0f).align(Alignment::TopLeft);

        auto open_btn = button(text("Open Drawer"), [this] {
            drawer_ctrl->open();
        });

        auto body = centerBox(open_btn);
        auto body_container = container(body);
        body_container->color(0xFF0F172A).flex(1.0f);

        auto d = drawer(drawer_content, body_container, opts);
        d->setController(drawer_ctrl);
        return d;
    }
};

class DrawerDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override { return std::make_unique<DrawerDemoState>(); }
    std::string_view typeName() const override { return "DrawerDemoApp"; }
};

int main() {
    AppConfig config;
    config.title       = "Enki Engine — Drawer Demo";
    config.width       = 800;
    config.height      = 600;
    config.clear_color = 0xFF0F172A;
    return runApp(std::make_shared<DrawerDemoApp>(), config);
}
