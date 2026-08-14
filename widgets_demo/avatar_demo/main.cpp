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
        // Title
        auto title = text("Advanced Avatar Widget Demo");
        title->fontSize(24.0f).color(0xFFFFFFFF).bold();
        
        auto sub = text("Initials, Images, Badges, and Avatar Groups in ENKI");
        sub->fontSize(14.0f).color(0xFF94A3B8);
        
        std::vector<WidgetPtr> t_children = {
            std::static_pointer_cast<Widget>(title), 
            std::static_pointer_cast<Widget>(sub)
        };
        auto titleCol = column(t_children);
        titleCol->alignItems(Align::Center).margin(StyleInsets::only(0, 0, 40.0f, 0));

        // 1. Initials Avatar
        AvatarOptions opt_init;
        opt_init.background_color = 0xFF8B5CF6; // Purple
        auto av_init = avatar("MK", opt_init);

        // 2. Avatar with Status Badge
        AvatarOptions opt_badge;
        opt_badge.background_color = 0xFFF59E0B; // Amber
        opt_badge.show_badge = true;
        opt_badge.badge_color = 0xFF10B981; // Green online
        opt_badge.radius = 32.0f;
        auto av_badge = avatar("TS", opt_badge);

        // 3. Avatar with Image and Border
        AvatarOptions opt_border;
        opt_border.image_path = "/home/x/Work/enki/assets/vaxp.png";
        opt_border.border_width = 3.0f;
        opt_border.border_color = 0xFFFFFFFF;
        opt_border.radius = 40.0f;
        opt_border.shadow_blur = 10.0f;
        auto av_border = avatar("", opt_border);

        // Row 1: Single Avatars
        std::vector<WidgetPtr> r1_children = {
            std::static_pointer_cast<Widget>(av_init), 
            std::static_pointer_cast<Widget>(av_badge), 
            std::static_pointer_cast<Widget>(av_border)
        };
        auto row1 = row(r1_children);
        row1->gap(30.0f).alignItems(Align::Center).justifyContent(Justify::Center).margin(StyleInsets::only(0, 0, 40.0f, 0));

        // 4. Avatar Group
        AvatarOptions opt_g1; opt_g1.background_color = 0xFFEF4444; opt_g1.border_width = 2.0f; opt_g1.radius = 20.0f;
        AvatarOptions opt_g2; opt_g2.background_color = 0xFF3B82F6; opt_g2.border_width = 2.0f; opt_g2.radius = 20.0f;
        AvatarOptions opt_g3; opt_g3.background_color = 0xFF10B981; opt_g3.border_width = 2.0f; opt_g3.radius = 20.0f;
        AvatarOptions opt_g4; opt_g4.background_color = 0xFFF59E0B; opt_g4.border_width = 2.0f; opt_g4.radius = 20.0f;
        AvatarOptions opt_g5; opt_g5.background_color = 0xFF8B5CF6; opt_g5.border_width = 2.0f; opt_g5.radius = 20.0f;
        
        std::vector<WidgetPtr> group_list = {
            std::static_pointer_cast<Widget>(avatar("A", opt_g1)),
            std::static_pointer_cast<Widget>(avatar("B", opt_g2)),
            std::static_pointer_cast<Widget>(avatar("C", opt_g3)),
            std::static_pointer_cast<Widget>(avatar("D", opt_g4)),
            std::static_pointer_cast<Widget>(avatar("E", opt_g5)), // Will be hidden in group if max=3, replaced with +2
        };
        
        auto group = avatarGroup(group_list, -12.0f, 3);
        
        auto group_label = text("Avatar Group (Max 3)");
        group_label->fontSize(14.0f).color(0xFF94A3B8);
        std::vector<WidgetPtr> g_children = {
            std::static_pointer_cast<Widget>(group), 
            std::static_pointer_cast<Widget>(group_label)
        };
        auto group_col = column(g_children);
        group_col->gap(10.0f).alignItems(Align::Center);

        // Main Layout
        std::vector<WidgetPtr> m_children = {
            std::static_pointer_cast<Widget>(titleCol), 
            std::static_pointer_cast<Widget>(row1), 
            std::static_pointer_cast<Widget>(group_col)
        };
        auto main_col = column(m_children);
        main_col->alignItems(Align::Center).justifyContent(Justify::Center);

        auto root = container(main_col);
        root->color(0xFF0F172A).align(Alignment::Center);

        return root;
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
    config.clear_color = 0xFF0F172A;

    return runApp(std::make_shared<AvatarDemoApp>(), config);
}
