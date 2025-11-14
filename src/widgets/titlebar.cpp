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

    WindowButtonWidget(WindowButtonType t, bool max, Color n_fg, Color h_bg, Color h_fg, std::function<void()> cb)
        : StatefulWidget(Key::none()),
          type(t), is_maximized(max), normal_fg(n_fg), hover_bg(h_bg), hover_fg(h_fg), on_pressed(std::move(cb)) {}

    std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "WindowButtonWidget"; }
};

class WindowButtonState : public State {
public:
    bool is_hovered_ = false;
    bool is_pressed_ = false;

    WidgetPtr build(BuildContext& /*ctx*/) override {
        const auto* btn = static_cast<const WindowButtonWidget*>(widget());
        Color current_bg = is_hovered_ ? btn->hover_bg : Colors::Transparent;
        Color current_fg = is_hovered_ ? btn->hover_fg : btn->normal_fg;
        auto on_click = btn->on_pressed;

        auto icon_widget = icon(getWindowButtonIconData(btn->type, btn->is_maximized), 11.0f, current_fg);

        auto btn_container = container(ContainerProps{
            .color = current_bg,
            .align = Alignment::Center,
            .width = StyleValue::point(46.0f),
            .height = StyleValue::percent(100.0f),
            .child = icon_widget
        });

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

        gp.child = btn_container;
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

        // 1. Draggable Body (Leading + Title + Trailing)
        std::vector<WidgetPtr> drag_children;

        // Leading
        if (p.leading) {
            drag_children.push_back(container(ContainerProps{
                .margin = StyleInsets::only(0.0f, 8.0f, 0.0f, 12.0f),
                .child = p.leading
            }));
        } else {
            drag_children.push_back(container(ContainerProps{.width = 16.0f}));
        }

        // Title widget or text
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

        // Expanded spacer in the middle of drag area
        drag_children.push_back(expanded(
            container(ContainerProps{.height = StyleValue::percent(100.0f)})
        ));

        // Trailing widget (if any)
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

        GestureDetectorProps drag_gesture;
        drag_gesture.hit_test_behavior = HitTestBehavior::Opaque;
        drag_gesture.child = drag_content;

        // Native compositor move dragging
        drag_gesture.on_pan_start = [win](const DragStartDetails& d) {
            if (win) {
                win->beginMove(d.global_position.x, d.global_position.y, 1);
            }
        };

        // Double click to maximize / restore
        if (p.double_click_maximize) {
            drag_gesture.on_double_tap = [win]() {
                if (win) {
                    win->toggleMaximize();
                }
            };
        }

        // Right-click window menu
        if (p.right_click_menu) {
            drag_gesture.on_secondary_tap_down = [win](const TapDownDetails& d) {
                if (win) {
                    win->showWindowMenu(d.local_position.x, d.local_position.y, 3);
                }
            };
        }

        auto draggable_area = expanded(
            std::make_shared<GestureDetector>(drag_gesture)
        );

        // 2. Control Buttons
        std::vector<WidgetPtr> buttons;

        if (p.show_minimize) {
            auto on_min = p.on_minimize;
            buttons.push_back(std::make_shared<WindowButtonWidget>(
                WindowButtonType::Minimize,
                is_maximized_,
                p.button_fg,
                p.button_hover_bg,
                p.button_hover_fg,
                [win, on_min]() {
                    if (on_min) {
                        on_min();
                    } else if (win) {
                        win->setMinimized(true);
                    }
                }
            ));
        }

        if (p.show_maximize) {
            auto on_max = p.on_maximize;
            buttons.push_back(std::make_shared<WindowButtonWidget>(
                WindowButtonType::Maximize,
                is_maximized_,
                p.button_fg,
                p.button_hover_bg,
                p.button_hover_fg,
                [win, on_max]() {
                    if (on_max) {
                        on_max();
                    } else if (win) {
                        win->toggleMaximize();
                    }
                }
            ));
        }

        if (p.show_close) {
            auto on_cls = p.on_close;
            buttons.push_back(std::make_shared<WindowButtonWidget>(
                WindowButtonType::Close,
                is_maximized_,
                p.button_fg,
                p.close_hover_bg,
                p.close_hover_fg,
                [win, on_cls]() {
                    if (on_cls) {
                        on_cls();
                    } else if (win) {
                        win->onClose().emit();
                    }
                }
            ));
        }

        auto buttons_row = row(RowProps{
            .align_items = Align::Center,
            .height = StyleValue::percent(100.0f),
            .children = std::move(buttons)
        });

        // 3. Assemble TitleBar
        auto full_row = row(RowProps{
            .align_items = Align::Center,
            .height = StyleValue::percent(100.0f),
            .children = { draggable_area, buttons_row }
        });

        return container(ContainerProps{
            .color = bg_color,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::point(p.height),
            .child = full_row
        });
    }
};

std::unique_ptr<State> TitleBar::createState() {
    return std::make_unique<TitleBarState>();
}

} // namespace enki
