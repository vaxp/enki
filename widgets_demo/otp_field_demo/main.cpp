#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/otp_field.hpp"
#include "enki/state/state.hpp"
#include <iostream>

using namespace enki;

class OTPFieldDemoState : public State {
    std::string current_otp_ = "";
    std::string completed_otp_ = "None";

public:
    WidgetPtr build(BuildContext&) override {
        auto title = text("OTPField Interactive Demo", {
            .color = 0xFFFFFFFF,
            .font_size = 22.0f,
            .font_weight = FontWeight::Bold
        });
        auto subtitle = text("Segmented 6-Digit 2FA Verification Field (Section 15)", {
            .color = 0xFF00E5FF,
            .font_size = 13.0f,
            .font_weight = FontWeight::Medium
        });

        auto status = text("Live Code: " + (current_otp_.empty() ? "[Waiting for input]" : current_otp_), {
            .color = 0xFF94A3B8,
            .font_size = 14.0f,
        });

        auto verified = text("Verified OTP: " + completed_otp_, {
            .color = 0xFF10B981,
            .font_size = 16.0f,
            .font_weight = FontWeight::Bold
        });

        auto otp = otpField({
            .length = 6,
            .box_size = 54.0f,
            .gap = 12.0f,
            .auto_focus = true,
            .on_changed = [this](const std::string& code) {
                current_otp_ = code;
                setState([]{});
            },
            .on_completed = [this](const std::string& code) {
                completed_otp_ = code;
                std::cout << ">>> Standalone OTP Completed: " << code << std::endl;
                setState([]{});
            },
        });

        auto main_col = column(FlexboxProps{
            .align_items = Align::Center,
            .gap = StyleValue::point(28.0f),
            .children = {title, subtitle, otp, status, verified}
        });

        return container(ContainerProps{
            .color = 0xFF0B1320,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(40.0f),
            .child = main_col
        });
    }
};

class OTPFieldDemoApp : public StatefulWidget {
public:
    std::string_view typeName() const override { return "OTPFieldDemoApp"; }
    std::unique_ptr<State> createState() override { return std::make_unique<OTPFieldDemoState>(); }
};

int main() {
    std::cout << "=== ENKI OTPField Standalone Demo ===\n";
    AppConfig config;
    config.title = "ENKI — OTPField Demo";
    config.width = 720;
    config.height = 420;
    config.resizable = true;
    config.vsync = false;
    config.target_fps = 0;
    config.show_performance_overlay = true;
    config.clear_color = 0xFF0B1320;

    return runApp(std::make_shared<OTPFieldDemoApp>(), config);
}
