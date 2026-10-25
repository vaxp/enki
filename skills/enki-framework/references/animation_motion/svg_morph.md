# SVG Vector Path Morphing

> Continuous vector shape interpolation and topology normalization powered by Skia path measurement.

- **Header Files**:
  - `#include "enki/animation/path_morph.hpp"` (Core `PathMorph` vector interpolation engine & `SvgMorphPaths` presets)
  - `#include "enki/widgets/svg_morph.hpp"` (Declarative `SvgMorph` widget and `svgMorph()` factory)
- **Primary C++ Classes**:
  - `enki::PathMorph` (Continuous arc-length perimeter sampling & evaluation engine)
  - `enki::SvgMorph` (Declarative widget struct supporting C++20 designated initializers)
  - `enki::SvgMorphPaths` (Standard iconic path presets: Menu, Close, Play, Pause, Checkmark, Arrow, Star, Circle)

---

## Overview & Architecture

Interpolating between two vector icons in standard SVG engines often fails when the paths contain different numbers of vertices or topologies (for example, a 3-segment Hamburger menu transitioning into an intersecting 'X' Close icon, or a 3-vertex Play triangle morphing into a 2-segment Pause bar).

Enki's **`PathMorph` Engine** solves this by implementing **uniform arc-length perimeter resampling** via Skia (`SkPathMeasure`):

1. **Topology Normalization**: Regardless of how many commands or segments exist in the original SVG paths, both curves are parameterized by their normalized length $s \in [0, L]$.
2. **Equidistant Sampling**: Both contours are sampled into $N$ equidistant points (default 80 points) along their perimeters.
3. **Linear Vertex Interpolation**: When evaluating progress $t \in [0, 1]$, each corresponding point $P_A(i)$ and $P_B(i)$ is linearly blended in constant time:
   $$P(i, t) = (1 - t) P_A(i) + t P_B(i)$$
4. **Auto-Fitting & Scaling**: The resulting vector shape is automatically centered and scaled to fit the destination widget rectangle (`Rect{0, 0, width, height}`).
5. **Thread-Safe Caching**: Morph instances are cached globally via a mutex-guarded lookup table so identical path transitions do not recompute sampling tables.

---

## Built-in Icon Presets (`SvgMorphPaths`)

All standard presets are defined in a normalized coordinate viewport of `100 x 100`:

| Preset Constant | SVG Path String | Visual Preview |
|---|---|---|
| `SvgMorphPaths::hamburger` | `"M 15 25 L 85 25 M 15 50 L 85 50 M 15 75 L 85 75"` | 3-bar hamburger menu |
| `SvgMorphPaths::close` | `"M 20 20 L 80 80 M 80 20 L 20 80"` | 'X' close / dismiss icon |
| `SvgMorphPaths::play` | `"M 25 15 L 85 50 L 25 85 Z"` | Right-facing play triangle |
| `SvgMorphPaths::pause` | `"M 30 15 L 45 15 L 45 85 L 30 85 Z M 55 15 L 70 15 L 70 85 L 55 85 Z"` | Dual pause bars |
| `SvgMorphPaths::checkmark` | `"M 18 52 L 40 74 L 82 26"` | Checkmark icon |
| `SvgMorphPaths::arrow_right`| `"M 15 50 L 85 50 M 55 20 L 85 50 L 55 80"` | Right navigation arrow |
| `SvgMorphPaths::star` | `"M 50 10 L 62 35 L 90 38 L 69 58 L 75 86 L 50 72 L 25 86 L 31 58 L 10 38 L 38 35 Z"` | 5-point star |
| `SvgMorphPaths::circle` | `"M 50 10 A 40 40 0 1 0 50 90 A 40 40 0 1 0 50 10 Z"` | Smooth circle |

---

## C++ API Reference

### 1. `SvgMorph` Declarative Struct
```cpp
namespace enki {

struct SvgMorph {
    std::string_view    from_path     = SvgMorphPaths::hamburger; ///< Source SVG path data
    std::string_view    to_path       = SvgMorphPaths::close;     ///< Target SVG path data
    float               progress      = 0.0f;                     ///< Interpolation progress [0.0, 1.0]
    Color               color         = 0xFFFFFFFF;               ///< Stroke or fill color
    float               stroke_width  = 2.5f;                     ///< Vector stroke width
    bool                is_stroke     = true;                     ///< True for stroke, false for fill
    std::optional<bool> is_closed     = std::nullopt;             ///< Override contour closing
    StyleValue          width         = 24.0f;                    ///< Widget width
    StyleValue          height        = 24.0f;                    ///< Widget height
    Key                 key           = Key::none();

    operator WidgetPtr() const;
};

inline WidgetPtr svgMorph(const SvgMorph& props = {});

} // namespace enki
```

### 2. `PathMorph` Core Engine
```cpp
namespace enki {

class PathMorph {
public:
    PathMorph() = default;
    PathMorph(std::string_view from_svg_path, std::string_view to_svg_path, size_t sample_points = 80);
    PathMorph(const Path& from_path, const Path& to_path, size_t sample_points = 80);

    [[nodiscard]] std::shared_ptr<Path> evaluate(float t) const;
    void render(Canvas& canvas, const Rect& dst, float t, const Paint& paint, bool is_stroke = true) const;

    [[nodiscard]] bool isValid() const;
    [[nodiscard]] size_t sampleCount() const;
    [[nodiscard]] Rect bounds() const;
    [[nodiscard]] bool isClosed() const;
    void setClosed(bool closed);
};

} // namespace enki
```

---

## Real Code Examples

### Example 1: Animated Iconic Transitions (`widgets_demo/animation_suite_demo/main.cpp`)

```cpp
#include "enki/widgets/svg_morph.hpp"
#include "enki/animation/animation_controller.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/state/state.hpp"

using namespace enki;

class MorphDemoState : public State {
private:
    AnimationController morph_ctrl_{std::chrono::milliseconds(1200)};

public:
    void initState() override {
        State::initState();

        // Loop forward and backward (Ping-Pong)
        morph_ctrl_.setPingPong(true);
        morph_ctrl_.setRepeats(true);
        morph_ctrl_.addListener([this] {
            setState([] {}); // Re-render when morph progress changes
        });
        morph_ctrl_.forward();
    }

    void dispose() override {
        morph_ctrl_.dispose();
        State::dispose();
    }

    WidgetPtr build(BuildContext&) override {
        float t = morph_ctrl_.value();

        return row({
            .justify_content = Justify::Center,
            .gap = StyleValue::point(40.0f),
            .children = {
                // 1. Menu <-> Close
                column({
                    .align_items = Align::Center,
                    .gap = StyleValue::point(8.0f),
                    .children = {
                        svgMorph({
                            .from_path = SvgMorphPaths::hamburger,
                            .to_path = SvgMorphPaths::close,
                            .progress = t,
                            .color = 0xFF38BDF8,
                            .stroke_width = 3.0f,
                            .width = 48.0f,
                            .height = 48.0f,
                        }),
                        text("Menu ⟷ Close", { .color = 0xFFFFFFFF, .font_size = 13.0f }),
                    }
                }),

                // 2. Play <-> Pause
                column({
                    .align_items = Align::Center,
                    .gap = StyleValue::point(8.0f),
                    .children = {
                        svgMorph({
                            .from_path = SvgMorphPaths::play,
                            .to_path = SvgMorphPaths::pause,
                            .progress = t,
                            .color = 0xFF34D399,
                            .stroke_width = 3.0f,
                            .width = 48.0f,
                            .height = 48.0f,
                        }),
                        text("Play ⟷ Pause", { .color = 0xFFFFFFFF, .font_size = 13.0f }),
                    }
                }),

                // 3. Arrow <-> Checkmark
                column({
                    .align_items = Align::Center,
                    .gap = StyleValue::point(8.0f),
                    .children = {
                        svgMorph({
                            .from_path = SvgMorphPaths::arrow_right,
                            .to_path = SvgMorphPaths::checkmark,
                            .progress = t,
                            .color = 0xFFA855F7,
                            .stroke_width = 3.0f,
                            .width = 48.0f,
                            .height = 48.0f,
                        }),
                        text("Arrow ⟷ Check", { .color = 0xFFFFFFFF, .font_size = 13.0f }),
                    }
                })
            }
        });
    }
};
```

### Example 2: Morphing Custom Vector Shapes from SVG String

```cpp
#include "enki/widgets/svg_morph.hpp"

using namespace enki;

// Triangle to Hexagon
static constexpr std::string_view kTriangle = "M 50 15 L 85 85 L 15 85 Z";
static constexpr std::string_view kHexagon  = "M 50 10 L 85 30 L 85 70 L 50 90 L 15 70 L 15 30 Z";

WidgetPtr buildCustomMorph(float progress) {
    return svgMorph({
        .from_path = kTriangle,
        .to_path = kHexagon,
        .progress = progress,
        .color = 0xFFF59E0B, // Amber
        .stroke_width = 4.0f,
        .is_stroke = true,
        .width = 64.0f,
        .height = 64.0f,
    });
}
```
