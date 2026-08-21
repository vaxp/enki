#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/icon.hpp"
#include "enki/widgets/floating_action_button.hpp"
#include "enki/widgets/icons_material.hpp"
#include <iostream>

using namespace enki;

class FABDemoWidget : public StatelessWidget {
public:
    [[nodiscard]] std::string_view typeName() const override { return "FABDemoWidget"; }

    [[nodiscard]] WidgetPtr build(BuildContext& context) override {
        return container({
            .color = 0xFF1E293B,
            .padding = StyleInsets::all(40.0f),
            .child = column({
                .gap = StyleValue::point(40.0f),
                .children = {
                    text("FAB Demo", { .color = 0xFFFFFFFF, .font_size = 24.0f, .font_weight = FontWeight::Bold }),
                    
                    // 1. Standard FAB
                    row({
                        .align_items = Align::Center,
                        .gap = StyleValue::point(16.0f),
                        .children = {
                            text("Standard Primary FAB:", { .color = 0xFFCCCCCC }),
                            floatingActionButton({
                                .child = icon(IconData::font(0xe145, "Material Icons"), { .size = 24.0f, .color = 0xFFFFFFFF }),
                                .on_pressed = []{}
                            })
                        }
                    }),
                    
                    // 2. Custom Color FAB
                    row({
                        .align_items = Align::Center,
                        .gap = StyleValue::point(16.0f),
                        .children = {
                            text("Custom Color FAB:", { .color = 0xFFCCCCCC }),
                            floatingActionButton({
                                .child = icon(IconData::font(0xe3c9, "Material Icons"), { .size = 24.0f, .color = 0xFFFFFFFF }),
                                .on_pressed = []{},
                                .normal_color = 0xFF10B981,
                                .hover_color = 0xFF059669
                            })
                        }
                    }),

                    // 3. Extended / Rounded Rect FAB
                    row({
                        .align_items = Align::Center,
                        .gap = StyleValue::point(16.0f),
                        .children = {
                            text("Extended FAB:", { .color = 0xFFCCCCCC }),
                            floatingActionButton({
                                .child = row({
                                    .align_items = Align::Center,
                                    .gap = StyleValue::point(8.0f),
                                    .children = {
                                        icon(IconData::font(0xe150, "Material Icons"), { .size = 24.0f, .color = 0xFFFFFFFF }),
                                        text("Compose", { .color = 0xFFFFFFFF, .font_size = 16.0f, .font_weight = FontWeight::Bold })
                                    }
                                }),
                                .on_pressed = []{},
                                .normal_color = 0xFF8B5CF6,
                                .hover_color = 0xFF7C3AED,
                                .size = 48.0f,
                                .border_radius = 16.0f
                            })
                        }
                    })
                }
            })
        });
    }
};

int main() {
    std::cout << "Starting FloatingActionButton Demo..." << std::endl;
    AppConfig config;
    config.title = "ENKI FloatingActionButton Demo";
    config.width = 600;
    config.height = 600;
    config.target_fps = 60;
    
    return runApp(std::make_shared<FABDemoWidget>(), config);
}
