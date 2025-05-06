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
        auto title = std::make_shared<Text>("Badge Widget Demo", TextStyle{.color = 0xFFFFFFFF, .font_size = 24.0f});
        
        // Notification dot on an icon
        auto notification_icon = container(std::make_shared<Text>("🔔", TextStyle{.font_size=24.0f}));
        notification_icon->paddingAll(8.0f).color(0xFF2D3748).borderRadius(8.0f);
        auto badge_dot = badge(notification_icon);
        badge_dot->size(14.0f).bgColor(0xFFEF4444).offset(4.0f, -4.0f);
                            
        auto text1 = std::make_shared<Text>("Status Dot Badge", TextStyle{.color = 0xFFCCCCCC});
        std::vector<WidgetPtr> row1_items = {badge_dot, text1};
        auto row1 = row(row1_items);
        row1->gap(StyleValue::point(16.0f)).alignItems(Align::Center);

        // Counter badge with text
        auto msg_icon = container(std::make_shared<Text>("✉️", TextStyle{.font_size=24.0f}));
        msg_icon->paddingAll(8.0f).color(0xFF2D3748).borderRadius(8.0f);
        auto label_text = std::make_shared<Text>("3", TextStyle{.color = 0xFFFFFFFF, .font_size=10.0f, .font_weight=FontWeight::Bold});
        auto badge_count = badge(msg_icon, label_text);
        badge_count->bgColor(0xFF3B82F6).offset(6.0f, -6.0f); // Blue
                            
        auto text2 = std::make_shared<Text>("Counter Badge", TextStyle{.color = 0xFFCCCCCC});
        std::vector<WidgetPtr> row2_items = {badge_count, text2};
        auto row2 = row(row2_items);
        row2->gap(StyleValue::point(16.0f)).alignItems(Align::Center);

        // Left-aligned badge
        auto user_icon = container(std::make_shared<Text>("👤", TextStyle{.font_size=24.0f}));
        user_icon->paddingAll(8.0f).color(0xFF2D3748).borderRadius(8.0f);
        auto status_dot = badge(user_icon);
        status_dot->size(12.0f).bgColor(0xFF10B981).alignment(Alignment::BottomRight).offset(2.0f, 2.0f);
                            
        auto text3 = std::make_shared<Text>("Bottom Right Alignment", TextStyle{.color = 0xFFCCCCCC});
        std::vector<WidgetPtr> row3_items = {status_dot, text3};
        auto row3 = row(row3_items);
        row3->gap(StyleValue::point(16.0f)).alignItems(Align::Center);

        std::vector<WidgetPtr> col_items = {title, row1, row2, row3};
        auto col = column(col_items);
        col->gap(StyleValue::point(32.0f)).padding(StyleInsets::all(40.0f));

        auto bg = container(col);
        bg->color(0xFF0F172A);

        return bg;
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
