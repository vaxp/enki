# CustomMultiChildLayout

> A layout container that defers the sizing and positioning of multiple children to a custom delegate, where children are identified by string IDs.

- **Header File**: `#include "enki/widgets/custom_multi_child_layout.hpp"`
- **Category**: Section 11: Layout — Extended (Roadmap v0.2.0)
- **Primary Type**: `class CustomMultiChildLayoutWidget`, `struct CustomMultiChildLayoutProps`, `using CustomMultiChildLayout = CustomMultiChildLayoutProps`
- **Delegate Base**: `class MultiChildLayoutDelegate`
- **Identification Helper**: `layoutId("id", child)`

---

## Overview

`CustomMultiChildLayout` enables complex, non-standard multi-child layout algorithms that cannot be modeled by standard Flexbox (`Row`, `Column`) or `Stack`. Each child is wrapped with a `layoutId("id", child)` widget. A layout delegate then executes custom C++ logic to:
1. Determine the overall container size (`getSize`).
2. Query which children exist (`hasChild`).
3. Pass custom constraints to individual children and read back their computed sizes (`layoutChild`).
4. Position each child at a precise 2D offset (`positionChild`).
5. Determine when layout recalculation is necessary (`shouldRelayout`).

### Key Architectural Behaviors:
- **Child Identification via `layoutId`**: Children are identified by string identifiers, allowing the delegate to position dependent elements relative to one another regardless of tree order.
- **Dependent Sizing and Positioning**: For example, a badge or floating button can be laid out after measuring a card or header, positioning it exactly on the seam line.
- **Lambda Support**: Supports quick inline declarative layouts via `layout_callback` and `size_callback` without subclassing.
- **Accurate Hit-Testing**: Hit-testing automatically follows the local positions assigned in `positionChild`.

---

## C++ API Definition

### Header: `<enki/widgets/custom_multi_child_layout.hpp>`

```cpp
namespace enki {

// 1. Layout Identification
inline std::shared_ptr<LayoutIdWidget> layoutId(std::string id, WidgetPtr child, Key key = Key::none());

// 2. Delegate Base Class
class MultiChildLayoutDelegate {
public:
    virtual ~MultiChildLayoutDelegate() = default;

    virtual Size getSize(const BoxConstraints& constraints);
    virtual void performLayout(Size size) = 0;
    virtual bool shouldRelayout(const MultiChildLayoutDelegate& oldDelegate) const { return false; }

    // Helpers inside performLayout:
    [[nodiscard]] bool hasChild(std::string_view id) const;
    Size layoutChild(std::string_view id, const BoxConstraints& constraints);
    void positionChild(std::string_view id, Point offset);
};

// 3. Declarative Props
struct CustomMultiChildLayoutProps {
    Key                                       key             = Key::none();
    std::shared_ptr<MultiChildLayoutDelegate> delegate        = nullptr;
    LayoutCallback                            layout_callback = nullptr;
    SizeCallback                              size_callback   = nullptr;
    std::vector<WidgetPtr>                    children;

    operator WidgetPtr() const;
};

using CustomMultiChildLayout = CustomMultiChildLayoutProps;

// Factory Functions:
std::shared_ptr<CustomMultiChildLayoutWidget> customMultiChildLayout(CustomMultiChildLayoutProps props = {});
std::shared_ptr<CustomMultiChildLayoutWidget> customMultiChildLayout(
    std::shared_ptr<MultiChildLayoutDelegate> delegate,
    std::vector<WidgetPtr> children,
    Key key = Key::none());

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `key` | `Key` | `Key::none()` | Unique identifier for widget tree reconciliation and performance diffing. |
| `delegate` | `std::shared_ptr<MultiChildLayoutDelegate>` | `nullptr` | Custom delegate instance implementing `performLayout` and `getSize`. |
| `layout_callback` | `LayoutCallback` | `nullptr` | Optional inline C++20 lambda `(MultiChildLayoutDelegate&, Size)` for functional delegation. |
| `size_callback` | `SizeCallback` | `nullptr` | Optional inline C++20 lambda `(const BoxConstraints&)` to compute container size. |
| `children` | `std::vector<WidgetPtr>` | `{}` | List of children, typically wrapped in `layoutId("id", child)`. |

---

## Real Code Examples

### 1. FAB Seam Anchor Layout (From `widgets_demo/custom_multi_child_layout_demo/main.cpp`)
Positions a Floating Action Button exactly half-way over the dividing line between a header and body:

```cpp
#include "enki/widgets/custom_multi_child_layout.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

class FabAnchorLayoutDelegate : public MultiChildLayoutDelegate {
public:
    float header_height;
    float fab_margin_right;

    FabAnchorLayoutDelegate(float h_height, float fab_mr)
        : header_height(h_height), fab_margin_right(fab_mr) {}

    Size getSize(const BoxConstraints& constraints) override {
        return Size{
            constraints.hasBoundedWidth() ? constraints.max_width : 640.0f,
            380.0f
        };
    }

    void performLayout(Size size) override {
        // 1. Header card
        if (hasChild("header")) {
            layoutChild("header", BoxConstraints::tightFor(size.width, header_height));
            positionChild("header", Point{0.0f, 0.0f});
        }

        // 2. Body card below header
        float body_y = header_height + 14.0f;
        if (hasChild("body")) {
            layoutChild("body", BoxConstraints::tightFor(size.width, size.height - body_y));
            positionChild("body", Point{0.0f, body_y});
        }

        // 3. FAB anchored directly on the seam line
        if (hasChild("fab")) {
            Size fab_sz = layoutChild("fab", BoxConstraints::tight(Size{56.0f, 56.0f}));
            Point fab_pos = {
                size.width - fab_sz.width - fab_margin_right,
                header_height + 7.0f - (fab_sz.height / 2.0f)
            };
            positionChild("fab", fab_pos);
        }
    }

    bool shouldRelayout(const MultiChildLayoutDelegate& old) const override {
        const auto* o = dynamic_cast<const FabAnchorLayoutDelegate*>(&old);
        return !o || o->header_height != header_height || o->fab_margin_right != fab_margin_right;
    }
};

WidgetPtr buildFabAnchorScene(float headerHeight, float fabMargin) {
    return customMultiChildLayout({
        .key      = Key::string("fab_anchor_layout"),
        .delegate = std::make_shared<FabAnchorLayoutDelegate>(headerHeight, fabMargin),
        .children = {
            layoutId("header", container({ .color = 0xFF1E293B, .child = text("Header") })),
            layoutId("body",   container({ .color = 0xFF0F172A, .child = text("Body") })),
            layoutId("fab",    container({ .color = 0xFFEC4899, .shape = BoxShape::Circle, .child = text("＋") })),
        },
    });
}
```

### 2. Functional Delegation with C++20 Lambdas
Fast prototyping without writing a separate class:

```cpp
auto quickLayout = customMultiChildLayout({
    .layout_callback = [](MultiChildLayoutDelegate& del, Size size) {
        if (del.hasChild("title")) {
            del.layoutChild("title", BoxConstraints::tightFor(size.width, 40.0f));
            del.positionChild("title", Point{0.0f, 0.0f});
        }
    },
    .children = {
        layoutId("title", text("Hello World")),
    },
});
```

---

## See Also
- [**Flow**](./flow.md) — High-performance transform-based layout for animated/overlapping children.
- [**Stack**](./stack.md) — Standard overlapping container using alignment or absolute coordinates.
