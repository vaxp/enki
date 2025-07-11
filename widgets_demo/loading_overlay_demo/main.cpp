/// @file main.cpp
/// @brief ENKI Advanced LoadingOverlay Interactive Showcase Demo.

#include "enki/app/app.hpp"
#include "enki/widgets/loading_overlay.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/stack.hpp"
#include "enki/animation/ticker.hpp"
#include "enki/state/state.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <chrono>

using namespace enki;

class LoadingOverlayDemoState : public State {
private:
    std::shared_ptr<LoadingOverlayController> loading_ctrl_;
    std::unique_ptr<Ticker> sim_ticker_;

    std::string hud_msg_ = "Click any card button to launch an interactive loading overlay.";
    bool is_simulating_ = false;
    float sim_progress_ = 0.0f;
    std::string sim_mode_ = "";
    std::chrono::steady_clock::time_point sim_start_;

public:
    void initState() override {
        State::initState();
        loading_ctrl_ = std::make_shared<LoadingOverlayController>();

        // Ticker to simulate realistic live progress updates
        sim_ticker_ = createTicker([this] {
            if (!is_simulating_) return;

            auto now = std::chrono::steady_clock::now();
            double elapsed = std::chrono::duration<double>(now - sim_start_).count();

            if (sim_mode_ == "ring") {
                sim_progress_ = static_cast<float>(elapsed / 3.0); // 3 seconds to 100%
                if (sim_progress_ >= 1.0f) {
                    is_simulating_ = false;
                    loading_ctrl_->hide();
                    hud_msg_ = "Operation Completed: Firmware package deployed successfully (100%).";
                    setState([] {});
                } else {
                    int pct = static_cast<int>(sim_progress_ * 100.0f);
                    loading_ctrl_->setProgress(sim_progress_, "Transferred " + std::to_string(pct) + "% of 24.5 MB");
                }
            } else if (sim_mode_ == "bar") {
                sim_progress_ = static_cast<float>(elapsed / 3.5); // 3.5 seconds
                if (sim_progress_ >= 1.0f) {
                    is_simulating_ = false;
                    loading_ctrl_->hide();
                    hud_msg_ = "Pipeline Completed: All Skia Vulkan shaders compiled successfully.";
                    setState([] {});
                } else {
                    std::string step = "Step 1 of 4: Parsing SPIR-V bytecodes...";
                    if (sim_progress_ > 0.75f) step = "Step 4 of 4: Linking GPU render pipelines...";
                    else if (sim_progress_ > 0.50f) step = "Step 3 of 4: Compiling Vulkan shader modules...";
                    else if (sim_progress_ > 0.25f) step = "Step 2 of 4: Optimizing instruction registers...";

                    loading_ctrl_->setProgress(sim_progress_, step);
                }
            }
        });
        sim_ticker_->start();
    }

    void dispose() override {
        if (sim_ticker_) sim_ticker_->stop();
        State::dispose();
    }

    WidgetPtr build(BuildContext&) override {
        // ── Main Page Header ──────────────────────────────────────────
        auto title = text("Advanced LoadingOverlay Feedback Suite");
        title->fontSize(22.0f).bold().color(0xFFFFFFFF);

        auto sub = text("Non-blocking page overlay, 4 indicator styles (Spinner, ProgressRing, ProgressBar, Dots), cancelable actions, and live tracking");
        sub->fontSize(13.0f).color(0xFF94A3B8);

        std::vector<WidgetPtr> title_items = {title, sub};
        auto title_col = column(title_items);
        title_col->alignItems(Align::Center).gap(StyleValue::point(6.0f));

        // ── Helper to build trigger cards ─────────────────────────────
        auto makeLoaderCard = [this](std::string icon, std::string title, std::string desc,
                                     std::string btn_label, Color btn_col, std::function<void()> cb) -> WidgetPtr {
            auto ic = text(icon);
            ic->fontSize(20.0f);

            auto tit = text(title);
            tit->fontSize(14.5f).bold().color(0xFFF1F5F9);

            std::vector<WidgetPtr> h_items = {ic, tit};
            auto h_row = row(h_items);
            h_row->gap(StyleValue::point(8.0f)).alignItems(Align::Center);

            auto ds = text(desc);
            ds->fontSize(12.0f).color(0xFF94A3B8);

            auto b_lbl = text(btn_label);
            b_lbl->fontSize(12.5f).bold().color(0xFFFFFFFF);

            auto b_box = container(b_lbl);
            b_box->color(btn_col)
                 .borderRadius(6.0f)
                 .paddingSymmetric(8.0f, 16.0f);

            auto gd = std::make_shared<GestureDetector>(b_box);
            gd->cursor_type = SystemCursor::Pointer;
            gd->on_tap_up = [cb](const TapUpDetails&) {
                if (cb) cb();
            };

            std::vector<WidgetPtr> c_items = {h_row, ds, gd};
            auto col = column(c_items);
            col->gap(StyleValue::point(12.0f));

            auto card = container(col);
            card->color(0xFF1E293B)
                .borderRadius(10.0f)
                .border(0xFF334155, 1.0f)
                .paddingAll(16.0f)
                .width(250.0f);
            return card;
        };

        // ── 4 Interactive Loader Triggers ─────────────────────────────
        auto card1 = makeLoaderCard("🔄", "Indeterminate Spinner", "Continuous dual-arc rotating spinner with cancel button.",
                                    "⚡ Launch Spinner", 0xFF0284C7, [this] {
            is_simulating_ = false;
            hud_msg_ = "Active: Indeterminate dual-arc spinner overlay.";
            loading_ctrl_->showSpinner("Connecting to Cluster...",
                                      "Handshaking with API node 'enki-prod-us-east'...",
                                      true, [this] {
                hud_msg_ = "Cancelled connection attempt.";
                setState([] {});
            });
            setState([] {});
        });

        auto card2 = makeLoaderCard("⭕", "Determinate ProgressRing", "Live circular progress ring with center % readout.",
                                    "📦 Deploy Package", 0xFF059669, [this] {
            is_simulating_ = true;
            sim_mode_ = "ring";
            sim_progress_ = 0.0f;
            sim_start_ = std::chrono::steady_clock::now();
            hud_msg_ = "Active: Live determinate ProgressRing (0% -> 100%).";

            loading_ctrl_->showProgressRing(0.0f, "Deploying Firmware...", "Initializing package stream...", true, [this] {
                is_simulating_ = false;
                hud_msg_ = "Cancelled firmware deployment.";
                setState([] {});
            });
            setState([] {});
        });

        auto card3 = makeLoaderCard("📊", "Linear Multi-Step Bar", "Multi-stage pipeline compilation progress bar.",
                                    "🚀 Compile Shaders", 0xFF7C3AED, [this] {
            is_simulating_ = true;
            sim_mode_ = "bar";
            sim_progress_ = 0.0f;
            sim_start_ = std::chrono::steady_clock::now();
            hud_msg_ = "Active: Multi-step shader compilation pipeline.";

            loading_ctrl_->showProgressBar(0.0f, "Compiling Shaders...", "Step 1 of 4: Parsing SPIR-V bytecodes...", true, [this] {
                is_simulating_ = false;
                hud_msg_ = "Cancelled shader compilation pipeline.";
                setState([] {});
            });
            setState([] {});
        });

        auto card4 = makeLoaderCard("⏳", "Pulsing Orbit Dots", "Rhythmic pulsating dots indicator for background sync.",
                                    "📡 Sync Workspace", 0xFFD97706, [this] {
            is_simulating_ = false;
            hud_msg_ = "Active: Pulsing OrbitDots sync indicator.";
            loading_ctrl_->showDots("Syncing Repository...", "Exchanging delta objects with remote git server...");
            setState([] {});
        });

        std::vector<WidgetPtr> cards_list = {card1, card2, card3, card4};
        auto cards_row = row(cards_list);
        cards_row->gap(StyleValue::point(14.0f)).justifyContent(Justify::Center);

        // ── Manual Dismiss Bar ────────────────────────────────────────
        auto btn_dismiss = button(text("✕ Dismiss Active Loading Overlay"), [this] {
            is_simulating_ = false;
            loading_ctrl_->hide();
            hud_msg_ = "Dismissed active loading overlay.";
            setState([] {});
        });

        std::vector<WidgetPtr> act_items = {btn_dismiss};
        auto act_row = row(act_items);
        act_row->justifyContent(Justify::Center);

        // ── HUD / Status Box ──────────────────────────────────────────
        auto hud_txt = text("💡 " + hud_msg_);
        hud_txt->fontSize(12.5f).color(0xFF38BDF8);

        auto hud_row = row(std::vector<WidgetPtr>{hud_txt});
        auto hud_box = container(hud_row);
        hud_box->color(0xFF1E293B)
               .borderRadius(6.0f)
               .border(0xFF334155, 1.0f)
               .paddingSymmetric(8.0f, 16.0f)
               .width(1080.0f);

        // ── Assemble Page Body ────────────────────────────────────────
        std::vector<WidgetPtr> page_items = {title_col, cards_row, act_row, hud_box};
        auto page_col = column(page_items);
        page_col->gap(StyleValue::point(26.0f)).alignItems(Align::Center);

        auto background_page = container(page_col);
        background_page->color(0xFF0B1120)
                       .paddingAll(24.0f)
                       .width(StyleValue::percent(100.0f))
                       .height(StyleValue::percent(100.0f));

        // Wrap with LoadingOverlay
        auto overlay = loadingOverlay(background_page, loading_ctrl_);
        return overlay;
    }
};

class LoadingOverlayDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<LoadingOverlayDemoState>();
    }
    std::string_view typeName() const override { return "LoadingOverlayDemoApp"; }
};

int main() {
    std::cout << "====================================================\n";
    std::cout << "  ENKI Engine — Advanced LoadingOverlay Demo\n";
    std::cout << "====================================================\n";

    AppConfig config;
    config.title       = "Enki Engine — Advanced LoadingOverlay Demo";
    config.width       = 1180;
    config.height      = 680;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 60;
    config.clear_color = 0xFF0B1120;

    return runApp(std::make_shared<LoadingOverlayDemoApp>(), config);
}
