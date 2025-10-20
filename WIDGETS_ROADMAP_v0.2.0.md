# ENKI Engine — Widgets Roadmap v0.2.0

> **Release Target**: v0.2.0  
> **New Widgets**: 50 atomic/primitive widgets (not component compositions)  
> **Built Upon**: All ✅ widgets from v0.1.0 Roadmap

---

## 11. Layout — Extended (6 widgets)

- [ ] **IntrinsicWidth** — Sizes child to its intrinsic/natural width (useful for text-width columns)
- [ ] **IntrinsicHeight** — Sizes child to its intrinsic/natural height
- [ ] **OverflowBox** — Renders child beyond its own bounds, clipping or not
- [ ] **LimitedBox** — Applies max-width/height only when unconstrained
- [ ] **CustomMultiChildLayout** — Allows fine-grained multi-child positioning via layout delegates
- [ ] **Flow** — A high-performance, transform-based layout for animated/overlapping children

---

## 12. Paint & Visual Effects (8 widgets)

- [x] **ClipRect** — Clips child to a rectangular boundary
- [x] **ClipRRect** — Clips child to a rounded rectangle boundary
- [x] **ClipOval** — Clips child to an ellipse/circle boundary
- [x] **ClipPath** — Clips child to an arbitrary Skia path
- [x] **BackdropFilter** — Applies Skia ImageFilter (blur, color matrix) to content *behind* it
- [x] **DecoratedBox** — Paints a BoxDecoration (gradient, border, shadow, image) around/behind a child
- [x] **ShaderMask** — Applies a gradient or image shader as a mask over child paint
- [x] **ColorFiltered** — Applies a ColorFilter (e.g. grayscale, sepia, invert) to child rendering

---

## 13. Animation & Motion (7 widgets)

- [x] **AnimatedOpacity** — Smoothly interpolates opacity via a Tween<double> animation
- [x] **AnimatedContainer** — Animates changes to size, color, padding, margin automatically
- [x] **AnimatedScale** — Smooth scale transform animation driven by a target scale value
- [x] **AnimatedRotation** — Smooth rotation transform animation driven by a target angle
- [x] **AnimatedSlide** — Smooth offset translation animation driven by a target Offset
- [x] **AnimatedSwitcher** — Cross-fades / transitions between two child widget trees
- [x] **SlideTransition** — Low-level positional slide driven by an explicit Animation<Offset>

---

## 14. Scrolling — Extended (4 widgets)

- [x] **SliverList** — A sliver delegate that lazily builds list items within a CustomScrollView
- [x] **SliverGrid** — A sliver delegate that lazily builds grid items within a CustomScrollView
- [x] **SliverAppBar** — A collapsible, pinned, or floating header sliver within a CustomScrollView
- [x] **CustomScrollView** — A scroll viewport accepting an ordered list of slivers

---

## 15. Rich Input Controls (8 widgets)

- [ ] **OTPField** — One-time password input: N segmented single-character boxes with auto-focus advance
- [ ] **PinField** — Secure PIN entry: boxed digit inputs with masked display
- [ ] **TagInput** — Inline multi-tag text input: type + press Enter/comma to add removable tag chips
- [ ] **RatingBar** — Star (or custom icon) rating input widget with half-star and keyboard support
- [ ] **Knob** — Circular rotary dial input mapping angle to a numeric range (audio/studio style)
- [ ] **ToggleButton** — Single pressable button that switches between ON/OFF visual states
- [ ] **SegmentedControl** — Horizontally grouped mutually-exclusive option buttons (iOS/Material style)
- [ ] **FileDropZone** — A drag-and-drop surface that accepts file drops from the OS and reports paths

---

## 16. Media & Canvas (5 widgets)

- [ ] **VideoPlayer** — Embeds a native video surface with play/pause/seek controls
- [ ] **AudioWaveform** — Renders a real-time or static audio amplitude waveform visualization
- [ ] **SkiaCanvas** — Raw Skia canvas widget: exposes a PaintCallback for custom 2D drawing
- [ ] **Lottie** — Renders Lottie / Rive JSON animations on a Skia canvas
- [ ] **WebView** — Embeds a native browser/webview surface (Chromium Embedded / WebKitGTK)

---

## 17. Typography — Extended (3 widgets)

- [ ] **SelectableText** — Text widget with mouse-selection highlight, copy-to-clipboard support
- [ ] **Marquee** — Auto-scrolling single-line text (ticker tape) with configurable speed and direction
- [ ] **CodeBlock** — Syntax-highlighted, monospace code display with line numbers and copy button

---

## 18. Feedback & Status — Extended (4 widgets)

- [x] **Skeleton** — Shimmer placeholder boxes rendered while async data loads (content-aware shape)
- [x] **Ripple** — Material ink-ripple overlay that animates outward from a tap point
- [x] **Pulse** — Looping concentric-ring animation radiating from a center point (live indicator)
- [x] **CountBadge** — Animated numeric badge that counts up/down with spring or flip transitions

---

## 19. Overlay & Popup — Extended (3 widgets)

- [ ] **CommandPalette** — Keyboard-driven fuzzy-search command launcher overlay (Ctrl+K style)
- [ ] **Spotlight** — Full-screen dimmed overlay with a highlighted "spotlight" region around a widget
- [ ] **FloatingPanel** — Draggable, resizable floating window rendered above the main widget tree

---

## 20. Utility / Behavioral (2 widgets)

- [x] **Visibility** — Toggles child visibility (show/hide) without removing from the widget tree
- [x] **IgnorePointer** — Passes all pointer events through to widgets underneath it

---

## Summary

| Category | New Widgets | Running Total |
|---|---|---|
| 11. Layout Extended | 6 | 6 |
| 12. Paint & Visual Effects | 8 | 14 |
| 13. Animation & Motion | 7 | 21 |
| 14. Scrolling Extended | 4 | 25 |
| 15. Rich Input Controls | 8 | 33 |
| 16. Media & Canvas | 5 | 38 |
| 17. Typography Extended | 3 | 41 |
| 18. Feedback & Status Extended | 4 | 45 |
| 19. Overlay & Popup Extended | 3 | 48 |
| 20. Utility / Behavioral | 2 | **50** |

> **Note**: All 50 entries are atomic/primitive widgets. None of them are simply "wrapping two existing widgets together."
> **Prerequisite**: Before starting v0.2.0, complete the 7 pending v0.1.0 items: `VerticalDivider`, `Chip`, `Placeholder`, `ReorderableList`, `Form`, `FormField`, `Autocomplete`.
