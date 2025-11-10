# Snackbar

> A non-blocking in-window toast notification overlay widget supporting 6-way viewport placements, animated countdown progress bars, pause-on-hover timers, semantic alerts, and interactive undo actions.

- **Header File**: `#include "enki/widgets/snackbar.hpp"`
- **C++ Class**: `enki::SnackbarWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::Snackbar` (converts implicitly to `WidgetPtr`)
- **Options Struct**: `enki::SnackbarOptions`
- **Action Descriptor**: `enki::SnackbarAction`
- **Controller**: `enki::SnackbarController`
- **Enums**: `enki::SnackbarType`, `enki::SnackbarPlacement`

---

## Overview

`Snackbar` presents non-blocking transient alerts (toasts) above application content. It wraps the page `body` inside an overlay stack and positions the toast across **6 viewport locations** (e.g. `BottomRight` for desktop IDEs, `TopCenter` for banners, `BottomCenter` for mobile). It includes an animated countdown progress bar along the bottom edge, automatically pauses when hovered by the cursor, and supports interactive action buttons (such as "UNDO").

---

## C++ API Definition

### Enums & Action Descriptor
```cpp
namespace enki {

enum class SnackbarType {
    Standard,   ///< Regular dark slate card
    Success,    ///< Positive confirmation with emerald accents
    Error,      ///< Failure notification with crimson accents
    Warning,    ///< Cautionary alert with amber accents
    Info,       ///< Informational notice with sky accents
    Loading     ///< Async operation with animated spinner
};

enum class SnackbarPlacement {
    BottomCenter,   ///< Centered along bottom edge (Standard)
    BottomRight,    ///< Bottom-right corner (Desktop default)
    BottomLeft,     ///< Bottom-left corner
    TopCenter,      ///< Centered along top edge (Alert banner)
    TopRight,       ///< Top-right corner (Toast notification)
    TopLeft         ///< Top-left corner
};

struct SnackbarAction {
    std::string           label     = "";
    bool                  is_danger = false;
    std::function<void()> on_click  = nullptr;

    SnackbarAction(std::string lbl, std::function<void()> cb = nullptr, bool danger = false);
};

} // namespace enki
```

### Controller (`SnackbarController`)
```cpp
namespace enki {

class SnackbarController {
public:
    void show(const SnackbarOptions& opts);
    void hide();
    [[nodiscard]] bool isOpen() const;

    // Semantic Convenience Helpers
    void showSuccess(std::string msg, std::string title = "Success",
                     std::optional<SnackbarAction> action = std::nullopt,
                     SnackbarPlacement placement = SnackbarPlacement::BottomRight);

    void showError(std::string msg, std::string title = "Error Occurred",
                   std::optional<SnackbarAction> action = std::nullopt,
                   SnackbarPlacement placement = SnackbarPlacement::BottomRight);

    void showWarning(std::string msg, std::string title = "Warning",
                     std::optional<SnackbarAction> action = std::nullopt,
                     SnackbarPlacement placement = SnackbarPlacement::BottomRight);

    void showInfo(std::string msg, std::string title = "Information",
                  std::optional<SnackbarAction> action = std::nullopt,
                  SnackbarPlacement placement = SnackbarPlacement::BottomRight);
};

} // namespace enki
```

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct Snackbar {
    Key                                 key             = Key::none();
    WidgetPtr                           body            = nullptr; ///< Application page content
    std::shared_ptr<SnackbarController> controller      = nullptr;
    SnackbarOptions                     initial_options = {};

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `body` | `WidgetPtr` | `nullptr` | Application page content wrapped by the toast layer. |
| `controller` | `shared_ptr<SnackbarController>`| `nullptr`| Handle for dispatching notifications from anywhere in the app. |
| `options.placement` | `SnackbarPlacement` | `BottomCenter`| Viewport anchor location (e.g. `BottomRight`, `TopRight`). |
| `options.duration_ms` | `int` | `4000` | Auto-dismiss countdown in milliseconds (0 = persistent). |
| `options.show_progress_bar`| `bool`| `true` | Renders animated countdown bar along bottom card edge. |
| `options.pause_on_hover`| `bool` | `true` | Freezes the countdown timer while mouse cursor is over toast. |
| `options.action` | `optional<SnackbarAction>`| `nullopt` | Interactive action button (e.g. "Undo", "Retry"). |

---

## Code Examples (From `widgets_demo/snackbar_demo/main.cpp`)

### 1. Simple Semantic Success Toast
```cpp
#include "enki/widgets/snackbar.hpp"
#include "enki/widgets/button.hpp"

using namespace enki;

class MyPageState : public State {
    std::shared_ptr<SnackbarController> snackbar_ = std::make_shared<SnackbarController>();

public:
    WidgetPtr build(BuildContext& ctx) override {
        auto pageContent = button(text("Publish Article"), [this]() {
            snackbar_->showSuccess(
                "Article published to 3 endpoints.",
                "Published!",
                SnackbarAction("VIEW", []{ /* Navigate */ }),
                SnackbarPlacement::BottomRight
            );
        });

        return Snackbar {
            .body = pageContent,
            .controller = snackbar_
        };
    }
};
```

### 2. Custom Error Toast with Undo Action
```cpp
SnackbarOptions opts;
opts.type = SnackbarType::Error;
opts.icon = "❌";
opts.title = "File Deleted";
opts.message = "archive_2026.tar.gz was moved to trash.";
opts.placement = SnackbarPlacement::BottomRight;
opts.duration_ms = 6000;
opts.action = SnackbarAction("UNDO", []{
    std::cout << "Undo file deletion clicked!\n";
});

snackbarController->show(opts);
```

---

## See Also
- [**Dialog**](./dialog.md) — Modal confirmation prompts requiring explicit dismissal.
- [**BottomSheet**](./bottom_sheet.md) — Multi-detent slide-up bottom panel.
- [**Tooltip**](../NativePopups/tooltip.md) — Contextual hover tooltips.
