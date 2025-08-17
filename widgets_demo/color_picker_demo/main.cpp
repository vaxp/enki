/// @file main.cpp
/// @brief ENKI Advanced ColorPicker Interactive Showcase Demo.

#include "enki/app/app.hpp"
#include "enki/widgets/color_picker.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/state/state.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <memory>

using namespace enki;

class ColorPickerDemoState : public State {
private:
    std::string hud_msg_ = "Adjust colors via the 2D Canvas & Hue Slider on the right, or click the Color Wells on the left to see live theme updates.";

    Color brand_color_ = 0xFF38BDF8;      // Sky 400
    Color accent_color_ = 0xFF8B5CF6;     // Violet 500
    Color surface_color_ = 0xFF1E293B;    // Slate 800

public:
    WidgetPtr build(BuildContext&) override {
        // ── Main Page Header ──────────────────────────────────────────
        auto title = text("Advanced ColorPicker & Theme Studio");
        title->fontSize(22.0f).bold().color(0xFFFFFFFF);

        auto sub = text("Enterprise color inspector (Category 3. Input / Forms), 2D Saturation-Value plane, Hue & Alpha sliders, HEX/RGBA/HSV formats, and Palettes");
        sub->fontSize(13.0f).color(0xFF94A3B8);

        std::vector<WidgetPtr> title_items = {title, sub};
        auto title_col = column(title_items);
        title_col->alignItems(Align::Center).gap(StyleValue::point(6.0f));

        // ── Left Column: Theme Palette Settings (Input Popups) ────────
        auto form_title = text("🎨 Theme Design Tokens");
        form_title->fontSize(15.0f).bold().color(0xFF38BDF8);

        auto lbl_brand = text("Primary Brand Color:");
        lbl_brand->fontSize(11.5f).bold().color(0xFF94A3B8);

        ColorPickerProps brand_opts;
        brand_opts.initial_color = brand_color_;
        brand_opts.on_color_changed = [this](Color c) {
            brand_color_ = c;
            hud_msg_ = "Primary Brand Color updated to: " + std::to_string(c);
            setState([] {});
        };
        auto brand_picker = colorPicker(brand_opts);

        auto lbl_acc = text("Accent & Glow Color:");
        lbl_acc->fontSize(11.5f).bold().color(0xFF94A3B8);

        ColorPickerProps acc_opts;
        acc_opts.initial_color = accent_color_;
        acc_opts.on_color_changed = [this](Color c) {
            accent_color_ = c;
            hud_msg_ = "Accent Color updated to: " + std::to_string(c);
            setState([] {});
        };
        auto acc_picker = colorPicker(acc_opts);

        std::vector<WidgetPtr> left_items = {
            form_title,
            lbl_brand, brand_picker,
            lbl_acc, acc_picker
        };
        auto left_col = column(left_items);
        left_col->gap(StyleValue::point(10.0f));

        auto left_card = container(left_col);
        left_card->color(0xFF0F172A)
                 .border(0xFF334155, 1.0f)
                 .borderRadius(12.0f)
                 .paddingAll(20.0f)
                 .width(320.0f);

        // ── Center Column: Live Inline Color Studio ───────────────────
        auto studio_title = text("🖌️ Color Studio (Inline)");
        studio_title->fontSize(15.0f).bold().color(0xFF10B981);

        ColorPickerProps studio_opts;
        studio_opts.mode = ColorPickerMode::Inline;
        studio_opts.initial_color = brand_color_;
        studio_opts.enable_alpha = true;
        studio_opts.on_color_changed = [this](Color c) {
            brand_color_ = c;
            hud_msg_ = "Studio Color changed via 2D Canvas";
            setState([] {});
        };
        auto studio_picker = colorPicker(studio_opts);

        std::vector<WidgetPtr> studio_items = {studio_title, studio_picker};
        auto studio_col = column(studio_items);
        studio_col->gap(StyleValue::point(10.0f)).alignItems(Align::Center);

        auto studio_card = container(studio_col);
        studio_card->color(0xFF0F172A)
                   .border(0xFF334155, 1.0f)
                   .borderRadius(12.0f)
                   .paddingAll(20.0f)
                   .width(320.0f);

        // ── Right Column: Live Dynamic Preview UI ─────────────────────
        auto prev_title = text("✨ Live Dynamic Theme Preview");
        prev_title->fontSize(15.0f).bold().color(0xFFF59E0B);

        auto prev_card_title = text("Enterprise Dashboard Pro");
        prev_card_title->fontSize(14.0f).bold().color(brand_color_);

        auto prev_desc = text("This card dynamically reacts to the selected colors, borders, and accent glow.");
        prev_desc->fontSize(11.5f).color(0xFFCBD5E1);

        // Styled button using brand_color_
        auto btn_t = text("🚀 Launch Mission");
        btn_t->fontSize(12.0f).bold().color(0xFFFFFFFF);
        auto btn_b = container(btn_t);
        btn_b->color(brand_color_).borderRadius(6.0f).paddingSymmetric(8.0f, 16.0f);

        // Styled badge using accent_color_
        auto badg_t = text("PRO FEATURE");
        badg_t->fontSize(10.0f).bold().color(accent_color_);
        auto badg_b = container(badg_t);
        badg_b->color(0x338B5CF6).border(accent_color_, 1.0f).borderRadius(12.0f).paddingSymmetric(3.0f, 8.0f);

        std::vector<WidgetPtr> row_act = {btn_b, badg_b};
        auto row_actions = row(row_act);
        row_actions->gap(StyleValue::point(10.0f)).alignItems(Align::Center);

        std::vector<WidgetPtr> prev_items = {prev_title, prev_card_title, prev_desc, row_actions};
        auto prev_col = column(prev_items);
        prev_col->gap(StyleValue::point(14.0f));

        auto prev_card = container(prev_col);
        prev_card->color(surface_color_)
                 .border(brand_color_, 1.5f)
                 .borderRadius(12.0f)
                 .paddingAll(20.0f)
                 .width(320.0f)
                 .shadow(BoxShadow(brand_color_ & 0x66FFFFFF, {0.0f, 6.0f}, 20.0f));

        // ── Side-by-Side Main Sections ────────────────────────────────
        std::vector<WidgetPtr> sections = {left_card, studio_card, prev_card};
        auto sections_row = row(sections);
        sections_row->gap(StyleValue::point(20.0f))
                    .justifyContent(Justify::Center)
                    .alignItems(Align::Start);

        // ── HUD / Status Box ──────────────────────────────────────────
        auto hud_txt = text("💡 " + hud_msg_);
        hud_txt->fontSize(12.5f).color(0xFF38BDF8);

        auto hud_row = row(std::vector<WidgetPtr>{hud_txt});
        auto hud_box = container(hud_row);
        hud_box->color(0xFF1E293B)
               .borderRadius(6.0f)
               .border(0xFF334155, 1.0f)
               .paddingSymmetric(8.0f, 16.0f)
               .width(960.0f);

        // ── Assemble Page Body ────────────────────────────────────────
        std::vector<WidgetPtr> page_items = {title_col, sections_row, hud_box};
        auto page_col = column(page_items);
        page_col->gap(StyleValue::point(20.0f)).alignItems(Align::Center);

        auto background_page = container(page_col);
        background_page->color(0xFF0B1120)
                       .paddingAll(24.0f)
                       .width(StyleValue::percent(100.0f))
                       .height(StyleValue::percent(100.0f));

        return background_page;
    }
};

class ColorPickerDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<ColorPickerDemoState>();
    }
    std::string_view typeName() const override { return "ColorPickerDemoApp"; }
};

int main() {
    std::cout << "====================================================\n";
    std::cout << "  ENKI Engine — Advanced ColorPicker Demo\n";
    std::cout << "====================================================\n";

    AppConfig config;
    config.title       = "Enki Engine — Advanced ColorPicker Demo";
    config.width       = 1240;
    config.height      = 760;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 60;
    config.clear_color = 0xFF0B1120;

    return runApp(std::make_shared<ColorPickerDemoApp>(), config);
}
