/// @file csd_window_demo.cpp
/// @brief Interactive showcase for ENKI Client-Side Decorations (CSD).
/// Demonstrates custom TitleBar, native window dragging, 8-direction resizing,
/// maximize/restore toggling, and live compositor state synchronization.

#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/titlebar.hpp"
#include "enki/widgets/window_frame.hpp"
#include "enki/state/state.hpp"

#include <iostream>
#include <memory>
#include <string>

using namespace enki;

class CsdDemoState : public State {
public:
    bool is_max_ = false;
    bool is_focused_ = true;

    SlotId max_slot_ = 0;
    SlotId focus_slot_ = 0;

    void initState() override {
        State::initState();
        if (App::instance()) {
            auto& win = App::instance()->window();
            is_max_ = win.isMaximized();
            is_focused_ = win.isActivated();

            max_slot_ = win.onMaximized().connect([this](bool max) {
                if (!mounted()) return;
                setState([this, max]() { is_max_ = max; });
            });
            focus_slot_ = win.onFocus().connect([this](bool focus) {
                if (!mounted()) return;
                setState([this, focus]() { is_focused_ = focus; });
            });
        }
    }

    void dispose() override {
        if (App::instance()) {
            auto& win = App::instance()->window();
            if (max_slot_ != 0) win.onMaximized().disconnect(max_slot_);
            if (focus_slot_ != 0) win.onFocus().disconnect(focus_slot_);
        }
        State::dispose();
    }

    WidgetPtr build(BuildContext&) override {
        // Tag pill helper
        auto make_tag = [](const std::string& label, Color bg, Color fg) {
            return container(ContainerProps{
                .color = bg,
                .border_radius = BorderRadius::circular(12.0f),
                .padding = StyleInsets::symmetric(4.0f, 12.0f),
                .child = text(label, TextStyle{
                    .color = fg,
                    .font_size = 12.0f,
                    .font_weight = FontWeight::SemiBold,
                })
            });
        };

        auto status_tag = make_tag(
            is_max_ ? "State: Maximized" : "State: Floating / Restored",
            is_max_ ? 0xFF2E7D32 : 0xFF1565C0,
            0xFFFFFFFF
        );

        auto focus_tag = make_tag(
            is_focused_ ? "Active / Focused" : "Inactive / Unfocused",
            is_focused_ ? 0xFF00695C : 0xFF424242,
            0xFFFFFFFF
        );

        auto header = column(ColumnProps{
            .align_items = Align::Center,
            .margin = StyleInsets::only(0.0f, 0.0f, 24.0f, 0.0f),
            .children = {
                text("Client-Side Decorations (CSD)", TextStyle{
                    .color = 0xFFECEFF4,
                    .font_size = 26.0f,
                    .font_weight = FontWeight::Bold,
                }),
                container(ContainerProps{.height = 6.0f}),
                text(" Native Wayland & X11 Window Frame Architecture", TextStyle{
                    .color = 0xFF88C0D0,
                    .font_size = 14.0f,
                    .font_weight = FontWeight::Medium,
                }),
            }
        });

        auto badges_row = row(RowProps{
            .justify_content = Justify::Center,
            .gap = 12.0f,
            .margin = StyleInsets::only(0.0f, 0.0f, 24.0f, 0.0f),
            .children = { status_tag, focus_tag }
        });

        // Interactive control buttons inside the app
        auto btn_maximize = Button {
            .child = text(is_max_ ? "Restore Window" : "Maximize Window", TextStyle{
                .color = 0xFFFFFFFF,
                .font_size = 13.0f,
                .font_weight = FontWeight::SemiBold,
            }),
            .on_pressed = [this]() {
                if (App::instance()) {
                    App::instance()->window().toggleMaximize();
                }
            }
        };

        auto btn_minimize = Button {
            .child = text("Minimize Window", TextStyle{
                .color = 0xFFFFFFFF,
                .font_size = 13.0f,
                .font_weight = FontWeight::SemiBold,
            }),
            .on_pressed = []() {
                if (App::instance()) {
                    App::instance()->window().setMinimized(true);
                }
            }
        };

        auto btn_fullscreen = Button {
            .child = text("Toggle Fullscreen", TextStyle{
                .color = 0xFFFFFFFF,
                .font_size = 13.0f,
                .font_weight = FontWeight::SemiBold,
            }),
            .on_pressed = []() {
                if (App::instance()) {
                    auto& w = App::instance()->window();
                    w.setFullscreen(!w.isFullscreen());
                }
            }
        };

        auto actions_row = row(RowProps{
            .justify_content = Justify::Center,
            .gap = 12.0f,
            .margin = StyleInsets::only(0.0f, 0.0f, 28.0f, 0.0f),
            .children = { btn_maximize, btn_minimize, btn_fullscreen }
        });

        // Feature cards
        auto card1 = container(ContainerProps{
            .color = 0xFF1F2430,
            .border_radius = BorderRadius::circular(8.0f),
            .border = Border(0x22FFFFFF, 1.0f),
            .width = 240.0f,
            .padding = StyleInsets::all(16.0f),
            .child = column(ColumnProps{
                .align_items = Align::Start,
                .children = {
                    text("Drag & Move", TextStyle{
                        .color = 0xFF81A1C1,
                        .font_size = 15.0f,
                        .font_weight = FontWeight::Bold,
                    }),
                    container(ContainerProps{.height = 6.0f}),
                    text("Click and drag the TitleBar header to seamlessly move the window using native Wayland xdg_toplevel_move and X11 _NET_WM_MOVERESIZE.", TextStyle{
                        .color = 0xFFD8DEE9,
                        .font_size = 12.0f,
                    })
                }
            })
        });

        auto card2 = container(ContainerProps{
            .color = 0xFF1F2430,
            .border_radius = BorderRadius::circular(8.0f),
            .border = Border(0x22FFFFFF, 1.0f),
            .width = 240.0f,
            .padding = StyleInsets::all(16.0f),
            .child = column(ColumnProps{
                .align_items = Align::Start,
                .children = {
                    text("8-Way Resizing", TextStyle{
                        .color = 0xFFA3BE8C,
                        .font_size = 15.0f,
                        .font_weight = FontWeight::Bold,
                    }),
                    container(ContainerProps{.height = 6.0f}),
                    text("Hover over any edge or corner to see responsive resize cursors and drag to resize smoothly via native compositor protocol.", TextStyle{
                        .color = 0xFFD8DEE9,
                        .font_size = 12.0f,
                    })
                }
            })
        });

        auto card3 = container(ContainerProps{
            .color = 0xFF1F2430,
            .border_radius = BorderRadius::circular(8.0f),
            .border = Border(0x22FFFFFF, 1.0f),
            .width = 240.0f,
            .padding = StyleInsets::all(16.0f),
            .child = column(ColumnProps{
                .align_items = Align::Start,
                .children = {
                    text("CSD Negotiation", TextStyle{
                        .color = 0xFFEBCB8B,
                        .font_size = 15.0f,
                        .font_weight = FontWeight::Bold,
                    }),
                    container(ContainerProps{.height = 6.0f}),
                    text("Automatic client-side decoration negotiation via zxdg_decoration_manager_v1 on Wayland and _MOTIF_WM_HINTS on X11.", TextStyle{
                        .color = 0xFFD8DEE9,
                        .font_size = 12.0f,
                    })
                }
            })
        });

        auto cards_row = row(RowProps{
            .justify_content = Justify::Center,
            .gap = 16.0f,
            .children = { card1, card2, card3 }
        });

        auto main_content = column(ColumnProps{
            .justify_content = Justify::Center,
            .align_items = Align::Center,
            .children = { header, badges_row, actions_row, cards_row }
        });

        auto app_body = container(ContainerProps{
            .color = 0xFF14171F,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(24.0f),
            .child = main_content
        });

        // Wrap the entire app in the WindowFrame
        return windowFrame(WindowFrameProps{
            .content = app_body,
            .title = "ENKI — Client-Side Decorations Showcase",
            .border_radius = 12.0f,
            .background_color = 0xFF14171F,
        });
    }
};

class CsdDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<CsdDemoState>();
    }
    [[nodiscard]] std::string_view typeName() const override { return "CsdDemoApp"; }
};

int main() {
    AppConfig config;
    config.title = "ENKI — CSD Showcase";
    config.width = 920;
    config.height = 620;
    config.enable_csd = true;
    config.target_fps  = 0;
    config.vsync       = false;
    config.show_performance_overlay = true;
    config.clear_color = 0xFF14171F;
    config.app_id = "org.enki.csd_demo";

    auto app_res = App::create(std::make_shared<CsdDemoApp>(), config);
    if (!app_res.isOk()) {
        std::cerr << "Failed to create ENKI App: " << app_res.error().message << "\n";
        return 1;
    }

    return app_res.value()->run();
}
