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
        auto hdr = text("Share Document to Contacts", {
            .color = 0xFFF1F5F9,
            .font_size = 14.0f,
            .font_weight = FontWeight::Bold,
        });

        // Share grid items
        auto makeSharePill = [this](std::string icon, std::string label) -> WidgetPtr {
            auto ic = text(icon, { .font_size = 20.0f });
            auto lbl = text(label, { .color = 0xFFE2E8F0, .font_size = 11.5f });

            auto col = column({
                .align_items = Align::Center,
                .gap = StyleValue::point(4.0f),
                .children = {ic, lbl},
            });

            auto box = container({
                .color = 0xFF0F172A,
                .border_radius = BorderRadius::circular(8.0f),
                .border = Border(0xFF334155, 1.0f),
                .width = StyleValue::point(120.0f),
                .padding = StyleInsets::all(12.0f),
                .child = col,
            });

            return gestureDetector({
                .child = box,
                .cursor_type = SystemCursor::Pointer,
                .on_tap_up = [this, label](const TapUpDetails&) {
                    hud_msg_ = "Shared via " + label + "!";
                    sheet_ctrl_->hide();
                    setState([] {});
                },
            });
        };

        auto r1 = row({
            .justify_content = Justify::Center,
            .gap = StyleValue::point(10.0f),
            .children = {
                makeSharePill("📡", "AirDrop"),
                makeSharePill("✈️", "Telegram"),
                makeSharePill("💬", "Slack"),
                makeSharePill("✉️", "Email"),
            },
        });

        auto r2 = row({
            .justify_content = Justify::Center,
            .gap = StyleValue::point(10.0f),
            .children = {
                makeSharePill("🔗", "Copy Link"),
                makeSharePill("📝", "Notes"),
                makeSharePill("💾", "Save File"),
                makeSharePill("🔒", "Encrypt"),
            },
        });

        return column({
            .align_items = Align::Center,
            .gap = StyleValue::point(12.0f),
            .children = {hdr, r1, r2},
        });
    }

    WidgetPtr buildPlayerContent() {
        auto song_title = text("🎵 Synthwave Odyssey — Hyperion Core", {
            .color = 0xFFFFFFFF,
            .font_size = 15.0f,
            .font_weight = FontWeight::Bold,
        });

        auto artist = text("Artist: ENKI Audio Lab • Album: Cybernetic Horizons (2026)", {
            .color = 0xFF94A3B8,
            .font_size = 12.0f,
        });

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

        auto ctrl_row = row({
            .align_items = Align::Center,
            .gap = StyleValue::point(10.0f),
            .children = {btn_prev, btn_play, btn_next},
        });

        auto queue_lbl = text("Up Next in Queue (3 tracks):", {
            .color = 0xFF38BDF8,
            .font_size = 12.5f,
            .font_weight = FontWeight::Bold,
        });

        auto q1 = text("1. Quantum Flux — 03:42", { .color = 0xFFCBD5E1, .font_size = 12.0f });
        auto q2 = text("2. Neon Highway — 04:15", { .color = 0xFFCBD5E1, .font_size = 12.0f });

        return column({
            .align_items = Align::Center,
            .gap = StyleValue::point(10.0f),
            .children = {song_title, artist, ctrl_row, queue_lbl, q1, q2},
        });
    }

    WidgetPtr buildCheckoutContent() {
        auto title = text("🛍️ Enterprise Order Summary", {
            .color = 0xFFFFFFFF,
            .font_size = 15.0f,
            .font_weight = FontWeight::Bold,
        });

        auto sub = text("Invoice #ENKI-2026-94829 • Due immediately", {
            .color = 0xFF94A3B8,
            .font_size = 12.0f,
        });

        auto makeItemRow = [](std::string name, std::string price) -> WidgetPtr {
            auto n = text(name, { .color = 0xFFE2E8F0, .font_size = 12.5f });
            auto p = text(price, { .color = 0xFF38BDF8, .font_size = 12.5f, .font_weight = FontWeight::Bold });

            return row({
                .justify_content = Justify::SpaceBetween,
                .width = StyleValue::point(480.0f),
                .children = {n, p},
            });
        };

        return column({
            .align_items = Align::Center,
            .gap = StyleValue::point(10.0f),
            .children = {
                title, sub,
                makeItemRow("1. ENKI Enterprise Engine License (Perpetual)", "$ 1,250.00"),
                makeItemRow("2. Dedicated Skia GPU Acceleration Module", "$ 450.00"),
                makeItemRow("3. 24/7 Priority SLA Support Package", "$ 300.00"),
                makeItemRow("Total Due:", "$ 2,000.00"),
            },
        });
    }

public:
    void initState() override {
        State::initState();
        sheet_ctrl_ = std::make_shared<BottomSheetController>();
    }

    WidgetPtr build(BuildContext&) override {
        // Main Header
        auto title = text("Advanced BottomSheet Presentation Suite", {
            .color = 0xFFFFFFFF,
            .font_size = 22.0f,
            .font_weight = FontWeight::Bold,
        });

        auto sub = text("Modal & persistent presentation, multi-detent snapping (peek, half, full), drag physics, and scrim backdrop", {
            .color = 0xFF94A3B8,
            .font_size = 13.0f,
        });

        auto title_col = column({
            .align_items = Align::Center,
            .children = {title, sub},
        });

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

        auto btn_row = row({
            .justify_content = Justify::Center,
            .gap = StyleValue::point(12.0f),
            .children = {btn_share, btn_player, btn_checkout},
        });

        auto c_title = text("Interactive Presentation Controls", {
            .color = 0xFF38BDF8,
            .font_size = 14.5f,
            .font_weight = FontWeight::Bold,
        });

        auto c_desc = text("Click buttons below to present sheets with different detent snap points and content builders.", {
            .color = 0xFF94A3B8,
            .font_size = 12.0f,
        });

        auto trigger_card = container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(10.0f),
            .border = Border(0xFF334155, 1.0f),
            .width = StyleValue::point(900.0f),
            .padding = StyleInsets::all(20.0f),
            .child = column({
                .align_items = Align::Center,
                .gap = StyleValue::point(12.0f),
                .children = {c_title, c_desc, btn_row},
            }),
        });

        // ── Active BottomSheet Setup ──────────────────────────────────
        std::string sheet_title = "Share Options";
        std::string sheet_subtitle = "Send files, links and encrypted payloads to contacts";
        BottomSheetType s_type = BottomSheetType::Modal;

        if (current_demo_mode_ == "player") {
            s_type = BottomSheetType::Persistent;
            sheet_title = "Now Playing";
            sheet_subtitle = "Hi-Fi Skia Audio Stream • 320 kbps";
        } else if (current_demo_mode_ == "checkout") {
            s_type = BottomSheetType::Modal;
            sheet_title = "Checkout Review";
            sheet_subtitle = "Verify items and complete billing process";
        }

        WidgetPtr sheet_body;
        if (current_demo_mode_ == "share") {
            sheet_body = buildShareContent();
        } else if (current_demo_mode_ == "player") {
            sheet_body = buildPlayerContent();
        } else {
            sheet_body = buildCheckoutContent();
        }

        return BottomSheet {
            .sheet_content = sheet_body,
            .body = container({
                .color = 0xFF0B1120,
                .width = StyleValue::percent(100.0f),
                .height = StyleValue::percent(100.0f),
                .padding = StyleInsets::all(20.0f),
                .child = column({
                    .align_items = Align::Center,
                    .gap = StyleValue::point(16.0f),
                    .children = {
                        title_col,
                        trigger_card,
                        container({
                            .color = 0xFF1E293B,
                            .border_radius = BorderRadius::circular(6.0f),
                            .border = Border(0xFF334155, 1.0f),
                            .width = StyleValue::point(900.0f),
                            .padding = StyleInsets::symmetric(8.0f, 16.0f),
                            .child = row({
                                .children = { text("💡 " + hud_msg_, { .color = 0xFF38BDF8, .font_size = 12.0f }) }
                            })
                        })
                    }
                })
            }),
            .type = s_type,
            .show_drag_handle = true,
            .show_close_button = true,
            .title = sheet_title,
            .subtitle = sheet_subtitle,
            .on_closed = [this] {
                hud_msg_ = "BottomSheet dismissed.";
                setState([] {});
            },
            .on_detent_changed = [this](BottomSheetDetent d) {
                std::string d_str = "Half";
                if (d == BottomSheetDetent::Peek) d_str = "Peek";
                else if (d == BottomSheetDetent::Full) d_str = "Full";
                else if (d == BottomSheetDetent::Hidden) d_str = "Hidden";
                hud_msg_ = "Snapped to " + d_str + " detent.";
                setState([] {});
            },
            .controller = sheet_ctrl_,
        };
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
    config.target_fps  = 0;
    config.show_performance_overlay = true;
    config.clear_color = 0xFF0B1120;

    return runApp(std::make_shared<BottomSheetDemoApp>(), config);
}
