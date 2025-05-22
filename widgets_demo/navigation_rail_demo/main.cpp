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
        auto rail = navigationRail({
            NavigationRailItem{"Dashboard", Icons::Material::dashboard(), ""},
            NavigationRailItem{"Analytics", Icons::Material::analytics(), "2"},
            NavigationRailItem{"Users",     Icons::Material::people(), ""},
            NavigationRailItem{"Settings",  Icons::Material::settings(), ""}
        }, selected_idx, [this](int i) {
            setState([this, i] { selected_idx = i; });
        });

        std::string page_text;
        switch (selected_idx) {
            case 0: page_text = "Dashboard Content"; break;
            case 1: page_text = "Users Management"; break;
            case 2: page_text = "Reports Generation"; break;
            case 3: page_text = "System Settings"; break;
        }

        auto body = centerBox(text(page_text));
        auto body_container = container(body);
        body_container->flex(1.0f);

        auto row_layout = std::make_shared<Row>(std::vector<WidgetPtr>{
            std::static_pointer_cast<Widget>(rail),
            std::static_pointer_cast<Widget>(body_container)
        });
        row_layout->style.height = StyleValue::percent(100.0f);

        auto root = container(row_layout);
        root->color(0xFF0F172A);
        return root;
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
