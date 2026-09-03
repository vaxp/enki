#include <iostream>
#include <vector>
#include <string>

#include "enki/app/app.hpp"
#include "enki/state/state.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/window_frame.hpp"
#include "enki/widgets/card.hpp"

// Section 15: Rich Input Controls
#include "enki/widgets/toggle_button.hpp"
#include "enki/widgets/segmented_control.hpp"
#include "enki/widgets/rating_bar.hpp"
#include "enki/widgets/knob.hpp"
#include "enki/widgets/otp_field.hpp"
#include "enki/widgets/pin_field.hpp"
#include "enki/widgets/tag_input.hpp"
#include "enki/widgets/file_drop_zone.hpp"

using namespace enki;

class RichInputsDemoState : public State {
public:
    // State values
    float knob_volume_ = 75.0f;
    float knob_pan_ = 0.0f;
    int selected_wave_ = 0;
    bool boost_toggled_ = true;
    bool filter_toggled_ = false;

    std::string otp_code_ = "";
    std::string pin_code_ = "";
    float current_rating_ = 4.5f;
    std::vector<std::string> tags_ = {"C++20", "Skia", "VAXP-OS", "ZeroCopy"};
    std::vector<std::string> dropped_files_ = {"sample_video.mp4", "synth_track.wav"};

    WidgetPtr buildCard(const std::string& title, const std::string& subtitle, const std::vector<WidgetPtr>& content) {
        auto header = column({
            .gap = StyleValue::point(3.0f),
            .children = {
                text(title, {
                    .color = 0xFF38BDF8,
                    .font_size = 15.0f,
                    .font_weight = FontWeight::Bold,
                }),
                text(subtitle, {
                    .color = 0xFF64748B,
                    .font_size = 11.5f,
                }),
            },
        });

        auto body = column({
            .align_items = Align::Center,
            .gap = StyleValue::point(14.0f),
            .children = content,
        });

        return container({
            .color = 0x660B1320,
            .border_radius = BorderRadius::circular(14.0f),
            .border = Border(0x3300E5FF, 1.0f),
            .width = StyleValue::point(440.0f),
            .padding = StyleInsets::all(16.0f),
            .child = column({
                .gap = StyleValue::point(14.0f),
                .children = {header, body},
            }),
        });
    }

    WidgetPtr build(BuildContext&) override {
        // ── Header Title ──────────────────────────────────────────────
        auto title_bar = column({
            .align_items = Align::Center,
            .gap = StyleValue::point(4.0f),
            .children = {
                text("ENKI Studio — Rich Input Controls Suite", {
                    .color = 0xFFFFFFFF,
                    .font_size = 20.0f,
                    .font_weight = FontWeight::Bold,
                }),
                text("Roadmap v0.2.0 • Section 15 • 8 Atomic Native Controls for VAXP-OS", {
                    .color = 0xFF00E5FF,
                    .font_size = 12.0f,
                    .font_weight = FontWeight::Medium,
                }),
            },
        });

        // ── Card 1: Studio Audio & DSP Controls (Knob, SegmentedControl, ToggleButton) ──
        auto wave_selector = segmentedControl({
            .items = {
                SegmentItem("Sine", "∿"),
                SegmentItem("Square", "⊓"),
                SegmentItem("Saw", "⋀"),
                SegmentItem("Triangle", "⋏"),
            },
            .selected_index = selected_wave_,
            .on_change = [this](int idx) {
                setState([this, idx]() { selected_wave_ = idx; });
            },
            .height = 36.0f,
            .width = 380.0f,
        });

        auto knobs_row = row({
            .justify_content = Justify::Center,
            .align_items = Align::Center,
            .gap = StyleValue::point(28.0f),
            .children = {
                knob({
                    .value = knob_volume_,
                    .min_value = 0.0f,
                    .max_value = 100.0f,
                    .step = 1.0f,
                    .size = 76.0f,
                    .label = "MASTER VOL",
                    .unit = "%",
                    .active_color = 0xFF00E5FF,
                    .on_value_changed = [this](float v) {
                        knob_volume_ = v;
                    },
                }),
                knob({
                    .value = knob_pan_,
                    .min_value = -50.0f,
                    .max_value = 50.0f,
                    .step = 1.0f,
                    .size = 76.0f,
                    .is_bipolar = true,
                    .label = "STEREO PAN",
                    .unit = "",
                    .active_color = 0xFFF59E0B,
                    .on_value_changed = [this](float v) {
                        knob_pan_ = v;
                    },
                }),
            },
        });

        auto toggle_row = row({
            .justify_content = Justify::Center,
            .align_items = Align::Center,
            .gap = StyleValue::point(12.0f),
            .children = {
                toggleButton({
                    .is_toggled = boost_toggled_,
                    .on_toggle = [this](bool val) {
                        setState([this, val]() { boost_toggled_ = val; });
                    },
                    .label = "Bass Boost",
                    .icon = "⚡",
                    .style = ToggleButtonStyle::Glow,
                    .active_color = 0xFF00E5FF,
                }),
                toggleButton({
                    .is_toggled = filter_toggled_,
                    .on_toggle = [this](bool val) {
                        setState([this, val]() { filter_toggled_ = val; });
                    },
                    .label = "Hi-Pass Filter",
                    .icon = "🎛",
                    .style = ToggleButtonStyle::Outlined,
                    .active_color = 0xFF38BDF8,
                }),
            },
        });

        auto card1 = buildCard("🎚 Studio Audio & Waveform Controls", "Knob (Rotary Dial), SegmentedControl, and ToggleButton", {
            wave_selector,
            knobs_row,
            toggle_row,
        });

        // ── Card 2: Security & Authentication (OTPField, PinField) ────
        auto otp_box = otpField({
            .length = 6,
            .box_size = 46.0f,
            .gap = 8.0f,
            .initial_value = otp_code_,
            .on_changed = [this](const std::string& code) {
                otp_code_ = code;
            },
            .on_completed = [this](const std::string& code) {
                std::cout << ">>> 2FA OTP Completed: " << code << std::endl;
                setState([this, code]() { otp_code_ = code; });
            },
        });

        auto otp_status = text(
            otp_code_.length() == 6 ? ("✅ 2FA Verified: " + otp_code_) : ("Status: Enter 6-digit 2FA code (" + std::to_string(otp_code_.length()) + "/6)"),
            {.color = otp_code_.length() == 6 ? 0xFF34D399 : 0xFF94A3B8, .font_size = 12.0f}
        );

        auto pin_box = pinField({
            .length = 4,
            .box_size = 44.0f,
            .gap = 12.0f,
            .obscure_delay_ms = 450,
            .initial_value = pin_code_,
            .on_changed = [this](const std::string& pin) {
                pin_code_ = pin;
            },
            .on_completed = [this](const std::string& pin) {
                std::cout << ">>> Master PIN Entered: " << pin << std::endl;
                setState([this, pin]() { pin_code_ = pin; });
            },
        });

        auto pin_status = text(
            pin_code_.length() == 4 ? "🔒 Master PIN Secured" : "Enter 4-digit Vault PIN",
            {.color = pin_code_.length() == 4 ? 0xFF38BDF8 : 0xFF94A3B8, .font_size = 12.0f}
        );

        auto card2 = buildCard("🔐 Security & Credentials Input", "Segmented OTPField (Auto-Advance) & Masked PinField", {
            otp_box,
            otp_status,
            pin_box,
            pin_status,
        });

        // ── Card 3: Metadata & Rating (RatingBar, TagInput) ───────────
        char rating_buf[32];
        std::snprintf(rating_buf, sizeof(rating_buf), "Rating: %.1f / 5.0 Stars", current_rating_);

        auto rating_widget = ratingBar({
            .rating = current_rating_,
            .max_rating = 5,
            .item_size = 28.0f,
            .item_spacing = 8.0f,
            .allow_half = true,
            .active_color = 0xFFF59E0B,
            .on_rating_changed = [this](float r) {
                setState([this, r]() { current_rating_ = r; });
            },
        });

        auto rating_label = text(rating_buf, {
            .color = 0xFFF59E0B,
            .font_size = 12.5f,
            .font_weight = FontWeight::Bold,
        });

        auto tag_widget = tagInput({
            .tags = tags_,
            .placeholder = "Type tag + press Enter...",
            .on_tags_changed = [this](const std::vector<std::string>& t) {
                setState([this, t]() { tags_ = t; });
            },
        });

        auto card3 = buildCard("⭐ Rating & Project Tags", "Fractional RatingBar (Half-Star) and Dynamic TagInput", {
            rating_widget,
            rating_label,
            tag_widget,
        });

        // ── Card 4: OS File Ingestion (FileDropZone) ───────────────────
        auto drop_zone = fileDropZone({
            .allowed_extensions = {".mp4", ".mkv", ".wav", ".png"},
            .prompt_text = "Drag & Drop Files Here",
            .sub_text = "MP4, MKV, WAV, PNG supported",
            .width = 400.0f,
            .height = 130.0f,
            .on_files_dropped = [this](const std::vector<std::string>& files) {
                setState([this, files]() { dropped_files_ = files; });
            },
        });

        std::string drop_summary = "Loaded: ";
        for (size_t i = 0; i < dropped_files_.size(); ++i) {
            if (i > 0) drop_summary += ", ";
            drop_summary += dropped_files_[i];
        }

        auto drop_status = text(drop_summary, {
            .color = 0xFF38BDF8,
            .font_size = 11.5f,
        });

        auto card4 = buildCard("📥 OS Desktop File Drop Zone", "FileDropZone with Animated Neon Dashed Conveyor Border", {
            drop_zone,
            drop_status,
        });

        // ── Grid Layout ───────────────────────────────────────────────
        auto top_row = row(FlexboxProps{
            .justify_content = Justify::Center,
            .align_items = Align::Start,
            .gap = StyleValue::point(16.0f),
            .children = {card1, card2},
        });

        auto bottom_row = row(FlexboxProps{
            .justify_content = Justify::Center,
            .align_items = Align::Start,
            .gap = StyleValue::point(16.0f),
            .children = {card3, card4},
        });

        auto main_content = column(FlexboxProps{
            .align_items = Align::Center,
            .gap = StyleValue::point(16.0f),
            .children = {title_bar, top_row, bottom_row},
        });

        auto app_body = container(Container{
            .color = 0x4D000000,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .padding = StyleInsets::symmetric(14.0f, 18.0f),
            .child = main_content,
        });

        return windowFrame(WindowFrameProps{
            .content = app_body,
            .title = "ENKI Studio — Rich Input Controls Suite (VAXP-OS)",
            .border_radius = 16.0f,
            .border_color = 0x3300E5FF,
            .border_width = 1.0f,
            .background_color = 0x4D000000,
            .titlebar_background_color = 0x4D000000,
            .titlebar_inactive_background_color = 0x4D000000,
            .titlebar_style = TitleBarStyle::VAXPOS,
        });
    }
};

class RichInputsDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<RichInputsDemoState>();
    }
    std::string_view typeName() const override { return "RichInputsDemoApp"; }
};

int main() {
    std::cout << "====================================================\n";
    std::cout << "  ENKI Engine — Rich Input Controls Studio Demo     \n";
    std::cout << "  Roadmap v0.2.0 | Section 15                       \n";
    std::cout << "====================================================\n";

    AppConfig config;
    config.title       = "ENKI Studio — Rich Input Controls Suite (VAXP-OS)";
    config.width       = 960;
    config.height      = 720;
    config.resizable   = true;
    config.enable_csd  = true;
    config.target_fps  = 0;
    config.vsync       = false;
    config.show_performance_overlay = true;
    config.clear_color = 0x0000004D;
    config.app_id      = "org.vaxp.enki.rich_inputs";

    return runApp(std::make_shared<RichInputsDemoApp>(), config);
}
