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
        auto title = text("Advanced In-Window Overlay Snackbar Suite");
        title->fontSize(22.0f).bold().color(0xFFFFFFFF);

        auto sub = text("Non-blocking floating toasts (Category 7. Overlays), auto-dismiss timers, countdown progress bar, and pause-on-hover");
        sub->fontSize(13.0f).color(0xFF94A3B8);

        std::vector<WidgetPtr> title_items = {title, sub};
        auto title_col = column(title_items);
        title_col->alignItems(Align::Center).gap(StyleValue::point(6.0f));

        // ── Helper: Action Buttons Grid ───────────────────────────────
        auto makeTriggerBtn = [this](std::string icon, std::string label, Color bg, Color border, std::function<void()> cb) -> WidgetPtr {
            auto ic = text(icon);
            ic->fontSize(15.0f);

            auto lbl = text(label);
            lbl->fontSize(12.5f).bold().color(0xFFFFFFFF);

            std::vector<WidgetPtr> b_items = {ic, lbl};
            auto b_row = row(b_items);
            b_row->gap(StyleValue::point(8.0f)).alignItems(Align::Center).justifyContent(Justify::Center);

            auto box = container(b_row);
            box->color(bg)
               .border(border, 1.0f)
               .borderRadius(8.0f)
               .paddingSymmetric(10.0f, 16.0f)
               .width(220.0f);

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

        std::vector<WidgetPtr> trigger_row1 = {btn_success, btn_error};
        auto r1 = row(trigger_row1);
        r1->gap(StyleValue::point(14.0f)).justifyContent(Justify::Center);

        std::vector<WidgetPtr> trigger_row2 = {btn_warning, btn_info};
        auto r2 = row(trigger_row2);
        r2->gap(StyleValue::point(14.0f)).justifyContent(Justify::Center);

        std::vector<WidgetPtr> triggers_col_items = {r1, r2};
        auto triggers_col = column(triggers_col_items);
        triggers_col->gap(StyleValue::point(12.0f)).alignItems(Align::Center);

        auto c1_title = text("1. Interactive Feedback Trigger Actions");
        c1_title->fontSize(14.5f).bold().color(0xFF38BDF8);

        auto c1_sub = text("Click buttons below to launch rich snackbars. Hover mouse over the active card to pause the timer!");
        c1_sub->fontSize(12.0f).color(0xFF94A3B8);

        std::vector<WidgetPtr> card1_items = {c1_title, c1_sub, triggers_col};
        auto card1_col = column(card1_items);
        card1_col->gap(StyleValue::point(14.0f)).alignItems(Align::Center);

        auto trigger_card = container(card1_col);
        trigger_card->color(0xFF1E293B)
                    .borderRadius(10.0f)
                    .border(0xFF334155, 1.0f)
                    .paddingAll(20.0f)
                    .width(580.0f);

        // ── 2. Placement Switcher Card ────────────────────────────────
        auto c2_title = text("2. Viewport Placement Configuration");
        c2_title->fontSize(14.5f).bold().color(0xFF10B981);

        auto c2_sub = text("Select screen alignment for incoming toast notifications:");
        c2_sub->fontSize(12.0f).color(0xFF94A3B8);

        auto makePosPill = [this](std::string label, SnackbarPlacement p) -> WidgetPtr {
            bool is_active = (current_placement_ == p);
            auto t_lbl = text(label);
            t_lbl->fontSize(12.0f).color(is_active ? 0xFFFFFFFF : 0xFF94A3B8);
            if (is_active) t_lbl->bold();

            auto box = container(t_lbl);
            box->color(is_active ? 0xFF0284C7 : 0xFF0F172A)
               .border(is_active ? 0xFF38BDF8 : 0xFF334155, 1.0f)
               .borderRadius(6.0f)
               .paddingSymmetric(6.0f, 14.0f);

            auto gd = std::make_shared<GestureDetector>(box);
            gd->cursor_type = SystemCursor::Pointer;
            gd->on_tap_up = [this, p, label](const TapUpDetails&) {
                current_placement_ = p;
                hud_msg_ = "Changed toast placement to: " + label;
                setState([] {});
            };
            return gd;
        };

        std::vector<WidgetPtr> pos_row1_items = {
            makePosPill("Top-Left", SnackbarPlacement::TopLeft),
            makePosPill("Top-Center", SnackbarPlacement::TopCenter),
            makePosPill("Top-Right (Toast)", SnackbarPlacement::TopRight)
        };
        auto pos_r1 = row(pos_row1_items);
        pos_r1->gap(StyleValue::point(8.0f)).justifyContent(Justify::Center);

        std::vector<WidgetPtr> pos_row2_items = {
            makePosPill("Bottom-Left", SnackbarPlacement::BottomLeft),
            makePosPill("Bottom-Center", SnackbarPlacement::BottomCenter),
            makePosPill("Bottom-Right (Desktop)", SnackbarPlacement::BottomRight)
        };
        auto pos_r2 = row(pos_row2_items);
        pos_r2->gap(StyleValue::point(8.0f)).justifyContent(Justify::Center);

        std::vector<WidgetPtr> card2_items = {c2_title, c2_sub, pos_r1, pos_r2};
        auto card2_col = column(card2_items);
        card2_col->gap(StyleValue::point(12.0f)).alignItems(Align::Center);

        auto pos_card = container(card2_col);
        pos_card->color(0xFF1E293B)
                .borderRadius(10.0f)
                .border(0xFF334155, 1.0f)
                .paddingAll(20.0f)
                .width(480.0f);

        // ── Main Page Layout ──────────────────────────────────────────
        std::vector<WidgetPtr> cards_list = {trigger_card, pos_card};
        auto cards_row = row(cards_list);
        cards_row->gap(StyleValue::point(16.0f)).justifyContent(Justify::Center);

        // HUD / Status Box
        auto hud_txt = text("💡 " + hud_msg_);
        hud_txt->fontSize(12.5f).color(0xFF38BDF8);

        auto hud_row = row(std::vector<WidgetPtr>{hud_txt});
        auto hud_box = container(hud_row);
        hud_box->color(0xFF1E293B)
               .borderRadius(6.0f)
               .border(0xFF334155, 1.0f)
               .paddingSymmetric(8.0f, 16.0f)
               .width(1076.0f);

        std::vector<WidgetPtr> page_items = {title_col, cards_row, hud_box};
        auto page_col = column(page_items);
        page_col->gap(StyleValue::point(24.0f)).alignItems(Align::Center);

        auto background_page = container(page_col);
        background_page->color(0xFF0B1120)
                       .paddingAll(24.0f)
                       .width(StyleValue::percent(100.0f))
                       .height(StyleValue::percent(100.0f));

        // ── Wrap with Snackbar Overlay ────────────────────────────────
        auto snackbar_overlay = snackbar(background_page, snackbar_ctrl_);
        return snackbar_overlay;
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
    config.target_fps  = 60;
    config.clear_color = 0xFF0B1120;

    return runApp(std::make_shared<SnackbarDemoApp>(), config);
}
