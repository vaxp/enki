/// @file main.cpp
/// @brief ENKI Advanced NumberField Interactive Showcase Demo.
/// Demonstrates precision, steppers, drag scrubbing, expressions, units, and responsive layouts.

#include "enki/app/app.hpp"
#include "enki/widgets/number_field.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/card.hpp"
#include "enki/state/state.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <iomanip>
#include <sstream>

using namespace enki;

// Helper to build a styled card section
static WidgetPtr buildSectionCard(
    const std::string& title_str,
    const std::string& desc_str,
    Color accent_color,
    WidgetPtr field_widget,
    const std::string& current_val_label) {

    auto t = text(title_str, {
        .color = accent_color,
        .font_size = 14.5f,
        .font_weight = FontWeight::Bold,
    });

    auto d = text(desc_str, {
        .color = 0xFF94A3B8,
        .font_size = 12.0f,
    });

    auto val_txt = text(current_val_label, {
        .color = 0xFF38BDF8,
        .font_size = 12.0f,
    });

    auto header_col = column({
        .gap = StyleValue::point(3.0f),
        .children = {t, d}
    });

    auto card_col = column({
        .gap = StyleValue::point(12.0f),
        .children = {header_col, field_widget, val_txt}
    });

    return container({
        .color = 0xFF1E293B,
        .border_radius = BorderRadius::circular(10.0f),
        .border = Border(0xFF334155, 1.0f),
        .width = StyleValue::point(360.0f),
        .padding = StyleInsets::all(16.0f),
        .child = card_col
    });
}

class NumberFieldDemoState : public State {
private:
    std::shared_ptr<NumberFieldController> currency_ctrl_;
    std::shared_ptr<NumberFieldController> dimension_ctrl_;
    std::shared_ptr<NumberFieldController> percent_ctrl_;
    std::shared_ptr<NumberFieldController> cad_ctrl_;
    std::shared_ptr<NumberFieldController> quantity_ctrl_;
    std::shared_ptr<NumberFieldController> temp_ctrl_;

public:
    void initState() override {
        State::initState();
        currency_ctrl_  = std::make_shared<NumberFieldController>(1250.0);
        dimension_ctrl_ = std::make_shared<NumberFieldController>(1920.0);
        percent_ctrl_   = std::make_shared<NumberFieldController>(75.5);
        cad_ctrl_       = std::make_shared<NumberFieldController>(45.0);
        quantity_ctrl_  = std::make_shared<NumberFieldController>(5.0);
        temp_ctrl_      = std::make_shared<NumberFieldController>(24.0);
    }

    WidgetPtr build(BuildContext&) override {
        // Main Title Header
        auto title = text("Advanced NumberField Widget Suite", {
            .color = 0xFFFFFFFF,
            .font_size = 22.0f,
            .font_weight = FontWeight::Bold,
        });

        auto sub = text("Precision numeric inputs with steppers, CAD drag scrubbing, math expressions, and unit formatting", {
            .color = 0xFF94A3B8,
            .font_size = 13.0f,
        });

        auto title_col = column({
            .align_items = Align::Center,
            .children = {title, sub}
        });

        // ── Card 1: Currency & Financial Input ────────────────────────
        auto curr_field = NumberField {
            .controller = currency_ctrl_,
            .min_value = 0.0,
            .step = 50.0,
            .large_step = 500.0,
            .precision = 2,
            .show_thousands_separator = true,
            .stepper_position = NumberFieldStepperPosition::RightVertical,
            .prefix_text = "$ ",
            .on_changed = [this](double) { setState([] {}); }
        };
        std::ostringstream ss1;
        ss1 << "Bound Value: $" << std::fixed << std::setprecision(2) << currency_ctrl_->getValue();
        auto card1 = buildSectionCard(
            "1. Financial / Currency Input",
            "Thousands separator, step=$50, min=$0",
            0xFF38BDF8, curr_field, ss1.str()
        );

        // ── Card 2: CSS / UI Dimensions (Expressions) ────────────────
        auto dim_field = NumberField {
            .controller = dimension_ctrl_,
            .min_value = 100.0,
            .max_value = 7680.0,
            .step = 10.0,
            .large_step = 100.0,
            .precision = 0,
            .allow_expressions = true,
            .stepper_position = NumberFieldStepperPosition::RightVertical,
            .suffix_text = " px",
            .on_changed = [this](double) { setState([] {}); }
        };
        std::ostringstream ss2;
        ss2 << "Calculated Value: " << static_cast<int>(dimension_ctrl_->getValue()) << " px (Supports e.g. 1920/2)";
        auto card2 = buildSectionCard(
            "2. Dimensions & Math Expressions",
            "Try typing '1920 / 2' or '800 + 400' and press Enter",
            0xFF818CF8, dim_field, ss2.str()
        );

        // ── Card 3: Percentage & Opacity (Sides Steppers) ─────────────
        auto pct_field = NumberField {
            .controller = percent_ctrl_,
            .min_value = 0.0,
            .max_value = 100.0,
            .step = 0.5,
            .large_step = 5.0,
            .fine_step = 0.1,
            .precision = 1,
            .stepper_position = NumberFieldStepperPosition::Sides,
            .suffix_text = " %",
            .focus_border_color = 0xFF10B981,
            .on_changed = [this](double) { setState([] {}); }
        };
        std::ostringstream ss3;
        ss3 << "Opacity Level: " << std::fixed << std::setprecision(1) << percent_ctrl_->getValue() << " %";
        auto card3 = buildSectionCard(
            "3. Percentage (Sides Steppers)",
            "Step=0.5%, min=0%, max=100%, [−] Left, [+] Right",
            0xFF10B981, pct_field, ss3.str()
        );

        // ── Card 4: CAD / 3D Transform Scrubbing ──────────────────────
        auto cad_field = NumberField {
            .controller = cad_ctrl_,
            .step = 1.0,
            .precision = 2,
            .enable_scrubbing = true,
            .stepper_position = NumberFieldStepperPosition::None,
            .prefix_text = "Rot: ",
            .suffix_text = " deg",
            .focus_border_color = 0xFFF59E0B,
            .on_changed = [this](double) { setState([] {}); }
        };
        std::ostringstream ss4;
        ss4 << "Active Angle: " << std::fixed << std::setprecision(2) << cad_ctrl_->getValue() << "° (Click & drag to scrub)";
        auto card4 = buildSectionCard(
            "4. CAD / 3D Drag Scrubbing",
            "Horizontal drag-to-adjust with precision cursor",
            0xFFF59E0B, cad_field, ss4.str()
        );

        // ── Card 5: Compact Table / Cart Quantity ─────────────────────
        auto qty_field = NumberField {
            .controller = quantity_ctrl_,
            .min_value = 1.0,
            .max_value = 99.0,
            .step = 1.0,
            .precision = 0,
            .stepper_position = NumberFieldStepperPosition::RightHorizontal,
            .size = NumberFieldSize::Small,
            .on_changed = [this](double) { setState([] {}); }
        };
        std::ostringstream ss5;
        ss5 << "Selected Quantity: " << static_cast<int>(quantity_ctrl_->getValue()) << " items (Small Size)";
        auto card5 = buildSectionCard(
            "5. Compact Size & Right Steppers",
            "Small ~32px preset with side-by-side [−][+] buttons",
            0xFFEC4899, qty_field, ss5.str()
        );

        // ── Card 6: Temperature Physics & Wrap Mode ───────────────────
        auto temp_field = NumberField {
            .controller = temp_ctrl_,
            .min_value = -40.0,
            .max_value = 120.0,
            .step = 0.5,
            .precision = 1,
            .stepper_position = NumberFieldStepperPosition::RightVertical,
            .wrap_mode = NumberFieldWrapMode::Wrap,
            .suffix_text = " °C",
            .focus_border_color = 0xFF06B6D4,
            .on_changed = [this](double) { setState([] {}); }
        };
        std::ostringstream ss6;
        ss6 << "Thermal Reading: " << std::fixed << std::setprecision(1) << temp_ctrl_->getValue() << " °C (Wrap Enabled)";
        auto card6 = buildSectionCard(
            "6. Thermal Range with Wrap Mode",
            "Wraps between -40.0°C and +120.0°C on overflow",
            0xFF06B6D4, temp_field, ss6.str()
        );

        // Row 1 of cards
        auto row1 = row({
            .justify_content = Justify::Center,
            .gap = StyleValue::point(16.0f),
            .children = {card1, card2, card3}
        });

        // Row 2 of cards
        auto row2 = row({
            .justify_content = Justify::Center,
            .gap = StyleValue::point(16.0f),
            .children = {card4, card5, card6}
        });

        // Main Stack Column
        auto main_col = column({
            .align_items = Align::Center,
            .gap = StyleValue::point(18.0f),
            .children = {title_col, row1, row2}
        });

        return container({
            .color = 0xFF0B1120,
            .padding = StyleInsets::all(20.0f),
            .child = main_col
        });
    }
};

class NumberFieldDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<NumberFieldDemoState>();
    }
    std::string_view typeName() const override { return "NumberFieldDemoApp"; }
};

int main() {
    std::cout << "====================================================\n";
    std::cout << "  ENKI Engine — Advanced NumberField Widget Demo\n";
    std::cout << "====================================================\n";

    AppConfig config;
    config.title       = "Enki Engine — Advanced NumberField Demo";
    config.width       = 1180;
    config.height      = 660;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 60;
    config.clear_color = 0xFF0B1120;

    return runApp(std::make_shared<NumberFieldDemoApp>(), config);
}
