#pragma once
/// @file image.hpp
/// @brief Image widget and RenderImage for hardware-accelerated image rendering with BoxFit.
///
/// Features:
///   - Automatic caching via ImageCache to avoid redundant disk I/O and decoding.
///   - Full BoxFit support: Cover, Contain, Fill, FitWidth, FitHeight, None, ScaleDown.
///   - Alignment inside destination box (Center, TopLeft, BottomRight, etc.).
///   - Geometric clipping via custom BorderRadius or BoxShape::Circle.
///   - Intrinsic sizing & aspect-ratio measurement integration with Anu Flexbox.
///   - Color tinting with BlendMode and alpha opacity.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/rendering/image.hpp"
#include "enki/rendering/paint.hpp"
#include "enki/tree/widget.hpp"
#include "enki/tree/render_object.hpp"
#include <layout_engine/Anu.h>
#include <string>
#include <string_view>
#include <memory>
#include <optional>
#include <vector>

namespace enki {

// ════════════════════════════════════════════════════════════════
// ImageCache — Thread-safe In-Memory LRU / Map Cache
// ════════════════════════════════════════════════════════════════

class ImageCache {
public:
    /// Get cached image or load from file if not present.
    static std::shared_ptr<Image> getOrLoad(std::string_view path);

    /// Store an image in cache.
    static void put(std::string_view key, std::shared_ptr<Image> image);

    /// Retrieve an image if cached; nullptr otherwise.
    static std::shared_ptr<Image> get(std::string_view key);

    /// Clear all cached images.
    static void clear();

    /// Get current number of cached entries.
    static size_t count();
};

// ════════════════════════════════════════════════════════════════
// ImageStyle — Configuration for ImageWidget & RenderImage
// ════════════════════════════════════════════════════════════════

struct ImageStyle {
    std::shared_ptr<Image>    image;
    std::string               source_path;

    std::optional<StyleValue> width;
    std::optional<StyleValue> height;
    std::optional<StyleValue> min_width;
    std::optional<StyleValue> min_height;
    std::optional<StyleValue> max_width;
    std::optional<StyleValue> max_height;

    BoxFit                    fit            = BoxFit::Cover;
    Alignment                 alignment      = Alignment::Center;
    BorderRadius              border_radius  = BorderRadius::zero();
    BoxShape                  shape          = BoxShape::Rectangle;

    std::optional<Color>      tint_color;
    BlendMode                 blend_mode     = BlendMode::SrcIn;
    float                     opacity        = 1.0f;
    bool                      clip_content   = true;

    bool operator==(const ImageStyle& other) const {
        return image == other.image &&
               source_path == other.source_path &&
               width == other.width &&
               height == other.height &&
               fit == other.fit &&
               alignment == other.alignment &&
               border_radius == other.border_radius &&
               shape == other.shape &&
               tint_color == other.tint_color &&
               blend_mode == other.blend_mode &&
               opacity == other.opacity &&
               clip_content == other.clip_content;
    }
};

// ════════════════════════════════════════════════════════════════
// RenderImage — RenderBox for high-performance image display
// ════════════════════════════════════════════════════════════════

class RenderImage : public RenderBox {
public:
    RenderImage();
    explicit RenderImage(ImageStyle style);
    ~RenderImage() override;

    void setStyle(const ImageStyle& style);
    [[nodiscard]] const ImageStyle& style() const { return style_; }

    void paint(PaintContext& context) override;

    /// Calculate source crop and destination geometry based on BoxFit & Alignment.
    static void calculateBoxFitGeometry(BoxFit fit, Alignment align,
                                        Size src_size, Size dst_size,
                                        Rect& out_src, Rect& out_dst);

private:
    void applyStyleToNode();
    static ANUSize measureCallback(ANUNodeConstRef node, float width, ANUMeasureMode widthMode,
                                   float height, ANUMeasureMode heightMode);

    ImageStyle style_;
};

// ════════════════════════════════════════════════════════════════
// ImageWidget — Declarative Leaf Widget for Images
// ════════════════════════════════════════════════════════════════

class ImageWidget : public SingleChildRenderObjectWidget {
public:
    ImageStyle style;

    ImageWidget() = default;
    explicit ImageWidget(std::shared_ptr<Image> image) {
        style.image = std::move(image);
    }
    explicit ImageWidget(std::string_view path) {
        style.source_path = std::string(path);
        style.image = ImageCache::getOrLoad(path);
    }
    ImageWidget(Key key, std::shared_ptr<Image> image)
        : SingleChildRenderObjectWidget(std::move(key)) {
        style.image = std::move(image);
    }
    ImageWidget(Key key, std::string_view path)
        : SingleChildRenderObjectWidget(std::move(key)) {
        style.source_path = std::string(path);
        style.image = ImageCache::getOrLoad(path);
    }

    [[nodiscard]] std::string_view typeName() const override { return "ImageWidget"; }

    std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;

    // ── Static Factories ───────────────────────────────────────

    static std::shared_ptr<ImageWidget> asset(std::string_view path) {
        return std::make_shared<ImageWidget>(path);
    }

    static std::shared_ptr<ImageWidget> file(std::string_view path) {
        return std::make_shared<ImageWidget>(path);
    }

    static std::shared_ptr<ImageWidget> memory(const std::vector<uint8_t>& data) {
        auto res = Image::loadFromMemory(data);
        if (res.isOk()) {
            return std::make_shared<ImageWidget>(res.value());
        }
        return std::make_shared<ImageWidget>();
    }

    static std::shared_ptr<ImageWidget> fromImage(std::shared_ptr<Image> img) {
        return std::make_shared<ImageWidget>(std::move(img));
    }

    // ── Fluent Builder API ─────────────────────────────────────

    ImageWidget& fit(BoxFit f) { style.fit = f; return *this; }
    ImageWidget& alignment(Alignment a) { style.alignment = a; return *this; }

    ImageWidget& width(float w) { style.width = StyleValue::point(w); return *this; }
    ImageWidget& height(float h) { style.height = StyleValue::point(h); return *this; }
    ImageWidget& width(StyleValue w) { style.width = w; return *this; }
    ImageWidget& height(StyleValue h) { style.height = h; return *this; }
    ImageWidget& size(float w, float h) {
        style.width = StyleValue::point(w);
        style.height = StyleValue::point(h);
        return *this;
    }

    ImageWidget& minWidth(float w) { style.min_width = StyleValue::point(w); return *this; }
    ImageWidget& minHeight(float h) { style.min_height = StyleValue::point(h); return *this; }
    ImageWidget& maxWidth(float w) { style.max_width = StyleValue::point(w); return *this; }
    ImageWidget& maxHeight(float h) { style.max_height = StyleValue::point(h); return *this; }

    ImageWidget& borderRadius(float r) { style.border_radius = BorderRadius::all(r); return *this; }
    ImageWidget& borderRadius(BorderRadius r) { style.border_radius = r; return *this; }
    ImageWidget& shape(BoxShape s) { style.shape = s; return *this; }
    ImageWidget& circle() { style.shape = BoxShape::Circle; return *this; }

    ImageWidget& color(Color c, BlendMode mode = BlendMode::SrcIn) {
        style.tint_color = c;
        style.blend_mode = mode;
        return *this;
    }
    ImageWidget& tint(Color c, BlendMode mode = BlendMode::SrcIn) {
        return color(c, mode);
    }
    ImageWidget& opacity(float o) { style.opacity = o; return *this; }
    ImageWidget& clip(bool c) { style.clip_content = c; return *this; }
};

// ════════════════════════════════════════════════════════════════
// Global Factory Helpers
// ════════════════════════════════════════════════════════════════

inline std::shared_ptr<ImageWidget> imageAsset(std::string_view path) {
    return ImageWidget::asset(path);
}

inline std::shared_ptr<ImageWidget> imageFile(std::string_view path) {
    return ImageWidget::file(path);
}

inline std::shared_ptr<ImageWidget> imageMemory(const std::vector<uint8_t>& data) {
    return ImageWidget::memory(data);
}

struct ImageProps {
    Key key = Key::none();
    std::shared_ptr<Image> image;
    std::string source_path;

    std::optional<StyleValue> width;
    std::optional<StyleValue> height;
    std::optional<StyleValue> min_width;
    std::optional<StyleValue> min_height;
    std::optional<StyleValue> max_width;
    std::optional<StyleValue> max_height;

    BoxFit fit = BoxFit::Cover;
    Alignment alignment = Alignment::Center;
    BorderRadius border_radius = BorderRadius::zero();
    BoxShape shape = BoxShape::Rectangle;

    std::optional<Color> tint_color;
    BlendMode blend_mode = BlendMode::SrcIn;
    float opacity = 1.0f;
    bool clip_content = true;
};

inline std::shared_ptr<ImageWidget> image(ImageProps props) {
    auto img = props.image ? std::make_shared<ImageWidget>(std::move(props.key), std::move(props.image)) 
                           : std::make_shared<ImageWidget>(std::move(props.key), props.source_path);
    
    if (props.width) img->style.width = props.width;
    if (props.height) img->style.height = props.height;
    if (props.min_width) img->style.min_width = props.min_width;
    if (props.min_height) img->style.min_height = props.min_height;
    if (props.max_width) img->style.max_width = props.max_width;
    if (props.max_height) img->style.max_height = props.max_height;
    
    img->style.fit = props.fit;
    img->style.alignment = props.alignment;
    img->style.border_radius = props.border_radius;
    img->style.shape = props.shape;
    img->style.tint_color = props.tint_color;
    img->style.blend_mode = props.blend_mode;
    img->style.opacity = props.opacity;
    img->style.clip_content = props.clip_content;
    
    return img;
}

inline std::shared_ptr<ImageWidget> imageAsset(std::string_view path, ImageProps props) {
    props.source_path = std::string(path);
    return image(std::move(props));
}

inline std::shared_ptr<ImageWidget> image(std::shared_ptr<Image> img, ImageProps props) {
    props.image = std::move(img);
    return image(std::move(props));
}

inline std::shared_ptr<ImageWidget> image(std::shared_ptr<Image> img) {
    return ImageWidget::fromImage(std::move(img));
}

} // namespace enki
