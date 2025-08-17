/// @file main.cpp
/// @brief ENKI Advanced Form & FormField Interactive Showcase Demo.

#include "enki/app/app.hpp"
#include "enki/widgets/form.hpp"
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

class FormDemoState : public State {
private:
    std::shared_ptr<FormState> form_state_ = std::make_shared<FormState>();

    // Form Field Controllers
    std::shared_ptr<TextFieldController> name_ctrl_ = std::make_shared<TextFieldController>();
    std::shared_ptr<TextFieldController> email_ctrl_ = std::make_shared<TextFieldController>();
    std::shared_ptr<TextFieldController> pass_ctrl_ = std::make_shared<TextFieldController>();
    std::shared_ptr<TextFieldController> confirm_pass_ctrl_ = std::make_shared<TextFieldController>();

    // Saved Form State
    std::string submitted_json_ = "{\n  \"status\": \"Awaiting Submission\"\n}";
    std::string hud_msg_ = "Fill out the registration form and click 'Submit & Validate'. Try entering invalid values to test validation rules.";
    bool form_valid_ = false;

public:
    WidgetPtr build(BuildContext&) override {
        // ── Main Page Header ──────────────────────────────────────────
        auto title = text("Advanced Form & FormField Validation Suite");
        title->fontSize(22.0f).bold().color(0xFFFFFFFF);

        auto sub = text("Enterprise state & validation engine (Category 3. Input / Forms), FormState, TextFormField, CheckboxFormField, and Composable Validators");
        sub->fontSize(13.0f).color(0xFF94A3B8);

        std::vector<WidgetPtr> title_items = {title, sub};
        auto title_col = column(title_items);
        title_col->alignItems(Align::Center).gap(StyleValue::point(6.0f));

        // ── Left Card: Registration Form ──────────────────────────────
        auto f_title = text("📝 Enterprise User Registration");
        f_title->fontSize(15.5f).bold().color(0xFF38BDF8);

        // 1. Full Name Field
        TextFormFieldProps name_opts;
        name_opts.label = "Full Name";
        name_opts.hint = "e.g. Sarah Connor";
        name_opts.required = true;
        name_opts.width = 380.0f;
        name_opts.controller = name_ctrl_;
        name_opts.form_state = form_state_;
        name_opts.validator = Validators::compose({
            Validators::required("Please enter your full name"),
            Validators::minLength(3, "Full name must be at least 3 characters")
        });
        auto name_field = textFormField(name_opts);

        // 2. Email Field
        TextFormFieldProps email_opts;
        email_opts.label = "Work Email";
        email_opts.hint = "e.g. sarah.connor@enki.dev";
        email_opts.required = true;
        email_opts.width = 380.0f;
        email_opts.controller = email_ctrl_;
        email_opts.form_state = form_state_;
        email_opts.validator = Validators::compose({
            Validators::required("Work email is required"),
            Validators::email("Please enter a valid email format (name@domain.com)")
        });
        auto email_field = textFormField(email_opts);

        // 3. Password Field
        TextFormFieldProps pass_opts;
        pass_opts.label = "Password";
        pass_opts.hint = "At least 8 characters";
        pass_opts.obscure_text = true;
        pass_opts.required = true;
        pass_opts.width = 380.0f;
        pass_opts.controller = pass_ctrl_;
        pass_opts.form_state = form_state_;
        pass_opts.validator = Validators::compose({
            Validators::required("Password cannot be empty"),
            Validators::minLength(8, "Password must contain at least 8 characters")
        });
        auto pass_field = textFormField(pass_opts);

        // 4. Confirm Password Field
        auto pass_ctrl_ref = pass_ctrl_;
        TextFormFieldProps confirm_opts;
        confirm_opts.label = "Confirm Password";
        confirm_opts.hint = "Re-enter password";
        confirm_opts.obscure_text = true;
        confirm_opts.required = true;
        confirm_opts.width = 380.0f;
        confirm_opts.controller = confirm_pass_ctrl_;
        confirm_opts.form_state = form_state_;
        confirm_opts.validator = Validators::compose({
            Validators::required("Please confirm your password"),
            Validators::match([pass_ctrl_ref]() { return pass_ctrl_ref->text; }, "Passwords do not match")
        });
        auto confirm_field = textFormField(confirm_opts);

        // 5. Terms Checkbox Field
        CheckboxFormFieldProps terms_opts;
        terms_opts.label = "I agree to the ENKI Terms of Service & Privacy Policy";
        terms_opts.required = true;
        terms_opts.form_state = form_state_;
        terms_opts.validator = [](bool val) -> std::optional<std::string> {
            if (!val) return "You must agree to the Terms of Service to proceed";
            return std::nullopt;
        };
        auto terms_field = checkboxFormField(terms_opts);

        // 6. Action Buttons (Submit & Reset)
        auto sub_btn_txt = text("🚀 Submit & Create Account");
        sub_btn_txt->fontSize(12.5f).bold().color(0xFFFFFFFF);
        auto sub_btn_box = container(sub_btn_txt);
        sub_btn_box->color(0xFF0284C7).borderRadius(8.0f).paddingSymmetric(8.0f, 18.0f);

        auto sub_gd = std::make_shared<GestureDetector>(sub_btn_box);
        sub_gd->cursor_type = SystemCursor::Pointer;
        sub_gd->on_tap_up = [this](const TapUpDetails&) {
            bool valid = form_state_->validate();
            form_valid_ = valid;
            if (valid) {
                submitted_json_ = "{\n"
                                  "  \"status\": \"SUCCESS (200 OK)\",\n"
                                  "  \"user\": {\n"
                                  "    \"name\": \"" + name_ctrl_->text + "\",\n"
                                  "    \"email\": \"" + email_ctrl_->text + "\",\n"
                                  "    \"terms_accepted\": true\n"
                                  "  }\n"
                                  "}";
                hud_msg_ = "🎉 Form validated successfully! Account payload dispatched.";
            } else {
                submitted_json_ = "{\n  \"status\": \"VALIDATION_ERROR\",\n  \"error\": \"One or more fields failed validation\"\n}";
                hud_msg_ = "⚠️ Validation failed! Please correct the highlighted errors in the form.";
            }
            setState([] {});
        };

        auto rst_btn_txt = text("🔄 Reset Form");
        rst_btn_txt->fontSize(12.5f).bold().color(0xFFCBD5E1);
        auto rst_btn_box = container(rst_btn_txt);
        rst_btn_box->color(0xFF1E293B).border(0xFF475569, 1.0f).borderRadius(8.0f).paddingSymmetric(8.0f, 16.0f);

        auto rst_gd = std::make_shared<GestureDetector>(rst_btn_box);
        rst_gd->cursor_type = SystemCursor::Pointer;
        rst_gd->on_tap_up = [this](const TapUpDetails&) {
            form_state_->reset();
            submitted_json_ = "{\n  \"status\": \"Form Reset\"\n}";
            hud_msg_ = "Form fields and validation errors reset.";
            setState([] {});
        };

        std::vector<WidgetPtr> btn_items = {sub_gd, rst_gd};
        auto btn_row = row(btn_items);
        btn_row->gap(StyleValue::point(10.0f)).alignItems(Align::Center);

        std::vector<WidgetPtr> form_elements = {
            f_title, name_field, email_field, pass_field, confirm_field, terms_field, btn_row
        };
        auto form_col = column(form_elements);
        form_col->gap(StyleValue::point(14.0f));

        auto form_widget = form(form_col);

        auto form_card = container(form_widget);
        form_card->color(0xFF0F172A)
                 .border(0xFF334155, 1.0f)
                 .borderRadius(12.0f)
                 .paddingAll(24.0f)
                 .width(440.0f);

        // ── Right Card: Live JSON & Submission Inspector ──────────────
        auto ins_title = text("📡 Live Form Payload & Submission Inspector");
        ins_title->fontSize(15.5f).bold().color(0xFF10B981);

        auto st_pill = text(form_valid_ ? "● VALIDATED" : "○ PENDING");
        st_pill->fontSize(11.0f).bold().color(form_valid_ ? 0xFF10B981 : 0xFFF59E0B);
        auto st_box = container(st_pill);
        st_box->color(form_valid_ ? 0x2210B981 : 0x22F59E0B).borderRadius(4.0f).paddingSymmetric(2.0f, 8.0f);

        std::vector<WidgetPtr> ins_head_items = {ins_title, st_box};
        auto ins_head_row = row(ins_head_items);
        ins_head_row->justifyContent(Justify::SpaceBetween).alignItems(Align::Center).width(StyleValue::percent(100.0f));

        auto json_txt = text(submitted_json_);
        json_txt->fontSize(12.0f).color(0xFF38BDF8);

        auto json_box = container(json_txt);
        json_box->color(0xFF020617)
                .border(0xFF1E293B, 1.0f)
                .borderRadius(8.0f)
                .paddingAll(14.0f)
                .width(StyleValue::percent(100.0f));

        auto rule_title = text("⚙️ Applied Validation Rules");
        rule_title->fontSize(13.0f).bold().color(0xFFE2E8F0);

        auto r1 = text("• Full Name: Required, Minimum 3 chars");
        r1->fontSize(11.5f).color(0xFF94A3B8);
        auto r2 = text("• Work Email: Required, Valid RFC Email regex");
        r2->fontSize(11.5f).color(0xFF94A3B8);
        auto r3 = text("• Password: Required, Minimum 8 chars");
        r3->fontSize(11.5f).color(0xFF94A3B8);
        auto r4 = text("• Confirm Password: Must match Password exactly");
        r4->fontSize(11.5f).color(0xFF94A3B8);
        auto r5 = text("• Terms Agreement: Mandatory boolean checkbox");
        r5->fontSize(11.5f).color(0xFF94A3B8);

        std::vector<WidgetPtr> rules_items = {rule_title, r1, r2, r3, r4, r5};
        auto rules_col = column(rules_items);
        rules_col->gap(StyleValue::point(6.0f));

        std::vector<WidgetPtr> ins_items = {ins_head_row, json_box, rules_col};
        auto ins_col = column(ins_items);
        ins_col->gap(StyleValue::point(16.0f));

        auto ins_card = container(ins_col);
        ins_card->color(0xFF0F172A)
                .border(0xFF334155, 1.0f)
                .borderRadius(12.0f)
                .paddingAll(24.0f)
                .width(460.0f);

        // ── Side-by-Side Main Sections ────────────────────────────────
        std::vector<WidgetPtr> sections = {form_card, ins_card};
        auto sections_row = row(sections);
        sections_row->gap(StyleValue::point(24.0f))
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
               .width(924.0f);

        // ── Assemble Page Body ────────────────────────────────────────
        std::vector<WidgetPtr> page_items = {title_col, sections_row, hud_box};
        auto page_col = column(page_items);
        page_col->gap(StyleValue::point(18.0f)).alignItems(Align::Center);

        auto background_page = container(page_col);
        background_page->color(0xFF0B1120)
                       .paddingAll(24.0f)
                       .width(StyleValue::percent(100.0f))
                       .height(StyleValue::percent(100.0f));

        return background_page;
    }
};

class FormDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<FormDemoState>();
    }
    std::string_view typeName() const override { return "FormDemoApp"; }
};

int main() {
    std::cout << "====================================================\n";
    std::cout << "  ENKI Engine — Advanced Form & Validation Demo\n";
    std::cout << "====================================================\n";

    AppConfig config;
    config.title       = "Enki Engine — Advanced Form Demo";
    config.width       = 1240;
    config.height      = 740;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 60;
    config.clear_color = 0xFF0B1120;

    return runApp(std::make_shared<FormDemoApp>(), config);
}
