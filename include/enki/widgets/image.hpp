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
               min_width == other.min_width &&
               min_height == other.min_height &&
               max_width == other.max_width &&
               max_height == other.max_height &&
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
// ImageWidget — Leaf Widget Implementation for Images
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
    ImageWidget(Key key, ImageStyle style_)
        : SingleChildRenderObjectWidget(std::move(key)), style(std::move(style_)) {}

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
};

// ════════════════════════════════════════════════════════════════
// Declarative ImageProps (C++20 Designated Initializers)
// ════════════════════════════════════════════════════════════════

struct ImageProps {
    Key key = Key::none();
    std::shared_ptr<Image> image = nullptr;
    std::string source_path = "";

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

    operator WidgetPtr() const;
};

// ════════════════════════════════════════════════════════════════
// Factory Functions
// ════════════════════════════════════════════════════════════════

inline std::shared_ptr<ImageWidget> image(ImageProps props) {
    ImageStyle s;
    if (props.image) {
        s.image = std::move(props.image);
    } else if (!props.source_path.empty()) {
        s.source_path = props.source_path;
        s.image = ImageCache::getOrLoad(props.source_path);
    }
    s.width = props.width;
    s.height = props.height;
    s.min_width = props.min_width;
    s.min_height = props.min_height;
    s.max_width = props.max_width;
    s.max_height = props.max_height;
    s.fit = props.fit;
    s.alignment = props.alignment;
    s.border_radius = props.border_radius;
    s.shape = props.shape;
    s.tint_color = props.tint_color;
    s.blend_mode = props.blend_mode;
    s.opacity = props.opacity;
    s.clip_content = props.clip_content;

    return std::make_shared<ImageWidget>(props.key, std::move(s));
}

inline std::shared_ptr<ImageWidget> image(std::string_view path, ImageProps props = {}) {
    props.source_path = std::string(path);
    return image(std::move(props));
}

inline std::shared_ptr<ImageWidget> image(std::shared_ptr<Image> img, ImageProps props = {}) {
    props.image = std::move(img);
    return image(std::move(props));
}

inline std::shared_ptr<ImageWidget> imageAsset(std::string_view path, ImageProps props = {}) {
    props.source_path = std::string(path);
    return image(std::move(props));
}

inline std::shared_ptr<ImageWidget> imageFile(std::string_view path, ImageProps props = {}) {
    props.source_path = std::string(path);
    return image(std::move(props));
}

inline std::shared_ptr<ImageWidget> imageMemory(const std::vector<uint8_t>& data, ImageProps props = {}) {
    auto res = Image::loadFromMemory(data);
    if (res.isOk()) {
        props.image = res.value();
    }
    return image(std::move(props));
}

inline ImageProps::operator WidgetPtr() const {
    return ::enki::image(*this);
}

} // namespace enki
