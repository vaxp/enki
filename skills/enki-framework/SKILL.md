---
name: enki-framework
description: >-
  Comprehensive guide and expert pair programmer for ENKI, the native C++20 reactive
  GUI framework. Use this skill whenever creating, modifying, refactoring, or debugging
  applications built with ENKI, using ENKI widgets (Row, Column, Container, ListView, Button, etc.),
  reactive state management (Cubit, BlocProvider, BlocBuilder), animations, Skia graphics,
  application lifecycle (runApp, AppConfig), or Meson build setups.
---

# ENKI Framework — AI Assistant & Developer Guide

**ENKI** is a high-performance native **C++20 reactive GUI framework** powered by Skia GPU rasterization, an in-house Flexbox engine (Anu), and a BLoC/Cubit state management architecture. It targets Linux (Wayland, X11, bare-metal DRM/KMS), Windows, Android, and WebAssembly with a single codebase.

---

## 1. Golden Rules for AI Agents

When generating or editing ENKI code, **strictly follow these rules**:

1. **Master Includes**:
   - `#include "enki/enki.hpp"` — Provides all 130+ declarative widgets, layout engine, graphics, and lifecycle.
   - `#include "enki/state.hpp"` — Provides reactive state management (`Cubit<S>`, `BlocProvider`, `BlocBuilder`).
   - Do **NOT** include Qt, GTK, wxWidgets, or ImGui headers.
2. **Memory Management**:
   - Widgets are represented as `WidgetPtr` (`std::shared_ptr<Widget>`).
   - Instantiate custom widgets with `std::make_shared<MyWidget>()`.
   - Factory functions (e.g. `row()`, `column()`, `container()`, `text()`, `button()`) return `WidgetPtr`.
3. **C++20 Designated Initializers**:
   - Use designated initializer syntax for widget configuration:
     ```cpp
     text("Hello ENKI", {
         .color = 0xFFFFFFFF,
         .font_size = 18.0f,
         .font_weight = FontWeight::Bold,
     });
     ```
4. **Color Format**:
   - 32-bit ARGB Hex: `0xAARRGGBB` (e.g., `0xFF0F172A` for dark slate, `0xFFFFFFFF` for white, `0x80000000` for 50% translucent black).
5. **Granular Reactive Rebuilds**:
   - **Never** rebuild the entire application tree on state changes.
   - Pages and screens should be `StatelessWidget`.
   - Wrap dynamic subtrees in `bloc_builder<MyCubit>([](BuildContext&, const MyState& s) -> WidgetPtr { ... })`. Only the subtree inside `bloc_builder` is rebuilt.
6. **State Immutability**:
   - States are immutable value structs with C++20 defaulted equality:
     ```cpp
     struct AppState {
         int counter = 0;
         bool operator==(const AppState&) const = default;
     };
     ```

---

## 2. Minimal Starter Application

```cpp
#include "enki/enki.hpp"

using namespace enki;

class HelloApp : public StatelessWidget {
public:
    WidgetPtr build(BuildContext&) override {
        return container({
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .color = 0xFF0F172A,
            .child = center(
                text("Welcome to ENKI C++20!", {
                    .color = 0xFF38BDF8,
                    .font_size = 24.0f,
                    .font_weight = FontWeight::Bold,
                })
            )
        });
    }

    std::string_view typeName() const override { return "HelloApp"; }
};

int main() {
    AppConfig config;
    config.title = "ENKI Minimal App";
    config.width = 800;
    config.height = 600;
    config.resizable = true;
    config.vsync = true;
    config.clear_color = 0xFF0F172A;

    return runApp(std::make_shared<HelloApp>(), config);
}
```

---

## 3. Application Lifecycle & `AppConfig`

Applications start via `enki::runApp(WidgetPtr root, AppConfig config = {})`.

### `AppConfig` Key Properties (`include/enki/app/app.hpp`)

| Property | Type | Default | Description |
|---|---|---|---|
| `title` | `std::string` | `"ENKI App"` | Window title for OS taskbar and native titlebar. |
| `width` | `int` | `800` | Window initial width in logical pixels. |
| `height` | `int` | `600` | Window initial height in logical pixels. |
| `resizable` | `bool` | `true` | Allows or restricts window resizing by user. |
| `vsync` | `bool` | `true` | Vertical sync with monitor refresh rate (60/120/144/240Hz). |
| `msaa` | `bool` | `true` | Multi-sample anti-aliasing for crisp vector shapes & fonts. |
| `clear_color` | `Color` | `0xFF0F172A` | Framebuffer clear color before painting widgets. |
| `target_fps` | `int` | `60` | Frame rate limit when `vsync = false` (set `0` for uncapped). |
| `enable_csd` | `bool` | `false` | Enable Client-Side Decorations (custom window frame & titlebar). |
| `show_performance_overlay` | `bool` | `false` | In-engine telemetry HUD showing FPS, layout/paint times, and dirty elements. |

👉 For full lifecycle documentation, see [references/lifecycle/start_application.md](references/lifecycle/start_application.md).

---

## 4. Reactive State Management (`Cubit<S>`)

ENKI adopts a BLoC/Cubit reactive stream architecture:

### Step 1: Define Immutable State
```cpp
struct UserState {
    std::string username = "";
    bool is_logged_in = false;
    bool operator==(const UserState&) const = default;
};
```

### Step 2: Implement `Cubit<S>`
```cpp
class UserCubit : public enki::Cubit<UserState> {
public:
    UserCubit() : Cubit(UserState{}) {}

    void login(std::string name) {
        emit(UserState{ .username = std::move(name), .is_logged_in = true });
    }

    void logout() {
        emit(UserState{});
    }
};
```

### Step 3: Inject with `BlocProvider`
```cpp
WidgetPtr build(BuildContext&) override {
    return bloc_provider<UserCubit>(
        [](BuildContext&) { return std::make_shared<UserCubit>(); },
        std::make_shared<MainDashboard>()
    );
}
```

### Step 4: Consume with `BlocBuilder` & `context.read`
```cpp
WidgetPtr build(BuildContext& context) override {
    auto user_cubit = context.read<UserCubit>();

    return bloc_builder<UserCubit>([user_cubit](BuildContext&, const UserState& state) -> WidgetPtr {
        if (!state.is_logged_in) {
            return button({
                .label = "Sign In",
                .on_click = [user_cubit]() { user_cubit->login("Alice"); }
            });
        }

        return text("Hello, " + state.username + "!", { .color = 0xFFFFFFFF });
    });
}
```

👉 For full state management documentation, see [references/state_management/cubit.md](references/state_management/cubit.md).

---

## 5. Layout Engine (Anu Flexbox)

Layout in ENKI is handled by the Anu engine:

- **Row**: Horizontal flex container. `row({ .justify_content = Justify::Center, .gap = StyleValue::point(12.0f), .children = { ... } })`
- **Column**: Vertical flex container. `column({ .align_items = Align::Stretch, .children = { ... } })`
- **Container**: Sized box with padding, margin, decoration (borders, radius, background color, gradient, shadows).
- **Stack & Positioned**: Absolute overlapping layers (e.g. badges, floating buttons, HUDs).
- **Expanded & Flexible**: Child widgets that expand to fill available space in a Row/Column according to flex factor.
- **Padding & Margin**: `StyleInsets::all(16.0f)`, `StyleInsets::symmetric(v, h)`, `StyleInsets::only(t, r, b, l)`.
- **Dimensions**: `StyleValue::point(100.0f)`, `StyleValue::percent(100.0f)`, `StyleValue::auto_val()`.

👉 For full layout widget documentation, see [references/layout/](references/layout/).

---

## 6. Widget Taxonomy & Reference Index

Every widget has a dedicated documentation file in `references/`:

### Layout & Sizing (`references/layout/`)
| Widget | Function / Struct | Description | Reference |
|---|---|---|---|
| `Row` | `row(FlexboxProps)` | Horizontal flex layout | [row.md](references/layout/row.md) |
| `Column` | `column(FlexboxProps)` | Vertical flex layout | [column.md](references/layout/column.md) |
| `Container` | `container(ContainerProps)` | Box with background, border, radius, padding | [container.md](references/layout/container.md) |
| `Stack` | `stack(StackProps)` | Overlapping layered children | [stack.md](references/layout/stack.md) |
| `Padding` | `padding(PaddingProps)` | Inset wrapper around child | [padding.md](references/layout/padding.md) |
| `Center` | `center(WidgetPtr child)` | Centers child in available space | [center.md](references/layout/center.md) |
| `Align` | `align(AlignProps)` | Aligns child within parent bounds | [align.md](references/layout/align.md) |
| `SizedBox` | `sized_box(SizedBoxProps)` | Explicit fixed width/height container | [sized_box.md](references/layout/sized_box.md) |
| `Expanded` | `expanded(WidgetPtr, int flex = 1)` | Expands child along flex main-axis | [expanded.md](references/layout/expanded.md) |
| `Flexible` | `flexible(WidgetPtr, int flex = 1)` | Flexibly sizes child along main-axis | [flexible.md](references/layout/flexible.md) |
| `Wrap` | `wrap(WrapProps)` | Multi-line wrapping flow of children | [wrap.md](references/layout/wrap.md) |
| `Spacer` | `spacer(int flex = 1)` | Consumes remaining space in Row/Column | [spacer.md](references/layout/spacer.md) |
| `SplitView` | `split_view(SplitViewProps)` | Resizable split layout (horizontal/vertical) | [split_view.md](references/advanced_data_ui/split_view.md) |
| `ResizablePanel` | `resizable_panel(...)` | Panel with drag-to-resize borders | [resizable_panel.md](references/advanced_data_ui/resizable_panel.md) |

### Basic UI Controls (`references/basic_ui/`)
| Widget | Struct / Function | Description | Reference |
|---|---|---|---|
| `Button` | `button(ButtonProps)` / `Button{}` | Interactive button with hover, ripple, shaders | [button.md](references/basic_ui/button.md) |
| `IconButton` | `icon_button(IconButtonProps)` | Icon-only button with hover/press feedback | [icon_button.md](references/basic_ui/icon_button.md) |
| `FloatingActionButton` | `floating_action_button(...)` | Circular high-priority floating action button | [floating_action_button.md](references/basic_ui/floating_action_button.md) |
| `Text` | `text(string, TextProps)` | High-precision SkParagraph formatted text | [text.md](references/basic_ui/text.md) |
| `RichText` | `rich_text(RichTextProps)` | Multi-style styled text spans | [rich_text.md](references/basic_ui/rich_text.md) |
| `Icon` | `icon(IconData, IconProps)` | Vector / Material icon glyph | [icon.md](references/basic_ui/icon.md) |
| `Image` | `image(ImageProps)` | Image loader with fit modes, decoders, caching | [image.md](references/basic_ui/image.md) |
| `Card` | `card(CardProps)` | Elevated surface container with rounded corners | [card.md](references/basic_ui/card.md) |
| `Chip` | `chip(ChipProps)` | Compact interactive tag / badge / filter | [chip.md](references/basic_ui/chip.md) |
| `Badge` | `badge(BadgeProps)` | Number / indicator badge overlay | [badge.md](references/basic_ui/badge.md) |
| `Avatar` | `avatar(AvatarProps)` | Circular profile picture or initials | [avatar.md](references/basic_ui/avatar.md) |
| `Divider` | `divider(DividerProps)` | Horizontal line separator | [divider.md](references/basic_ui/divider.md) |

### Input & Form Controls (`references/input_forms/` & `references/rich_input_controls/`)
| Widget | Function / Struct | Description | Reference |
|---|---|---|---|
| `TextField` | `text_field(TextFieldProps)` | Single-line text input | [text_field.md](references/input_forms/text_field.md) |
| `TextArea` | `text_area(TextAreaProps)` | Multi-line text editor with scrolling | [text_area.md](references/input_forms/text_area.md) |
| `Form` / `FormField` | `Form{}`, `TextFormField{}` | Form validation engine with `Validators` | [form.md](references/input_forms/form.md) |
| `Checkbox` | `checkbox(CheckboxProps)` | Boolean checkbox | [checkbox.md](references/input_forms/checkbox.md) |
| `Radio` | `radio(RadioProps)` | Mutually exclusive radio option | [radio.md](references/input_forms/radio.md) |
| `Switch` | `switch_widget(SwitchProps)` | Smooth toggle switch | [switch.md](references/input_forms/switch.md) |
| `Slider` | `slider(SliderProps)` | Continuous or discrete range slider | [slider.md](references/input_forms/slider.md) |
| `ComboBox` | `combo_box(ComboBoxProps)` | Select dropdown with search | [combo_box.md](references/input_forms/combo_box.md) |
| `DatePicker` / `TimePicker` | `date_picker(...)`, `time_picker(...)` | Native date and time selectors | [date_picker.md](references/input_forms/date_picker.md) |
| `SegmentedControl` | `segmented_control(...)` | iOS / macOS style pill segment switcher | [segmented_control.md](references/rich_input_controls/segmented_control.md) |
| `Knob` | `knob(KnobProps)` | Rotary audio/synth style knob controller | [knob.md](references/rich_input_controls/knob.md) |
| `OtpField` / `PinField` | `otp_field(...)`, `pin_field(...)` | Multi-digit verification code input | [otp_field.md](references/rich_input_controls/otp_field.md) |

### Scrolling & Lists (`references/scrolling_lists/`)
| Widget | Struct / Function | Description | Reference |
|---|---|---|---|
| `ListView` | `ListView{ .item_builder = ... }` | Linear scrollable list with lazy builder | [list_view.md](references/scrolling_lists/list_view.md) |
| `GridView` | `GridView{ .item_builder = ... }` | 2D scrollable grid with flexible columns | [grid_view.md](references/scrolling_lists/grid_view.md) |
| `DataTable` | `data_table(DataTableProps)` | Structured table with sorting & selection | [data_table.md](references/scrolling_lists/data_table.md) |
| `DataGrid` | `data_grid(DataGridProps)` | Enterprise virtualized grid with column resize | [data_grid.md](references/advanced_data_ui/data_grid.md) |
| `TreeView` | `tree_view(TreeViewProps)` | Hierarchical collapsible tree | [tree_view.md](references/scrolling_lists/tree_view.md) |
| `Scrollbar` | `scrollbar(ScrollbarProps)` | Visual scrollbar handle | [scrollbar.md](references/scrolling_lists/scrollbar.md) |
| `ReorderableList`| `reorderable_list(...)` | Drag-and-drop reorderable items | [reorderable_list.md](references/scrolling_lists/reorderable_list.md) |

### Overlays, Dialogs & Menus (`references/overlays/` & `references/native_popups/`)
| Widget | Function / Struct | Description | Reference |
|---|---|---|---|
| `Dialog` | `dialog(DialogProps)` | Modal alert or confirmation dialog | [dialog.md](references/overlays/dialog.md) |
| `BottomSheet` | `bottom_sheet(BottomSheetProps)`| Slide-up mobile/desktop modal sheet | [bottom_sheet.md](references/overlays/bottom_sheet.md) |
| `Snackbar` | `snackbar(SnackbarProps)` | Floating ephemeral message bar | [snackbar.md](references/overlays/snackbar.md) |
| `CommandPalette`| `command_palette(...)` | VS Code style Ctrl+P / Cmd+K search palette | [command_palette.md](references/overlays/command_palette.md) |
| `ContextMenu` | `context_menu(...)` | Right-click popup menu | [context_menu.md](references/native_popups/context_menu.md) |
| `Tooltip` | `tooltip(TooltipProps)` | Hover information tooltip | [tooltip.md](references/native_popups/tooltip.md) |
| `Popover` | `popover(PopoverProps)` | Anchored flyout card | [popover.md](references/native_popups/popover.md) |

### Navigation (`references/navigation/`)
| Widget | Function / Struct | Description | Reference |
|---|---|---|---|
| `Navigator` | `navigator(NavigatorProps)` | Stack-based page routing with transitions | [navigator.md](references/navigation/navigator.md) |
| `NavigationBar`| `navigation_bar(...)` | Bottom or top app navigation bar | [navigation_bar.md](references/navigation/navigation_bar.md) |
| `NavigationRail`| `navigation_rail(...)` | Side compact icon bar for desktop/tablet | [navigation_rail.md](references/navigation/navigation_rail.md) |
| `Sidebar` | `sidebar(SidebarProps)` | Collapsible multi-tier navigation sidebar | [sidebar.md](references/navigation/sidebar.md) |
| `Drawer` | `drawer(DrawerProps)` | Off-canvas slide-in menu drawer | [drawer.md](references/navigation/drawer.md) |
| `TabBar` / `TabView` | `tab_bar(...)`, `tab_view(...)`| Multi-tab switching view | [tab_bar.md](references/navigation/tab_bar.md) |

### Animation & Skia Graphics (`references/animation_motion/` & `references/paint_visual_effects/`)
| Widget / Concept | Description | Reference |
|---|---|---|
| `AnimatedContainer` | Smoothly interpolates color, padding, size on change | [animated_container.md](references/animation_motion/animated_container.md) |
| `BackdropFilter` | Glassmorphism, blur, color filter effects | [backdrop_filter.md](references/paint_visual_effects/backdrop_filter.md) |
| `ClipRRect` | Clips child with rounded corners | [clip_rrect.md](references/paint_visual_effects/clip_rrect.md) |
| `ShaderMask` | Injects custom SkSL runtime shaders | [shader_mask.md](references/paint_visual_effects/shader_mask.md) |
| `SkiaCanvas` | Direct immediate-mode Skia 2D rendering | [skia_canvas.md](references/advanced_widgets/skia_canvas.md) |
| `Lottie` | Vector animations rendered via Skottie | [lottie.md](references/advanced_widgets/lottie.md) |
| `AudioWaveform` | Live FFT spectrum analyzer (PulseAudio) | [audio_waveform.md](references/advanced_widgets/audio_waveform.md) |
| `VideoPlayer` | Hardware VA-API accelerated video player (FFmpeg) | [video_player.md](references/advanced_widgets/video_player.md) |

---

## 7. Working Examples Catalog

In the `examples/` directory:
- [counter_cubit.cpp](examples/counter_cubit.cpp) — Complete BLoC/Cubit reactive counter application.
- [form_validation.cpp](examples/form_validation.cpp) — Enterprise input form with composable validators.
- [meson.build](examples/meson.build) — Standard build configuration file.

---

## 8. CLI & Developer Tooling

Run the native CLI from repository root:
- `enki doctor` — Run diagnostics on compilers, SDKs, and platform display servers.
- `enki devices` — List connected screens, DRM/KMS graphics cards, and Android devices.
- `enki create my_app` — Scaffold a new application with clean architecture.
- `enki run <target> desktop` — Build and launch on Linux/Windows.
- `enki run <target> drm` — Launch direct on bare-metal DRM/KMS without an X11 or Wayland server.
- `enki run <target> android` — Build, package, and install APK to connected Android device.
- `enki clean` — Clean all build caches.

👉 For build details, see [references/build_system.md](references/build_system.md).
