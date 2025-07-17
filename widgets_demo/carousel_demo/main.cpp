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

    // ── Helper to build rich banner slides ────────────────────────
    WidgetPtr buildHeroSlide(std::string badge, Color badge_col, std::string icon,
                             std::string title, std::string subtitle, std::string stat_label,
                             Color stat_col, std::string btn_label, std::function<void()> cb) {
        // Badge
        auto b_txt = text(badge);
        b_txt->fontSize(11.0f).bold().color(0xFFFFFFFF);
        auto b_box = container(b_txt);
        b_box->color(badge_col).borderRadius(4.0f).paddingSymmetric(3.0f, 8.0f);

        // Icon + Title
        auto ic = text(icon);
        ic->fontSize(22.0f);

        auto tit = text(title);
        tit->fontSize(18.0f).bold().color(0xFFFFFFFF);

        std::vector<WidgetPtr> h_items = {ic, tit};
        auto h_row = row(h_items);
        h_row->gap(StyleValue::point(10.0f)).alignItems(Align::Center);

        // Subtitle
        auto sub = text(subtitle);
        sub->fontSize(13.0f).color(0xFFCBD5E1);

        // Stat Label
        auto stat = text(stat_label);
        stat->fontSize(12.5f).bold().color(stat_col);

        // Action Button
        auto b_lbl = text(btn_label);
        b_lbl->fontSize(12.5f).bold().color(0xFFFFFFFF);
        auto btn_box = container(b_lbl);
        btn_box->color(0xFF0284C7).borderRadius(6.0f).paddingSymmetric(8.0f, 18.0f);

        auto btn_gd = std::make_shared<GestureDetector>(btn_box);
        btn_gd->cursor_type = SystemCursor::Pointer;
        btn_gd->on_tap_up = [cb](const TapUpDetails&) {
            if (cb) cb();
        };

        std::vector<WidgetPtr> left_items = {b_box, h_row, sub, stat, btn_gd};
        auto left_col = column(left_items);
        left_col->gap(StyleValue::point(12.0f)).alignItems(Align::Start);

        auto slide_container = container(left_col);
        slide_container->paddingSymmetric(28.0f, 36.0f)
                       .width(StyleValue::percent(100.0f))
                       .height(StyleValue::percent(100.0f));

        return slide_container;
    }

public:
    void initState() override {
        State::initState();
        carousel_ctrl_ = std::make_shared<CarouselController>();
    }

    WidgetPtr build(BuildContext&) override {
        // ── Main Page Header ──────────────────────────────────────────
        auto title = text("Advanced Carousel & Slideshow Suite");
        title->fontSize(22.0f).bold().color(0xFFFFFFFF);

        auto sub = text("Multi-card swiper (Category 10. Advanced / Data UI), autoplay with pause-on-hover, swipe gestures, and pagination dots");
        sub->fontSize(13.0f).color(0xFF94A3B8);

        std::vector<WidgetPtr> title_items = {title, sub};
        auto title_col = column(title_items);
        title_col->alignItems(Align::Center).gap(StyleValue::point(6.0f));

        // ── 4 Rich Slide Cards ────────────────────────────────────────
        auto slide1 = buildHeroSlide("FEATURED ENGINE", 0xFF0284C7, "⚡",
                                     "Skia Real-Time 60 FPS Compositor",
                                     "Ultra-low latency hardware accelerated rendering pipeline supporting Vulkan & Wayland.",
                                     "🚀 Performance: 4.2ms frame latency • 120 FPS capable", 0xFF38BDF8,
                                     "Explore Architecture", [this] {
            hud_msg_ = "Action: Opened Skia Compositor Architecture.";
            setState([] {});
        });

        auto slide2 = buildHeroSlide("CLOUD DEPLOYMENT", 0xFF059669, "🌐",
                                     "240 Edge Kubernetes Clusters Worldwide",
                                     "Global edge distribution network with instant cold-start container initialization.",
                                     "🟢 Global Status: 99.999% Uptime across 14 regions", 0xFF10B981,
                                     "View Cloud Regions", [this] {
            hud_msg_ = "Action: Opened Global Cloud Infrastructure page.";
            setState([] {});
        });

        auto slide3 = buildHeroSlide("AI ACCELERATION", 0xFF7C3AED, "🤖",
                                     "NVIDIA H100 GPU Vector Processing",
                                     "Native TensorRT neural network engine integration for real-time model inference.",
                                     "⚡ Compute: 640GB VRAM pool allocated for LLM inference", 0xFFA78BFA,
                                     "Manage GPU Pools", [this] {
            hud_msg_ = "Action: Opened AI & GPU Cluster Manager.";
            setState([] {});
        });

        auto slide4 = buildHeroSlide("SECURITY SUITE", 0xFFD97706, "🛡️",
                                     "Zero-Trust Cryptographic Access & IAM",
                                     "Hardware-enforced enclave keys with end-to-end telemetry encryption and audit logs.",
                                     "🔒 Security: 0 Active Vulnerabilities • SOC-2 Type II Certified", 0xFFF59E0B,
                                     "Review Security Logs", [this] {
            hud_msg_ = "Action: Opened IAM & Audit Logs Dashboard.";
            setState([] {});
        });

        std::vector<WidgetPtr> slides_list = {slide1, slide2, slide3, slide4};

        CarouselOptions carousel_opts;
        carousel_opts.height = 300.0f;
        carousel_opts.auto_play = is_playing_;
        carousel_opts.auto_play_interval_ms = 3500;
        carousel_opts.pause_on_hover = true;
        carousel_opts.on_page_changed = [this](int idx) {
            hud_msg_ = "Current Slide: #" + std::to_string(idx + 1) + " of 4";
            setState([] {});
        };

        auto carousel_widget = carousel(slides_list, carousel_opts, carousel_ctrl_);

        auto carousel_frame = container(carousel_widget);
        carousel_frame->width(880.0f)
                      .borderRadius(14.0f)
                      .border(0xFF334155, 1.0f)
                      .shadow(BoxShadow(0x99000000, {0.0f, 8.0f}, 24.0f));

        // ── Programmatic Navigation Bar ───────────────────────────────
        auto btn_prev = button(text("◀ Previous Slide"), [this] {
            carousel_ctrl_->previousPage();
        });

        auto btn_play_pause = button(text(is_playing_ ? "⏸ Pause Autoplay" : "▶ Resume Autoplay"), [this] {
            is_playing_ = !is_playing_;
            carousel_ctrl_->setAutoPlay(is_playing_);
            hud_msg_ = is_playing_ ? "Resumed Autoplay." : "Paused Autoplay.";
            setState([] {});
        });

        auto btn_next = button(text("Next Slide ▶"), [this] {
            carousel_ctrl_->nextPage();
        });

        std::vector<WidgetPtr> nav_btn_items = {btn_prev, btn_play_pause, btn_next};
        auto nav_btn_row = row(nav_btn_items);
        nav_btn_row->gap(StyleValue::point(12.0f)).justifyContent(Justify::Center);

        // Direct Jump Pills
        auto makeJumpPill = [this](std::string label, int page) -> WidgetPtr {
            auto t = text(label);
            t->fontSize(12.0f).color(0xFF94A3B8);

            auto b = container(t);
            b->color(0xFF0F172A)
             .border(0xFF334155, 1.0f)
             .borderRadius(6.0f)
             .paddingSymmetric(6.0f, 14.0f);

            auto gd = std::make_shared<GestureDetector>(b);
            gd->cursor_type = SystemCursor::Pointer;
            gd->on_tap_up = [this, page](const TapUpDetails&) {
                carousel_ctrl_->jumpToPage(page);
            };
            return gd;
        };

        std::vector<WidgetPtr> jump_pills = {
            makeJumpPill("⚡ Skia Compositor", 0),
            makeJumpPill("🌐 Edge Kubernetes", 1),
            makeJumpPill("🤖 AI Clusters", 2),
            makeJumpPill("🛡️ Security Suite", 3)
        };
        auto jump_row = row(jump_pills);
        jump_row->gap(StyleValue::point(8.0f)).justifyContent(Justify::Center);

        // ── HUD / Status Box ──────────────────────────────────────────
        auto hud_txt = text("💡 " + hud_msg_);
        hud_txt->fontSize(12.5f).color(0xFF38BDF8);

        auto hud_row = row(std::vector<WidgetPtr>{hud_txt});
        auto hud_box = container(hud_row);
        hud_box->color(0xFF1E293B)
               .borderRadius(6.0f)
               .border(0xFF334155, 1.0f)
               .paddingSymmetric(8.0f, 16.0f)
               .width(880.0f);

        // ── Assemble Page Body ────────────────────────────────────────
        std::vector<WidgetPtr> page_items = {title_col, carousel_frame, nav_btn_row, jump_row, hud_box};
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
