#include "enki/widgets/scroll_view.hpp"
#include "enki/tree/build_context.hpp"
#include "enki/rendering/canvas.hpp"
#include <algorithm>
#include <iostream>

namespace enki {

// ════════════════════════════════════════════════════════════════
// RenderScrollView
// ════════════════════════════════════════════════════════════════

RenderScrollView::RenderScrollView(ScrollOptions opt) : options(opt) {
    // We set Overflow::Scroll on our Anu node so that children can expand infinitely
    ANUNodeStyleSetOverflow(anu_node_, ANUOverflowScroll);

    // Set flex direction to match scroll direction so the child is not constrained (stretched) in the scroll axis
    if (options.direction == Axis::Horizontal) {
        ANUNodeStyleSetFlexDirection(anu_node_, ANUFlexDirectionRow);
    } else {
        ANUNodeStyleSetFlexDirection(anu_node_, ANUFlexDirectionColumn);
    }
    
    // A ScrollView should default to filling its parent container's available space,
    // otherwise it will just grow to the size of its unconstrained children, rendering scrolling useless.
    ANUNodeStyleSetWidthPercent(anu_node_, 100.0f);
    ANUNodeStyleSetHeightPercent(anu_node_, 100.0f);
    
    // Bind pan recognizer callbacks
    pan_recognizer.on_pan_update = [this](const DragUpdateDetails& e) {
        float dx = options.direction == Axis::Horizontal ? e.delta.x : 0.0f;
        float dy = options.direction == Axis::Vertical ? e.delta.y : 0.0f;
        // Invert delta because dragging up means scrolling down (increasing offset)
        updateScrollOffsets(-dx, -dy);
    };
}

void RenderScrollView::setOptions(const ScrollOptions& opt) {
    if (options != opt) {
        options = opt;
        markNeedsPaint();
    }
}

void RenderScrollView::syncLayout() {
    RenderBox::syncLayout();

    if (children_.empty() || !children_[0]) {
        max_scroll_x = 0.0f;
        max_scroll_y = 0.0f;
        return;
    }

    auto* child = children_[0];
    // Anu layout has already run. child size is computed.
    float cw = child->size().width;
    float ch = child->size().height;
    
    std::cout << "[RenderScrollView::syncLayout] Viewport: " << size_.width << "x" << size_.height 
              << " | Child: " << cw << "x" << ch << std::endl;
    
    if (options.direction == Axis::Vertical) {
        max_scroll_y = std::max(0.0f, ch - size_.height);
        max_scroll_x = 0.0f;
    } else {
        max_scroll_x = std::max(0.0f, cw - size_.width);
        max_scroll_y = 0.0f;
    }

    // Clamp current offset just in case the layout shrank
    updateScrollOffsets(0.0f, 0.0f);
}

void RenderScrollView::updateScrollOffsets(float dx, float dy) {
    float prev_x = scroll_offset_x;
    float prev_y = scroll_offset_y;

    scroll_offset_x += dx;
    scroll_offset_y += dy;

    if (options.clamp_overscroll) {
        scroll_offset_x = std::clamp(scroll_offset_x, 0.0f, max_scroll_x);
        scroll_offset_y = std::clamp(scroll_offset_y, 0.0f, max_scroll_y);
    }

    if (prev_x != scroll_offset_x || prev_y != scroll_offset_y) {
        markNeedsPaint();
    }
}

void RenderScrollView::paint(PaintContext& context) {
    if (size_.width <= 0.0f || size_.height <= 0.0f) return;

    Rect bounds = Rect::fromPointSize(context.offset, size_);
    
    // 1. Save and Clip
    context.canvas.save();
    context.canvas.clipRect(bounds);

    // 2. Paint child with offset
    if (!children_.empty() && children_[0]) {
        auto* child = children_[0];
        Point scroll_shift = {-scroll_offset_x, -scroll_offset_y};
        PaintContext child_ctx = context.withOffset(child->offset() + scroll_shift);
        child->paint(child_ctx);
    }

    // 3. Paint scrollbar if requested
    if (options.show_scrollbar) {
        Paint thumb_paint;
        thumb_paint.setColor(0x80808080); // Translucent gray
        thumb_paint.setAntiAlias(true);

        if (options.direction == Axis::Vertical && max_scroll_y > 0.0f) {
            float viewport_h = size_.height;
            float child_h = viewport_h + max_scroll_y;
            float thumb_h = std::max(20.0f, (viewport_h / child_h) * viewport_h);
            float scroll_percent = scroll_offset_y / max_scroll_y;
            float thumb_y = bounds.y + scroll_percent * (viewport_h - thumb_h);
            
            Rect thumb_rect = {bounds.x + bounds.width - 6.0f, thumb_y, 4.0f, thumb_h};
            context.canvas.drawRRect(thumb_rect, BorderRadius::circular(2.0f), thumb_paint);
        } else if (options.direction == Axis::Horizontal && max_scroll_x > 0.0f) {
            float viewport_w = size_.width;
            float child_w = viewport_w + max_scroll_x;
            float thumb_w = std::max(20.0f, (viewport_w / child_w) * viewport_w);
            float scroll_percent = scroll_offset_x / max_scroll_x;
            float thumb_x = bounds.x + scroll_percent * (viewport_w - thumb_w);
            
            Rect thumb_rect = {thumb_x, bounds.y + bounds.height - 6.0f, thumb_w, 4.0f};
            context.canvas.drawRRect(thumb_rect, BorderRadius::circular(2.0f), thumb_paint);
        }
    }

    // 4. Restore
    context.canvas.restore();
}

bool RenderScrollView::hitTestChildren(HitTestResult& result, Point localPoint) {
    if (children_.empty() || !children_[0]) return false;
    auto* child = children_[0];
    
    // Check if the point is even within our bounds (clipRect)
    if (localPoint.x < 0 || localPoint.x > size_.width ||
        localPoint.y < 0 || localPoint.y > size_.height) {
        return false;
    }

    // Reverse the translation for hit testing
    Point scroll_shifted_point = {
        localPoint.x + scroll_offset_x - child->offset().x,
        localPoint.y + scroll_offset_y - child->offset().y
    };
    
    return child->hitTest(result, scroll_shifted_point);
}

void RenderScrollView::handlePointerScroll(float dx, float dy) {
    if (options.direction == Axis::Horizontal) {
        float delta = std::abs(dx) > 0.0f ? dx : dy;
        updateScrollOffsets(-delta * options.scroll_speed, 0.0f);
    } else {
        updateScrollOffsets(0.0f, -dy * options.scroll_speed);
    }
}

void RenderScrollView::handlePointerDown(const PointerEvent& e) {
    pan_recognizer.handlePointerDown(e);
}
void RenderScrollView::handlePointerMove(const PointerEvent& e) {
    pan_recognizer.handlePointerMove(e);
}
void RenderScrollView::handlePointerUp(const PointerEvent& e) {
    pan_recognizer.handlePointerUp(e);
}

// ════════════════════════════════════════════════════════════════
// ScrollView Widget
// ════════════════════════════════════════════════════════════════

std::unique_ptr<RenderObject> ScrollViewWidget::createRenderObject(BuildContext& ctx) {
    return std::make_unique<RenderScrollView>(options);
}

void ScrollViewWidget::updateRenderObject(BuildContext& ctx, RenderObject& renderObject) {
    if (auto* rsv = dynamic_cast<RenderScrollView*>(&renderObject)) {
        rsv->setOptions(options);
    }
}

} // namespace enki
