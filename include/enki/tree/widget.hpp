#pragma once
/// @file widget.hpp
/// @brief Widget base classes — the declarative description layer.
///
/// Widgets are lightweight, immutable descriptions of a piece of the UI.
/// They form the first tree in ENKI's Three-Tree Architecture:
///
///   Widget Tree (declarative) → Element Tree (lifecycle) → RenderObject Tree (layout + paint)
///
/// Widget classes:
///   - Widget:                      Abstract base for all widgets.
///   - StatelessWidget:             A widget that has no mutable state.
///   - StatefulWidget:              A widget that has mutable State.
///   - RenderObjectWidget:          A widget that directly creates a RenderObject.
///   - SingleChildRenderObjectWidget: RenderObjectWidget with exactly one child.
///   - MultiChildRenderObjectWidget:  RenderObjectWidget with multiple children.
///
/// Design principles:
///   - Widgets are cheap to create (constructed every frame during rebuild).
///   - Widgets are immutable — their configuration never changes after construction.
///   - All mutable state lives in Element (lifecycle) or State (user state).
///   - Widgets describe WHAT to render, not HOW to render.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/key.hpp"
#include <memory>
#include <vector>
#include <string_view>
#include <typeinfo>
#include <functional>

namespace enki {

// Forward declarations
class Element;
class State;
class BuildContext;
class RenderObject;

/// @brief Abstract base class for all widgets in the ENKI framework.
///
/// A Widget is an immutable description of part of a user interface.
/// Widgets are cheap to construct and are rebuilt frequently (potentially
/// every frame). The framework uses the widget tree to diff against the
/// previous tree and update only what changed.
///
/// Every Widget subclass must implement:
///   - `createElement()`: Creates the Element that manages this widget's lifecycle.
///   - `typeName()`:      Returns a stable string identifier for this widget type.
///
/// @note Widgets should NOT hold mutable state. Use StatefulWidget + State for
///       widgets that need to change over time.
class Widget : public std::enable_shared_from_this<Widget> {
public:
    virtual ~Widget() = default;

    // ── Core Interface ─────────────────────────────────────────

    /// Create the Element that will manage this widget in the element tree.
    /// Each widget type creates a specialized element (StatelessElement,
    /// StatefulElement, RenderObjectElement, etc.).
    ///
    /// @return A unique_ptr to the newly created element.
    [[nodiscard]] virtual std::unique_ptr<Element> createElement() = 0;

    /// A stable type name for this widget class. Used during reconciliation
    /// to determine whether two widgets are of the same type.
    ///
    /// @return A string_view that uniquely identifies this widget class.
    [[nodiscard]] virtual std::string_view typeName() const = 0;

    // ── Reconciliation ─────────────────────────────────────────

    /// Determine whether an existing Element (created by `oldWidget`) can be
    /// updated to reflect this widget's new configuration.
    ///
    /// The default implementation returns true if both widgets have the same
    /// runtime type (via typeName()) and equal keys.
    ///
    /// @param oldWidget The widget that the existing element was created from.
    /// @return true if the element can be updated in place.
    [[nodiscard]] virtual bool canUpdate(const Widget& oldWidget) const;

    // ── Identity ───────────────────────────────────────────────

    /// The key for this widget, used during reconciliation.
    /// Widgets with keys are matched by key rather than position.
    Key key = Key::none();

    // ── Debug ──────────────────────────────────────────────────

    /// @return A human-readable description for debugging.
    [[nodiscard]] virtual std::string debugDescription() const;

protected:
    Widget() = default;
    explicit Widget(Key k) : key(std::move(k)) {}
};

using WidgetPtr = std::shared_ptr<Widget>;

// ════════════════════════════════════════════════════════════════
// StatelessWidget
// ════════════════════════════════════════════════════════════════

/// @brief A widget that does not have any mutable state.
///
/// StatelessWidget's UI is determined entirely by its constructor arguments.
/// The `build()` method is called once per rebuild to produce the child widget tree.
///
/// Usage:
/// @code
///   class Greeting : public StatelessWidget {
///   public:
///       std::string name;
///
///       Greeting(std::string n) : name(std::move(n)) {}
///
///       WidgetPtr build(BuildContext& ctx) override {
///           return text("Hello, " + name + "!");
///       }
///
///       std::string_view typeName() const override { return "Greeting"; }
///   };
/// @endcode
class StatelessWidget : public Widget {
public:
    /// Build the widget tree for this stateless widget.
    /// Called every time the widget is rebuilt.
    ///
    /// @param ctx The build context, providing access to inherited data,
    ///            ancestor widgets, and theme information.
    /// @return The child widget tree.
    [[nodiscard]] virtual WidgetPtr build(BuildContext& ctx) = 0;

    /// Creates a StatelessElement to manage this widget.
    [[nodiscard]] std::unique_ptr<Element> createElement() override;

protected:
    StatelessWidget() = default;
    explicit StatelessWidget(Key k) : Widget(std::move(k)) {}
};

// ════════════════════════════════════════════════════════════════
// StatefulWidget
// ════════════════════════════════════════════════════════════════

/// @brief A widget that has mutable state managed by a State object.
///
/// The widget itself remains immutable; all mutable data is stored in
/// the State object returned by `createState()`. The State persists
/// across rebuilds as long as the element remains in the tree.
///
/// Usage:
/// @code
///   class Counter : public StatefulWidget {
///   public:
///       std::unique_ptr<State> createState() override {
///           return std::make_unique<CounterState>();
///       }
///       std::string_view typeName() const override { return "Counter"; }
///   };
///
///   class CounterState : public State {
///       int count_ = 0;
///   public:
///       WidgetPtr build(BuildContext& ctx) override {
///           return text(std::to_string(count_));
///       }
///   };
/// @endcode
class StatefulWidget : public Widget {
public:
    /// Create the State object for this widget.
    /// Called exactly once when the element is first mounted.
    /// The State object persists across rebuilds.
    ///
    /// @return A unique_ptr to the newly created state.
    [[nodiscard]] virtual std::unique_ptr<State> createState() = 0;

    /// Creates a StatefulElement to manage this widget and its state.
    [[nodiscard]] std::unique_ptr<Element> createElement() override;

protected:
    StatefulWidget() = default;
    explicit StatefulWidget(Key k) : Widget(std::move(k)) {}
};

// ════════════════════════════════════════════════════════════════
// RenderObjectWidget
// ════════════════════════════════════════════════════════════════

/// @brief A widget that creates a RenderObject for layout and painting.
///
/// RenderObjectWidgets are the "leaf" widgets that actually produce visual
/// output. They create and configure RenderObjects, which handle the
/// actual layout computation and painting.
///
/// Most built-in widgets (Container, Text, Image) extend RenderObjectWidget
/// or its single-child / multi-child variants.
///
/// @see SingleChildRenderObjectWidget
/// @see MultiChildRenderObjectWidget
class RenderObjectWidget : public Widget {
public:
    /// Create the RenderObject for this widget.
    /// Called when the element is first mounted.
    ///
    /// @param ctx The build context.
    /// @return A unique_ptr to the newly created render object.
    [[nodiscard]] virtual std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) = 0;

    /// Update an existing RenderObject with this widget's new configuration.
    /// Called when the widget is rebuilt and the element is reused.
    ///
    /// The default implementation is a no-op. Subclasses should override
    /// to copy relevant properties from the widget to the render object.
    ///
    /// @param ctx           The build context.
    /// @param renderObject  The existing render object to update.
    virtual void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) {}

    /// Clean up the render object when the widget is about to be removed.
    /// Optional override for widgets that need to release resources.
    ///
    /// @param renderObject  The render object being detached.
    virtual void didUnmountRenderObject(RenderObject& renderObject) {}

    /// Creates a RenderObjectElement to manage this widget.
    [[nodiscard]] std::unique_ptr<Element> createElement() override;

protected:
    RenderObjectWidget() = default;
    explicit RenderObjectWidget(Key k) : Widget(std::move(k)) {}
};

// ════════════════════════════════════════════════════════════════
// SingleChildRenderObjectWidget
// ════════════════════════════════════════════════════════════════

/// @brief A RenderObjectWidget with exactly one child.
///
/// Commonly used for wrapper widgets like Container, Padding, etc.
class SingleChildRenderObjectWidget : public RenderObjectWidget {
public:
    /// The single child widget (may be nullptr for leaf widgets like Text).
    WidgetPtr child;

    /// Creates a SingleChildRenderObjectElement.
    [[nodiscard]] std::unique_ptr<Element> createElement() override;

protected:
    SingleChildRenderObjectWidget() = default;
    explicit SingleChildRenderObjectWidget(Key k) : RenderObjectWidget(std::move(k)) {}
    SingleChildRenderObjectWidget(Key k, WidgetPtr child)
        : RenderObjectWidget(std::move(k)), child(std::move(child)) {}
};

// ════════════════════════════════════════════════════════════════
// MultiChildRenderObjectWidget
// ════════════════════════════════════════════════════════════════

/// @brief A RenderObjectWidget with multiple children.
///
/// Used for layout widgets like Row, Column, Stack, etc.
class MultiChildRenderObjectWidget : public RenderObjectWidget {
public:
    /// The child widgets.
    std::vector<WidgetPtr> children;

    /// Creates a MultiChildRenderObjectElement.
    [[nodiscard]] std::unique_ptr<Element> createElement() override;

protected:
    MultiChildRenderObjectWidget() = default;
    explicit MultiChildRenderObjectWidget(Key k) : RenderObjectWidget(std::move(k)) {}
    MultiChildRenderObjectWidget(Key k, std::vector<WidgetPtr> children)
        : RenderObjectWidget(std::move(k)), children(std::move(children)) {}
};

// ════════════════════════════════════════════════════════════════
// ProxyWidget — for InheritedWidget / Provider pattern
// ════════════════════════════════════════════════════════════════

/// @brief A widget that provides data to its descendants.
///
/// ProxyWidget is the base for InheritedWidget-style data propagation.
/// It wraps a child and provides data that descendant widgets can access
/// through the BuildContext.
class ProxyWidget : public Widget {
public:
    /// The child widget.
    WidgetPtr child;

    /// Creates a ProxyElement.
    [[nodiscard]] std::unique_ptr<Element> createElement() override;

    /// Called when dependents should be notified.
    /// @param oldWidget The previous widget configuration.
    /// @return true if dependents should be rebuilt.
    [[nodiscard]] virtual bool updateShouldNotify(const ProxyWidget& oldWidget) const {
        return true;
    }

protected:
    ProxyWidget() = default;
    ProxyWidget(Key k, WidgetPtr child)
        : Widget(std::move(k)), child(std::move(child)) {}
};

}  // namespace enki
