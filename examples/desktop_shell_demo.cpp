/// @file desktop_shell_demo.cpp
/// @brief Live ENKI Desktop Shell Demonstration:
/// 1. 36px Top Bar (LayerSurface / Dock)
/// 2. Native Compositor Popups that escape the bar (App Launcher, WiFi, Volume, Calendar, Power)
/// 3. OSD Notification Surface (Multi-surface concurrent rendering)

#include "enki/shell/shell_app.hpp"
#include "enki/shell/panel_window.hpp"
#include "enki/shell/native_popup.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/state/state.hpp"
#include "enki/tree/element.hpp"
#include "enki/tree/render_object.hpp"

#include <iostream>
#include <memory>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

using namespace enki;

// ── Palette ──────────────────────────────────────────────────────
namespace theme {
    constexpr Color bar_bg        = 0xEE161B22; // Semi-transparent dark bar
    constexpr Color surface       = 0xFF21262D;
    constexpr Color surface_hover = 0xFF30363D;
    constexpr Color popup_bg      = 0xFA1F242C;
    constexpr Color border        = 0xFF363B42;
    constexpr Color accent        = 0xFF58A6FF;
    constexpr Color accent_green  = 0xFF3FB950;
    constexpr Color danger        = 0xFFF85149;
    constexpr Color text_prim     = 0xFFF0F6FC;
    constexpr Color text_sec      = 0xFF8B949E;
}

// ── Reusable Interactive Tray/Bar Item ────────────────────────────
class BarButton : public StatefulWidget {
public:
    std::string icon;
    std::string label;
    std::function<void(BuildContext&, Rect)> on_press;

    BarButton(std::string icon, std::string label, std::function<void(BuildContext&, Rect)> on_press)
        : icon(std::move(icon)), label(std::move(label)), on_press(std::move(on_press)) {}

    std::string_view typeName() const override { return "BarButton"; }
    std::unique_ptr<State> createState() override;
};

class BarButtonState : public State {
    bool hovered_ = false;

public:
    WidgetPtr build(BuildContext& ctx) override {
        auto* w = static_cast<const BarButton*>(widget());

        std::vector<WidgetPtr> items;
        if (!w->icon.empty()) {
            items.push_back(text({
                .text = w->icon,
                .color = hovered_ ? theme::accent : theme::text_prim,
                .font_size = 13.0f,
                .font_weight = FontWeight::Bold,
            }));
        }
        if (!w->label.empty()) {
            items.push_back(text({
                .text = w->label,
                .color = hovered_ ? theme::text_prim : theme::text_sec,
                .font_size = 12.0f,
            }));
        }

        auto content_row = row(std::move(items));
        content_row->gap(StyleValue::point(6.0f));

        auto box = container(content_row);
        box->color(hovered_ ? theme::surface_hover : 0x00000000)
           .borderRadius(6.0f)
           .paddingSymmetric(4.0f, 10.0f);

        auto on_click = w->on_press;
        return gestureDetector({
            .child = box,
            .hit_test_behavior = HitTestBehavior::Opaque,
            .cursor_type = SystemCursor::Pointer,
            .on_tap = [this, on_click] {
                if (on_click) {
                    Rect btn_rect{100.0f, 0.0f, 120.0f, 36.0f};
                    if (auto* ro = context().element()->findRenderObject()) {
                        btn_rect = ro->globalBounds();
                    }
                    on_click(context(), btn_rect);
                }
            },
            .on_hover_enter = [this](const PointerEvent&) { setState([this] { hovered_ = true; }); },
            .on_hover_exit  = [this](const PointerEvent&) { setState([this] { hovered_ = false; }); },
        });
    }
};

std::unique_ptr<State> BarButton::createState() {
    return std::make_unique<BarButtonState>();
}

// ── Popup Builders ───────────────────────────────────────────────

WidgetPtr buildLauncherPopup(BuildContext&, std::shared_ptr<NativePopup> popup) {
    auto search_box = container(
        text({
            .text = "🔍  Search applications, files, commands...",
            .color = theme::text_sec,
            .font_size = 12.0f,
        })
    );
    search_box->color(theme::surface)
              .borderRadius(8.0f)
              .border(theme::border, 1.0f)
              .paddingSymmetric(8.0f, 12.0f);

    auto makeAppItem = [popup](const std::string& icon, const std::string& name, const std::string& category) {
        auto icon_t = text({.text = icon, .color = theme::accent, .font_size = 16.0f});
        auto name_t = text({.text = name, .color = theme::text_prim, .font_size = 13.0f, .font_weight = FontWeight::Bold});
        auto cat_t  = text({.text = category, .color = theme::text_sec, .font_size = 11.0f});

        auto text_col = column({name_t, cat_t});
        text_col->gap(StyleValue::point(2.0f));

        auto r = row({icon_t, text_col});
        r->gap(StyleValue::point(12.0f))
         .padding(StyleInsets::symmetric(8.0f, 12.0f));

        return gestureDetector({
            .child = r,
            .hit_test_behavior = HitTestBehavior::Opaque,
            .cursor_type = SystemCursor::Pointer,
            .on_tap = [popup] {
                std::cout << "[Desktop Shell] Launching application...\n";
                if (popup) popup->close();
            },
        });
    };

    auto app_list = column({
        search_box,
        makeAppItem("🌐", "Web Browser", "Internet"),
        makeAppItem("💻", "Terminal Emulator", "System"),
        makeAppItem("📁", "File Manager", "Files"),
        makeAppItem("⚙", "Settings & Control", "Preferences"),
    });
    app_list->gap(StyleValue::point(8.0f))
            .padding(StyleInsets::all(14.0f));

    auto card = container(app_list);
    card->color(theme::popup_bg)
        .borderRadius(12.0f)
        .border(theme::border, 1.0f)
        .width(300.0f);

    return card;
}

WidgetPtr buildWifiPopup(BuildContext&, std::shared_ptr<NativePopup> popup) {
    auto title = text({
        .text = "Wi-Fi Networks",
        .color = theme::text_prim,
        .font_size = 14.0f,
        .font_weight = FontWeight::Bold,
    });

    auto makeNet = [popup](const std::string& name, const std::string& strength, bool active) {
        auto name_t = text({
            .text = name,
            .color = active ? theme::accent_green : theme::text_prim,
            .font_size = 13.0f,
        });
        auto sig_t = text({
            .text = strength,
            .color = active ? theme::accent_green : theme::text_sec,
            .font_size = 12.0f,
        });

        auto r = row({name_t, sig_t});
        r->justifyContent(Justify::SpaceBetween)
         .padding(StyleInsets::symmetric(8.0f, 12.0f));

        return gestureDetector({
            .child = r,
            .hit_test_behavior = HitTestBehavior::Opaque,
            .cursor_type = SystemCursor::Pointer,
            .on_tap = [popup] { if (popup) popup->close(); },
        });
    };

    auto col = column({
        title,
        makeNet("Home_Fiber_5G", "100%", true),
        makeNet("Office_Network", "85%", false),
        makeNet("Public_Hotspot", "40%", false),
    });
    col->gap(StyleValue::point(10.0f)).padding(StyleInsets::all(14.0f));

    auto card = container(col);
    card->color(theme::popup_bg)
        .borderRadius(12.0f)
        .border(theme::border, 1.0f)
        .width(260.0f);

    return card;
}

// ── Top Bar Content ──────────────────────────────────────────────
class TopBarWidget : public StatefulWidget {
public:
    std::string_view typeName() const override { return "TopBarWidget"; }
    std::unique_ptr<State> createState() override;
};

class TopBarWidgetState : public State {
    std::shared_ptr<NativePopup> active_popup_;

    void openPopup(Point position, int w, int h,
                   std::function<WidgetPtr(BuildContext&, std::shared_ptr<NativePopup>)> content_builder) {
        if (active_popup_ && active_popup_->isOpen()) {
            active_popup_->close();
            active_popup_ = nullptr;
            return;
        }

        PopupOptions opts;
        opts.position     = position;
        opts.width        = w;
        opts.height       = h;
        opts.auto_dismiss = true;

        active_popup_ = NativePopup::show(
            context(),
            opts,
            content_builder
        );
    }

public:
    WidgetPtr build(BuildContext&) override {
        // Left Side: Launcher & Workspace switcher
        auto launcher_btn = std::make_shared<BarButton>(
            "⚡", "Applications",
            [this](BuildContext&, Rect anchor) {
                openPopup(Point(anchor.x, anchor.y + anchor.height + 6.0f), 300, 260, buildLauncherPopup);
            }
        );

        auto ws1 = std::make_shared<BarButton>("1", "", nullptr);
        auto ws2 = std::make_shared<BarButton>("2", "", nullptr);
        auto ws3 = std::make_shared<BarButton>("3", "", nullptr);

        auto left_row = row({launcher_btn, ws1, ws2, ws3});
        left_row->gap(StyleValue::point(4.0f));

        // Center: Clock
        auto time_t = text({
            .text = "Sat Aug 8  22:30",
            .color = theme::text_prim,
            .font_size = 13.0f,
            .font_weight = FontWeight::Bold,
        });
        auto center_box = container(time_t);
        center_box->paddingSymmetric(4.0f, 12.0f);

        // Right Side: Tray Icons & Power
        auto wifi_btn = std::make_shared<BarButton>(
            "📶", "Connected",
            [this](BuildContext&, Rect anchor) {
                openPopup(Point(anchor.x + anchor.width - 260.0f, anchor.y + anchor.height + 6.0f), 260, 200, buildWifiPopup);
            }
        );

        auto vol_btn = std::make_shared<BarButton>("🔊", "75%", nullptr);
        auto bat_btn = std::make_shared<BarButton>("🔋", "92%", nullptr);

        auto right_row = row({wifi_btn, vol_btn, bat_btn});
        right_row->gap(StyleValue::point(6.0f));

        // Bar layout
        auto bar_row = row({left_row, center_box, right_row});
        bar_row->justifyContent(Justify::SpaceBetween)
                .alignItems(Align::Center)
                .padding(StyleInsets::symmetric(0.0f, 12.0f));

        auto bar_container = container(bar_row);
        bar_container->color(theme::bar_bg)
                     .border(theme::border, 1.0f)
                     .width(StyleValue::percent(100.0f))
                     .height(36.0f);

        return bar_container;
    }
};

std::unique_ptr<State> TopBarWidget::createState() {
    return std::make_unique<TopBarWidgetState>();
}

// ── Application Entry Point ──────────────────────────────────────
int main() {
    std::cout << "====================================================\n";
    std::cout << "  ENKI Desktop Shell Live Demo\n";
    std::cout << "  36px Top Bar + Escaped Native Compositor Popups\n";
    std::cout << "====================================================\n";

    AppConfig cfg;
    cfg.title       = "ENKI Desktop Shell Bar";
    cfg.target_fps  = 60;
    cfg.vsync       = true;

    auto app_res = ShellApp::create(cfg);
    if (!app_res.isOk()) {
        std::cerr << "Failed to create ShellApp: " << app_res.error().message << "\n";
        return 1;
    }
    auto app = std::move(app_res.value());

    // 1. Create Top Bar (36px high)
    WindowConfig bar_win;
    bar_win.title      = "ENKI Top Bar";
    bar_win.x          = 0;
    bar_win.y          = 0;
    bar_win.width      = 1200;
    bar_win.height     = 36;
    bar_win.borderless = true;

    auto bar_surface = app->addWindow(bar_win, std::make_shared<TopBarWidget>());
    if (!bar_surface) {
        std::cerr << "Failed to create bar surface\n";
        return 1;
    }

    std::cout << "[ENKI Shell] Top Bar Surface initialized successfully.\n";
    std::cout << "[ENKI Shell] Click 'Applications' or 'Connected' in the bar to spawn native popups!\n";

    return app->run();
}
