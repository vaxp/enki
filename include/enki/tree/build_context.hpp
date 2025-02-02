#pragma once
/// @file build_context.hpp
/// @brief BuildContext — provides access to the element tree during build.
///
/// BuildContext is the interface through which widgets access their
/// environment during the build phase. It provides access to:
///   - Ancestor widgets and elements (for BlocProvider::of, Theme::of, etc.)
///   - The current element's position in the tree
///   - Media queries (screen size)
///   - Theme data
///
/// Every `build()` and `createRenderObject()` call receives a BuildContext.
/// In practice, a BuildContext IS an Element — but the interface restricts
/// what the widget author can access for safety and encapsulation.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include <cassert>
#include <typeinfo>

namespace enki {

// Forward declarations
class Element;
class Widget;
class RenderObject;

/// @brief Context available to widgets during the build phase.
///
/// BuildContext provides safe, scoped access to the element tree.
/// It is used to look up inherited data (themes, BLoCs, providers),
/// query the screen size, and navigate the widget tree.
///
/// @note BuildContext is backed by an Element. All methods delegate
///       to the underlying element. However, widget authors should
///       interact with BuildContext rather than Element directly.
///
/// @code
///   WidgetPtr build(BuildContext& ctx) override {
///       auto& theme = ctx.theme();
///       auto& counter = BlocProvider<CounterCubit>::of(ctx);
///       auto screenSize = ctx.mediaSize();
///       return text("Hello", {.color = theme.on_primary});
///   }
/// @endcode
class BuildContext {
public:
    /// Construct a BuildContext from an Element.
    /// @param element The backing element (must not be null).
    explicit BuildContext(Element* element);
    ~BuildContext() = default;

    // ── Ancestor Lookup ────────────────────────────────────────

    /// Find an ancestor widget of a specific type.
    /// Walks up the element tree and returns the first widget
    /// that matches the requested type.
    ///
    /// @tparam T The widget type to search for.
    /// @return Pointer to the ancestor widget, or nullptr if not found.
    ///
    /// @code
    ///   auto* scaffold = ctx.findAncestorWidgetOfType<ScaffoldWidget>();
    /// @endcode
    template<typename T>
    [[nodiscard]] T* findAncestorWidgetOfType() const;

    /// Find an ancestor element of a specific type.
    ///
    /// @tparam T The element type to search for.
    /// @return Pointer to the ancestor element, or nullptr if not found.
    template<typename T>
    [[nodiscard]] T* findAncestorElementOfType() const;

    /// Find the nearest ancestor RenderObject.
    /// Useful for widgets that need to know their parent's layout.
    ///
    /// @return Pointer to the nearest RenderObject, or nullptr.
    [[nodiscard]] RenderObject* findAncestorRenderObject() const;

    // ── Media Queries ──────────────────────────────────────────

    /// @return The size of the rendering surface (window size).
    [[nodiscard]] Size mediaSize() const;

    // ── Element Access ─────────────────────────────────────────

    /// @return The backing element (for framework-internal use).
    [[nodiscard]] Element* element() const { return element_; }

    /// @return The current widget.
    [[nodiscard]] Widget* widget() const;

    /// @return The depth of this context in the element tree.
    [[nodiscard]] size_t depth() const;

    // ── State Notification ─────────────────────────────────────

    /// Mark the element as needing a rebuild.
    /// Called internally by setState() and BLoC infrastructure.
    void markNeedsBuild();

private:
    Element* element_;
};

// ── Template implementations ────────────────────────────────────
// (These need Element to be fully defined — included in the .cpp
//  or by the user who includes both headers.)

}  // namespace enki
