# VideoPlayer

> High-performance native hardware-accelerated video player widget for ENKI, featuring Zero-Copy VA-API / DRM DMA-BUF GPU rendering, PulseAudio Master-Clock audio synchronization, and a sleek glassmorphic overlay HUD.

- **Header File**: `#include "enki/widgets/video_player.hpp"`
- **C++ Class**: `enki::VideoPlayerWidget` (inherits from `enki::SingleChildRenderObjectWidget`)
- **Declarative Struct**: `enki::VideoPlayerProps`, `enki::VideoPlayer` (converts implicitly to `WidgetPtr`)
- **Render Object**: `enki::RenderVideoPlayer` (inherits from `enki::RenderBox`)
- **Media Subsystem**: `video/` (`<video/video_controller.hpp>`, `<video/video_decoder.hpp>`, `<video/video_types.hpp>`)
- **Factory Helper**: `enki::videoPlayer()`

---

## Architecture & Zero-Copy Pipeline

The ENKI Video Subsystem is cleanly isolated in `/home/x/Work/enki/video/` and compiled as an independent static library `libenki_video.a`. It is 100% free of  dependencies, relying purely on the industry-standard **FFmpeg 6.1** engine.

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    Zero-Copy Hardware Video Pipeline                        │
│                                                                             │
│  [Video File / Stream]                                                      │
│           │                                                                 │
│           ▼ (H.264 / HEVC / VP9 / AV1 encoded packets)                      │
│  [Hardware Decoder via VA-API / renderD128]                                 │
│           │                                                                 │
│           ▼ (Decoded directly into VRAM surfaces — AV_PIX_FMT_VAAPI)        │
│  [DRM PRIME DMA-BUF Descriptor (AVDRMFrameDescriptor)]                      │
│           │                                                                 │
│           ▼ (Zero CPU bytes copied, zero memory churn)                      │
│  [EGLImageKHR / OpenGL Texture Binding]                                     │
│           │                                                                 │
│           ▼ (Color conversion done inside Skia GPU Fragment Shader)         │
│  [Skia GPU Canvas: context.canvas.drawImageRect(...)]                       │
│           │                                                                 │
│           ▼ (60+ FPS, ~0% CPU utilization)                                  │
│  [Wayland / X11 Window Surface]                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### Audio-Video Synchronization (A/V Sync)
Audio and video synchronization is achieved using an **Audio Clock Master** approach:
1. Audio packets are decoded and resampled to 48kHz Stereo using `libswresample`.
2. Audio samples are output via a native low-latency PulseAudio playback stream (`PA_STREAM_PLAYBACK`).
3. Hardware latency is monitored with `pa_simple_get_latency()`, establishing the authoritative Master Clock time.
4. The video rendering pipeline presents frames when `frame.pts <= master_clock` with intelligent frame-skipping to prevent latency accumulation.

---

## C++ Declarative Definition

```cpp
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

    std::optional<StyleValue>               width           = std::nullopt;
    std::optional<StyleValue>               height          = std::nullopt;
    std::optional<StyleValue>               min_width       = std::nullopt;
    std::optional<StyleValue>               min_height      = std::nullopt;
    std::optional<StyleValue>               max_width       = std::nullopt;
    std::optional<StyleValue>               max_height      = std::nullopt;

    Key                                     key             = Key::none();

    operator WidgetPtr() const;
};

inline WidgetPtr videoPlayer(const VideoPlayerProps& props = {});

} // namespace enki
```

---

## Usage Examples

### 1. Basic Media Player

```cpp
#include "enki/widgets/video_player.hpp"

using namespace enki;

WidgetPtr buildPlayer() {
    return videoPlayer({
        .source = "assets/trailers/nature.mp4",
        .auto_play = true,
        .looping = true,
        .show_controls = true,
        .fit = BoxFit::Contain,
        .border_radius = BorderRadius::circular(16.0f),
        .width = 800.0f,
        .height = 450.0f,
    });
}
```

### 2. Custom VideoController Integration

```cpp
auto controller = std::make_shared<video::VideoController>();
controller->open("assets/video.mp4");
controller->setPlaybackSpeed(1.25f);
controller->setVolume(0.8f);

// Inside widget tree:
auto player = videoPlayer({
    .controller = controller,
    .fit = BoxFit::Cover,
    .show_controls = true,
    .width = StyleValue::percent(100.0f),
    .height = 500.0f,
});

// Controlling playback programmatically:
controller->togglePlay();
controller->seek(45.0); // Jump to 45 seconds
```
