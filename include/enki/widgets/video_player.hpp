#pragma once
/// @file video_player.hpp
/// @brief Declarative VideoPlayer Widget with Hardware Zero-Copy & Skia GPU Rendering.
///
/// Features:
///   - Pure C++20 Designated Initializers syntax
///   - Hardware Zero-Copy video decoding (VA-API / DRM DMA-BUF / Skia GPU)
///   - Audio-Video Master Clock Synchronization
///   - Aspect Ratio Handling: BoxFit::Contain, Cover, Fill, FitWidth, FitHeight
///   - Sleek Glassmorphic Overlay HUD with Auto-hide on mouse idle
///   - Transport controls: Play, Pause, Seek Scrubber, Volume, Loop, Speed
///   - Zero Widget-Tree Rebuilds during playback (driven by RenderBox Ticker)
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/rendering/canvas.hpp"
#include "video/video_controller.hpp"

#include <string>
#include <memory>
#include <optional>

namespace enki {

struct VideoPlayerProps {
    std::string                             source          = "";
    std::shared_ptr<video::VideoController> controller      = nullptr;
    bool                                    auto_play       = true;
    bool                                    looping         = false;
    bool                                    show_controls   = true;
    BoxFit                                  fit             = BoxFit::Contain;
    BorderRadius                            border_radius   = BorderRadius::circular(12.0f);
    Color                                   background_color = 0xFF000000;
    float                                   volume          = 1.0f;
    float                                   playback_speed  = 1.0f;

    // Layout Sizing
    std::optional<StyleValue>               width           = std::nullopt;
    std::optional<StyleValue>               height          = std::nullopt;
    std::optional<StyleValue>               min_width       = std::nullopt;
    std::optional<StyleValue>               min_height      = std::nullopt;
    std::optional<StyleValue>               max_width       = std::nullopt;
    std::optional<StyleValue>               max_height      = std::nullopt;

    Key                                     key             = Key::none();

    operator WidgetPtr() const;
};

struct VideoPlayer : public VideoPlayerProps {
    using VideoPlayerProps::VideoPlayerProps;
};

inline WidgetPtr videoPlayer(const VideoPlayerProps& props = {}) {
    return static_cast<WidgetPtr>(props);
}

class VideoPlayerWidget : public SingleChildRenderObjectWidget {
public:
    VideoPlayerProps props;

    explicit VideoPlayerWidget(VideoPlayerProps p)
        : SingleChildRenderObjectWidget(p.key, nullptr), props(std::move(p)) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
    [[nodiscard]] std::string_view typeName() const override { return "VideoPlayer"; }
};

} // namespace enki
