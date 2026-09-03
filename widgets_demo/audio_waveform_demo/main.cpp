/// @file main.cpp
/// @brief Interactive Showcase Demo for ENKI AudioWaveform Widget.
///
/// Demonstrates Luxurious & Distinctive Visualizer Styles:
///   1. SegmentedLed: Studio Rack LED ladder with translucent glass floor reflection
///   2. RadialArc: 360° circular pulsing sound reactor with bass beat core
///   3. LayeredAurora: Triple-layer translucent neon spline waves with ambient glow
///   4. SymmetricWings: Center-outward mirrored cyber wings
///   5. VoiceBars: Dynamic speech pills with neon emerald glow
///   6. ScrollingLive: Right-to-left scrolling audio history tape
///   7. LiveOscilloscope: Real-time vibration oscilloscope
///
/// @copyright ENKI Framework — MIT License

#include "enki/app/app.hpp"
#include "enki/widgets/audio_waveform.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/titlebar.hpp"
#include "enki/widgets/window_frame.hpp"
#include "enki/state/state.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <memory>

using namespace enki;

class AudioWaveformDemoState : public State {
private:
    int current_tab_ = 0; // 0 = Wave Music (System Audio), 1 = Wave Mic (Microphone)

    std::shared_ptr<audio::AudioController> music_controller_;
    std::shared_ptr<audio::AudioController> mic_controller_;

public:
    void initState() override {
        State::initState();

        music_controller_ = std::make_shared<audio::AudioController>();
        music_controller_->startSystemAudio();

        mic_controller_ = std::make_shared<audio::AudioController>();
        mic_controller_->startMicrophone();
    }

    void dispose() override {
        if (music_controller_) music_controller_->stop();
        if (mic_controller_) mic_controller_->stop();
        State::dispose();
    }

    WidgetPtr build(BuildContext&) override {
        // ── Header ────────────────────────────────────────────────────
        auto title = text("AudioWaveform Studio Suite", {
            .color = 0xFFF8FAFC,
            .font_size = 24.0f,
            .font_weight = FontWeight::Bold,
        });

        auto subtitle = text("Real-time System Audio Spectrum Analyzer & Live Microphone Waveform — Roadmap Section 16", {
            .color = 0xFF94A3B8,
            .font_size = 13.0f,
        });

        auto header = column({
            .align_items = Align::Center,
            .gap = StyleValue::point(6.0f),
            .children = {title, subtitle},
        });

        // ── Tab Switcher Buttons ──────────────────────────────────────
        auto tab_music_btn = button(
            text("🎵 Wave Music (System Audio — Spotify / YouTube / Desktop)", {
                .color = (current_tab_ == 0) ? 0xFFFFFFFF : 0xFF94A3B8,
                .font_size = 13.0f,
                .font_weight = (current_tab_ == 0) ? FontWeight::Bold : FontWeight::Medium,
            }),
            [this]() {
                if (current_tab_ != 0) {
                    setState([this]() { current_tab_ = 0; });
                }
            },
            {
                .normal_color = (current_tab_ == 0) ? 0xFF0284C7 : 0xFF1E293B,
                .hover_color  = (current_tab_ == 0) ? 0xFF0369A1 : 0xFF334155,
                .border_radius = 10.0f,
                .padding = EdgeInsets::symmetric(10.0f, 20.0f),
            }
        );

        auto tab_mic_btn = button(
            text("🎙️ Wave Mic (Live Microphone Input)", {
                .color = (current_tab_ == 1) ? 0xFFFFFFFF : 0xFF94A3B8,
                .font_size = 13.0f,
                .font_weight = (current_tab_ == 1) ? FontWeight::Bold : FontWeight::Medium,
            }),
            [this]() {
                if (current_tab_ != 1) {
                    setState([this]() { current_tab_ = 1; });
                }
            },
            {
                .normal_color = (current_tab_ == 1) ? 0xFF059669 : 0xFF1E293B,
                .hover_color  = (current_tab_ == 1) ? 0xFF047857 : 0xFF334155,
                .border_radius = 10.0f,
                .padding = EdgeInsets::symmetric(10.0f, 20.0f),
            }
        );

        auto tab_bar = row({
            .justify_content = Justify::Center,
            .align_items = Align::Center,
            .gap = StyleValue::point(14.0f),
            .children = {tab_music_btn, tab_mic_btn},
        });

        // ── Tab Content ───────────────────────────────────────────────
        WidgetPtr active_view = (current_tab_ == 0) ? buildMusicTab() : buildMicTab();

        auto main_col = column({
            .align_items = Align::Center,
            .gap = StyleValue::point(18.0f),
            .children = {header, tab_bar, active_view},
        });

        auto app_body = container({
            .color = 0x4D000000,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .padding = StyleInsets::symmetric(14.0f, 18.0f),
            .child = main_col,
        });

        return windowFrame(WindowFrameProps{
            .content = app_body,
            .title = "AudioWaveform Studio Suite — VAXP Audio Engine",
            .border_radius = 16.0f,
            .border_color = 0x3300E5FF,
            .border_width = 1.0f,
            .background_color = 0x4D000000,
            .titlebar_background_color = 0x4D000000,
            .titlebar_inactive_background_color = 0x4D000000,
            .titlebar_style = TitleBarStyle::VAXPOS,
        });
    }

private:
    WidgetPtr buildMusicTab() {
        // ── Card 1: Studio Segmented LED Rack with Glass Reflection ─────
        auto led_title = text("🎛️ Studio Segmented LED Rack (Glass Reflection & Floating Peaks)", {
            .color = 0xFF00E5FF,
            .font_size = 14.0f,
            .font_weight = FontWeight::Bold,
        });

        auto led_widget = audioWaveform({
            .type = AudioWaveformType::Music,
            .style = WaveformStyle::SegmentedLed,
            .controller = music_controller_,
            .bands = 38,
            .bar_width = 9.0f,
            .bar_gap = 3.5f,
            .primary_color = 0xFF00E5FF,
            .secondary_color = 0xFFA855F7,
            .accent_color = 0xFFF43F5E,
            .show_peaks = true,
            .show_reflection = true,
            .sensitivity = 1.15f,
            .decay_rate = 0.045f,
            .width = 540.0f,
            .height = 200.0f,
        });

        auto led_card = container({
            .color = 0x8D000000,
            .border_radius = BorderRadius::circular(16.0f),
            .border = Border(0xFF1E293B, 1.2f),
            .padding = StyleInsets::all(16.0f),
            .child = column({
                .align_items = Align::Center,
                .gap = StyleValue::point(10.0f),
                .children = {led_title, led_widget},
            }),
        });

        // ── Card 2: Radial Arc 360° Circular Sound Reactor ─────────────
        auto radial_title = text("⚛️ 360° Circular Sound Reactor (Beat-Pulsing Bass Core)", {
            .color = 0xFF06B6D4,
            .font_size = 14.0f,
            .font_weight = FontWeight::Bold,
        });

        auto radial_widget = audioWaveform({
            .type = AudioWaveformType::Music,
            .style = WaveformStyle::RadialArc,
            .controller = music_controller_,
            .bands = 48,
            .bar_width = 2.5f,
            .primary_color = 0xFF06B6D4,
            .secondary_color = 0xFFEC4899,
            .accent_color = 0xFFF59E0B,
            .show_peaks = true,
            .sensitivity = 1.25f,
            .width = 460.0f,
            .height = 200.0f,
        });

        auto radial_card = container({
            .color = 0x8D000000,
            .border_radius = BorderRadius::circular(16.0f),
            .border = Border(0xFF1E293B, 1.2f),
            .padding = StyleInsets::all(16.0f),
            .child = column({
                .align_items = Align::Center,
                .gap = StyleValue::point(10.0f),
                .children = {radial_title, radial_widget},
            }),
        });

        // ── Card 3: Layered Aurora Neon Waves ──────────────────────────
        auto aurora_title = text("🌌 Layered Aurora Waves (Triple-Layer Translucent Splines)", {
            .color = 0xFF34D399,
            .font_size = 14.0f,
            .font_weight = FontWeight::Bold,
        });

        auto aurora_widget = audioWaveform({
            .type = AudioWaveformType::Music,
            .style = WaveformStyle::LayeredAurora,
            .controller = music_controller_,
            .bands = 36,
            .primary_color = 0xFF34D399,
            .secondary_color = 0xFF00E5FF,
            .accent_color = 0xFFA855F7,
            .sensitivity = 1.2f,
            .decay_rate = 0.05f,
            .width = 540.0f,
            .height = 150.0f,
        });

        auto aurora_card = container({
            .color = 0x8D000000,
            .border_radius = BorderRadius::circular(16.0f),
            .border = Border(0xFF1E293B, 1.2f),
            .padding = StyleInsets::all(14.0f),
            .child = column({
                .align_items = Align::Center,
                .gap = StyleValue::point(8.0f),
                .children = {aurora_title, aurora_widget},
            }),
        });

        // ── Card 4: Symmetric Cyber Wings ──────────────────────────────
        auto wings_title = text("🦋 Symmetric Cyber Wings (Center-Outward Mirrored Spectrum)", {
            .color = 0xFF00E5FF,
            .font_size = 14.0f,
            .font_weight = FontWeight::Bold,
        });

        auto wings_widget = audioWaveform({
            .type = AudioWaveformType::Music,
            .style = WaveformStyle::SymmetricWings,
            .controller = music_controller_,
            .bands = 40,
            .bar_width = 4.5f,
            .bar_gap = 2.0f,
            .primary_color = 0xFF00E5FF,
            .secondary_color = 0xFFA855F7,
            .sensitivity = 1.3f,
            .decay_rate = 0.05f,
            .width = 460.0f,
            .height = 150.0f,
        });

        auto wings_card = container({
            .color = 0x8D000000,
            .border_radius = BorderRadius::circular(16.0f),
            .border = Border(0xFF1E293B, 1.2f),
            .padding = StyleInsets::all(14.0f),
            .child = column({
                .align_items = Align::Center,
                .gap = StyleValue::point(8.0f),
                .children = {wings_title, wings_widget},
            }),
        });

        auto row_top = row({
            .justify_content = Justify::Center,
            .align_items = Align::Center,
            .gap = StyleValue::point(18.0f),
            .children = {led_card, radial_card},
        });

        auto row_bottom = row({
            .justify_content = Justify::Center,
            .align_items = Align::Center,
            .gap = StyleValue::point(18.0f),
            .children = {aurora_card, wings_card},
        });

        auto status_hint = text("💡 Real-time system audio visualizer — play any song on Spotify, YouTube, or VLC to see all 4 engines dance live.", {
            .color = 0xFF64748B,
            .font_size = 12.0f,
        });

        return column({
            .align_items = Align::Center,
            .gap = StyleValue::point(16.0f),
            .children = {row_top, row_bottom, status_hint},
        });
    }

    WidgetPtr buildMicTab() {
        // Card 1: Symmetrical Voice Activity Bars
        auto voice_title = text("🎙️ Centered Voice Activity Pills (Speech Harmonics)", {
            .color = 0xFF10B981,
            .font_size = 14.0f,
            .font_weight = FontWeight::Bold,
        });

        auto voice_widget = audioWaveform({
            .type = AudioWaveformType::Mic,
            .style = WaveformStyle::VoiceBars,
            .controller = mic_controller_,
            .bands = 32,
            .bar_width = 8.0f,
            .bar_gap = 4.0f,
            .primary_color = 0xFF10B981,
            .secondary_color = 0xFF34D399,
            .sensitivity = 1.9f,
            .decay_rate = 0.07f,
            .width = 500.0f,
            .height = 190.0f,
        });

        auto voice_card = container({
            .color = 0xFF0E1626,
            .border_radius = BorderRadius::circular(16.0f),
            .border = Border(0xFF1E293B, 1.2f),
            .padding = StyleInsets::all(16.0f),
            .child = column({
                .align_items = Align::Center,
                .gap = StyleValue::point(10.0f),
                .children = {voice_title, voice_widget},
            }),
        });

        // Card 2: 360° Circular Voice Orb (Mic Radial)
        auto mic_orb_title = text("🔮 360° Circular Voice Orb (Speech Pitch)", {
            .color = 0xFF34D399,
            .font_size = 14.0f,
            .font_weight = FontWeight::Bold,
        });

        auto mic_orb_widget = audioWaveform({
            .type = AudioWaveformType::Mic,
            .style = WaveformStyle::RadialArc,
            .controller = mic_controller_,
            .bands = 40,
            .bar_width = 3.0f,
            .primary_color = 0xFF10B981,
            .secondary_color = 0xFF00E5FF,
            .accent_color = 0xFFF59E0B,
            .show_peaks = true,
            .sensitivity = 2.0f,
            .width = 500.0f,
            .height = 190.0f,
        });

        auto mic_orb_card = container({
            .color = 0xFF0E1626,
            .border_radius = BorderRadius::circular(16.0f),
            .border = Border(0xFF1E293B, 1.2f),
            .padding = StyleInsets::all(16.0f),
            .child = column({
                .align_items = Align::Center,
                .gap = StyleValue::point(10.0f),
                .children = {mic_orb_title, mic_orb_widget},
            }),
        });

        // Card 3: Scrolling Live Speech Tape
        auto scroll_title = text("📊 Scrolling Speech Tape (Right-to-Left History)", {
            .color = 0xFF06B6D4,
            .font_size = 14.0f,
            .font_weight = FontWeight::Bold,
        });

        auto scroll_widget = audioWaveform({
            .type = AudioWaveformType::Mic,
            .style = WaveformStyle::ScrollingLive,
            .controller = mic_controller_,
            .bar_width = 4.0f,
            .bar_gap = 2.0f,
            .primary_color = 0xFF06B6D4,
            .sensitivity = 1.7f,
            .width = 500.0f,
            .height = 150.0f,
        });

        auto scroll_card = container({
            .color = 0xFF0E1626,
            .border_radius = BorderRadius::circular(16.0f),
            .border = Border(0xFF1E293B, 1.2f),
            .padding = StyleInsets::all(14.0f),
            .child = column({
                .align_items = Align::Center,
                .gap = StyleValue::point(8.0f),
                .children = {scroll_title, scroll_widget},
            }),
        });

        // Card 4: High-Frequency Oscilloscope
        auto osc_title = text("〰️ Real-Time Audio Vibration Wave", {
            .color = 0xFFF59E0B,
            .font_size = 14.0f,
            .font_weight = FontWeight::Bold,
        });

        auto osc_widget = audioWaveform({
            .type = AudioWaveformType::Mic,
            .style = WaveformStyle::LiveOscilloscope,
            .controller = mic_controller_,
            .bands = 54,
            .primary_color = 0xFFF59E0B,
            .sensitivity = 1.5f,
            .width = 500.0f,
            .height = 150.0f,
        });

        auto osc_card = container({
            .color = 0x4D000000,
            .border_radius = BorderRadius::circular(16.0f),
            .border = Border(0xFF1E293B, 1.2f),
            .padding = StyleInsets::all(14.0f),
            .child = column({
                .align_items = Align::Center,
                .gap = StyleValue::point(8.0f),
                .children = {osc_title, osc_widget},
            }),
        });

        auto mic_hint = text("💡 Speak into your microphone — the visualizer responds instantly to speech envelope, pitch, and harmonics.", {
            .color = 0xFF64748B,
            .font_size = 12.0f,
        });

        auto row_top = row({
            .justify_content = Justify::Center,
            .align_items = Align::Center,
            .gap = StyleValue::point(18.0f),
            .children = {voice_card, mic_orb_card},
        });

        auto row_bottom = row({
            .justify_content = Justify::Center,
            .align_items = Align::Center,
            .gap = StyleValue::point(18.0f),
            .children = {scroll_card, osc_card},
        });

        return column({
            .align_items = Align::Center,
            .gap = StyleValue::point(16.0f),
            .children = {row_top, row_bottom, mic_hint},
        });
    }
};

class AudioWaveformDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<AudioWaveformDemoState>();
    }
    std::string_view typeName() const override { return "AudioWaveformDemoApp"; }
};

int main() {
    std::cout << "====================================================\n";
    std::cout << "  ENKI Engine — AudioWaveform Studio Showcase Demo\n";
    std::cout << "  Roadmap v0.2.0 | Section 16 Media & Canvas\n";
    std::cout << "====================================================\n";

    AppConfig config;
    config.title       = "AudioWaveform Studio Suite — VAXP Audio Engine";
    config.width       = 1140;
    config.height      = 740;
    config.resizable   = true;
    config.enable_csd  = true;
    config.vsync       = false;
    config.target_fps  = 0;
    config.show_performance_overlay = false;
    config.clear_color = 0x0000004D;
    config.app_id      = "org.vaxp.enki.audio_waveform";

    return runApp(std::make_shared<AudioWaveformDemoApp>(), config);
}
