/// @file toplevel_dock_demo.cpp
/// @brief ENKI Foreign Toplevel Management & Taskbar Dock Showcase.
/// Built with ENKI Declarative Widget Tree (StatefulWidget, GestureDetector, Flexbox, Container, Text).

#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/state/state.hpp"
#include "enki/platform/platform.hpp"
#include "enki/platform/toplevel.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>

using namespace enki;

// ════════════════════════════════════════════════════════════════
// Design Tokens & Colors
// ════════════════════════════════════════════════════════════════

namespace Theme {
    constexpr uint32_t bg_window      = 0xFF0B0E17;
    constexpr uint32_t bg_dock        = 0xF0131826;
    constexpr uint32_t bg_card        = 0xFF1B2337;
    constexpr uint32_t bg_card_active = 0xFF243B63;
    constexpr uint32_t bg_badge       = 0xFF161E30;
    constexpr uint32_t primary        = 0xFF38BDF8; // Sky blue
    constexpr uint32_t primary_dark   = 0xFF0284C7;
    constexpr uint32_t accent_green   = 0xFF10B981; // Active status
    constexpr uint32_t accent_amber   = 0xFFF59E0B; // Minimized
    constexpr uint32_t accent_purple  = 0xFFA855F7; // Max / Full
    constexpr uint32_t accent_rose    = 0xFFF43F5E; // Close button
    constexpr uint32_t text_primary   = 0xFFF8FAFC;
    constexpr uint32_t text_secondary = 0xFF94A3B8;
    constexpr uint32_t text_muted     = 0xFF64748B;
    constexpr uint32_t border_subtle  = 0x25FFFFFF;
    constexpr uint32_t border_active  = 0xFF38BDF8;
}

// ════════════════════════════════════════════════════════════════
// Dock State & Logic
// ════════════════════════════════════════════════════════════════

class DockAppState : public State {
public:
    std::vector<std::shared_ptr<ToplevelWindow>> windows;
    std::shared_ptr<ToplevelWindow> active_window;
    std::string status_msg = "Ready. Click on windows to interact.";

    void initState() override {
        State::initState();

        auto* platform = Platform::instance();
        if (!platform) return;

        windows = platform->getToplevels();
        active_window = platform->getActiveToplevel();

        platform->onToplevelCreated().connect([this](std::shared_ptr<ToplevelWindow> w) {
            if (!mounted()) return;
            std::cout << "[DOCK EVENT] Window Created: " << w->appId() << " — \"" << w->title() << "\"\n";
            setState([this, w]() {
                auto* p = Platform::instance();
                if (p) {
                    windows = p->getToplevels();
                    active_window = p->getActiveToplevel();
                }
                status_msg = "App opened: " + (w->appId().empty() ? "Window" : w->appId());
            });
        });

        platform->onToplevelClosed().connect([this](std::shared_ptr<ToplevelWindow> w) {
            if (!mounted()) return;
            std::cout << "[DOCK EVENT] Window Closed: " << w->appId() << "\n";
            setState([this, w]() {
                auto* p = Platform::instance();
                if (p) {
                    windows = p->getToplevels();
                    active_window = p->getActiveToplevel();
                }
                status_msg = "App closed: " + (w->appId().empty() ? "Window" : w->appId());
            });
        });

        platform->onActiveToplevelChanged().connect([this](std::shared_ptr<ToplevelWindow> w) {
            if (!mounted()) return;
            if (w) {
                std::cout << "[DOCK EVENT] Focus: " << w->appId() << " — \"" << w->title() << "\"\n";
            }
            setState([this, w]() {
                active_window = w;
                if (w) {
                    status_msg = "Focused: " + (w->appId().empty() ? "Window" : w->appId());
                }
            });
        });

        platform->onToplevelTitleChanged().connect([this](std::shared_ptr<ToplevelWindow> w, std::string_view title) {
            if (!mounted()) return;
            setState([this]() {});
        });

        platform->onToplevelStateChanged().connect([this](std::shared_ptr<ToplevelWindow> w, WindowState) {
            if (!mounted()) return;
            setState([this]() {});
        });
    }

    WidgetPtr build(BuildContext& context) override;

private:
    WidgetPtr buildHeader();
    WidgetPtr buildWindowCard(const std::shared_ptr<ToplevelWindow>& tl);
    WidgetPtr buildEmptyState();
};

// ════════════════════════════════════════════════════════════════
// Dock StatefulWidget
// ════════════════════════════════════════════════════════════════

class DockApp : public StatefulWidget {
public:
    DockApp() : StatefulWidget(Key::string("dock_app_root")) {}

    std::unique_ptr<State> createState() override {
        return std::make_unique<DockAppState>();
    }

    [[nodiscard]] std::string_view typeName() const override { return "DockApp"; }
};

// ════════════════════════════════════════════════════════════════
// UI Builders
// ════════════════════════════════════════════════════════════════

WidgetPtr DockAppState::buildHeader() {
    auto title_text = text({
        .text = "⚡ ENKI TASKBAR DOCK",
        .color = Theme::primary,
        .font_size = 14.0f,
        .font_weight = FontWeight::Bold,
    });

    auto sub_text = text({
        .text = "Native Foreign Toplevel Management (wlr-foreign-toplevel & EWMH)",
        .color = Theme::text_secondary,
        .font_size = 10.5f,
    });

    auto header_left = column({
        .children = {
            title_text,
            sizedBox(0, 2.0f),
            sub_text,
        }
    });

    // Active Window indicator
    std::string active_name = active_window ? active_window->appId() : "None";
    if (active_name.empty()) active_name = "Window";

    auto active_badge_lbl = text({
        .text = "ACTIVE APP: " + active_name,
        .color = Theme::accent_green,
        .font_size = 10.5f,
        .font_weight = FontWeight::Bold,
    });

    auto active_badge = container(active_badge_lbl);
    active_badge->padding(EdgeInsets::symmetric(5.0f, 12.0f))
                .color(0x2010B981)
                .borderRadius(14.0f)
                .border(Theme::accent_green, 1.0f);

    // Window count badge
    auto count_lbl = text({
        .text = std::to_string(windows.size()) + " Running",
        .color = Theme::text_primary,
        .font_size = 11.0f,
        .font_weight = FontWeight::Bold,
    });

    auto count_badge = container(count_lbl);
    count_badge->padding(EdgeInsets::symmetric(5.0f, 12.0f))
               .color(Theme::bg_badge)
               .borderRadius(14.0f)
               .border(Theme::border_subtle, 1.0f);

    auto header_right = row({
        .justify_content = Justify::End,
        .align_items = Align::Center,
        .children = {
            active_badge,
            sizedBox(10.0f, 0),
            count_badge,
        }
    });

    auto header_row = row({
        .justify_content = Justify::SpaceBetween,
        .align_items = Align::Center,
        .children = {
            header_left,
            header_right,
        }
    });

    auto header_box = container(header_row);
    header_box->padding(EdgeInsets::symmetric(12.0f, 16.0f))
              .margin(EdgeInsets::only(0, 0, 12.0f, 0))
              .color(0x30000000)
              .borderRadius(10.0f)
              .border(Theme::border_subtle, 1.0f);

    return header_box;
}

WidgetPtr DockAppState::buildWindowCard(const std::shared_ptr<ToplevelWindow>& tl) {
    bool is_active = (active_window && active_window->id() == tl->id()) || tl->isActivated();

    // App ID Header
    std::string app_name = tl->appId().empty() ? "Application" : tl->appId();
    if (app_name.length() > 18) app_name = app_name.substr(0, 15) + "...";

    auto app_text = text({
        .text = app_name,
        .color = is_active ? Theme::primary : Theme::text_primary,
        .font_size = 12.0f,
        .font_weight = FontWeight::Bold,
    });

    // Close button (Dedicated GestureDetector)
    auto close_x = text({
        .text = "✕",
        .color = Theme::accent_rose,
        .font_size = 11.0f,
        .font_weight = FontWeight::Bold,
    });

    auto close_box = container(close_x);
    close_box->padding(EdgeInsets::symmetric(2.0f, 6.0f))
             .color(0x20F43F5E)
             .borderRadius(6.0f);

    auto close_gd = gestureDetector({
        .key = Key::string("close_btn_" + std::to_string(tl->id())),
        .child = close_box,
        .hit_test_behavior = HitTestBehavior::Opaque,
        .cursor_type = SystemCursor::Pointer,
        .on_tap = [tl]() {
            std::cout << "[DOCK ACTION] Close window: " << tl->appId() << "\n";
            tl->close();
        },
    });

    auto card_top_row = row({
        .justify_content = Justify::SpaceBetween,
        .align_items = Align::Center,
        .children = {
            app_text,
            close_gd,
        }
    });

    // Window Title
    std::string win_title = tl->title().empty() ? "(Untitled Window)" : tl->title();
    if (win_title.length() > 22) win_title = win_title.substr(0, 19) + "...";

    auto title_widget = text({
        .text = win_title,
        .color = Theme::text_secondary,
        .font_size = 10.5f,
    });

    // Badges Row
    std::vector<WidgetPtr> badges;

    if (is_active) {
        auto b = text({ .text = "ACTIVE", .color = Theme::accent_green, .font_size = 8.5f, .font_weight = FontWeight::Bold });
        auto bc = container(b); bc->padding(EdgeInsets::symmetric(2.0f, 6.0f)).color(0x2810B981).borderRadius(4.0f);
        badges.push_back(bc);
        badges.push_back(sizedBox(4.0f, 0));
    }
    if (tl->isMinimized()) {
        auto b = text({ .text = "MIN", .color = Theme::accent_amber, .font_size = 8.5f, .font_weight = FontWeight::Bold });
        auto bc = container(b); bc->padding(EdgeInsets::symmetric(2.0f, 6.0f)).color(0x28F59E0B).borderRadius(4.0f);
        badges.push_back(bc);
        badges.push_back(sizedBox(4.0f, 0));
    }
    if (tl->isMaximized()) {
        auto b = text({ .text = "MAX", .color = Theme::primary, .font_size = 8.5f, .font_weight = FontWeight::Bold });
        auto bc = container(b); bc->padding(EdgeInsets::symmetric(2.0f, 6.0f)).color(0x280284C7).borderRadius(4.0f);
        badges.push_back(bc);
        badges.push_back(sizedBox(4.0f, 0));
    }
    if (tl->isFullscreen()) {
        auto b = text({ .text = "FULL", .color = Theme::accent_purple, .font_size = 8.5f, .font_weight = FontWeight::Bold });
        auto bc = container(b); bc->padding(EdgeInsets::symmetric(2.0f, 6.0f)).color(0x28A855F7).borderRadius(4.0f);
        badges.push_back(bc);
    }

    if (badges.empty()) {
        auto b = text({ .text = "NORMAL", .color = Theme::text_muted, .font_size = 8.5f });
        auto bc = container(b); bc->padding(EdgeInsets::symmetric(2.0f, 6.0f)).color(0x18FFFFFF).borderRadius(4.0f);
        badges.push_back(bc);
    }

    auto badges_row = row({
        .justify_content = Justify::Start,
        .align_items = Align::Center,
        .children = std::move(badges),
    });

    auto card_col = column({
        .justify_content = Justify::SpaceBetween,
        .align_items = Align::Start,
        .children = {
            card_top_row,
            title_widget,
            badges_row,
        }
    });

    auto card_container = container(card_col);
    card_container->width(190.0f)
                  .height(84.0f)
                  .paddingAll(10.0f)
                  .color(is_active ? Theme::bg_card_active : Theme::bg_card)
                  .borderRadius(10.0f)
                  .border(is_active ? Theme::border_active : Theme::border_subtle, is_active ? 1.5f : 1.0f)
                  .shadow(is_active ? 0x4038BDF8 : 0x20000000, {0, 3}, 8.0f);

    // Wrap whole card in GestureDetector for Activate / Minimize / Maximize gestures
    return gestureDetector({
        .key = Key::string("card_gd_" + std::to_string(tl->id())),
        .child = card_container,
        .hit_test_behavior = HitTestBehavior::Opaque,
        .cursor_type = SystemCursor::Pointer,
        .on_tap = [tl]() {
            std::cout << "[DOCK GESTURE] Tap -> Activate: " << tl->appId() << "\n";
            if (tl->isMinimized()) {
                tl->setMinimized(false);
            }
            tl->activate();
        },
        .on_secondary_tap = [tl]() {
            std::cout << "[DOCK GESTURE] Right-Click -> Toggle Minimize: " << tl->appId() << "\n";
            tl->setMinimized(!tl->isMinimized());
        },
        .on_double_tap = [tl]() {
            std::cout << "[DOCK GESTURE] Double-Tap -> Toggle Maximize: " << tl->appId() << "\n";
            tl->setMaximized(!tl->isMaximized());
        },
    });
}

WidgetPtr DockAppState::buildEmptyState() {
    auto empty_icon = text({
        .text = "🔍",
        .font_size = 22.0f,
    });

    auto empty_title = text({
        .text = "No External Windows Detected",
        .color = Theme::text_primary,
        .font_size = 13.0f,
        .font_weight = FontWeight::Bold,
    });

    auto empty_desc = text({
        .text = "Listening for zwlr_foreign_toplevel & X11 EWMH events in real time...",
        .color = Theme::text_muted,
        .font_size = 10.5f,
    });

    auto empty_col = column({
        .justify_content = Justify::Center,
        .align_items = Align::Center,
        .children = {
            empty_icon,
            sizedBox(0, 4.0f),
            empty_title,
            sizedBox(0, 2.0f),
            empty_desc,
        }
    });

    auto empty_box = container(empty_col);
    empty_box->height(84.0f)
             .paddingAll(12.0f)
             .color(Theme::bg_card)
             .borderRadius(10.0f)
             .border(Theme::border_subtle, 1.0f);

    return empty_box;
}

WidgetPtr DockAppState::build(BuildContext&) {
    std::vector<WidgetPtr> card_widgets;

    if (windows.empty()) {
        card_widgets.push_back(buildEmptyState());
    } else {
        for (const auto& tl : windows) {
            if (!card_widgets.empty()) {
                card_widgets.push_back(sizedBox(10.0f, 0));
            }
            card_widgets.push_back(buildWindowCard(tl));
        }
    }

    auto cards_row = row({
        .justify_content = Justify::Start,
        .align_items = Align::Center,
        .children = std::move(card_widgets),
    });

    // Footer Hint
    auto hint_text = text({
        .text = "💡 Gestures: Left-Click = Activate & Focus  |  Right-Click = Minimize/Restore  |  Double-Click = Maximize  |  ✕ = Close",
        .color = Theme::text_secondary,
        .font_size = 10.0f,
    });

    auto main_col = column({
        .children = {
            buildHeader(),
            cards_row,
            sizedBox(0, 10.0f),
            hint_text,
        }
    });

    auto root_container = container(main_col);
    root_container->paddingAll(16.0f)
                  .color(Theme::bg_window);

    return root_container;
}

// ════════════════════════════════════════════════════════════════
// Application Entry Point
// ════════════════════════════════════════════════════════════════

int main(int argc, char** argv) {
    std::cout << "================================================\n";
    std::cout << "  ENKI — Foreign Toplevel Dock & Window Manager \n";
    std::cout << "  Built with Declarative GestureDetector Widgets \n";
    std::cout << "================================================\n";

    AppConfig config;
    config.title                    = "ENKI Window Manager Dock";
    config.width                    = 1020;
    config.height                   = 195;
    config.window_mode              = WindowMode::Normal;
    config.vsync                    = true;
    config.show_performance_overlay = false;

    return runApp(std::make_shared<DockApp>(), config);
}
