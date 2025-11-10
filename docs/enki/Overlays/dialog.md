# Dialog

> An in-window modal overlay dialog widget featuring scale-and-fade entrance animations, semi-transparent backdrop scrims, Escape-key dismissal, structured action buttons, and semantic presets.

- **Header File**: `#include "enki/widgets/dialog.hpp"`
- **C++ Class**: `enki::DialogWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::Dialog` (converts implicitly to `WidgetPtr`)
- **Options Struct**: `enki::DialogOptions`
- **Action Descriptor**: `enki::DialogAction`
- **Controller**: `enki::DialogController`
- **Enums**: `enki::DialogType`, `enki::DialogAnimation`

---

## Overview

`Dialog` presents a critical modal prompt centered within the application window. It wraps the main page content (`child`), layering an animated dimmed backdrop scrim (`0x99000000`) and a centered card (`dialog_content`). The dialog captures keyboard focus, supports dismissing on backdrop tap or the `Escape` key, and animates smoothly using internal `AnimationController` and `Ticker` systems.

---

## C++ API Definition

### Enums & Actions
```cpp
namespace enki {

enum class DialogType {
    Standard,   ///< Regular neutral modal card
    Info,       ///< Informational notification with sky/blue accents
    Success,    ///< Positive confirmation with emerald/green accents
    Warning,    ///< Precautionary notice with amber/yellow accents
    Danger      ///< Destructive action with crimson/red accents
};

enum class DialogAnimation {
    ScaleAndFade,   ///< Scale from 0.88 -> 1.0 with opacity fade (Default)
    SlideAndFade,   ///< Slide down from top with opacity fade
    FadeOnly        ///< Clean opacity fade
};

struct DialogAction {
    static DialogAction primary(std::string label, std::function<void()> cb = nullptr);
    static DialogAction cancel(std::string label = "Cancel", std::function<void()> cb = nullptr);
    static DialogAction danger(std::string label, std::function<void()> cb = nullptr);
};

} // namespace enki
```

### Controller (`DialogController`)
```cpp
namespace enki {

class DialogController {
public:
    void show();
    void hide();
    void toggle();
    [[nodiscard]] bool isOpen() const;
};

} // namespace enki
```

### Configuration Options (`DialogOptions`)
```cpp
namespace enki {

struct DialogOptions {
    DialogType                type                = DialogType::Standard;
    DialogAnimation           animation           = DialogAnimation::ScaleAndFade;

    float                     width               = 480.0f;
    float                     max_height          = 600.0f;
    float                     border_radius       = 14.0f;

    bool                      barrier_dismissible = true;  ///< Tap scrim to dismiss
    bool                      escape_to_close     = true;  ///< Escape key to dismiss
    bool                      show_close_button   = true;  ///< Top-right ✕ button

    std::string               icon                = "";    ///< Leading emoji/icon (e.g. ⚠️, 🗑️, ✅)
    std::string               title               = "";
    std::string               subtitle            = "";

    std::vector<DialogAction> actions;

    Color                     background_color    = 0xFF1E293B; // Slate 800
    Color                     border_color        = 0xFF334155;
    Color                     overlay_color       = 0x99000000; // Scrim backdrop
};

} // namespace enki
```

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct Dialog {
    Key                               key            = Key::none();
    WidgetPtr                         dialog_content = nullptr; ///< Custom body inside dialog
    WidgetPtr                         child          = nullptr; ///< Application page body
    bool                              initial_open   = false;
    DialogOptions                     options;
    std::shared_ptr<DialogController> controller     = nullptr;

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `child` | `WidgetPtr` | `nullptr` | Application page content wrapped by the modal layer. |
| `dialog_content` | `WidgetPtr` | `nullptr` | Optional custom widget layout rendered inside the card. |
| `controller` | `shared_ptr<DialogController>`| `nullptr`| Controller to show or hide the dialog programmatically. |
| `options.type` | `DialogType` | `Standard` | Semantic preset (tints headers and accents). |
| `options.animation` | `DialogAnimation` | `ScaleAndFade`| Entrance animation curve. |
| `options.actions` | `std::vector<DialogAction>`| `{}` | Action buttons displayed along the bottom. |
| `options.barrier_dismissible`| `bool` | `true` | Allows dismissing when clicking outside the modal. |
| `options.escape_to_close`| `bool` | `true` | Allows dismissing by pressing the `Escape` key. |

---

## Code Examples (From `widgets_demo/dialog_demo/main.cpp`)

### 1. Destructive Confirmation Modal Dialog
```cpp
#include "enki/widgets/dialog.hpp"
#include "enki/widgets/button.hpp"

using namespace enki;

class ClusterViewState : public State {
    std::shared_ptr<DialogController> dialog_ = std::make_shared<DialogController>();

public:
    WidgetPtr build(BuildContext& ctx) override {
        auto mainView = button(text("🗑 Delete Cluster"), [this]() {
            dialog_->show();
        });

        return Dialog {
            .child = mainView,
            .controller = dialog_,
            .options = {
                .type = DialogType::Danger,
                .icon = "⚠️",
                .title = "Delete Kubernetes Cluster?",
                .subtitle = "This action is permanent and cannot be rolled back.",
                .actions = {
                    DialogAction::cancel("Keep Cluster"),
                    DialogAction::danger("Permanently Delete", [this]() {
                        std::cout << "Cluster deleted.\n";
                        dialog_->hide();
                    })
                }
            }
        };
    }
};
```

---

## See Also
- [**BottomSheet**](./bottom_sheet.md) — Slide-up bottom sheet with multi-detent snapping.
- [**Snackbar**](./snackbar.md) — Non-modal toast notification alerts.
- [**Popup**](../NativePopups/popup.md) — OS-level multi-window floating surfaces.
