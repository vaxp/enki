#pragma once
/// @file path_morph.hpp
/// @brief Vector SVG path morphing engine using Skia path measurement.
///
/// Features:
///   - Smoothly interpolates between any two arbitrary vector shapes.
///   - Uniform perimeter sampling via SkPathMeasure handles mismatched topologies.
///   - Built-in iconic shapes (Hamburger, Close, Play, Pause, Checkmark, Arrow).
///   - Direct Canvas rendering with auto-scaling to destination bounds.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/rendering/path.hpp"
#include "enki/rendering/paint.hpp"
#include "enki/rendering/canvas.hpp"
#include <string_view>
#include <vector>
#include <memory>

namespace enki {

// ════════════════════════════════════════════════════════════════
// SvgMorphPaths — Curated standard vector icons for morphing
// ════════════════════════════════════════════════════════════════

struct SvgMorphPaths {
    /// Hamburger 3-bar menu (box 0 0 100 100)
    static constexpr std::string_view hamburger =
        "M 15 25 L 85 25 M 15 50 L 85 50 M 15 75 L 85 75";

    /// Close 'X' icon (box 0 0 100 100)
    static constexpr std::string_view close =
        "M 20 20 L 80 80 M 80 20 L 20 80";

    /// Play button triangle (box 0 0 100 100)
    static constexpr std::string_view play =
        "M 25 15 L 85 50 L 25 85 Z";

    /// Pause button 2 bars (box 0 0 100 100)
    static constexpr std::string_view pause =
        "M 30 15 L 45 15 L 45 85 L 30 85 Z M 55 15 L 70 15 L 70 85 L 55 85 Z";

    /// Checkmark icon (box 0 0 100 100)
    static constexpr std::string_view checkmark =
        "M 18 52 L 40 74 L 82 26";

    /// Right arrow (box 0 0 100 100)
    static constexpr std::string_view arrow_right =
        "M 15 50 L 85 50 M 55 20 L 85 50 L 55 80";

    /// Star icon (box 0 0 100 100)
    static constexpr std::string_view star =
        "M 50 10 L 62 35 L 90 38 L 69 58 L 75 86 L 50 72 L 25 86 L 31 58 L 10 38 L 38 35 Z";

    /// Circle icon (box 0 0 100 100)
    static constexpr std::string_view circle =
        "M 50 10 A 40 40 0 1 0 50 90 A 40 40 0 1 0 50 10 Z";
};

// ════════════════════════════════════════════════════════════════
// PathMorph — Vector Path Interpolator
// ════════════════════════════════════════════════════════════════

class PathMorph {
public:
    PathMorph() = default;

    /// Construct from two SVG path data strings (e.g. "M10,10 L90,90 ...")
    PathMorph(std::string_view from_svg_path, std::string_view to_svg_path, size_t sample_points = 80);

    /// Construct from two existing Path objects
    PathMorph(const Path& from_path, const Path& to_path, size_t sample_points = 80);

    /// Compute interpolated Path at progress t in [0.0, 1.0]
    [[nodiscard]] std::shared_ptr<Path> evaluate(float t) const;

    /// Render morphed vector path scaled and centered into target rectangle dst
    void render(Canvas& canvas, const Rect& dst, float t, const Paint& paint, bool is_stroke = true) const;

    [[nodiscard]] bool isValid() const { return is_valid_; }
    [[nodiscard]] size_t sampleCount() const { return points_a_.size(); }
    [[nodiscard]] Rect bounds() const { return bounds_union_; }
    [[nodiscard]] bool isClosed() const { return is_closed_; }
    void setClosed(bool closed) { is_closed_ = closed; }

private:
    void resample(const void* sk_path_a, const void* sk_path_b, size_t samples);

    std::vector<Point> points_a_;
    std::vector<Point> points_b_;
    Rect bounds_union_{0, 0, 100, 100};
    bool is_closed_ = true;
    bool is_valid_  = false;
};

} // namespace enki
