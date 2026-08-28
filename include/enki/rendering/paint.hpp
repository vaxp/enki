#pragma once
/// @file paint.hpp
/// @brief Painting style properties and Skia Shader / ImageFilter helpers.

#include "enki/core/types.hpp"
#include "enki/rendering/color.hpp"
#include <vector>
#include <memory>

namespace enki {

enum class PaintStyle {
    Fill,
    Stroke,
    StrokeAndFill
};

enum class StrokeCap {
    Butt,
    Round,
    Square
};

enum class StrokeJoin {
    Miter,
    Round,
    Bevel
};

enum class BlendMode {
    Clear, Src, Dst, SrcOver, DstOver, SrcIn, DstIn, SrcOut, DstOut,
    SrcATop, DstATop, Xor, Plus, Modulate, Screen, Overlay, Darken, Lighten,
    ColorDodge, ColorBurn, HardLight, SoftLight, Difference, Exclusion,
    Multiply, Hue, Saturation, Color, Luminosity
};

class Shader {
public:
    virtual ~Shader() = default;
    virtual void* getNativeHandle() const = 0;
};

class ImageFilter {
public:
    virtual ~ImageFilter() = default;
    virtual void* getNativeHandle() const = 0;

    static std::shared_ptr<ImageFilter> blur(float sigmaX, float sigmaY);
};

class ColorFilter {
public:
    virtual ~ColorFilter() = default;
    virtual void* getNativeHandle() const = 0;

    static std::shared_ptr<ColorFilter> mode(Color color, BlendMode mode);
    static std::shared_ptr<ColorFilter> matrix(const float matrix[20]);
    static std::shared_ptr<ColorFilter> matrix(const std::vector<float>& matrix);
    static std::shared_ptr<ColorFilter> grayscale();
    static std::shared_ptr<ColorFilter> sepia();
    static std::shared_ptr<ColorFilter> invert();
    static std::shared_ptr<ColorFilter> tint(Color color, float strength = 1.0f);
    static std::shared_ptr<ColorFilter> srgbToLinearGamma();
    static std::shared_ptr<ColorFilter> linearToSrgbGamma();
};

/// Gradient shader generators for rich desktop UI effects.
struct Gradient {
    static std::shared_ptr<Shader> linear(
        Point start, Point end,
        const std::vector<Color>& colors,
        const std::vector<float>& positions = {}
    );

    static std::shared_ptr<Shader> radial(
        Point center, float radius,
        const std::vector<Color>& colors,
        const std::vector<float>& positions = {}
    );
};

/// Paint encapsulates styling information for drawing operations.
class Paint {
public:
    Paint();
    ~Paint() = default;

    void setColor(Color color) { color_ = color; }
    [[nodiscard]] Color getColor() const { return color_; }

    void setStyle(PaintStyle style) { style_ = style; }
    [[nodiscard]] PaintStyle getStyle() const { return style_; }

    void setStrokeWidth(float width) { stroke_width_ = width; }
    [[nodiscard]] float getStrokeWidth() const { return stroke_width_; }

    void setStrokeCap(StrokeCap cap) { stroke_cap_ = cap; }
    [[nodiscard]] StrokeCap getStrokeCap() const { return stroke_cap_; }

    void setStrokeJoin(StrokeJoin join) { stroke_join_ = join; }
    [[nodiscard]] StrokeJoin getStrokeJoin() const { return stroke_join_; }

    void setAntiAlias(bool aa) { anti_alias_ = aa; }
    [[nodiscard]] bool isAntiAlias() const { return anti_alias_; }

    void setBlendMode(BlendMode mode) { blend_mode_ = mode; }
    [[nodiscard]] BlendMode getBlendMode() const { return blend_mode_; }

    void setShader(std::shared_ptr<Shader> shader) { shader_ = std::move(shader); }
    [[nodiscard]] std::shared_ptr<Shader> getShader() const { return shader_; }

    void setImageFilter(std::shared_ptr<ImageFilter> filter) { image_filter_ = std::move(filter); }
    [[nodiscard]] std::shared_ptr<ImageFilter> getImageFilter() const { return image_filter_; }

    void setColorFilter(std::shared_ptr<ColorFilter> filter) { color_filter_ = std::move(filter); }
    [[nodiscard]] std::shared_ptr<ColorFilter> getColorFilter() const { return color_filter_; }

    void setShadow(Color color, float blur, float dx, float dy) {
        shadow_color_ = color;
        shadow_blur_ = blur;
        shadow_offset_dx_ = dx;
        shadow_offset_dy_ = dy;
        has_shadow_ = true;
    }
    [[nodiscard]] bool hasShadow() const { return has_shadow_; }
    [[nodiscard]] Color getShadowColor() const { return shadow_color_; }
    [[nodiscard]] float getShadowBlur() const { return shadow_blur_; }
    [[nodiscard]] float getShadowDx() const { return shadow_offset_dx_; }
    [[nodiscard]] float getShadowDy() const { return shadow_offset_dy_; }

private:
    Color      color_        = 0xFF000000;
    PaintStyle style_        = PaintStyle::Fill;
    float      stroke_width_ = 0.0f;
    StrokeCap  stroke_cap_   = StrokeCap::Butt;
    StrokeJoin stroke_join_  = StrokeJoin::Miter;
    bool       anti_alias_   = true;
    BlendMode  blend_mode_   = BlendMode::SrcOver;

    std::shared_ptr<Shader> shader_;
    std::shared_ptr<ImageFilter> image_filter_;
    std::shared_ptr<ColorFilter> color_filter_;

    bool       has_shadow_       = false;
    Color      shadow_color_     = 0x00000000;
    float      shadow_blur_      = 0.0f;
    float      shadow_offset_dx_ = 0.0f;
    float      shadow_offset_dy_ = 0.0f;
};

}  // namespace enki
