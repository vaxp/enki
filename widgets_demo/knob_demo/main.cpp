#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/knob.hpp"
#include "enki/widgets/card.hpp"
#include "enki/state/state.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>

using namespace enki;

class KnobDemoState : public State {
    float master_vol_ = 75.0f;
    float pan_val_ = 0.0f;
    float cutoff_val_ = 2500.0f;

public:
    WidgetPtr build(BuildContext&) override {
        auto title = text("Knob Widget Interactive Demo", {
            .color = 0xFFFFFFFF,
            .font_size = 22.0f,
            .font_weight = FontWeight::Bold
        });
        auto subtitle = text("Standalone Studio Rotary Dial Control (Section 15)", {
            .color = 0xFF00E5FF,
            .font_size = 13.0f,
            .font_weight = FontWeight::Medium
        });

        std::stringstream ss_vol;
        ss_vol << "Volume: " << std::fixed << std::setprecision(0) << master_vol_ << "%";
        auto lbl_vol = text(ss_vol.str(), { .color = 0xFF38BDF8, .font_size = 14.0f, .font_weight = FontWeight::Bold });

        std::stringstream ss_pan;
        ss_pan << "Pan: " << (pan_val_ < 0 ? "L " : (pan_val_ > 0 ? "R " : "C "))
               << std::fixed << std::setprecision(0) << std::abs(pan_val_);
        auto lbl_pan = text(ss_pan.str(), { .color = 0xFFF59E0B, .font_size = 14.0f, .font_weight = FontWeight::Bold });

        std::stringstream ss_cut;
        ss_cut << "Cutoff: " << std::fixed << std::setprecision(0) << cutoff_val_ << " Hz";
        auto lbl_cut = text(ss_cut.str(), { .color = 0xFF10B981, .font_size = 14.0f, .font_weight = FontWeight::Bold });

        auto k1 = knob({
            .value = master_vol_,
            .min_value = 0.0f,
            .max_value = 100.0f,
            .step = 1.0f,
            .size = 80.0f,
            .label = "VOLUME",
            .unit = "%",
            .active_color = 0xFF00E5FF,
            .on_value_changed = [this](float v) {
                master_vol_ = v;
                setState([]{});
            },
        });

        auto k2 = knob({
            .value = pan_val_,
            .min_value = -50.0f,
            .max_value = 50.0f,
            .step = 1.0f,
            .size = 80.0f,
            .is_bipolar = true,
            .label = "PAN",
            .unit = "",
            .active_color = 0xFFF59E0B,
            .on_value_changed = [this](float v) {
                pan_val_ = v;
                setState([]{});
            },
        });

        auto k3 = knob({
            .value = cutoff_val_,
            .min_value = 20.0f,
            .max_value = 20000.0f,
            .step = 50.0f,
            .size = 80.0f,
            .label = "CUTOFF",
            .unit = "Hz",
            .active_color = 0xFF10B981,
            .on_value_changed = [this](float v) {
                cutoff_val_ = v;
                setState([]{});
            },
        });

        auto col1 = column({
            .align_items = Align::Center,
            .gap = StyleValue::point(12.0f),
            .children = {k1, lbl_vol}
        });

        auto col2 = column({
            .align_items = Align::Center,
            .gap = StyleValue::point(12.0f),
            .children = {k2, lbl_pan}
        });

        auto col3 = column({
            .align_items = Align::Center,
            .gap = StyleValue::point(12.0f),
            .children = {k3, lbl_cut}
        });

        auto knobs_row = row({
            .justify_content = Justify::Center,
            .align_items = Align::Center,
            .gap = StyleValue::point(40.0f),
            .children = {col1, col2, col3}
        });

        auto main_col = column(FlexboxProps{
            .align_items = Align::Center,
            .gap = StyleValue::point(28.0f),
            .children = {title, subtitle, knobs_row}
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

class KnobDemoApp : public StatefulWidget {
public:
    std::string_view typeName() const override { return "KnobDemoApp"; }
    std::unique_ptr<State> createState() override { return std::make_unique<KnobDemoState>(); }
};

int main() {
    std::cout << "=== ENKI Knob Standalone Demo ===\n";
    AppConfig config;
    config.title = "ENKI — Knob Widget Demo";
    config.width = 720;
    config.height = 420;
    config.resizable = true;
    config.vsync = false;
    config.target_fps = 0;
    config.show_performance_overlay = true;
    config.clear_color = 0xFF0B1320;

    return runApp(std::make_shared<KnobDemoApp>(), config);
}
