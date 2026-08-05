# FlexBox, Column, and Row

## Usage

```cpp
#include "enki/widgets/flexbox.hpp"

// Example: A Row aligning children horizontally and centering them
auto my_row = Row({
    .justify = Justify::Center,
    .align = Align::Center,
    .height = 50.0f,
    .children = {
        Text("Left"),
        Text("Middle"),
        Text("Right")
    }
});

// Example: A Column stacking children vertically
auto my_col = Column({
    .justify = Justify::SpaceBetween,
    .widthPercent = 100.0f,
    .padding = std::pair{Edge::All, 10.0f},
    .backgroundColor = 0xFF1E1E1E,
    .children = {
        /* ... child widgets ... */
    }
});
```

## Properties (`FlexBoxConfig`)

The configuration struct for FlexBox exposes **22 properties**:

### Layout & Alignment
1. **`direction`** (`FlexDirection`): Specifies the layout axis (`Column`, `ColumnReverse`, `Row`, `RowReverse`). Defaults to `Column`.
2. **`justify`** (`Justify`): Aligns children along the *main axis* (e.g., `Start`, `Center`, `End`, `SpaceBetween`, `SpaceAround`, `SpaceEvenly`). Defaults to `Start`.
3. **`align`** (`Align`): Aligns children along the *cross axis* (e.g., `Auto`, `Start`, `Center`, `End`, `Stretch`, `Baseline`). Defaults to `Stretch`.
4. **`alignContent`** (`Align`): Aligns multiple lines of children along the cross axis when wrapping is enabled. Defaults to `Start`.
5. **`alignSelf`** (`Align`): Overrides the parent's `align` property for this specific node. Defaults to `Auto`.
6. **`wrap`** (`FlexWrap`): Determines whether children should wrap to the next line if there is not enough space (`NoWrap`, `Wrap`, `WrapReverse`). Defaults to `NoWrap`.
7. **`gap`** (`float`): Defines the spacing (in pixels) between children. Defaults to `ANUUndefined` (0).

### Sizing
8. **`width`** (`float`): Fixed width in pixels.
9. **`widthPercent`** (`float`): Width relative to the parent's width, from `0.0` to `100.0`.
10. **`height`** (`float`): Fixed height in pixels.
11. **`heightPercent`** (`float`): Height relative to the parent's height, from `0.0` to `100.0`.

### Flex Behaviors (When placed inside another FlexBox)
12. **`flex`** (`float`): Specifies how the item will grow or shrink to fit the space available.
13. **`flexGrow`** (`float`): Defines the ability for a flex item to grow if necessary. Defaults to `0.0`.
14. **`flexShrink`** (`float`): Defines the ability for a flex item to shrink if necessary. Defaults to `0.0`.
15. **`flexBasis`** (`float`): Default size of an element before the remaining space is distributed.
16. **`flexBasisPercent`** (`float`): Default size as a percentage of the parent.

### Spacing & Positioning
17. **`padding`** (`std::optional<std::pair<Edge, float>>`): Internal spacing around the children. Passed as a pair of `Edge` and pixel value (e.g., `std::pair{Edge::All, 10.0f}`).
18. **`margin`** (`std::optional<std::pair<Edge, float>>`): External spacing around the FlexBox itself.
19. **`positionType`** (`PositionType`): Defines if the node is positioned relative to its normal flow (`Relative`) or absolutely (`Absolute`). Defaults to `Relative`.
20. **`position`** (`std::optional<std::pair<Edge, float>>`): The exact offset when `positionType` is `Absolute` or explicit offset when `Relative`.

### Styling & Content
21. **`backgroundColor`** (`Color`): Background color of the FlexBox (ARGB format, e.g., `0xFF1E1E1E`). Defaults to transparent.
22. **`borderRadius`** (`float`): Corner rounding radius in pixels. Defaults to `0.0`.
23. **`overflow`** (`Overflow`): Defines how children that overflow the bounds should be handled (`Visible`, `Hidden`, `Scroll`). Defaults to `Visible`.

### Children
24. **`children`** (`std::vector<WidgetPtr>`): A list of child widgets to be laid out.
