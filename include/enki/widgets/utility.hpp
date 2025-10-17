#pragma once
/// @file utility.hpp
/// @brief ENKI Section 20: Utility / Behavioral widgets (C++20 Declarative API).
///
/// Widgets:
///   1. Visibility — Toggles child visibility (show/hide) with maintain options
///   2. IgnorePointer — Controls whether child widgets receive pointer/mouse events
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include <memory>
#include <string_view>

namespace enki {

// ════════════════════════════════════════════════════════════════
// 1. Visibility Widget
// ════════════════════════════════════════════════════════════════

class VisibilityWidget : public StatelessWidget {
public:
    WidgetPtr child = nullptr;
    WidgetPtr replacement = nullptr;
    bool      visible = true;
    bool      maintain_state = false;
    bool      maintain_animation = false;
    bool      maintain_size = false;
    bool      maintain_interactivity = false;

    explicit VisibilityWidget(Key key = Key::none()) : StatelessWidget(std::move(key)) {}

    [[nodiscard]] WidgetPtr build(BuildContext& ctx) override;
    [[nodiscard]] std::string_view typeName() const override { return "Visibility"; }
};

struct VisibilityProps {
    Key       key = Key::none();
    WidgetPtr child = nullptr;
    WidgetPtr replacement = nullptr;
    bool      visible = true;
    bool      maintain_state = false;
    bool      maintain_animation = false;
    bool      maintain_size = false;
    bool      maintain_interactivity = false;
};

struct Visibility {
    Key       key = Key::none();
    WidgetPtr child = nullptr;
    WidgetPtr replacement = nullptr;
    bool      visible = true;
    bool      maintain_state = false;
    bool      maintain_animation = false;
    bool      maintain_size = false;
    bool      maintain_interactivity = false;

    operator WidgetPtr() const {
        auto w = std::make_shared<VisibilityWidget>(key);
        w->child = child;
        w->replacement = replacement;
        w->visible = visible;
        w->maintain_state = maintain_state;
        w->maintain_animation = maintain_animation;
        w->maintain_size = maintain_size;
        w->maintain_interactivity = maintain_interactivity;
        return w;
    }
};

inline WidgetPtr visibility(const VisibilityProps& props) {
    auto w = std::make_shared<VisibilityWidget>(props.key);
    w->child = props.child;
    w->replacement = props.replacement;
    w->visible = props.visible;
    w->maintain_state = props.maintain_state;
    w->maintain_animation = props.maintain_animation;
    w->maintain_size = props.maintain_size;
    w->maintain_interactivity = props.maintain_interactivity;
    return w;
}

inline WidgetPtr visibility(bool visible, WidgetPtr child, WidgetPtr replacement = nullptr) {
    return visibility({
        .child = std::move(child),
        .replacement = std::move(replacement),
        .visible = visible,
    });
}

// ════════════════════════════════════════════════════════════════
// 2. IgnorePointer Widget
// ════════════════════════════════════════════════════════════════

class IgnorePointerWidget : public SingleChildRenderObjectWidget {
public:
    bool ignoring = true;
    bool ignoring_semantics = false;

    IgnorePointerWidget(Key key, bool ign, bool ign_sem, WidgetPtr child)
        : SingleChildRenderObjectWidget(std::move(key), std::move(child)),
          ignoring(ign), ignoring_semantics(ign_sem) {}

    std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
    [[nodiscard]] std::string_view typeName() const override { return "IgnorePointer"; }
};

struct IgnorePointerProps {
    Key       key = Key::none();
    WidgetPtr child = nullptr;
    bool      ignoring = true;
    bool      ignoring_semantics = false;
};

struct IgnorePointer {
    Key       key = Key::none();
    WidgetPtr child = nullptr;
    bool      ignoring = true;
    bool      ignoring_semantics = false;

    operator WidgetPtr() const {
        return std::make_shared<IgnorePointerWidget>(
            key, ignoring, ignoring_semantics, child
        );
    }
};

inline WidgetPtr ignorePointer(const IgnorePointerProps& props) {
    return std::make_shared<IgnorePointerWidget>(
        props.key, props.ignoring, props.ignoring_semantics, props.child
    );
}

inline WidgetPtr ignorePointer(bool ignoring, WidgetPtr child) {
    return ignorePointer({
        .child = std::move(child),
        .ignoring = ignoring,
    });
}

} // namespace enki
