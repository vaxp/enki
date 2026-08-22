/// @file main.cpp
/// @brief ENKI Advanced Carousel & Swiper Interactive Showcase Demo.

#include "enki/app/app.hpp"
#include "enki/widgets/carousel.hpp"
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

class CarouselDemoState : public State {
private:
    std::shared_ptr<CarouselController> carousel_ctrl_;
    std::string hud_msg_ = "Carousel is autoplaying (slides advance every 3.5s). Hover mouse over slide to pause timer!";
    bool is_playing_ = true;

    void initState() override {
        State::initState();
        carousel_ctrl_ = std::make_shared<CarouselController>();
    }

    WidgetPtr build(BuildContext&) override {
        auto buildHeroSlide = [this](std::string badge, Color badge_col, std::string icon,
                             std::string title, std::string subtitle, std::string stat_label,
                             Color stat_col, std::string btn_label, std::function<void()> cb) {
            return container({
                .width = StyleValue::percent(100.0f),
                .height = StyleValue::percent(100.0f),
                .padding = StyleInsets::symmetric(28.0f, 36.0f),
                .child = column({
                    .align_items = Align::Start,
                    .gap = StyleValue::point(12.0f),
                    .children = {
                        container({
                            .color = badge_col,
                            .border_radius = BorderRadius::circular(4.0f),
                            .padding = StyleInsets::symmetric(3.0f, 8.0f),
                            .child = text(badge, { .color = 0xFFFFFFFF, .font_size = 11.0f, .font_weight = FontWeight::Bold })
                        }),
                        row({
                            .align_items = Align::Center,
                            .gap = StyleValue::point(10.0f),
                            .children = {
                                text(icon, { .font_size = 22.0f }),
                                text(title, { .color = 0xFFFFFFFF, .font_size = 18.0f, .font_weight = FontWeight::Bold })
                            }
                        }),
                        text(subtitle, { .color = 0xFFCBD5E1, .font_size = 13.0f }),
                        text(stat_label, { .color = stat_col, .font_size = 12.5f, .font_weight = FontWeight::Bold }),
                        gestureDetector({
                            .child = container({
                                .color = 0xFF0284C7,
                                .border_radius = BorderRadius::circular(6.0f),
                                .padding = StyleInsets::symmetric(8.0f, 18.0f),
                                .child = text(btn_label, { .color = 0xFFFFFFFF, .font_size = 12.5f, .font_weight = FontWeight::Bold })
                            }),
                            .cursor_type = SystemCursor::Pointer,
                            .on_tap_up = [cb](const TapUpDetails&) { if (cb) cb(); }
                        })
                    }
                })
            });
        };

        auto makeJumpPill = [this](std::string label, int page) -> WidgetPtr {
            return gestureDetector({
                .child = container({
                    .color = 0xFF0F172A,
                    .border_radius = BorderRadius::circular(6.0f),
                    .border = Border(0xFF334155, 1.0f),
                    .padding = StyleInsets::symmetric(6.0f, 14.0f),
                    .child = text(label, { .color = 0xFF94A3B8, .font_size = 12.0f })
                }),
                .cursor_type = SystemCursor::Pointer,
                .on_tap_up = [this, page](const TapUpDetails&) { carousel_ctrl_->jumpToPage(page); }
            });
        };

        return container({
            .color = 0xFF0B1120,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(24.0f),
            .child = column({
                .align_items = Align::Center,
                .gap = StyleValue::point(18.0f),
                .children = {
                    column({
                        .align_items = Align::Center,
                        .gap = StyleValue::point(6.0f),
                        .children = {
                            text("Advanced Carousel & Slideshow Suite", { .color = 0xFFFFFFFF, .font_size = 22.0f, .font_weight = FontWeight::Bold }),
                            text("Multi-card swiper (Category 10. Advanced / Data UI), autoplay with pause-on-hover, swipe gestures, and pagination dots", { .color = 0xFF94A3B8, .font_size = 13.0f })
                        }
                    }),
                    container({
                        .border_radius = BorderRadius::circular(14.0f),
                        .border = Border(0xFF334155, 1.0f),
                        .box_shadow = {BoxShadow(0x99000000, {0.0f, 8.0f}, 24.0f)},
                        .width = StyleValue::point(880.0f),
                        .child = Carousel {
                            .slides = {
                                buildHeroSlide("FEATURED ENGINE", 0xFF0284C7, "⚡",
                                               "Skia Real-Time 60 FPS Compositor",
                                               "Ultra-low latency hardware accelerated rendering pipeline supporting Vulkan & Wayland.",
                                               "🚀 Performance: 4.2ms frame latency • 120 FPS capable", 0xFF38BDF8,
                                               "Explore Architecture", [this] {
                                    hud_msg_ = "Action: Opened Skia Compositor Architecture.";
                                    setState([] {});
                                }),
                                buildHeroSlide("CLOUD DEPLOYMENT", 0xFF059669, "🌐",
                                               "240 Edge Kubernetes Clusters Worldwide",
                                               "Global edge distribution network with instant cold-start container initialization.",
                                               "🟢 Global Status: 99.999% Uptime across 14 regions", 0xFF10B981,
                                               "View Cloud Regions", [this] {
                                    hud_msg_ = "Action: Opened Global Cloud Infrastructure page.";
                                    setState([] {});
                                }),
                                buildHeroSlide("AI ACCELERATION", 0xFF7C3AED, "🤖",
                                               "NVIDIA H100 GPU Vector Processing",
                                               "Native TensorRT neural network engine integration for real-time model inference.",
                                               "⚡ Compute: 640GB VRAM pool allocated for LLM inference", 0xFFA78BFA,
                                               "Manage GPU Pools", [this] {
                                    hud_msg_ = "Action: Opened AI & GPU Cluster Manager.";
                                    setState([] {});
                                }),
                                buildHeroSlide("SECURITY SUITE", 0xFFD97706, "🛡️",
                                               "Zero-Trust Cryptographic Access & IAM",
                                               "Hardware-enforced enclave keys with end-to-end telemetry encryption and audit logs.",
                                               "🔒 Security: 0 Active Vulnerabilities • SOC-2 Type II Certified", 0xFFF59E0B,
                                               "Review Security Logs", [this] {
                                    hud_msg_ = "Action: Opened IAM & Audit Logs Dashboard.";
                                    setState([] {});
                                })
                            },
                            .controller = carousel_ctrl_,
                            .auto_play = is_playing_,
                            .auto_play_interval_ms = 3500,
                            .pause_on_hover = true,
                            .height = 300.0f,
                            .on_page_changed = [this](int idx) {
                                hud_msg_ = "Current Slide: #" + std::to_string(idx + 1) + " of 4";
                                setState([] {});
                            }
                        }
                    }),
                    row({
                        .justify_content = Justify::Center,
                        .gap = StyleValue::point(12.0f),
                        .children = {
                            button(text("◀ Previous Slide"), [this] { carousel_ctrl_->previousPage(); }),
                            button(text(is_playing_ ? "⏸ Pause Autoplay" : "▶ Resume Autoplay"), [this] {
                                is_playing_ = !is_playing_;
                                carousel_ctrl_->setAutoPlay(is_playing_);
                                hud_msg_ = is_playing_ ? "Resumed Autoplay." : "Paused Autoplay.";
                                setState([] {});
                            }),
                            button(text("Next Slide ▶"), [this] { carousel_ctrl_->nextPage(); })
                        }
                    }),
                    row({
                        .justify_content = Justify::Center,
                        .gap = StyleValue::point(8.0f),
                        .children = {
                            makeJumpPill("⚡ Skia Compositor", 0),
                            makeJumpPill("🌐 Edge Kubernetes", 1),
                            makeJumpPill("🤖 AI Clusters", 2),
                            makeJumpPill("🛡️ Security Suite", 3)
                        }
                    }),
                    container({
                        .color = 0xFF1E293B,
                        .border_radius = BorderRadius::circular(6.0f),
                        .border = Border(0xFF334155, 1.0f),
                        .width = StyleValue::point(880.0f),
                        .padding = StyleInsets::symmetric(8.0f, 16.0f),
                        .child = row({
                            .children = { text("💡 " + hud_msg_, { .color = 0xFF38BDF8, .font_size = 12.5f }) }
                        })
                    })
                }
            })
        });
    }
};

class CarouselDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<CarouselDemoState>();
    }
    std::string_view typeName() const override { return "CarouselDemoApp"; }
};

int main() {
    std::cout << "====================================================\n";
    std::cout << "  ENKI Engine — Advanced Carousel & Swiper Demo\n";
    std::cout << "====================================================\n";

    AppConfig config;
    config.title       = "Enki Engine — Advanced Carousel & Swiper Demo";
    config.width       = 1180;
    config.height      = 720;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 60;
    config.clear_color = 0xFF0B1120;

    return runApp(std::make_shared<CarouselDemoApp>(), config);
}
