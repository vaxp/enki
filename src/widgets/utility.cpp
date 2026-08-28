/// @file utility.cpp
/// @brief Implementation of ENKI Section 20 Utility / Behavioral widgets.
///
/// Widgets:
///   1. Visibility
///   2. IgnorePointer
///
/// @copyright ENKI Framework — MIT License

#include "enki/widgets/utility.hpp"
#include "enki/widgets/container.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/rendering/canvas.hpp"

#include <cmath>
#include <algorithm>

namespace enki {

// ════════════════════════════════════════════════════════════════
// 1. IgnorePointer Render Object & Widget
// ════════════════════════════════════════════════════════════════

class RenderIgnorePointer : public RenderBox {
public:
    bool ignoring_ = true;

    explicit RenderIgnorePointer(bool ignoring) : ignoring_(ignoring) {}

    void setIgnoring(bool ignoring) {
        if (ignoring_ != ignoring) {
            ignoring_ = ignoring;
            markNeedsPaint();
        }
    }

    void paint(PaintContext& ctx) override {
        for (auto* child : children_) {
            if (child) {
                auto child_ctx = ctx.withOffset(child->offset());
                child->paint(child_ctx);
            }
        }
    }

    bool hitTest(HitTestResult& result, Point localPoint) override {
        if (ignoring_) return false;
        return RenderBox::hitTest(result, localPoint);
    }

    bool hitTestChildren(HitTestResult& result, Point localPoint) override {
        if (ignoring_) return false;
        return RenderBox::hitTestChildren(result, localPoint);
    }
};

std::unique_ptr<RenderObject> IgnorePointerWidget::createRenderObject(BuildContext&) {
    return std::make_unique<RenderIgnorePointer>(ignoring);
}

void IgnorePointerWidget::updateRenderObject(BuildContext&, RenderObject& renderObject) {
    auto& r = static_cast<RenderIgnorePointer&>(renderObject);
    r.setIgnoring(ignoring);
}

// ════════════════════════════════════════════════════════════════
// 2. Visibility Widget
// ════════════════════════════════════════════════════════════════

class RenderInvisibleBox : public RenderBox {
public:
    RenderInvisibleBox() = default;

    void paint(PaintContext&) override {
        // Invisible: paint nothing
    }

    bool hitTest(HitTestResult&, Point) override {
        // Invisible: pass through hit tests
        return false;
    }

    bool hitTestChildren(HitTestResult&, Point) override {
        return false;
    }
};

class InvisibleBoxWidget : public SingleChildRenderObjectWidget {
public:
    explicit InvisibleBoxWidget(WidgetPtr child)
        : SingleChildRenderObjectWidget(Key::none(), std::move(child)) {}

    std::unique_ptr<RenderObject> createRenderObject(BuildContext&) override {
        return std::make_unique<RenderInvisibleBox>();
    }

    [[nodiscard]] std::string_view typeName() const override { return "InvisibleBox"; }
};

WidgetPtr VisibilityWidget::build(BuildContext&) {
    if (visible) {
        return child ? child : (replacement ? replacement : container({}));
    }

    if (maintain_size) {
        WidgetPtr hidden = child ? child : container({});
        if (!maintain_interactivity) {
            hidden = ignorePointer(true, hidden);
        }
        // Preserves layout dimensions while rendering nothing
        return std::make_shared<InvisibleBoxWidget>(hidden);
    }

    if (maintain_state) {
        WidgetPtr hidden = child ? child : container({});
        hidden = ignorePointer(true, hidden);
        return container({
            .width = StyleValue::point(0.0f),
            .height = StyleValue::point(0.0f),
            .child = hidden,
        });
    }

    return replacement ? replacement : container({});
}

} // namespace enki
