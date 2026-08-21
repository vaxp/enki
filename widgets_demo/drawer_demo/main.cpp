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
        return drawer({
            .child = container({
                .align = Alignment::TopLeft,
                .padding = StyleInsets::all(20.0f),
                .child = text("Drawer Menu")
            }),
            .body = container({
                .color = 0xFF0F172A,
                .flex = 1.0f,
                .child = centerBox(button(text("Open Drawer"), [this] {
                    drawer_ctrl->open();
                }))
            }),
            .options = {
                .width = 300.0f,
                .background_color = 0xFF1E293B
            },
            .controller = drawer_ctrl
        });
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
