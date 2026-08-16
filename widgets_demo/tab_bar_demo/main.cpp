#include "enki/app/app.hpp"
#include "enki/widgets/tab_bar.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/state/state.hpp"
#include <iostream>

using namespace enki;

class TabBarDemoState : public State {
    int selected_tab = 0;
public:
    WidgetPtr build(BuildContext& ctx) override {
        // TabBar
        auto tabs = tabBar({
            TabItem{"Discover", Icons::Material::explore(), ""},
            TabItem{"Library", Icons::Material::library_books(), ""},
            TabItem{"Messages", Icons::Material::chat(), "3"},
            TabItem{"Settings", Icons::Material::settings(), ""}
        }, selected_tab, [this](int idx) {
            setState([this, idx] { selected_tab = idx; });
        });

        auto home_page = centerBox(text("Welcome to the Home Page"));
        auto analytics_page = centerBox(text("Analytics Dashboard"));
        auto messages_page = centerBox(text("You have 3 unread messages"));
        auto settings_page = centerBox(text("Settings Configuration"));

        auto content = tabView(selected_tab, {
            home_page, analytics_page, messages_page, settings_page
        });

        auto flex_content = container(content);
        flex_content->flex(1.0f);

        auto col = std::make_shared<Column>(std::vector<WidgetPtr>{
            std::static_pointer_cast<Widget>(tabs),
            std::static_pointer_cast<Widget>(flex_content)
        });

        col->style.height = StyleValue::percent(100.0f);

        auto root = container(col);
        root->color(0xFF0F172A).width(StyleValue::percent(100.0f)).height(StyleValue::percent(100.0f));

        return root;
    }
};

class TabBarDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override { return std::make_unique<TabBarDemoState>(); }
    std::string_view typeName() const override { return "TabBarDemoApp"; }
};

int main() {
    AppConfig config;
    config.title       = "Enki Engine — TabBar Demo";
    config.width       = 800;
    config.height      = 600;
    config.clear_color = 0xFF0F172A;
    return runApp(std::make_shared<TabBarDemoApp>(), config);
}
