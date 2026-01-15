# Hero & Shared Element Transitions

> Seamless visual element morphing and flight transitions between screens and routes using automatic global coordinate tracking.

- **Header File**: `#include "enki/widgets/hero.hpp"`
- **Primary C++ Classes**:
  - `enki::HeroWidget` (Single-child render object widget associating a unique string `tag`)
  - `enki::HeroProps` & `enki::Hero` (Declarative widget struct supporting C++20 designated initializers)
  - `enki::HeroRegistry` (Singleton registry mapping tags to render objects and global bounding boxes)
  - `enki::RenderHero` (Render object managing visibility and placeholder state during flight)
  - `enki::HeroFlightWidget` & `enki::heroFlight()` (Floating transition widget interpolating between source and destination rectangles)

---

## Overview & Architecture

When navigating between two screens in a modern application (for example, clicking a thumbnail in a list to open a detailed full-screen view), abruptly switching views creates cognitive discontinuity.

A **Hero Transition** (shared element transition) smoothly animates the shared element from its position and size on the source screen directly into its position and size on the destination screen:

1. **Tag Association**: Both the source widget and destination widget wrap their content in a `Hero` widget with the same string `tag` (e.g., `"product_image_42"`).
2. **Global Coordinate Tracking**: During the layout and paint phase, `RenderHero` queries its absolute screen-space coordinates via `RenderObject::globalBounds()` and updates the centralized `HeroRegistry`.
3. **Flight Shuttle Creation**: When a transition triggers, a `HeroFlightWidget` is mounted on the top overlay layer. It interpolates its bounding box from `start_rect` to `end_rect` over an animation curve (or physics spring).
4. **Placeholder Visibility**: While the flight is in progress, the destination and source widgets can be masked (`is_placeholder = true`) so the user only perceives a single smooth flying object.

---

## C++ API Reference

### 1. `Hero` Declarative Struct
```cpp
namespace enki {

struct HeroProps {
    std::string tag;            ///< Unique string identifier linking shared elements
    WidgetPtr   child = nullptr;///< Content widget to morph and fly
    Key         key   = Key::none();

    operator WidgetPtr() const;
};

struct Hero : public HeroProps {
    using HeroProps::HeroProps;
};

inline WidgetPtr hero(const HeroProps& props);

} // namespace enki
```

### 2. `HeroRegistry` Global Tracking Singleton
```cpp
namespace enki {

struct HeroRecord {
    std::string   tag;
    RenderHero*   render_object = nullptr;
    Rect          last_global_bounds;
    WidgetPtr     widget = nullptr;
};

class HeroRegistry {
public:
    static HeroRegistry& instance();

    void registerHero(const std::string& tag, RenderHero* ro, WidgetPtr w);
    void unregisterHero(const std::string& tag, RenderHero* ro);
    void updateBounds(const std::string& tag, RenderHero* ro, const Rect& bounds);

    [[nodiscard]] const HeroRecord* findHero(const std::string& tag) const;
    [[nodiscard]] bool hasHero(const std::string& tag) const;
    void clear();
};

} // namespace enki
```

### 3. `HeroFlightWidget`
Floating canvas flight layer transitioning between two absolute rectangles:

```cpp
namespace enki {

class HeroFlightWidget : public SingleChildRenderObjectWidget {
public:
    Rect  start_rect;
    Rect  end_rect;
    float progress; // Normalized [0.0, 1.0]

    HeroFlightWidget(Rect s, Rect e, float p, WidgetPtr child);
};

inline WidgetPtr heroFlight(Rect start_rect, Rect end_rect, float progress, WidgetPtr child);

} // namespace enki
```

---

## Real Code Examples

### Example 1: Defining a Shared Element in a List & Detail View

```cpp
#include "enki/widgets/hero.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/image.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

// Screen 1: Thumbnail in a Grid Item
WidgetPtr buildThumbnailCard(const std::string& item_id, const std::string& image_path) {
    return container({
        .border_radius = BorderRadius::circular(8.0f),
        .width = StyleValue::point(120.0f),
        .height = StyleValue::point(120.0f),
        .child = hero({
            .tag = "item_hero_" + item_id, // Same tag on both screens
            .child = image(image_path),
        }),
    });
}

// Screen 2: Expanded Header in Details View
WidgetPtr buildDetailsHeader(const std::string& item_id, const std::string& image_path) {
    return container({
        .width = StyleValue::percent(100.0f),
        .height = StyleValue::point(350.0f),
        .child = hero({
            .tag = "item_hero_" + item_id, // Matches Screen 1 tag
            .child = image(image_path),
        }),
    });
}
```

### Example 2: Inspecting Hero Registration in Tests (`tests/widgets/test_hero.cpp`)

```cpp
#include "enki/widgets/hero.hpp"
#include "enki/widgets/container.hpp"
#include <cassert>

using namespace enki;

void testHeroLifecycle() {
    auto& registry = HeroRegistry::instance();
    registry.clear();

    // Create a hero widget
    auto h = hero({
        .tag = "avatar_hero",
        .child = container({
            .width = StyleValue::point(80.0f),
            .height = StyleValue::point(80.0f),
        }),
    });

    assert(h != nullptr);
}
```
