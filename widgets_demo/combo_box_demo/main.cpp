/// @file main.cpp
/// @brief ENKI Advanced ComboBox / Searchable Select Interactive Showcase Demo.

#include "enki/app/app.hpp"
#include "enki/widgets/combo_box.hpp"
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

class ComboBoxDemoState : public State {
private:
    std::shared_ptr<ComboBoxController> region_ctrl_;
    std::shared_ptr<ComboBoxController> tech_ctrl_;

    std::string selected_region_ = "us-east-1";
    std::string region_label_ = "🇺🇸 US East (N. Virginia)";
    std::string hud_msg_ = "Click any combo box input field below to open the searchable selection menu.";

public:
    void initState() override {
        State::initState();
        region_ctrl_ = std::make_shared<ComboBoxController>();
        tech_ctrl_ = std::make_shared<ComboBoxController>();
    }

    WidgetPtr build(BuildContext&) override {
        // ── Main Page Header ──────────────────────────────────────────
        auto title = text("Advanced ComboBox & Searchable Select Suite");
        title->fontSize(22.0f).bold().color(0xFFFFFFFF);

        auto sub = text("Rich form input (Category 3. Input / Forms), single & multi-select tag modes, option grouping, and floating overlay");
        sub->fontSize(13.0f).color(0xFF94A3B8);

        std::vector<WidgetPtr> title_items = {title, sub};
        auto title_col = column(title_items);
        title_col->alignItems(Align::Center).gap(StyleValue::point(6.0f));

        // ── 1. Card 1: Single-Select Cloud Region Selector ────────────
        auto c1_title = text("1. Single-Select: Cloud Deployment Region");
        c1_title->fontSize(14.5f).bold().color(0xFF38BDF8);

        auto c1_sub = text("Select your primary Kubernetes datacenter location:");
        c1_sub->fontSize(12.0f).color(0xFF94A3B8);

        // Input Trigger Box
        auto reg_txt = text(region_label_);
        reg_txt->fontSize(13.0f).color(0xFFFFFFFF);

        auto chv_txt = text("⌄");
        chv_txt->fontSize(14.0f).bold().color(0xFF94A3B8);

        std::vector<WidgetPtr> input1_items = {reg_txt, chv_txt};
        auto input1_row = row(input1_items);
        input1_row->justifyContent(Justify::SpaceBetween)
                  .alignItems(Align::Center)
                  .width(StyleValue::percent(100.0f));

        auto input1_box = container(input1_row);
        input1_box->color(0xFF0F172A)
                  .border(0xFF334155, 1.0f)
                  .borderRadius(8.0f)
                  .paddingSymmetric(10.0f, 14.0f)
                  .width(360.0f);

        auto input1_gd = std::make_shared<GestureDetector>(input1_box);
        input1_gd->cursor_type = SystemCursor::Pointer;
        input1_gd->on_tap_up = [this](const TapUpDetails&) {
            region_ctrl_->toggle();
        };

        std::vector<WidgetPtr> card1_items = {c1_title, c1_sub, input1_gd};
        auto card1_col = column(card1_items);
        card1_col->gap(StyleValue::point(12.0f));

        auto card1 = container(card1_col);
        card1->color(0xFF1E293B)
             .borderRadius(12.0f)
             .border(0xFF334155, 1.0f)
             .paddingAll(20.0f)
             .width(420.0f);

        // ── 2. Card 2: Multi-Select Tech Stack Selector ───────────────
        auto c2_title = text("2. Multi-Select: Tech Stack & Libraries");
        c2_title->fontSize(14.5f).bold().color(0xFF10B981);

        auto c2_sub = text("Select packages to bundle into the runtime build:");
        c2_sub->fontSize(12.0f).color(0xFF94A3B8);

        auto tech_txt = text("⚡ Skia, Vulkan, Anu Layout (Click to edit)");
        tech_txt->fontSize(13.0f).color(0xFF38BDF8);

        auto chv2_txt = text("⌄");
        chv2_txt->fontSize(14.0f).bold().color(0xFF94A3B8);

        std::vector<WidgetPtr> input2_items = {tech_txt, chv2_txt};
        auto input2_row = row(input2_items);
        input2_row->justifyContent(Justify::SpaceBetween)
                  .alignItems(Align::Center)
                  .width(StyleValue::percent(100.0f));

        auto input2_box = container(input2_row);
        input2_box->color(0xFF0F172A)
                  .border(0xFF334155, 1.0f)
                  .borderRadius(8.0f)
                  .paddingSymmetric(10.0f, 14.0f)
                  .width(360.0f);

        auto input2_gd = std::make_shared<GestureDetector>(input2_box);
        input2_gd->cursor_type = SystemCursor::Pointer;
        input2_gd->on_tap_up = [this](const TapUpDetails&) {
            tech_ctrl_->toggle();
        };

        std::vector<WidgetPtr> card2_items = {c2_title, c2_sub, input2_gd};
        auto card2_col = column(card2_items);
        card2_col->gap(StyleValue::point(12.0f));

        auto card2 = container(card2_col);
        card2->color(0xFF1E293B)
             .borderRadius(12.0f)
             .border(0xFF334155, 1.0f)
             .paddingAll(20.0f)
             .width(420.0f);

        std::vector<WidgetPtr> cards_list = {card1, card2};
        auto cards_row = row(cards_list);
        cards_row->gap(StyleValue::point(20.0f)).justifyContent(Justify::Center);

        // ── HUD / Status Box ──────────────────────────────────────────
        auto hud_txt = text("💡 " + hud_msg_);
        hud_txt->fontSize(12.5f).color(0xFF38BDF8);

        auto hud_row = row(std::vector<WidgetPtr>{hud_txt});
        auto hud_box = container(hud_row);
        hud_box->color(0xFF1E293B)
               .borderRadius(6.0f)
               .border(0xFF334155, 1.0f)
               .paddingSymmetric(8.0f, 16.0f)
               .width(860.0f);

        // ── Assemble Page Body ────────────────────────────────────────
        std::vector<WidgetPtr> page_items = {title_col, cards_row, hud_box};
        auto page_col = column(page_items);
        page_col->gap(StyleValue::point(24.0f)).alignItems(Align::Center);

        auto background_page = container(page_col);
        background_page->color(0xFF0B1120)
                       .paddingAll(24.0f)
                       .width(StyleValue::percent(100.0f))
                       .height(StyleValue::percent(100.0f));

        // ── Setup ComboBox 1 Options & Items (Cloud Regions) ──────────
        std::vector<ComboBoxItem> region_items = {
            ComboBoxItem("us-east-1", "US East (N. Virginia)", "🇺🇸", "4.2 ms • 24 Nodes", "North America").setBadge("FAST", 0xFF10B981),
            ComboBoxItem("us-west-2", "US West (Oregon)", "🇺🇸", "12 ms • 18 Nodes", "North America"),
            ComboBoxItem("eu-central-1", "Europe (Frankfurt)", "🇩🇪", "28 ms • 32 Nodes", "Europe").setBadge("EU", 0xFF38BDF8),
            ComboBoxItem("eu-west-1", "Europe (Ireland)", "🇮🇪", "31 ms • 16 Nodes", "Europe"),
            ComboBoxItem("ap-northeast-1", "Asia Pacific (Tokyo)", "🇯🇵", "68 ms • 20 Nodes", "Asia Pacific"),
            ComboBoxItem("ap-southeast-1", "Asia Pacific (Singapore)", "🇸🇬", "74 ms • 14 Nodes", "Asia Pacific")
        };

        ComboBoxProps reg_opts;
        reg_opts.items = region_items;
        reg_opts.body = background_page;
        reg_opts.controller = region_ctrl_;
        reg_opts.mode = ComboBoxMode::Single;
        reg_opts.width = 360.0f;
        reg_opts.input_height = 42.0f;
        reg_opts.anchor_x = 178.0f; // Calibrated X of Card 1 input
        reg_opts.anchor_y = 168.0f; // Calibrated Y of Card 1 input top
        reg_opts.on_selected = [this](const ComboBoxItem& it) {
            selected_region_ = it.id;
            region_label_ = it.icon + " " + it.label;
            hud_msg_ = "Selected Region: " + it.label + " (" + it.subtitle + ")";
            setState([] {});
        };

        // ── Setup ComboBox 2 Options & Items (Tech Stack) ─────────────
        std::vector<ComboBoxItem> tech_items = {
            ComboBoxItem("skia", "Skia 2D Compositor", "⚡", "Hardware GPU Vulkan backend", "Core Engine").setBadge("CORE", 0xFF10B981),
            ComboBoxItem("anu", "Anu Flexbox Engine", "📐", "Zero-calculation layout engine", "Core Engine").setBadge("CORE", 0xFF10B981),
            ComboBoxItem("rust", "Rust FFI Bindings", "🦀", "High-performance memory safety", "Languages"),
            ComboBoxItem("cpp20", "Modern C++20 Core", "⚡", "Standard template modules", "Languages"),
            ComboBoxItem("wayland", "Wayland Native Protocol", "🌐", "X11 & Wayland compositing", "Display Drivers"),
            ComboBoxItem("vulkan", "Vulkan Compute Shaders", "🚀", "SPIR-V GPU pipeline", "Display Drivers")
        };

        ComboBoxProps tech_opts;
        tech_opts.items = tech_items;
        // combo2 wraps combo1 as its body
        auto combo1 = comboBox(std::move(reg_opts));
        tech_opts.body = combo1;
        tech_opts.controller = tech_ctrl_;
        tech_opts.mode = ComboBoxMode::Multi;
        tech_opts.width = 360.0f;
        tech_opts.input_height = 42.0f;
        tech_opts.anchor_x = 618.0f; // Calibrated X of Card 2 input
        tech_opts.anchor_y = 168.0f; // Calibrated Y of Card 2 input top
        tech_opts.on_multi_changed = [this](const std::vector<ComboBoxItem>& items) {
            std::string summary = "Active Packages (" + std::to_string(items.size()) + "): ";
            for (size_t i = 0; i < items.size(); ++i) {
                if (i > 0) summary += ", ";
                summary += items[i].label;
            }
            hud_msg_ = summary;
            setState([] {});
        };

        auto combo2 = comboBox(std::move(tech_opts));

        return combo2;
    }
};

class ComboBoxDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<ComboBoxDemoState>();
    }
    std::string_view typeName() const override { return "ComboBoxDemoApp"; }
};

int main() {
    std::cout << "====================================================\n";
    std::cout << "  ENKI Engine — Advanced ComboBox Component Demo\n";
    std::cout << "====================================================\n";

    AppConfig config;
    config.title       = "Enki Engine — Advanced ComboBox Component Demo";
    config.width       = 1180;
    config.height      = 680;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 60;
    config.clear_color = 0xFF0B1120;

    return runApp(std::make_shared<ComboBoxDemoApp>(), config);
}
