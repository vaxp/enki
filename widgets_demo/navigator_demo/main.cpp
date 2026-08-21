#include "enki/app/app.hpp"
#include "enki/widgets/navigator.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/tree/build_context.hpp"

using namespace enki;

class DetailsPage : public StatelessWidget {
public:
    WidgetPtr build(BuildContext& ctx) override {
        return container({
            .color = 0xFF1E293B,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .child = column({
                .justify_content = Justify::Center,
                .align_items = Align::Center,
                .gap = StyleValue::point(20.0f),
                .children = {
                    text("Details Page", { .font_size = 24.0f }),
                    button(text("Go Back"), [ctx] {
                        auto c = ctx;
                        Navigator::pop(c);
                    })
                }
            })
        });
    }
    std::string_view typeName() const override { return "DetailsPage"; }
};

class HomePage : public StatelessWidget {
public:
    WidgetPtr build(BuildContext& ctx) override {
        return container({
            .color = 0xFF0F172A,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .child = column({
                .justify_content = Justify::Center,
                .align_items = Align::Center,
                .gap = StyleValue::point(20.0f),
                .children = {
                    text("Home Page", { .font_size = 24.0f }),
                    button(text("Go to Details"), [ctx] {
                        auto c = ctx;
                        Navigator::push(c, RouteConfig("details", []{ return std::make_shared<DetailsPage>(); }));
                    })
                }
            })
        });
    }
    std::string_view typeName() const override { return "HomePage"; }
};

class NavigatorDemoApp : public StatelessWidget {
public:
    WidgetPtr build(BuildContext& ctx) override {
        return navigator({
            .initial_routes = {
                RouteConfig("home", []{ return std::make_shared<HomePage>(); })
            }
        });
    }
    std::string_view typeName() const override { return "NavigatorDemoApp"; }
};

int main() {
    AppConfig config;
    config.title = "Enki - Navigator Demo";
    config.width = 600;
    config.height = 400;
    
    return runApp(std::make_shared<NavigatorDemoApp>(), config);
}
