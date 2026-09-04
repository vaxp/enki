#pragma once
/// @file flow.hpp
/// @brief ENKI Section 11: Flow widget and FlowDelegate (C++20 Declarative API).
///
/// Provides an extremely high-performance layout mechanism that separates child sizing
/// from child positioning and transforms during painting. Children are sized once during
/// the layout pass, while translation, rotation, scale, and opacity are computed on-the-fly
/// in paintChildren via FlowPaintingContext.
///
/// Features:
///   - Matrix4 transformations with 2D/3D affine translation, rotation, scale, and inverse mapping.
///   - Zero-reflow / Zero-relayout performance: animations update transforms during paint,
///     completely bypassing Anu Flexbox reflow and guaranteeing 800+ to .
///   - Precise hit testing: clicks and pointer events map through the inverse matrix
///     of each child's transform at paint time.
///   - Reusable class-based FlowDelegate or inline C++20 functional lambdas.
///   - Strict designated initializer syntax (FlowProps).
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/tree/render_object.hpp"
#include <memory>
#include <vector>
#include <functional>
#include <cmath>
#include <algorithm>
#include <string_view>

namespace enki {

// Forward declarations
class RenderFlow;
class FlowPaintingContext;

// ════════════════════════════════════════════════════════════════
// Matrix4 Math Utility for Flow Transforms
// ════════════════════════════════════════════════════════════════

/// @brief 4x4 Transformation Matrix (stored in column-major order)
///        tailored for high-performance 2D/3D visual layout transforms.
struct Matrix4 {
    float storage[16] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

    static Matrix4 identity() {
        return Matrix4{};
    }

    static Matrix4 translation(float tx, float ty, float tz = 0.0f) {
        Matrix4 m;
        m.storage[12] = tx;
        m.storage[13] = ty;
        m.storage[14] = tz;
        return m;
    }

    static Matrix4 scale(float sx, float sy, float sz = 1.0f) {
        Matrix4 m;
        m.storage[0] = sx;
        m.storage[5] = sy;
        m.storage[10] = sz;
        return m;
    }

    static Matrix4 rotationZ(float radians) {
        Matrix4 m;
        float c = std::cos(radians);
        float s = std::sin(radians);
        m.storage[0] = c;
        m.storage[1] = s;
        m.storage[4] = -s;
        m.storage[5] = c;
        return m;
    }

    static Matrix4 compose(Point translation, float rotation_rad = 0.0f, float scale_factor = 1.0f) {
        Matrix4 m;
        float c = std::cos(rotation_rad) * scale_factor;
        float s = std::sin(rotation_rad) * scale_factor;
        m.storage[0] = c;
        m.storage[1] = s;
        m.storage[4] = -s;
        m.storage[5] = c;
        m.storage[12] = translation.x;
        m.storage[13] = translation.y;
        return m;
    }

    Matrix4 operator*(const Matrix4& rhs) const {
        Matrix4 res;
        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 4; ++col) {
                float sum = 0.0f;
                for (int k = 0; k < 4; ++k) {
                    sum += storage[k * 4 + row] * rhs.storage[col * 4 + k];
                }
                res.storage[col * 4 + row] = sum;
            }
        }
        return res;
    }

    /// @brief Convert this 4x4 matrix to a 9-element 3x3 SkMatrix buffer (row-major).
    void toSkMatrix9(float out[9]) const {
        out[0] = storage[0];   // scaleX
        out[1] = storage[4];   // skewX
        out[2] = storage[12];  // transX
        out[3] = storage[1];   // skewY
        out[4] = storage[5];   // scaleY
        out[5] = storage[13];  // transY
        out[6] = storage[3];   // persp0
        out[7] = storage[7];   // persp1
        out[8] = storage[15];  // persp2
    }

    /// @brief Transform a 2D local point using this matrix.
    [[nodiscard]] Point mapPoint(Point p) const {
        float x = storage[0] * p.x + storage[4] * p.y + storage[12];
        float y = storage[1] * p.x + storage[5] * p.y + storage[13];
        float w = storage[3] * p.x + storage[7] * p.y + storage[15];
        if (w != 0.0f && w != 1.0f) {
            x /= w;
            y /= w;
        }
        return Point{x, y};
    }

    /// @brief Map a point from transformed space back to child local space (for hit-testing).
    [[nodiscard]] Point mapPointInverse(Point p) const {
        float a = storage[0];
        float b = storage[1];
        float c = storage[4];
        float d = storage[5];
        float tx = storage[12];
        float ty = storage[13];

        float det = a * d - b * c;
        if (std::abs(det) < 1e-6f) {
            return Point{p.x - tx, p.y - ty};
        }

        float inv_det = 1.0f / det;
        float px = p.x - tx;
        float py = p.y - ty;
        return Point{
            inv_det * (d * px - c * py),
            inv_det * (-b * px + a * py)
        };
    }
};

// ════════════════════════════════════════════════════════════════
// FlowPaintingContext
// ════════════════════════════════════════════════════════════════

/// @brief Context passed to FlowDelegate::paintChildren. Provides layout
///        sizes of children and painting methods supporting arbitrary transforms.
class FlowPaintingContext {
public:
    FlowPaintingContext(RenderFlow& flow, PaintContext& paint_context);
    ~FlowPaintingContext() = default;

    /// @brief The size of the Flow container.
    [[nodiscard]] Size size() const;

    /// @brief The number of children in the Flow.
    [[nodiscard]] size_t childCount() const;

    /// @brief Returns the layout size of the child at the specified index.
    [[nodiscard]] Size getChildSize(size_t index) const;

    /// @brief Paint the child at index with an offset and optional opacity.
    void paintChild(size_t index, Point offset = {0.0f, 0.0f}, float opacity = 1.0f);

    /// @brief Paint the child at index with a full Matrix4 transform and optional opacity.
    void paintChild(size_t index, const Matrix4& transform, float opacity = 1.0f);

    /// @brief Paint the child at index with a 9-element 3x3 SkMatrix buffer.
    void paintChild(size_t index, const float matrix9[9], float opacity = 1.0f);

private:
    RenderFlow&   flow_;
    PaintContext& paint_context_;
};

// ════════════════════════════════════════════════════════════════
// FlowDelegate Abstract Base
// ════════════════════════════════════════════════════════════════

/// @brief Delegate that controls the sizing and painting transforms of children in a Flow.
class FlowDelegate {
public:
    virtual ~FlowDelegate() = default;

    /// @brief Determine the size of the Flow container given incoming constraints.
    virtual Size getSize(const BoxConstraints& constraints) {
        if (constraints.hasBoundedWidth() && constraints.hasBoundedHeight()) {
            return constraints.biggest();
        }
        return Size{
            constraints.hasBoundedWidth() ? constraints.max_width : 0.0f,
            constraints.hasBoundedHeight() ? constraints.max_height : 0.0f
        };
    }

    /// @brief Determine the constraints for sizing the child at the specified index.
    virtual BoxConstraints getConstraintsForChild(size_t /*index*/, const BoxConstraints& constraints) {
        return BoxConstraints{0.0f, constraints.max_width, 0.0f, constraints.max_height};
    }

    /// @brief Paint children using the FlowPaintingContext.
    virtual void paintChildren(FlowPaintingContext& context) = 0;

    /// @brief Return true if this delegate requires recalculating layout compared to oldDelegate.
    virtual bool shouldRelayout(const FlowDelegate& /*oldDelegate*/) const {
        return false;
    }

    /// @brief Return true if this delegate requires repainting children compared to oldDelegate.
    virtual bool shouldRepaint(const FlowDelegate& /*oldDelegate*/) const {
        return false;
    }
};

// ════════════════════════════════════════════════════════════════
// Functional Flow Delegate (Lambda Support)
// ════════════════════════════════════════════════════════════════

using FlowPaintCallback       = std::function<void(FlowPaintingContext&)>;
using FlowSizeCallback        = std::function<Size(const BoxConstraints&)>;
using FlowConstraintsCallback = std::function<BoxConstraints(size_t, const BoxConstraints&)>;

class FunctionalFlowDelegate : public FlowDelegate {
public:
    FlowPaintCallback       on_paint;
    FlowSizeCallback        on_get_size;
    FlowConstraintsCallback on_get_constraints;

    FunctionalFlowDelegate(FlowPaintCallback paint,
                           FlowSizeCallback size_cb = nullptr,
                           FlowConstraintsCallback constraints_cb = nullptr)
        : on_paint(std::move(paint)),
          on_get_size(std::move(size_cb)),
          on_get_constraints(std::move(constraints_cb)) {}

    Size getSize(const BoxConstraints& constraints) override {
        if (on_get_size) {
            return on_get_size(constraints);
        }
        return FlowDelegate::getSize(constraints);
    }

    BoxConstraints getConstraintsForChild(size_t index, const BoxConstraints& constraints) override {
        if (on_get_constraints) {
            return on_get_constraints(index, constraints);
        }
        return FlowDelegate::getConstraintsForChild(index, constraints);
    }

    void paintChildren(FlowPaintingContext& context) override {
        if (on_paint) {
            on_paint(context);
        }
    }

    bool shouldRelayout(const FlowDelegate&) const override {
        return true;
    }

    bool shouldRepaint(const FlowDelegate&) const override {
        return true;
    }
};

// ════════════════════════════════════════════════════════════════
// Flow Widget & Props
// ════════════════════════════════════════════════════════════════

class FlowWidget : public MultiChildRenderObjectWidget {
public:
    std::shared_ptr<FlowDelegate> delegate;

    FlowWidget() = default;
    FlowWidget(Key key,
               std::shared_ptr<FlowDelegate> del,
               std::vector<WidgetPtr> children = {})
        : MultiChildRenderObjectWidget(std::move(key), std::move(children)),
          delegate(std::move(del)) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
    [[nodiscard]] std::string_view typeName() const override { return "Flow"; }
};

struct FlowProps {
    Key                             key                  = Key::none();
    std::shared_ptr<FlowDelegate>   delegate             = nullptr;
    FlowPaintCallback               paint_callback       = nullptr;
    FlowSizeCallback                size_callback        = nullptr;
    FlowConstraintsCallback         constraints_callback = nullptr;
    std::vector<WidgetPtr>          children;

    operator WidgetPtr() const;
};

// ════════════════════════════════════════════════════════════════
// Factory Functions
// ════════════════════════════════════════════════════════════════

inline std::shared_ptr<FlowWidget> flow(FlowProps props = {}) {
    std::shared_ptr<FlowDelegate> del = std::move(props.delegate);
    if (!del && props.paint_callback) {
        del = std::make_shared<FunctionalFlowDelegate>(
            std::move(props.paint_callback),
            std::move(props.size_callback),
            std::move(props.constraints_callback)
        );
    }
    return std::make_shared<FlowWidget>(
        props.key,
        std::move(del),
        std::move(props.children)
    );
}

inline std::shared_ptr<FlowWidget> flow(
    std::shared_ptr<FlowDelegate> delegate,
    std::vector<WidgetPtr> children,
    Key key = Key::none()) {
    return std::make_shared<FlowWidget>(
        std::move(key),
        std::move(delegate),
        std::move(children)
    );
}

inline FlowProps::operator WidgetPtr() const {
    return flow(*this);
}

using Flow = FlowProps;

} // namespace enki
