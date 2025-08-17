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
        auto title = text("Advanced Native Popover (NativePopup)");
        title->fontSize(24.0f).bold().color(0xFFFFFFFF);

        auto sub = text("Click buttons to spawn rich interactive floating popover surfaces with pointer arrows");
        sub->fontSize(14.0f).color(0xFF94A3B8);

        std::vector<WidgetPtr> t_children = {title, sub};
        auto titleCol = column(t_children);
        titleCol->alignItems(Align::Center).margin(StyleInsets::only(0, 0, 40.0f, 0));

        // 1. User Profile Popover
        auto profile_btn_text = text("👤 User Profile");
        profile_btn_text->fontSize(14.0f).color(0xFFFFFFFF).bold();
        auto profile_btn = button(profile_btn_text, nullptr);

        PopoverProps opt1;
        opt1.direction    = PopoverDirection::Top;
        opt1.content_size = Size{240.0f, 150.0f};
        opt1.background_color = 0xFA1F242C;
        opt1.border_color     = 0xFF38BDF8;

        auto profile_popover = popover(profile_btn, [](BuildContext& sub_ctx) {
            auto name = text("Alexander Wright");
            name->fontSize(15.0f).bold().color(0xFFF8FAFC);

            auto role = text("Lead Systems Architect");
            role->fontSize(12.0f).color(0xFF38BDF8);

            auto status = text("Status: 🟢 Active");
            status->fontSize(11.0f).color(0xFF94A3B8);

            auto edit_btn_text = text("Edit Profile");
            edit_btn_text->fontSize(12.0f).color(0xFFFFFFFF);
            auto edit_btn = button(edit_btn_text, []() {
                std::cout << "[Popover] Edit Profile Clicked!\n";
            });

            std::vector<WidgetPtr> pop_items = {name, role, status, edit_btn};
            auto pop_col = column(pop_items);
            pop_col->alignItems(Align::Start).gap(StyleValue::point(8.0f));

            return pop_col;
        }, opt1);

        // 2. Quick Settings Popover
        auto settings_btn_text = text("⚙️ Quick Settings");
        settings_btn_text->fontSize(14.0f).color(0xFFFFFFFF).bold();
        auto settings_btn = button(settings_btn_text, nullptr);

        PopoverProps opt2;
        opt2.direction    = PopoverDirection::Bottom;
        opt2.content_size = Size{260.0f, 140.0f};
        opt2.background_color = 0xFA0F172A;
        opt2.border_color     = 0xFF334155;

        auto settings_popover = popover(settings_btn, [](BuildContext& sub_ctx) {
            auto st_title = text("Display Configuration");
            st_title->fontSize(14.0f).bold().color(0xFFF1F5F9);

            auto dark_mode = text("• Theme: Dark Mode (Active)");
            dark_mode->fontSize(12.0f).color(0xFFCBD5E1);

            auto vsync = text("• V-Sync: Enabled (60 FPS)");
            vsync->fontSize(12.0f).color(0xFFCBD5E1);

            auto save_btn_text = text("Apply Settings");
            save_btn_text->fontSize(12.0f).color(0xFFFFFFFF);
            auto save_btn = button(save_btn_text, []() {
                std::cout << "[Popover] Settings Applied!\n";
            });

            std::vector<WidgetPtr> pop_items = {st_title, dark_mode, vsync, save_btn};
            auto pop_col = column(pop_items);
            pop_col->alignItems(Align::Start).gap(StyleValue::point(8.0f));

            return pop_col;
        }, opt2);

        // 3. Programmatically Controlled Popover
        auto ctrl_btn_text = text("⚡ Programmatic Toggle");
        ctrl_btn_text->fontSize(14.0f).color(0xFFFFFFFF).bold();
        auto ctrl_btn = button(ctrl_btn_text, [this]() {
            pop_controller_->toggle();
        });

        PopoverProps opt3;
        opt3.direction    = PopoverDirection::Top;
        opt3.trigger      = PopoverTrigger::Manual;
        opt3.content_size = Size{220.0f, 100.0f};

        auto ctrl_popover = popover(ctrl_btn, [](BuildContext& sub_ctx) {
            auto msg = text("Triggered via PopoverController!");
            msg->fontSize(13.0f).color(0xFF38BDF8).bold();

            auto desc = text("Can be called from anywhere in code.");
            desc->fontSize(11.0f).color(0xFF94A3B8);

            return column({msg, desc});
        }, opt3, pop_controller_);

        // Layout rows
        std::vector<WidgetPtr> r_children = {profile_popover, settings_popover, ctrl_popover};
        auto buttonsRow = row(r_children);
        buttonsRow->justifyContent(Justify::Center).alignItems(Align::Center).gap(30_px);

        std::vector<WidgetPtr> m_children = {titleCol, buttonsRow};
        auto mainCol = column(m_children);
        mainCol->alignItems(Align::Center).justifyContent(Justify::Center);

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
    config.target_fps  = 60;
    config.clear_color = 0xFF0F172A;

    return runApp(std::make_shared<PopoverDemoApp>(), config);
}
