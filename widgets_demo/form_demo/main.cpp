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
        auto title_col = column({
            .align_items = Align::Center,
            .gap = StyleValue::point(6.0f),
            .children = {
                text("Advanced Form & FormField Validation Suite", { .color = 0xFFFFFFFF, .font_size = 22.0f, .font_weight = FontWeight::Bold }),
                text("Enterprise state & validation engine (Category 3. Input / Forms), FormState, TextFormField, CheckboxFormField, and Composable Validators", { .color = 0xFF94A3B8, .font_size = 13.0f })
            }
        });

        // ── Left Card: Registration Form ──────────────────────────────
        auto f_title = text("📝 Enterprise User Registration", { .color = 0xFF38BDF8, .font_size = 15.5f, .font_weight = FontWeight::Bold });

        // 1. Full Name Field
        auto name_field = TextFormField {
            .label = "Full Name",
            .hint = "e.g. Sarah Connor",
            .required = true,
            .width = 380.0f,
            .form_state = form_state_,
            .controller = name_ctrl_,
            .validator = Validators::compose({
                Validators::required("Please enter your full name"),
                Validators::minLength(3, "Full name must be at least 3 characters")
            })
        };

        // 2. Email Field
        auto email_field = TextFormField {
            .label = "Work Email",
            .hint = "e.g. sarah.connor@enki.dev",
            .required = true,
            .width = 380.0f,
            .form_state = form_state_,
            .controller = email_ctrl_,
            .validator = Validators::compose({
                Validators::required("Work email is required"),
                Validators::email("Please enter a valid email format (name@domain.com)")
            })
        };

        // 3. Password Field
        auto pass_field = TextFormField {
            .label = "Password",
            .hint = "At least 8 characters",
            .required = true,
            .obscure_text = true,
            .width = 380.0f,
            .form_state = form_state_,
            .controller = pass_ctrl_,
            .validator = Validators::compose({
                Validators::required("Password cannot be empty"),
                Validators::minLength(8, "Password must contain at least 8 characters")
            })
        };

        // 4. Confirm Password Field
        auto pass_ctrl_ref = pass_ctrl_;
        auto confirm_field = TextFormField {
            .label = "Confirm Password",
            .hint = "Re-enter password",
            .required = true,
            .obscure_text = true,
            .width = 380.0f,
            .form_state = form_state_,
            .controller = confirm_pass_ctrl_,
            .validator = Validators::compose({
                Validators::required("Please confirm your password"),
                Validators::match([pass_ctrl_ref]() { return pass_ctrl_ref->text; }, "Passwords do not match")
            })
        };

        // 5. Terms Checkbox Field
        auto terms_field = CheckboxFormField {
            .label = "I agree to the ENKI Terms of Service & Privacy Policy",
            .required = true,
            .form_state = form_state_,
            .validator = [](bool val) -> std::optional<std::string> {
                if (!val) return "You must agree to the Terms of Service to proceed";
                return std::nullopt;
            }
        };

        // 6. Action Buttons (Submit & Reset)
        auto sub_btn_box = container({
            .color = 0xFF0284C7,
            .border_radius = BorderRadius::circular(8.0f),
            .padding = StyleInsets::symmetric(8.0f, 18.0f),
            .child = text("🚀 Submit & Create Account", { .color = 0xFFFFFFFF, .font_size = 12.5f, .font_weight = FontWeight::Bold })
        });

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

        auto rst_btn_box = container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(8.0f),
            .border = Border(0xFF475569, 1.0f),
            .padding = StyleInsets::symmetric(8.0f, 16.0f),
            .child = text("🔄 Reset Form", { .color = 0xFFCBD5E1, .font_size = 12.5f, .font_weight = FontWeight::Bold })
        });

        auto rst_gd = std::make_shared<GestureDetector>(rst_btn_box);
        rst_gd->cursor_type = SystemCursor::Pointer;
        rst_gd->on_tap_up = [this](const TapUpDetails&) {
            form_state_->reset();
            submitted_json_ = "{\n  \"status\": \"Form Reset\"\n}";
            hud_msg_ = "Form fields and validation errors reset.";
            setState([] {});
        };

        auto btn_row = row({
            .align_items = Align::Center,
            .gap = StyleValue::point(10.0f),
            .children = {sub_gd, rst_gd}
        });

        auto form_widget = Form {
            .child = column({
                .gap = StyleValue::point(14.0f),
                .children = {
                    f_title, name_field, email_field, pass_field, confirm_field, terms_field, btn_row
                }
            })
        };

        auto form_card = container({
            .color = 0xFF0F172A,
            .border_radius = BorderRadius::circular(12.0f),
            .border = Border(0xFF334155, 1.0f),
            .width = StyleValue::point(440.0f),
            .padding = StyleInsets::all(24.0f),
            .child = form_widget
        });

        // ── Right Card: Live JSON & Submission Inspector ──────────────
        auto ins_title = text("📡 Live Form Payload & Submission Inspector", { .color = 0xFF10B981, .font_size = 15.5f, .font_weight = FontWeight::Bold });

        auto st_box = container({
            .color = form_valid_ ? 0x2210B981 : 0x22F59E0B,
            .border_radius = BorderRadius::circular(4.0f),
            .padding = StyleInsets::symmetric(2.0f, 8.0f),
            .child = text(form_valid_ ? "● VALIDATED" : "○ PENDING", {
                .color = form_valid_ ? 0xFF10B981 : 0xFFF59E0B,
                .font_size = 11.0f,
                .font_weight = FontWeight::Bold
            })
        });

        auto ins_head_row = row({
            .justify_content = Justify::SpaceBetween,
            .align_items = Align::Center,
            .width = StyleValue::percent(100.0f),
            .children = {ins_title, st_box}
        });

        auto json_box = container({
            .color = 0xFF020617,
            .border_radius = BorderRadius::circular(8.0f),
            .border = Border(0xFF1E293B, 1.0f),
            .width = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(14.0f),
            .child = text(submitted_json_, { .color = 0xFF38BDF8, .font_size = 12.0f })
        });

        auto rules_col = column({
            .gap = StyleValue::point(6.0f),
            .children = {
                text("⚙️ Applied Validation Rules", { .color = 0xFFE2E8F0, .font_size = 13.0f, .font_weight = FontWeight::Bold }),
                text("• Full Name: Required, Minimum 3 chars", { .color = 0xFF94A3B8, .font_size = 11.5f }),
                text("• Work Email: Required, Valid RFC Email regex", { .color = 0xFF94A3B8, .font_size = 11.5f }),
                text("• Password: Required, Minimum 8 chars", { .color = 0xFF94A3B8, .font_size = 11.5f }),
                text("• Confirm Password: Must match Password exactly", { .color = 0xFF94A3B8, .font_size = 11.5f }),
                text("• Terms Agreement: Mandatory boolean checkbox", { .color = 0xFF94A3B8, .font_size = 11.5f })
            }
        });

        auto ins_card = container({
            .color = 0xFF0F172A,
            .border_radius = BorderRadius::circular(12.0f),
            .border = Border(0xFF334155, 1.0f),
            .width = StyleValue::point(460.0f),
            .padding = StyleInsets::all(24.0f),
            .child = column({
                .gap = StyleValue::point(16.0f),
                .children = {ins_head_row, json_box, rules_col}
            })
        });

        // ── Side-by-Side Main Sections ────────────────────────────────
        auto sections_row = row({
            .justify_content = Justify::Center,
            .align_items = Align::Start,
            .gap = StyleValue::point(24.0f),
            .children = {form_card, ins_card}
        });

        // ── HUD / Status Box ──────────────────────────────────────────
        auto hud_box = container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(6.0f),
            .border = Border(0xFF334155, 1.0f),
            .width = StyleValue::point(924.0f),
            .padding = StyleInsets::symmetric(8.0f, 16.0f),
            .child = row({
                .children = {
                    text("💡 " + hud_msg_, { .color = 0xFF38BDF8, .font_size = 12.5f })
                }
            })
        });

        return container({
            .color = 0xFF0B1120,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(24.0f),
            .child = column({
                .align_items = Align::Center,
                .gap = StyleValue::point(18.0f),
                .children = {title_col, sections_row, hud_box}
            })
        });
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
