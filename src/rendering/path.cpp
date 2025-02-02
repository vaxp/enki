/// @file path.cpp
/// @brief Path implementation using Skia.

#include "enki/rendering/path.hpp"
#include <include/core/SkPath.h>
#include <include/core/SkRRect.h>

namespace enki {

struct Path::Impl {
    SkPath path;
};

Path::Path() : impl_(std::make_unique<Impl>()) {}

Path::~Path() = default;

Path::Path(Path&& other) noexcept = default;
Path& Path::operator=(Path&& other) noexcept = default;

void Path::moveTo(float x, float y) {
    impl_->path.moveTo(x, y);
}

void Path::lineTo(float x, float y) {
    impl_->path.lineTo(x, y);
}

void Path::quadTo(float x1, float y1, float x2, float y2) {
    impl_->path.quadTo(x1, y1, x2, y2);
}

void Path::cubicTo(float x1, float y1, float x2, float y2, float x3, float y3) {
    impl_->path.cubicTo(x1, y1, x2, y2, x3, y3);
}

void Path::close() {
    impl_->path.close();
}

void Path::addRect(const Rect& rect) {
    impl_->path.addRect(SkRect::MakeXYWH(rect.x, rect.y, rect.width, rect.height));
}

void Path::addCircle(float x, float y, float radius) {
    impl_->path.addCircle(x, y, radius);
}

void Path::addRoundRect(const Rect& rect, const BorderRadius& radius) {
    SkRect sk_rect = SkRect::MakeXYWH(rect.x, rect.y, rect.width, rect.height);
    SkVector radii[4] = {
        {radius.top_left, radius.top_left},
        {radius.top_right, radius.top_right},
        {radius.bottom_right, radius.bottom_right},
        {radius.bottom_left, radius.bottom_left}
    };
    SkRRect rrect;
    rrect.setRectRadii(sk_rect, radii);
    impl_->path.addRRect(rrect);
}

Rect Path::getBounds() const {
    const SkRect& b = impl_->path.getBounds();
    return Rect::fromLTRB(b.left(), b.top(), b.right(), b.bottom());
}

void* Path::getNativeHandle() const {
    return &impl_->path;
}

}  // namespace enki
