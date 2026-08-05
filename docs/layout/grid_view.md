# GridView

## Usage

```cpp
#include "enki/widgets/grid_view.hpp"

auto my_grid = GridView({
    .crossAxisCount = 3,           // 3 columns
    .mainAxisSpacing = 10.0f,      // 10px vertical spacing between rows
    .crossAxisSpacing = 10.0f,     // 10px horizontal spacing between columns
    .childAspectRatio = 1.0f,      // Square items (1:1 ratio)
    .widthPercent = 100.0f,
    .children = {
        // ... many child widgets ...
    }
});
```

## Properties (`GridViewConfig`)

The `GridViewConfig` exposes **12 properties**:

### Grid Geometry
1. **`crossAxisCount`** (`int`): The number of columns in the grid. Defaults to `2`.
2. **`mainAxisSpacing`** (`float`): Vertical spacing (gap) between rows in pixels. Defaults to `0.0f`.
3. **`crossAxisSpacing`** (`float`): Horizontal spacing (gap) between columns in pixels. Defaults to `0.0f`.
4. **`childAspectRatio`** (`float`): The width-to-height ratio of the children. Set to `1.0f` for perfect squares. Set to `0.0f` to let the height size automatically based on child content. Defaults to `1.0f`.

### Container Settings
5. **`width`** (`float`): Fixed width in pixels.
6. **`height`** (`float`): Fixed height in pixels.
7. **`widthPercent`** (`float`): Relative width percentage (0 to 100).
8. **`heightPercent`** (`float`): Relative height percentage (0 to 100).
9. **`flexGrow`** (`float`): Flex grow factor when placed inside another FlexBox. Defaults to `0.0f`.
10. **`flexShrink`** (`float`): Flex shrink factor when placed inside another FlexBox. Defaults to `0.0f`.
11. **`padding`** (`std::optional<std::pair<Edge, float>>`): Internal spacing around the grid's bounding box.
12. **`margin`** (`std::optional<std::pair<Edge, float>>`): External spacing around the grid.
13. **`backgroundColor`** (`Color`): Background color for the grid area. Defaults to transparent.

### Content
14. **`children`** (`std::vector<WidgetPtr>`): A list of widgets to display in the grid cells.
