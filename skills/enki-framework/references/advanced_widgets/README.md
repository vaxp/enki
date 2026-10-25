# Enki Advanced Widgets & Vector Animations Suite

> GPU-accelerated vector animation runtime powered by Skia Skottie, supporting high-FPS Lottie/Bodymovin JSON playback, timeline controllers, marker segments, interactive hover/tap triggers, and dynamic vector layer recoloring.

The **Advanced Widgets** category hosts specialized, high-performance visual components that integrate deep graphical runtimes into Enki's layout and rendering trees.

---

## Architectural Highlights: The Lottie & Skottie Engine

Enki integrates Google's hardware-accelerated **Skia Skottie** animation module directly into the rendering pipeline:

```
┌─────────────────────────────────────────────────────────────┐
│                         LottieWidget                        │
│                (LottieProps / LottieStyle)                  │
└──────────────────────────────┬──────────────────────────────┘
                               │
       ┌───────────────────────┴───────────────────────┐
       ▼                                               ▼
┌───────────────────────────────┐     ┌───────────────────────────────┐
│     Anu Layout (Flexbox)      │     │       LottieController        │
│ - Intrinsic sizing & aspect   │     │ - Play, Pause, Reverse, Seek  │
│ - BoxFit (Contain, Cover...)  │     │ - Markers ("intro", "hover")  │
│ - Responsive constraints      │     │ - Frame rate ticker sync      │
└──────────────┬────────────────┘     └───────────────┬───────────────┘
               │                                      │
               └──────────────────┬───────────────────┘
                                  ▼
┌─────────────────────────────────────────────────────────────┐
│                 Skia GPU Rendering Pipeline                 │
│ - skottie::Animation decoding & frame evaluation            │
│ - Thread-safe in-memory caching via LottieCache             │
│ - Subpixel anti-aliasing with zero rasterization artifacts  │
│ - Real-time dynamic layer color & opacity overrides         │
└─────────────────────────────────────────────────────────────┘
```

- **Resolution Independence**: Vector animations scale flawlessly to 4K and 8K displays without blurriness or memory inflation.
- **Microsecond In-Memory Caching (`LottieCache`)**: Prevents repetitive file reads and JSON parses across duplicate widget instances.
- **Granular Timeline Scrubbing**: Frame-accurate seeking via normalized progress `[0.0..1.0]`, absolute frame numbers, or named timeline markers.
- **Live Vector Recoloring**: Modify strokes, fills, and opacities in real-time (`composition->setColor("NodeName", color)`) for theme matching and state alerts without reloading the JSON asset.

---

## Widget Catalog (Advanced Widgets)

| # | Widget | Primary Type / Helper | Header | Description |
|---|---|---|---|---|
| 1 | [**Lottie**](./lottie.md) | `struct LottieProps`, `lottie(...)` | `<enki/widgets/lottie.hpp>` | High-performance vector animation player with controllers, markers, and hover triggers. |
| 2 | [**SkiaCanvas**](./skia_canvas.md) | `struct SkiaCanvas`, `skiaCanvas(...)` | `<enki/widgets/skia_canvas.hpp>` | Raw Skia 2D canvas widget exposing PaintCallback for custom GPU drawing. |
| 3 | [**AudioWaveform**](./audio_waveform.md) | `struct AudioWaveform`, `audioWaveform(...)` | `<enki/widgets/audio_waveform.hpp>` | Real-time audio spectrum analyzer (Wave Music) and live microphone waveform (Wave Mic). |
| 4 | [**VideoPlayer**](./video_player.md) | `struct VideoPlayerProps`, `videoPlayer(...)` | `<enki/widgets/video_player.hpp>` | Hardware Zero-Copy video player with A/V Master Clock sync and glassmorphic HUD. |

---

## Quick Example (Interactive Lottie Icon with Hover & Scrubbing)

```cpp
#include "enki/widgets/lottie.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"

using namespace enki;

WidgetPtr buildHeartLikeButton() {
    return container({
        .color = 0xFF1E293B,
        .border_radius = BorderRadius::circular(12.0f),
        .padding = EdgeInsets::all(16.0f),
        .child = lottie({
            .asset = "assets/animations/heart_pulse.json",
            .width = 120,
            .height = 120,
            .animate_on_hover = true,  // Automatically plays forward when hovered
            .fit = BoxFit::Contain,
            .alignment = Alignment::Center
        })
    });
}
```
