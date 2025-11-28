/// @file titlebar.cpp
/// @brief Modern Client-Side Decoration (CSD) TitleBar implementation.
///
/// @copyright ENKI Framework — MIT License

#include "enki/widgets/titlebar.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/icon.hpp"
#include "enki/app/app.hpp"
#include "enki/state/state.hpp"

namespace enki {

// ════════════════════════════════════════════════════════════════
// Button Type & Icon Resolver
// ════════════════════════════════════════════════════════════════

enum class WindowButtonType {
    Minimize,
    Maximize,
    Close
};

static IconData getWindowButtonIconData(WindowButtonType type, bool is_maximized) {
    switch (type) {
        case WindowButtonType::Minimize:
            // Standard horizontal minus bar
            return IconData::svg("M4 11h16v2H4z");

        case WindowButtonType::Maximize:
            if (is_maximized) {
                // Restore icon: two overlapping square outlines
                return IconData::svg("M4 8h4V4h12v12h-4v4H4V8zm2 2v8h8v-8H6zm12-4H10v2h6v6h2V6z");
            } else {
                // Maximize icon: square outline
                return IconData::svg("M4 4h16v16H4V4zm2 2v12h12V6H6z");
            }

        case WindowButtonType::Close:
            // Standard crisp X
            return IconData::svg("M19 6.41L17.59 5 12 10.59 6.41 5 5 6.41 10.59 12 5 17.59 6.41 19 12 13.41 17.59 19 19 17.59 13.41 12z");
    }
    return {};
}

// ════════════════════════════════════════════════════════════════
// Window Button StatefulWidget
// ════════════════════════════════════════════════════════════════

class WindowButtonWidget : public StatefulWidget {
public:
    WindowButtonType type;
    bool is_maximized;
    Color normal_fg;
    Color hover_bg;
    Color hover_fg;
    std::function<void()> on_pressed;
    TitleBarStyle style = TitleBarStyle::Default;
    bool is_focused = true;

    WindowButtonWidget(WindowButtonType t, bool max, Color n_fg, Color h_bg, Color h_fg,
                       std::function<void()> cb, TitleBarStyle st = TitleBarStyle::Default, bool focused = true)
        : StatefulWidget(Key::none()),
          type(t), is_maximized(max), normal_fg(n_fg), hover_bg(h_bg), hover_fg(h_fg),
          on_pressed(std::move(cb)), style(st), is_focused(focused) {}

    std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "WindowButtonWidget"; }
};

class WindowButtonState : public State {
public:
    bool is_hovered_ = false;
    bool is_pressed_ = false;

    WidgetPtr build(BuildContext& /*ctx*/) override {
        const auto* btn = static_cast<const WindowButtonWidget*>(widget());
        auto on_click = btn->on_pressed;

        WidgetPtr btn_content;

        if (btn->style == TitleBarStyle::VAXPOS) {
            Color circle_color = 0;
            Color glow_color = 0;
            Color glyph_color = 0;

            switch (btn->type) {
                case WindowButtonType::Close:
                    circle_color = (btn->is_focused || is_hovered_) ? 0xFFFF5F56 : 0xFF5C6068;
                    glow_color   = 0xB3FF5F56;
                    glyph_color  = 0xEE4C0000;
                    break;
                case WindowButtonType::Minimize:
                    circle_color = (btn->is_focused || is_hovered_) ? 0xFFFFBD2E : 0xFF5C6068;
                    glow_color   = 0xB3FFBD2E;
                    glyph_color  = 0xEE5C3C00;
                    break;
                case WindowButtonType::Maximize:
                    circle_color = (btn->is_focused || is_hovered_) ? 0xFF27C93F : 0xFF5C6068;
                    glow_color   = 0xB327C93F;
                    glyph_color  = 0xEE004000;
                    break;
            }

            if (is_pressed_) {
                glow_color = (glow_color & 0x00FFFFFF) | 0x66000000;
            }

            WidgetPtr icon_child = nullptr;
            if (is_hovered_) {
                icon_child = icon(getWindowButtonIconData(btn->type, btn->is_maximized), 7.5f, glyph_color);
            }

            std::vector<BoxShadow> shadows;
            if (is_hovered_) {
                shadows.push_back(BoxShadow::glow(glow_color, 8.0f, 1.5f));
            }

            auto circle = container(ContainerProps{
                .color = circle_color,
                .border_radius = BorderRadius::circular(6.5f),
                .border = Border(0x33000000, 0.5f),
                .box_shadow = std::move(shadows),
                .align = Alignment::Center,
                .width = StyleValue::point(13.0f),
                .height = StyleValue::point(13.0f),
                .child = icon_child,
            });

            btn_content = circle;
        } else {
            Color current_bg = is_hovered_ ? btn->hover_bg : Colors::Transparent;
            Color current_fg = is_hovered_ ? btn->hover_fg : btn->normal_fg;

            auto icon_widget = icon(getWindowButtonIconData(btn->type, btn->is_maximized), 11.0f, current_fg);

            btn_content = container(ContainerProps{
                .color = current_bg,
                .align = Alignment::Center,
                .width = StyleValue::point(46.0f),
                .height = StyleValue::percent(100.0f),
                .child = icon_widget
            });
        }

        GestureDetectorProps gp;
        gp.hit_test_behavior = HitTestBehavior::Opaque;
        gp.cursor_type = SystemCursor::Default;

        gp.on_hover_enter = [this](const PointerEvent&) {
            if (!mounted()) return;
            setState([this]() { is_hovered_ = true; });
        };
        gp.on_hover_exit = [this](const PointerEvent&) {
            if (!mounted()) return;
            setState([this]() { is_hovered_ = false; is_pressed_ = false; });
        };
        gp.on_tap_down = [this](const TapDownDetails&) {
            if (!mounted()) return;
            setState([this]() { is_pressed_ = true; });
        };
        gp.on_tap_up = [this](const TapUpDetails&) {
            if (!mounted()) return;
            setState([this]() { is_pressed_ = false; });
        };
        gp.on_tap = [on_click]() {
            if (on_click) on_click();
        };

        gp.child = btn_content;
        return std::make_shared<GestureDetector>(gp);
    }
};

std::unique_ptr<State> WindowButtonWidget::createState() {
    return std::make_unique<WindowButtonState>();
}

// ════════════════════════════════════════════════════════════════
// TitleBar State Implementation
// ════════════════════════════════════════════════════════════════

class TitleBarState : public State {
public:
    bool is_maximized_ = false;
    bool is_focused_ = true;
    Window* window_ = nullptr;

    SlotId max_slot_ = 0;
    SlotId focus_slot_ = 0;
    SlotId state_slot_ = 0;

    void initState() override {
        State::initState();
        const auto* tb = static_cast<const TitleBar*>(widget());
        window_ = tb->props.window;
        if (!window_ && App::instance()) {
            window_ = &App::instance()->window();
        }
        if (window_) {
            is_maximized_ = window_->isMaximized();
            is_focused_ = window_->isActivated();

            max_slot_ = window_->onMaximized().connect([this](bool max) {
                if (!mounted()) return;
                setState([this, max]() { is_maximized_ = max; });
            });
            focus_slot_ = window_->onFocus().connect([this](bool focus) {
                if (!mounted()) return;
                setState([this, focus]() { is_focused_ = focus; });
            });
            state_slot_ = window_->onStateChanged().connect([this](WindowState state) {
                if (!mounted()) return;
                setState([this, state]() {
                    is_maximized_ = hasWindowState(state, WindowState::Maximized);
                    is_focused_ = hasWindowState(state, WindowState::Activated);
                });
            });
        }
    }

    void dispose() override {
        if (window_) {
            if (max_slot_ != 0) window_->onMaximized().disconnect(max_slot_);
            if (focus_slot_ != 0) window_->onFocus().disconnect(focus_slot_);
            if (state_slot_ != 0) window_->onStateChanged().disconnect(state_slot_);
        }
        State::dispose();
    }

    WidgetPtr build(BuildContext& /*ctx*/) override {
        const auto* tb = static_cast<const TitleBar*>(widget());
        const auto& p = tb->props;
        Window* win = window_;

        Color bg_color = is_focused_ ? p.background_color : p.inactive_background_color;
        Color fg_title_color = is_focused_ ? p.title_color : p.inactive_title_color;

        auto make_btn = [&](WindowButtonType type) -> WidgetPtr {
            switch (type) {
                case WindowButtonType::Close: {
                    auto on_cls = p.on_close;
                    return std::make_shared<WindowButtonWidget>(
                        WindowButtonType::Close,
                        is_maximized_,
                        p.button_fg,
                        p.close_hover_bg,
                        p.close_hover_fg,
                        [win, on_cls]() {
                            if (on_cls) on_cls();
                            else if (win) win->onClose().emit();
                        },
                        p.style,
                        is_focused_
                    );
                }
                case WindowButtonType::Minimize: {
                    auto on_min = p.on_minimize;
                    return std::make_shared<WindowButtonWidget>(
                        WindowButtonType::Minimize,
                        is_maximized_,
                        p.button_fg,
                        p.button_hover_bg,
                        p.button_hover_fg,
                        [win, on_min]() {
                            if (on_min) on_min();
                            else if (win) win->setMinimized(true);
                        },
                        p.style,
                        is_focused_
                    );
                }
                case WindowButtonType::Maximize: {
                    auto on_max = p.on_maximize;
                    return std::make_shared<WindowButtonWidget>(
                        WindowButtonType::Maximize,
                        is_maximized_,
                        p.button_fg,
                        p.button_hover_bg,
                        p.button_hover_fg,
                        [win, on_max]() {
                            if (on_max) on_max();
                            else if (win) win->toggleMaximize();
                        },
                        p.style,
                        is_focused_
                    );
                }
            }
            return nullptr;
        };

        auto make_drag_gesture = [&](WidgetPtr child) -> WidgetPtr {
            GestureDetectorProps drag_gesture;
            drag_gesture.hit_test_behavior = HitTestBehavior::Opaque;
            drag_gesture.child = std::move(child);

            drag_gesture.on_pan_start = [win](const DragStartDetails& d) {
                if (win) {
                    win->beginMove(d.global_position.x, d.global_position.y, 1);
                }
            };

            if (p.double_click_maximize) {
                drag_gesture.on_double_tap = [win]() {
                    if (win) {
                        win->toggleMaximize();
                    }
                };
            }

            if (p.right_click_menu) {
                drag_gesture.on_secondary_tap_down = [win](const TapDownDetails& d) {
                    if (win) {
                        win->showWindowMenu(d.local_position.x, d.local_position.y, 3);
                    }
                };
            }

            return std::make_shared<GestureDetector>(drag_gesture);
        };

        WidgetPtr content_row;

        if (p.style == TitleBarStyle::VAXPOS) {
            // VAXPOS layout:
            // Centered Title, Circular buttons (Minimize, Maximize, Close) on Right
            std::vector<WidgetPtr> buttons;
            if (p.show_minimize) buttons.push_back(make_btn(WindowButtonType::Minimize));
            if (p.show_maximize) buttons.push_back(make_btn(WindowButtonType::Maximize));
            if (p.show_close)    buttons.push_back(make_btn(WindowButtonType::Close));

            auto buttons_container = container(ContainerProps{
                .height = StyleValue::percent(100.0f),
                .margin = StyleInsets::only(0.0f, 14.0f, 0.0f, 8.0f),
                .child = row(RowProps{
                    .align_items = Align::Center,
                    .gap = 8.0f,
                    .height = StyleValue::percent(100.0f),
                    .children = std::move(buttons)
                })
            });

            // Title widget
            WidgetPtr title_w = p.title_widget ? p.title_widget : std::make_shared<Text>(
                p.title,
                TextStyle{
                    .color = fg_title_color,
                    .font_size = p.font_size,
                    .font_weight = FontWeight::Medium,
                }
            );

            // Left spacer matching the 77px width of buttons_container to ensure exact centering
            WidgetPtr left_spacer = p.leading ? container(ContainerProps{
                .align = Alignment::CenterLeft,
                .width = StyleValue::point(77.0f),
                .height = StyleValue::percent(100.0f),
                .padding = StyleInsets::only(0.0f, 0.0f, 0.0f, 14.0f),
                .child = p.leading
            }) : make_drag_gesture(container(ContainerProps{
                .width = StyleValue::point(77.0f),
                .height = StyleValue::percent(100.0f),
            }));

            auto left_drag = expanded(make_drag_gesture(container(ContainerProps{
                .height = StyleValue::percent(100.0f)
            })));

            auto right_drag = expanded(make_drag_gesture(container(ContainerProps{
                .height = StyleValue::percent(100.0f)
            })));

            auto center_title = make_drag_gesture(container(ContainerProps{
                .child = title_w
            }));

            content_row = row(RowProps{
                .align_items = Align::Center,
                .height = StyleValue::percent(100.0f),
                .children = {
                    left_spacer,
                    left_drag,
                    center_title,
                    right_drag,
                    buttons_container
                }
            });
        } else {
            // Default layout: Title & Icon on Left, Buttons on Right
            std::vector<WidgetPtr> drag_children;

            if (p.leading) {
                drag_children.push_back(container(ContainerProps{
                    .margin = StyleInsets::only(0.0f, 8.0f, 0.0f, 12.0f),
                    .child = p.leading
                }));
            } else {
                drag_children.push_back(container(ContainerProps{.width = 16.0f}));
            }

            if (p.title_widget) {
                drag_children.push_back(p.title_widget);
            } else {
                drag_children.push_back(std::make_shared<Text>(
                    p.title,
                    TextStyle{
                        .color = fg_title_color,
                        .font_size = p.font_size,
                        .font_weight = FontWeight::Medium,
                    }
                ));
            }

            drag_children.push_back(expanded(
                container(ContainerProps{.height = StyleValue::percent(100.0f)})
            ));

            if (p.trailing) {
                drag_children.push_back(container(ContainerProps{
                    .margin = StyleInsets::only(0.0f, 8.0f, 0.0f, 0.0f),
                    .child = p.trailing
                }));
            }

            auto drag_content = row(RowProps{
                .align_items = Align::Center,
                .height = StyleValue::percent(100.0f),
                .children = std::move(drag_children)
            });

            auto draggable_area = expanded(make_drag_gesture(drag_content));

            std::vector<WidgetPtr> buttons;
            if (p.show_minimize) buttons.push_back(make_btn(WindowButtonType::Minimize));
            if (p.show_maximize) buttons.push_back(make_btn(WindowButtonType::Maximize));
            if (p.show_close)    buttons.push_back(make_btn(WindowButtonType::Close));

            auto buttons_row = row(RowProps{
                .align_items = Align::Center,
                .height = StyleValue::percent(100.0f),
                .children = std::move(buttons)
            });

            content_row = row(RowProps{
                .align_items = Align::Center,
                .height = StyleValue::percent(100.0f),
                .children = { draggable_area, buttons_row }
            });
        }

        return container(ContainerProps{
            .color = bg_color,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::point(p.height),
            .child = content_row
        });
    }
};

std::unique_ptr<State> TitleBar::createState() {
    return std::make_unique<TitleBarState>();
}

} // namespace enki
