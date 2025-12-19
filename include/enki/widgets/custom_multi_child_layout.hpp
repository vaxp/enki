#pragma once
/// @file custom_multi_child_layout.hpp
/// @brief ENKI Section 11: CustomMultiChildLayout widget and MultiChildLayoutDelegate (C++20 Declarative API).
///
/// Provides a layout mechanism that defers the layout of multiple children
/// to a delegate. Each child is identified by a LayoutId widget.
///
/// Features:
///   - Identification of children via layoutId("id", child).
///   - Fine-grained layout control via MultiChildLayoutDelegate:
///       * getSize(constraints)
///       * performLayout(size)
///       * hasChild(id), layoutChild(id, constraints), positionChild(id, offset)
///       * shouldRelayout(oldDelegate)
///   - Functional delegate support via C++20 lambdas (layout_callback).
///   - Designated initializer support with CustomMultiChildLayoutProps.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <functional>
#include <optional>

namespace enki {

// Forward declaration
class RenderCustomMultiChildLayout;

// ════════════════════════════════════════════════════════════════
// LayoutId Widget & Props
// ════════════════════════════════════════════════════════════════

class LayoutIdWidget : public SingleChildRenderObjectWidget {
public:
    std::string id;

    LayoutIdWidget(std::string id_str, WidgetPtr child, Key key = Key::none())
        : SingleChildRenderObjectWidget(std::move(key), std::move(child)), id(std::move(id_str)) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
    [[nodiscard]] std::string_view typeName() const override { return "LayoutId"; }
};

struct LayoutIdProps {
    Key         key = Key::none();
    std::string id;
    WidgetPtr   child = nullptr;

    operator WidgetPtr() const;
};

inline std::shared_ptr<LayoutIdWidget> layoutId(std::string id, WidgetPtr child, Key key = Key::none()) {
    return std::make_shared<LayoutIdWidget>(std::move(id), std::move(child), std::move(key));
}

inline std::shared_ptr<LayoutIdWidget> layoutId(LayoutIdProps props) {
    return std::make_shared<LayoutIdWidget>(std::move(props.id), std::move(props.child), std::move(props.key));
}

inline LayoutIdProps::operator WidgetPtr() const {
    return layoutId(*this);
}

// ════════════════════════════════════════════════════════════════
// MultiChildLayoutDelegate Abstract Base
// ════════════════════════════════════════════════════════════════

class MultiChildLayoutDelegate {
public:
    virtual ~MultiChildLayoutDelegate() = default;

    /// @brief Determine the size of the CustomMultiChildLayout widget given incoming constraints.
    virtual Size getSize(const BoxConstraints& constraints) {
        if (constraints.hasBoundedWidth() && constraints.hasBoundedHeight()) {
            return constraints.biggest();
        }
        return Size{
            constraints.hasBoundedWidth() ? constraints.max_width : 0.0f,
            constraints.hasBoundedHeight() ? constraints.max_height : 0.0f
        };
    }

    /// @brief Perform the layout pass for all identified children.
    virtual void performLayout(Size size) = 0;

    /// @brief Return true if this delegate requires recalculating layout compared to oldDelegate.
    virtual bool shouldRelayout(const MultiChildLayoutDelegate& /*oldDelegate*/) const {
        return false;
    }

    // ── Helper Queries & Mutators for use inside performLayout ───

    /// @brief Check if a child with the specified id exists in this layout.
    [[nodiscard]] bool hasChild(std::string_view id) const;

    /// @brief Measure and layout the child with the given id according to constraints.
    /// @return The resulting size of the child after layout.
    Size layoutChild(std::string_view id, const BoxConstraints& constraints);

    /// @brief Position the child with the given id at the specified local offset.
    void positionChild(std::string_view id, Point offset);

private:
    friend class RenderCustomMultiChildLayout;
    RenderCustomMultiChildLayout* layout_ = nullptr;
};

// ════════════════════════════════════════════════════════════════
// Functional Delegate (Lambda Support)
// ════════════════════════════════════════════════════════════════

using LayoutCallback = std::function<void(MultiChildLayoutDelegate&, Size)>;
using SizeCallback   = std::function<Size(const BoxConstraints&)>;

class FunctionalMultiChildLayoutDelegate : public MultiChildLayoutDelegate {
public:
    LayoutCallback on_layout;
    SizeCallback   on_get_size;

    FunctionalMultiChildLayoutDelegate(LayoutCallback layout, SizeCallback size_cb = nullptr)
        : on_layout(std::move(layout)), on_get_size(std::move(size_cb)) {}

    Size getSize(const BoxConstraints& constraints) override {
        if (on_get_size) {
            return on_get_size(constraints);
        }
        return MultiChildLayoutDelegate::getSize(constraints);
    }

    void performLayout(Size size) override {
        if (on_layout) {
            on_layout(*this, size);
        }
    }

    bool shouldRelayout(const MultiChildLayoutDelegate&) const override {
        return true;
    }
};

// ════════════════════════════════════════════════════════════════
// CustomMultiChildLayout Widget & Props
// ════════════════════════════════════════════════════════════════

class CustomMultiChildLayoutWidget : public MultiChildRenderObjectWidget {
public:
    std::shared_ptr<MultiChildLayoutDelegate> delegate;

    CustomMultiChildLayoutWidget() = default;
    CustomMultiChildLayoutWidget(Key key,
                                 std::shared_ptr<MultiChildLayoutDelegate> del,
                                 std::vector<WidgetPtr> children = {})
        : MultiChildRenderObjectWidget(std::move(key), std::move(children)),
          delegate(std::move(del)) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
    [[nodiscard]] std::string_view typeName() const override { return "CustomMultiChildLayout"; }
};

struct CustomMultiChildLayoutProps {
    Key                                       key = Key::none();
    std::shared_ptr<MultiChildLayoutDelegate> delegate = nullptr;
    LayoutCallback                            layout_callback = nullptr;
    SizeCallback                              size_callback = nullptr;
    std::vector<WidgetPtr>                    children;

    operator WidgetPtr() const;
};

// ════════════════════════════════════════════════════════════════
// Factory Functions
// ════════════════════════════════════════════════════════════════

inline std::shared_ptr<CustomMultiChildLayoutWidget> customMultiChildLayout(CustomMultiChildLayoutProps props = {}) {
    std::shared_ptr<MultiChildLayoutDelegate> del = std::move(props.delegate);
    if (!del && props.layout_callback) {
        del = std::make_shared<FunctionalMultiChildLayoutDelegate>(
            std::move(props.layout_callback),
            std::move(props.size_callback)
        );
    }
    return std::make_shared<CustomMultiChildLayoutWidget>(
        props.key,
        std::move(del),
        std::move(props.children)
    );
}

inline std::shared_ptr<CustomMultiChildLayoutWidget> customMultiChildLayout(
    std::shared_ptr<MultiChildLayoutDelegate> delegate,
    std::vector<WidgetPtr> children,
    Key key = Key::none()) {
    return std::make_shared<CustomMultiChildLayoutWidget>(
        std::move(key),
        std::move(delegate),
        std::move(children)
    );
}

inline CustomMultiChildLayoutProps::operator WidgetPtr() const {
    return customMultiChildLayout(*this);
}

using CustomMultiChildLayout = CustomMultiChildLayoutProps;

} // namespace enki
