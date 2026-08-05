/// @file paint.cpp
/// @brief Paint, Gradient, and ImageFilter implementation using Skia.

#include "enki/rendering/paint.hpp"
#include <include/core/SkShader.h>
#include <include/effects/SkGradientShader.h>
#include <include/effects/SkImageFilters.h>
#include <vector>

namespace enki {

class SkiaShaderWrapper : public Shader {
public:
    explicit SkiaShaderWrapper(sk_sp<SkShader> shader) : shader_(std::move(shader)) {}
    void* getNativeHandle() const override { return shader_.get(); }
private:
    sk_sp<SkShader> shader_;
};

class SkiaImageFilterWrapper : public ImageFilter {
public:
    explicit SkiaImageFilterWrapper(sk_sp<SkImageFilter> filter) : filter_(std::move(filter)) {}
    void* getNativeHandle() const override { return filter_.get(); }
private:
    sk_sp<SkImageFilter> filter_;
};

Paint::Paint() = default;

std::shared_ptr<Shader> Gradient::linear(
    Point start, Point end,
    const std::vector<Color>& colors,
    const std::vector<float>& positions
) {
    if (colors.empty()) return nullptr;

    SkPoint pts[2] = {SkPoint::Make(start.x, start.y), SkPoint::Make(end.x, end.y)};
    const SkScalar* pos_ptr = (positions.size() == colors.size()) ? positions.data() : nullptr;

    auto sk_shader = SkGradientShader::MakeLinear(
        pts,
        reinterpret_cast<const SkColor*>(colors.data()),
        pos_ptr,
        static_cast<int>(colors.size()),
        SkTileMode::kClamp
    );
    return std::make_shared<SkiaShaderWrapper>(std::move(sk_shader));
}

std::shared_ptr<Shader> Gradient::radial(
    Point center, float radius,
    const std::vector<Color>& colors,
    const std::vector<float>& positions
) {
    if (colors.empty() || radius <= 0.0f) return nullptr;

    SkPoint c = SkPoint::Make(center.x, center.y);
    const SkScalar* pos_ptr = (positions.size() == colors.size()) ? positions.data() : nullptr;

    auto sk_shader = SkGradientShader::MakeRadial(
        c,
        radius,
        reinterpret_cast<const SkColor*>(colors.data()),
        pos_ptr,
        static_cast<int>(colors.size()),
        SkTileMode::kClamp
    );
    return std::make_shared<SkiaShaderWrapper>(std::move(sk_shader));
}

std::shared_ptr<ImageFilter> ImageFilter::blur(float sigmaX, float sigmaY) {
    auto filter = SkImageFilters::Blur(sigmaX, sigmaY, nullptr);
    return std::make_shared<SkiaImageFilterWrapper>(std::move(filter));
}

} // namespace enki
