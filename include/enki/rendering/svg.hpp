#pragma once
/// @file svg.hpp
/// @brief Vector SVG types and rendering for ENKI.
///
/// Features:
///   - SvgFit: Stretch, Contain, Cover.
///   - SvgSlice: 9-Slice scaling configuration for decorative borders.
///   - SvgDocument: In-memory parsed SVG vector graphic supporting paths, fills, strokes, and viewBox.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/rendering/color.hpp"
#include "enki/rendering/paint.hpp"
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <optional>

namespace enki {

class Canvas;

/// Mode for scaling vector graphics into destination box
enum class SvgFit {
    Stretch,  ///< Stretch vector to completely fill the target rectangle.
    Contain,  ///< Scale uniformly maintaining aspect ratio to fit inside target rectangle.
    Cover,    ///< Scale uniformly to completely cover the target rectangle (may clip).
};

/// 9-Slice insets for vector border slicing (preserving corners)
struct SvgSlice {
    float top    = 0.0f;
    float right  = 0.0f;
    float bottom = 0.0f;
    float left   = 0.0f;

    constexpr SvgSlice() = default;
    constexpr SvgSlice(float t, float r, float b, float l)
        : top(t), right(r), bottom(b), left(l) {}

    static constexpr SvgSlice all(float v) {
        return SvgSlice(v, v, v, v);
    }
    static constexpr SvgSlice symmetric(float vertical, float horizontal) {
        return SvgSlice(vertical, horizontal, vertical, horizontal);
    }
    bool operator==(const SvgSlice&) const = default;
};

/// Parsed, resolution-independent SVG vector document
class SvgDocument {
public:
    /// Parse SVG from raw SVG XML string, direct SVG path ('M...'), or file path (e.g. 'assets/frame.svg')
    static std::shared_ptr<SvgDocument> parse(std::string_view svg_or_path);

    virtual ~SvgDocument() = default;

    [[nodiscard]] virtual bool isValid() const = 0;
    [[nodiscard]] virtual Rect getBounds() const = 0;

    /// Render vector fitted to destination bounds
    virtual void render(Canvas& canvas, const Rect& dst, SvgFit fit = SvgFit::Stretch,
                        const Paint* override_paint = nullptr, bool is_stroke = false) = 0;

    /// Render vector using 9-slice corner-preserving scaling
    virtual void renderNineSlice(Canvas& canvas, const Rect& dst, const SvgSlice& slice,
                                 const Paint* override_paint = nullptr, bool is_stroke = false) = 0;
};

} // namespace enki
