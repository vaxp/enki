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
        return container({
            .color = 0xFF0F172A,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .child = column({
                .height = StyleValue::percent(100.0f),
                .children = {
                    tabBar({
                        .tabs = {
                            {"Discover", Icons::Material::explore(), ""},
                            {"Library", Icons::Material::library_books(), ""},
                            {"Messages", Icons::Material::chat(), "3"},
                            {"Settings", Icons::Material::settings(), ""}
                        },
                        .selected_index = selected_tab,
                        .on_tab_changed = [this](int idx) {
                            setState([this, idx] { selected_tab = idx; });
                        }
                    }),
                    container({
                        .flex_grow = 1.0f,
                        .child = tabView({
                            .selected_index = selected_tab,
                            .children = {
                                centerBox(text("Welcome to the Home Page", { .color = 0xFFFFFFFF })),
                                centerBox(text("Analytics Dashboard", { .color = 0xFFFFFFFF })),
                                centerBox(text("You have 3 unread messages", { .color = 0xFFFFFFFF })),
                                centerBox(text("Settings Configuration", { .color = 0xFFFFFFFF }))
                            }
                        })
                    })
                }
            })
        });
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
