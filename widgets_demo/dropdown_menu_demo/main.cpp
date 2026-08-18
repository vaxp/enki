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
        auto lbl = text(label);
        lbl->fontSize(12.5f).color(highlighted ? 0xFFF1F5F9 : 0xFFF1F5F9);

        auto box = container(lbl);
        box->color(0xFF0F172A)
           .borderRadius(6.0f)
           .border(highlighted ? 0xFF38BDF8 : 0xFF334155, 1.0f)
           .paddingSymmetric(6.0f, 12.0f)
           .width(width);

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
        auto title = text("Advanced DropdownMenu Overlay Suite");
        title->fontSize(22.0f).bold().color(0xFFFFFFFF);

        auto sub = text("In-window stack overlay (Category 7. Overlays) — select triggers, custom triggers, rich item types");
        sub->fontSize(13.0f).color(0xFF94A3B8);

        std::vector<WidgetPtr> title_items = {title, sub};
        auto title_col = column(title_items);
        title_col->alignItems(Align::Center).gap(StyleValue::point(6.0f));

        // ── Card 1 — Technology Selector ─────────────────────────
        auto c1_title = text("1. Select Framework");
        c1_title->fontSize(14.0f).bold().color(0xFF38BDF8);

        auto c1_sub = text("Standard select trigger with icons & badges");
        c1_sub->fontSize(11.5f).color(0xFF94A3B8);

        // Show currently selected value in trigger
        std::string tech_label = "⚡ C++20 (Enki Native)  ▼";
        if (sel_tech_ == "rust") tech_label = "🦀 Rust Skia Engine  ▼";
        else if (sel_tech_ == "go") tech_label = "🐹 Go Concurrency  ▼";
        else if (sel_tech_ == "ts") tech_label = "🔷 TypeScript / Node  ▼";
        else if (sel_tech_ == "py") tech_label = "🐍 Python 3.12  ▼";
        else if (sel_tech_ == "kt") tech_label = "☕ Kotlin  ▼";
        auto tech_btn = makeTriggerBtn(tech_label, 260.0f, ctrl_tech_);

        std::vector<WidgetPtr> c1_items = {c1_title, c1_sub, tech_btn};
        auto c1_col = column(c1_items);
        c1_col->gap(StyleValue::point(10.0f));
        auto card1 = container(c1_col);
        card1->color(0xFF1E293B).borderRadius(10.0f)
              .border(0xFF334155, 1.0f).paddingAll(16.0f).width(300.0f);

        // ── Card 2 — Workspace Actions ───────────────────────────
        auto c2_title = text("2. Workspace Actions");
        c2_title->fontSize(14.0f).bold().color(0xFF10B981);

        auto c2_sub = text("Custom kebab (⋮) trigger with shortcuts & toggles");
        c2_sub->fontSize(11.5f).color(0xFF94A3B8);

        auto ws_btn = makeTriggerBtn("⋮  Options ▾", 130.0f, ctrl_ws_);

        std::vector<WidgetPtr> c2_items = {c2_title, c2_sub, ws_btn};
        auto c2_col = column(c2_items);
        c2_col->gap(StyleValue::point(10.0f));
        auto card2 = container(c2_col);
        card2->color(0xFF1E293B).borderRadius(10.0f)
              .border(0xFF334155, 1.0f).paddingAll(16.0f).width(300.0f);

        // ── Card 3 — Environment Selector ────────────────────────
        auto c3_title = text("3. Cloud Deployment Target");
        c3_title->fontSize(14.0f).bold().color(0xFFF59E0B);

        auto c3_sub = text("Radio group with status badges");
        c3_sub->fontSize(11.5f).color(0xFF94A3B8);

        std::string env_label = "🌐 Production (US-East)  ▼";
        if (sel_env_ == "local") env_label = "💻 Localhost (Port 8080)  ▼";
        else if (sel_env_ == "stage") env_label = "🧪 Staging Cluster (QA)  ▼";
        auto env_btn = makeTriggerBtn(env_label, 260.0f, ctrl_env_, true);

        std::vector<WidgetPtr> c3_items = {c3_title, c3_sub, env_btn};
        auto c3_col = column(c3_items);
        c3_col->gap(StyleValue::point(10.0f));
        auto card3 = container(c3_col);
        card3->color(0xFF1E293B).borderRadius(10.0f)
              .border(0xFF334155, 1.0f).paddingAll(16.0f).width(300.0f);

        // Cards row
        std::vector<WidgetPtr> cards = {card1, card2, card3};
        auto cards_row = row(cards);
        cards_row->gap(StyleValue::point(16.0f)).justifyContent(Justify::Center);

        // HUD
        auto hud_txt = text("💡 " + hud_msg_);
        hud_txt->fontSize(12.5f).color(0xFF38BDF8);
        auto hud_box = container(hud_txt);
        hud_box->color(0xFF1E293B).borderRadius(6.0f)
               .border(0xFF334155, 1.0f).paddingSymmetric(8.0f, 16.0f)
               .width(960.0f);

        std::vector<WidgetPtr> page_items = {title_col, cards_row, hud_box};
        auto page_col = column(page_items);
        page_col->gap(StyleValue::point(24.0f)).alignItems(Align::Center);

        auto body = container(page_col);
        body->color(0xFF0B1120).paddingAll(24.0f)
             .width(StyleValue::percent(100.0f))
             .height(StyleValue::percent(100.0f));
        return body;
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
        DropdownMenuOptions tech_opts;
        tech_opts.selected_id    = sel_tech_;
        tech_opts.menu_width     = 280.0f;
        tech_opts.trigger_height = 38.0f;
        tech_opts.anchor_x       = 125.0f;   // x of Card 1 trigger
        tech_opts.anchor_y       = 165.0f;   // y of Card 1 trigger top
        tech_opts.on_selected    = [this](const DropdownMenuItem& it) {
            sel_tech_ = it.id;
            hud_msg_ = "Technology: " + it.label;
            setState([] {});
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
        DropdownMenuOptions ws_opts;
        ws_opts.menu_width     = 260.0f;
        ws_opts.trigger_height = 38.0f;
        ws_opts.anchor_x       = 453.0f;  // x of Card 2 trigger
        ws_opts.anchor_y       = 165.0f;
        ws_opts.on_selected    = [this](const DropdownMenuItem& it) {
            hud_msg_ = "Action: " + it.label;
            setState([] {});
        };
        ws_opts.on_toggle_checked = [this](const std::string& id, bool chk) {
            if (id == "dark") dark_theme_ = chk;
            else if (id == "auto") auto_save_ = chk;
            else if (id == "gpu")  gpu_skia_ = chk;
            hud_msg_ = "Setting [" + id + "] → " + (chk ? "ON" : "OFF");
            setState([] {});
        };

        // ── Dropdown 3 — Environment ──────────────────────────────
        std::vector<DropdownMenuItem> env_items = {
            DropdownMenuItem::header("DEPLOYMENT TARGET"),
            DropdownMenuItem::radio("local", "Localhost (Port 8080)",  sel_env_ == "local", "💻").setSubtitle("Dev & hot reload"),
            DropdownMenuItem::radio("stage", "Staging Cluster (QA)",   sel_env_ == "stage", "🧪").setSubtitle("Pre-production validation"),
            DropdownMenuItem::radio("prod",  "Production (US-East)",   sel_env_ == "prod",  "🌐")
                .setBadge("LIVE", 0x20F59E0B, 0xFFF59E0B).setSubtitle("99.99% HA SLA"),
        };
        DropdownMenuOptions env_opts;
        env_opts.selected_id   = sel_env_;
        env_opts.menu_width    = 290.0f;
        env_opts.trigger_height = 38.0f;
        env_opts.anchor_x      = 780.0f;  // x of Card 3 trigger
        env_opts.anchor_y      = 165.0f;
        env_opts.on_selected   = [this](const DropdownMenuItem& it) {
            sel_env_ = it.id;
            hud_msg_ = "Cluster: " + it.label;
            setState([] {});
        };

        // Chain: dm1(body) → dm2(dm1) → dm3(dm2)  [same body-wrapping as Drawer]
        auto dm1 = std::make_shared<DropdownMenu>(tech_items, body, tech_opts);
        dm1->setController(ctrl_tech_);

        auto dm2 = std::make_shared<DropdownMenu>(ws_items, dm1, ws_opts);
        dm2->setController(ctrl_ws_);

        auto dm3 = std::make_shared<DropdownMenu>(env_items, dm2, env_opts);
        dm3->setController(ctrl_env_);

        return dm3;
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
    config.target_fps  = 60;
    config.clear_color = 0xFF0B1120;

    return runApp(std::make_shared<DropdownMenuDemoApp>(), config);
}
