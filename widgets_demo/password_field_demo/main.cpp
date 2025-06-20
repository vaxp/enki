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
        auto title = text("Advanced PasswordField Widget Suite");
        title->fontSize(22.0f).bold().color(0xFFFFFFFF);

        auto sub = text("Live strength evaluator, criteria checklist, CapsLock detection, eye toggle/peek, and secure generator");
        sub->fontSize(13.0f).color(0xFF94A3B8);

        std::vector<WidgetPtr> title_items = {title, sub};
        auto title_col = column(title_items);
        title_col->alignItems(Align::Center);

        // ── Card 1: User Registration / Sign-Up (Full Suite) ─────────
        PasswordFieldOptions signup_opts;
        signup_opts.placeholder = "Create a strong password...";
        signup_opts.show_strength_meter = true;
        signup_opts.show_rules_checklist = true;
        signup_opts.show_generator_button = true;
        signup_opts.show_visibility_toggle = true;
        signup_opts.show_clear_button = true;
        signup_opts.on_changed = [this](std::string_view) { setState([] {}); };

        auto signup_field = passwordField(signup_ctrl_, signup_opts);

        auto c1_title = text("1. User Registration (Strength Meter & Criteria)");
        c1_title->fontSize(14.0f).bold().color(0xFF38BDF8);

        auto c1_desc = text("Includes live 4-segment strength meter, 5-point criteria checklist, and [🎲] generator.");
        c1_desc->fontSize(12.0f).color(0xFF94A3B8);

        std::vector<WidgetPtr> c1_items = {c1_title, c1_desc, signup_field};
        auto c1_col = column(c1_items);
        c1_col->gap(StyleValue::point(8.0f));

        auto card1 = container(c1_col);
        card1->color(0xFF1E293B)
             .borderRadius(10.0f)
             .border(0xFF334155, 1.0f)
             .paddingAll(16.0f)
             .width(540.0f);

        // ── Card 2: User Login Password ──────────────────────────────
        PasswordFieldOptions login_opts;
        login_opts.placeholder = "Enter account password...";
        login_opts.show_visibility_toggle = true;
        login_opts.show_capslock_warning = true;
        login_opts.focus_border_color = 0xFF10B981;
        login_opts.on_changed = [this](std::string_view) { setState([] {}); };

        auto login_field = passwordField(login_ctrl_, login_opts);

        auto c2_title = text("2. User Login (Eye Toggle & CapsLock Warning)");
        c2_title->fontSize(14.0f).bold().color(0xFF10B981);

        auto c2_desc = text("Clean input with toggle button [👁] and automatic [⇪ CAPS] warning detection.");
        c2_desc->fontSize(12.0f).color(0xFF94A3B8);

        std::vector<WidgetPtr> c2_items = {c2_title, c2_desc, login_field};
        auto c2_col = column(c2_items);
        c2_col->gap(StyleValue::point(8.0f));

        auto card2 = container(c2_col);
        card2->color(0xFF1E293B)
             .borderRadius(10.0f)
             .border(0xFF334155, 1.0f)
             .paddingAll(16.0f)
             .width(540.0f);

        // ── Card 3: Hold-to-Peek PIN / Password ───────────────────────
        PasswordFieldOptions peek_opts;
        peek_opts.placeholder = "Enter 6-digit PIN...";
        peek_opts.show_visibility_toggle = true;
        peek_opts.hold_to_peek = true;
        peek_opts.focus_border_color = 0xFFF59E0B;
        peek_opts.on_changed = [this](std::string_view) { setState([] {}); };

        auto peek_field = passwordField(peek_ctrl_, peek_opts);

        auto c3_title = text("3. Hold-to-Peek Mode (Press & Hold Eye Button)");
        c3_title->fontSize(14.0f).bold().color(0xFFF59E0B);

        auto c3_desc = text("Press and hold the eye button [👁] to reveal temporarily, re-obscures on release.");
        c3_desc->fontSize(12.0f).color(0xFF94A3B8);

        std::vector<WidgetPtr> c3_items = {c3_title, c3_desc, peek_field};
        auto c3_col = column(c3_items);
        c3_col->gap(StyleValue::point(8.0f));

        auto card3 = container(c3_col);
        card3->color(0xFF1E293B)
             .borderRadius(10.0f)
             .border(0xFF334155, 1.0f)
             .paddingAll(16.0f)
             .width(540.0f);

        // ── Security Inspector Panel ──────────────────────────────────
        auto status_hdr = text("🔒 Live Security & Entropy Inspector");
        status_hdr->fontSize(13.5f).bold().color(0xFFFFFFFF);

        std::ostringstream ss_ent;
        ss_ent << "Sign-Up Entropy: " << std::fixed << std::setprecision(1) << signup_ctrl_->calculateEntropy() << " bits";
        auto st_ent = text(ss_ent.str());
        st_ent->fontSize(12.0f).color(0xFF38BDF8);

        std::string st_str = "Sign-Up Rules Passed: " + std::string(signup_ctrl_->meetsAllRules() ? "YES (All Satisfied ✓)" : "NO (Criteria Missing)");
        auto st_rules = text(st_str);
        st_rules->fontSize(12.0f).color(signup_ctrl_->meetsAllRules() ? 0xFF10B981 : 0xFFF59E0B);

        auto st_len = text("Sign-Up Password Length: " + std::to_string(signup_ctrl_->getPassword().length()) + " characters");
        st_len->fontSize(12.0f).color(0xFF94A3B8);

        std::vector<WidgetPtr> status_items = {status_hdr, st_ent, st_rules, st_len};
        auto status_col = column(status_items);
        status_col->gap(StyleValue::point(4.0f));

        auto status_box = container(status_col);
        status_box->color(0xFF0F172A)
                  .borderRadius(8.0f)
                  .border(0xFF334155, 1.0f)
                  .paddingAll(12.0f)
                  .width(540.0f);

        // Top Row: Card1 & Card2
        std::vector<WidgetPtr> row1_items = {card1, card2};
        auto row1 = row(row1_items);
        row1->gap(StyleValue::point(16.0f))
             .justifyContent(Justify::Center);

        // Bottom Row: Card3 & Status Box
        std::vector<WidgetPtr> row2_items = {card3, status_box};
        auto row2 = row(row2_items);
        row2->gap(StyleValue::point(16.0f))
             .justifyContent(Justify::Center);

        // Main Column
        std::vector<WidgetPtr> main_items = {title_col, row1, row2};
        auto main_col = column(main_items);
        main_col->gap(StyleValue::point(16.0f))
                .alignItems(Align::Center);

        auto app_root = container(main_col);
        app_root->color(0xFF0B1120)
                .paddingAll(20.0f)
                .flexGrow(1.0f);

        return app_root;
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
