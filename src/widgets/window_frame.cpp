/// @file window_frame.cpp
/// @brief Modern Client-Side Decoration (CSD) WindowFrame container implementation.
///
/// @copyright ENKI Framework — MIT License

#include "enki/widgets/window_frame.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/stack.hpp"
#include "enki/app/app.hpp"
#include "enki/state/state.hpp"

namespace enki {

class WindowFrameState : public State {
public:
    bool is_maximized_ = false;
    bool is_focused_ = true;
    Window* window_ = nullptr;

    SlotId max_slot_ = 0;
    SlotId focus_slot_ = 0;
    SlotId state_slot_ = 0;

    void initState() override {
        State::initState();
        const auto* wf = static_cast<const WindowFrame*>(widget());
        window_ = wf->props.window;
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

    WidgetPtr makeResizeHandle(WindowEdge edge, SystemCursor cursor,
                               std::optional<float> top, std::optional<float> right,
                               std::optional<float> bottom, std::optional<float> left,
                               std::optional<float> width, std::optional<float> height) {
        Window* win = window_;

        GestureDetectorProps gp;
        gp.hit_test_behavior = HitTestBehavior::Opaque;
        gp.cursor_type = cursor;
        gp.child = container(ContainerProps{
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
        });

        gp.on_pan_start = [win, edge](const DragStartDetails& d) {
            if (win) {
                win->beginResize(edge, d.global_position.x, d.global_position.y, 1);
            }
        };

        auto gd = std::make_shared<GestureDetector>(gp);

        PositionedProps pp;
        pp.child = gd;
        if (top)    pp.top    = StyleValue::point(*top);
        if (right)  pp.right  = StyleValue::point(*right);
        if (bottom) pp.bottom = StyleValue::point(*bottom);
        if (left)   pp.left   = StyleValue::point(*left);
        if (width)  pp.width  = StyleValue::point(*width);
        if (height) pp.height = StyleValue::point(*height);

        return positioned(pp);
    }

    WidgetPtr build(BuildContext& /*ctx*/) override {
        const auto* wf = static_cast<const WindowFrame*>(widget());
        const auto& p = wf->props;
        Window* win = window_;

        // 1. TitleBar
        WidgetPtr tb = p.titlebar;
        if (!tb) {
            TitleBarProps tbp;
            tbp.window = win;
            tbp.title = p.title;
            tbp.style = p.titlebar_style;
            if (p.titlebar_background_color) {
                tbp.background_color = *p.titlebar_background_color;
            }
            if (p.titlebar_inactive_background_color) {
                tbp.inactive_background_color = *p.titlebar_inactive_background_color;
            }
            tb = std::make_shared<TitleBar>(tbp);
        }

        // Wrap TitleBar in a fixed-height container so that both TitleBar and Content
        // are SingleChildRenderObjectWidgets and preserve their exact top-to-bottom layout
        auto tb_wrapper = container(ContainerProps{
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::point(38.0f),
            .child = tb,
        });

        // 2. Main Window Content Body
        std::vector<WidgetPtr> col_children;
        col_children.push_back(tb_wrapper);
        if (p.content) {
            col_children.push_back(expanded(p.content));
        }

        auto window_column = column(ColumnProps{
            .flex_direction = FlexDirection::Column,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .children = std::move(col_children),
        });

        // Visual frame styling
        float radius = is_maximized_ ? 0.0f : p.border_radius;
        std::optional<Border> frame_border = std::nullopt;
        if (!is_maximized_ && (p.border_width > 0.0f || !p.border_shader.empty() || !p.border_svg.empty())) {
            float bw = p.border_width > 0.0f ? p.border_width : 2.0f;
            frame_border = Border(p.border_color, bw);
        }

        std::string cur_border_shader = "";
        if (!is_maximized_) {
            if (!is_focused_ && !p.inactive_border_shader.empty()) {
                cur_border_shader = p.inactive_border_shader;
            } else {
                cur_border_shader = p.border_shader;
            }
        }

        std::string cur_border_svg = (!is_maximized_ ? p.border_svg : "");
        auto frame_body = container(ContainerProps{
            .color = p.background_color,
            .border_radius = BorderRadius::circular(radius),
            .border = frame_border,
            .clip_content = true,
            .background_shader = (!is_maximized_ ? p.background_shader : ""),
            .border_shader = std::move(cur_border_shader),
            .background_svg = (!is_maximized_ ? p.background_svg : ""),
            .border_svg = std::move(cur_border_svg),
            .svg_slice = p.border_svg_slice,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .child = window_column,
        });

        // 3. Assemble Stack (ALWAYS returns StackWidget to prevent widget tree disposal on maximize/restore)
        std::vector<WidgetPtr> stack_children;
        stack_children.push_back(positioned(0.0f, 0.0f, 0.0f, 0.0f, frame_body));

        // 8-Direction Resize Overlay Handles (only active when not maximized)
        if (!is_maximized_ && p.enable_resize && win) {
            float t = p.resize_thickness;
            float c = p.corner_size;

            // Edges
            // Top
            stack_children.push_back(makeResizeHandle(
                WindowEdge::Top, SystemCursor::ResizeVertical,
                0.0f, c, std::nullopt, c, std::nullopt, t));
            // Bottom
            stack_children.push_back(makeResizeHandle(
                WindowEdge::Bottom, SystemCursor::ResizeVertical,
                std::nullopt, c, 0.0f, c, std::nullopt, t));
            // Left
            stack_children.push_back(makeResizeHandle(
                WindowEdge::Left, SystemCursor::ResizeHorizontal,
                c, std::nullopt, c, 0.0f, t, std::nullopt));
            // Right
            stack_children.push_back(makeResizeHandle(
                WindowEdge::Right, SystemCursor::ResizeHorizontal,
                c, 0.0f, c, std::nullopt, t, std::nullopt));

            // Corners
            // TopLeft
            stack_children.push_back(makeResizeHandle(
                WindowEdge::TopLeft, SystemCursor::ResizeTopLeft,
                0.0f, std::nullopt, std::nullopt, 0.0f, c, c));
            // TopRight
            stack_children.push_back(makeResizeHandle(
                WindowEdge::TopRight, SystemCursor::ResizeTopRight,
                0.0f, 0.0f, std::nullopt, std::nullopt, c, c));
            // BottomLeft
            stack_children.push_back(makeResizeHandle(
                WindowEdge::BottomLeft, SystemCursor::ResizeBottomLeft,
                std::nullopt, std::nullopt, 0.0f, 0.0f, c, c));
            // BottomRight
            stack_children.push_back(makeResizeHandle(
                WindowEdge::BottomRight, SystemCursor::ResizeBottomRight,
                std::nullopt, 0.0f, 0.0f, std::nullopt, c, c));
        }

        StackProps sp;
        sp.children = std::move(stack_children);
        sp.fit = StackFit::Expand;
        return stack(std::move(sp));
    }
};

std::unique_ptr<State> WindowFrame::createState() {
    return std::make_unique<WindowFrameState>();
}

} // namespace enki
