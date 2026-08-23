/// @file main.cpp
/// @brief ENKI Advanced DropdownMenu In-Window Overlay Demo.
///
/// Architecture: DropdownMenu wraps the entire page body.
/// Trigger buttons live INSIDE the body and call controller->toggle().
/// Opening a dropdown adds a Positioned floating panel above all page content.

#include "enki/app/app.hpp"
#include "enki/widgets/dropdown_menu.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/stack.hpp"
#include "enki/state/state.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <memory>

using namespace enki;

class DropdownDemoState : public State {
private:
    std::string hud_msg_  = "Click any dropdown trigger to open the in-window overlay menu.";
    std::string sel_tech_ = "cpp";
    std::string sel_env_  = "prod";
    bool dark_theme_      = true;
    bool auto_save_       = true;
    bool gpu_skia_        = true;

    std::shared_ptr<DropdownMenuController> ctrl_tech_;
    std::shared_ptr<DropdownMenuController> ctrl_ws_;
    std::shared_ptr<DropdownMenuController> ctrl_env_;

public:
    void initState() override {
        State::initState();
        ctrl_tech_ = std::make_shared<DropdownMenuController>();
        ctrl_ws_   = std::make_shared<DropdownMenuController>();
        ctrl_env_  = std::make_shared<DropdownMenuController>();
    }

    // ── Helper: Clickable Trigger Box ─────────────────────────────

    WidgetPtr makeTriggerBtn(const std::string& label, float width,
                             std::shared_ptr<DropdownMenuController> ctrl,
                             bool highlighted = false) {
        auto lbl = text(label, {
            .color = highlighted ? 0xFFF1F5F9 : 0xFFF1F5F9,
            .font_size = 12.5f,
        });

        auto box = container({
            .color = 0xFF0F172A,
            .border_radius = BorderRadius::circular(6.0f),
            .border = Border(highlighted ? 0xFF38BDF8 : 0xFF334155, 1.0f),
            .width = StyleValue::point(width),
            .padding = StyleInsets::symmetric(6.0f, 12.0f),
            .child = lbl,
        });

        auto gd = std::make_shared<GestureDetector>(box);
        gd->cursor_type = SystemCursor::Pointer;
        gd->on_tap_up = [ctrl](const TapUpDetails&) {
            if (ctrl) ctrl->toggle();
        };
        gd->on_hover_enter = [box](const PointerEvent&) {
            box->border(0xFF64748B, 1.0f);
        };
        return gd;
    }

    // ── Page Body ─────────────────────────────────────────────────

    WidgetPtr buildPageBody() {
        // Title
        auto title = text("Advanced DropdownMenu Overlay Suite", {
            .color = 0xFFFFFFFF,
            .font_size = 22.0f,
            .font_weight = FontWeight::Bold,
        });

        auto sub = text("In-window stack overlay (Category 7. Overlays) — select triggers, custom triggers, rich item types", {
            .color = 0xFF94A3B8,
            .font_size = 13.0f,
        });

        auto title_col = column({
            .align_items = Align::Center,
            .gap = StyleValue::point(6.0f),
            .children = {title, sub},
        });

        // ── Card 1 — Technology Selector ─────────────────────────
        auto c1_title = text("1. Select Framework", {
            .color = 0xFF38BDF8,
            .font_size = 14.0f,
            .font_weight = FontWeight::Bold,
        });

        auto c1_sub = text("Standard select trigger with icons & badges", {
            .color = 0xFF94A3B8,
            .font_size = 11.5f,
        });

        // Show currently selected value in trigger
        std::string tech_label = "⚡ C++20 (Enki Native)  ▼";
        if (sel_tech_ == "rust") tech_label = "🦀 Rust Skia Engine  ▼";
        else if (sel_tech_ == "go") tech_label = "🐹 Go Concurrency  ▼";
        else if (sel_tech_ == "ts") tech_label = "🔷 TypeScript / Node  ▼";
        else if (sel_tech_ == "py") tech_label = "🐍 Python 3.12  ▼";
        else if (sel_tech_ == "kt") tech_label = "☕ Kotlin  ▼";
        auto tech_btn = makeTriggerBtn(tech_label, 260.0f, ctrl_tech_);

        auto card1 = container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(10.0f),
            .border = Border(0xFF334155, 1.0f),
            .width = StyleValue::point(300.0f),
            .padding = StyleInsets::all(16.0f),
            .child = column({
                .gap = StyleValue::point(10.0f),
                .children = {c1_title, c1_sub, tech_btn},
            }),
        });

        // ── Card 2 — Workspace Actions ───────────────────────────
        auto c2_title = text("2. Workspace Actions", {
            .color = 0xFF10B981,
            .font_size = 14.0f,
            .font_weight = FontWeight::Bold,
        });

        auto c2_sub = text("Custom kebab (⋮) trigger with shortcuts & toggles", {
            .color = 0xFF94A3B8,
            .font_size = 11.5f,
        });

        auto ws_btn = makeTriggerBtn("⋮  Options ▾", 130.0f, ctrl_ws_);

        auto card2 = container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(10.0f),
            .border = Border(0xFF334155, 1.0f),
            .width = StyleValue::point(300.0f),
            .padding = StyleInsets::all(16.0f),
            .child = column({
                .gap = StyleValue::point(10.0f),
                .children = {c2_title, c2_sub, ws_btn},
            }),
        });

        // ── Card 3 — Environment Selector ────────────────────────
        auto c3_title = text("3. Cloud Deployment Target", {
            .color = 0xFFF59E0B,
            .font_size = 14.0f,
            .font_weight = FontWeight::Bold,
        });

        auto c3_sub = text("Radio group with status badges", {
            .color = 0xFF94A3B8,
            .font_size = 11.5f,
        });

        std::string env_label = "🌐 Production (US-East)  ▼";
        if (sel_env_ == "local") env_label = "💻 Localhost (Port 8080)  ▼";
        else if (sel_env_ == "stage") env_label = "🧪 Staging Cluster (QA)  ▼";
        auto env_btn = makeTriggerBtn(env_label, 260.0f, ctrl_env_, true);

        auto card3 = container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(10.0f),
            .border = Border(0xFF334155, 1.0f),
            .width = StyleValue::point(300.0f),
            .padding = StyleInsets::all(16.0f),
            .child = column({
                .gap = StyleValue::point(10.0f),
                .children = {c3_title, c3_sub, env_btn},
            }),
        });

        // Cards row
        auto cards_row = row({
            .justify_content = Justify::Center,
            .gap = StyleValue::point(16.0f),
            .children = {card1, card2, card3},
        });

        // HUD
        auto hud_box = container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(6.0f),
            .border = Border(0xFF334155, 1.0f),
            .width = StyleValue::point(960.0f),
            .padding = StyleInsets::symmetric(8.0f, 16.0f),
            .child = text("💡 " + hud_msg_, { .color = 0xFF38BDF8, .font_size = 12.5f }),
        });

        return container({
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
    }

    WidgetPtr build(BuildContext&) override {
        auto body = buildPageBody();

        // ── Dropdown 1 — Technology (anchor below Card 1 trigger) ──
        std::vector<DropdownMenuItem> tech_items = {
            DropdownMenuItem::header("SYSTEMS & NATIVE"),
            DropdownMenuItem::standard("cpp",  "C++20 (Enki Native)",    "⚡").setBadge("CORE", 0x2010B981, 0xFF10B981),
            DropdownMenuItem::standard("rust", "Rust Skia Engine",       "🦀").setBadge("FAST", 0x2038BDF8, 0xFF38BDF8),
            DropdownMenuItem::standard("go",   "Go Concurrency",         "🐹"),
            DropdownMenuItem::divider(),
            DropdownMenuItem::header("MANAGED & WEB"),
            DropdownMenuItem::standard("ts",   "TypeScript / Node",      "🔷"),
            DropdownMenuItem::standard("py",   "Python 3.12 Bindings",   "🐍").setBadge("AI", 0x20F59E0B, 0xFFF59E0B),
            DropdownMenuItem::standard("kt",   "Kotlin Multiplatform",   "☕"),
        };

        WidgetPtr dm1 = DropdownMenu {
            .items          = std::move(tech_items),
            .body           = body,
            .controller     = ctrl_tech_,
            .menu_width     = 280.0f,
            .selected_id    = sel_tech_,
            .trigger_height = 38.0f,
            .anchor_x       = 125.0f,
            .anchor_y       = 165.0f,
            .on_selected    = [this](const DropdownMenuItem& it) {
                sel_tech_ = it.id;
                hud_msg_ = "Technology: " + it.label;
                setState([] {});
            },
        };

        // ── Dropdown 2 — Workspace Actions ────────────────────────
        std::vector<DropdownMenuItem> ws_items = {
            DropdownMenuItem::header("WORKSPACE ACTIONS"),
            DropdownMenuItem::standard("new",  "New Project",       "📁", "Ctrl+N"),
            DropdownMenuItem::standard("open", "Open File",         "📂", "Ctrl+O"),
            DropdownMenuItem::standard("save", "Save Snapshot",     "💾", "Ctrl+S"),
            DropdownMenuItem::divider(),
            DropdownMenuItem::header("PREFERENCES"),
            DropdownMenuItem::checkbox("dark", "Dark Mode Theme",   dark_theme_,  "🌙"),
            DropdownMenuItem::checkbox("auto", "Auto-Save Changes", auto_save_,   "⚡"),
            DropdownMenuItem::checkbox("gpu",  "GPU Acceleration",  gpu_skia_,    "🚀"),
            DropdownMenuItem::divider(),
            DropdownMenuItem::standard("del",  "Delete Workspace",  "🗑️", "Shift+Del").setDanger(true),
        };

        WidgetPtr dm2 = DropdownMenu {
            .items          = std::move(ws_items),
            .body           = dm1,
            .controller     = ctrl_ws_,
            .menu_width     = 260.0f,
            .trigger_height = 38.0f,
            .anchor_x       = 453.0f,
            .anchor_y       = 165.0f,
            .on_selected    = [this](const DropdownMenuItem& it) {
                hud_msg_ = "Action: " + it.label;
                setState([] {});
            },
            .on_toggle_checked = [this](const std::string& id, bool chk) {
                if (id == "dark") dark_theme_ = chk;
                else if (id == "auto") auto_save_ = chk;
                else if (id == "gpu")  gpu_skia_ = chk;
                hud_msg_ = "Setting [" + id + "] → " + (chk ? "ON" : "OFF");
                setState([] {});
            },
        };

        // ── Dropdown 3 — Environment ──────────────────────────────
        std::vector<DropdownMenuItem> env_items = {
            DropdownMenuItem::header("DEPLOYMENT TARGET"),
            DropdownMenuItem::radio("local", "Localhost (Port 8080)",  sel_env_ == "local", "💻").setSubtitle("Dev & hot reload"),
            DropdownMenuItem::radio("stage", "Staging Cluster (QA)",   sel_env_ == "stage", "🧪").setSubtitle("Pre-production validation"),
            DropdownMenuItem::radio("prod",  "Production (US-East)",   sel_env_ == "prod",  "🌐")
                .setBadge("LIVE", 0x20F59E0B, 0xFFF59E0B).setSubtitle("99.99% HA SLA"),
        };

        return DropdownMenu {
            .items          = std::move(env_items),
            .body           = dm2,
            .controller     = ctrl_env_,
            .menu_width     = 290.0f,
            .selected_id    = sel_env_,
            .trigger_height = 38.0f,
            .anchor_x       = 780.0f,
            .anchor_y       = 165.0f,
            .on_selected    = [this](const DropdownMenuItem& it) {
                sel_env_ = it.id;
                hud_msg_ = "Cluster: " + it.label;
                setState([] {});
            },
        };
    }
};

class DropdownMenuDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<DropdownDemoState>();
    }
    std::string_view typeName() const override { return "DropdownMenuDemoApp"; }
};

int main() {
    std::cout << "====================================================\n";
    std::cout << "  ENKI Engine — Advanced DropdownMenu Overlay Demo\n";
    std::cout << "====================================================\n";

    AppConfig config;
    config.title       = "Enki Engine — Advanced DropdownMenu Overlay Demo";
    config.width       = 1180;
    config.height      = 680;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 0;
    config.show_performance_overlay = true;
    config.clear_color = 0xFF0B1120;

    return runApp(std::make_shared<DropdownMenuDemoApp>(), config);
}
