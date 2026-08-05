# ScrollView

## Usage

```cpp
#include "enki/widgets/scroll_view.hpp"
#include "enki/widgets/flexbox.hpp"

// Example: A scrollable column of items
auto my_scroll_view = ScrollView({
    .show_scrollbar = true,
    .scroll_speed = 40.0f,
    .border_radius = 8.0f,
    .child = Column({
        .widthPercent = 100.0f,
        // ... many children that would exceed the screen height ...
    })
});
```

## Properties (`ScrollViewConfig`)

The `ScrollViewConfig` exposes **4 properties**:

1. **`child`** (`WidgetPtr`): The single child widget that will be scrolled. Typically, this is a `Column` or a `FlexBox` containing multiple items.
2. **`show_scrollbar`** (`bool`): Whether to display a scrollbar indicator. Defaults to `true`.
3. **`scroll_speed`** (`float`): Multiplier for how fast the content scrolls in response to mouse wheel or touch pad events. Defaults to `30.0f`.
4. **`border_radius`** (`float`): Applies a corner radius to the scroll view clipping mask, ensuring scrolled content is clipped nicely if the view has rounded corners. Defaults to `0.0f`.
