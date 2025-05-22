#include "enki/app/app.hpp"
#include "enki/widgets/navigation_bar.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/state/state.hpp"

using namespace enki;

class NavigationBarDemoState : public State {
    int selected_idx = 0;
public:
    WidgetPtr build(BuildContext& ctx) override {
        auto nav_bar = navigationBar({
            NavigationBarItem{"Explore", Icons::Material::search(), ""},
            NavigationBarItem{"Saved", Icons::Material::favorite(), ""},
            NavigationBarItem{"Notifications", Icons::Material::notifications(), "5"},
            NavigationBarItem{"Profile", Icons::Material::person(), ""}
        }, selected_idx, [this](int i) {
            setState([this, i] { selected_idx = i; });
        });

        std::string page_text;
        switch (selected_idx) {
            case 0: page_text = "Explore Page"; break;
            case 1: page_text = "Saved Items"; break;
            case 2: page_text = "Notifications (5)"; break;
            case 3: page_text = "User Profile"; break;
        }

        auto body = centerBox(text(page_text));
        auto body_container = container(body);
        body_container->flex(1.0f);

        auto col = std::make_shared<Column>(std::vector<WidgetPtr>{
            std::static_pointer_cast<Widget>(body_container),
            std::static_pointer_cast<Widget>(nav_bar)
        });
        col->style.height = StyleValue::percent(100.0f);

        auto root = container(col);
        root->color(0xFF0F172A);
        return root;
    }
};

class NavigationBarDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override { return std::make_unique<NavigationBarDemoState>(); }
    std::string_view typeName() const override { return "NavigationBarDemoApp"; }
};

int main() {
    AppConfig config;
    config.title       = "Enki Engine — NavigationBar Demo";
    config.width       = 400;
    config.height      = 800;
    config.clear_color = 0xFF0F172A;
    return runApp(std::make_shared<NavigationBarDemoApp>(), config);
}
