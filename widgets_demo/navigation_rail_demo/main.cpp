#include "enki/app/app.hpp"
#include "enki/widgets/navigation_rail.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/state/state.hpp"

using namespace enki;

class NavigationRailDemoState : public State {
    int selected_idx = 0;
public:
    WidgetPtr build(BuildContext& ctx) override {
        std::string page_text;
        switch (selected_idx) {
            case 0: page_text = "Dashboard Content"; break;
            case 1: page_text = "Users Management"; break;
            case 2: page_text = "Reports Generation"; break;
            case 3: page_text = "System Settings"; break;
        }

        return container({
            .color = 0xFF0F172A,
            .child = row({
                .height = StyleValue::percent(100.0f),
                .children = {
                    NavigationRail {
                        .items = {
                            {"Dashboard", Icons::Material::dashboard(), ""},
                            {"Analytics", Icons::Material::analytics(), "2"},
                            {"Users",     Icons::Material::people(), ""},
                            {"Settings",  Icons::Material::settings(), ""}
                        },
                        .selected_index = selected_idx,
                        .on_item_selected = [this](int i) {
                            setState([this, i] { selected_idx = i; });
                        }
                    },
                    container({
                        .flex_grow = 1.0f,
                        .child = centerBox(text(page_text, { .color = 0xFFFFFFFF, .font_size = 20.0f }))
                    })
                }
            })
        });
    }
};

class NavigationRailDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override { return std::make_unique<NavigationRailDemoState>(); }
    std::string_view typeName() const override { return "NavigationRailDemoApp"; }
};

int main() {
    AppConfig config;
    config.title       = "Enki Engine — NavigationRail Demo";
    config.width       = 1000;
    config.height      = 700;
    config.clear_color = 0xFF0F172A;
    return runApp(std::make_shared<NavigationRailDemoApp>(), config);
}
