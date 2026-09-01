# Enki Feedback & Status Indicators Suite

> Comprehensive system status indicators, determinate and indeterminate progress bars, circular progress rings, procedural spinners, notification centers, busy overlays, content-aware skeletons, tactile ink ripples, live beacon pulses, and animated count badges.

The **Feedback** category provides graphical indicators and communication channels that keep users informed about application activities, long-running asynchronous tasks, system health, and background processes. Enki renders these indicators directly using hardware-accelerated Skia shaders, supporting smooth 600+ FPS rotations, multi-stop gradients, neon glow effects, and custom procedural SkSL shader injection.

---

## Widget Catalog (Feedback)

### Core Feedback & Progress Primitives (Section 8)
| # | Widget | Primary Type / Helper | Header | Description |
|---|---|---|---|---|
| 1 | [**ProgressBar**](./progress_bar.md) | `struct ProgressBar`, `class ProgressBarWidget` | `<enki/widgets/progress_bar.hpp>` | Linear progress bar with determinate/indeterminate modes, gradients, and glow. |
| 2 | [**ProgressRing**](./progress_ring.md) | `struct ProgressRing`, `class ProgressRingWidget` | `<enki/widgets/progress_ring.hpp>` | Circular arc progress ring with sweep gradients, round caps, and center child slot. |
| 3 | [**Spinner**](./spinner.md) | `struct Spinner`, `enum class SpinnerStyle` | `<enki/widgets/spinner.hpp>` | Multi-style loading spinner (Spokes, OrbitDots, DualArc, CustomShader). |
| 4 | [**Notification**](./notification.md) | `struct NotificationOverlay`, `NotificationBell` | `<enki/widgets/notification.hpp>` | Multi-channel notification system with floating push toasts and slide-out feed drawer. |
| 5 | [**LoadingOverlay**](./loading_overlay.md) | `struct LoadingOverlay`, `LoadingOverlayController` | `<enki/widgets/loading_overlay.hpp>` | Scoped or full-window busy overlay with progress tracking and cancel callbacks. |

### Extended Feedback & Status (Section 18)
| # | Widget | Primary Type / Helper | Header | Description |
|---|---|---|---|---|
| 6 | [**Skeleton**](./skeleton.md) | `struct Skeleton`, `skeletonRect(...)` | `<enki/widgets/feedback_status.hpp>` | Shimmer-animated placeholder boxes rendered while asynchronous data loads. |
| 7 | [**Ripple**](./ripple.md) | `struct Ripple`, `ripple(...)` | `<enki/widgets/feedback_status.hpp>` | Material ink-ripple overlay expanding outward from a mouse click or touch tap point. |
| 8 | [**Pulse**](./pulse.md) | `struct Pulse`, `pulse(...)` | `<enki/widgets/feedback_status.hpp>` | Concentric looping radar ring animation radiating outward to signal live statuses. |
| 9 | [**CountBadge**](./count_badge.md) | `struct CountBadge`, `countBadge(...)` | `<enki/widgets/feedback_status.hpp>` | Animated numeric notification badge with spring pop transitions and overflow ("99+"). |

---

## Progress Indicators: Determinate vs Indeterminate

Enki distinguishes between operational modes across progress widgets:

```
┌─────────────────────────────────────────────────────────────┐
│ 1. Determinate Progress (Known total quantity / length)     │
│    Value: 0.0f ───────────────────► 1.0f (0% to 100%)       │
│    [████████████████████░░░░░░░░░░░░░░░░] 65% Complete      │
├─────────────────────────────────────────────────────────────┤
│ 2. Indeterminate Progress (Unknown duration / continuous)   │
│    Value: N/A (Continuous hardware animation sweep)         │
│    [░░░░░░░░░░░░░██████████░░░░░░░░░░░░░] Working...        │
└─────────────────────────────────────────────────────────────┘
```

- **Determinate Mode** (`indeterminate = false`): Explicitly updates as file bytes are transferred, records processed, or batches computed.
- **Indeterminate Mode** (`indeterminate = true`): Automatically animates a continuous smooth sweep or glow shimmer across the track without requiring manual state increments.

---

## Quick Example (Combined Status Dashboard with Extended Feedback)

```cpp
#include "enki/widgets/feedback_status.hpp"
#include "enki/widgets/progress_bar.hpp"
#include "enki/widgets/progress_ring.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildStatusPanel(float progress, int unreadCount, bool isConnected) {
    return row({
        .align_items = Align::Center,
        .gap = 20_px,
        .children = {
            // 1. Live Radar Beacon (Pulse)
            pulse({
                .color = isConnected ? 0xFF10B981 : 0xFFEF4444,
                .ring_count = 2,
                .max_radius = 16.0f,
                .dot_radius = 5.0f
            }),

            // 2. Linear Progress Bar
            ProgressBar {
                .value = progress,
                .height = 10.0f,
                .gradient_colors = {0xFF38BDF8, 0xFF818CF8}
            },

            // 3. Anchored Count Badge over an interactive ripple card
            countBadge({
                .child = ripple({
                    .child = container({
                        .color = 0xFF1E293B,
                        .border_radius = BorderRadius::circular(8.0f),
                        .padding = EdgeInsets::symmetric(8.0f, 14.0f),
                        .child = text("Inbox", { .color = 0xFFFFFFFF })
                    }),
                    .on_tap = [] { std::cout << "Opened inbox!\n"; }
                }),
                .count = unreadCount,
                .max_count = 99
            })
        }
    });
}
```
