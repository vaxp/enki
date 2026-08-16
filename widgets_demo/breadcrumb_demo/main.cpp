#include "enki/app/app.hpp"
#include "enki/widgets/breadcrumb.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/state/state.hpp"

using namespace enki;

class BreadcrumbDemoState : public State {
    std::vector<std::string> path_names = {"Home", "Products", "Electronics", "Laptops"};

    WidgetPtr build(BuildContext& ctx) override {
        std::vector<BreadcrumbItem> items;
        for (size_t i = 0; i < path_names.size(); ++i) {
            items.push_back(BreadcrumbItem{
                path_names[i],
                [this, i]() {
                    setState([this, i] {
                        path_names.erase(path_names.begin() + i + 1, path_names.end());
                    });
                }
            });
        }
        
        auto bc = breadcrumb(items, BreadcrumbOptions{});

        auto btn_content = container(text("Go Deeper"));
        btn_content->color(0xFF3B82F6).paddingSymmetric(8.0f, 16.0f).borderRadius(4.0f);
        
        auto add_btn = std::make_shared<GestureDetector>(btn_content);
        add_btn->on_tap = [this] {
            setState([this] {
                path_names.push_back("Item " + std::to_string(path_names.size()));
            });
        };

        auto btn_margin = container(add_btn);
        btn_margin->marginAll(20.0f);

        auto col = std::make_shared<Column>(std::vector<WidgetPtr>{
            std::static_pointer_cast<Widget>(bc),
            std::static_pointer_cast<Widget>(btn_margin)
        });

        auto root = container(col);
        root->color(0xFF0F172A).paddingAll(20.0f);
        return root;
    }
};

class BreadcrumbDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override { return std::make_unique<BreadcrumbDemoState>(); }
    std::string_view typeName() const override { return "BreadcrumbDemoApp"; }
};

int main() {
    AppConfig config;
    config.title       = "Enki Engine — Breadcrumb Demo";
    config.width       = 800;
    config.height      = 600;
    config.clear_color = 0xFF0F172A;
    return runApp(std::make_shared<BreadcrumbDemoApp>(), config);
}
