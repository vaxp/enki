# Container

> A comprehensive visual decoration and layout box combining Skia background styling, runtime SkSL shader injection, vector SVG rendering, and 9-slice borders with full Anu Flexbox geometry and constraints.

- **Header File**: `#include "enki/widgets/container.hpp"`
- **C++ Class**: `enki::ContainerWidget`
- **Declarative Struct**: `enki::Container` (converts implicitly to `WidgetPtr`)
- **Props Type**: `enki::ContainerProps` (alias for `enki::Container`)
- **Render Object**: `enki::RenderDecoratedBox`
- **Underlying Engine**: Skia 2D rendering + Anu Layout Engine + Skia SkSL Runtime Effects

---

## Overview

`Container` is the most versatile single-child widget in Enki. It seamlessly merges visual rendering (`BoxDecoration`) with declarative Flexbox layout (`FlexboxStyle`).

In addition to standard background fills, linear/radial gradients, multi-drop shadows, rounded corners, and borders, `Container` provides two first-class GPU-accelerated graphics capabilities:
1. **SkSL Shader Injection**: Inject custom GLSL/SkSL fragment shaders directly into `background_shader` or `border_shader`. The engine automatically detects the `uniform float time;` uniform to drive smooth 60 FPS animations via internal frame tickers.
2. **Vector SVG & 9-Slice Injection**: Render resolution-independent vector art directly into `background_svg` or `border_svg` using inline XML, path strings, or asset paths, with full support for 9-slice corner-preserving scaling (`SvgSlice`).

---

## C++ API Definition

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct Container {
    // ── Visual Decoration ──────────────────────────────────────
    std::optional<Color>          color;
    std::optional<GradientConfig> gradient;
    std::optional<BorderRadius>   border_radius;
    std::optional<Border>         border;
    std::vector<BoxShadow>        box_shadow;
    std::optional<BoxShape>       shape;
    std::optional<bool>           clip_content;

    // ── SkSL Shader Injection ──────────────────────────────────
    std::string                   background_shader = "";
    std::string                   border_shader     = "";

    // ── Vector SVG & 9-Slice Injection ─────────────────────────
    std::string                   background_svg    = "";
    std::string                   border_svg        = "";
    SvgFit                        svg_fit           = SvgFit::Stretch;
    std::optional<SvgSlice>       svg_slice         = std::nullopt;

    // ── Child Alignment ────────────────────────────────────────
    std::optional<Alignment>      align;

    // ── Dimensions & Constraints ───────────────────────────────
    std::optional<StyleValue>     width;
    std::optional<StyleValue>     height;
    std::optional<StyleValue>     min_width;
    std::optional<StyleValue>     min_height;
    std::optional<StyleValue>     max_width;
    std::optional<StyleValue>     max_height;
    std::optional<float>          aspect_ratio;

    // ── Insets ─────────────────────────────────────────────────
    std::optional<StyleInsets>    padding;
    std::optional<StyleInsets>    margin;
    std::optional<StyleInsets>    position;

    // ── Flexbox Item Properties ────────────────────────────────
    std::optional<float>          flex;
    std::optional<float>          flex_grow;
    std::optional<float>          flex_shrink;
    std::optional<StyleValue>     flex_basis;
    std::optional<Align>          align_self;
    std::optional<PositionType>   position_type;

    WidgetPtr                     child = nullptr;
    Key                           key   = Key::none();

    operator WidgetPtr() const;
};

using ContainerProps = Container;

} // namespace enki
```

### Factory Helper Functions
```cpp
namespace enki {

inline std::shared_ptr<ContainerWidget> container(ContainerProps props = {});
inline std::shared_ptr<ContainerWidget> sizedBox(float width, float height, WidgetPtr child = nullptr);
inline std::shared_ptr<ContainerWidget> paddingBox(EdgeInsets insets, WidgetPtr child);
inline std::shared_ptr<ContainerWidget> centerBox(WidgetPtr child);

} // namespace enki
```

---

## Properties Reference

### Visual Decoration
| Property | Type | Default | Description |
|---|---|---|---|
| `color` | `std::optional<Color>` | `Colors::Transparent` | Background 32-bit ARGB color (e.g. `0xFF1E293B`). |
| `gradient` | `std::optional<GradientConfig>` | `std::nullopt` | Linear or radial background gradient. |
| `border_radius` | `std::optional<BorderRadius>` | `BorderRadius::zero()` | Corner radius (`circular(r)`, `all(r)`, `only(...)`). |
| `border` | `std::optional<Border>` | `std::nullopt` | Stroke outline (`Border(Color, width)`). |
| `box_shadow` | `std::vector<BoxShadow>` | `{}` | List of drop or glow shadows (`BoxShadow::standard()`, `BoxShadow::glow()`). |
| `shape` | `std::optional<BoxShape>` | `BoxShape::Rectangle` | Box shape (`BoxShape::Rectangle` or `BoxShape::Circle`). |
| `clip_content` | `std::optional<bool>` | `false` | Clips the child widget hierarchy to the container's rounded bounds. |

### SkSL Shader Injection
| Property | Type | Default | Description |
|---|---|---|---|
| `background_shader` | `std::string` | `""` | SkSL fragment shader code evaluated dynamically across the container background fill. |
| `border_shader` | `std::string` | `""` | SkSL fragment shader code evaluated dynamically across the container border stroke. |

> [!NOTE]
> When `border_shader` is supplied without an explicit `.border`, Enki automatically creates a default 1.0px transparent border so the stroke allocation and layout constraints are seamlessly handled.

### Vector SVG & 9-Slice Injection
| Property | Type | Default | Description |
|---|---|---|---|
| `background_svg` | `std::string` | `""` | SVG XML markup, SVG path string (`"M ..."`), or asset file path rendered into the background. |
| `border_svg` | `std::string` | `""` | SVG XML markup, SVG path string, or asset file path rendered into the container's border. |
| `svg_fit` | `SvgFit` | `SvgFit::Stretch` | Fit strategy: `SvgFit::Stretch`, `SvgFit::Contain`, or `SvgFit::Cover`. |
| `svg_slice` | `std::optional<SvgSlice>` | `std::nullopt` | 9-slice insets for corner-preserving scaling (`SvgSlice::all(v)`, `SvgSlice::symmetric(v, h)`). |

### Layout & Sizing
| Property | Type | Default | Description |
|---|---|---|---|
| `align` | `std::optional<Alignment>` | `std::nullopt` | Anchors the child inside the container (`Alignment::Center`, `Alignment::TopLeft`, etc.). |
| `width` | `std::optional<StyleValue>` | `auto_val` | Explicit width (`200_px`, `50_pct`). |
| `height` | `std::optional<StyleValue>` | `auto_val` | Explicit height (`100_px`, `100_pct`). |
| `min_width` / `max_width` | `std::optional<StyleValue>` | `undefined_val` | Minimum and maximum width boundaries. |
| `min_height` / `max_height` | `std::optional<StyleValue>` | `undefined_val` | Minimum and maximum height boundaries. |
| `aspect_ratio` | `std::optional<float>` | `std::nullopt` | Enforces width-to-height proportion (e.g. `16.0f / 9.0f`). |
| `padding` | `std::optional<StyleInsets>` | `0` | Inner padding between container border and child. |
| `margin` | `std::optional<StyleInsets>` | `0` | Outer margin around the container. |
| `child` | `WidgetPtr` | `nullptr` | The nested child widget. |

---

## SkSL Shader Architecture

When you pass an SkSL fragment code string to `background_shader` or `border_shader`, Enki's `RenderDecoratedBox` compiles it on the GPU using `SkRuntimeEffect`.

### Built-in Uniforms
Enki inspects your SkSL code and automatically injects standard uniforms:
1. `uniform float time;`
   - Real-time elapsed execution time in seconds.
   - **Zero Overhead**: If your shader declares `uniform float time;`, Enki's render object automatically attaches an internal `Ticker` to re-render at the display refresh rate. If `time` is not declared, no ticker is started and the shader evaluates statically with zero CPU wakeups.
2. `uniform vec2 resolution;`
   - Dynamically populated with the rendered container bounds (`{width, height}` in logical pixels).

### SkSL Entry Signature
Your shader must define the standard entry point:
```glsl
vec4 main(vec2 fragCoord) {
    vec2 uv = fragCoord / resolution;
    // ... compute color
    return vec4(r, g, b, a);
}
```

---

## Vector SVG & 9-Slice Architecture

Enki's vector engine parses and renders resolution-independent graphics with zero pixelation at arbitrary DPI scaling factors.

### Input Formats
The `background_svg` and `border_svg` properties accept:
- **Raw SVG XML**: Direct `<svg viewBox="...">...</svg>` strings.
- **SVG Path Strings**: Standard SVG path strings, e.g. `"M 0 15 L 15 0 H 85 L 100 15 ... Z"`.
- **File Asset Paths**: Relative or absolute paths to SVG files, e.g. `"assets/frames/ornate_border.svg"`.

### Scaling Modes (`SvgFit`)
```cpp
enum class SvgFit {
    Stretch,  // Stretches vector graphic to fill target container bounds exactly.
    Contain,  // Scales uniformly preserving aspect ratio to fit inside bounds.
    Cover,    // Scales uniformly to cover the bounds completely (may crop).
};
```

### 9-Slice Border Scaling (`SvgSlice`)
For ornate frames, game UI borders, and fantasy cards, standard vector scaling would stretch corner filigrees and rivets. `SvgSlice` preserves the 4 corners at their native size while stretching only the top, bottom, left, right edges and center.

```cpp
struct SvgSlice {
    float top    = 0.0f;
    float right  = 0.0f;
    float bottom = 0.0f;
    float left   = 0.0f;

    static constexpr SvgSlice all(float v);
    static constexpr SvgSlice symmetric(float vertical, float horizontal);
};
```

### Hybrid Vector + SkSL Shader
You can combine `border_svg` (or `background_svg`) with `border_shader` (or `background_shader`). When both are present, Enki renders the vector geometry masked and textured by the real-time SkSL shader!

---

## Code Examples

### 1. Modern Glassmorphism Card
```cpp
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildMetricCard() {
    return container({
        .color = 0xFF161D2F,
        .border_radius = BorderRadius::circular(14.0f),
        .border = Border(0x4038BDF8, 1.5f),
        .box_shadow = {
            BoxShadow(0x30000000, {0.0f, 6.0f}, 12.0f, 0.0f),
        },
        .padding = StyleInsets::all(20_px),
        .width = 280_px,
        .child = text("Active Users: 12,480", {
            .color = 0xFFF8FAFC,
            .font_size = 15.0f,
            .font_weight = FontWeight::SemiBold,
        })
    });
}
```

### 2. Live Animated SkSL Plasma Card
*(From `widgets_demo/container_demo/main.cpp`)*
```cpp
const std::string plasma_shader = R"(
    uniform float time;
    uniform vec2 resolution;

    vec4 main(vec2 fragCoord) {
        vec2 uv = fragCoord / resolution;
        float t = time * 0.8;
        float v = sin(uv.x * 6.0 + t) + sin(uv.y * 6.0 + t) + sin((uv.x + uv.y) * 6.0 + t);
        v = v * 0.5;
        vec3 col = 0.5 + 0.5 * cos(v + vec3(0.0, 2.0, 4.0));
        return vec4(col * 0.85, 1.0);
    }
)";

auto plasmaCard = container({
    .border_radius = BorderRadius::circular(16.0f),
    .border = Border(0x60FFFFFF, 1.5f),
    .box_shadow = { BoxShadow::glow(0x608B5CF6, 20.0f) },
    .background_shader = plasma_shader,
    .align = Alignment::Center,
    .width = 260_px,
    .height = 160_px,
    .padding = StyleInsets::all(16.0f),
    .child = text("Live SkSL Plasma", {
        .color = 0xFFFFFFFF,
        .font_size = 16.0f,
        .font_weight = FontWeight::Bold,
    })
});
```

### 3. Cyber Glowing SkSL Border Shader
*(From `widgets_demo/container_demo/main.cpp`)*
```cpp
const std::string cyber_border_shader = R"(
    uniform float time;
    uniform vec2 resolution;

    vec4 main(vec2 fragCoord) {
        vec2 uv = fragCoord / resolution;
        float angle = atan(uv.y - 0.5, uv.x - 0.5);
        float glow = sin(angle * 3.0 + time * 3.0) * 0.5 + 0.5;
        vec3 cyan = vec3(0.06, 0.72, 0.95);
        vec3 pink = vec3(0.93, 0.28, 0.60);
        vec3 color = mix(cyan, pink, glow);
        return vec4(color, 1.0);
    }
)";

auto cyberCard = container({
    .color = 0xE60F172A,
    .border_radius = BorderRadius::circular(16.0f),
    .border = Border(Colors::Transparent, 3.0f),
    .box_shadow = { BoxShadow::glow(0x4006B6D4, 25.0f) },
    .border_shader = cyber_border_shader,
    .align = Alignment::Center,
    .width = 260_px,
    .height = 160_px,
    .padding = StyleInsets::all(16.0f),
    .child = text("Neon Rotating Border", {
        .color = 0xFF38BDF8,
        .font_size = 16.0f,
        .font_weight = FontWeight::Bold,
    })
});
```

### 4. Direct SVG Sci-Fi HUD Border
*(From `widgets_demo/container_demo/main.cpp`)*
```cpp
const std::string scifi_hud_svg = R"(
    <svg viewBox="0 0 360 200">
        <path d="M 0 30 L 30 0 H 260 L 280 20 H 330 L 360 50 V 170 L 330 200 H 100 L 80 180 H 30 L 0 150 Z"
              fill="#161B22" stroke="#00E5FF" stroke-width="2"/>
        <path d="M 35 12 H 140" stroke="#00E5FF" stroke-width="1.5"/>
        <path d="M 348 60 V 140" stroke="#00E5FF" stroke-width="2"/>
        <polygon points="12,30 30,12 36,18 18,36" fill="#00E5FF"/>
        <polygon points="348,170 330,188 324,182 342,164" fill="#00E5FF"/>
    </svg>
)";

auto hudCard = container({
    .border = Border(Colors::Transparent, 2.0f),
    .box_shadow = { BoxShadow::glow(0x4000E5FF, 20.0f) },
    .border_svg = scifi_hud_svg,
    .align = Alignment::Center,
    .width = 360_px,
    .height = 200_px,
    .padding = StyleInsets::all(20.0f),
    .child = text("Sci-Fi Vector HUD", {
        .color = 0xFF00E5FF,
        .font_size = 18.0f,
        .font_weight = FontWeight::Bold,
    })
});
```

### 5. 9-Slice Ornate Fantasy Frame (`SvgSlice`)
*(From `widgets_demo/container_demo/main.cpp`)*
```cpp
const std::string ornate_9slice_svg = R"(
    <svg viewBox="0 0 100 100">
        <path d="M 0 24 V 8 C 0 2 2 0 8 0 H 24 C 14 4 10 10 10 18 C 10 22 14 24 24 24 Z" fill="#F59E0B" stroke="#D97706" stroke-width="1.5"/>
        <path d="M 100 24 V 8 C 100 2 98 0 92 0 H 76 C 86 4 90 10 90 18 C 90 22 86 24 76 24 Z" fill="#F59E0B" stroke="#D97706" stroke-width="1.5"/>
        <path d="M 0 76 V 92 C 0 98 2 100 8 100 H 24 C 14 96 10 90 10 82 C 10 78 14 76 24 76 Z" fill="#F59E0B" stroke="#D97706" stroke-width="1.5"/>
        <path d="M 100 76 V 92 C 100 98 98 100 92 100 H 76 C 86 96 90 90 90 82 C 90 78 86 76 76 76 Z" fill="#F59E0B" stroke="#D97706" stroke-width="1.5"/>
        <line x1="24" y1="2" x2="76" y2="2" stroke="#FBBF24" stroke-width="2"/>
        <line x1="24" y1="98" x2="76" y2="98" stroke="#FBBF24" stroke-width="2"/>
        <line x1="2" y1="24" x2="2" y2="76" stroke="#FBBF24" stroke-width="2"/>
        <line x1="98" y1="24" x2="98" y2="76" stroke="#FBBF24" stroke-width="2"/>
    </svg>
)";

auto ornateCard = container({
    .color = 0xEB1A1412,
    .border = Border(Colors::Transparent, 2.0f),
    .box_shadow = { BoxShadow::glow(0x60F59E0B, 20.0f) },
    .border_svg = ornate_9slice_svg,
    .svg_slice = SvgSlice::all(24.0f), // Preserves all 4 ornate corners exactly
    .align = Alignment::Center,
    .width = 360_px,
    .height = 200_px,
    .padding = StyleInsets::all(20.0f),
    .child = text("9-Slice Vector Frame", {
        .color = 0xFFFCD34D,
        .font_size = 18.0f,
        .font_weight = FontWeight::Bold,
    })
});
```

### 6. Hybrid Combo: Vector Shape + Live SkSL Shader
*(From `widgets_demo/container_demo/main.cpp`)*
```cpp
const std::string hexagon_svg_path = "M 40 0 L 180 0 L 220 75 L 180 150 L 40 150 L 0 75 Z";

auto hybridCard = container({
    .color = 0xE60D1117,
    .border = Border(Colors::Transparent, 3.0f),
    .box_shadow = { BoxShadow::glow(0x60EC4899, 25.0f) },
    .border_shader = cyber_border_shader, // SkSL shader fills the stroke
    .border_svg = hexagon_svg_path,       // Vector path defines the geometry
    .align = Alignment::Center,
    .width = 220_px,
    .height = 200_px,
    .child = text("Hexagon + Neon Shader", {
        .color = 0xFFFFFFFF,
        .font_size = 15.0f,
        .font_weight = FontWeight::Bold,
    })
});
```

---

## See Also
- [**WindowFrame**](../Desktop/window_frame.md) — Window decorator utilizing Container's SkSL shaders for custom titlebars & glow outlines.
- [**SvgMorph**](../Animation%20&%20Motion/svg_morph.md) — Dynamic path morphing animations.
- [**SizedBox**](./sized_box.md) — Lightweight dimension-only box.
- [**Padding**](./padding.md) — Dedicated padding box wrapper.
- [**Align**](./align.md) & [**Center**](./center.md) — Alignment within containers.
- [**ConstrainedBox**](./constrained_box.md) — Min/max size bounds.
