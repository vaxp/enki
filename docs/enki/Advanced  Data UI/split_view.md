# SplitView

> A resizable dual-pane layout widget supporting horizontal and vertical orientations, draggable divider handles with grip dots, minimum/maximum size constraints, and snap-to-collapse behavior.

- **Header File**: `#include "enki/widgets/split_view.hpp"`
- **C++ Class**: `enki::SplitViewWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::SplitView` (converts implicitly to `WidgetPtr`)
- **Options Struct**: `enki::SplitViewOptions`
- **Controller**: `enki::SplitViewController`
- **Orientation Enum**: `enki::SplitOrientation` (`Horizontal`, `Vertical`)

---

## Overview

`SplitView` partitions a layout into two resizable panes (left/right or top/bottom) separated by an interactive divider bar. Users can drag the divider with real-time cursor feedback (`SystemCursor::ResizeLeftRight` or `SystemCursor::ResizeUpDown`). It enforces minimum pane dimensions and can snap-collapse panes when dragged past the threshold boundary.

---

## C++ API Definition

### `SplitOrientation` Enum
```cpp
namespace enki {

enum class SplitOrientation {
    Horizontal, ///< Left / Right panes
    Vertical    ///< Top / Bottom panes
};

} // namespace enki
```

### Controller (`SplitViewController`)
```cpp
namespace enki {

class SplitViewController {
public:
    void setRatio(float ratio);
    [[nodiscard]] float getRatio() const;
    void collapseLeading();
    void collapseTrailing();
    void reset();
};

} // namespace enki
```

### Options & Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct SplitViewOptions {
    SplitOrientation      orientation        = SplitOrientation::Horizontal;
    float                 initial_ratio      = 0.50f;         ///< 0.0f to 1.0f

    float                 min_leading_size   = 100.0f;        ///< Min pixels for leading pane
    float                 min_trailing_size  = 100.0f;        ///< Min pixels for trailing pane

    bool                  allow_collapse     = true;
    float                 snap_threshold     = 40.0f;         ///< Drag distance to snap-close

    float                 handle_thickness   = 6.0f;          ///< Width/height of divider bar
    Color                 handle_color       = 0xFF334155;    ///< Slate 700
    Color                 handle_hover_color = 0xFF38BDF8;    ///< Sky 400
    Color                 handle_active_color= 0xFF0284C7;    ///< Dragging blue
    bool                  show_handle_grip   = true;          ///< Grip dots (⋮⋮) on divider

    // Callbacks
    std::function<void(float ratio)>      on_split_changed = nullptr;
    std::function<void(bool is_leading)>  on_pane_collapsed = nullptr;
};

struct SplitView {
    Key                                  key          = Key::none();
    WidgetPtr                            leading      = nullptr; ///< First pane (left/top)
    WidgetPtr                            trailing     = nullptr; ///< Second pane (right/bottom)
    SplitViewOptions                     options      = {};
    std::shared_ptr<SplitViewController> controller   = nullptr;

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `leading` | `WidgetPtr` | `nullptr` | First pane content (left or top). |
| `trailing` | `WidgetPtr` | `nullptr` | Second pane content (right or bottom). |
| `options.orientation`| `SplitOrientation` | `Horizontal` | Split axis (`Horizontal` or `Vertical`). |
| `options.initial_ratio`| `float` | `0.50f` | Proportion of space allocated to leading pane. |
| `options.min_leading_size`| `float` | `100.0f` | Minimum pixel constraint for leading pane. |
| `options.min_trailing_size`| `float` | `100.0f` | Minimum pixel constraint for trailing pane. |
| `options.allow_collapse`| `bool` | `true` | Allows collapsing a pane when dragged beyond `snap_threshold`. |
| `options.handle_thickness`| `float`| `6.0f` | Thickness of the draggable divider handle. |

---

## Code Examples (From `widgets_demo/split_view_demo/main.cpp`)

### 1. IDE Split Workspace with Resizable Divider
```cpp
#include "enki/widgets/split_view.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildCodeEditorWorkspace() {
    auto splitCtrl = std::make_shared<SplitViewController>();

    auto fileTreePane = container({
        .color = 0xFF0F172A,
        .padding = EdgeInsets::all(16.0f),
        .child = text("📁 Workspace Project Tree", { .color = 0xFF94A3B8 })
    });

    auto editorPane = container({
        .color = 0xFF1E293B,
        .padding = EdgeInsets::all(16.0f),
        .child = text("int main() { return 0; }", { .color = 0xFFF1F5F9 })
    });

    return SplitView {
        .leading = fileTreePane,
        .trailing = editorPane,
        .controller = splitCtrl,
        .options = {
            .orientation = SplitOrientation::Horizontal,
            .initial_ratio = 0.30f,
            .min_leading_size = 180.0f,
            .min_trailing_size = 300.0f,
            .handle_thickness = 6.0f,
            .show_handle_grip = true,
            .on_split_changed = [](float r) {
                std::cout << "Sidebar ratio: " << r << "\n";
            }
        }
    };
}
```

---

## See Also
- [**ResizablePanel**](./resizable_panel.md) — Freeform floating window with multi-edge resizing.
- [**Sidebar**](../Navigation/sidebar.md) — Fixed or collapsible navigation sidebar.
- [**DataGrid**](./data_grid.md) — Tabular layout commonly paired with SplitViews.
