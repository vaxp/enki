#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/rating_bar.hpp"
#include "enki/state/state.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>

using namespace enki;

class RatingBarDemoState : public State {
    float rating1_ = 3.5f;
    float rating2_ = 4.0f;

public:
    WidgetPtr build(BuildContext&) override {
        auto title = text("RatingBar Interactive Demo", {
            .color = 0xFFFFFFFF,
            .font_size = 22.0f,
            .font_weight = FontWeight::Bold
        });
        auto subtitle = text("Fractional Half-Star & Integer Star Rating Bar (Section 15)", {
            .color = 0xFF00E5FF,
            .font_size = 13.0f,
            .font_weight = FontWeight::Medium
        });

        std::stringstream ss1;
        ss1 << "Score: " << std::fixed << std::setprecision(1) << rating1_ << " / 5.0 (Half-Star Enabled)";
        auto lbl1 = text(ss1.str(), { .color = 0xFFF59E0B, .font_size = 14.0f, .font_weight = FontWeight::Bold });

        auto rb1 = ratingBar({
            .rating = rating1_,
            .max_rating = 5,
            .item_size = 36.0f,
            .item_spacing = 10.0f,
            .allow_half = true,
            .active_color = 0xFFF59E0B,
            .on_rating_changed = [this](float r) {
                rating1_ = r;
                setState([]{});
            },
        });

        std::stringstream ss2;
        ss2 << "Score: " << std::fixed << std::setprecision(0) << rating2_ << " / 5 (Full-Star Only)";
        auto lbl2 = text(ss2.str(), { .color = 0xFF00E5FF, .font_size = 14.0f, .font_weight = FontWeight::Bold });

        auto rb2 = ratingBar({
            .rating = rating2_,
            .max_rating = 5,
            .item_size = 36.0f,
            .item_spacing = 10.0f,
            .allow_half = false,
            .active_color = 0xFF00E5FF,
            .on_rating_changed = [this](float r) {
                rating2_ = r;
                setState([]{});
            },
        });

        auto main_col = column(FlexboxProps{
            .align_items = Align::Center,
            .gap = StyleValue::point(24.0f),
            .children = {
                title,
                subtitle,
                lbl1,
                rb1,
                lbl2,
                rb2,
            }
        });

        return container(ContainerProps{
            .color = 0xFF0B1320,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(40.0f),
            .child = main_col
        });
    }
};

class RatingBarDemoApp : public StatefulWidget {
public:
    std::string_view typeName() const override { return "RatingBarDemoApp"; }
    std::unique_ptr<State> createState() override { return std::make_unique<RatingBarDemoState>(); }
};

int main() {
    std::cout << "=== ENKI RatingBar Standalone Demo ===\n";
    AppConfig config;
    config.title = "ENKI — RatingBar Demo";
    config.width = 720;
    config.height = 420;
    config.resizable = true;
    config.vsync = false;
    config.target_fps = 0;
    config.show_performance_overlay = true;
    config.clear_color = 0xFF0B1320;

    return runApp(std::make_shared<RatingBarDemoApp>(), config);
}
