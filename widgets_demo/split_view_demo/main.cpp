/// @file main.cpp
/// @brief ENKI Advanced SplitView Interactive Showcase Demo.

#include "enki/app/app.hpp"
#include "enki/widgets/split_view.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/scroll_view.hpp"
#include "enki/state/state.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <memory>

using namespace enki;

class SplitViewDemoState : public State {
private:
    std::shared_ptr<SplitViewController> split_ctrl_;
    SplitOrientation current_orientation_ = SplitOrientation::Horizontal;
    std::string hud_msg_ = "Drag the divider handle (⋮) to resize panes in real-time, or double-click it to reset.";

    // ── 1. Left Pane: Project Explorer ────────────────────────────
    WidgetPtr buildExplorerPane() {
        auto t = text("📁 Workspace Project Explorer");
        t->fontSize(13.5f).bold().color(0xFFFFFFFF);

        auto f1 = text("  ├─ 📂 src/widgets/split_view.cpp");
        f1->fontSize(12.0f).color(0xFF38BDF8);

        auto f2 = text("  ├─ 📂 include/enki/widgets/split_view.hpp");
        f2->fontSize(12.0f).color(0xFF94A3B8);

        auto f3 = text("  ├─ 📂 core/skia_compositor.cpp");
        f3->fontSize(12.0f).color(0xFF94A3B8);

        auto f4 = text("  └─ 📜 meson.build");
        f4->fontSize(12.0f).color(0xFFF59E0B);

        std::vector<WidgetPtr> items = {t, f1, f2, f3, f4};
        auto col = column(items);
        col->gap(StyleValue::point(8.0f));

        auto box = container(col);
        box->color(0xFF0F172A)
           .paddingAll(16.0f)
           .width(StyleValue::percent(100.0f))
           .height(StyleValue::percent(100.0f));
        return box;
    }

    // ── 2. Right / Main Pane: Code Editor ─────────────────────────
    WidgetPtr buildEditorPane() {
        auto t = text("📝 Editor & Vulkan Pipeline Canvas");
        t->fontSize(13.5f).bold().color(0xFFFFFFFF);

        auto code_line1 = text("// ENKI Skia Real-Time Compositor");
        code_line1->fontSize(12.0f).color(0xFF64748B);

        auto code_line2 = text("auto split = splitView(explorer_pane, editor_pane);");
        code_line2->fontSize(12.5f).bold().color(0xFF38BDF8);

        auto code_line3 = text("split->setRatio(0.30f); // 30% sidebar, 70% editor canvas");
        code_line3->fontSize(12.0f).color(0xFF10B981);

        std::vector<WidgetPtr> items = {t, code_line1, code_line2, code_line3};
        auto col = column(items);
        col->gap(StyleValue::point(8.0f));

        auto box = container(col);
        box->color(0xFF1E293B)
           .paddingAll(16.0f)
           .width(StyleValue::percent(100.0f))
           .height(StyleValue::percent(100.0f));
        return box;
    }

public:
    void initState() override {
        State::initState();
        split_ctrl_ = std::make_shared<SplitViewController>();
    }

    WidgetPtr build(BuildContext&) override {
        // ── Main Page Header ──────────────────────────────────────────
        auto title = text("Advanced SplitView Component Suite");
        title->fontSize(22.0f).bold().color(0xFFFFFFFF);

        auto sub = text("Interactive resizable panes (Category 10. Advanced / Data UI), drag handles, cursor updates, and ratio presets");
        sub->fontSize(13.0f).color(0xFF94A3B8);

        std::vector<WidgetPtr> title_items = {title, sub};
        auto title_col = column(title_items);
        title_col->alignItems(Align::Center).gap(StyleValue::point(6.0f));

        // ── Control Bar ───────────────────────────────────────────────
        auto makePill = [](std::string label, bool active, std::function<void()> cb) -> WidgetPtr {
            auto t = text(label);
            t->fontSize(12.0f).color(active ? 0xFFFFFFFF : 0xFF94A3B8);
            if (active) t->bold();

            auto b = container(t);
            b->color(active ? 0xFF0284C7 : 0xFF0F172A)
             .border(active ? 0xFF38BDF8 : 0xFF334155, 1.0f)
             .borderRadius(6.0f)
             .paddingSymmetric(6.0f, 14.0f);

            auto gd = std::make_shared<GestureDetector>(b);
            gd->cursor_type = SystemCursor::Pointer;
            gd->on_tap_up = [cb](const TapUpDetails&) {
                if (cb) cb();
            };
            return gd;
        };

        auto pill_horiz = makePill("↔ Horizontal Split (Left / Right)", current_orientation_ == SplitOrientation::Horizontal, [this] {
            current_orientation_ = SplitOrientation::Horizontal;
            hud_msg_ = "Switched to Horizontal Split layout.";
            setState([] {});
        });

        auto pill_vert = makePill("↕ Vertical Split (Top / Bottom)", current_orientation_ == SplitOrientation::Vertical, [this] {
            current_orientation_ = SplitOrientation::Vertical;
            hud_msg_ = "Switched to Vertical Split layout.";
            setState([] {});
        });

        std::vector<WidgetPtr> orient_items = {pill_horiz, pill_vert};
        auto orient_row = row(orient_items);
        orient_row->gap(StyleValue::point(10.0f)).justifyContent(Justify::Center);

        // Programmatic Ratio Presets
        auto btn_25 = button(text("Snap 25%"), [this] {
            split_ctrl_->setRatio(0.25f);
            hud_msg_ = "Set split ratio to 25% / 75%.";
            setState([] {});
        });

        auto btn_50 = button(text("Snap 50%"), [this] {
            split_ctrl_->setRatio(0.50f);
            hud_msg_ = "Set split ratio to 50% / 50%.";
            setState([] {});
        });

        auto btn_75 = button(text("Snap 75%"), [this] {
            split_ctrl_->setRatio(0.75f);
            hud_msg_ = "Set split ratio to 75% / 25%.";
            setState([] {});
        });

        auto btn_reset = button(text("Reset Default"), [this] {
            split_ctrl_->reset();
            hud_msg_ = "Reset split ratio to default.";
            setState([] {});
        });

        std::vector<WidgetPtr> ratio_items = {btn_25, btn_50, btn_75, btn_reset};
        auto ratio_row = row(ratio_items);
        ratio_row->gap(StyleValue::point(10.0f)).justifyContent(Justify::Center);

        // ── Assemble SplitView Component ──────────────────────────────
        SplitViewOptions opts;
        opts.orientation = current_orientation_;
        opts.initial_ratio = 0.35f;
        opts.handle_thickness = 8.0f;
        opts.on_split_changed = [this](float r) {
            int pct = static_cast<int>(r * 100.0f);
            hud_msg_ = "Current Split Ratio: " + std::to_string(pct) + "% / " + std::to_string(100 - pct) + "%";
            setState([] {});
        };

        auto split_widget = splitView(buildExplorerPane(), buildEditorPane(), opts, split_ctrl_);

        auto split_frame = container(split_widget);
        split_frame->color(0xFF0F172A)
                   .border(0xFF334155, 1.0f)
                   .borderRadius(10.0f)
                   .width(960.0f)
                   .height(360.0f)
                   .shadow(BoxShadow(0x99000000, {0.0f, 6.0f}, 20.0f));

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
        std::vector<WidgetPtr> page_items = {title_col, orient_row, ratio_row, split_frame, hud_box};
        auto page_col = column(page_items);
        page_col->gap(StyleValue::point(16.0f)).alignItems(Align::Center);

        auto background_page = container(page_col);
        background_page->color(0xFF0B1120)
                       .paddingAll(24.0f)
                       .width(StyleValue::percent(100.0f))
                       .height(StyleValue::percent(100.0f));

        return background_page;
    }
};

class SplitViewDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<SplitViewDemoState>();
    }
    std::string_view typeName() const override { return "SplitViewDemoApp"; }
};

int main() {
    std::cout << "====================================================\n";
    std::cout << "  ENKI Engine — Advanced SplitView Component Demo\n";
    std::cout << "====================================================\n";

    AppConfig config;
    config.title       = "Enki Engine — Advanced SplitView Component Demo";
    config.width       = 1180;
    config.height      = 720;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 60;
    config.clear_color = 0xFF0B1120;

    return runApp(std::make_shared<SplitViewDemoApp>(), config);
}
