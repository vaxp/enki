# Container

## Usage

```cpp
#include "enki/widgets/container.hpp"

auto my_container = Container({
    .color = 0xFF1E1E1E,
    .width = 200.0f,
    .height = 100.0f,
    .padding = EdgeInsets::all(16.0f),
    .border_radius = BorderRadius::circular(12.0f),
    .child = Text("Hello, World!")
});
```

## Properties (`ContainerConfig`)

The `ContainerConfig` exposes an extensive set of properties to control every aspect of the node's layout and appearance. 

### Styling
1. **`color`** (`Color`): Background color of the container. Defaults to `Colors::Transparent`.
2. **`border_radius`** (`BorderRadius`): Corner radii (e.g., `BorderRadius::circular(12)`).
3. **`border`** (`Border`): Border style and width around the container.
4. **`shadow`** (`std::optional<Shadow>`): Box shadow applied to the container.
5. **`gradient`** (`std::optional<Gradient>`): Background gradient (overrides `color`).
6. **`opacity`** (`float`): Opacity level from `0.0` to `1.0`. Defaults to `1.0`.
7. **`clip_content`** (`bool`): Whether to clip child elements that overflow the bounds (useful with `border_radius`). Defaults to `false`.

### Fixed Sizing
8. **`width`** / **`height`** (`float`): Explicit fixed sizes. Set to `0` for auto-sizing based on children.
9. **`width_percent`** / **`height_percent`** (`std::optional<float>`): Size as a percentage of the parent (e.g., `100.0f`).
10. **`width_auto`** / **`height_auto`** (`bool`): Explicitly set auto-sizing.
11. **`aspect_ratio`** (`float`): Enforces a specific aspect ratio for the container.

### Constrained Sizing
12. **`min_width`** / **`min_height`** (`float`): Minimum dimensions.
13. **`min_width_percent`** / **`min_height_percent`** (`std::optional<float>`): Minimum dimensions as percentages.
14. **`max_width`** / **`max_height`** (`float`): Maximum dimensions.
15. **`max_width_percent`** / **`max_height_percent`** (`std::optional<float>`): Maximum dimensions as percentages.

### Spacing (Padding & Margin)
16. **`padding`** (`EdgeInsets`): Inner spacing between the container's edge and its child.
17. **`padding_percent`** (`std::optional<EdgeInsets>`): Padding relative to parent size.
18. **`margin`** (`EdgeInsets`): Outer spacing between the container and other adjacent elements.
19. **`margin_percent`** (`std::optional<EdgeInsets>`): Margin relative to parent size.
20. **`margin_auto`** (`bool`): Use auto margins.

### Flex Behaviors
*(Applicable when the container is a child of a `FlexBox`)*
21. **`flex_shrink`** (`float`): Defines how much the container will shrink relative to other items if there is not enough space. Defaults to `0.0`.
22. **`flex_grow`** (`float`): Defines how much the container will grow. Defaults to `0.0`.
23. **`flex_basis`** (`float`): Default size before distributing remaining space.
24. **`flex_basis_percent`** (`std::optional<float>`): Flex basis as a percentage.
25. **`flex_basis_auto`** (`bool`): Use auto flex basis.

### Absolute Positioning
26. **`position_type`** (`ANUPositionType`): Defines positioning mode (`ANUPositionTypeRelative` or `ANUPositionTypeAbsolute`).
27. **`position_top`**, **`position_bottom`**, **`position_left`**, **`position_right`**, **`position_start`**, **`position_end`** (`float`): Explicit edge offsets when using Absolute positioning.
28. **`position_auto`** (`bool`): Automatically determine position.
29. **`position_*_percent`**: Percentage-based offsets.

### Advanced Alignment
30. **`alignment`** (`Alignment`): Quick alignment of the child inside the container (e.g., `Alignment::Center`, `Alignment::TopLeft`). Defaults to `Alignment::Center`.
31. **`align_self`** (`ANUAlign`): Overrides cross-axis alignment for this specific container.
32. **`align_content`** (`ANUAlign`): Cross-axis alignment of multiple lines (if container acts as a flex container).
33. **`justify_content`**, **`align_items`**, **`flex_direction`**, **`flex_wrap`**: Optional overrides allowing the Container to act directly as a custom flex container.

### Other Flexbox Props
34. **`direction`** (`ANUDirection`): Layout flow direction (LTR or RTL).
35. **`display`** (`ANUDisplay`): Display property (`Flex` or `None`).
36. **`overflow`** (`ANUOverflow`): Determines if overflow affects layout.
37. **`box_sizing`** (`ANUBoxSizing`): Usually `BorderBox`.
38. **`gap`**, **`row_gap`**, **`column_gap`** (`float`): Gap spacing values.

### Content
39. **`child`** (`WidgetPtr`): A single child widget wrapped by the container.
