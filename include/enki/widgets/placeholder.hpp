#pragma once
/// @file placeholder.hpp
/// @brief Advanced Placeholder & Skeleton Shimmer widget for ENKI Framework (Category 2. Basic UI).
/// Supports Blueprint Crosshairs with real-time dimension tags, Animated Skeleton Shimmer loaders,
/// Empty State Media Slots with dashed borders, and Pre-composed Skeleton Templates.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/rendering/color.hpp"

#include <string>
#include <string_view>
#include <functional>
#include <memory>
#include <vector>
#include <optional>

namespace enki {

/// ════════════════════════════════════════════════════════════════
/// Placeholder Enums & Options
/// ════════════════════════════════════════════════════════════════

enum class PlaceholderStyle {
    Blueprint,      ///< Wireframe crosshair with diagonal X and dimension tag
    Skeleton,       ///< Smooth animated linear shimmer loader
    MediaSlot,      ///< Dashed border empty state / drop zone slot
    Solid           ///< Clean tinted scaffolding block
};

struct PlaceholderOptions {
    PlaceholderStyle style = PlaceholderStyle::Blueprint;

    float width  = 200.0f;
    float height = 120.0f;
    float corner_radius = 8.0f;
    float stroke_width = 1.5f;

    std::string label;               ///< Custom title / slot tag (e.g. "Chart Slot")
    std::string sublabel;            ///< Optional subtext (e.g. "Drop file here")
    std::string icon = "📷";         ///< Icon for MediaSlot style

    bool show_dimensions = true;     ///< Shows "300 × 200 px" badge
    bool animated_shimmer = true;    ///< Shimmer animation for skeleton mode

    // Styling Colors
    Color background_color = 0x22334155; // Translucent Slate
    Color stroke_color     = 0xFF475569; // Slate 600
    Color crosshair_color  = 0x4464748B; // Slate 500 cross lines
    Color shimmer_color    = 0x4438BDF8; // Sky 400 Shimmer Highlight
    Color text_color       = 0xFFCBD5E1; // Slate 300
    Color badge_bg_color   = 0xCC0F172A; // Deep Slate Badge

    // Callbacks
    std::function<void()> on_tap;
};

/// ════════════════════════════════════════════════════════════════
/// Placeholder Widget
/// ════════════════════════════════════════════════════════════════

class Placeholder : public SingleChildRenderObjectWidget {
public:
    PlaceholderOptions options;

    Placeholder() : SingleChildRenderObjectWidget(Key::none(), nullptr) {}
    explicit Placeholder(PlaceholderOptions opts)
        : SingleChildRenderObjectWidget(Key::none(), nullptr), options(std::move(opts)) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
    [[nodiscard]] std::string_view typeName() const override { return "Placeholder"; }
};

/// ════════════════════════════════════════════════════════════════
/// Convenience Factory Helpers
/// ════════════════════════════════════════════════════════════════

inline std::shared_ptr<Placeholder> placeholder(PlaceholderOptions options = {}) {
    return std::make_shared<Placeholder>(std::move(options));
}

inline std::shared_ptr<Placeholder> placeholderBlueprint(float w = 240.0f, float h = 140.0f, std::string label = "") {
    PlaceholderOptions opts;
    opts.style = PlaceholderStyle::Blueprint;
    opts.width = w;
    opts.height = h;
    opts.label = std::move(label);
    return std::make_shared<Placeholder>(opts);
}

inline std::shared_ptr<Placeholder> placeholderSkeleton(float w = 200.0f, float h = 20.0f, float radius = 4.0f) {
    PlaceholderOptions opts;
    opts.style = PlaceholderStyle::Skeleton;
    opts.width = w;
    opts.height = h;
    opts.corner_radius = radius;
    opts.show_dimensions = false;
    return std::make_shared<Placeholder>(opts);
}

inline std::shared_ptr<Placeholder> placeholderMediaSlot(std::string label = "Upload Media",
                                                         std::string icon = "📁",
                                                         float w = 280.0f, float h = 160.0f,
                                                         std::function<void()> on_tap = nullptr) {
    PlaceholderOptions opts;
    opts.style = PlaceholderStyle::MediaSlot;
    opts.label = std::move(label);
    opts.icon = std::move(icon);
    opts.width = w;
    opts.height = h;
    opts.on_tap = std::move(on_tap);
    return std::make_shared<Placeholder>(opts);
}

/// Pre-composed skeleton card layout
WidgetPtr placeholderCardSkeleton(float w = 280.0f);

/// Pre-composed skeleton list layout
WidgetPtr placeholderListSkeleton(int rows = 3, float w = 320.0f);

} // namespace enki
