#pragma once
/// @file draggable.hpp
/// @brief Ultra high-performance Draggable, DragTarget, and DragOverlay widget system for ENKI Framework (Category 9. Gestures / Interaction).
/// Supports zero-rebuild 600+ FPS direct Skia overlay rendering, drag-and-drop payloads, and drop target hit testing.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/state/state.hpp"
#include "enki/rendering/color.hpp"

#include <string>
#include <string_view>
#include <functional>
#include <memory>
#include <vector>
#include <any>
#include <set>

namespace enki {

class RenderDragTarget;
class RenderDragOverlay;

/// ════════════════════════════════════════════════════════════════
/// Global Drag Bus & Session
/// ════════════════════════════════════════════════════════════════

struct DragSession {
    bool is_active = false;
    std::string tag = "";
    std::string preview_label = "";
    std::any data;
    Point current_pointer = {0.0f, 0.0f};
    WidgetPtr feedback;
};

class DragManager {
public:
    static DragManager& instance() {
        static DragManager mgr;
        return mgr;
    }

    DragSession session;
    std::set<RenderDragTarget*> targets;
    RenderDragOverlay* active_overlay = nullptr;
    std::function<void()> on_drag_state_changed;

    void registerTarget(RenderDragTarget* t) { targets.insert(t); }
    void unregisterTarget(RenderDragTarget* t) { targets.erase(t); }

    void startDrag(std::string tag, std::any data, WidgetPtr feedback, Point start_pos, std::string preview_label = "");
    void updatePointer(Point p);
    void endDrag();
};

/// ════════════════════════════════════════════════════════════════
/// Draggable Widget
/// ════════════════════════════════════════════════════════════════

class Draggable : public StatefulWidget {
public:
    std::string tag = "";
    std::string preview_label = "";
    std::any data;
    WidgetPtr child;
    WidgetPtr feedback;
    WidgetPtr child_when_dragging;

    std::function<void()> on_drag_started;
    std::function<void()> on_drag_end;
    std::function<void()> on_drag_completed;

    Draggable() = default;
    Draggable(std::string tag_, std::any data_, WidgetPtr child_, WidgetPtr feedback_ = nullptr,
              WidgetPtr child_when_dragging_ = nullptr, std::string preview_lbl_ = "")
        : tag(std::move(tag_)), preview_label(std::move(preview_lbl_)),
          data(std::move(data_)), child(std::move(child_)),
          feedback(std::move(feedback_)), child_when_dragging(std::move(child_when_dragging_)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "Draggable"; }
};

inline std::shared_ptr<Draggable> draggable(
    std::string tag,
    std::any data,
    WidgetPtr child,
    WidgetPtr feedback = nullptr,
    WidgetPtr child_when_dragging = nullptr,
    std::string preview_label = "") {
    return std::make_shared<Draggable>(std::move(tag), std::move(data), std::move(child),
                                      std::move(feedback), std::move(child_when_dragging),
                                      std::move(preview_label));
}

struct DraggableProps {
    Key key = Key::none();
    WidgetPtr child = nullptr;

    std::string tag = "";
    std::string preview_label = "";
    std::any data;
    WidgetPtr feedback;
    WidgetPtr child_when_dragging;

    std::function<void()> on_drag_started;
    std::function<void()> on_drag_end;
    std::function<void()> on_drag_completed;
};

inline std::shared_ptr<Draggable> draggable(DraggableProps props) {
    auto d = std::make_shared<Draggable>(std::move(props.tag), std::move(props.data), std::move(props.child),
                                         std::move(props.feedback), std::move(props.child_when_dragging),
                                         std::move(props.preview_label));
    d->key = std::move(props.key);
    d->on_drag_started = std::move(props.on_drag_started);
    d->on_drag_end = std::move(props.on_drag_end);
    d->on_drag_completed = std::move(props.on_drag_completed);
    return d;
}

inline std::shared_ptr<Draggable> draggable(DraggableProps props, WidgetPtr child) {
    props.child = std::move(child);
    return draggable(std::move(props));
}

/// ════════════════════════════════════════════════════════════════
/// DragTarget Widget
/// ════════════════════════════════════════════════════════════════

class DragTarget : public StatefulWidget {
public:
    std::string accepted_tag = "";
    using TargetBuilder = std::function<WidgetPtr(BuildContext& context, bool is_hovered, const std::any& candidate_data)>;
    TargetBuilder builder;

    std::function<bool(const std::any& data)> on_will_accept;
    std::function<void(const std::any& data)> on_accept;
    std::function<void()> on_leave;

    DragTarget() = default;
    DragTarget(TargetBuilder builder_, std::function<void(const std::any&)> on_accept_ = nullptr,
               std::string accepted_tag_ = "")
        : accepted_tag(std::move(accepted_tag_)), builder(std::move(builder_)),
          on_accept(std::move(on_accept_)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "DragTarget"; }
};

inline std::shared_ptr<DragTarget> dragTarget(
    DragTarget::TargetBuilder builder,
    std::function<void(const std::any&)> on_accept = nullptr,
    std::string accepted_tag = "") {
    return std::make_shared<DragTarget>(std::move(builder), std::move(on_accept), std::move(accepted_tag));
}

struct DragTargetProps {
    Key key = Key::none();
    std::string accepted_tag = "";
    DragTarget::TargetBuilder builder;

    std::function<bool(const std::any& data)> on_will_accept;
    std::function<void(const std::any& data)> on_accept;
    std::function<void()> on_leave;
};

inline std::shared_ptr<DragTarget> dragTarget(DragTargetProps props) {
    auto dt = std::make_shared<DragTarget>(std::move(props.builder), std::move(props.on_accept), std::move(props.accepted_tag));
    dt->key = std::move(props.key);
    dt->on_will_accept = std::move(props.on_will_accept);
    dt->on_leave = std::move(props.on_leave);
    return dt;
}

/// ════════════════════════════════════════════════════════════════
/// DragOverlay Widget (Direct Canvas Fast Renderer)
/// ════════════════════════════════════════════════════════════════

class DragOverlay : public SingleChildRenderObjectWidget {
public:
    explicit DragOverlay(WidgetPtr child)
        : SingleChildRenderObjectWidget(Key::none(), std::move(child)) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext&) override;
    void updateRenderObject(BuildContext&, RenderObject&) override;
    [[nodiscard]] std::string_view typeName() const override { return "DragOverlay"; }
};

inline std::shared_ptr<DragOverlay> dragOverlay(WidgetPtr child) {
    return std::make_shared<DragOverlay>(std::move(child));
}

} // namespace enki
