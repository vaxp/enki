#include "enki/enki.hpp"
#include <iostream>

using namespace enki;

class FormDemoApp : public StatelessWidget {
public:
    WidgetPtr build(BuildContext&) override {
        auto form_state = std::make_shared<FormState>();
        auto name_ctrl  = std::make_shared<TextFieldController>();
        auto email_ctrl = std::make_shared<TextFieldController>();

        return container({
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .color = 0xFF0B0F19,
            .child = center(
                container({
                    .width = StyleValue::point(460.0f),
                    .padding = StyleInsets::all(32.0f),
                    .decoration = BoxDecoration{
                        .color = 0xFF151C2C,
                        .border_radius = BorderRadius::circular(16.0f),
                        .border = Border::all(0xFF26334D, 1.0f),
                    },
                    .child = column({
                        .gap = StyleValue::point(20.0f),
                        .children = {
                            text("Create Account", {
                                .color = 0xFFFFFFFF,
                                .font_size = 24.0f,
                                .font_weight = FontWeight::Bold,
                            }),
                            text("Fill in the fields below to register with ENKI Form validation.", {
                                .color = 0xFF94A3B8,
                                .font_size = 14.0f,
                            }),

                            // Full Name Input
                            text_form_field({
                                .label = "Full Name",
                                .hint = "Enter your display name",
                                .controller = name_ctrl,
                                .form_state = form_state,
                                .validator = Validators::compose({
                                    Validators::required("Full name is required"),
                                    Validators::minLength(3, "Must be at least 3 characters"),
                                }),
                            }),

                            // Email Input
                            text_form_field({
                                .label = "Email Address",
                                .hint = "user@example.com",
                                .controller = email_ctrl,
                                .form_state = form_state,
                                .validator = Validators::compose({
                                    Validators::required("Email is required"),
                                    Validators::email("Please enter a valid email address"),
                                }),
                            }),

                            // Submit Button
                            button({
                                .label = "Submit Form",
                                .on_click = [form_state, name_ctrl, email_ctrl]() {
                                    if (form_state->validate()) {
                                        std::cout << "Form is valid!\n";
                                        std::cout << "Name: " << name_ctrl->text() << "\n";
                                        std::cout << "Email: " << email_ctrl->text() << "\n";
                                    } else {
                                        std::cout << "Validation failed. Please review the errors.\n";
                                    }
                                },
                            }),
                        }
                    })
                })
            )
        });
    }

    std::string_view typeName() const override { return "FormDemoApp"; }
};

int main() {
    AppConfig config;
    config.title = "ENKI Form & Validation";
    config.width = 900;
    config.height = 700;
    return runApp(std::make_shared<FormDemoApp>(), config);
}
