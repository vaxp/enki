/// @file draggable.cpp
/// @brief Ultra high-performance implementation of Draggable, DragTarget, and DragOverlay for ENKI Framework.

#include "enki/widgets/draggable.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/container.hpp"
#include "enki/rendering/canvas.hpp"
#include "enki/rendering/paint.hpp"
#include "enki/state/state.hpp"
#include "enki/tree/build_context.hpp"

#include <iostream>

namespace enki {

// ════════════════════════════════════════════════════════════════
// RenderDragTarget
// ════════════════════════════════════════════════════════════════

class RenderDragTarget : public RenderBox {
public:
    std::string accepted_tag;
    std::function<bool(const std::any&)> on_will_accept;
    std::function<void(const std::any&)> on_accept;
    std::function<void()> on_leave;
    std::function<void(bool)> on_hover_changed;
    bool is_hovered = false;

    RenderDragTarget() {
        DragManager::instance().registerTarget(this);
    }

    ~RenderDragTarget() override {
        DragManager::instance().unregisterTarget(this);
    }

    void paint(PaintContext& context) override {
        for (auto* child : children_) {
            if (child) child->paint(context);
        }
    }
};

class DragTargetRenderWidget : public SingleChildRenderObjectWidget {
public:
    std::string accepted_tag;
    std::function<bool(const std::any&)> on_will_accept;
    std::function<void(const std::any&)> on_accept;
    std::function<void()> on_leave;
    std::function<void(bool)> on_hover_changed;

    DragTargetRenderWidget(WidgetPtr child, std::string tag,
                           std::function<bool(const std::any&)> will_acc,
                           std::function<void(const std::any&)> acc,
                           std::function<void()> leave,
                           std::function<void(bool)> h_changed)
        : SingleChildRenderObjectWidget(Key::none(), std::move(child)),
          accepted_tag(std::move(tag)), on_will_accept(std::move(will_acc)),
          on_accept(std::move(acc)), on_leave(std::move(leave)),
          on_hover_changed(std::move(h_changed)) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext&) override {
        auto r = std::make_unique<RenderDragTarget>();
        r->accepted_tag = accepted_tag;
        r->on_will_accept = on_will_accept;
        r->on_accept = on_accept;
        r->on_leave = on_leave;
        r->on_hover_changed = on_hover_changed;
        return r;
    }

    void updateRenderObject(BuildContext&, RenderObject& ro) override {
        auto& r = static_cast<RenderDragTarget&>(ro);
        r.accepted_tag = accepted_tag;
        r.on_will_accept = on_will_accept;
        r.on_accept = on_accept;
        r.on_leave = on_leave;
        r.on_hover_changed = on_hover_changed;
    }

    [[nodiscard]] std::string_view typeName() const override { return "DragTargetRenderWidget"; }
};

// ════════════════════════════════════════════════════════════════
// RenderDragOverlay (Direct Skia Canvas Render Object)
// ════════════════════════════════════════════════════════════════

class RenderDragOverlay : public RenderBox {
public:
    RenderDragOverlay() {
        DragManager::instance().active_overlay = this;
    }

    ~RenderDragOverlay() override {
        if (DragManager::instance().active_overlay == this) {
            DragManager::instance().active_overlay = nullptr;
        }
    }

    void paint(PaintContext& ctx) override {
        // 1. Paint underlying page children without any rebuild
        for (auto* child : children_) {
            if (child) child->paint(ctx);
        }

        // 2. Direct fast paint for active floating drag card (zero layout / zero widget allocations)
        if (DragManager::instance().session.is_active) {
            Point p = DragManager::instance().session.current_pointer;
            Rect card_rect = Rect::fromLTWH(p.x - 110.0f, p.y - 20.0f, 220.0f, 40.0f);

            // Card Background
            Paint bg_paint;
            bg_paint.setColor(0xFA0F172A); // Deep slate
            ctx.canvas.drawRRect(card_rect, BorderRadius::circular(8.0f), bg_paint);

            // Glowing cyan border
            Paint border_paint;
            border_paint.setColor(0xFF38BDF8); // Sky 400
            border_paint.setStyle(PaintStyle::Stroke);
            border_paint.setStrokeWidth(2.0f);
            ctx.canvas.drawRRect(card_rect, BorderRadius::circular(8.0f), border_paint);

            // Card title text
            std::string label = DragManager::instance().session.preview_label.empty()
                                    ? "Dragging Card..."
                                    : DragManager::instance().session.preview_label;
            Paint text_paint;
            text_paint.setColor(0xFFFFFFFF);
            Point text_pos = Point(card_rect.x + 12.0f, card_rect.y + 25.0f);
            ctx.canvas.drawText(label, text_pos, text_paint, 12.0f, nullptr, true);
        }
    }
};

std::unique_ptr<RenderObject> DragOverlayWidget::createRenderObject(BuildContext&) {
    return std::make_unique<RenderDragOverlay>();
}

void DragOverlayWidget::updateRenderObject(BuildContext&, RenderObject&) {}

// ════════════════════════════════════════════════════════════════
// DragManager Implementation
// ════════════════════════════════════════════════════════════════

void DragManager::startDrag(std::string tag, std::any data, WidgetPtr feedback, Point start_pos, std::string preview_label) {
    session.is_active = true;
    session.tag = std::move(tag);
    session.preview_label = std::move(preview_label);
    session.data = std::move(data);
    session.feedback = std::move(feedback);
    session.current_pointer = start_pos;

    if (active_overlay) {
        active_overlay->markNeedsPaint();
    }
}

void DragManager::updatePointer(Point p) {
    if (!session.is_active) return;
    session.current_pointer = p;

    for (auto* t : targets) {
        Rect gb = t->globalBounds();
        bool contains = gb.contains(p);
        bool tag_matches = t->accepted_tag.empty() || t->accepted_tag == session.tag;
        bool accepts = tag_matches && (!t->on_will_accept || t->on_will_accept(session.data));

        if (contains && accepts) {
            if (!t->is_hovered) {
                t->is_hovered = true;
                if (t->on_hover_changed) t->on_hover_changed(true);
            }
        } else {
            if (t->is_hovered) {
                t->is_hovered = false;
                if (t->on_leave) t->on_leave();
                if (t->on_hover_changed) t->on_hover_changed(false);
            }
        }
    }

    // Only trigger ultra-fast repaint on overlay without tree rebuilds
    if (active_overlay) {
        active_overlay->markNeedsPaint();
    }
}

void DragManager::endDrag() {
    if (!session.is_active) return;

    for (auto* t : targets) {
        if (t->is_hovered) {
            if (t->on_accept) t->on_accept(session.data);
            t->is_hovered = false;
            if (t->on_hover_changed) t->on_hover_changed(false);
        }
    }

    session.is_active = false;
    session.feedback = nullptr;

    if (active_overlay) {
        active_overlay->markNeedsPaint();
    }
}

// ════════════════════════════════════════════════════════════════
// Draggable State
// ════════════════════════════════════════════════════════════════

class DraggableState : public State {
private:
    bool is_dragging_ = false;

public:
    WidgetPtr build(BuildContext&) override {
        auto* w = static_cast<const DraggableWidget*>(widget());

        WidgetPtr display_child;
        if (is_dragging_) {
            if (w->child_when_dragging) {
                display_child = w->child_when_dragging;
            } else {
                auto placeholder = container({
                    .color = 0x33334155,
                    .border_radius = BorderRadius::circular(8.0f),
                    .child = w->child,
                });
                display_child = placeholder;
            }
        } else {
            display_child = w->child;
        }

        return gestureDetector({
            .child = display_child,
            .cursor_type = SystemCursor::Move,
            .on_pan_start = [this, w](const DragStartDetails& d) {
                is_dragging_ = true;
                DragManager::instance().startDrag(w->tag, w->data, w->feedback ? w->feedback : w->child,
                                                d.global_position, w->preview_label);
                if (w->on_drag_started) w->on_drag_started();
                setState([] {});
            },
            .on_pan_update = [](const DragUpdateDetails& d) {
                DragManager::instance().updatePointer(d.global_position);
            },
            .on_pan_end = [this, w](const DragEndDetails&) {
                is_dragging_ = false;
                DragManager::instance().endDrag();
                if (w->on_drag_end) w->on_drag_end();
                setState([] {});
            },
        });
    }
};

std::unique_ptr<State> DraggableWidget::createState() {
    return std::make_unique<DraggableState>();
}

// ════════════════════════════════════════════════════════════════
// DragTarget State
// ════════════════════════════════════════════════════════════════

class DragTargetState : public State {
private:
    bool is_hovered_ = false;

public:
    WidgetPtr build(BuildContext& context) override {
        auto* w = static_cast<const DragTargetWidget*>(widget());

        WidgetPtr target_content;
        if (w->builder) {
            target_content = w->builder(context, is_hovered_, DragManager::instance().session.data);
        } else {
            target_content = container();
        }

        auto target_widget = std::make_shared<DragTargetRenderWidget>(
            target_content, w->accepted_tag, w->on_will_accept, w->on_accept, w->on_leave,
            [this](bool h) {
                is_hovered_ = h;
                setState([] {});
            }
        );

        return target_widget;
    }
};

std::unique_ptr<State> DragTargetWidget::createState() {
    return std::make_unique<DragTargetState>();
}

} // namespace enki
