# Flow

> A high-performance, transform-based layout for animated, overlapping, or dynamically positioned children with zero-reflow paint passes.

- **Header File**: `#include "enki/widgets/flow.hpp"`
- **Category**: Section 11: Layout — Extended (Roadmap v0.2.0)
- **Primary Type**: `class FlowWidget`, `struct FlowProps`, `using Flow = FlowProps`
- **Delegate Base**: `class FlowDelegate`, `class FunctionalFlowDelegate`
- **Context Type**: `class FlowPaintingContext`
- **Transform Math**: `struct Matrix4`
- **Helper Function**: `flow(...)`

---

## Overview

`Flow` is designed for ultra-high-performance UI scenarios where children move, rotate, fan out, or overlap dynamically (such as radial action menus, cascading card stacks, or physics-driven ribbons).

Traditional layouts recalculate the entire flexbox tree (triggering reflow in Anu) whenever a child's position changes. In contrast, `Flow` decouples layout from painting:
1. **Layout Phase (`syncLayout`)**: Children are measured and sized **once** using constraints supplied by `FlowDelegate::getConstraintsForChild`.
2. **Paint Phase (`paintChildren`)**: Child positions, rotations, scales, and opacities are applied dynamically during the paint pass via `FlowPaintingContext::paintChild(index, transform, opacity)`.

Because moving or animating children only invalidates the paint phase (`markNeedsPaint`) and never the layout phase (`markNeedsLayout`), `Flow` guarantees rock-solid framerates of **+ FPS**.

### Key Architectural Behaviors:
- **Zero-Reflow Performance**: Animations (e.g. expanding radial menus) completely bypass the Anu Flexbox layout tree.
- **Matrix4 2D/3D Transforms**: Full affine transformation engine supporting translations, Z-rotations, 2D/3D scaling, matrix multiplication (`*`), and direct Skia matrix concatenation (`toSkMatrix9`).
- **Inverse Matrix Hit-Testing**: Even when children are rotated, scaled, or translated arbitrarily, `Flow` maps incoming pointer clicks back to each child's local coordinates via `Matrix4::mapPointInverse()`, providing 100% accurate hit-testing.
- **Functional C++20 Lambdas**: Fast declarative setups via `paint_callback`, `size_callback`, and `constraints_callback`.

---

## C++ API Definition

### Header: `<enki/widgets/flow.hpp>`

```cpp
namespace enki {

// 1. Transformation Math
struct Matrix4 {
    float storage[16]; // Column-major

    static Matrix4 identity();
    static Matrix4 translation(float tx, float ty, float tz = 0.0f);
    static Matrix4 scale(float sx, float sy, float sz = 1.0f);
    static Matrix4 rotationZ(float radians);
    static Matrix4 compose(Point translation, float rotation_rad = 0.0f, float scale_factor = 1.0f);

    Matrix4 operator*(const Matrix4& rhs) const;
    void toSkMatrix9(float out[9]) const;
    [[nodiscard]] Point mapPoint(Point p) const;
    [[nodiscard]] Point mapPointInverse(Point p) const;
};

// 2. Painting Context
class FlowPaintingContext {
public:
    [[nodiscard]] Size size() const;
    [[nodiscard]] size_t childCount() const;
    [[nodiscard]] Size getChildSize(size_t index) const;

    void paintChild(size_t index, Point offset = {0.0f, 0.0f}, float opacity = 1.0f);
    void paintChild(size_t index, const Matrix4& transform, float opacity = 1.0f);
    void paintChild(size_t index, const float matrix9[9], float opacity = 1.0f);
};

// 3. Delegate Base Class
class FlowDelegate {
public:
    virtual ~FlowDelegate() = default;

    virtual Size getSize(const BoxConstraints& constraints);
    virtual BoxConstraints getConstraintsForChild(size_t index, const BoxConstraints& constraints);
    virtual void paintChildren(FlowPaintingContext& context) = 0;
    virtual bool shouldRelayout(const FlowDelegate& oldDelegate) const { return false; }
    virtual bool shouldRepaint(const FlowDelegate& oldDelegate) const { return false; }
};

// 4. Declarative Props
using FlowPaintCallback       = std::function<void(FlowPaintingContext&)>;
using FlowSizeCallback        = std::function<Size(const BoxConstraints&)>;
using FlowConstraintsCallback = std::function<BoxConstraints(size_t, const BoxConstraints&)>;

struct FlowProps {
    Key                     key                  = Key::none();
    std::shared_ptr<FlowDelegate> delegate       = nullptr;
    FlowPaintCallback       paint_callback       = nullptr;
    FlowSizeCallback        size_callback        = nullptr;
    FlowConstraintsCallback constraints_callback = nullptr;
    std::vector<WidgetPtr>  children;

    operator WidgetPtr() const;
};

using Flow = FlowProps;

// Factory Functions:
std::shared_ptr<FlowWidget> flow(FlowProps props = {});
std::shared_ptr<FlowWidget> flow(std::shared_ptr<FlowDelegate> delegate,
                                std::vector<WidgetPtr> children,
                                Key key = Key::none());

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `key` | `Key` | `Key::none()` | Unique identifier for widget tree reconciliation and diffing. |
| `delegate` | `std::shared_ptr<FlowDelegate>` | `nullptr` | Custom delegate instance managing sizing and paint transformations. |
| `paint_callback` | `FlowPaintCallback` | `nullptr` | Inline C++20 lambda `(FlowPaintingContext&)` to paint children. |
| `size_callback` | `FlowSizeCallback` | `nullptr` | Inline C++20 lambda `(const BoxConstraints&)` to compute container size. |
| `constraints_callback` | `FlowConstraintsCallback` | `nullptr` | Inline C++20 lambda `(size_t index, const BoxConstraints&)` for child constraints. |
| `children` | `std::vector<WidgetPtr>` | `{}` | List of child widgets rendered through the flow. |

---

## Real Code Examples

### 1. Radial Fan-Out Speed Dial Menu (From `widgets_demo/flow_demo/main.cpp`)
Animates child action buttons in a circular arc around a central FAB button using matrix transformations:

```cpp
#include "enki/widgets/flow.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include <cmath>

using namespace enki;

class RadialMenuFlowDelegate : public FlowDelegate {
public:
    float radius;
    float start_angle_deg;
    float sweep_angle_deg;
    float open_progress; // 0.0 (closed) to 1.0 (open)
    Point center_origin;

    RadialMenuFlowDelegate(float r, float start_deg, float sweep_deg, float prog, Point origin)
        : radius(r), start_angle_deg(start_deg), sweep_angle_deg(sweep_deg),
          open_progress(prog), center_origin(origin) {}

    Size getSize(const BoxConstraints& constraints) override {
        return Size{
            constraints.hasBoundedWidth() ? constraints.max_width : 640.0f,
            380.0f
        };
    }

    BoxConstraints getConstraintsForChild(size_t index, const BoxConstraints& /*c*/) override {
        // Center button: 64x64, orbital items: 52x52
        return (index == 0) ? BoxConstraints::tight(Size{64.0f, 64.0f})
                            : BoxConstraints::tight(Size{52.0f, 52.0f});
    }

    void paintChildren(FlowPaintingContext& context) override {
        if (context.childCount() == 0) return;

        // 1. Center Hub Button
        Size hub_sz = context.getChildSize(0);
        float hub_x = center_origin.x - (hub_sz.width / 2.0f);
        float hub_y = center_origin.y - (hub_sz.height / 2.0f);
        context.paintChild(0, Point{hub_x, hub_y}, 1.0f);

        // 2. Orbital Action Items
        size_t action_count = context.childCount() - 1;
        if (action_count == 0 || open_progress <= 0.001f) return;

        float current_radius = radius * open_progress;
        float start_rad = start_angle_deg * 3.14159265f / 180.0f;
        float sweep_rad = sweep_angle_deg * 3.14159265f / 180.0f;
        float step = (action_count > 1) ? (sweep_rad / static_cast<float>(action_count - 1)) : 0.0f;

        for (size_t i = 1; i < context.childCount(); ++i) {
            size_t idx = i - 1;
            float angle = start_rad + static_cast<float>(idx) * step;

            Size child_sz = context.getChildSize(i);
            float target_x = center_origin.x + current_radius * std::cos(angle) - (child_sz.width / 2.0f);
            float target_y = center_origin.y + current_radius * std::sin(angle) - (child_sz.height / 2.0f);

            float scale_val = 0.4f + (0.6f * open_progress);
            float rot_rad   = (1.0f - open_progress) * 1.57f;

            auto transform = Matrix4::translation(target_x, target_y)
                           * Matrix4::rotationZ(rot_rad)
                           * Matrix4::scale(scale_val, scale_val);

            float opacity = std::clamp(open_progress * 1.2f, 0.0f, 1.0f);
            context.paintChild(i, transform, opacity);
        }
    }

    bool shouldRepaint(const FlowDelegate& old) const override {
        const auto* o = dynamic_cast<const RadialMenuFlowDelegate*>(&old);
        return !o || o->open_progress != open_progress || o->radius != radius;
    }
};

WidgetPtr buildRadialMenu(bool isOpen, float progress, std::vector<WidgetPtr> items) {
    return flow({
        .key      = Key::string("radial_menu_flow"),
        .delegate = std::make_shared<RadialMenuFlowDelegate>(130.0f, 210.0f, 300.0f, progress, Point{320.0f, 190.0f}),
        .children = std::move(items),
    });
}
```

### 2. Functional Delegation with C++20 Lambdas
Zero-boilerplate layout with inline paint logic:

```cpp
auto waveFlow = flow({
    .paint_callback = [](FlowPaintingContext& ctx) {
        for (size_t i = 0; i < ctx.childCount(); ++i) {
            float x = static_cast<float>(i) * 80.0f + 20.0f;
            float y = 100.0f + 50.0f * std::sin(x * 0.02f);
            ctx.paintChild(i, Matrix4::translation(x, y));
        }
    },
    .children = { item1, item2, item3, item4 },
    .key = Key::string("wave_flow"),
});
```

---

## See Also
- [**CustomMultiChildLayout**](./custom_multi_child_layout.md) — Multi-child layout where children are sized and positioned during layout via ID strings.
- [**Stack**](./stack.md) — Standard 2.5D multi-layer container.
- [**SkiaCanvas**](../Paint%20&%20Visual%20Effects/skia_canvas.md) — Custom immediate-mode Skia 2D drawing.
