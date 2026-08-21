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
        
        return container({
            .color = 0xFF0F172A,
            .padding = StyleInsets::all(20.0f),
            .child = column({
                .children = {
                    breadcrumb({
                        .items = items
                    }),
                    container({
                        .margin = StyleInsets::all(20.0f),
                        .child = gestureDetector({
                            .child = container({
                                .color = 0xFF3B82F6,
                                .border_radius = BorderRadius::circular(4.0f),
                                .padding = StyleInsets::symmetric(8.0f, 16.0f),
                                .child = text("Go Deeper")
                            }),
                            .on_tap = [this] {
                                setState([this] {
                                    path_names.push_back("Item " + std::to_string(path_names.size()));
                                });
                            }
                        })
                    })
                }
            })
        });
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
