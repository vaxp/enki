/// @file main.cpp
/// @brief ENKI Advanced PasswordField Interactive Showcase Demo.

#include "enki/app/app.hpp"
#include "enki/widgets/password_field.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/state/state.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <iomanip>
#include <sstream>

using namespace enki;

class PasswordFieldDemoState : public State {
private:
    std::shared_ptr<PasswordFieldController> signup_ctrl_;
    std::shared_ptr<PasswordFieldController> login_ctrl_;
    std::shared_ptr<PasswordFieldController> peek_ctrl_;

public:
    void initState() override {
        State::initState();
        signup_ctrl_ = std::make_shared<PasswordFieldController>();
        login_ctrl_  = std::make_shared<PasswordFieldController>("SuperSecret123!");
        peek_ctrl_   = std::make_shared<PasswordFieldController>("987654");
    }

    WidgetPtr build(BuildContext&) override {
        // Main Header
        auto title = text("Advanced PasswordField Widget Suite", {
            .color = 0xFFFFFFFF,
            .font_size = 22.0f,
            .font_weight = FontWeight::Bold,
        });

        auto sub = text("Live strength evaluator, criteria checklist, CapsLock detection, eye toggle/peek, and secure generator", {
            .color = 0xFF94A3B8,
            .font_size = 13.0f,
        });

        auto title_col = column({
            .align_items = Align::Center,
            .children = {title, sub}
        });

        // ── Card 1: User Registration / Sign-Up (Full Suite) ─────────
        auto signup_field = PasswordField {
            .controller = signup_ctrl_,
            .placeholder = "Create a strong password...",
            .show_visibility_toggle = true,
            .show_clear_button = true,
            .show_generator_button = true,
            .show_strength_meter = true,
            .show_rules_checklist = true,
            .on_changed = [this](std::string_view) { setState([] {}); }
        };

        auto c1_title = text("1. User Registration (Strength Meter & Criteria)", {
            .color = 0xFF38BDF8,
            .font_size = 14.0f,
            .font_weight = FontWeight::Bold,
        });

        auto c1_desc = text("Includes live 4-segment strength meter, 5-point criteria checklist, and [🎲] generator.", {
            .color = 0xFF94A3B8,
            .font_size = 12.0f,
        });

        auto card1 = container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(10.0f),
            .border = Border(0xFF334155, 1.0f),
            .width = StyleValue::point(540.0f),
            .padding = StyleInsets::all(16.0f),
            .child = column({
                .gap = StyleValue::point(8.0f),
                .children = {c1_title, c1_desc, signup_field}
            })
        });

        // ── Card 2: User Login Password ──────────────────────────────
        auto login_field = PasswordField {
            .controller = login_ctrl_,
            .placeholder = "Enter account password...",
            .show_visibility_toggle = true,
            .show_capslock_warning = true,
            .focus_border_color = 0xFF10B981,
            .on_changed = [this](std::string_view) { setState([] {}); }
        };

        auto c2_title = text("2. User Login (Eye Toggle & CapsLock Warning)", {
            .color = 0xFF10B981,
            .font_size = 14.0f,
            .font_weight = FontWeight::Bold,
        });

        auto c2_desc = text("Clean input with toggle button [👁] and automatic [⇪ CAPS] warning detection.", {
            .color = 0xFF94A3B8,
            .font_size = 12.0f,
        });

        auto card2 = container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(10.0f),
            .border = Border(0xFF334155, 1.0f),
            .width = StyleValue::point(540.0f),
            .padding = StyleInsets::all(16.0f),
            .child = column({
                .gap = StyleValue::point(8.0f),
                .children = {c2_title, c2_desc, login_field}
            })
        });

        // ── Card 3: Hold-to-Peek PIN / Password ───────────────────────
        auto peek_field = PasswordField {
            .controller = peek_ctrl_,
            .placeholder = "Enter 6-digit PIN...",
            .show_visibility_toggle = true,
            .hold_to_peek = true,
            .focus_border_color = 0xFFF59E0B,
            .on_changed = [this](std::string_view) { setState([] {}); }
        };

        auto c3_title = text("3. Hold-to-Peek Mode (Press & Hold Eye Button)", {
            .color = 0xFFF59E0B,
            .font_size = 14.0f,
            .font_weight = FontWeight::Bold,
        });

        auto c3_desc = text("Press and hold the eye button [👁] to reveal temporarily, re-obscures on release.", {
            .color = 0xFF94A3B8,
            .font_size = 12.0f,
        });

        auto card3 = container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(10.0f),
            .border = Border(0xFF334155, 1.0f),
            .width = StyleValue::point(540.0f),
            .padding = StyleInsets::all(16.0f),
            .child = column({
                .gap = StyleValue::point(8.0f),
                .children = {c3_title, c3_desc, peek_field}
            })
        });

        // ── Security Inspector Panel ──────────────────────────────────
        auto status_hdr = text("🔒 Live Security & Entropy Inspector", {
            .color = 0xFFFFFFFF,
            .font_size = 13.5f,
            .font_weight = FontWeight::Bold,
        });

        std::ostringstream ss_ent;
        ss_ent << "Sign-Up Entropy: " << std::fixed << std::setprecision(1) << signup_ctrl_->calculateEntropy() << " bits";
        auto st_ent = text(ss_ent.str(), {
            .color = 0xFF38BDF8,
            .font_size = 12.0f,
        });

        std::string st_str = "Sign-Up Rules Passed: " + std::string(signup_ctrl_->meetsAllRules() ? "YES (All Satisfied ✓)" : "NO (Criteria Missing)");
        auto st_rules = text(st_str, {
            .color = signup_ctrl_->meetsAllRules() ? 0xFF10B981 : 0xFFF59E0B,
            .font_size = 12.0f,
        });

        auto st_len = text("Sign-Up Password Length: " + std::to_string(signup_ctrl_->getPassword().length()) + " characters", {
            .color = 0xFF94A3B8,
            .font_size = 12.0f,
        });

        auto status_box = container({
            .color = 0xFF0F172A,
            .border_radius = BorderRadius::circular(8.0f),
            .border = Border(0xFF334155, 1.0f),
            .width = StyleValue::point(540.0f),
            .padding = StyleInsets::all(12.0f),
            .child = column({
                .gap = StyleValue::point(4.0f),
                .children = {status_hdr, st_ent, st_rules, st_len}
            })
        });

        // Top Row: Card1 & Card2
        auto row1 = row({
            .justify_content = Justify::Center,
            .gap = StyleValue::point(16.0f),
            .children = {card1, card2}
        });

        // Bottom Row: Card3 & Status Box
        auto row2 = row({
            .justify_content = Justify::Center,
            .gap = StyleValue::point(16.0f),
            .children = {card3, status_box}
        });

        // Main Column
        auto main_col = column({
            .align_items = Align::Center,
            .gap = StyleValue::point(16.0f),
            .children = {title_col, row1, row2}
        });

        return container({
            .color = 0xFF0B1120,
            .padding = StyleInsets::all(20.0f),
            .flex_grow = 1.0f,
            .child = main_col
        });
    }
};

class PasswordFieldDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<PasswordFieldDemoState>();
    }
    std::string_view typeName() const override { return "PasswordFieldDemoApp"; }
};

int main() {
    std::cout << "====================================================\n";
    std::cout << "  ENKI Engine — Advanced PasswordField Widget Demo\n";
    std::cout << "====================================================\n";

    AppConfig config;
    config.title       = "Enki Engine — Advanced PasswordField Demo";
    config.width       = 1180;
    config.height      = 680;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 60;
    config.clear_color = 0xFF0B1120;

    return runApp(std::make_shared<PasswordFieldDemoApp>(), config);
}
