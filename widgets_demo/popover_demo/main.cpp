/// @file main.cpp
/// @brief ENKI Advanced Popover Widget Interactive Showcase.
/// Demonstrates rich floating popovers, pointer arrows, programmatic controllers, and interactive internal forms.

#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/popover.hpp"
#include "enki/widgets/avatar.hpp"
#include "enki/state/state.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <memory>

using namespace enki;

class PopoverDemoState : public State {
private:
    std::shared_ptr<PopoverController> pop_controller_;

public:
    void initState() override {
        State::initState();
        pop_controller_ = std::make_shared<PopoverController>();
    }

    WidgetPtr build(BuildContext& ctx) override {
        // Title & Description Header
        auto title = text({
            .text = "Advanced Native Popover (NativePopup)",
            .color = 0xFFFFFFFF,
            .font_size = 24.0f,
            .font_weight = FontWeight::Bold,
        });

        auto sub = text({
            .text = "Click buttons to spawn rich interactive floating popover surfaces with pointer arrows",
            .color = 0xFF94A3B8,
            .font_size = 14.0f,
        });

        auto titleCol = column({
            .align_items = Align::Center,
            .margin = StyleInsets::only(0, 0, 40.0f, 0),
            .children = { title, sub }
        });

        // 1. User Profile Popover
        auto profile_btn_text = text({
            .text = "👤 User Profile",
            .color = 0xFFFFFFFF,
            .font_size = 14.0f,
            .font_weight = FontWeight::Bold,
        });
        WidgetPtr profile_btn = button(profile_btn_text, nullptr);

        PopoverProps opt1;
        opt1.direction    = PopoverDirection::Top;
        opt1.content_size = Size{240.0f, 150.0f};
        opt1.background_color = 0xFA1F242C;
        opt1.border_color     = 0xFF38BDF8;

        WidgetPtr profile_popover = popover(profile_btn, [](BuildContext& sub_ctx) -> WidgetPtr {
            auto name = text({
                .text = "Alexander Wright",
                .color = 0xFFF8FAFC,
                .font_size = 15.0f,
                .font_weight = FontWeight::Bold,
            });

            auto role = text({
                .text = "Lead Systems Architect",
                .color = 0xFF38BDF8,
                .font_size = 12.0f,
            });

            auto status = text({
                .text = "Status: 🟢 Active",
                .color = 0xFF94A3B8,
                .font_size = 11.0f,
            });

            auto edit_btn_text = text({
                .text = "Edit Profile",
                .color = 0xFFFFFFFF,
                .font_size = 12.0f,
            });
            auto edit_btn = button(edit_btn_text, []() {
                std::cout << "[Popover] Edit Profile Clicked!\n";
            });

            return column({
                .align_items = Align::Start,
                .gap = StyleValue::point(8.0f),
                .children = { name, role, status, edit_btn }
            });
        }, opt1);

        // 2. Quick Settings Popover
        auto settings_btn_text = text({
            .text = "⚙️ Quick Settings",
            .color = 0xFFFFFFFF,
            .font_size = 14.0f,
            .font_weight = FontWeight::Bold,
        });
        WidgetPtr settings_btn = button(settings_btn_text, nullptr);

        PopoverProps opt2;
        opt2.direction    = PopoverDirection::Bottom;
        opt2.content_size = Size{260.0f, 140.0f};
        opt2.background_color = 0xFA0F172A;
        opt2.border_color     = 0xFF334155;

        WidgetPtr settings_popover = popover(settings_btn, [](BuildContext& sub_ctx) -> WidgetPtr {
            auto st_title = text({
                .text = "Display Configuration",
                .color = 0xFFF1F5F9,
                .font_size = 14.0f,
                .font_weight = FontWeight::Bold,
            });

            auto dark_mode = text({
                .text = "• Theme: Dark Mode (Active)",
                .color = 0xFFCBD5E1,
                .font_size = 12.0f,
            });

            auto vsync = text({
                .text = "• V-Sync: Enabled (60 FPS)",
                .color = 0xFFCBD5E1,
                .font_size = 12.0f,
            });

            auto save_btn_text = text({
                .text = "Apply Settings",
                .color = 0xFFFFFFFF,
                .font_size = 12.0f,
            });
            auto save_btn = button(save_btn_text, []() {
                std::cout << "[Popover] Settings Applied!\n";
            });

            return column({
                .align_items = Align::Start,
                .gap = StyleValue::point(8.0f),
                .children = { st_title, dark_mode, vsync, save_btn }
            });
        }, opt2);

        // 3. Programmatically Controlled Popover
        auto ctrl_btn_text = text({
            .text = "⚡ Programmatic Toggle",
            .color = 0xFFFFFFFF,
            .font_size = 14.0f,
            .font_weight = FontWeight::Bold,
        });
        WidgetPtr ctrl_btn = button(ctrl_btn_text, [this]() {
            pop_controller_->toggle();
        });

        PopoverProps opt3;
        opt3.direction    = PopoverDirection::Top;
        opt3.trigger      = PopoverTrigger::Manual;
        opt3.content_size = Size{220.0f, 100.0f};

        WidgetPtr ctrl_popover = popover(ctrl_btn, [](BuildContext& sub_ctx) -> WidgetPtr {
            auto msg = text({
                .text = "Triggered via PopoverController!",
                .color = 0xFF38BDF8,
                .font_size = 13.0f,
                .font_weight = FontWeight::Bold,
            });

            auto desc = text({
                .text = "Can be called from anywhere in code.",
                .color = 0xFF94A3B8,
                .font_size = 11.0f,
            });

            return column({
                .children = { msg, desc }
            });
        }, opt3, pop_controller_);

        // Layout rows
        auto buttonsRow = row({
            .justify_content = Justify::Center,
            .align_items = Align::Center,
            .gap = 30_px,
            .children = { profile_popover, settings_popover, ctrl_popover }
        });

        auto mainCol = column({
            .justify_content = Justify::Center,
            .align_items = Align::Center,
            .children = { titleCol, buttonsRow }
        });

        auto appRoot = container(mainCol);
        appRoot->color(0xFF0F172A)
               .paddingAll(40.0f)
               .flexGrow(1.0f);

        return appRoot;
    }
};

class PopoverDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<PopoverDemoState>();
    }
    std::string_view typeName() const override { return "PopoverDemoApp"; }
};

int main() {
    std::cout << "====================================================\n";
    std::cout << "  ENKI Engine — Popover Widget Demo (NativePopup)\n";
    std::cout << "====================================================\n";

    AppConfig config;
    config.title       = "Enki Engine — Popover Demo";
    config.width       = 850;
    config.height      = 450;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 0;
    config.show_performance_overlay = true;
    config.clear_color = 0xFF0F172A;

    return runApp(std::make_shared<PopoverDemoApp>(), config);
}
