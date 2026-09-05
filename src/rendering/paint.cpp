/// @file paint.cpp
/// @brief Paint, Gradient, and ImageFilter implementation using Skia.

#include "enki/rendering/paint.hpp"
#include <include/core/SkShader.h>
#include <include/core/SkColorFilter.h>
#include <include/effects/SkGradientShader.h>
#include <include/effects/SkImageFilters.h>
#include <vector>
#include <unordered_map>
#include <cstdint>

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

class SkiaColorFilterWrapper : public ColorFilter {
public:
    explicit SkiaColorFilterWrapper(sk_sp<SkColorFilter> filter) : filter_(std::move(filter)) {}
    void* getNativeHandle() const override { return filter_.get(); }
private:
    sk_sp<SkColorFilter> filter_;
};

Paint::Paint() = default;

// ════════════════════════════════════════════════════════════════
// Gradient Implementations
// ════════════════════════════════════════════════════════════════

std::shared_ptr<Shader> Gradient::linear(
    Point start,
    Point end,
    const std::vector<Color>& colors,
    const std::vector<float>& stops
) {
    if (colors.empty()) return nullptr;

    SkPoint pts[2] = {
        SkPoint::Make(start.x, start.y),
        SkPoint::Make(end.x, end.y)
    };

    const float* pos_ptr = stops.empty() ? nullptr : stops.data();

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
    Point center,
    float radius,
    const std::vector<Color>& colors,
    const std::vector<float>& stops
) {
    if (colors.empty() || radius <= 0.0f) return nullptr;

    SkPoint c = SkPoint::Make(center.x, center.y);
    const float* pos_ptr = stops.empty() ? nullptr : stops.data();

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
    struct BlurKey {
        float x, y;
        bool operator==(const BlurKey& o) const noexcept { return x == o.x && y == o.y; }
    };
    struct BlurHash {
        size_t operator()(const BlurKey& k) const noexcept {
            return std::hash<float>()(k.x) ^ (std::hash<float>()(k.y) << 1);
        }
    };
    static std::unordered_map<BlurKey, std::shared_ptr<ImageFilter>, BlurHash> s_blur_cache;

    BlurKey key{sigmaX, sigmaY};
    auto it = s_blur_cache.find(key);
    if (it != s_blur_cache.end()) {
        return it->second;
    }

    auto filter = SkImageFilters::Blur(sigmaX, sigmaY, nullptr);
    auto res = std::make_shared<SkiaImageFilterWrapper>(std::move(filter));
    s_blur_cache[key] = res;
    return res;
}

std::shared_ptr<ColorFilter> ColorFilter::mode(Color color, BlendMode mode) {
    auto filter = SkColorFilters::Blend(static_cast<SkColor>(color), static_cast<SkBlendMode>(mode));
    return std::make_shared<SkiaColorFilterWrapper>(std::move(filter));
}

std::shared_ptr<ColorFilter> ColorFilter::matrix(const float matrix[20]) {
    if (!matrix) return nullptr;
    auto filter = SkColorFilters::Matrix(matrix);
    return std::make_shared<SkiaColorFilterWrapper>(std::move(filter));
}

std::shared_ptr<ColorFilter> ColorFilter::matrix(const std::vector<float>& matrix) {
    if (matrix.size() < 20) return nullptr;
    return ColorFilter::matrix(matrix.data());
}

std::shared_ptr<ColorFilter> ColorFilter::grayscale() {
    const float m[20] = {
        0.2126f, 0.7152f, 0.0722f, 0.0f, 0.0f,
        0.2126f, 0.7152f, 0.0722f, 0.0f, 0.0f,
        0.2126f, 0.7152f, 0.0722f, 0.0f, 0.0f,
        0.0f,    0.0f,    0.0f,    1.0f, 0.0f
    };
    return ColorFilter::matrix(m);
}

std::shared_ptr<ColorFilter> ColorFilter::sepia() {
    const float m[20] = {
        0.393f, 0.769f, 0.189f, 0.0f, 0.0f,
        0.349f, 0.686f, 0.168f, 0.0f, 0.0f,
        0.272f, 0.534f, 0.131f, 0.0f, 0.0f,
        0.0f,   0.0f,   0.0f,   1.0f, 0.0f
    };
    return ColorFilter::matrix(m);
}

std::shared_ptr<ColorFilter> ColorFilter::invert() {
    const float m[20] = {
        -1.0f,  0.0f,  0.0f, 0.0f, 255.0f,
         0.0f, -1.0f,  0.0f, 0.0f, 255.0f,
         0.0f,  0.0f, -1.0f, 0.0f, 255.0f,
         0.0f,  0.0f,  0.0f, 1.0f, 0.0f
    };
    return ColorFilter::matrix(m);
}

std::shared_ptr<ColorFilter> ColorFilter::tint(Color color, float strength) {
    (void)strength;
    return ColorFilter::mode(color, BlendMode::SrcATop);
}

std::shared_ptr<ColorFilter> ColorFilter::linearToSrgbGamma() {
    auto filter = SkColorFilters::LinearToSRGBGamma();
    return std::make_shared<SkiaColorFilterWrapper>(std::move(filter));
}

std::shared_ptr<ColorFilter> ColorFilter::srgbToLinearGamma() {
    auto filter = SkColorFilters::SRGBToLinearGamma();
    return std::make_shared<SkiaColorFilterWrapper>(std::move(filter));
}

} // namespace enki
