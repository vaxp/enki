#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/icon.hpp"
#include "enki/widgets/icon_button.hpp"
#include "enki/widgets/icons_material.hpp"
#include "enki/widgets/stack.hpp"
#include <iostream>

using namespace enki;

class IconButtonDemoWidget : public StatelessWidget {
public:
    [[nodiscard]] std::string_view typeName() const override { return "IconButtonDemoWidget"; }

    [[nodiscard]] WidgetPtr build(BuildContext& context) override {
        return container({
            .color = 0xFF1E293B, // Dark slate background
            .child = column({
                .gap = StyleValue::point(32.0f),
                .padding = StyleInsets::all(40.0f),
                .children = {
                    text("IconButton Demo", { .color = 0xFFFFFFFF, .font_size = 24.0f, .font_weight = FontWeight::Bold }),
                    
                    // 1. Standard IconButton
                    row({
                        .align_items = Align::Center,
                        .gap = StyleValue::point(16.0f),
                        .children = {
                            text("Standard Menu:", { .color = 0xFFCCCCCC }),
                            IconButton {
                                .icon = icon(IconData::font(0xe5d2, "Material Icons"), { .size = 24.0f, .color = 0xFFE2E8F0 }),
                                .on_pressed = []{}
                            }
                        }
                    }),
                    
                    // 2. Custom Color IconButton
                    row({
                        .align_items = Align::Center,
                        .gap = StyleValue::point(16.0f),
                        .children = {
                            text("Custom Tint Hover:", { .color = 0xFFCCCCCC }),
                            IconButton {
                                .icon = icon(IconData::font(0xe87d, "Material Icons"), { .size = 24.0f, .color = 0xFFEF4444 }),
                                .on_pressed = []{},
                                .hover_color = 0x33EF4444 // Red tinted hover
                            }
                        }
                    }),

                    // 3. Large IconButton
                    row({
                        .align_items = Align::Center,
                        .gap = StyleValue::point(16.0f),
                        .children = {
                            text("Large Size:", { .color = 0xFFCCCCCC }),
                            IconButton {
                                .icon = icon(IconData::font(0xe0b0, "Material Icons"), { .size = 36.0f, .color = 0xFF10B981 }),
                                .on_pressed = []{},
                                .size = 64.0f
                            }
                        }
                    })
                }
            })
        });
    }
};

int main() {
    std::cout << "Starting IconButton Demo..." << std::endl;
    AppConfig config;
    config.title = "ENKI IconButton Demo";
    config.width = 600;
    config.height = 600;
    config.target_fps = 60;
    
    return runApp(std::make_shared<IconButtonDemoWidget>(), config);
}
