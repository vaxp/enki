#pragma once
/// @file svg_morph.hpp
/// @brief Declarative SVG Vector Morphing Widget for ENKI.
///
/// Features:
///   - C++20 designated initializer syntax.
///   - Smoothly morphs between two SVG paths or iconic presets.
///   - Integrated with Skia Canvas hardware acceleration.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/animation/path_morph.hpp"
#include "enki/widgets/skia_canvas.hpp"
#include <string_view>
#include <memory>
#include <unordered_map>
#include <mutex>
#include <string>
#include <optional>

namespace enki {

// ════════════════════════════════════════════════════════════════
// SvgMorph & Declarative Factory
// ════════════════════════════════════════════════════════════════

struct SvgMorph {
    std::string_view    from_path     = SvgMorphPaths::hamburger;
    std::string_view    to_path       = SvgMorphPaths::close;
    float               progress      = 0.0f;
    Color               color         = 0xFFFFFFFF; // White
    float               stroke_width  = 2.5f;
    bool                is_stroke     = true;
    std::optional<bool> is_closed     = std::nullopt;
    StyleValue          width         = 24.0f;
    StyleValue          height        = 24.0f;
    Key                 key           = Key::none();

    operator WidgetPtr() const;
};

inline WidgetPtr svgMorph(const SvgMorph& props = {}) {
    return static_cast<WidgetPtr>(props);
}

inline SvgMorph::operator WidgetPtr() const {
    static std::unordered_map<std::string, std::shared_ptr<PathMorph>> s_morph_cache;
    static std::mutex s_morph_mutex;

    std::string cache_key;
    cache_key.reserve(from_path.size() + to_path.size() + 2);
    cache_key.append(from_path);
    cache_key.push_back('#');
    cache_key.append(to_path);

    std::shared_ptr<PathMorph> morph;
    {
        std::lock_guard<std::mutex> lock(s_morph_mutex);
        auto it = s_morph_cache.find(cache_key);
        if (it != s_morph_cache.end()) {
            morph = it->second;
        } else {
            morph = std::make_shared<PathMorph>(from_path, to_path);
            s_morph_cache[cache_key] = morph;
        }
    }

    if (is_closed.has_value()) {
        morph->setClosed(*is_closed);
    }

    float p = progress;
    Color col = color;
    float sw = stroke_width;
    bool stroke = is_stroke;

    return skiaCanvas(SkiaCanvasProps{
        .painter = [morph, p, col, sw, stroke](Canvas& canvas, Size size) {
            Paint paint;
            paint.setColor(col);
            paint.setStrokeWidth(sw);
            morph->render(canvas, Rect{0, 0, size.width, size.height}, p, paint, stroke);
        },
        .width = width,
        .height = height,
        .key = key,
    });
}

} // namespace enki
