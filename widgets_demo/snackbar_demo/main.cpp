/// @file main.cpp
/// @brief ENKI Advanced In-Window Overlay Snackbar & Toast Interactive Demo.

#include "enki/app/app.hpp"
#include "enki/widgets/snackbar.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/stack.hpp"
#include "enki/state/state.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <memory>

using namespace enki;

class SnackbarDemoState : public State {
private:
    std::shared_ptr<SnackbarController> snackbar_ctrl_;
    SnackbarPlacement current_placement_ = SnackbarPlacement::BottomRight;
    std::string hud_msg_ = "Click any button below to trigger transient toast notifications with live progress countdowns.";
    int undo_count_ = 0;

public:
    void initState() override {
        State::initState();
        snackbar_ctrl_ = std::make_shared<SnackbarController>();
    }

    WidgetPtr build(BuildContext&) override {
        // ── Header ────────────────────────────────────────────────────
        auto title = text("Advanced In-Window Overlay Snackbar Suite", {
            .color = 0xFFFFFFFF,
            .font_size = 22.0f,
            .font_weight = FontWeight::Bold,
        });

        auto sub = text("Non-blocking floating toasts (Category 7. Overlays), auto-dismiss timers, countdown progress bar, and pause-on-hover", {
            .color = 0xFF94A3B8,
            .font_size = 13.0f,
        });

        auto title_col = column({
            .align_items = Align::Center,
            .gap = StyleValue::point(6.0f),
            .children = {title, sub}
        });

        // ── Helper: Action Buttons Grid ───────────────────────────────
        auto makeTriggerBtn = [this](std::string icon, std::string label, Color bg, Color border, std::function<void()> cb) -> WidgetPtr {
            auto ic = text(icon, { .font_size = 15.0f });
            auto lbl = text(label, { .color = 0xFFFFFFFF, .font_size = 12.5f, .font_weight = FontWeight::Bold });

            auto b_row = row({
                .justify_content = Justify::Center,
                .align_items = Align::Center,
                .gap = StyleValue::point(8.0f),
                .children = {ic, lbl}
            });

            auto box = container({
                .color = bg,
                .border_radius = BorderRadius::circular(8.0f),
                .border = Border(border, 1.0f),
                .width = StyleValue::point(220.0f),
                .padding = StyleInsets::symmetric(10.0f, 16.0f),
                .child = b_row
            });

            auto gd = std::make_shared<GestureDetector>(box);
            gd->cursor_type = SystemCursor::Pointer;
            gd->on_tap_up = [cb](const TapUpDetails&) {
                if (cb) cb();
            };
            return gd;
        };

        // ── 1. Semantic Feedback Buttons ──────────────────────────────
        auto btn_success = makeTriggerBtn("✅", "File Saved (with Undo)", 0xFF065F46, 0xFF10B981, [this] {
            hud_msg_ = "Triggered: Success Toast with interactive [Undo] action.";
            snackbar_ctrl_->showSuccess(
                "Document 'architecture_v2.enk' was saved to cloud storage.",
                "Saved Successfully",
                SnackbarAction("Undo", [this] {
                    undo_count_++;
                    hud_msg_ = "Action Executed: Reverted last file save (Undo count: " + std::to_string(undo_count_) + ").";
                    setState([] {});
                }),
                current_placement_
            );
            setState([] {});
        });

        auto btn_error = makeTriggerBtn("❌", "Network Error (Retry)", 0xFF881337, 0xFFEF4444, [this] {
            hud_msg_ = "Triggered: Error Toast with [Retry] button.";
            snackbar_ctrl_->showError(
                "Gateway timeout 504 while connecting to cluster 'us-east-1'.",
                "Connection Failed",
                SnackbarAction("Retry", [this] {
                    hud_msg_ = "Action Executed: Retrying network connection...";
                    setState([] {});
                }),
                current_placement_
            );
            setState([] {});
        });

        auto btn_warning = makeTriggerBtn("⚠️", "Storage Quota Warning", 0xFF78350F, 0xFFF59E0B, [this] {
            hud_msg_ = "Triggered: Storage Quota Warning Toast.";
            snackbar_ctrl_->showWarning(
                "High GPU VRAM consumption detected (92% of 24GB allocated).",
                "Resource Threshold",
                std::nullopt,
                current_placement_
            );
            setState([] {});
        });

        auto btn_info = makeTriggerBtn("ℹ️", "Engine Update Info", 0xFF0C4A6E, 0xFF38BDF8, [this] {
            hud_msg_ = "Triggered: Informational Toast Notification.";
            snackbar_ctrl_->showInfo(
                "ENKI Skia Compositor v0.2.0 is now available for download.",
                "Update Available",
                SnackbarAction("Learn More", [this] {
                    hud_msg_ = "Action Executed: Opened release changelog.";
                    setState([] {});
                }),
                current_placement_
            );
            setState([] {});
        });

        auto r1 = row({
            .justify_content = Justify::Center,
            .gap = StyleValue::point(14.0f),
            .children = {btn_success, btn_error}
        });

        auto r2 = row({
            .justify_content = Justify::Center,
            .gap = StyleValue::point(14.0f),
            .children = {btn_warning, btn_info}
        });

        auto triggers_col = column({
            .align_items = Align::Center,
            .gap = StyleValue::point(12.0f),
            .children = {r1, r2}
        });

        auto c1_title = text("1. Interactive Feedback Trigger Actions", {
            .color = 0xFF38BDF8,
            .font_size = 14.5f,
            .font_weight = FontWeight::Bold,
        });

        auto c1_sub = text("Click buttons below to launch rich snackbars. Hover mouse over the active card to pause the timer!", {
            .color = 0xFF94A3B8,
            .font_size = 12.0f,
        });

        auto trigger_card = container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(10.0f),
            .border = Border(0xFF334155, 1.0f),
            .width = StyleValue::point(580.0f),
            .padding = StyleInsets::all(20.0f),
            .child = column({
                .align_items = Align::Center,
                .gap = StyleValue::point(14.0f),
                .children = {c1_title, c1_sub, triggers_col}
            })
        });

        // ── 2. Placement Switcher Card ────────────────────────────────
        auto c2_title = text("2. Viewport Placement Configuration", {
            .color = 0xFF10B981,
            .font_size = 14.5f,
            .font_weight = FontWeight::Bold,
        });

        auto c2_sub = text("Select screen alignment for incoming toast notifications:", {
            .color = 0xFF94A3B8,
            .font_size = 12.0f,
        });

        auto makePosPill = [this](std::string label, SnackbarPlacement p) -> WidgetPtr {
            bool is_active = (current_placement_ == p);
            auto t_lbl = text(label, {
                .color = is_active ? 0xFFFFFFFF : 0xFF94A3B8,
                .font_size = 12.0f,
                .font_weight = is_active ? FontWeight::Bold : FontWeight::Normal,
            });

            auto box = container({
                .color = is_active ? 0xFF0284C7 : 0xFF0F172A,
                .border_radius = BorderRadius::circular(6.0f),
                .border = Border(is_active ? 0xFF38BDF8 : 0xFF334155, 1.0f),
                .padding = StyleInsets::symmetric(6.0f, 14.0f),
                .child = t_lbl
            });

            auto gd = std::make_shared<GestureDetector>(box);
            gd->cursor_type = SystemCursor::Pointer;
            gd->on_tap_up = [this, p, label](const TapUpDetails&) {
                current_placement_ = p;
                hud_msg_ = "Changed toast placement to: " + label;
                setState([] {});
            };
            return gd;
        };

        auto pos_r1 = row({
            .justify_content = Justify::Center,
            .gap = StyleValue::point(8.0f),
            .children = {
                makePosPill("Top-Left", SnackbarPlacement::TopLeft),
                makePosPill("Top-Center", SnackbarPlacement::TopCenter),
                makePosPill("Top-Right (Toast)", SnackbarPlacement::TopRight)
            }
        });

        auto pos_r2 = row({
            .justify_content = Justify::Center,
            .gap = StyleValue::point(8.0f),
            .children = {
                makePosPill("Bottom-Left", SnackbarPlacement::BottomLeft),
                makePosPill("Bottom-Center", SnackbarPlacement::BottomCenter),
                makePosPill("Bottom-Right (Desktop)", SnackbarPlacement::BottomRight)
            }
        });

        auto pos_card = container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(10.0f),
            .border = Border(0xFF334155, 1.0f),
            .width = StyleValue::point(480.0f),
            .padding = StyleInsets::all(20.0f),
            .child = column({
                .align_items = Align::Center,
                .gap = StyleValue::point(12.0f),
                .children = {c2_title, c2_sub, pos_r1, pos_r2}
            })
        });

        // ── Wrap with Snackbar Overlay ────────────────────────────────
        return Snackbar {
            .body = container({
                .color = 0xFF0B1120,
                .width = StyleValue::percent(100.0f),
                .height = StyleValue::percent(100.0f),
                .padding = StyleInsets::all(24.0f),
                .child = column({
                    .align_items = Align::Center,
                    .gap = StyleValue::point(24.0f),
                    .children = {
                        title_col,
                        row({
                            .justify_content = Justify::Center,
                            .gap = StyleValue::point(16.0f),
                            .children = { trigger_card, pos_card }
                        }),
                        container({
                            .color = 0xFF1E293B,
                            .border_radius = BorderRadius::circular(6.0f),
                            .border = Border(0xFF334155, 1.0f),
                            .width = StyleValue::point(1076.0f),
                            .padding = StyleInsets::symmetric(8.0f, 16.0f),
                            .child = row({
                                .children = { text("💡 " + hud_msg_, { .color = 0xFF38BDF8, .font_size = 12.5f }) }
                            })
                        })
                    }
                })
            }),
            .controller = snackbar_ctrl_
        };
    }
};

class SnackbarDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<SnackbarDemoState>();
    }
    std::string_view typeName() const override { return "SnackbarDemoApp"; }
};

int main() {
    std::cout << "====================================================\n";
    std::cout << "  ENKI Engine — Advanced Snackbar & Toast Overlay Demo\n";
    std::cout << "====================================================\n";

    AppConfig config;
    config.title       = "Enki Engine — Advanced Snackbar & Toast Overlay Demo";
    config.width       = 1180;
    config.height      = 680;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 0;
    config.show_performance_overlay = true;
    config.clear_color = 0xFF0B1120;

    return runApp(std::make_shared<SnackbarDemoApp>(), config);
}
