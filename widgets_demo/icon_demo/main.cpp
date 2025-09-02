#include "enki/app/app.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/icon.hpp"
#include "enki/rendering/font_manager.hpp"
#include "enki/state/state.hpp"
#include <iostream>

using namespace enki;

class IconDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override;
    std::string_view typeName() const override { return "IconDemoApp"; }
};

class IconDemoState : public State {
public:
    void initState() override {
        State::initState(); // Required for lifecycle
        
        // Load custom icon font
        bool loaded = FontManager::loadFont("assets/fonts/MaterialIcons-Regular.ttf", "Material Icons");
        if (!loaded) {
            std::cerr << "Warning: Could not load Material Icons font." << std::endl;
        }
    }

    WidgetPtr build(BuildContext& context) override {
        // SVG Icons Section
        auto svg_section = column({
            .children = {
                container({
                    .margin = StyleInsets::symmetric(0.0f, 10.0f),
                    .child = text("SVG Icons", { .color = 0xFF94A3B8, .font_size = 18.0f })
                }),
                row({
                    .align_items = Align::Center,
                    .gap = StyleValue::point(20.0f),
                    .children = {
                        Icon { .data = Icons::SVG::play(),  .size = 32.0f, .color = 0xFF10B981 },
                        Icon { .data = Icons::SVG::check(), .size = 48.0f, .color = 0xFF3B82F6 },
                        Icon { .data = Icons::SVG::play(),  .size = 64.0f, .color = 0xFFEF4444 },
                    }
                })
            }
        });

        // Material Icons Section
        auto font_section = column({
            .children = {
                container({
                    .margin = StyleInsets::symmetric(0.0f, 20.0f),
                    .child = text("Material Icons (Font)", { .color = 0xFF94A3B8, .font_size = 18.0f })
                }),
                row({
                    .align_items = Align::Center,
                    .gap = StyleValue::point(20.0f),
                    .children = {
                        Icon { .data = Icons::Material::favorite(), .size = 32.0f, .color = 0xFFEC4899 },
                        Icon { .data = Icons::Material::home(),     .size = 48.0f, .color = 0xFF8B5CF6 },
                        Icon { .data = Icons::Material::settings(), .size = 64.0f, .color = 0xFF6366F1 },
                        Icon { .data = Icons::Material::search(),   .size = 48.0f, .color = 0xFFF59E0B },
                        Icon { .data = Icons::Material::add(),      .size = 32.0f, .color = 0xFF10B981 },
                    }
                })
            }
        });

        // Main Layout
        auto main_col = column({
            .justify_content = Justify::Center,
            .align_items = Align::Center,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .children = {
                text("ENKI Icon Widget", { .color = 0xFFFFFFFF, .font_size = 28.0f, .font_weight = FontWeight::Bold }),
                container({
                    .margin = StyleInsets::symmetric(0.0f, 20.0f),
                    .child = text("Zero calculations. 100% Anu Layout. Native SVG & Fonts.", { .color = 0xFF94A3B8, .font_size = 14.0f })
                }),
                svg_section,
                font_section
            }
        });

        return container({
            .color = 0xFF0F172A,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .child = main_col
        });
    }
};

std::unique_ptr<State> IconDemoApp::createState() {
    return std::make_unique<IconDemoState>();
}

int main() {
    AppConfig config;
    config.title       = "Enki Engine — Icon Demo";
    config.width       = 800;
    config.height      = 600;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 0;
    config.clear_color = 0xFF0F172A;

    return runApp(std::make_shared<IconDemoApp>(), config);
}
