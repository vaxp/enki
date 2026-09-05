# Physics-Based Spring Engine

> Physics-driven damped harmonic oscillator engine for natural, interruptible UI animations with exact analytical solutions and zero idle CPU consumption.

- **Header Files**:
  - `#include "enki/animation/spring_simulation.hpp"` (Physical equations, analytical solver, and `SpringCurve`)
  - `#include "enki/animation/spring_controller.hpp"` (Ticker-driven `SpringController` & `SpringValue<T>`)
- **Primary C++ Classes**:
  - `enki::SpringDescription` (Mass, stiffness, damping parameters)
  - `enki::SpringSimulation` (Closed-form analytical $O(1)$ harmonic oscillator)
  - `enki::SpringCurve` (Adapter implementing `enki::Curve` for standard animated widgets)
  - `enki::SpringController` (Interactive velocity-preserving physics driver)
  - `enki::SpringValue<T>` (Typed reactive spring wrapper for `float`, `Point`, `Size`, `Color`)
- **Standard Presets**: `enki::Springs` (`bouncy`, `smooth`, `snappy`, `interactive`, `gentle`)

---

## Overview & Architecture

Traditional animations rely on fixed duration bezier curves (e.g., cubic easing over 300ms). While predictable, fixed-duration curves feel artificial when interrupted by user gestures: retargeting mid-flight either causes an unnatural visual jump or resets duration, breaking physical momentum.

Enki's **Spring Engine** implements an **exact analytical closed-form solution** of the second-order differential equation governing damped harmonic oscillators:

$$m \frac{d^2x}{dt^2} + c \frac{dx}{dt} + k (x - x_{\text{target}}) = 0$$

Where:
- $m$: **Mass** (inertia of moving element, $m > 0$).
- $k$: **Stiffness** (spring tension pulling towards target, $k > 0$).
- $c$: **Damping** (frictional resistance dissipating kinetic energy, $c \ge 0$).

### Key Advantages
1. **True Momentum Preservation**: If target changes while in motion, `SpringController::animateTo()` retains current velocity $\frac{dx}{dt}$, seamlessly redirecting movement without visual jarring.
2. **Analytical $O(1)$ Evaluation**: Position $x(t)$ and velocity $\frac{dx}{dt}(t)$ are calculated in constant time without numerical integration (Euler/Verlet) errors or frame-rate dependency.
3. **Zero Idle CPU**: The controller monitors kinetic and potential energy thresholds (`distance_tolerance` and `velocity_tolerance`). Once settled, the internal `Ticker` automatically stops (0.0% CPU).
4. **Direct Curve Compatibility**: Via `SpringCurve`, spring dynamics can be plugged into existing implicit widgets (`AnimatedContainer`, `AnimatedSlide`, etc.).

---

## Physics Parameters & Presets

### `SpringDescription`
Defines the physical characteristics of the spring-mass-damper system:

```cpp
namespace enki {

struct SpringDescription {
    float mass      = 1.0f;    ///< Mass of the moving object (m > 0)
    float stiffness = 180.0f;  ///< Spring stiffness constant k (k > 0)
    float damping   = 12.0f;   ///< Frictional damping drag coefficient c (c >= 0)

    constexpr SpringDescription() = default;
    constexpr SpringDescription(float m, float k, float d);

    /// Construct via damping ratio zeta and natural frequency omega0
    static SpringDescription fromRatioAndFrequency(float damping_ratio, float natural_frequency, float mass = 1.0f);

    [[nodiscard]] float dampingRatio() const;      ///< zeta = c / (2 * sqrt(k * m))
    [[nodiscard]] float naturalFrequency() const;  ///< omega0 = sqrt(k / m)
};

} // namespace enki
```

### Standard Presets (`enki::Springs`)

| Preset | Mass | Stiffness | Damping | Damping Ratio ($\zeta$) | Behavior / Intended Use |
|---|---|---|---|---|---|
| `Springs::bouncy` | `1.0f` | `180.0f` | `12.0f` | $\approx 0.45$ (Underdamped) | Playful, organic bounce with visible overshoot. Ideal for badges, likes, cards. |
| `Springs::smooth` | `1.0f` | `180.0f` | `26.83f`| $1.00$ (Critically Damped)| Buttery smooth with zero overshoot. Matches SwiftUI/macOS default physics. |
| `Springs::snappy` | `1.0f` | `300.0f` | `30.0f` | $\approx 0.87$ (Underdamped) | Fast, highly responsive with micro-bounce. Ideal for buttons, toggles, selectors. |
| `Springs::interactive` | `0.5f` | `250.0f` | `20.0f` | $\approx 0.89$ (Underdamped) | Low mass, ultra-responsive tracking for direct gestures, drag, and sliders. |
| `Springs::gentle` | `1.0f` | `100.0f` | `15.0f` | $\approx 0.75$ (Underdamped) | Soft, graceful travel for large panels, bottom sheets, and modal dialogs. |

---

## C++ API Reference

### 1. `SpringController`
Controls physical motion using a platform `Ticker`.

```cpp
namespace enki {

class SpringController {
public:
    using Listener = std::function<void()>;
    using StatusListener = std::function<void(AnimationStatus)>;

    explicit SpringController(SpringDescription desc = Springs::smooth, float initial_value = 0.0f);
    ~SpringController();

    // Configuration
    void setSpring(const SpringDescription& desc);
    [[nodiscard]] const SpringDescription& spring() const;
    void setTolerance(float distance_tol, float velocity_tol = 1e-2f);

    // Interactive Control
    void animateTo(float target, std::optional<float> initial_velocity = std::nullopt);
    void snapTo(float value);
    void stop();
    void reset();

    // State Queries
    [[nodiscard]] float value() const;
    [[nodiscard]] float velocity() const;
    [[nodiscard]] float target() const;
    [[nodiscard]] bool isAnimating() const;
    [[nodiscard]] AnimationStatus status() const;

    // Listeners & Lifecycle
    void addListener(Listener listener);
    void addStatusListener(StatusListener listener);
    void clearListeners();
    void dispose();
};

} // namespace enki
```

### 2. `SpringValue<T>`
Type-safe property wrapper supporting `float`, `Point`, `Size`, and `Color`.

```cpp
namespace enki {

template<typename T>
class SpringValue {
public:
    explicit SpringValue(T initial, SpringDescription desc = Springs::smooth);

    void animateTo(T target, std::optional<float> velocity = std::nullopt);
    void snapTo(T val);

    [[nodiscard]] T get() const;
    [[nodiscard]] bool isAnimating() const;
    void addListener(SpringController::Listener l);
    SpringController& controller();
};

} // namespace enki
```

### 3. `SpringCurve`
Curve adapter mapping spring physics to normalized $[0, 1]$ time:

```cpp
namespace enki {

class SpringCurve final : public Curve {
public:
    explicit SpringCurve(SpringDescription desc = Springs::bouncy, float settling_duration = 0.6f);
    [[nodiscard]] double evaluate(double t) const override;
};

} // namespace enki
```

---

## Real Code Examples

### Example 1: Interactive Physics Puck with Presets (`widgets_demo/animation_suite_demo/main.cpp`)

```cpp
#include "enki/animation/spring_simulation.hpp"
#include "enki/animation/spring_controller.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/skia_canvas.hpp"
#include "enki/state/state.hpp"

using namespace enki;

class SpringDemoWidget : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<SpringDemoState>();
    }
    std::string_view typeName() const override { return "SpringDemoWidget"; }
};

class SpringDemoState : public State {
private:
    SpringController spring_ctrl_{Springs::bouncy, 0.0f};

public:
    void initState() override {
        State::initState();
        spring_ctrl_.addListener([this] {
            setState([] {}); // Re-render UI when physics ticks
        });
    }

    void dispose() override {
        spring_ctrl_.dispose();
        State::dispose();
    }

    WidgetPtr build(BuildContext&) override {
        float spring_val = spring_ctrl_.value();
        float puck_x = 40.0f + spring_val * 480.0f;

        return column({
            .gap = StyleValue::point(16.0f),
            .children = {
                // Preset buttons
                row({
                    .gap = StyleValue::point(8.0f),
                    .children = {
                        button(ButtonProps{
                            .child = text("Bouncy", { .color = 0xFFFFFFFF }),
                            .on_pressed = [this] {
                                spring_ctrl_.setSpring(Springs::bouncy);
                                spring_ctrl_.animateTo(spring_ctrl_.target() > 0.5f ? 0.0f : 1.0f);
                            },
                            .normal_color = 0xFF3B82F6,
                        }),
                        button(ButtonProps{
                            .child = text("Snappy", { .color = 0xFFFFFFFF }),
                            .on_pressed = [this] {
                                spring_ctrl_.setSpring(Springs::snappy);
                                spring_ctrl_.animateTo(spring_ctrl_.target() > 0.5f ? 0.0f : 1.0f);
                            },
                            .normal_color = 0xFFF59E0B,
                        }),
                    }
                }),

                // Physics Canvas Track
                skiaCanvas(SkiaCanvasProps{
                    .painter = [puck_x](Canvas& canvas, Size size) {
                        Paint track_paint;
                        track_paint.setColor(0xFF334155);
                        track_paint.setStrokeWidth(6.0f);
                        canvas.drawLine({40.0f, size.height * 0.5f}, {size.width - 40.0f, size.height * 0.5f}, track_paint);

                        Paint puck_paint;
                        puck_paint.setColor(0xFF38BDF8);
                        canvas.drawCircle({puck_x, size.height * 0.5f}, 18.0f, puck_paint);
                    },
                    .width = StyleValue::point(580.0f),
                    .height = StyleValue::point(70.0f),
                })
            }
        });
    }
};
```

### Example 2: Using `SpringCurve` with `AnimatedContainer`

```cpp
#include "enki/animation/spring_simulation.hpp"
#include "enki/widgets/motion.hpp"

using namespace enki;

// Create an adapter curve from a Bouncy spring
static SpringCurve s_bouncy_curve(Springs::bouncy, 0.7f);

WidgetPtr buildSpringyCard(bool is_expanded) {
    return animatedContainer({
        .color = is_expanded ? 0xFF10B981 : 0xFF3B82F6,
        .width = is_expanded ? StyleValue::point(300.0f) : StyleValue::point(150.0f),
        .height = StyleValue::point(80.0f),
        .border_radius = BorderRadius::circular(16.0f),
        .duration = std::chrono::milliseconds(700),
        .curve = &s_bouncy_curve, // Direct spring physics
    });
}
```
