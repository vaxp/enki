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
        auto title = text("Advanced ComboBox & Searchable Select Suite", {
            .color = 0xFFFFFFFF,
            .font_size = 22.0f,
            .font_weight = FontWeight::Bold,
        });

        auto sub = text("Rich form input (Category 3. Input / Forms), single & multi-select tag modes, option grouping, and floating overlay", {
            .color = 0xFF94A3B8,
            .font_size = 13.0f,
        });

        auto title_col = column({
            .align_items = Align::Center,
            .gap = StyleValue::point(6.0f),
            .children = {title, sub},
        });

        // ── 1. Card 1: Single-Select Cloud Region Selector ────────────
        auto c1_title = text("1. Single-Select: Cloud Deployment Region", {
            .color = 0xFF38BDF8,
            .font_size = 14.5f,
            .font_weight = FontWeight::Bold,
        });

        auto c1_sub = text("Select your primary Kubernetes datacenter location:", {
            .color = 0xFF94A3B8,
            .font_size = 12.0f,
        });

        // Input Trigger Box
        auto reg_txt = text(region_label_, { .color = 0xFFFFFFFF, .font_size = 13.0f });
        auto chv_txt = text("⌄", { .color = 0xFF94A3B8, .font_size = 14.0f, .font_weight = FontWeight::Bold });

        auto input1_row = row({
            .justify_content = Justify::SpaceBetween,
            .align_items = Align::Center,
            .width = StyleValue::percent(100.0f),
            .children = {reg_txt, chv_txt},
        });

        auto input1_box = container({
            .color = 0xFF0F172A,
            .border_radius = BorderRadius::circular(8.0f),
            .border = Border(0xFF334155, 1.0f),
            .width = StyleValue::point(360.0f),
            .padding = StyleInsets::symmetric(10.0f, 14.0f),
            .child = input1_row,
        });

        auto input1_gd = std::make_shared<GestureDetector>(input1_box);
        input1_gd->cursor_type = SystemCursor::Pointer;
        input1_gd->on_tap_up = [this](const TapUpDetails&) {
            region_ctrl_->toggle();
        };

        auto card1 = container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(12.0f),
            .border = Border(0xFF334155, 1.0f),
            .width = StyleValue::point(420.0f),
            .padding = StyleInsets::all(20.0f),
            .child = column({
                .gap = StyleValue::point(12.0f),
                .children = {c1_title, c1_sub, input1_gd},
            }),
        });

        // ── 2. Card 2: Multi-Select Tech Stack Selector ───────────────
        auto c2_title = text("2. Multi-Select: Tech Stack & Libraries", {
            .color = 0xFF10B981,
            .font_size = 14.5f,
            .font_weight = FontWeight::Bold,
        });

        auto c2_sub = text("Select packages to bundle into the runtime build:", {
            .color = 0xFF94A3B8,
            .font_size = 12.0f,
        });

        auto tech_txt = text("⚡ Skia, Vulkan, Anu Layout (Click to edit)", { .color = 0xFF38BDF8, .font_size = 13.0f });
        auto chv2_txt = text("⌄", { .color = 0xFF94A3B8, .font_size = 14.0f, .font_weight = FontWeight::Bold });

        auto input2_row = row({
            .justify_content = Justify::SpaceBetween,
            .align_items = Align::Center,
            .width = StyleValue::percent(100.0f),
            .children = {tech_txt, chv2_txt},
        });

        auto input2_box = container({
            .color = 0xFF0F172A,
            .border_radius = BorderRadius::circular(8.0f),
            .border = Border(0xFF334155, 1.0f),
            .width = StyleValue::point(360.0f),
            .padding = StyleInsets::symmetric(10.0f, 14.0f),
            .child = input2_row,
        });

        auto input2_gd = std::make_shared<GestureDetector>(input2_box);
        input2_gd->cursor_type = SystemCursor::Pointer;
        input2_gd->on_tap_up = [this](const TapUpDetails&) {
            tech_ctrl_->toggle();
        };

        auto card2 = container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(12.0f),
            .border = Border(0xFF334155, 1.0f),
            .width = StyleValue::point(420.0f),
            .padding = StyleInsets::all(20.0f),
            .child = column({
                .gap = StyleValue::point(12.0f),
                .children = {c2_title, c2_sub, input2_gd},
            }),
        });

        auto cards_row = row({
            .justify_content = Justify::Center,
            .gap = StyleValue::point(20.0f),
            .children = {card1, card2},
        });

        // ── HUD / Status Box ──────────────────────────────────────────
        auto hud_box = container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(6.0f),
            .border = Border(0xFF334155, 1.0f),
            .width = StyleValue::point(860.0f),
            .padding = StyleInsets::symmetric(8.0f, 16.0f),
            .child = row({
                .children = {
                    text("💡 " + hud_msg_, { .color = 0xFF38BDF8, .font_size = 12.5f }),
                },
            }),
        });

        // ── Assemble Page Body ────────────────────────────────────────
        auto background_page = container({
            .color = 0xFF0B1120,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(24.0f),
            .child = column({
                .align_items = Align::Center,
                .gap = StyleValue::point(24.0f),
                .children = {title_col, cards_row, hud_box},
            }),
        });

        // ── Setup ComboBox 1 Options & Items (Cloud Regions) ──────────
        std::vector<ComboBoxItem> region_items = {
            ComboBoxItem("us-east-1", "US East (N. Virginia)", "🇺🇸", "4.2 ms • 24 Nodes", "North America").setBadge("FAST", 0xFF10B981),
            ComboBoxItem("us-west-2", "US West (Oregon)", "🇺🇸", "12 ms • 18 Nodes", "North America"),
            ComboBoxItem("eu-central-1", "Europe (Frankfurt)", "🇩🇪", "28 ms • 32 Nodes", "Europe").setBadge("EU", 0xFF38BDF8),
            ComboBoxItem("eu-west-1", "Europe (Ireland)", "🇮🇪", "31 ms • 16 Nodes", "Europe"),
            ComboBoxItem("ap-northeast-1", "Asia Pacific (Tokyo)", "🇯🇵", "68 ms • 20 Nodes", "Asia Pacific"),
            ComboBoxItem("ap-southeast-1", "Asia Pacific (Singapore)", "🇸🇬", "74 ms • 14 Nodes", "Asia Pacific"),
        };

        // ── Setup ComboBox 2 Options & Items (Tech Stack) ─────────────
        std::vector<ComboBoxItem> tech_items = {
            ComboBoxItem("skia", "Skia 2D Compositor", "⚡", "Hardware GPU Vulkan backend", "Core Engine").setBadge("CORE", 0xFF10B981),
            ComboBoxItem("anu", "Anu Flexbox Engine", "📐", "Zero-calculation layout engine", "Core Engine").setBadge("CORE", 0xFF10B981),
            ComboBoxItem("rust", "Rust FFI Bindings", "🦀", "High-performance memory safety", "Languages"),
            ComboBoxItem("cpp20", "Modern C++20 Core", "⚡", "Standard template modules", "Languages"),
            ComboBoxItem("wayland", "Wayland Native Protocol", "🌐", "X11 & Wayland compositing", "Display Drivers"),
            ComboBoxItem("vulkan", "Vulkan Compute Shaders", "🚀", "SPIR-V GPU pipeline", "Display Drivers"),
        };

        // combo1 wraps background_page
        WidgetPtr combo1 = ComboBox {
            .items = std::move(region_items),
            .body = background_page,
            .controller = region_ctrl_,
            .mode = ComboBoxMode::Single,
            .width = 360.0f,
            .input_height = 42.0f,
            .anchor_x = 178.0f,
            .anchor_y = 168.0f,
            .on_selected = [this](const ComboBoxItem& it) {
                selected_region_ = it.id;
                region_label_ = it.icon + " " + it.label;
                hud_msg_ = "Selected Region: " + it.label + " (" + it.subtitle + ")";
                setState([] {});
            },
        };

        // combo2 wraps combo1 as its body
        return ComboBox {
            .items = std::move(tech_items),
            .body = combo1,
            .controller = tech_ctrl_,
            .mode = ComboBoxMode::Multi,
            .width = 360.0f,
            .input_height = 42.0f,
            .anchor_x = 618.0f,
            .anchor_y = 168.0f,
            .on_multi_changed = [this](const std::vector<ComboBoxItem>& items) {
                std::string summary = "Active Packages (" + std::to_string(items.size()) + "): ";
                for (size_t i = 0; i < items.size(); ++i) {
                    if (i > 0) summary += ", ";
                    summary += items[i].label;
                }
                hud_msg_ = summary;
                setState([] {});
            },
        };
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
    config.target_fps  = 0;
    config.show_performance_overlay = true;
    config.clear_color = 0xFF0B1120;

    return runApp(std::make_shared<ComboBoxDemoApp>(), config);
}
