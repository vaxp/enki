#pragma once
/// @file draggable.hpp
/// @brief Ultra high-performance Draggable, DragTarget, and DragOverlay widget system for ENKI Framework (Category 9. Gestures / Interaction).
/// Supports zero-rebuild  direct Skia overlay rendering, drag-and-drop payloads, and drop target hit testing.
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
/// Draggable Widget Implementation
/// ════════════════════════════════════════════════════════════════

class DraggableWidget : public StatefulWidget {
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

    DraggableWidget() = default;
    DraggableWidget(std::string tag_, std::any data_, WidgetPtr child_, WidgetPtr feedback_ = nullptr,
                    WidgetPtr child_when_dragging_ = nullptr, std::string preview_lbl_ = "")
        : tag(std::move(tag_)), preview_label(std::move(preview_lbl_)),
          data(std::move(data_)), child(std::move(child_)),
          feedback(std::move(feedback_)), child_when_dragging(std::move(child_when_dragging_)) {}
    DraggableWidget(Key key, std::string tag_, std::any data_, WidgetPtr child_, WidgetPtr feedback_ = nullptr,
                    WidgetPtr child_when_dragging_ = nullptr, std::string preview_lbl_ = "")
        : StatefulWidget(std::move(key)), tag(std::move(tag_)), preview_label(std::move(preview_lbl_)),
          data(std::move(data_)), child(std::move(child_)),
          feedback(std::move(feedback_)), child_when_dragging(std::move(child_when_dragging_)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "Draggable"; }
};

/// ════════════════════════════════════════════════════════════════
/// DragTarget Widget Implementation
/// ════════════════════════════════════════════════════════════════

class DragTargetWidget : public StatefulWidget {
public:
    std::string accepted_tag = "";
    using TargetBuilder = std::function<WidgetPtr(BuildContext& context, bool is_hovered, const std::any& candidate_data)>;
    TargetBuilder builder;

    std::function<bool(const std::any& data)> on_will_accept;
    std::function<void(const std::any& data)> on_accept;
    std::function<void()> on_leave;

    DragTargetWidget() = default;
    DragTargetWidget(TargetBuilder builder_, std::function<void(const std::any&)> on_accept_ = nullptr,
                     std::string accepted_tag_ = "")
        : accepted_tag(std::move(accepted_tag_)), builder(std::move(builder_)),
          on_accept(std::move(on_accept_)) {}
    DragTargetWidget(Key key, TargetBuilder builder_, std::function<void(const std::any&)> on_accept_ = nullptr,
                     std::string accepted_tag_ = "")
        : StatefulWidget(std::move(key)), accepted_tag(std::move(accepted_tag_)), builder(std::move(builder_)),
          on_accept(std::move(on_accept_)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "DragTarget"; }
};

/// ════════════════════════════════════════════════════════════════
/// DragOverlay Widget Implementation (Direct Canvas Fast Renderer)
/// ════════════════════════════════════════════════════════════════

class DragOverlayWidget : public SingleChildRenderObjectWidget {
public:
    explicit DragOverlayWidget(WidgetPtr child)
        : SingleChildRenderObjectWidget(Key::none(), std::move(child)) {}
    DragOverlayWidget(Key key, WidgetPtr child)
        : SingleChildRenderObjectWidget(std::move(key), std::move(child)) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext&) override;
    void updateRenderObject(BuildContext&, RenderObject&) override;
    [[nodiscard]] std::string_view typeName() const override { return "DragOverlay"; }
};

/// ════════════════════════════════════════════════════════════════
/// Declarative Proxy Structs (C++20 Designated Initializers)
/// ════════════════════════════════════════════════════════════════

struct Draggable {
    Key key = Key::none();
    std::string tag = "";
    std::string preview_label = "";
    std::any data;
    WidgetPtr child = nullptr;
    WidgetPtr feedback = nullptr;
    WidgetPtr child_when_dragging = nullptr;

    std::function<void()> on_drag_started = nullptr;
    std::function<void()> on_drag_end = nullptr;
    std::function<void()> on_drag_completed = nullptr;

    operator WidgetPtr() const {
        auto w = std::make_shared<DraggableWidget>(key, tag, data, child, feedback, child_when_dragging, preview_label);
        w->on_drag_started = on_drag_started;
        w->on_drag_end = on_drag_end;
        w->on_drag_completed = on_drag_completed;
        return w;
    }
};

struct DragTarget {
    Key key = Key::none();
    std::string accepted_tag = "";
    using TargetBuilder = std::function<WidgetPtr(BuildContext& context, bool is_hovered, const std::any& candidate_data)>;
    TargetBuilder builder = nullptr;

    std::function<bool(const std::any& data)> on_will_accept = nullptr;
    std::function<void(const std::any& data)> on_accept = nullptr;
    std::function<void()> on_leave = nullptr;

    operator WidgetPtr() const {
        auto w = std::make_shared<DragTargetWidget>(key, builder, on_accept, accepted_tag);
        w->on_will_accept = on_will_accept;
        w->on_leave = on_leave;
        return w;
    }
};

struct DragOverlay {
    Key key = Key::none();
    WidgetPtr child = nullptr;

    operator WidgetPtr() const {
        return std::make_shared<DragOverlayWidget>(key, child);
    }
};

} // namespace enki
