# Enki In-Window Overlays Suite

> In-window overlay architecture, modal dialogs, multi-detent bottom sheets, 6-way toast snackbars, and dropdown selection menus rendered within the application's widget stack.

The **Overlays** category encompasses floating cards, modals, sheets, and notification toasts that render **within the main application window's viewport**. Rather than spawning operating-system-level subsurfaces (as done in `NativePopups`), Overlays leverage Enki's **Container-Wrapping Architecture**: they wrap the underlying page `body` inside an unclipped `100% × 100%` `Stack`, mounting semi-transparent click-catcher scrims and hardware-accelerated animated layers (`Positioned`) above the content.

---

## Architectural Architecture: In-Window Overlays vs Native Popups

```
┌─────────────────────────────────────────────────────────────┐
│                    Application Window                       │
│                                                             │
│   ┌─────────────────────────────────────────────────────┐   │
│   │ Stack (100% Width × 100% Height)                    │   │
│   │                                                     │   │
│   │   Layer 0: Positioned::fill(body_widget)            │   │
│   │            (Main page content, invariant layout)    │   │
│   │                                                     │   │
│   │   Layer 1: Scrim Backdrop (0x99000000)              │   │
│   │            (Dismiss click-catcher)                  │   │
│   │                                                     │   │
│   │   Layer 2: Positioned Overlay Element               │   │
│   │            (Dialog / BottomSheet / Snackbar / Menu) │   │
│   │                                                     │   │
│   └─────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
```

- **Seamless Compositing**: Dialogs, sheets, and toasts are rendered directly in the application's main render pass, inheriting local styles, animations, and focus rings.
- **Hardware-Accelerated Physics**: Scale-and-fade, slide-in transitions, and drag physics driven by internal `AnimationController` and `Ticker` systems.
- **Zero Window Server Overhead**: Ideal for high-frequency notifications and standard application workflows that do not require cross-window floating.

---

## Widget Catalog (Overlays)

| # | Widget | Primary Type / Helper | Header | Description |
|---|---|---|---|---|
| 1 | [**DropdownMenu**](./dropdown_menu.md) | `struct DropdownMenu`, `DropdownMenuItem` | `<enki/widgets/dropdown_menu.hpp>` | Floating selection dropdown with checkboxes, radios, badges, and auto-flip placement. |
| 2 | [**Dialog**](./dialog.md) | `struct Dialog`, `DialogOptions` | `<enki/widgets/dialog.hpp>` | Modal overlay card with scale/fade animations, escape dismissal, and action buttons. |
| 3 | [**BottomSheet**](./bottom_sheet.md) | `struct BottomSheet`, `BottomSheetDetent` | `<enki/widgets/bottom_sheet.hpp>` | Multi-detent bottom panel with drag physics, snap stages (Peek, Half, Full), and scrim. |
| 4 | [**Snackbar**](./snackbar.md) | `struct Snackbar`, `SnackbarController` | `<enki/widgets/snackbar.hpp>` | 6-way toast notification with countdown progress bar, pause-on-hover, and semantic alerts. |
| 5 | [**Spotlight**](./spotlight.md) | `struct Spotlight`, `SpotlightStep` | `<enki/widgets/spotlight.hpp>` | Full-screen dimmed overlay with inverse-cutout focus hole & step-by-step tour popover. |
| 6 | [**FloatingPanel**](./floating_panel.md) | `struct FloatingPanel`, `FloatingPanelOptions` | `<enki/widgets/floating_panel.hpp>` | Freeform draggable & 8-direction resizable floating window overlay with state toggling. |

---

## Quick Example (Unified Overlay Application Shell)

```cpp
#include "enki/app/app.hpp"
#include "enki/widgets/snackbar.hpp"
#include "enki/widgets/dialog.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/flexbox.hpp"

using namespace enki;

class DashboardShellState : public State {
    std::shared_ptr<SnackbarController> snack_ctrl = std::make_shared<SnackbarController>();
    std::shared_ptr<DialogController>   diag_ctrl  = std::make_shared<DialogController>();

public:
    WidgetPtr build(BuildContext& ctx) override {
        // Main page body
        auto pageContent = column({
            .justify_content = Justify::Center,
            .align_items = Align::Center,
            .gap = 16_px,
            .children = {
                button(text("Trigger Success Toast"), [this] {
                    snack_ctrl->showSuccess("Project deployed successfully!", "Deployment");
                }),
                button(text("Open Confirmation Dialog"), [this] {
                    diag_ctrl->show();
                }),
            }
        });

        // Wrap with Dialog, then wrap with Snackbar
        auto withDialog = Dialog {
            .child = pageContent,
            .controller = diag_ctrl,
            .options = {
                .title = "Confirm Deletion",
                .subtitle = "Are you sure you want to permanently remove this cluster?",
                .actions = {
                    DialogAction::cancel("Cancel"),
                    DialogAction::danger("Delete", [this]{ diag_ctrl->hide(); })
                }
            }
        };

        return Snackbar {
            .body = withDialog,
            .controller = snack_ctrl
        };
    }
};
```
