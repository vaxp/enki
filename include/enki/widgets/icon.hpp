#pragma once
/// @file icon.hpp
/// @brief A highly professional Icon widget supporting both Icon Fonts and SVG Paths.

#include "enki/tree/widget.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/rendering/color.hpp"
#include "include/core/SkPath.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"
#include <string>
#include <memory>
#include <cstdint>
#include <optional>

namespace enki {

/// @brief Represents the data source for an Icon.
struct IconData {
    uint32_t codepoint = 0;
    std::string font_family;
    std::string svg_path; // if not empty, it's an SVG path

    /// @brief Create an IconData for a font glyph (e.g. Material Icons)
    static IconData font(uint32_t cp, std::string family) {
        IconData data;
        data.codepoint = cp;
        data.font_family = std::move(family);
        return data;
    }

    /// @brief Create an IconData for an SVG path
    static IconData svg(std::string path) {
        IconData data;
        data.svg_path = std::move(path);
        return data;
    }

    bool isSvg() const { return !svg_path.empty(); }
    bool empty() const { return codepoint == 0 && svg_path.empty(); }
    
    bool operator==(const IconData& other) const {
        return codepoint == other.codepoint && 
               font_family == other.font_family && 
               svg_path == other.svg_path;
    }
    bool operator!=(const IconData& other) const { return !(*this == other); }
};

#include "enki/widgets/icons_material.hpp"

namespace Icons {
    namespace SVG {
        // A standard play icon SVG path
        inline IconData play() { return IconData::svg("M8 5v14l11-7z"); }
        inline IconData check() { return IconData::svg("M9 16.17 4.83 12l-1.42 1.41L9 19 21 7l-1.41-1.41z"); }
    }
}

/// @brief Render object for drawing the icon.
class RenderIcon : public RenderBox {
public:
    RenderIcon(IconData data, float size, Color color);

    void setIconData(const IconData& data);
    void setSize(float size);
    void setColor(Color color);

    void paint(PaintContext& context) override;
    bool hitTestChildren(HitTestResult& result, Point localPoint) override { return false; } // Leaf node

private:
    void rebuildSvgPath();
    void rebuildFontCache();

    IconData data_;
    float    size_;
    Color    color_;

    // SVG cache
    SkPath cached_svg_path_;

    // Font-icon cache — rebuilt only when data_ or size_ changes
    sk_sp<SkTypeface>  cached_typeface_;
    sk_sp<SkTextBlob>  cached_blob_;
    float              cached_dx_     = 0.0f;
    float              cached_dy_     = 0.0f;
    bool               font_cache_dirty_ = true;
};

struct IconProps {
    Key key = Key::none();
    IconData data = {};
    std::optional<float> size = std::nullopt;
    std::optional<Color> color = std::nullopt;
};

/// @brief Icon widget that can draw vector shapes from Fonts or SVG paths.
class IconWidget : public SingleChildRenderObjectWidget {
public:
    IconData data;
    float size_val = 24.0f;
    Color color_val = 0xFFFFFFFF; // Default white

    IconWidget() = default;
    explicit IconWidget(IconData d, float size = 24.0f, Color color = 0xFFFFFFFF)
        : data(std::move(d)), size_val(size), color_val(color) {}
    IconWidget(Key key, IconData d, float size = 24.0f, Color color = 0xFFFFFFFF)
        : SingleChildRenderObjectWidget(std::move(key)), data(std::move(d)), size_val(size), color_val(color) {}

    std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override {
        return std::make_unique<RenderIcon>(data, size_val, color_val);
    }
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override {
        auto* r_icon = static_cast<RenderIcon*>(&renderObject);
        r_icon->setIconData(data);
        r_icon->setSize(size_val);
        r_icon->setColor(color_val);
    }
    std::string_view typeName() const override { return "Icon"; }
};

// ════════════════════════════════════════════════════════════════
// Declarative Icon Struct (C++20 Designated Initializers)
// ════════════════════════════════════════════════════════════════

struct Icon {
    Key key = Key::none();
    IconData data = {};
    IconData icon = {}; // alias
    float size = 24.0f;
    Color color = 0xFFFFFFFF;

    operator WidgetPtr() const {
        IconData d = !data.empty() ? data : icon;
        return std::make_shared<IconWidget>(key, std::move(d), size, color);
    }
};

// ════════════════════════════════════════════════════════════════
// Factory Functions
// ════════════════════════════════════════════════════════════════

inline std::shared_ptr<IconWidget> icon(IconData data, IconProps props = {}) {
    float sz = props.size.value_or(24.0f);
    Color col = props.color.value_or(0xFFFFFFFF);
    return std::make_shared<IconWidget>(std::move(props.key), std::move(data), sz, col);
}

inline std::shared_ptr<IconWidget> icon(IconProps props) {
    auto d = std::move(props.data);
    float sz = props.size.value_or(24.0f);
    Color col = props.color.value_or(0xFFFFFFFF);
    return std::make_shared<IconWidget>(std::move(props.key), std::move(d), sz, col);
}

inline std::shared_ptr<IconWidget> icon(IconData data, float size, Color color = 0xFFFFFFFF) {
    return std::make_shared<IconWidget>(std::move(data), size, color);
}

} // namespace enki
