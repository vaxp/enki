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

struct PlaceholderProps {
    Key key = Key::none();
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
/// Placeholder Widget Implementation
/// ════════════════════════════════════════════════════════════════

class PlaceholderWidget : public SingleChildRenderObjectWidget {
public:
    PlaceholderProps options;

    PlaceholderWidget() : SingleChildRenderObjectWidget(Key::none(), nullptr) {}
    explicit PlaceholderWidget(PlaceholderProps opts)
        : SingleChildRenderObjectWidget(opts.key, nullptr), options(std::move(opts)) {}
    PlaceholderWidget(Key key, PlaceholderProps opts)
        : SingleChildRenderObjectWidget(std::move(key), nullptr), options(std::move(opts)) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
    [[nodiscard]] std::string_view typeName() const override { return "Placeholder"; }
};

/// ════════════════════════════════════════════════════════════════
/// Declarative Proxy Struct (C++20 Designated Initializers)
/// ════════════════════════════════════════════════════════════════

struct Placeholder {
    Key key = Key::none();
    PlaceholderStyle style = PlaceholderStyle::Blueprint;

    float width  = 200.0f;
    float height = 120.0f;
    float corner_radius = 8.0f;
    float stroke_width = 1.5f;

    std::string label;
    std::string sublabel;
    std::string icon = "📷";

    bool show_dimensions = true;
    bool animated_shimmer = true;

    Color background_color = 0x22334155;
    Color stroke_color     = 0xFF475569;
    Color crosshair_color  = 0x4464748B;
    Color shimmer_color    = 0x4438BDF8;
    Color text_color       = 0xFFCBD5E1;
    Color badge_bg_color   = 0xCC0F172A;

    std::function<void()> on_tap = nullptr;

    operator WidgetPtr() const {
        PlaceholderProps opts;
        opts.key = key;
        opts.style = style;
        opts.width = width;
        opts.height = height;
        opts.corner_radius = corner_radius;
        opts.stroke_width = stroke_width;
        opts.label = label;
        opts.sublabel = sublabel;
        opts.icon = icon;
        opts.show_dimensions = show_dimensions;
        opts.animated_shimmer = animated_shimmer;
        opts.background_color = background_color;
        opts.stroke_color = stroke_color;
        opts.crosshair_color = crosshair_color;
        opts.shimmer_color = shimmer_color;
        opts.text_color = text_color;
        opts.badge_bg_color = badge_bg_color;
        opts.on_tap = on_tap;

        return std::make_shared<PlaceholderWidget>(key, std::move(opts));
    }
};

/// Pre-composed skeleton card layout
WidgetPtr placeholderCardSkeleton(float w = 280.0f);

/// Pre-composed skeleton list layout
WidgetPtr placeholderListSkeleton(int rows = 3, float w = 320.0f);

} // namespace enki
