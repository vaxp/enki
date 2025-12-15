/// @file floating_panel.cpp
/// @brief Ultra high-performance implementation of Advanced Draggable & Resizable Floating Window overlay widget.

#include "enki/widgets/floating_panel.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/stack.hpp"
#include "enki/state/state.hpp"
#include "enki/tree/build_context.hpp"
#include "enki/platform/platform.hpp"

#include <algorithm>
#include <iostream>
#include <cmath>

namespace enki {

// ════════════════════════════════════════════════════════════════
// RenderFloatingPanelBox — Direct Skia & Anu-level Positioning Box
// ════════════════════════════════════════════════════════════════

class RenderFloatingPanelBox : public RenderBox {
public:
    float x_ = 200.0f;
    float y_ = 100.0f;
    float w_ = 460.0f;
    float h_ = 340.0f;
    bool is_minimized_ = false;

    RenderFloatingPanelBox(float x, float y, float w, float h, bool is_minimized)
        : x_(x), y_(y), w_(w), h_(h), is_minimized_(is_minimized) {
        applyStyleToNode();
    }

    void setGeometry(float x, float y, float w, float h, bool is_minimized) {
        bool changed = (x_ != x || y_ != y || w_ != w || h_ != h || is_minimized_ != is_minimized);
        if (!changed) return;
        x_ = x;
        y_ = y;
        w_ = w;
        h_ = h;
        is_minimized_ = is_minimized;
        offset_.x = x;
        offset_.y = y;
        size_.width = w;
        size_.height = is_minimized ? 40.0f : h;
        applyStyleToNode();
        markNeedsLayout();
    }

    void setPosition(float x, float y) {
        if (x_ == x && y_ == y) return;
        x_ = x;
        y_ = y;
        offset_.x = x;
        offset_.y = y;
        if (anu_node_) {
            ANUNodeStyleSetPosition(anu_node_, ANUEdgeLeft, x_);
            ANUNodeStyleSetPosition(anu_node_, ANUEdgeTop, y_);
        }
        markNeedsLayout();
    }

    void setSize(float w, float h) {
        if (w_ == w && h_ == h) return;
        w_ = w;
        h_ = h;
        size_.width = w;
        size_.height = is_minimized_ ? 40.0f : h;
        if (anu_node_) {
            ANUNodeStyleSetWidth(anu_node_, w_);
            ANUNodeStyleSetHeight(anu_node_, is_minimized_ ? 40.0f : h_);
        }
        markNeedsLayout();
    }

    void applyStyleToNode() {
        if (!anu_node_) return;
        ANUNodeStyleSetPositionType(anu_node_, ANUPositionTypeAbsolute);
        ANUNodeStyleSetPosition(anu_node_, ANUEdgeLeft, x_);
        ANUNodeStyleSetPosition(anu_node_, ANUEdgeTop, y_);
        ANUNodeStyleSetWidth(anu_node_, w_);
        ANUNodeStyleSetHeight(anu_node_, is_minimized_ ? 40.0f : h_);
        ANUNodeStyleSetFlexDirection(anu_node_, ANUFlexDirectionColumn);
        ANUNodeStyleSetAlignItems(anu_node_, ANUAlignStretch);
    }

    void paint(PaintContext& context) override {
        for (auto* child : children_) {
            if (child) {
                PaintContext child_ctx = context.withOffset(child->offset());
                child->paint(child_ctx);
            }
        }
    }

    bool hitTestChildren(HitTestResult& result, Point localPoint) override {
        return RenderObject::hitTestChildren(result, localPoint);
    }
};

// ════════════════════════════════════════════════════════════════
// FloatingPanelBoxWidget — SingleChild wrapper for RenderFloatingPanelBox
// ════════════════════════════════════════════════════════════════

class FloatingPanelBoxWidget : public SingleChildRenderObjectWidget {
public:
    float x;
    float y;
    float w;
    float h;
    bool is_minimized;
    std::shared_ptr<RenderFloatingPanelBox*> box_holder;

    FloatingPanelBoxWidget(WidgetPtr child, float x, float y, float w, float h,
                           bool is_minimized, std::shared_ptr<RenderFloatingPanelBox*> holder)
        : SingleChildRenderObjectWidget(Key::none(), std::move(child)),
          x(x), y(y), w(w), h(h), is_minimized(is_minimized), box_holder(std::move(holder)) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext&) override {
        auto r = std::make_unique<RenderFloatingPanelBox>(x, y, w, h, is_minimized);
        if (box_holder) *box_holder = r.get();
        return r;
    }

    void updateRenderObject(BuildContext&, RenderObject& ro) override {
        auto& r = static_cast<RenderFloatingPanelBox&>(ro);
        if (box_holder) *box_holder = &r;
        r.setGeometry(x, y, w, h, is_minimized);
    }

    void didUnmountRenderObject(RenderObject&) override {
        if (box_holder && *box_holder) *box_holder = nullptr;
    }

    [[nodiscard]] std::string_view typeName() const override { return "FloatingPanelBoxWidget"; }
};

// ════════════════════════════════════════════════════════════════
// FloatingPanel State
// ════════════════════════════════════════════════════════════════

class FloatingPanelState : public State {
private:
    bool is_open_ = true;
    bool is_active_ = true;
    FloatingPanelDisplayState display_state_ = FloatingPanelDisplayState::Normal;

    float current_x_ = 200.0f;
    float current_y_ = 100.0f;
    float current_w_ = 460.0f;
    float current_h_ = 340.0f;

    // Cache for restoring from maximized/minimized
    float saved_x_ = 200.0f;
    float saved_y_ = 100.0f;
    float saved_w_ = 460.0f;
    float saved_h_ = 340.0f;

    // Drag-to-move state
    float drag_start_mouse_x_ = 0.0f;
    float drag_start_mouse_y_ = 0.0f;
    float drag_start_panel_x_ = 0.0f;
    float drag_start_panel_y_ = 0.0f;

    std::shared_ptr<RenderFloatingPanelBox*> render_box_holder_ =
        std::make_shared<RenderFloatingPanelBox*>(nullptr);

    RenderFloatingPanelBox* getRenderBox() const {
        return render_box_holder_ ? *render_box_holder_ : nullptr;
    }

public:
    void initState() override {
        State::initState();
        auto* w = static_cast<const FloatingPanelWidget*>(widget());
        is_open_ = w->initial_open;

        current_x_ = w->options.initial_x;
        current_y_ = w->options.initial_y;
        current_w_ = w->options.initial_width;
        current_h_ = w->options.initial_height;

        saved_x_ = current_x_;
        saved_y_ = current_y_;
        saved_w_ = current_w_;
        saved_h_ = current_h_;

        wireController();
    }

    void didUpdateWidget(const Widget& old) override {
        State::didUpdateWidget(old);
        wireController();
    }

    void wireController() {
        auto* w = static_cast<const FloatingPanelWidget*>(widget());
        if (w->controller) {
            w->controller->show_fn = [this] {
                is_open_ = true;
                is_active_ = true;
                setState([] {});
            };
            w->controller->hide_fn = [this] {
                is_open_ = false;
                setState([] {});
            };
            w->controller->toggle_fn = [this] {
                is_open_ = !is_open_;
                setState([] {});
            };
            w->controller->is_open_fn = [this] { return is_open_; };
            w->controller->set_position_fn = [this](float x, float y) {
                current_x_ = x;
                current_y_ = y;
                auto* rb = getRenderBox();
                if (rb) {
                    rb->setPosition(x, y);
                }
                auto* fp = static_cast<const FloatingPanelWidget*>(widget());
                if (fp->options.on_moved) fp->options.on_moved(x, y);
                setState([] {});
            };
            w->controller->get_position_fn = [this] { return Point{current_x_, current_y_}; };
            w->controller->set_size_fn = [this](float width, float height) {
                auto* fp = static_cast<const FloatingPanelWidget*>(widget());
                current_w_ = std::clamp(width, fp->options.min_width, fp->options.max_width);
                current_h_ = std::clamp(height, fp->options.min_height, fp->options.max_height);
                auto* rb = getRenderBox();
                if (rb) {
                    rb->setSize(current_w_, current_h_);
                }
                if (fp->options.on_resized) fp->options.on_resized(current_w_, current_h_);
                setState([] {});
            };
            w->controller->get_size_fn = [this] { return Size{current_w_, current_h_}; };
            w->controller->set_state_fn = [this](FloatingPanelDisplayState st) {
                setDisplayState(st);
            };
            w->controller->get_state_fn = [this] { return display_state_; };
            w->controller->bring_to_front_fn = [this] {
                if (!is_active_) {
                    is_active_ = true;
                    setState([] {});
                }
            };
        }
    }

    void setDisplayState(FloatingPanelDisplayState next_state) {
        if (display_state_ == next_state) return;

        if (display_state_ == FloatingPanelDisplayState::Normal) {
            saved_x_ = current_x_;
            saved_y_ = current_y_;
            saved_w_ = current_w_;
            saved_h_ = current_h_;
        }

        display_state_ = next_state;

        if (display_state_ == FloatingPanelDisplayState::Normal) {
            current_x_ = saved_x_;
            current_y_ = saved_y_;
            current_w_ = saved_w_;
            current_h_ = saved_h_;
        } else if (display_state_ == FloatingPanelDisplayState::Maximized) {
            current_x_ = 10.0f;
            current_y_ = 10.0f;
            current_w_ = 1100.0f;
            current_h_ = 620.0f;
        }

        auto* rb = getRenderBox();
        if (rb) {
            rb->setGeometry(current_x_, current_y_, current_w_, current_h_,
                            display_state_ == FloatingPanelDisplayState::Minimized);
        }

        auto* w = static_cast<const FloatingPanelWidget*>(widget());
        if (w->options.on_state_changed) {
            w->options.on_state_changed(display_state_);
        }
        setState([] {});
    }

    void toggleMinimize() {
        if (display_state_ == FloatingPanelDisplayState::Minimized) {
            setDisplayState(FloatingPanelDisplayState::Normal);
        } else {
            setDisplayState(FloatingPanelDisplayState::Minimized);
        }
    }

    void toggleMaximize() {
        if (display_state_ == FloatingPanelDisplayState::Maximized) {
            setDisplayState(FloatingPanelDisplayState::Normal);
        } else {
            setDisplayState(FloatingPanelDisplayState::Maximized);
        }
    }

    void closePanel() {
        is_open_ = false;
        auto* w = static_cast<const FloatingPanelWidget*>(widget());
        if (w->options.on_closed) {
            w->options.on_closed();
        }
        setState([] {});
    }

    // ── Build Header Title Bar ────────────────────────────────────
    WidgetPtr buildTitleBar(const FloatingPanelWidget* w) {
        const auto& opts = w->options;

        // Leading Icon + Title
        auto title_content = row({
            .align_items = Align::Center,
            .gap = StyleValue::point(8.0f),
            .children = {
                text(opts.icon, { .color = opts.title_color, .font_size = 15.0f }),
                text(opts.title, {
                    .color = opts.title_color,
                    .font_size = 13.5f,
                    .font_weight = FontWeight::Bold
                })
            }
        });

        // Window Control Buttons
        std::vector<WidgetPtr> buttons;

        // 1. Minimize button (—)
        if (opts.allow_minimize) {
            buttons.push_back(gestureDetector({
                .child = container({
                    .color = 0x22FFFFFF,
                    .border_radius = BorderRadius::circular(4.0f),
                    .padding = StyleInsets::symmetric(3.0f, 7.0f),
                    .child = text("—", { .color = 0xFF94A3B8, .font_size = 10.0f, .font_weight = FontWeight::Bold })
                }),
                .cursor_type = SystemCursor::Pointer,
                .on_tap_up = [this](const TapUpDetails&) {
                    toggleMinimize();
                }
            }));
        }

        // 2. Maximize / Restore button (□ / ❐)
        if (opts.allow_maximize) {
            bool is_max = (display_state_ == FloatingPanelDisplayState::Maximized);
            buttons.push_back(gestureDetector({
                .child = container({
                    .color = 0x22FFFFFF,
                    .border_radius = BorderRadius::circular(4.0f),
                    .padding = StyleInsets::symmetric(3.0f, 7.0f),
                    .child = text(is_max ? "❐" : "□", { .color = 0xFF94A3B8, .font_size = 10.5f, .font_weight = FontWeight::Bold })
                }),
                .cursor_type = SystemCursor::Pointer,
                .on_tap_up = [this](const TapUpDetails&) {
                    toggleMaximize();
                }
            }));
        }

        // 3. Close button (✕)
        if (opts.allow_close) {
            buttons.push_back(gestureDetector({
                .child = container({
                    .color = 0x33EF4444, // Red subtle pill
                    .border_radius = BorderRadius::circular(4.0f),
                    .padding = StyleInsets::symmetric(3.0f, 7.0f),
                    .child = text("✕", { .color = 0xFFFCA5A5, .font_size = 10.5f, .font_weight = FontWeight::Bold })
                }),
                .cursor_type = SystemCursor::Pointer,
                .on_tap_up = [this](const TapUpDetails&) {
                    closePanel();
                }
            }));
        }

        auto controls_row = row({
            .align_items = Align::Center,
            .gap = StyleValue::point(6.0f),
            .children = buttons
        });

        auto titlebar_inner = container({
            .color = opts.titlebar_bg_color,
            .border_radius = BorderRadius::only(opts.border_radius, opts.border_radius, 0.0f, 0.0f),
            .border = Border(opts.border_color, 1.0f),
            .padding = StyleInsets::symmetric(9.0f, 14.0f),
            .child = row({
                .justify_content = Justify::SpaceBetween,
                .align_items = Align::Center,
                .children = {
                    title_content,
                    controls_row
                }
            })
        });

        // Wrap with Drag-to-move GestureDetector (Zero Rebuild)
        return gestureDetector({
            .child = titlebar_inner,
            .cursor_type = (opts.allow_drag && display_state_ != FloatingPanelDisplayState::Maximized)
                            ? SystemCursor::Move : SystemCursor::Default,
            .on_pan_start = [this](const DragStartDetails& d) {
                auto* w = static_cast<const FloatingPanelWidget*>(widget());
                if (!w->options.allow_drag || display_state_ == FloatingPanelDisplayState::Maximized) return;
                drag_start_mouse_x_ = d.global_position.x;
                drag_start_mouse_y_ = d.global_position.y;
                drag_start_panel_x_ = current_x_;
                drag_start_panel_y_ = current_y_;
                if (!is_active_) {
                    is_active_ = true;
                    setState([] {});
                }
            },
            .on_pan_update = [this](const DragUpdateDetails& d) {
                auto* w = static_cast<const FloatingPanelWidget*>(widget());
                if (!w->options.allow_drag || display_state_ == FloatingPanelDisplayState::Maximized) return;

                float dx = d.global_position.x - drag_start_mouse_x_;
                float dy = d.global_position.y - drag_start_mouse_y_;

                current_x_ = drag_start_panel_x_ + dx;
                current_y_ = drag_start_panel_y_ + dy;

                // Edge Snapping
                if (w->options.snap_to_edges) {
                    float snap = w->options.snap_threshold;
                    if (current_x_ < snap) current_x_ = 0.0f;
                    if (current_y_ < snap) current_y_ = 0.0f;
                }

                // Clamping to visible viewport
                if (current_x_ < 0.0f) current_x_ = 0.0f;
                if (current_y_ < 0.0f) current_y_ = 0.0f;

                auto* rb = getRenderBox();
                if (rb) {
                    rb->setPosition(current_x_, current_y_);
                }

                if (w->options.on_drag_update) {
                    w->options.on_drag_update(current_x_, current_y_);
                }
            },
            .on_pan_end = [this](const DragEndDetails&) {
                auto* w = static_cast<const FloatingPanelWidget*>(widget());
                if (w->options.on_moved) {
                    w->options.on_moved(current_x_, current_y_);
                }
            }
        });
    }

    // ── Build Resize Handles (All 8 Directions, Zero Rebuild) ─────
    std::vector<WidgetPtr> buildResizeHandles(const FloatingPanelWidget* w) {
        std::vector<WidgetPtr> handles;
        const auto& opts = w->options;
        if (!opts.allow_resize || display_state_ != FloatingPanelDisplayState::Normal) {
            return handles;
        }

        float th = opts.resize_handle_thickness;

        // 1. Right Edge (E)
        handles.push_back(Positioned {
            .child = gestureDetector({
                .child = container({
                    .width = StyleValue::point(th),
                    .height = StyleValue::percent(100.0f)
                }),
                .cursor_type = SystemCursor::ResizeHorizontal,
                .on_pan_update = [this](const DragUpdateDetails& d) {
                    auto* fp = static_cast<const FloatingPanelWidget*>(widget());
                    current_w_ = std::clamp(current_w_ + d.delta.x, fp->options.min_width, fp->options.max_width);
                    auto* rb = getRenderBox();
                    if (rb) rb->setSize(current_w_, current_h_);
                    if (fp->options.on_resize_update) fp->options.on_resize_update(current_w_, current_h_);
                },
                .on_pan_end = [this](const DragEndDetails&) {
                    auto* fp = static_cast<const FloatingPanelWidget*>(widget());
                    if (fp->options.on_resized) fp->options.on_resized(current_w_, current_h_);
                }
            }),
            .top = StyleValue::point(th),
            .right = StyleValue::point(0.0f),
            .bottom = StyleValue::point(th),
            .width = StyleValue::point(th)
        });

        // 2. Bottom Edge (S)
        handles.push_back(Positioned {
            .child = gestureDetector({
                .child = container({
                    .width = StyleValue::percent(100.0f),
                    .height = StyleValue::point(th)
                }),
                .cursor_type = SystemCursor::ResizeVertical,
                .on_pan_update = [this](const DragUpdateDetails& d) {
                    auto* fp = static_cast<const FloatingPanelWidget*>(widget());
                    current_h_ = std::clamp(current_h_ + d.delta.y, fp->options.min_height, fp->options.max_height);
                    auto* rb = getRenderBox();
                    if (rb) rb->setSize(current_w_, current_h_);
                    if (fp->options.on_resize_update) fp->options.on_resize_update(current_w_, current_h_);
                },
                .on_pan_end = [this](const DragEndDetails&) {
                    auto* fp = static_cast<const FloatingPanelWidget*>(widget());
                    if (fp->options.on_resized) fp->options.on_resized(current_w_, current_h_);
                }
            }),
            .right = StyleValue::point(th),
            .bottom = StyleValue::point(0.0f),
            .left = StyleValue::point(th),
            .height = StyleValue::point(th)
        });

        // 3. Left Edge (W)
        handles.push_back(Positioned {
            .child = gestureDetector({
                .child = container({
                    .width = StyleValue::point(th),
                    .height = StyleValue::percent(100.0f)
                }),
                .cursor_type = SystemCursor::ResizeHorizontal,
                .on_pan_update = [this](const DragUpdateDetails& d) {
                    auto* fp = static_cast<const FloatingPanelWidget*>(widget());
                    float new_w = std::clamp(current_w_ - d.delta.x, fp->options.min_width, fp->options.max_width);
                    float diff = current_w_ - new_w;
                    current_x_ += diff;
                    current_w_ = new_w;
                    auto* rb = getRenderBox();
                    if (rb) rb->setGeometry(current_x_, current_y_, current_w_, current_h_, false);
                    if (fp->options.on_resize_update) fp->options.on_resize_update(current_w_, current_h_);
                },
                .on_pan_end = [this](const DragEndDetails&) {
                    auto* fp = static_cast<const FloatingPanelWidget*>(widget());
                    if (fp->options.on_resized) fp->options.on_resized(current_w_, current_h_);
                }
            }),
            .top = StyleValue::point(th),
            .bottom = StyleValue::point(th),
            .left = StyleValue::point(0.0f),
            .width = StyleValue::point(th)
        });

        // 4. Top Edge (N)
        handles.push_back(Positioned {
            .child = gestureDetector({
                .child = container({
                    .width = StyleValue::percent(100.0f),
                    .height = StyleValue::point(th)
                }),
                .cursor_type = SystemCursor::ResizeVertical,
                .on_pan_update = [this](const DragUpdateDetails& d) {
                    auto* fp = static_cast<const FloatingPanelWidget*>(widget());
                    float new_h = std::clamp(current_h_ - d.delta.y, fp->options.min_height, fp->options.max_height);
                    float diff = current_h_ - new_h;
                    current_y_ += diff;
                    current_h_ = new_h;
                    auto* rb = getRenderBox();
                    if (rb) rb->setGeometry(current_x_, current_y_, current_w_, current_h_, false);
                    if (fp->options.on_resize_update) fp->options.on_resize_update(current_w_, current_h_);
                },
                .on_pan_end = [this](const DragEndDetails&) {
                    auto* fp = static_cast<const FloatingPanelWidget*>(widget());
                    if (fp->options.on_resized) fp->options.on_resized(current_w_, current_h_);
                }
            }),
            .top = StyleValue::point(0.0f),
            .right = StyleValue::point(th),
            .left = StyleValue::point(th),
            .height = StyleValue::point(th)
        });

        // 5. Bottom-Right Corner (SE)
        handles.push_back(Positioned {
            .child = gestureDetector({
                .child = container({
                    .width = StyleValue::point(th * 2.0f),
                    .height = StyleValue::point(th * 2.0f)
                }),
                .cursor_type = SystemCursor::ResizeBottomRight,
                .on_pan_update = [this](const DragUpdateDetails& d) {
                    auto* fp = static_cast<const FloatingPanelWidget*>(widget());
                    current_w_ = std::clamp(current_w_ + d.delta.x, fp->options.min_width, fp->options.max_width);
                    current_h_ = std::clamp(current_h_ + d.delta.y, fp->options.min_height, fp->options.max_height);
                    auto* rb = getRenderBox();
                    if (rb) rb->setSize(current_w_, current_h_);
                    if (fp->options.on_resize_update) fp->options.on_resize_update(current_w_, current_h_);
                },
                .on_pan_end = [this](const DragEndDetails&) {
                    auto* fp = static_cast<const FloatingPanelWidget*>(widget());
                    if (fp->options.on_resized) fp->options.on_resized(current_w_, current_h_);
                }
            }),
            .right = StyleValue::point(0.0f),
            .bottom = StyleValue::point(0.0f),
            .width = StyleValue::point(th * 2.0f),
            .height = StyleValue::point(th * 2.0f)
        });

        // 6. Bottom-Left Corner (SW)
        handles.push_back(Positioned {
            .child = gestureDetector({
                .child = container({
                    .width = StyleValue::point(th * 2.0f),
                    .height = StyleValue::point(th * 2.0f)
                }),
                .cursor_type = SystemCursor::ResizeBottomLeft,
                .on_pan_update = [this](const DragUpdateDetails& d) {
                    auto* fp = static_cast<const FloatingPanelWidget*>(widget());
                    float new_w = std::clamp(current_w_ - d.delta.x, fp->options.min_width, fp->options.max_width);
                    current_x_ += (current_w_ - new_w);
                    current_w_ = new_w;
                    current_h_ = std::clamp(current_h_ + d.delta.y, fp->options.min_height, fp->options.max_height);
                    auto* rb = getRenderBox();
                    if (rb) rb->setGeometry(current_x_, current_y_, current_w_, current_h_, false);
                    if (fp->options.on_resize_update) fp->options.on_resize_update(current_w_, current_h_);
                },
                .on_pan_end = [this](const DragEndDetails&) {
                    auto* fp = static_cast<const FloatingPanelWidget*>(widget());
                    if (fp->options.on_resized) fp->options.on_resized(current_w_, current_h_);
                }
            }),
            .bottom = StyleValue::point(0.0f),
            .left = StyleValue::point(0.0f),
            .width = StyleValue::point(th * 2.0f),
            .height = StyleValue::point(th * 2.0f)
        });

        // 7. Top-Right Corner (NE)
        handles.push_back(Positioned {
            .child = gestureDetector({
                .child = container({
                    .width = StyleValue::point(th * 2.0f),
                    .height = StyleValue::point(th * 2.0f)
                }),
                .cursor_type = SystemCursor::ResizeTopRight,
                .on_pan_update = [this](const DragUpdateDetails& d) {
                    auto* fp = static_cast<const FloatingPanelWidget*>(widget());
                    float new_h = std::clamp(current_h_ - d.delta.y, fp->options.min_height, fp->options.max_height);
                    current_y_ += (current_h_ - new_h);
                    current_h_ = new_h;
                    current_w_ = std::clamp(current_w_ + d.delta.x, fp->options.min_width, fp->options.max_width);
                    auto* rb = getRenderBox();
                    if (rb) rb->setGeometry(current_x_, current_y_, current_w_, current_h_, false);
                    if (fp->options.on_resize_update) fp->options.on_resize_update(current_w_, current_h_);
                },
                .on_pan_end = [this](const DragEndDetails&) {
                    auto* fp = static_cast<const FloatingPanelWidget*>(widget());
                    if (fp->options.on_resized) fp->options.on_resized(current_w_, current_h_);
                }
            }),
            .top = StyleValue::point(0.0f),
            .right = StyleValue::point(0.0f),
            .width = StyleValue::point(th * 2.0f),
            .height = StyleValue::point(th * 2.0f)
        });

        // 8. Top-Left Corner (NW)
        handles.push_back(Positioned {
            .child = gestureDetector({
                .child = container({
                    .width = StyleValue::point(th * 2.0f),
                    .height = StyleValue::point(th * 2.0f)
                }),
                .cursor_type = SystemCursor::ResizeTopLeft,
                .on_pan_update = [this](const DragUpdateDetails& d) {
                    auto* fp = static_cast<const FloatingPanelWidget*>(widget());
                    float new_w = std::clamp(current_w_ - d.delta.x, fp->options.min_width, fp->options.max_width);
                    float new_h = std::clamp(current_h_ - d.delta.y, fp->options.min_height, fp->options.max_height);
                    current_x_ += (current_w_ - new_w);
                    current_y_ += (current_h_ - new_h);
                    current_w_ = new_w;
                    current_h_ = new_h;
                    auto* rb = getRenderBox();
                    if (rb) rb->setGeometry(current_x_, current_y_, current_w_, current_h_, false);
                    if (fp->options.on_resize_update) fp->options.on_resize_update(current_w_, current_h_);
                },
                .on_pan_end = [this](const DragEndDetails&) {
                    auto* fp = static_cast<const FloatingPanelWidget*>(widget());
                    if (fp->options.on_resized) fp->options.on_resized(current_w_, current_h_);
                }
            }),
            .top = StyleValue::point(0.0f),
            .left = StyleValue::point(0.0f),
            .width = StyleValue::point(th * 2.0f),
            .height = StyleValue::point(th * 2.0f)
        });

        return handles;
    }

    WidgetPtr build(BuildContext&) override {
        auto* w = static_cast<const FloatingPanelWidget*>(widget());
        const auto& opts = w->options;

        // ── 1. Page Body ──────────────────────────────────────────────
        WidgetPtr body_widget;
        if (w->body) {
            body_widget = Positioned::fill(container({
                .width = StyleValue::percent(100.0f),
                .height = StyleValue::percent(100.0f),
                .child = w->body
            }));
        } else {
            body_widget = Positioned::fill(container({
                .width = StyleValue::percent(100.0f),
                .height = StyleValue::percent(100.0f)
            }));
        }

        if (!is_open_) {
            return stack({
                .children = {body_widget},
                .width = StyleValue::percent(100.0f),
                .height = StyleValue::percent(100.0f)
            });
        }

        // ── 2. Inside Content & Panel Card ────────────────────────────
        std::vector<WidgetPtr> panel_column_items;
        panel_column_items.push_back(buildTitleBar(w));

        // When not minimized: render body content inside panel
        if (display_state_ != FloatingPanelDisplayState::Minimized) {
            WidgetPtr inside_content = w->content;
            if (!inside_content) {
                inside_content = text("Panel Content Workspace", { .color = 0xFF94A3B8, .font_size = 13.0f });
            }

            panel_column_items.push_back(container({
                .color = opts.background_color,
                .border_radius = BorderRadius::only(0.0f, 0.0f, opts.border_radius, opts.border_radius),
                .border = Border(opts.border_color, 1.0f),
                .padding = StyleInsets::all(14.0f),
                .flex = 1.0f,
                .child = inside_content
            }));
        }

        Color active_border = is_active_ ? opts.active_border_color : opts.border_color;

        auto panel_card = container({
            .color = opts.background_color,
            .border_radius = BorderRadius::circular(opts.border_radius),
            .border = Border(active_border, 1.0f),
            .box_shadow = {
                BoxShadow(0x99000000, {0.0f, 14.0f}, 36.0f),
                BoxShadow(is_active_ ? 0x2E38BDF8 : 0x00000000, {0.0f, 0.0f}, 20.0f)
            },
            .width = StyleValue::percent(100.0f),
            .height = (display_state_ == FloatingPanelDisplayState::Minimized)
                        ? StyleValue::point(40.0f) : StyleValue::percent(100.0f),
            .child = column({
                .width = StyleValue::percent(100.0f),
                .height = StyleValue::percent(100.0f),
                .children = panel_column_items
            })
        });

        // ── 3. Build Floating Window with Resize Handles ──────────────
        std::vector<WidgetPtr> window_stack_layers;
        window_stack_layers.push_back(panel_card);

        auto resize_handles = buildResizeHandles(w);
        for (auto& h : resize_handles) {
            window_stack_layers.push_back(h);
        }

        auto floating_window = stack({
            .children = window_stack_layers,
            .width = StyleValue::percent(100.0f),
            .height = (display_state_ == FloatingPanelDisplayState::Minimized)
                        ? StyleValue::point(40.0f) : StyleValue::percent(100.0f)
        });

        // Click on panel brings it to front / active state (zero rebuild if already active)
        auto interactive_window = gestureDetector({
            .child = floating_window,
            .on_tap_down = [this](const TapDownDetails&) {
                if (!is_active_) {
                    is_active_ = true;
                    setState([] {});
                }
            }
        });

        // Wrap in FloatingPanelBoxWidget (Zero-rebuild RenderBox)
        auto floating_box = std::make_shared<FloatingPanelBoxWidget>(
            interactive_window,
            current_x_,
            current_y_,
            current_w_,
            (display_state_ == FloatingPanelDisplayState::Minimized) ? 40.0f : current_h_,
            display_state_ == FloatingPanelDisplayState::Minimized,
            render_box_holder_
        );

        // ── 4. Main Stack Composition: Body + Floating Window ─────────
        return stack({
            .children = {
                body_widget,
                floating_box
            },
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f)
        });
    }
};

std::unique_ptr<State> FloatingPanelWidget::createState() {
    return std::make_unique<FloatingPanelState>();
}

} // namespace enki
