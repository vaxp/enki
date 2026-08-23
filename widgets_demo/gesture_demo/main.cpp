/// @file main.cpp
/// @brief ENKI GestureDetector & Interaction Showcase.
/// Demonstrates Tap, Double-Tap, Secondary (Right) Click, Long Press, 2D Pan/Drag,
/// Sliders, System Cursors, HitTestBehaviors, and Live Event Logging.

#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/state/state.hpp"
#include "enki/rendering/canvas.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <sstream>
#include <iomanip>
#include <algorithm>

using namespace enki;

// ════════════════════════════════════════════════════════════════
// Design Tokens & Colors
// ════════════════════════════════════════════════════════════════

namespace Style {
    constexpr uint32_t bg_main        = 0xFF0D0F18;
    constexpr uint32_t bg_sidebar     = 0xFF121522;
    constexpr uint32_t bg_card        = 0xFF181C2E;
    constexpr uint32_t bg_card_hover  = 0xFF20263E;
    constexpr uint32_t bg_input       = 0xFF0F111C;
    constexpr uint32_t primary        = 0xFF7C4DFF;
    constexpr uint32_t primary_light  = 0xFF9E7AFF;
    constexpr uint32_t secondary      = 0xFF00E5FF;
    constexpr uint32_t accent_pink    = 0xFFFF4081;
    constexpr uint32_t accent_amber   = 0xFFFFAB00;
    constexpr uint32_t accent_green   = 0xFF00E676;
    constexpr uint32_t text_primary   = 0xFFF5F7FF;
    constexpr uint32_t text_secondary = 0xFF8C94B2;
    constexpr uint32_t text_muted     = 0xFF5A6280;
    constexpr uint32_t border_subtle  = 0x28FFFFFF;
    constexpr uint32_t border_active  = 0xA07C4DFF;
}

// ════════════════════════════════════════════════════════════════
// Demo Application State
// ════════════════════════════════════════════════════════════════

class GestureDemoState : public State {
public:
    int current_tab = 0; // 0: Taps & Clicks, 1: Pan & Drag, 2: Cursors, 3: Hit Testing

    // ── Tab 0: Taps & Clicks State ─────────────────────────────
    int  single_tap_count     = 0;
    int  double_tap_count     = 0;
    int  secondary_tap_count  = 0;
    int  long_press_count     = 0;
    std::string last_action_message = "Interact with any target below to start testing gestures.";
    std::vector<std::string> event_logs;

    // ── Tab 1: Pan & Drag State ────────────────────────────────
    float card_offset_x = 0.0f;
    float card_offset_y = 0.0f;
    float card_velocity_x = 0.0f;
    float card_velocity_y = 0.0f;
    bool  is_dragging_card = false;

    float slider_value = 0.45f; // 0.0 to 1.0

    // ── Tab 2: Cursors State ───────────────────────────────────
    std::string hovered_cursor_name = "Default (Arrow)";

    // ── Tab 3: Hit Testing State ───────────────────────────────
    int parent_hits = 0;
    int child_hits = 0;
    HitTestBehavior current_behavior = HitTestBehavior::Opaque;

    void logEvent(const std::string& msg) {
        std::ostringstream ss;
        ss << "[" << event_logs.size() + 1 << "] " << msg;
        event_logs.insert(event_logs.begin(), ss.str());
        if (event_logs.size() > 7) {
            event_logs.pop_back();
        }
        last_action_message = msg;
    }

    WidgetPtr build(BuildContext& context) override;

private:
    WidgetPtr buildSidebar();
    WidgetPtr buildHeader();
    WidgetPtr buildTapsTab();
    WidgetPtr buildPanTab();
    WidgetPtr buildCursorsTab();
    WidgetPtr buildHitTestTab();
};

// ════════════════════════════════════════════════════════════════
// Gesture Demo StatefulWidget
// ════════════════════════════════════════════════════════════════

class GestureDemoApp : public StatefulWidget {
public:
    GestureDemoApp() : StatefulWidget(Key::string("gesture_demo_app")) {}
    std::unique_ptr<State> createState() override {
        return std::make_unique<GestureDemoState>();
    }
    [[nodiscard]] std::string_view typeName() const override { return "GestureDemoApp"; }
};

// ════════════════════════════════════════════════════════════════
// Sidebar & Navigation
// ════════════════════════════════════════════════════════════════

WidgetPtr GestureDemoState::buildSidebar() {
    struct TabItem {
        std::string title;
        std::string subtitle;
        std::string icon;
    };

    const std::vector<TabItem> tabs = {
        {"Taps & Clicks", "Tap, Double, Right, Long Press", "🎯"},
        {"Pan & Drag", "2D Canvas & 1D Sliders", "🖐️"},
        {"System Cursors", "Dynamic Cursor Gallery", "🖱️"},
        {"Hit Testing", "Opaque vs Defer Behaviors", "🌀"},
    };

    std::vector<WidgetPtr> tab_widgets;

    // Header Logo
    auto t1 = text({
        .text = "ENKI ENGINE",
        .color = Style::primary_light,
        .font_size = 11.0f,
        .font_weight = FontWeight::Bold,
    });
    auto t2 = text({
        .text = "Gesture Detector",
        .color = Style::text_primary,
        .font_size = 18.0f,
        .font_weight = FontWeight::Bold,
    });
    auto t3 = text({
        .text = "Full Pointer & Touch Suite",
        .color = Style::text_secondary,
        .font_size = 11.0f,
    });

    auto logo_col = column({
        t1,
        sizedBox(0, 4.0f),
        t2,
        sizedBox(0, 2.0f),
        t3,
    });

    auto logo_card = container(logo_col);
    logo_card->paddingAll(18.0f)
             .margin(EdgeInsets::only(0, 0, 16.0f, 0))
             .color(0x287C4DFF)
             .borderRadius(12.0f)
             .border(0x507C4DFF, 1.0f);

    tab_widgets.push_back(logo_card);

    // Tab Buttons
    for (size_t i = 0; i < tabs.size(); ++i) {
        bool is_active = (current_tab == static_cast<int>(i));

        uint32_t bg_color   = is_active ? Style::primary : 0x00000000;
        uint32_t text_col   = is_active ? 0xFFFFFFFF : Style::text_primary;
        uint32_t subtext_col= is_active ? 0xDCFFFFFF : Style::text_secondary;
        uint32_t border_col = is_active ? Style::primary_light : Style::border_subtle;

        auto tab_title = text({
            .text = tabs[i].title,
            .color = text_col,
            .font_size = 13.5f,
            .font_weight = FontWeight::Bold,
        });

        auto tab_sub = text({
            .text = tabs[i].subtitle,
            .color = subtext_col,
            .font_size = 10.5f,
        });

        auto tab_icon = text({
            .text = tabs[i].icon,
            .font_size = 18.0f,
        });

        auto tab_text_col = column({
            tab_title,
            sizedBox(0, 2.0f),
            tab_sub,
        });

        auto tab_row = row(Justify::Start, Align::Center, {
            tab_icon,
            sizedBox(12.0f, 0),
            tab_text_col,
        });

        auto btn_content = container(tab_row);
        btn_content->padding(EdgeInsets::symmetric(12.0f, 14.0f))
                   .margin(EdgeInsets::only(0, 0, 8.0f, 0))
                   .color(bg_color)
                   .borderRadius(10.0f)
                   .border(border_col, 1.0f);

        int tab_index = static_cast<int>(i);
        auto gd = gestureDetector(Key::string("nav_tab_" + std::to_string(i)), btn_content);
        gd->onTap([this, tab_index]() {
            setState([this, tab_index]() {
                current_tab = tab_index;
            });
        }).cursor(SystemCursor::Pointer);

        tab_widgets.push_back(gd);
    }

    // Engine Info Footer
    auto status_title = text({
        .text = "Engine Status",
        .color = Style::accent_green,
        .font_size = 11.0f,
        .font_weight = FontWeight::Bold,
    });

    auto status_desc = text({
        .text = "• Zero-allocation recognizers\n• Desktop & Touch Ready\n• Sub-pixel Delta Tracking",
        .color = Style::text_secondary,
        .font_size = 10.0f,
    });

    auto status_col = column({
        status_title,
        sizedBox(0, 4.0f),
        status_desc,
    });

    auto status_card = container(status_col);
    status_card->margin(EdgeInsets::only(20.0f, 0, 0, 0))
               .paddingAll(12.0f)
               .color(0x18FFFFFF)
               .borderRadius(8.0f);

    tab_widgets.push_back(status_card);

    auto sidebar_col = column(std::move(tab_widgets));
    auto sidebar_box = container(sidebar_col);
    sidebar_box->width(270.0f)
               .paddingAll(16.0f)
               .color(Style::bg_sidebar)
               .border(Style::border_subtle, 1.0f);

    return sidebar_box;
}

// ════════════════════════════════════════════════════════════════
// Header & Live Status Bar
// ════════════════════════════════════════════════════════════════

WidgetPtr GestureDemoState::buildHeader() {
    auto header_title = text({
        .text = "Active Feedback Stream",
        .color = Style::secondary,
        .font_size = 11.5f,
        .font_weight = FontWeight::Bold,
    });

    auto header_msg = text({
        .text = last_action_message,
        .color = Style::text_primary,
        .font_size = 14.0f,
        .font_weight = FontWeight::Bold,
    });

    auto title_col = column({
        header_title,
        sizedBox(0, 3.0f),
        header_msg,
    });

    auto badge_text = text({
        .text = "● LIVE POINTER",
        .color = Style::accent_green,
        .font_size = 10.5f,
        .font_weight = FontWeight::Bold,
    });

    auto badge = container(badge_text);
    badge->padding(EdgeInsets::symmetric(6.0f, 14.0f))
         .color(0x2000E676)
         .borderRadius(20.0f)
         .border(Style::accent_green, 1.0f);

    auto header_row = row(Justify::SpaceBetween, Align::Center, {
        title_col,
        badge,
    });

    auto header_card = container(header_row);
    header_card->padding(EdgeInsets::symmetric(16.0f, 20.0f))
               .margin(EdgeInsets::only(0, 0, 16.0f, 0))
               .color(Style::bg_card)
               .borderRadius(12.0f)
               .border(Style::border_subtle, 1.0f);

    return header_card;
}

// ════════════════════════════════════════════════════════════════
// Tab 0: Taps & Clicks
// ════════════════════════════════════════════════════════════════

WidgetPtr GestureDemoState::buildTapsTab() {
    // 1. Single Tap Card
    auto single_btn_text = text({
        .text = "Tap Me (" + std::to_string(single_tap_count) + ")",
        .color = 0xFFFFFFFF,
        .font_size = 13.5f,
        .font_weight = FontWeight::Bold,
    });

    auto single_btn = container(single_btn_text);
    single_btn->padding(EdgeInsets::symmetric(12.0f, 24.0f))
              .color(Style::primary)
              .borderRadius(8.0f)
              .shadow(0x507C4DFF, {0, 4}, 10.0f);

    auto single_title = text({
        .text = "Single Tap / Click",
        .color = Style::text_primary,
        .font_size = 15.0f,
        .font_weight = FontWeight::Bold,
    });

    auto single_desc = text({
        .text = "Left pointer press & release within slop threshold.",
        .color = Style::text_secondary,
        .font_size = 11.0f,
    });

    auto single_col = column(Justify::Start, Align::Center, {
        single_title,
        sizedBox(0, 4.0f),
        single_desc,
        sizedBox(0, 14.0f),
        single_btn,
    });

    auto single_tap_card = container(single_col);
    single_tap_card->width(360.0f)
                   .paddingAll(18.0f)
                   .color(Style::bg_card)
                   .borderRadius(12.0f)
                   .border(Style::border_subtle, 1.0f);

    auto single_tap_gd = gestureDetector(Key::string("single_tap_target"), single_tap_card);
    single_tap_gd->onTap([this]() {
        setState([this]() {
            single_tap_count++;
            logEvent("Single Tap detected (Count = " + std::to_string(single_tap_count) + ")");
        });
    }).cursor(SystemCursor::Pointer)
      .hitTestBehavior(HitTestBehavior::Opaque);

    // 2. Double Tap Card
    auto double_btn_text = text({
        .text = "Double Click Fast! (" + std::to_string(double_tap_count) + ")",
        .color = 0xFF0A0F1E,
        .font_size = 13.5f,
        .font_weight = FontWeight::Bold,
    });

    auto double_btn = container(double_btn_text);
    double_btn->padding(EdgeInsets::symmetric(12.0f, 24.0f))
              .color(Style::secondary)
              .borderRadius(8.0f)
              .shadow(0x5000E5FF, {0, 4}, 10.0f);

    auto double_title = text({
        .text = "Double Tap (300ms)",
        .color = Style::text_primary,
        .font_size = 15.0f,
        .font_weight = FontWeight::Bold,
    });

    auto double_desc = text({
        .text = "Requires two taps within 300ms window.",
        .color = Style::text_secondary,
        .font_size = 11.0f,
    });

    auto double_col = column(Justify::Start, Align::Center, {
        double_title,
        sizedBox(0, 4.0f),
        double_desc,
        sizedBox(0, 14.0f),
        double_btn,
    });

    auto double_tap_card = container(double_col);
    double_tap_card->width(360.0f)
                   .paddingAll(18.0f)
                   .color(Style::bg_card)
                   .borderRadius(12.0f)
                   .border(Style::border_subtle, 1.0f);

    auto double_tap_gd = gestureDetector(Key::string("double_tap_target"), double_tap_card);
    double_tap_gd->onDoubleTap([this]() {
        setState([this]() {
            double_tap_count++;
            logEvent("⚡ DOUBLE TAP triggered! (Count = " + std::to_string(double_tap_count) + ")");
        });
    }).onTap([this]() {
        setState([this]() {
            logEvent("Tap 1 received — tap again quickly for double tap!");
        });
    }).cursor(SystemCursor::Pointer)
      .hitTestBehavior(HitTestBehavior::Opaque);

    // 3. Right Click (Secondary Tap) Card
    auto sec_btn_text = text({
        .text = "Right Click Here (" + std::to_string(secondary_tap_count) + ")",
        .color = 0xFFFFFFFF,
        .font_size = 13.5f,
        .font_weight = FontWeight::Bold,
    });

    auto sec_btn = container(sec_btn_text);
    sec_btn->padding(EdgeInsets::symmetric(12.0f, 24.0f))
           .color(Style::accent_pink)
           .borderRadius(8.0f)
           .shadow(0x50FF4081, {0, 4}, 10.0f);

    auto sec_title = text({
        .text = "Right Click (Secondary)",
        .color = Style::text_primary,
        .font_size = 15.0f,
        .font_weight = FontWeight::Bold,
    });

    auto sec_desc = text({
        .text = "Dedicated secondary button gesture handler.",
        .color = Style::text_secondary,
        .font_size = 11.0f,
    });

    auto sec_col = column(Justify::Start, Align::Center, {
        sec_title,
        sizedBox(0, 4.0f),
        sec_desc,
        sizedBox(0, 14.0f),
        sec_btn,
    });

    auto sec_tap_card = container(sec_col);
    sec_tap_card->width(360.0f)
                .paddingAll(18.0f)
                .color(Style::bg_card)
                .borderRadius(12.0f)
                .border(Style::border_subtle, 1.0f);

    auto sec_tap_gd = gestureDetector(Key::string("sec_tap_target"), sec_tap_card);
    sec_tap_gd->onSecondaryTap([this]() {
        setState([this]() {
            secondary_tap_count++;
            logEvent("🖱️ Right Click (Secondary Tap) detected (Count = " + std::to_string(secondary_tap_count) + ")");
        });
    }).cursor(SystemCursor::Pointer)
      .hitTestBehavior(HitTestBehavior::Opaque);

    // 4. Long Press Card
    auto lp_btn_text = text({
        .text = "Hold Down 400ms (" + std::to_string(long_press_count) + ")",
        .color = 0xFF141414,
        .font_size = 13.5f,
        .font_weight = FontWeight::Bold,
    });

    auto lp_btn = container(lp_btn_text);
    lp_btn->padding(EdgeInsets::symmetric(12.0f, 24.0f))
          .color(Style::accent_amber)
          .borderRadius(8.0f)
          .shadow(0x50FFAB00, {0, 4}, 10.0f);

    auto lp_title = text({
        .text = "Long Press (400ms)",
        .color = Style::text_primary,
        .font_size = 15.0f,
        .font_weight = FontWeight::Bold,
    });

    auto lp_desc = text({
        .text = "Hold pointer down for >400ms without moving.",
        .color = Style::text_secondary,
        .font_size = 11.0f,
    });

    auto lp_col = column(Justify::Start, Align::Center, {
        lp_title,
        sizedBox(0, 4.0f),
        lp_desc,
        sizedBox(0, 14.0f),
        lp_btn,
    });

    auto long_press_card = container(lp_col);
    long_press_card->width(360.0f)
                   .paddingAll(18.0f)
                   .color(Style::bg_card)
                   .borderRadius(12.0f)
                   .border(Style::border_subtle, 1.0f);

    auto long_press_gd = gestureDetector(Key::string("long_press_target"), long_press_card);
    long_press_gd->onLongPress([this]() {
        setState([this]() {
            long_press_count++;
            logEvent("⏱️ LONG PRESS activated after 400ms! (Count = " + std::to_string(long_press_count) + ")");
        });
    }).cursor(SystemCursor::Pointer)
      .hitTestBehavior(HitTestBehavior::Opaque);

    // Event History Panel
    std::vector<WidgetPtr> log_widgets;
    auto log_header = text({
        .text = "Recent Gesture Event Stream:",
        .color = Style::text_secondary,
        .font_size = 12.5f,
        .font_weight = FontWeight::Bold,
    });
    log_widgets.push_back(log_header);
    log_widgets.push_back(sizedBox(0, 6.0f));

    if (event_logs.empty()) {
        auto empty_text = text({
            .text = "No events captured yet. Click or interact with targets above.",
            .color = Style::text_muted,
            .font_size = 11.5f,
        });
        log_widgets.push_back(empty_text);
    } else {
        for (const auto& entry : event_logs) {
            auto log_item_text = text({
                .text = entry,
                .color = Style::text_primary,
                .font_size = 11.0f,
            });

            auto log_item = container(log_item_text);
            log_item->padding(EdgeInsets::symmetric(5.0f, 10.0f))
                    .margin(EdgeInsets::only(0, 0, 4.0f, 0))
                    .color(Style::bg_input)
                    .borderRadius(6.0f)
                    .border(Style::border_subtle, 1.0f);
            log_widgets.push_back(log_item);
        }
    }

    auto log_card = container(column(std::move(log_widgets)));
    log_card->paddingAll(14.0f)
            .margin(EdgeInsets::only(0, 14.0f, 0, 0))
            .color(Style::bg_card)
            .borderRadius(12.0f)
            .border(Style::border_subtle, 1.0f);

    return column({
        row({
            single_tap_gd,
            sizedBox(14.0f, 0),
            double_tap_gd,
        }),
        sizedBox(0, 14.0f),
        row({
            sec_tap_gd,
            sizedBox(14.0f, 0),
            long_press_gd,
        }),
        log_card,
    });
}

// ════════════════════════════════════════════════════════════════
// Tab 1: Pan & Drag Physics
// ════════════════════════════════════════════════════════════════

WidgetPtr GestureDemoState::buildPanTab() {
    // 1. Draggable Physics Floating Card
    auto drag_title = text({
        .text = "🖐️ 2D Draggable Card",
        .color = 0xFFFFFFFF,
        .font_size = 14.5f,
        .font_weight = FontWeight::Bold,
    });

    auto drag_state_text = text({
        .text = is_dragging_card ? "DRAGGING" : "IDLE",
        .color = is_dragging_card ? Style::secondary : Style::text_secondary,
        .font_size = 10.5f,
        .font_weight = FontWeight::Bold,
    });

    auto drag_header = row(Justify::SpaceBetween, Align::Center, {
        drag_title,
        drag_state_text,
    });

    auto x_text = text({
        .text = "X: " + std::to_string(static_cast<int>(card_offset_x)) + "px",
        .color = Style::secondary,
        .font_size = 12.0f,
    });

    auto y_text = text({
        .text = "Y: " + std::to_string(static_cast<int>(card_offset_y)) + "px",
        .color = Style::secondary,
        .font_size = 12.0f,
    });

    auto vel_text = text({
        .text = "Vel: " + std::to_string(static_cast<int>(card_velocity_x)) + " px/s",
        .color = Style::text_secondary,
        .font_size = 11.0f,
    });

    auto drag_metrics = row({
        x_text,
        sizedBox(14.0f, 0),
        y_text,
        sizedBox(14.0f, 0),
        vel_text,
    });

    auto drag_desc = text({
        .text = "Hold and drag anywhere to move freely across the canvas.",
        .color = 0xD0F0F0FF,
        .font_size = 11.0f,
    });

    auto drag_content = column({
        drag_header,
        sizedBox(0, 6.0f),
        drag_desc,
        sizedBox(0, 10.0f),
        drag_metrics,
    });

    auto drag_box = container(drag_content);
    drag_box->width(320.0f)
            .paddingAll(16.0f)
            .margin(EdgeInsets::fromLTRB(card_offset_x, card_offset_y, 0.0f, 0.0f))
            .color(is_dragging_card ? Style::primary : Style::bg_card_hover)
            .borderRadius(14.0f)
            .border(is_dragging_card ? Style::secondary : Style::primary_light, is_dragging_card ? 2.0f : 1.0f)
            .shadow(is_dragging_card ? 0x807C4DFF : 0x40000000, {0, is_dragging_card ? 10.0f : 4.0f}, is_dragging_card ? 20.0f : 8.0f);

    auto drag_gd = gestureDetector(Key::string("draggable_card"), drag_box);
    drag_gd->onPanStart([this](const DragStartDetails&) {
        setState([this]() {
            is_dragging_card = true;
            logEvent("Pan started on Draggable Card");
        });
    }).onPanUpdate([this](const DragUpdateDetails& d) {
        setState([this, d]() {
            card_offset_x = std::clamp(card_offset_x + d.delta.x, 0.0f, 380.0f);
            card_offset_y = std::clamp(card_offset_y + d.delta.y, 0.0f, 100.0f);
        });
    }).onPanEnd([this](const DragEndDetails& d) {
        setState([this, d]() {
            is_dragging_card = false;
            card_velocity_x  = d.velocity.x;
            card_velocity_y  = d.velocity.y;
            logEvent("Pan ended. Velocity: (" + std::to_string(static_cast<int>(d.velocity.x)) + ", " +
                     std::to_string(static_cast<int>(d.velocity.y)) + ")");
        });
    }).cursor(SystemCursor::Move)
      .hitTestBehavior(HitTestBehavior::Opaque);

    auto canvas_container = container(drag_gd);
    canvas_container->height(200.0f)
                    .paddingAll(14.0f)
                    .color(Style::bg_card)
                    .borderRadius(14.0f)
                    .border(Style::border_subtle, 1.0f);

    // 2. Interactive Horizontal Slider
    float total_slider_w = 680.0f;
    float thumb_w        = 22.0f;
    float max_travel     = total_slider_w - thumb_w;
    float fill_w         = std::clamp(slider_value * max_travel, 0.0f, max_travel);
    float remaining_w    = std::max(0.0f, max_travel - fill_w);

    auto fill_bar = container();
    fill_bar->width(fill_w)
            .height(8.0f)
            .color(Style::primary)
            .borderRadius(4.0f);

    auto thumb_elem = container();
    thumb_elem->width(thumb_w)
              .height(22.0f)
              .color(Style::secondary)
              .borderRadius(11.0f)
              .shadow(0x8000E5FF, {0, 2}, 8.0f);

    auto remaining_bar = container();
    remaining_bar->width(remaining_w)
                 .height(8.0f)
                 .color(Style::bg_input)
                 .borderRadius(4.0f);

    auto slider_row = row(Justify::Start, Align::Center, {
        fill_bar,
        thumb_elem,
        remaining_bar,
    });

    auto slider_hit_area = container(slider_row);
    slider_hit_area->width(total_slider_w)
                   .height(36.0f);

    auto slider_gd = gestureDetector(Key::string("custom_slider_gd"), slider_hit_area);
    slider_gd->onPanUpdate([this, total_slider_w](const DragUpdateDetails& d) {
        setState([this, d, total_slider_w]() {
            slider_value = std::clamp(slider_value + (d.delta.x / total_slider_w), 0.0f, 1.0f);
            logEvent("Slider value updated: " + std::to_string(static_cast<int>(slider_value * 100)) + "%");
        });
    }).onTapDown([this, total_slider_w](const TapDownDetails& d) {
        setState([this, d, total_slider_w]() {
            slider_value = std::clamp(d.local_position.x / total_slider_w, 0.0f, 1.0f);
            logEvent("Slider tapped: " + std::to_string(static_cast<int>(slider_value * 100)) + "%");
        });
    }).cursor(SystemCursor::Pointer)
      .hitTestBehavior(HitTestBehavior::Opaque);

    auto slider_title = text({
        .text = "1D Horizontal Pan Slider",
        .color = Style::text_primary,
        .font_size = 14.5f,
        .font_weight = FontWeight::Bold,
    });

    auto slider_pct = text({
        .text = std::to_string(static_cast<int>(slider_value * 100)) + "%",
        .color = Style::primary_light,
        .font_size = 16.0f,
        .font_weight = FontWeight::Bold,
    });

    auto slider_header = row(Justify::SpaceBetween, Align::Center, {
        slider_title,
        slider_pct,
    });

    auto slider_desc = text({
        .text = "Pan or click across the track to adjust value in real-time.",
        .color = Style::text_secondary,
        .font_size = 11.0f,
    });

    auto slider_card = container(column({
        slider_header,
        sizedBox(0, 6.0f),
        slider_desc,
        sizedBox(0, 14.0f),
        slider_gd,
    }));
    slider_card->paddingAll(18.0f)
               .color(Style::bg_card)
               .borderRadius(14.0f)
               .border(Style::border_subtle, 1.0f);

    // Reset Button
    auto reset_text = text({
        .text = "↺ Reset Card Position",
        .color = Style::text_primary,
        .font_size = 11.5f,
        .font_weight = FontWeight::Bold,
    });

    auto reset_btn = container(reset_text);
    reset_btn->padding(EdgeInsets::symmetric(9.0f, 18.0f))
             .color(Style::bg_card_hover)
             .borderRadius(8.0f)
             .border(Style::border_subtle, 1.0f);

    auto reset_gd = gestureDetector(Key::string("reset_pan_btn"), reset_btn);
    reset_gd->onTap([this]() {
        setState([this]() {
            card_offset_x = 0.0f;
            card_offset_y = 0.0f;
            logEvent("Draggable card position reset to center");
        });
    }).cursor(SystemCursor::Pointer)
      .hitTestBehavior(HitTestBehavior::Opaque);

    return column({
        canvas_container,
        sizedBox(0, 14.0f),
        slider_card,
        sizedBox(0, 10.0f),
        row(Justify::End, Align::Center, {reset_gd}),
    });
}

// ════════════════════════════════════════════════════════════════
// Tab 2: System Cursors Showcase
// ════════════════════════════════════════════════════════════════

WidgetPtr GestureDemoState::buildCursorsTab() {
    struct CursorItem {
        std::string name;
        std::string desc;
        SystemCursor cursor;
        std::string icon;
    };

    const std::vector<CursorItem> cursors = {
        {"Pointer (Hand)", "Clickable buttons and links", SystemCursor::Pointer, "👆"},
        {"Text (I-Beam)", "Selectable text and inputs", SystemCursor::Text, "✏️"},
        {"Move (Cross)", "Movable canvas objects", SystemCursor::Move, "✥"},
        {"Resize Horizontal", "Splitters & column dividers", SystemCursor::ResizeHorizontal, "↔"},
        {"Resize Vertical", "Panel height resizers", SystemCursor::ResizeVertical, "↕"},
        {"Not Allowed", "Disabled or restricted actions", SystemCursor::NotAllowed, "🚫"},
        {"Wait (Spinner)", "Background processing", SystemCursor::Wait, "⏳"},
        {"Default Arrow", "Standard system navigation", SystemCursor::Arrow, "↖"},
    };

    std::vector<WidgetPtr> col1;
    std::vector<WidgetPtr> col2;

    for (size_t i = 0; i < cursors.size(); ++i) {
        const auto& item = cursors[i];
        std::string c_name = item.name;

        auto icon_text = text({
            .text = item.icon,
            .font_size = 18.0f,
        });

        auto icon_box = container(icon_text);
        icon_box->width(42.0f)
                .height(42.0f)
                .color(0x287C4DFF)
                .borderRadius(21.0f);

        auto item_title = text({
            .text = item.name,
            .color = Style::text_primary,
            .font_size = 13.5f,
            .font_weight = FontWeight::Bold,
        });

        auto item_desc = text({
            .text = item.desc,
            .color = Style::text_secondary,
            .font_size = 10.5f,
        });

        auto text_col = column({
            item_title,
            sizedBox(0, 2.0f),
            item_desc,
        });

        auto card_row = row(Justify::Start, Align::Center, {
            icon_box,
            sizedBox(12.0f, 0),
            text_col,
        });

        auto card_content = container(card_row);
        card_content->width(350.0f)
                    .paddingAll(14.0f)
                    .margin(EdgeInsets::only(0, 0, 12.0f, 12.0f))
                    .color(Style::bg_card)
                    .borderRadius(12.0f)
                    .border(Style::border_subtle, 1.0f);

        auto gd = gestureDetector(Key::string("cursor_box_" + std::to_string(i)), card_content);
        gd->cursor(item.cursor)
          .onHoverEnter([this, c_name](const PointerEvent&) {
              setState([this, c_name]() {
                  hovered_cursor_name = c_name;
                  logEvent("Hovering: " + c_name);
              });
          });

        if (i % 2 == 0) col1.push_back(gd);
        else col2.push_back(gd);
    }

    auto banner_label = text({
        .text = "Active Hover Cursor: ",
        .color = Style::text_secondary,
        .font_size = 12.5f,
    });

    auto banner_val = text({
        .text = hovered_cursor_name,
        .color = Style::secondary,
        .font_size = 13.5f,
        .font_weight = FontWeight::Bold,
    });

    auto banner_row = row(Justify::Start, Align::Center, {
        banner_label,
        banner_val,
    });

    auto preview_banner = container(banner_row);
    preview_banner->padding(EdgeInsets::symmetric(12.0f, 18.0f))
                  .margin(EdgeInsets::only(0, 0, 14.0f, 0))
                  .color(Style::bg_input)
                  .borderRadius(10.0f)
                  .border(Style::border_active, 1.0f);

    auto c1_col = column(std::move(col1));
    auto c2_col = column(std::move(col2));

    auto cards_row = row({
        c1_col,
        c2_col,
    });

    return column({
        preview_banner,
        cards_row,
    });
}

// ════════════════════════════════════════════════════════════════
// Tab 3: Hit Testing & Advanced Behaviors
// ════════════════════════════════════════════════════════════════

WidgetPtr GestureDemoState::buildHitTestTab() {
    std::string behavior_name = (current_behavior == HitTestBehavior::Opaque) ? "Opaque" : "DeferToChild";

    // Inner Child Target
    auto inner_text = text({
        .text = "Inner Child (Hits: " + std::to_string(child_hits) + ")",
        .color = 0xFFFFFFFF,
        .font_size = 12.5f,
        .font_weight = FontWeight::Bold,
    });

    auto inner_box = container(inner_text);
    inner_box->padding(EdgeInsets::symmetric(12.0f, 20.0f))
             .color(Style::accent_pink)
             .borderRadius(8.0f);

    auto inner_gd = gestureDetector(Key::string("inner_child_gd"), inner_box);
    inner_gd->onTap([this]() {
        setState([this]() {
            child_hits++;
            logEvent("🎯 Child Hit! Count = " + std::to_string(child_hits));
        });
    }).cursor(SystemCursor::Pointer);

    // Outer Parent Target
    auto outer_title = text({
        .text = "Outer Container (" + behavior_name + ")",
        .color = Style::text_primary,
        .font_size = 14.0f,
        .font_weight = FontWeight::Bold,
    });

    auto outer_hits = text({
        .text = "Parent Hits: " + std::to_string(parent_hits),
        .color = Style::secondary,
        .font_size = 13.0f,
        .font_weight = FontWeight::Bold,
    });

    auto outer_header = row(Justify::SpaceBetween, Align::Center, {
        outer_title,
        outer_hits,
    });

    auto outer_desc = text({
        .text = "Clicking in empty space of outer box behaves according to HitTestBehavior.",
        .color = Style::text_secondary,
        .font_size = 11.0f,
    });

    auto outer_col = column({
        outer_header,
        sizedBox(0, 6.0f),
        outer_desc,
        sizedBox(0, 16.0f),
        inner_gd,
    });

    auto outer_box = container(outer_col);
    outer_box->height(160.0f)
             .paddingAll(18.0f)
             .color(Style::bg_card_hover)
             .borderRadius(14.0f)
             .border(Style::primary_light, 1.0f);

    auto outer_gd = gestureDetector(Key::string("outer_parent_gd"), outer_box);
    outer_gd->onTap([this]() {
        setState([this]() {
            parent_hits++;
            logEvent("Parent container hit! (Hits = " + std::to_string(parent_hits) + ")");
        });
    }).cursor(SystemCursor::Pointer)
      .hitTestBehavior(current_behavior);

    // Behavior Toggle Button
    auto toggle_text = text({
        .text = "Switch Behavior (Current: " + behavior_name + ")",
        .color = 0xFFFFFFFF,
        .font_size = 12.0f,
        .font_weight = FontWeight::Bold,
    });

    auto toggle_btn = container(toggle_text);
    toggle_btn->padding(EdgeInsets::symmetric(10.0f, 18.0f))
              .color(Style::primary)
              .borderRadius(8.0f);

    auto toggle_gd = gestureDetector(Key::string("toggle_behavior_btn"), toggle_btn);
    toggle_gd->onTap([this]() {
        setState([this]() {
            if (current_behavior == HitTestBehavior::Opaque) {
                current_behavior = HitTestBehavior::DeferToChild;
            } else {
                current_behavior = HitTestBehavior::Opaque;
            }
            logEvent("HitTestBehavior changed to " + std::string(current_behavior == HitTestBehavior::Opaque ? "Opaque" : "DeferToChild"));
        });
    }).cursor(SystemCursor::Pointer);

    return column({
        outer_gd,
        sizedBox(0, 14.0f),
        row({toggle_gd}),
    });
}

// ════════════════════════════════════════════════════════════════
// Root Build Function
// ════════════════════════════════════════════════════════════════

WidgetPtr GestureDemoState::build(BuildContext&) {
    WidgetPtr content_tab;
    switch (current_tab) {
        case 0: content_tab = buildTapsTab(); break;
        case 1: content_tab = buildPanTab(); break;
        case 2: content_tab = buildCursorsTab(); break;
        case 3: content_tab = buildHitTestTab(); break;
        default: content_tab = buildTapsTab(); break;
    }

    auto main_content = container(column({
        buildHeader(),
        content_tab,
    }));
    main_content->paddingAll(20.0f)
                .color(Style::bg_main);

    auto divider = container();
    divider->width(1.0f)
           .color(Style::border_subtle);

    return row({
        buildSidebar(),
        divider,
        main_content,
    });
}

// ════════════════════════════════════════════════════════════════
// Application Entry Point
// ════════════════════════════════════════════════════════════════

int main(int argc, char** argv) {
    AppConfig config;
    config.title                    = "ENKI — GestureDetector & Interaction Suite";
    config.width                    = 1080;
    config.height                   = 740;
    config.window_mode              = WindowMode::Normal; // Standard desktop window
    config.vsync                    = false;              // Disable VSync blocking
    config.target_fps               = 0;                  // 0 = Uncapped max speed
    config.show_performance_overlay = true;               // Display real-time FPS & Frame Time HUD

    return runApp(std::make_shared<GestureDemoApp>(), config);
}
