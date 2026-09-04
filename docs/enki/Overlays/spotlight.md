# Spotlight

> An in-window interactive feature tour & spotlight overlay widget featuring inverse-cutout focus masks, pulsing beacon halos, auto-positioned popover cards, multi-step tour controllers, and pass-through target interactivity.

- **Header File**: `#include "enki/widgets/spotlight.hpp"`
- **C++ Class**: `enki::SpotlightWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::Spotlight` (converts implicitly to `WidgetPtr`)
- **Options Struct**: `enki::SpotlightOptions`
- **Tour Step Descriptor**: `enki::SpotlightStep`
- **Controller**: `enki::SpotlightTourController`
- **Enums**: `enki::SpotlightShape`, `enki::SpotlightPlacement`
- **Helper Function**: `enki::spotlight(const SpotlightProps&)`

---

## Overview

`Spotlight` creates an onboarding feature tour and interactive walkthrough experience (similar to Driver.js, Shepherd.js, and macOS Guided Tours). It dims the entire application viewport with an obsidian scrim (`0xCC080C14`) while punching a crisp, hardware-accelerated "cutout hole" directly over a target widget using Skia difference rendering (`drawDRRect`).

### Key Capabilities
- **Multiple Cutout Shapes**: Supports `RoundedRectangle` with corner radius, circular/oval `Circle`, or sharp `Rectangle` with custom padding.
- **Pulsing Beacon Halo**: Automatically radiates an animated glowing beacon ring around the target cutout hole to focus user attention.
- **Auto-Positioned Popover Card**:
  - Automatically calculates available viewport space to position the tour card (Top, Bottom, Left, Right) with viewport boundary clamping.
  - Displays step counter pill (e.g. `STEP 2 OF 4`), title, detailed description, step indicator dots, and Next / Back / Skip buttons.
- **Multi-Step Tour Controller**:
  - Sequence management: `start()`, `next()`, `previous()`, `skip()`, `finish()`, `goToStep()`.
  - State queries: `getCurrentStepIndex()`, `getTotalSteps()`, `isActive()`.
  - Dynamic runtime target rect updates via `updateTargetRect(Rect)`.
- **Target Interaction**: Configurable `allow_target_click` to let users interact directly with the focused widget while the rest of the window remains blocked.

---

## C++ API Definition

### Data Structures & Enums

```cpp
namespace enki {

enum class SpotlightShape {
    RoundedRectangle, ///< Rounded rect with corner radius (Default)
    Circle,           ///< Perfect circle or oval centered on target
    Rectangle         ///< Sharp corner rectangle
};

enum class SpotlightPlacement {
    Auto,             ///< Auto-detect based on available space
    Top,              ///< Position above target
    Bottom,           ///< Position below target
    Left,             ///< Position to left of target
    Right             ///< Position to right of target
};

struct SpotlightStep {
    std::string id = "";
    std::string title = "";
    std::string description = "";

    Rect target_bounds = Rect{0.0f, 0.0f, 0.0f, 0.0f};
    Key target_key = Key::none();

    SpotlightShape shape = SpotlightShape::RoundedRectangle;
    float corner_radius = 10.0f;
    EdgeInsets padding = EdgeInsets::all(8.0f);

    SpotlightPlacement placement = SpotlightPlacement::Auto;
    bool allow_target_click = true;
    bool show_pulse_ring = true;

    std::string next_button_label = "";
    std::string back_button_label = "Back";
    std::string skip_button_label = "Skip Tour";
};

struct SpotlightOptions {
    Color overlay_color       = 0xCC080C14;
    Color pulse_ring_color    = 0xFF38BDF8;
    Color card_bg_color       = 0xF80F172A;
    Color card_border_color   = 0xFF334155;
    Color title_color         = 0xFFF8FAFC;
    Color description_color   = 0xFF94A3B8;
    Color step_badge_color    = 0xFF38BDF8;

    float card_width          = 340.0f;
    float card_border_radius   = 12.0f;
    float popover_distance    = 14.0f;

    bool dismiss_on_scrim_tap = false;
    bool show_step_indicator  = true;
    bool show_skip_button     = true;

    std::function<void(size_t index, const SpotlightStep&)> on_step_change;
    std::function<void()> on_finish;
    std::function<void()> on_skip;
};

class SpotlightTourController {
public:
    void start();
    void next();
    void previous();
    void skip();
    void finish();
    void goToStep(size_t idx);

    [[nodiscard]] size_t getCurrentStepIndex() const;
    [[nodiscard]] size_t getTotalSteps() const;
    [[nodiscard]] bool isActive() const;

    void updateTargetRect(Rect rect);
    void setSteps(std::vector<SpotlightStep> steps);
};

} // namespace enki
```

---

## Declarative Usage Example

```cpp
#include "enki/widgets/spotlight.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

auto tour_ctrl = std::make_shared<SpotlightTourController>();

std::vector<SpotlightStep> steps = {
    SpotlightStep("Navigation Sidebar", "Access all workspace modules here.", Rect{20, 60, 200, 400}),
    SpotlightStep("Command Search", "Press Ctrl+K anytime to launch commands.", Rect{240, 20, 320, 42}),
    SpotlightStep("Deploy Button", "One-click deployment to global edge.", Rect{600, 20, 140, 42})
};

WidgetPtr app_view = Spotlight {
    .body = container({
        .child = text("Main Dashboard View")
    }),
    .steps = steps,
    .options = {
        .card_width = 340.0f,
        .on_finish = [] { /* tour complete */ }
    },
    .controller = tour_ctrl
};
```

---

## Updating Roadmaps & Summary Table

In `WIDGETS_ROADMAP_v0.2.0.md`, section 19:
- [x] **Spotlight** — Full-screen dimmed overlay with a highlighted "spotlight" region around a widget
