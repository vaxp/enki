# Enki Rich Input Controls Suite

> Hardware-accelerated, studio-grade input controls and specialized verification primitives designed for high-performance desktop interfaces on Wayland and X11.

The **Rich Input Controls** category provides specialized interactive widgets for complex user inputs beyond basic text fields and checkboxes — including studio rotary dials, multi-segment controllers, star ratings, 2FA/PIN security boxes, tokenized tags, and native OS drag-and-drop file surfaces.

---

## Architectural Highlights

- **Native Protocol Integration**: Widgets like `FileDropZone` hook directly into Wayland `wl_data_device` and X11 `XDnD` without intermediate toolkit layers.
- **Hardware-Accelerated Rendering**: Custom `RenderBox` implementations use direct Skia canvas rendering with anti-aliasing, neon glows, drop shadows, and dashed conveyor border effects.
- **State Reconciliation & Lifecycle**: Stateful widgets (`OTPField`, `PinField`, `TagInput`) implement `didUpdateWidget` for synchronization during tree reconciliation.
- **Zero-Copy Layout**: Driven by the Anu Flexbox layout engine with sub-millisecond layout passes.

---

## Widget Catalog

| # | Widget | Primary Type / Helper | Header | Description |
|---|---|---|---|---|
| 1 | [**Knob**](./knob.md) | `struct KnobProps`, `knob(...)` | `<enki/widgets/knob.hpp>` | Studio-grade rotary dial for audio and parameter manipulation. |
| 2 | [**SegmentedControl**](./segmented_control.md) | `struct SegmentedControlProps`, `segmentedControl(...)` | `<enki/widgets/segmented_control.hpp>` | Horizontally grouped option buttons with animated sliding thumb indicator. |
| 3 | [**RatingBar**](./rating_bar.md) | `struct RatingBarProps`, `ratingBar(...)` | `<enki/widgets/rating_bar.hpp>` | Interactive fractional star rating widget with hover feedback and glowing stars. |
| 4 | [**ToggleButton**](./toggle_button.md) | `struct ToggleButtonProps`, `toggleButton(...)` | `<enki/widgets/toggle_button.hpp>` | Atomic binary state button with Filled, Outlined, Ghost, and Glow visual styles. |
| 5 | [**OTPField**](./otp_field.md) | `struct OTPFieldProps`, `otpField(...)` | `<enki/widgets/otp_field.hpp>` | Segmented One-Time Password boxes with auto-focus advance and clipboard pasting. |
| 6 | [**PinField**](./pin_field.md) | `struct PinFieldProps`, `pinField(...)` | `<enki/widgets/pin_field.hpp>` | Secure PIN entry with delayed timed masking and masked bullets. |
| 7 | [**TagInput**](./tag_input.md) | `struct TagInputProps`, `tagInput(...)` | `<enki/widgets/tag_input.hpp>` | Inline multi-tag text entry with removable chips and keyboard navigation. |
| 8 | [**FileDropZone**](./file_drop_zone.md) | `struct FileDropZoneProps`, `fileDropZone(...)` | `<enki/widgets/file_drop_zone.hpp>` | Real OS Drag-and-Drop file intake surface with animated dashed border. |
