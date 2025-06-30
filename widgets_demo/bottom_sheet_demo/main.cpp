/// @file main.cpp
/// @brief ENKI Advanced BottomSheet Interactive Showcase Demo.

#include "enki/app/app.hpp"
#include "enki/widgets/bottom_sheet.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/stack.hpp"
#include "enki/state/state.hpp"

#include "enki/widgets/gesture_detector.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <memory>

using namespace enki;

class BottomSheetDemoState : public State {
private:
    std::shared_ptr<BottomSheetController> sheet_ctrl_;
    std::string current_demo_mode_ = "share"; // "share", "player", "checkout"
    std::string hud_msg_ = "Click any button to present an interactive BottomSheet with drag-to-dismiss & detents.";

    WidgetPtr buildShareContent() {
        auto hdr = text("Share Document to Contacts");
        hdr->fontSize(14.0f).bold().color(0xFFF1F5F9);

        // Share grid items
        auto makeSharePill = [this](std::string icon, std::string label) -> WidgetPtr {
            auto ic = text(icon);
            ic->fontSize(20.0f);

            auto lbl = text(label);
            lbl->fontSize(11.5f).color(0xFFE2E8F0);

            std::vector<WidgetPtr> items = {ic, lbl};
            auto col = column(items);
            col->gap(StyleValue::point(4.0f)).alignItems(Align::Center);

            auto box = container(col);
            box->color(0xFF0F172A)
               .borderRadius(8.0f)
               .border(0xFF334155, 1.0f)
               .paddingAll(12.0f)
               .width(120.0f);

            auto gd = std::make_shared<GestureDetector>(box);
            gd->cursor_type = SystemCursor::Pointer;
            gd->on_tap_up = [this, label](const TapUpDetails&) {
                hud_msg_ = "Shared via " + label + "!";
                sheet_ctrl_->hide();
                setState([] {});
            };
            return gd;
        };

        std::vector<WidgetPtr> row1_items = {
            makeSharePill("📡", "AirDrop"),
            makeSharePill("✈️", "Telegram"),
            makeSharePill("💬", "Slack"),
            makeSharePill("✉️", "Email")
        };
        auto r1 = row(row1_items);
        r1->gap(StyleValue::point(10.0f)).justifyContent(Justify::Center);

        std::vector<WidgetPtr> row2_items = {
            makeSharePill("🔗", "Copy Link"),
            makeSharePill("📝", "Notes"),
            makeSharePill("💾", "Save File"),
            makeSharePill("🔒", "Encrypt")
        };
        auto r2 = row(row2_items);
        r2->gap(StyleValue::point(10.0f)).justifyContent(Justify::Center);

        std::vector<WidgetPtr> col_items = {hdr, r1, r2};
        auto main_col = column(col_items);
        main_col->gap(StyleValue::point(12.0f)).alignItems(Align::Center);

        return main_col;
    }

    WidgetPtr buildPlayerContent() {
        auto song_title = text("🎵 Synthwave Odyssey — Hyperion Core");
        song_title->fontSize(15.0f).bold().color(0xFFFFFFFF);

        auto artist = text("Artist: ENKI Audio Lab • Album: Cybernetic Horizons (2026)");
        artist->fontSize(12.0f).color(0xFF94A3B8);

        // Control buttons
        auto btn_prev = button(text("⏮ Prev"), [this] {
            hud_msg_ = "Playing previous track";
            setState([] {});
        });

        auto btn_play = button(text("▶ Play / Pause"), [this] {
            hud_msg_ = "Toggled playback state";
            setState([] {});
        });

        auto btn_next = button(text("Next ⏭"), [this] {
            hud_msg_ = "Playing next track";
            setState([] {});
        });

        std::vector<WidgetPtr> ctrl_items = {btn_prev, btn_play, btn_next};
        auto ctrl_row = row(ctrl_items);
        ctrl_row->gap(StyleValue::point(10.0f)).alignItems(Align::Center);

        auto queue_lbl = text("Up Next in Queue (3 tracks):");
        queue_lbl->fontSize(12.5f).bold().color(0xFF38BDF8);

        auto q1 = text("1. Quantum Flux — 03:42");
        q1->fontSize(12.0f).color(0xFFCBD5E1);

        auto q2 = text("2. Neon Highway — 04:15");
        q2->fontSize(12.0f).color(0xFFCBD5E1);

        std::vector<WidgetPtr> col_items = {song_title, artist, ctrl_row, queue_lbl, q1, q2};
        auto main_col = column(col_items);
        main_col->gap(StyleValue::point(10.0f)).alignItems(Align::Center);

        return main_col;
    }

    WidgetPtr buildCheckoutContent() {
        auto title = text("🛍️ Enterprise Order Summary");
        title->fontSize(15.0f).bold().color(0xFFFFFFFF);

        auto sub = text("Invoice #ENKI-2026-94829 • Due immediately");
        sub->fontSize(12.0f).color(0xFF94A3B8);

        auto makeItemRow = [](std::string name, std::string price) -> WidgetPtr {
            auto n = text(name);
            n->fontSize(12.5f).color(0xFFE2E8F0);

            auto p = text(price);
            p->fontSize(12.5f).bold().color(0xFF38BDF8);

            std::vector<WidgetPtr> items = {n, p};
            auto r = row(items);
            r->justifyContent(Justify::SpaceBetween).width(480.0f);
            return r;
        };

        std::vector<WidgetPtr> items_list = {
            title, sub,
            makeItemRow("1. ENKI Enterprise Engine License (Perpetual)", "$ 1,250.00"),
            makeItemRow("2. Dedicated Skia GPU Acceleration Module", "$ 450.00"),
            makeItemRow("3. 24/7 Priority SLA Support Package", "$ 300.00"),
            makeItemRow("Total Due:", "$ 2,000.00")
        };

        auto col = column(items_list);
        col->gap(StyleValue::point(10.0f)).alignItems(Align::Center);

        return col;
    }

public:
    void initState() override {
        State::initState();
        sheet_ctrl_ = std::make_shared<BottomSheetController>();
    }

    WidgetPtr build(BuildContext&) override {
        // Main Header
        auto title = text("Advanced BottomSheet Presentation Suite");
        title->fontSize(22.0f).bold().color(0xFFFFFFFF);

        auto sub = text("Modal & persistent presentation, multi-detent snapping (peek, half, full), drag physics, and scrim backdrop");
        sub->fontSize(13.0f).color(0xFF94A3B8);

        std::vector<WidgetPtr> title_items = {title, sub};
        auto title_col = column(title_items);
        title_col->alignItems(Align::Center);

        // ── Triggers Grid Card ────────────────────────────────────────
        auto btn_share = button(text("🚀 Open Share Sheet (Modal Half 50%)"), [this] {
            current_demo_mode_ = "share";
            sheet_ctrl_->show(BottomSheetDetent::Half);
            hud_msg_ = "Opened Share Sheet at 50% height. Drag up for full or down to dismiss.";
            setState([] {});
        });

        auto btn_player = button(text("🎵 Open Media Player (Peek 15%)"), [this] {
            current_demo_mode_ = "player";
            sheet_ctrl_->show(BottomSheetDetent::Peek);
            hud_msg_ = "Opened Mini-Player at Peek height. Drag handle up to expand.";
            setState([] {});
        });

        auto btn_checkout = button(text("🛍️ Open Order Summary (Full 88%)"), [this] {
            current_demo_mode_ = "checkout";
            sheet_ctrl_->show(BottomSheetDetent::Full);
            hud_msg_ = "Opened Order Summary at Full height.";
            setState([] {});
        });

        std::vector<WidgetPtr> btn_list = {btn_share, btn_player, btn_checkout};
        auto btn_row = row(btn_list);
        btn_row->gap(StyleValue::point(12.0f)).justifyContent(Justify::Center);

        auto c_title = text("Interactive Presentation Controls");
        c_title->fontSize(14.5f).bold().color(0xFF38BDF8);

        auto c_desc = text("Click buttons below to present sheets with different detent snap points and content builders.");
        c_desc->fontSize(12.0f).color(0xFF94A3B8);

        std::vector<WidgetPtr> card_items = {c_title, c_desc, btn_row};
        auto card_col = column(card_items);
        card_col->gap(StyleValue::point(12.0f)).alignItems(Align::Center);

        auto trigger_card = container(card_col);
        trigger_card->color(0xFF1E293B)
                    .borderRadius(10.0f)
                    .border(0xFF334155, 1.0f)
                    .paddingAll(20.0f)
                    .width(900.0f);

        // HUD / Status
        auto hud_txt = text("💡 " + hud_msg_);
        hud_txt->fontSize(12.0f).color(0xFF38BDF8);

        std::vector<WidgetPtr> hud_items = {hud_txt};
        auto hud_row = row(hud_items);
        auto hud_box = container(hud_row);
        hud_box->color(0xFF1E293B)
               .borderRadius(6.0f)
               .border(0xFF334155, 1.0f)
               .paddingSymmetric(8.0f, 16.0f)
               .width(900.0f);

        // Page Main Content
        std::vector<WidgetPtr> page_items = {title_col, trigger_card, hud_box};
        auto page_col = column(page_items);
        page_col->gap(StyleValue::point(16.0f)).alignItems(Align::Center);

        auto background_page = container(page_col);
        background_page->color(0xFF0B1120)
                       .paddingAll(20.0f)
                       .width(StyleValue::percent(100.0f))
                       .height(StyleValue::percent(100.0f));

        // ── Active BottomSheet Setup ──────────────────────────────────
        BottomSheetOptions sheet_opts;
        sheet_opts.type = (current_demo_mode_ == "player") ? BottomSheetType::Persistent : BottomSheetType::Modal;
        sheet_opts.show_drag_handle = true;
        sheet_opts.show_close_button = true;

        if (current_demo_mode_ == "share") {
            sheet_opts.title = "Share Options";
            sheet_opts.subtitle = "Send files, links and encrypted payloads to contacts";
        } else if (current_demo_mode_ == "player") {
            sheet_opts.title = "Now Playing";
            sheet_opts.subtitle = "Hi-Fi Skia Audio Stream • 320 kbps";
        } else {
            sheet_opts.title = "Checkout Review";
            sheet_opts.subtitle = "Verify items and complete billing process";
        }

        sheet_opts.on_closed = [this] {
            hud_msg_ = "BottomSheet dismissed.";
            setState([] {});
        };

        sheet_opts.on_detent_changed = [this](BottomSheetDetent d) {
            std::string d_str = "Half";
            if (d == BottomSheetDetent::Peek) d_str = "Peek";
            else if (d == BottomSheetDetent::Full) d_str = "Full";
            else if (d == BottomSheetDetent::Hidden) d_str = "Hidden";
            hud_msg_ = "Snapped to " + d_str + " detent.";
            setState([] {});
        };

        WidgetPtr sheet_body;
        if (current_demo_mode_ == "share") {
            sheet_body = buildShareContent();
        } else if (current_demo_mode_ == "player") {
            sheet_body = buildPlayerContent();
        } else {
            sheet_body = buildCheckoutContent();
        }

        auto active_sheet = bottomSheet(sheet_body, background_page, sheet_opts);
        active_sheet->setController(sheet_ctrl_);

        return active_sheet;
    }
};

class BottomSheetDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<BottomSheetDemoState>();
    }
    std::string_view typeName() const override { return "BottomSheetDemoApp"; }
};

int main() {
    std::cout << "====================================================\n";
    std::cout << "  ENKI Engine — Advanced BottomSheet Widget Demo\n";
    std::cout << "====================================================\n";

    AppConfig config;
    config.title       = "Enki Engine — Advanced BottomSheet Demo";
    config.width       = 1140;
    config.height      = 720;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 60;
    config.clear_color = 0xFF0B1120;

    return runApp(std::make_shared<BottomSheetDemoApp>(), config);
}
