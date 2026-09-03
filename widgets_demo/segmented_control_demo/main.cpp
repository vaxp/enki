#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/segmented_control.hpp"
#include "enki/state/state.hpp"
#include <iostream>

using namespace enki;

class SegmentedControlDemoState : public State {
    int selected_time_ = 1;
    int selected_view_ = 0;

public:
    WidgetPtr build(BuildContext&) override {
        auto title = text("SegmentedControl Interactive Demo", {
            .color = 0xFFFFFFFF,
            .font_size = 22.0f,
            .font_weight = FontWeight::Bold
        });
        auto subtitle = text("Standalone Animated Multi-Segment Selector (Section 15)", {
            .color = 0xFF00E5FF,
            .font_size = 13.0f,
            .font_weight = FontWeight::Medium
        });

        std::string time_str = "Active Period: ";
        if (selected_time_ == 0) time_str += "Daily";
        else if (selected_time_ == 1) time_str += "Weekly";
        else if (selected_time_ == 2) time_str += "Monthly";
        else time_str += "Yearly";

        auto status_time = text(time_str, {
            .color = 0xFF38BDF8,
            .font_size = 14.0f,
            .font_weight = FontWeight::Bold
        });

        auto seg_time = segmentedControl({
            .items = {
                SegmentItem("Day"),
                SegmentItem("Week"),
                SegmentItem("Month"),
                SegmentItem("Year"),
            },
            .selected_index = selected_time_,
            .on_change = [this](int idx) {
                selected_time_ = idx;
                setState([]{});
            },
            .height = 38.0f,
            .width = 440.0f,
        });

        std::string view_str = "Display Mode: ";
        if (selected_view_ == 0) view_str += "Grid View";
        else if (selected_view_ == 1) view_str += "List View";
        else view_str += "Table View";

        auto status_view = text(view_str, {
            .color = 0xFFF59E0B,
            .font_size = 14.0f,
            .font_weight = FontWeight::Bold
        });

        auto seg_view = segmentedControl({
            .items = {
                SegmentItem("Grid"),
                SegmentItem("List"),
                SegmentItem("Table"),
            },
            .selected_index = selected_view_,
            .on_change = [this](int idx) {
                selected_view_ = idx;
                setState([]{});
            },
            .thumb_color = 0xFF78350F,
            .thumb_border_color = 0xFFF59E0B,
            .active_text_color = 0xFFF59E0B,
            .height = 38.0f,
            .width = 380.0f,
        });

        auto main_col = column(FlexboxProps{
            .align_items = Align::Center,
            .gap = StyleValue::point(24.0f),
            .children = {
                title,
                subtitle,
                status_time,
                seg_time,
                status_view,
                seg_view,
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

class SegmentedControlDemoApp : public StatefulWidget {
public:
    std::string_view typeName() const override { return "SegmentedControlDemoApp"; }
    std::unique_ptr<State> createState() override { return std::make_unique<SegmentedControlDemoState>(); }
};

int main() {
    std::cout << "=== ENKI SegmentedControl Standalone Demo ===\n";
    AppConfig config;
    config.title = "ENKI — SegmentedControl Demo";
    config.width = 720;
    config.height = 420;
    config.resizable = true;
    config.vsync = false;
    config.target_fps = 0;
    config.show_performance_overlay = true;
    config.clear_color = 0xFF0B1320;

    return runApp(std::make_shared<SegmentedControlDemoApp>(), config);
}
