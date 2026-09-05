# Animation Timeline & Staggered Sequencer

> Multi-track timeline orchestration, keyframe sequencing, cascading list/grid animations, and sub-window interval curves.

- **Header Files**:
  - `#include "enki/animation/timeline.hpp"` (Multi-track `AnimationTimeline`, `Keyframe<T>`, and `KeyframeSequence<T>`)
  - `#include "enki/animation/stagger.hpp"` (`IntervalCurve`, `StaggerConfig`, and `StaggerHelper`)
- **Primary C++ Classes**:
  - `enki::AnimationTimeline` (Choreographer managing parallel, delayed, or sequential animation tracks)
  - `enki::Keyframe<T>` & `enki::KeyframeSequence<T>` (Multi-stop value interpolation with per-segment curves)
  - `enki::IntervalCurve` (Curve adapter transforming progress within a sub-interval $[t_1, t_2] \subseteq [0, 1]$)
  - `enki::StaggerHelper` & `enki::StaggerConfig` (Cascading delay and progress math for collections)

---

## Overview & Architecture

When designing complex UIs, animating multiple elements simultaneously with simple individual controllers leads to messy, unsynchronized code. The **Timeline & Stagger Subsystem** solves this through two powerful complementary tools:

1. **`AnimationTimeline`**: A unified master conductor that hosts multiple animation tracks. Each track defines its own start offset, duration, easing curve, and callback. Supports scrubbing (`seek()`), variable playback speed (`setSpeed()`), looping, and ping-pong reverse.
2. **`StaggerHelper` & `IntervalCurve`**: Calculates cascading staggered arrival windows for $N$ items (such as items entering a list or dashboard cards popping in one by one), ensuring that total duration and item overlap are mathematically precise and decoupled from widget state.

---

## C++ API Reference

### 1. `Keyframe<T>` & `KeyframeSequence<T>`
Enables multi-stop path or property interpolation across arbitrary timestamps.

```cpp
namespace enki {

template<typename T>
struct Keyframe {
    float        time_fraction = 0.0f;           ///< Milestone position in track timeline [0.0, 1.0]
    T            value{};                        ///< Target value at this keyframe
    const Curve* curve = &Curves::linear;        ///< Easing curve transitioning towards this keyframe

    Keyframe() = default;
    Keyframe(float fraction, T val, const Curve* c = &Curves::linear);
};

template<typename T>
class KeyframeSequence {
public:
    KeyframeSequence() = default;
    explicit KeyframeSequence(std::vector<Keyframe<T>> keyframes);

    void addKeyframe(float fraction, T value, const Curve* curve = &Curves::linear);
    [[nodiscard]] T evaluate(float t) const;
};

} // namespace enki
```
*Specialized for `float`, `Point`, `Size`, and `Color`.*

### 2. `AnimationTimeline`
Master multi-track conductor.

```cpp
namespace enki {

class AnimationTimeline {
public:
    using Duration = std::chrono::milliseconds;
    using Listener = std::function<void()>;
    using StatusListener = std::function<void(AnimationStatus)>;

    AnimationTimeline();
    explicit AnimationTimeline(Duration duration_override);
    ~AnimationTimeline();

    // Track Composition
    AnimationTimeline& add(Duration start_offset, Duration duration,
                           std::function<void(float progress)> on_update,
                           const Curve* curve = &Curves::linear);

    template<typename T>
    AnimationTimeline& addTween(Duration start_offset, Duration duration,
                                Tween<T> tween,
                                std::function<void(const T& val)> on_update);

    template<typename T>
    AnimationTimeline& addKeyframes(Duration start_offset, Duration duration,
                                    KeyframeSequence<T> sequence,
                                    std::function<void(const T& val)> on_update);

    // Playback Controls
    void play();
    void pause();
    void stop();
    void reset();
    void forward();
    void reverse();
    void seek(float progress);   ///< Scrub to normalized [0.0, 1.0]
    void seekMs(int64_t ms);     ///< Scrub to exact millisecond

    // Playback Settings
    void setSpeed(float speed);        ///< Multiplier (1.0 = normal, 2.0 = 2x, 0.5 = slow-mo)
    void setRepeat(bool repeat);       ///< Loop continuously
    void setPingPong(bool pingpong);   ///< Reverse upon reaching ends

    // State Queries
    [[nodiscard]] Duration totalDuration() const;
    [[nodiscard]] float progress() const;
    [[nodiscard]] bool isPlaying() const;
    [[nodiscard]] AnimationStatus status() const;

    // Listeners & Lifecycle
    void addListener(Listener listener);
    void addStatusListener(StatusListener listener);
    void clearListeners();
    void dispose();
};

} // namespace enki
```

### 3. `IntervalCurve`
Maps a normalized $[0, 1]$ master progress into an active sub-window $[begin, end]$:

```cpp
namespace enki {

class IntervalCurve final : public Curve {
public:
    constexpr IntervalCurve(float begin, float end, const Curve* inner_curve = &Curves::linear);
    [[nodiscard]] double evaluate(double t) const override;
};

} // namespace enki
```

### 4. `StaggerHelper` & `StaggerConfig`
Orchestrates collection animations:

```cpp
namespace enki {

struct StaggerConfig {
    std::chrono::milliseconds item_duration{250};
    std::chrono::milliseconds delay_between_items{40};
    const Curve*               curve = &Curves::easeOut;
};

class StaggerHelper {
public:
    /// Total duration needed for N items given item duration and inter-item delay
    static std::chrono::milliseconds totalDuration(size_t item_count, const StaggerConfig& config);

    /// Computes the [begin, end] interval in [0, 1] for an item at index
    static std::pair<float, float> itemInterval(size_t index, size_t total_items, const StaggerConfig& config);

    /// Evaluates individual progress [0, 1] for item at index given master progress
    static float itemProgress(size_t index, size_t total_items, float master_progress, const StaggerConfig& config);
};

} // namespace enki
```

---

## Real Code Examples

### Example 1: Cascading Dashboard Cards (`widgets_demo/animation_suite_demo/main.cpp`)

```cpp
#include "enki/animation/timeline.hpp"
#include "enki/animation/stagger.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/state/state.hpp"

using namespace enki;

class StaggerDemoState : public State {
private:
    AnimationTimeline timeline_;
    float master_progress_ = 0.0f;

public:
    void initState() override {
        State::initState();

        timeline_.reset();
        timeline_.setRepeat(true);
        timeline_.setPingPong(true);

        // Drive master progress from 0.0 to 1.0 over 2000ms
        timeline_.add(std::chrono::milliseconds(0), std::chrono::milliseconds(2000), [this](float p) {
            master_progress_ = p;
            setState([] {}); // Re-render UI
        }, &Curves::easeInOut);

        timeline_.play();
    }

    void dispose() override {
        timeline_.dispose();
        State::dispose();
    }

    WidgetPtr build(BuildContext&) override {
        StaggerConfig stg_cfg{
            .item_duration = std::chrono::milliseconds(300),
            .delay_between_items = std::chrono::milliseconds(80),
            .curve = &Curves::easeOut,
        };

        const char* card_titles[4] = {"Dashboard", "Analytics", "Reports", "Security"};
        Color card_colors[4] = {0xFF3B82F6, 0xFF10B981, 0xFFF59E0B, 0xFFEC4899};
        std::vector<WidgetPtr> cards;

        for (size_t i = 0; i < 4; ++i) {
            // Compute item's individual progress based on stagger delays
            float p = StaggerHelper::itemProgress(i, 4, master_progress_, stg_cfg);
            float card_scale = 0.85f + 0.15f * p;

            cards.push_back(container({
                .color = 0xEB1E293B,
                .border_radius = BorderRadius::circular(12.0f),
                .border = Border(card_colors[i], 1.5f),
                .width = StyleValue::point(240.0f * card_scale),
                .height = StyleValue::point(100.0f),
                .padding = StyleInsets::all(14.0f),
                .child = column({
                    .justify_content = Justify::Center,
                    .align_items = Align::Center,
                    .children = {
                        text(card_titles[i], { .color = card_colors[i], .font_size = 16.0f, .font_weight = FontWeight::Bold }),
                        text("Cascading Stagger Entry", { .color = 0xFF94A3B8, .font_size = 12.0f }),
                    }
                })
            }));
        }

        return row({
            .justify_content = Justify::Center,
            .gap = StyleValue::point(16.0f),
            .children = cards,
        });
    }
};
```

### Example 2: Multi-Track Keyframe Choreography

```cpp
#include "enki/animation/timeline.hpp"

using namespace enki;

AnimationTimeline createComplexIntroTimeline(
    std::function<void(float opacity)> on_fade,
    std::function<void(Point pos)>     on_move,
    std::function<void(Color color)>   on_color) 
{
    AnimationTimeline tl;

    // Track 1: Fade in (0ms -> 400ms)
    tl.add(std::chrono::milliseconds(0), std::chrono::milliseconds(400),
           on_fade, &Curves::easeIn);

    // Track 2: Positional keyframes (200ms -> 1000ms)
    KeyframeSequence<Point> move_seq;
    move_seq.addKeyframe(0.0f, Point{0.0f, -50.0f}, &Curves::easeOut);
    move_seq.addKeyframe(0.7f, Point{0.0f, 10.0f}, &Curves::easeInOut);
    move_seq.addKeyframe(1.0f, Point{0.0f, 0.0f}, &Curves::bounceOut);

    tl.addKeyframes<Point>(std::chrono::milliseconds(200),
                           std::chrono::milliseconds(800),
                           move_seq, on_move);

    // Track 3: Color tween (600ms -> 1200ms)
    tl.addTween<Color>(std::chrono::milliseconds(600),
                       std::chrono::milliseconds(600),
                       Tween<Color>(0xFF3B82F6, 0xFF10B981),
                       on_color);

    return tl;
}
```
