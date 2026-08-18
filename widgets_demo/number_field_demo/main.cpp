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

    auto t = text(title_str);
    t->fontSize(14.5f).bold().color(accent_color);

    auto d = text(desc_str);
    d->fontSize(12.0f).color(0xFF94A3B8);

    auto val_txt = text(current_val_label);
    val_txt->fontSize(12.0f).color(0xFF38BDF8);

    std::vector<WidgetPtr> header_items = {t, d};
    auto header_col = column(header_items);
    header_col->gap(StyleValue::point(3.0f));

    std::vector<WidgetPtr> card_items = {header_col, field_widget, val_txt};
    auto card_col = column(card_items);
    card_col->gap(StyleValue::point(12.0f));

    auto c_box = container(card_col);
    c_box->color(0xFF1E293B)
         .borderRadius(10.0f)
         .border(0xFF334155, 1.0f)
         .paddingAll(16.0f)
         .width(360.0f);

    return c_box;
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
        auto title = text("Advanced NumberField Widget Suite");
        title->fontSize(22.0f).bold().color(0xFFFFFFFF);

        auto sub = text("Precision numeric inputs with steppers, CAD drag scrubbing, math expressions, and unit formatting");
        sub->fontSize(13.0f).color(0xFF94A3B8);

        std::vector<WidgetPtr> title_items = {title, sub};
        auto title_col = column(title_items);
        title_col->alignItems(Align::Center);

        // ── Card 1: Currency & Financial Input ────────────────────────
        NumberFieldOptions curr_opts;
        curr_opts.prefix_text = "$ ";
        curr_opts.precision = 2;
        curr_opts.step = 50.0;
        curr_opts.large_step = 500.0;
        curr_opts.min_value = 0.0;
        curr_opts.show_thousands_separator = true;
        curr_opts.stepper_position = NumberFieldStepperPosition::RightVertical;
        curr_opts.on_changed = [this](double) { setState([] {}); };

        auto curr_field = numberField(currency_ctrl_, curr_opts);
        std::ostringstream ss1;
        ss1 << "Bound Value: $" << std::fixed << std::setprecision(2) << currency_ctrl_->getValue();
        auto card1 = buildSectionCard(
            "1. Financial / Currency Input",
            "Thousands separator, step=$50, min=$0",
            0xFF38BDF8, curr_field, ss1.str()
        );

        // ── Card 2: CSS / UI Dimensions (Expressions) ────────────────
        NumberFieldOptions dim_opts;
        dim_opts.suffix_text = " px";
        dim_opts.precision = 0;
        dim_opts.step = 10.0;
        dim_opts.large_step = 100.0;
        dim_opts.min_value = 100.0;
        dim_opts.max_value = 7680.0;
        dim_opts.allow_expressions = true;
        dim_opts.stepper_position = NumberFieldStepperPosition::RightVertical;
        dim_opts.on_changed = [this](double) { setState([] {}); };

        auto dim_field = numberField(dimension_ctrl_, dim_opts);
        std::ostringstream ss2;
        ss2 << "Calculated Value: " << static_cast<int>(dimension_ctrl_->getValue()) << " px (Supports e.g. 1920/2)";
        auto card2 = buildSectionCard(
            "2. Dimensions & Math Expressions",
            "Try typing '1920 / 2' or '800 + 400' and press Enter",
            0xFF818CF8, dim_field, ss2.str()
        );

        // ── Card 3: Percentage & Opacity (Sides Steppers) ─────────────
        NumberFieldOptions pct_opts;
        pct_opts.suffix_text = " %";
        pct_opts.precision = 1;
        pct_opts.step = 0.5;
        pct_opts.large_step = 5.0;
        pct_opts.fine_step = 0.1;
        pct_opts.min_value = 0.0;
        pct_opts.max_value = 100.0;
        pct_opts.stepper_position = NumberFieldStepperPosition::Sides;
        pct_opts.focus_border_color = 0xFF10B981;
        pct_opts.on_changed = [this](double) { setState([] {}); };

        auto pct_field = numberField(percent_ctrl_, pct_opts);
        std::ostringstream ss3;
        ss3 << "Opacity Level: " << std::fixed << std::setprecision(1) << percent_ctrl_->getValue() << " %";
        auto card3 = buildSectionCard(
            "3. Percentage (Sides Steppers)",
            "Step=0.5%, min=0%, max=100%, [−] Left, [+] Right",
            0xFF10B981, pct_field, ss3.str()
        );

        // ── Card 4: CAD / 3D Transform Scrubbing ──────────────────────
        NumberFieldOptions cad_opts;
        cad_opts.prefix_text = "Rot: ";
        cad_opts.suffix_text = " deg";
        cad_opts.precision = 2;
        cad_opts.step = 1.0;
        cad_opts.enable_scrubbing = true;
        cad_opts.stepper_position = NumberFieldStepperPosition::None;
        cad_opts.focus_border_color = 0xFFF59E0B;
        cad_opts.on_changed = [this](double) { setState([] {}); };

        auto cad_field = numberField(cad_ctrl_, cad_opts);
        std::ostringstream ss4;
        ss4 << "Active Angle: " << std::fixed << std::setprecision(2) << cad_ctrl_->getValue() << "° (Click & drag to scrub)";
        auto card4 = buildSectionCard(
            "4. CAD / 3D Drag Scrubbing",
            "Horizontal drag-to-adjust with precision cursor",
            0xFFF59E0B, cad_field, ss4.str()
        );

        // ── Card 5: Compact Table / Cart Quantity ─────────────────────
        NumberFieldOptions qty_opts;
        qty_opts.size = NumberFieldSize::Small;
        qty_opts.precision = 0;
        qty_opts.min_value = 1.0;
        qty_opts.max_value = 99.0;
        qty_opts.step = 1.0;
        qty_opts.stepper_position = NumberFieldStepperPosition::RightHorizontal;
        qty_opts.on_changed = [this](double) { setState([] {}); };

        auto qty_field = numberField(quantity_ctrl_, qty_opts);
        std::ostringstream ss5;
        ss5 << "Selected Quantity: " << static_cast<int>(quantity_ctrl_->getValue()) << " items (Small Size)";
        auto card5 = buildSectionCard(
            "5. Compact Size & Right Steppers",
            "Small ~32px preset with side-by-side [−][+] buttons",
            0xFFEC4899, qty_field, ss5.str()
        );

        // ── Card 6: Temperature Physics & Wrap Mode ───────────────────
        NumberFieldOptions temp_opts;
        temp_opts.suffix_text = " °C";
        temp_opts.precision = 1;
        temp_opts.step = 0.5;
        temp_opts.min_value = -40.0;
        temp_opts.max_value = 120.0;
        temp_opts.wrap_mode = NumberFieldWrapMode::Wrap;
        temp_opts.stepper_position = NumberFieldStepperPosition::RightVertical;
        temp_opts.focus_border_color = 0xFF06B6D4;
        temp_opts.on_changed = [this](double) { setState([] {}); };

        auto temp_field = numberField(temp_ctrl_, temp_opts);
        std::ostringstream ss6;
        ss6 << "Thermal Reading: " << std::fixed << std::setprecision(1) << temp_ctrl_->getValue() << " °C (Wrap Enabled)";
        auto card6 = buildSectionCard(
            "6. Thermal Range with Wrap Mode",
            "Wraps between -40.0°C and +120.0°C on overflow",
            0xFF06B6D4, temp_field, ss6.str()
        );

        // Row 1 of cards
        std::vector<WidgetPtr> row1_items = {card1, card2, card3};
        auto row1 = row(row1_items);
        row1->gap(StyleValue::point(16.0f))
             .justifyContent(Justify::Center);

        // Row 2 of cards
        std::vector<WidgetPtr> row2_items = {card4, card5, card6};
        auto row2 = row(row2_items);
        row2->gap(StyleValue::point(16.0f))
             .justifyContent(Justify::Center);

        // Main Stack Column
        std::vector<WidgetPtr> main_col_items = {title_col, row1, row2};
        auto main_col = column(main_col_items);
        main_col->gap(StyleValue::point(18.0f))
                .alignItems(Align::Center);

        auto app_root = container(main_col);
        app_root->color(0xFF0B1120)
                .paddingAll(20.0f)
                .flexGrow(1.0f);

        return app_root;
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
