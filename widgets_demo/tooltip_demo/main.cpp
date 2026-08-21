/// @file main.cpp
/// @brief ENKI Advanced Tooltip Widget Interactive Showcase.
/// Demonstrates native compositor tooltips, rich content, smart positioning, and custom styling using standard App & runApp.

#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/tooltip.hpp"
#include "enki/widgets/badge.hpp"
#include "enki/widgets/icon.hpp"
#include "enki/state/state.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <memory>

using namespace enki;

class TooltipDemoState : public State {
public:
    WidgetPtr build(BuildContext& ctx) override {
        // 1. Basic Text Tooltip on Button
        auto btn1 = button(text("Save Changes", { .color = 0xFFFFFFFF, .font_size = 14.0f, .font_weight = FontWeight::Bold }), [](){
            std::cout << "Save clicked\n";
        });

        auto tt1 = tooltip({
            .child = btn1,
            .message = "Click to save all your pending configuration changes",
            .options = { .position = TooltipPosition::Top }
        });

        // 2. Rich Content Tooltip (Text + Badge/Icon)
        auto btn_danger = button(text("Delete Repository", { .color = 0xFFFFFFFF, .font_size = 14.0f, .font_weight = FontWeight::Bold }), [](){
            std::cout << "Delete clicked\n";
        }, {
            .normal_color = 0xFFEF4444,
            .hover_color = 0xFFDC2626
        });

        auto tt2 = tooltip({
            .child = btn_danger,
            .rich_message = column({
                .align_items = Align::Start,
                .children = {
                    text("Warning: Permanent Action", { .color = 0xFFFCA5A5, .font_size = 13.0f, .font_weight = FontWeight::Bold }),
                    text("This action cannot be undone.", { .color = 0xFFE2E8F0, .font_size = 11.0f })
                }
            }),
            .options = {
                .background_color = 0xEE7F1D1D,
                .border_color = 0xFFEF4444,
                .position = TooltipPosition::Bottom
            }
        });

        // 3. Custom Position Tooltips (Left & Right)
        auto tt_left = tooltip({
            .child = button(text("Tooltip Left", { .color = 0xFFFFFFFF, .font_size = 13.0f }), nullptr),
            .message = "Positioned to the left of the button",
            .options = { .position = TooltipPosition::Left }
        });

        auto tt_right = tooltip({
            .child = button(text("Tooltip Right", { .color = 0xFFFFFFFF, .font_size = 13.0f }), nullptr),
            .message = "Positioned to the right of the button",
            .options = { .position = TooltipPosition::Right }
        });

        return container({
            .color = 0xFF0F172A,
            .padding = StyleInsets::all(40.0f),
            .flex_grow = 1.0f,
            .child = column({
                .justify_content = Justify::Center,
                .align_items = Align::Center,
                .children = {
                    column({
                        .align_items = Align::Center,
                        .margin = StyleInsets::only(0, 0, 40.0f, 0),
                        .children = {
                            text("Advanced Native Tooltips (NativePopup)", { .color = 0xFFFFFFFF, .font_size = 24.0f, .font_weight = FontWeight::Bold }),
                            text("Hover or interact with elements to spawn native floating desktop tooltips", { .color = 0xFF94A3B8, .font_size = 14.0f })
                        }
                    }),
                    column({
                        .align_items = Align::Center,
                        .gap = StyleValue::point(40.0f),
                        .children = {
                            row({
                                .justify_content = Justify::Center,
                                .align_items = Align::Center,
                                .gap = StyleValue::point(40.0f),
                                .children = { tt1, tt2 }
                            }),
                            row({
                                .justify_content = Justify::Center,
                                .align_items = Align::Center,
                                .gap = StyleValue::point(40.0f),
                                .children = { tt_left, tt_right }
                            })
                        }
                    })
                }
            })
        });
    }
};

class TooltipDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<TooltipDemoState>();
    }
    std::string_view typeName() const override { return "TooltipDemoApp"; }
};

int main() {
    std::cout << "================================================\n";
    std::cout << "  ENKI Engine — Tooltip Widget Demo (Native Popup in App)\n";
    std::cout << "================================================\n";

    AppConfig config;
    config.title       = "Enki Engine — Tooltip Demo";
    config.width       = 800;
    config.height      = 400;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 60;
    config.clear_color = 0xFF0F172A;

    return runApp(std::make_shared<TooltipDemoApp>(), config);
}
