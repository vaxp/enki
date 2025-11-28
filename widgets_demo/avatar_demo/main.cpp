#include "enki/app/app.hpp"
#include "enki/widgets/avatar.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/state/state.hpp"
#include <iostream>

using namespace enki;

class AvatarDemoState : public State {
public:
    WidgetPtr build(BuildContext& ctx) override {
        return container({
            .color = 0x4D000000,
            .align = Alignment::Center,
            .child = column({
                .justify_content = Justify::Center,
                .align_items = Align::Center,
                .children = {
                    column({
                        .align_items = Align::Center,
                        .margin = StyleInsets::only(0, 0, 40.0f, 0),
                        .children = {
                            text("Advanced Avatar Widget Demo", { .color = 0xFFFFFFFF, .font_size = 24.0f, .font_weight = FontWeight::Bold }),
                            text("Initials, Images, Badges, and Avatar Groups in ENKI", { .color = 0xFF94A3B8, .font_size = 14.0f })
                        }
                    }),
                    row({
                        .justify_content = Justify::Center,
                        .align_items = Align::Center,
                        .gap = StyleValue::point(30.0f),
                        .margin = StyleInsets::only(0, 0, 40.0f, 0),
                        .children = {
                            // 1. Initials Avatar
                            Avatar {
                                .background_color = 0xFF8B5CF6, // Purple
                                .initials = "MK"
                            },
                            
                            // 2. Avatar with Status Badge
                            Avatar {
                                .radius = 32.0f,
                                .background_color = 0xFFF59E0B, // Amber
                                .initials = "TS",
                                .show_badge = true,
                                .badge_color = 0xFF10B981 // Green online
                            },
                            
                            // 3. Avatar with Image and Border
                            Avatar {
                                .radius = 40.0f,
                                .background_color = 0x8D000000,
                                .image_path = "/home/x/Work/enki/assets/vaxp.png",
                                .border_width = 3.0f,
                                .border_color = 0x7D000000,
                                .shadow_blur = 10.0f
                            }
                        }
                    }),
                    column({
                        .align_items = Align::Center,
                        .gap = StyleValue::point(10.0f),
                        .children = {
                            // 4. Avatar Group
                            AvatarGroup {
                                .avatars = {
                                    Avatar { .radius = 20.0f, .background_color = 0xFFEF4444, .initials = "A", .border_width = 2.0f },
                                    Avatar { .radius = 20.0f, .background_color = 0x6D000000, .initials = "B", .border_width = 2.0f },
                                    Avatar { .radius = 20.0f, .background_color = 0xFF10B981, .initials = "C", .border_width = 2.0f },
                                    Avatar { .radius = 20.0f, .background_color = 0xFFF59E0B, .initials = "D", .border_width = 2.0f },
                                    Avatar { .radius = 20.0f, .background_color = 0xFF8B5CF6, .initials = "E", .border_width = 2.0f }
                                },
                                .spacing = -12.0f,
                                .max_avatars = 3
                            },
                            text("Avatar Group (Max 3)", { .color = 0xFF94A3B8, .font_size = 14.0f })
                        }
                    })
                }
            })
        });
    }
};

class AvatarDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<AvatarDemoState>();
    }
    std::string_view typeName() const override { return "AvatarDemoApp"; }
};

int main() {
    std::cout << "================================================\n";
    std::cout << "  ENKI Engine — Avatar Widget Demo   \n";
    std::cout << "================================================\n";

    AppConfig config;
    config.title       = "Enki Engine — Avatar Demo";
    config.width       = 800;
    config.height      = 500;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 0;
    config.show_performance_overlay = true;
    config.clear_color = 0x0000004D;

    return runApp(std::make_shared<AvatarDemoApp>(), config);
}
