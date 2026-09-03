/// @file main.cpp
/// @brief Interactive Showcase Demo for ENKI VideoPlayer Widget.
///
/// Features:
///   - Native Hardware Zero-Copy Video Decoding (FFmpeg 6.1 + VA-API)
///   - Master-Clock A/V Sync (PulseAudio Simple API)
///   - Declarative VideoPlayer Widget with Glassmorphic Overlay HUD
///   - Dynamic Aspect Ratio Switching (Contain, Cover, Fill)
///   - Playback Speed Selector (0.5x, 1.0x, 1.5x, 2.0x)
///   - Full Client-Side Decorations with TitleBarStyle::VAXPOS
///
/// @copyright ENKI Framework — MIT License

#include "enki/app/app.hpp"
#include "enki/widgets/video_player.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/titlebar.hpp"
#include "enki/widgets/window_frame.hpp"
#include "enki/state/state.hpp"

#include <iostream>
#include <memory>
#include <string>

using namespace enki;

class VideoPlayerDemoState : public State {
private:
    std::shared_ptr<video::VideoController> controller_;
    std::string current_source_ = "widgets_demo/video_player_demo/sample_bunny.mp4";
    BoxFit current_fit_ = BoxFit::Contain;
    float  current_speed_ = 1.0f;
    bool   is_looping_ = true;

public:
    void initState() override {
        State::initState();

        controller_ = std::make_shared<video::VideoController>();
        controller_->setLooping(is_looping_);
        controller_->open(current_source_);
        controller_->play();
    }

    void dispose() override {
        if (controller_) {
            controller_->stop();
            controller_->close();
        }
        State::dispose();
    }

    WidgetPtr build(BuildContext&) override {
        // ── Header Title & Subtitle ───────────────────────────────────
        auto title = text("VideoPlayer Studio Suite", {
            .color = 0xFFF8FAFC,
            .font_size = 22.0f,
            .font_weight = FontWeight::Bold,
        });

        auto subtitle = text("Hardware Zero-Copy Video Decoding & PulseAudio Master Clock — Roadmap Section 16", {
            .color = 0xFF00E5FF,
            .font_size = 13.0f,
            .font_weight = FontWeight::Medium,
        });

        auto header = column({
            .align_items = Align::Center,
            .gap = StyleValue::point(4.0f),
            .children = {title, subtitle},
        });

        // ── Main Video Player Screen ──────────────────────────────────
        auto player_widget = videoPlayer({
            .source = current_source_,
            .controller = controller_,
            .auto_play = true,
            .looping = is_looping_,
            .show_controls = true,
            .fit = current_fit_,
            .border_radius = BorderRadius::circular(14.0f),
            .background_color = 0xFF050811,
            .playback_speed = current_speed_,
            .width = 860.0f,
            .height = 480.0f,
        });

        auto video_card = container({
            .color = 0x8D000000,
            .border_radius = BorderRadius::circular(18.0f),
            .border = Border(0x3300E5FF, 1.2f),
            .padding = StyleInsets::all(10.0f),
            .child = player_widget,
        });

        // ── Video Source Switcher ──────────────────────────────────────
        auto make_src_btn = [this](const std::string& label, const std::string& path) {
            bool active = (current_source_ == path);
            return button(
                text(label, {
                    .color = active ? 0xFF38BDF8 : 0xFF94A3B8,
                    .font_size = 12.0f,
                    .font_weight = active ? FontWeight::Bold : FontWeight::Medium,
                }),
                [this, path]() {
                    if (current_source_ != path) {
                        setState([this, path]() {
                            current_source_ = path;
                            if (controller_) {
                                controller_->open(path);
                                controller_->setLooping(is_looping_);
                                controller_->setPlaybackSpeed(current_speed_);
                                controller_->play();
                            }
                        });
                    }
                },
                {
                    .normal_color = active ? 0xFF0C3559 : 0x4D000000,
                    .hover_color  = active ? 0xFF154C7E : 0xFF1E293B,
                    .border_radius = 8.0f,
                    .padding = EdgeInsets::symmetric(6.0f, 12.0f),
                }
            );
        };

        auto src_row = row({
            .align_items = Align::Center,
            .gap = StyleValue::point(8.0f),
            .children = {
                text("Video Track:", {.color = 0xFF94A3B8, .font_size = 12.0f}),
                make_src_btn("🎬 Big Buck Bunny (3.9MB Soundtrack)", "widgets_demo/video_player_demo/sample_bunny.mp4"),
                make_src_btn("📊 Synthetic A/V Sync Tone", "widgets_demo/video_player_demo/sample_test.mp4"),
            },
        });

        // ── Fit Mode Buttons ──────────────────────────────────────────
        auto make_fit_btn = [this](const std::string& label, BoxFit fit) {
            bool active = (current_fit_ == fit);
            return button(
                text(label, {
                    .color = active ? 0xFF00E5FF : 0xFF94A3B8,
                    .font_size = 12.0f,
                    .font_weight = active ? FontWeight::Bold : FontWeight::Medium,
                }),
                [this, fit]() {
                    setState([this, fit]() { current_fit_ = fit; });
                },
                {
                    .normal_color = active ? 0xFF0F2642 : 0x4D000000,
                    .hover_color  = active ? 0xFF173D68 : 0xFF1E293B,
                    .border_radius = 8.0f,
                    .padding = EdgeInsets::symmetric(6.0f, 14.0f),
                }
            );
        };

        auto fit_row = row({
            .align_items = Align::Center,
            .gap = StyleValue::point(8.0f),
            .children = {
                text("Aspect Fit:", {.color = 0xFF94A3B8, .font_size = 12.0f}),
                make_fit_btn("Contain (16:9)", BoxFit::Contain),
                make_fit_btn("Cover (Fill & Crop)", BoxFit::Cover),
                make_fit_btn("Fill (Stretch)", BoxFit::Fill),
            },
        });

        // ── Transport Quick Controls ──────────────────────────────────
        auto btn_play_pause = button(
            text(controller_ && controller_->isPlaying() ? "❚❚ Pause" : "▶ Play", {
                .color = 0xFFFFFFFF,
                .font_size = 12.0f,
                .font_weight = FontWeight::SemiBold,
            }),
            [this]() {
                if (controller_) {
                    controller_->togglePlay();
                    setState([]() {});
                }
            },
            {
                .normal_color = 0xFF0284C7,
                .hover_color  = 0xFF0369A1,
                .border_radius = 8.0f,
                .padding = EdgeInsets::symmetric(8.0f, 16.0f),
            }
        );

        auto btn_rewind = button(
            text("⏮ Restart", {.color = 0xFFFFFFFF, .font_size = 12.0f}),
            [this]() {
                if (controller_) {
                    controller_->seek(0.0);
                    controller_->play();
                    setState([]() {});
                }
            },
            {
                .normal_color = 0x6D000000,
                .hover_color  = 0xFF334155,
                .border_radius = 8.0f,
                .padding = EdgeInsets::symmetric(8.0f, 14.0f),
            }
        );

        auto btn_loop = button(
            text(is_looping_ ? "🔁 Loop: ON" : "🔁 Loop: OFF", {
                .color = is_looping_ ? 0xFFA855F7 : 0xFF64748B,
                .font_size = 12.0f,
                .font_weight = is_looping_ ? FontWeight::Bold : FontWeight::Medium,
            }),
            [this]() {
                setState([this]() {
                    is_looping_ = !is_looping_;
                    if (controller_) controller_->setLooping(is_looping_);
                });
            },
            {
                .normal_color = 0x6D000000,
                .hover_color  = 0xFF334155,
                .border_radius = 8.0f,
                .padding = EdgeInsets::symmetric(8.0f, 14.0f),
            }
        );

        auto btn_seek_back = button(
            text("⏪ -5s", {.color = 0xFFE2E8F0, .font_size = 12.0f}),
            [this]() {
                if (controller_) {
                    double cur = controller_->getCurrentPosition();
                    controller_->seek(std::max(0.0, cur - 5.0));
                    setState([]() {});
                }
            },
            {
                .normal_color = 0x6D000000,
                .hover_color  = 0xFF334155,
                .border_radius = 8.0f,
                .padding = EdgeInsets::symmetric(8.0f, 12.0f),
            }
        );

        auto btn_seek_fwd = button(
            text("⏩ +5s", {.color = 0xFFE2E8F0, .font_size = 12.0f}),
            [this]() {
                if (controller_) {
                    double cur = controller_->getCurrentPosition();
                    double dur = controller_->getDuration();
                    controller_->seek(std::min(dur, cur + 5.0));
                    setState([]() {});
                }
            },
            {
                .normal_color = 0x6D000000,
                .hover_color  = 0xFF334155,
                .border_radius = 8.0f,
                .padding = EdgeInsets::symmetric(8.0f, 12.0f),
            }
        );

        auto transport_row = row({
            .align_items = Align::Center,
            .gap = StyleValue::point(8.0f),
            .children = {btn_play_pause, btn_seek_back, btn_seek_fwd, btn_rewind, btn_loop},
        });

        // ── Controls & Options Strip ──────────────────────────────────
        auto options_row = row({
            .justify_content = Justify::SpaceBetween,
            .align_items = Align::Center,
            .width = 860.0f,
            .children = {transport_row, fit_row},
        });

        auto switch_row = row({
            .justify_content = Justify::Center,
            .align_items = Align::Center,
            .width = 860.0f,
            .children = {src_row},
        });

        // ── Stream Details / Stats Badge ──────────────────────────────
        auto meta = controller_ ? controller_->getMetadata() : video::VideoMetadata{};
        std::string meta_str = "📺 " + (meta.width > 0 ? std::to_string(meta.width) : "854") + "x" +
                               (meta.height > 0 ? std::to_string(meta.height) : "480") +
                               " | " + (meta.video_codec.empty() ? "H.264 AVC" : meta.video_codec) +
                               " | " + std::to_string(int(meta.fps > 0 ? meta.fps : 24)) + " FPS" +
                               " | " + (meta.audio_codec.empty() ? "AAC" : meta.audio_codec) +
                               " (" + std::to_string(meta.audio_sample_rate > 0 ? meta.audio_sample_rate : 48000) + "Hz " +
                               std::to_string(meta.audio_channels > 0 ? meta.audio_channels : 2) + "ch)" +
                               " | VA-API Hardware Zero-Copy";

        auto stats_badge = text(meta_str, {
            .color = 0xFF64748B,
            .font_size = 11.5f,
        });

        auto main_content = column({
            .align_items = Align::Center,
            .gap = StyleValue::point(12.0f),
            .children = {header, video_card, options_row, switch_row, stats_badge},
        });

        auto app_body = container({
            .color = 0x4D000000,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .padding = StyleInsets::symmetric(14.0f, 18.0f),
            .child = main_content,
        });

        // Wrap in full CSD WindowFrame
        return windowFrame(WindowFrameProps{
            .content = app_body,
            .title = "ENKI VideoPlayer Studio Suite — VAXP Media Engine",
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

class VideoPlayerDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<VideoPlayerDemoState>();
    }
    std::string_view typeName() const override { return "VideoPlayerDemoApp"; }
};

int main() {
    std::cout << "====================================================\n";
    std::cout << "  ENKI Engine — VideoPlayer Studio Showcase Demo\n";
    std::cout << "  Roadmap v0.2.0 | Section 16 Media & Canvas\n";
    std::cout << "====================================================\n";

    AppConfig config;
    config.title       = "ENKI VideoPlayer Studio Suite — VAXP Media Engine";
    config.width       = 920;
    config.height      = 760;
    config.resizable   = true;
    config.enable_csd  = true;
    config.vsync       = false;
    config.target_fps  = 0;
    config.show_performance_overlay = false;
    config.clear_color = 0x0000004D;
    config.app_id      = "org.vaxp.enki.video_player";

    return runApp(std::make_shared<VideoPlayerDemoApp>(), config);
}
