#pragma once
/// @file lottie_composition.hpp
/// @brief High-performance Lottie animation composition & caching backed by Skia Skottie.
///
/// Features:
///   - Fast decoding from JSON strings, memory buffers, and file paths.
///   - Thread-safe LRU/Map LottieCache to avoid re-parsing identical animations.
///   - Frame seeking via normalized progress [0..1], absolute frame index, or timestamp.
///   - Composition metadata: dimensions, FPS, duration, frame count, markers.
///   - Dynamic property overrides: layer recoloring, opacity tweaking, text replacement.
///   - Direct hardware-accelerated Skia GPU rendering.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/core/result.hpp"
#include "enki/rendering/color.hpp"
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <unordered_map>

namespace enki {

class Canvas;

// ════════════════════════════════════════════════════════════════
// LottieMarker — Named timeline segment
// ════════════════════════════════════════════════════════════════

struct LottieMarker {
    std::string name;
    float       start_time = 0.0f; ///< Start time in seconds relative to timeline origin
    float       end_time   = 0.0f; ///< End time in seconds relative to timeline origin
};

// ════════════════════════════════════════════════════════════════
// LottieComposition — Parsed Lottie Animation Resource
// ════════════════════════════════════════════════════════════════

class LottieComposition {
public:
    /// Load Lottie animation from filesystem path.
    static Result<std::shared_ptr<LottieComposition>> loadFromFile(std::string_view path);

    /// Load Lottie animation from JSON string.
    static Result<std::shared_ptr<LottieComposition>> loadFromJson(std::string_view json_content);

    /// Load Lottie animation from raw memory buffer.
    static Result<std::shared_ptr<LottieComposition>> loadFromMemory(const std::vector<uint8_t>& data);
    static Result<std::shared_ptr<LottieComposition>> loadFromMemory(const void* data, size_t size);

    ~LottieComposition();

    LottieComposition(const LottieComposition&) = delete;
    LottieComposition& operator=(const LottieComposition&) = delete;

    // ── Composition Metadata ─────────────────────────────────────
    [[nodiscard]] double duration() const;         ///< Total duration in seconds
    [[nodiscard]] double durationMs() const;       ///< Total duration in milliseconds
    [[nodiscard]] double fps() const;              ///< Target frame rate (frames / second)
    [[nodiscard]] double inPoint() const;          ///< In-point frame
    [[nodiscard]] double outPoint() const;         ///< Out-point frame
    [[nodiscard]] double frameCount() const;       ///< Total frame count
    [[nodiscard]] Size   getSize() const;          ///< Intrinsic composition size
    [[nodiscard]] float  getWidth() const;         ///< Intrinsic width in pixels
    [[nodiscard]] float  getHeight() const;        ///< Intrinsic height in pixels

    // ── Markers ──────────────────────────────────────────────────
    [[nodiscard]] const std::vector<LottieMarker>& getMarkers() const;
    [[nodiscard]] std::optional<LottieMarker> getMarker(std::string_view name) const;

    // ── Seeking & Timeline ───────────────────────────────────────
    /// Update animation state for normalized progress t in [0.0, 1.0].
    void seek(float progress);

    /// Update animation state for absolute frame index.
    void seekFrame(double frame_index);

    /// Update animation state for time in seconds.
    void seekTime(double seconds);

    // ── Dynamic Properties ───────────────────────────────────────
    /// Dynamically override color of a named layer or shape node.
    void setColor(std::string_view node_name, Color color);

    /// Dynamically override opacity of a named layer or effect node [0.0..1.0].
    void setOpacity(std::string_view node_name, float opacity);

    // ── Rendering ────────────────────────────────────────────────
    /// Render current frame onto destination canvas and bounds.
    void render(Canvas& canvas, const Rect& dst, uint32_t render_flags = 0) const;

    /// Get native skottie::Animation* pointer
    [[nodiscard]] void* getNativeAnimation() const;

private:
    LottieComposition();
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

// ════════════════════════════════════════════════════════════════
// LottieCache — Thread-safe In-Memory LRU / Map Cache
// ════════════════════════════════════════════════════════════════

class LottieCache {
public:
    /// Get cached composition or parse from file if not present.
    static std::shared_ptr<LottieComposition> getOrLoad(std::string_view path);

    /// Store a composition in cache.
    static void put(std::string_view key, std::shared_ptr<LottieComposition> comp);

    /// Retrieve a composition if cached; nullptr otherwise.
    static std::shared_ptr<LottieComposition> get(std::string_view key);

    /// Clear all cached compositions.
    static void clear();

    /// Get current number of cached entries.
    static size_t count();
};

} // namespace enki
