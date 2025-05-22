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
        auto t = text("Details Page");
        t->fontSize(24.0f);
        
        auto btn = button(text("Go Back"), [ctx] {
            auto c = ctx;
            Navigator::pop(c);
        });
        
        auto col = std::make_shared<Column>(std::vector<WidgetPtr>{
            std::static_pointer_cast<Widget>(t),
            std::static_pointer_cast<Widget>(btn)
        });
        col->style.align_items = Align::Center;
        col->style.justify_content = Justify::Center;
        col->style.gap = StyleValue::point(20.0f);
        
        auto root = container(col);
        root->color(0xFF1E293B).width(StyleValue::percent(100.0f)).height(StyleValue::percent(100.0f));
        return root;
    }
    std::string_view typeName() const override { return "DetailsPage"; }
};

class HomePage : public StatelessWidget {
public:
    WidgetPtr build(BuildContext& ctx) override {
        auto t = text("Home Page");
        t->fontSize(24.0f);
        
        auto btn = button(text("Go to Details"), [ctx] {
            std::function<WidgetPtr()> builder = []() -> WidgetPtr { 
                return std::make_shared<DetailsPage>(); 
            };
            auto c = ctx;
            Navigator::push(c, RouteConfig{"details", builder});
        });
        
        auto col = std::make_shared<Column>(std::vector<WidgetPtr>{
            std::static_pointer_cast<Widget>(t),
            std::static_pointer_cast<Widget>(btn)
        });
        col->style.align_items = Align::Center;
        col->style.justify_content = Justify::Center;
        col->style.gap = StyleValue::point(20.0f);
        
        auto root = container(col);
        root->color(0xFF0F172A).width(StyleValue::percent(100.0f)).height(StyleValue::percent(100.0f));
        return root;
    }
    std::string_view typeName() const override { return "HomePage"; }
};

class NavigatorDemoApp : public StatelessWidget {
public:
    WidgetPtr build(BuildContext& ctx) override {
        std::vector<RouteConfig> routes = {
            RouteConfig{"home", []() { return std::make_shared<HomePage>(); }}
        };
        
        return navigator(routes);
    }
    std::string_view typeName() const override { return "NavigatorDemoApp"; }
};

int main() {
    AppConfig config;
    config.title       = "Enki Engine — Navigator Demo";
    config.width       = 800;
    config.height      = 600;
    config.clear_color = 0xFF0F172A;
    return runApp(std::make_shared<NavigatorDemoApp>(), config);
}
