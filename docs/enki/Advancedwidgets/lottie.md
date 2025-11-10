# Lottie

> A declarative vector animation widget that renders Lottie and Bodymovin JSON animations at native GPU speeds via Skia Skottie, featuring timeline controllers, named marker playback, interactive hover/tap triggers, and dynamic layer recoloring.

- **Header File**: `#include "enki/widgets/lottie.hpp"`
- **C++ Class**: `enki::LottieWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::LottieProps` (converts implicitly to `WidgetPtr`)
- **Style Model**: `enki::LottieStyle`
- **Factory Helper**: `enki::lottie()`
- **Controller**: `enki::LottieController` (`<enki/animation/lottie_controller.hpp>`)
- **Resource Model**: `enki::LottieComposition` (`<enki/rendering/lottie_composition.hpp>`)
- **Cache System**: `enki::LottieCache` (`<enki/rendering/lottie_composition.hpp>`)

---

## Overview

`Lottie` enables developers to drop vector animations exported from Adobe After Effects directly into native desktop C++ applications. Built on Skia's `skottie` runtime, it offers:
- **Flawless Resolution Independence**: Vector shapes, strokes, and gradients scale smoothly to any DPI or display resolution.
- **Microsecond Caching (`LottieCache`)**: Ensures animations are parsed only once into an in-memory composition graph and shared across widget instances.
- **Interactive State Triggers**: Built-in `animate_on_hover` and `animate_on_tap` flags allow micro-interactions without writing boilerplate stateful controllers.
- **Dynamic Layer Recoloring**: Real-time programmatic tinting of individual shape nodes via `composition->setColor("NodeName", Color)`.

---

## C++ API Definition

### Declarative Props Struct & Factory
```cpp
namespace enki {

struct LottieProps {
    Key                                key              = Key::none();
    std::shared_ptr<LottieComposition> composition      = nullptr;             ///< Pre-parsed composition resource
    std::string                        asset;                                  ///< Filesystem asset path (e.g. "anim.json")
    std::string                        json_data;                              ///< Raw JSON string
    std::shared_ptr<LottieController>  controller       = nullptr;             ///< External timeline controller

    std::optional<StyleValue>          width            = std::nullopt;
    std::optional<StyleValue>          height           = std::nullopt;
    std::optional<StyleValue>          min_width        = std::nullopt;
    std::optional<StyleValue>          min_height       = std::nullopt;
    std::optional<StyleValue>          max_width        = std::nullopt;
    std::optional<StyleValue>          max_height       = std::nullopt;

    BoxFit                             fit              = BoxFit::Contain;     ///< Scale strategy
    Alignment                          alignment        = Alignment::Center;   ///< Viewport positioning
    BorderRadius                       border_radius    = BorderRadius::zero();///< Corner clipping
    BoxShape                           shape            = BoxShape::Rectangle; ///< Boundary mask

    float                              opacity          = 1.0f;
    bool                               clip_content     = true;
    bool                               auto_play        = true;                ///< Start playing immediately
    bool                               repeat           = true;                ///< Loop indefinitely
    float                              speed            = 1.0f;                ///< Playback speed multiplier
    std::string                        marker;                                 ///< Play specific named marker segment

    bool                               animate_on_hover = false;               ///< Auto-play forward on mouse enter
    bool                               animate_on_tap   = false;               ///< Auto-play forward on click

    std::function<void()>              on_end           = nullptr;             ///< Dispatched when playback completes
    std::function<void(int)>           on_loop          = nullptr;             ///< Dispatched on each loop iteration

    operator WidgetPtr() const;
};

inline WidgetPtr lottie(const LottieProps& props = {});

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `asset` | `string` | `""` | File path to the Lottie `.json` animation file. |
| `json_data` | `string` | `""` | Raw JSON string content for embedded or network-fetched animations. |
| `composition` | `shared_ptr<LottieComposition>` | `nullptr` | Pre-parsed animation composition object. |
| `controller` | `shared_ptr<LottieController>` | `nullptr` | Custom timeline controller for scrubbing, pausing, and marker jumps. |
| `width`, `height` | `optional<StyleValue>` | `nullopt` | Fixed or proportional viewport layout dimensions. |
| `fit` | `BoxFit` | `BoxFit::Contain` | Geometry scaling mode (`Contain`, `Cover`, `Fill`, `FitWidth`, `FitHeight`). |
| `alignment` | `Alignment` | `Center` | Alignment of the vector canvas within the layout bounds. |
| `repeat` | `bool` | `true` | When true, loops playback continuously. |
| `speed` | `float` | `1.0f` | Playback speed scale factor (`2.0f` = double speed, `-1.0f` = reverse). |
| `animate_on_hover`| `bool` | `false` | Plays the animation on mouse hover and rewinds on exit. |
| `animate_on_tap` | `bool` | `false` | Plays the animation forward once when tapped. |
| `marker` | `string` | `""` | Plays only the timeline segment defined by this named marker. |

---

## Advanced Architecture: Controllers, Markers & Recoloring

### 1. `LottieController` Playback Modes & Scrubbing
```cpp
namespace enki {

enum class LottiePlaybackMode {
    Loop,     ///< Play forward continuously
    Once,     ///< Play once and stop at end frame
    PingPong, ///< Oscillate back and forth
    Segment   ///< Restrict playback between specific bounds
};

class LottieController {
public:
    void play();
    void pause();
    void stop();
    void reset();
    void forward();
    void reverse();
    void seek(float progress);                            // Scrub to progress in [0.0f..1.0f]
    void seekFrame(double frame_index);                   // Scrub to absolute frame
    void playSegment(float start, float end, bool loop);  // Play custom range
    bool playMarker(std::string_view name, bool loop);    // Play named marker ("intro", "outro")
    void setSpeed(float speed);
    void setPlaybackMode(LottiePlaybackMode mode);

    [[nodiscard]] float progress() const;
    [[nodiscard]] double currentFrame() const;
    [[nodiscard]] int loopCount() const;

    size_t addListener(std::function<void()> listener);
    void dispose();
};

} // namespace enki
```

### 2. `LottieComposition` Metadata & Dynamic Recoloring
```cpp
auto comp = LottieCache::getOrLoad("assets/animations/spinner.json");

// Query composition metadata
double durationSeconds = comp->duration();
double targetFps       = comp->fps();
Size   nativeSize      = comp->getSize();

// Dynamically recolor vector layers on the fly
comp->setColor("RingStroke", 0xFF38BDF8); // Change stroke color to Sky Blue
comp->setColor("HeartFill",  0xFFF43F5E); // Change fill color to Rose
```

---

## Code Examples (From `widgets_demo/lottie_demo/main.cpp`)

### 1. Simple Looping Spinner
```cpp
#include "enki/widgets/lottie.hpp"

using namespace enki;

WidgetPtr buildLoadingSpinner() {
    return lottie({
        .asset = "assets/animations/loading_spinner.json",
        .width = 100,
        .height = 100,
        .repeat = true,
        .speed = 1.0f
    });
}
```

### 2. Interactive Like Button (Animate on Hover / Tap)
```cpp
WidgetPtr buildLikeHeart() {
    return lottie({
        .asset = "assets/animations/heart_pulse.json",
        .width = 120,
        .height = 120,
        .animate_on_hover = true // Plays burst animation whenever user hovers
    });
}
```

### 3. Full Timeline Controller with Scrubbing & Recoloring
```cpp
class CustomPlayerState : public State {
    std::shared_ptr<LottieComposition> comp_;
    std::shared_ptr<LottieController>  controller_;

public:
    void initState() override {
        State::initState();
        comp_ = LottieCache::getOrLoad("assets/animations/success_check.json");
        
        // Dynamically override checkmark path color to emerald
        comp_->setColor("CheckmarkPath", 0xFF10B981);

        controller_ = std::make_shared<LottieController>(comp_);
        controller_->addListener([this]() {
            if (mounted()) setState([] {});
        });
        controller_->play();
    }

    void dispose() override {
        if (controller_) controller_->dispose();
        State::dispose();
    }

    WidgetPtr build(BuildContext&) override {
        return lottie({
            .controller = controller_,
            .width = 160,
            .height = 160,
            .fit = BoxFit::Contain
        });
    }
};
```

---

## See Also
- [**AnimatedContainer**](../Animation%20&%20Motion/animated_container.md) — Implicit property interpolation.
- [**SlideTransition**](../Animation%20&%20Motion/slide_transition.md) — Explicit positional translation.
- [**Pulse**](../Feedback/pulse.md) — Live beacon radar animation.
