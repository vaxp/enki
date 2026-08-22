#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/badge.hpp"
#include "enki/widgets/icon.hpp"

#include <iostream>

using namespace enki;

class BadgeDemoWidget : public StatelessWidget {
public:
    std::string_view typeName() const override { return "BadgeDemoWidget"; }

    WidgetPtr build(BuildContext&) override {
        return container({
            .color = 0xFF0F172A,
            .child = column({
                .gap = StyleValue::point(32.0f),
                .padding = StyleInsets::all(40.0f),
                .children = {
                    text("Badge Widget Demo", { .color = 0xFFFFFFFF, .font_size = 24.0f }),
                    
                    // Notification dot on an icon
                    row({
                        .align_items = Align::Center,
                        .gap = StyleValue::point(16.0f),
                        .children = {
                            Badge {
                                .child = container({
                                    .color = 0xFF2D3748,
                                    .border_radius = BorderRadius::circular(8.0f),
                                    .padding = StyleInsets::all(8.0f),
                                    .child = text("🔔", { .font_size = 24.0f })
                                }),
                                .bg_color = 0xFFEF4444,
                                .offset = {4.0f, -4.0f},
                                .size = 14.0f
                            },
                            text("Status Dot Badge", { .color = 0xFFCCCCCC })
                        }
                    }),

                    // Counter badge with text
                    row({
                        .align_items = Align::Center,
                        .gap = StyleValue::point(16.0f),
                        .children = {
                            Badge {
                                .child = container({
                                    .color = 0xFF2D3748,
                                    .border_radius = BorderRadius::circular(8.0f),
                                    .padding = StyleInsets::all(8.0f),
                                    .child = text("✉️", { .font_size = 24.0f })
                                }),
                                .label = text("3", { .color = 0xFFFFFFFF, .font_size = 10.0f, .font_weight = FontWeight::Bold }),
                                .bg_color = 0xFF3B82F6, // Blue
                                .offset = {6.0f, -6.0f}
                            },
                            text("Counter Badge", { .color = 0xFFCCCCCC })
                        }
                    }),

                    // Left-aligned badge (Wait, BottomRight aligned actually)
                    row({
                        .align_items = Align::Center,
                        .gap = StyleValue::point(16.0f),
                        .children = {
                            Badge {
                                .child = container({
                                    .color = 0xFF2D3748,
                                    .border_radius = BorderRadius::circular(8.0f),
                                    .padding = StyleInsets::all(8.0f),
                                    .child = text("👤", { .font_size = 24.0f })
                                }),
                                .bg_color = 0xFF10B981,
                                .alignment = Alignment::BottomRight,
                                .offset = {2.0f, 2.0f},
                                .size = 12.0f
                            },
                            text("Bottom Right Alignment", { .color = 0xFFCCCCCC })
                        }
                    })
                }
            })
        });
    }
};

int main() {
    std::cout << "Starting Badge Demo...\n";
    AppConfig config;
    config.title = "ENKI Badge Demo";
    config.width = 600;
    config.height = 400;
    config.target_fps = 60;
    
    return runApp(std::make_shared<BadgeDemoWidget>(), config);
}
