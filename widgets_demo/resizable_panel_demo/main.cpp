/// @file main.cpp
/// @brief ENKI Advanced ResizablePanel & Floating Tool Window Interactive Showcase Demo.

#include "enki/app/app.hpp"
#include "enki/widgets/resizable_panel.hpp"
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

class ResizablePanelDemoState : public State {
private:
    std::shared_ptr<ResizablePanelController> panel_ctrl_;
    std::string hud_msg_ = "Drag the top title bar to MOVE the tool window, or drag the bottom-right corner grip (◢) to RESIZE it!";

    // ── Build Inspector Window Content ────────────────────────────
    WidgetPtr buildInspectorContent() {
        auto t = text("⚡ Real-Time Vulkan Shader Pipeline Inspector");
        t->fontSize(13.0f).bold().color(0xFF38BDF8);

        auto l1 = text("• Render Target: 1920x1080 (Skia VK_FORMAT_R8G8B8A8_UNORM)");
        l1->fontSize(11.5f).color(0xFF94A3B8);

        auto l2 = text("• Frame Latency: 4.2ms • VRAM Usage: 184MB / 8192MB");
        l2->fontSize(11.5f).color(0xFF10B981);

        auto l3 = text("• Active Pipeline: SPIR-V Fragment Stage (Optimization: O3)");
        l3->fontSize(11.5f).color(0xFFCBD5E1);

        auto btn_recompile = button(text("Recompile Shader"), [this] {
            hud_msg_ = "Action: SPIR-V Shader pipeline recompiled successfully (0 warnings).";
            setState([] {});
        });

        std::vector<WidgetPtr> items = {t, l1, l2, l3, btn_recompile};
        auto col = column(items);
        col->gap(StyleValue::point(10.0f));
        return col;
    }

public:
    void initState() override {
        State::initState();
        panel_ctrl_ = std::make_shared<ResizablePanelController>();
    }

    WidgetPtr build(BuildContext&) override {
        // ── Main Page Header ──────────────────────────────────────────
        auto title = text("Advanced ResizablePanel Component Suite");
        title->fontSize(22.0f).bold().color(0xFFFFFFFF);

        auto sub = text("Floating window & dockable tool panel (Category 10. Advanced / Data UI), drag-to-move, corner grip resizing, and minimize/maximize");
        sub->fontSize(13.0f).color(0xFF94A3B8);

        std::vector<WidgetPtr> title_items = {title, sub};
        auto title_col = column(title_items);
        title_col->alignItems(Align::Center).gap(StyleValue::point(6.0f));

        // ── Size Preset Buttons ───────────────────────────────────────
        auto btn_compact = button(text("Preset: Compact (380x240)"), [this] {
            panel_ctrl_->setSize(380.0f, 240.0f);
            hud_msg_ = "Resized window to Compact (380x240).";
            setState([] {});
        });

        auto btn_medium = button(text("Preset: Medium (520x340)"), [this] {
            panel_ctrl_->setSize(520.0f, 340.0f);
            hud_msg_ = "Resized window to Medium (520x340).";
            setState([] {});
        });

        auto btn_large = button(text("Preset: Large (680x420)"), [this] {
            panel_ctrl_->setSize(680.0f, 420.0f);
            hud_msg_ = "Resized window to Large (680x420).";
            setState([] {});
        });

        auto btn_reset = button(text("Reset Position & Size"), [this] {
            panel_ctrl_->reset();
            hud_msg_ = "Reset window to default position (240, 120) and size (460x320).";
            setState([] {});
        });

        std::vector<WidgetPtr> preset_items = {btn_compact, btn_medium, btn_large, btn_reset};
        auto preset_row = row(preset_items);
        preset_row->gap(StyleValue::point(10.0f)).justifyContent(Justify::Center);

        // ── Background Workspace Grid Presentation ────────────────────
        auto bg_info = text("🖥️ Workspace Canvas — Floating Tool Window Layered on Top");
        bg_info->fontSize(14.0f).bold().color(0xFF64748B);

        auto bg_desc = text("Drag the window around this desktop surface or resize it dynamically.");
        bg_desc->fontSize(12.5f).color(0xFF475569);

        std::vector<WidgetPtr> ws_items = {bg_info, bg_desc};
        auto ws_col = column(ws_items);
        ws_col->alignItems(Align::Center).gap(StyleValue::point(6.0f));

        auto ws_frame = container(ws_col);
        ws_frame->color(0xFF0F172A)
                .borderRadius(12.0f)
                .border(0xFF334155, 1.0f)
                .paddingAll(40.0f)
                .width(960.0f)
                .height(380.0f);

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
        std::vector<WidgetPtr> page_items = {title_col, preset_row, ws_frame, hud_box};
        auto page_col = column(page_items);
        page_col->gap(StyleValue::point(18.0f)).alignItems(Align::Center);

        auto background_page = container(page_col);
        background_page->color(0xFF0B1120)
                       .paddingAll(24.0f)
                       .width(StyleValue::percent(100.0f))
                       .height(StyleValue::percent(100.0f));

        // ── Resizable Panel Options & Chaining ────────────────────────
        ResizablePanelOptions opts;
        opts.title = "Vulkan Shader Inspector";
        opts.icon = "⚡";
        opts.initial_x = 340.0f;
        opts.initial_y = 190.0f;
        opts.initial_width = 480.0f;
        opts.initial_height = 260.0f;
        opts.on_resized = [this](float w, float h) {
            hud_msg_ = "Window Resized: " + std::to_string(static_cast<int>(w)) + "px × " + std::to_string(static_cast<int>(h)) + "px";
            setState([] {});
        };
        opts.on_moved = [this](float x, float y) {
            hud_msg_ = "Window Position: (" + std::to_string(static_cast<int>(x)) + ", " + std::to_string(static_cast<int>(y)) + ")";
            setState([] {});
        };

        auto resizable_widget = resizablePanel(buildInspectorContent(), background_page, opts, panel_ctrl_);
        return resizable_widget;
    }
};

class ResizablePanelDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<ResizablePanelDemoState>();
    }
    std::string_view typeName() const override { return "ResizablePanelDemoApp"; }
};

int main() {
    std::cout << "====================================================\n";
    std::cout << "  ENKI Engine — Advanced ResizablePanel Demo\n";
    std::cout << "====================================================\n";

    AppConfig config;
    config.title       = "Enki Engine — Advanced ResizablePanel Demo";
    config.width       = 1180;
    config.height      = 720;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 60;
    config.clear_color = 0xFF0B1120;

    return runApp(std::make_shared<ResizablePanelDemoApp>(), config);
}
