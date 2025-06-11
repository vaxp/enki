/// @file main.cpp
/// @brief ENKI Universal Popup Widget Interactive Showcase.
/// Demonstrates 12-direction placements, cursor tracking, dismiss controls, and custom Skia rendering.

#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/popup.hpp"
#include "enki/state/state.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <memory>

using namespace enki;

// ── Custom Interactive Button (BarButton style as in desktop_shell_demo) ───

class DemoButton : public StatefulWidget {
public:
    std::string icon;
    std::string label;
    Color bg_color;
    Color hover_color;

    DemoButton(std::string icon, std::string label, Color bg = 0xFF1E293B, Color hov = 0xFF334155)
        : icon(std::move(icon)), label(std::move(label)), bg_color(bg), hover_color(hov) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "DemoButton"; }
};

class DemoButtonState : public State {
private:
    bool hovered_ = false;

public:
    WidgetPtr build(BuildContext& ctx) override {
        auto* btn = static_cast<const DemoButton*>(widget());

        std::vector<WidgetPtr> items;
        if (!btn->icon.empty()) {
            auto icon_t = text(btn->icon);
            icon_t->fontSize(13.0f).bold().color(0xFFFFFFFF);
            items.push_back(icon_t);
        }
        if (!btn->label.empty()) {
            auto label_t = text(btn->label);
            label_t->fontSize(13.0f).bold().color(0xFFFFFFFF);
            items.push_back(label_t);
        }

        auto content_row = row(std::move(items));
        content_row->gap(StyleValue::point(8.0f))
                   .alignItems(Align::Center);

        auto box = container(content_row);
        box->color(hovered_ ? btn->hover_color : btn->bg_color)
           .borderRadius(8.0f)
           .border(0xFF475569, 1.0f)
           .paddingSymmetric(10.0f, 16.0f);

        auto detector = std::make_shared<GestureDetector>();
        detector->hit_test_behavior = HitTestBehavior::Opaque;
        detector->cursor_type       = SystemCursor::Pointer;

        detector->on_hover_enter = [this](const PointerEvent&) {
            setState([this] { hovered_ = true; });
        };
        detector->on_hover_exit = [this](const PointerEvent&) {
            setState([this] { hovered_ = false; });
        };

        detector->child = box;
        return detector;
    }
};

std::unique_ptr<State> DemoButton::createState() {
    return std::make_unique<DemoButtonState>();
}

// ── Main Popup Showcase Application ───────────────────────────────

class PopupDemoState : public State {
public:
    WidgetPtr build(BuildContext& ctx) override {
        // Title & Header
        auto title = text("Universal Native Popup Widget (NativePopup)");
        title->fontSize(24.0f).bold().color(0xFFFFFFFF);

        auto sub = text("Demonstrates 12-direction placements, cursor tracking, and instant auto-dismiss");
        sub->fontSize(14.0f).color(0xFF94A3B8);

        std::vector<WidgetPtr> t_children = {title, sub};
        auto titleCol = column(t_children);
        titleCol->alignItems(Align::Center).margin(StyleInsets::only(0, 0, 40.0f, 0));

        // 1. TopCenter Placement Popup
        auto top_btn = std::make_shared<DemoButton>("🔼", "TopCenter Popup", 0xFF1E293B, 0xFF334155);

        PopupWidgetOptions opt1;
        opt1.placement = PopupPlacement::TopCenter;
        opt1.content_size = Size{220.0f, 85.0f};
        opt1.background_color = 0xFA1E293B;
        opt1.border_color = 0xFF38BDF8;

        auto top_popup = popup(top_btn, [](BuildContext&, std::shared_ptr<NativePopup> popup) {
            auto h = text("TopCenter Popup Card");
            h->fontSize(13.0f).bold().color(0xFF38BDF8);
            auto b = text("Positioned precisely above anchor widget with auto-fitting.");
            b->fontSize(11.0f).color(0xFFCBD5E1);
            return column({h, b});
        }, opt1);

        // 2. BottomRight Placement Popup
        auto bottom_btn = std::make_shared<DemoButton>("🔽", "BottomRight Popup", 0xFF1E293B, 0xFF334155);

        PopupWidgetOptions opt2;
        opt2.placement = PopupPlacement::BottomRight;
        opt2.content_size = Size{220.0f, 85.0f};
        opt2.background_color = 0xFA1E293B;
        opt2.border_color = 0xFF10B981;

        auto bottom_popup = popup(bottom_btn, [](BuildContext&, std::shared_ptr<NativePopup> popup) {
            auto h = text("BottomRight Popup Card");
            h->fontSize(13.0f).bold().color(0xFF10B981);
            auto b = text("Aligned to bottom right corner with smooth drop shadows.");
            b->fontSize(11.0f).color(0xFFCBD5E1);
            return column({h, b});
        }, opt2);

        // 3. Follow Cursor Popup (Hover Trigger)
        auto hover_btn = std::make_shared<DemoButton>("🎯", "Hover (FollowCursor)", 0xFF1E293B, 0xFF334155);

        PopupWidgetOptions opt3;
        opt3.placement = PopupPlacement::FollowCursor;
        opt3.trigger = PopupTrigger::Hover;
        opt3.content_size = Size{200.0f, 65.0f};
        opt3.background_color = 0xFA0F172A;
        opt3.border_color = 0xFFF59E0B;

        auto cursor_popup = popup(hover_btn, [](BuildContext&, std::shared_ptr<NativePopup> popup) {
            auto h = text("Tracking Cursor Point");
            h->fontSize(12.0f).bold().color(0xFFF59E0B);
            auto b = text("Dynamically spawns near pointer position.");
            b->fontSize(11.0f).color(0xFF94A3B8);
            return column({h, b});
        }, opt3);

        // 4. Center Screen Modal Popup
        auto center_btn = std::make_shared<DemoButton>("⚡", "Center Modal Popup", 0xFF2563EB, 0xFF3B82F6);

        PopupWidgetOptions opt4;
        opt4.placement = PopupPlacement::CenterScreen;
        opt4.trigger = PopupTrigger::Click;
        opt4.content_size = Size{260.0f, 130.0f};
        opt4.background_color = 0xFA1E1E2E;
        opt4.border_color = 0xFFA855F7;

        auto center_popup = popup(center_btn, [](BuildContext& sub_ctx, std::shared_ptr<NativePopup> popup) {
            auto h = text("Center Screen Modal Popup");
            h->fontSize(14.0f).bold().color(0xFFA855F7);
            auto b = text("Spawned in center of screen with dismiss action button.");
            b->fontSize(11.0f).color(0xFFCBD5E1);

            // Dismiss Button using clean GestureDetector
            auto dismiss_txt = text("Dismiss");
            dismiss_txt->fontSize(12.0f).bold().color(0xFFFFFFFF);
            auto dismiss_box = container(dismiss_txt);
            dismiss_box->color(0xFF2563EB)
                       .borderRadius(6.0f)
                       .paddingSymmetric(6.0f, 16.0f)
                       .align(Alignment::Center);

            auto dismiss_gesture = std::make_shared<GestureDetector>();
            dismiss_gesture->hit_test_behavior = HitTestBehavior::Opaque;
            dismiss_gesture->cursor_type       = SystemCursor::Pointer;
            dismiss_gesture->on_tap = [popup] {
                std::cout << "[Popup] Dismiss Clicked — closing popup!\n";
                if (popup) popup->close();
            };
            dismiss_gesture->child = dismiss_box;

            auto col = column({h, b, dismiss_gesture});
            col->gap(StyleValue::point(10.0f));
            return col;
        }, opt4);

        // Layout rows
        std::vector<WidgetPtr> r_children = {top_popup, bottom_popup, cursor_popup, center_popup};
        auto buttonsRow = row(r_children);
        buttonsRow->justifyContent(Justify::Center).alignItems(Align::Center).gap(16_px);

        std::vector<WidgetPtr> m_children = {titleCol, buttonsRow};
        auto mainCol = column(m_children);
        mainCol->alignItems(Align::Center).justifyContent(Justify::Center);

        auto appRoot = container(mainCol);
        appRoot->color(0xFF0F172A)
               .paddingAll(40.0f)
               .flexGrow(1.0f);

        return appRoot;
    }
};

class PopupDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<PopupDemoState>();
    }
    std::string_view typeName() const override { return "PopupDemoApp"; }
};

int main() {
    std::cout << "====================================================\n";
    std::cout << "  ENKI Engine — Universal Popup Widget Demo\n";
    std::cout << "====================================================\n";

    AppConfig config;
    config.title       = "Enki Engine — Popup Demo";
    config.width       = 950;
    config.height      = 450;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 60;
    config.clear_color = 0xFF0F172A;

    return runApp(std::make_shared<PopupDemoApp>(), config);
}
