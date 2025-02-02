# Stack

## Usage

```cpp
#include "enki/widgets/stack.hpp"
#include "enki/widgets/container.hpp"

auto my_stack = Stack({
    .alignment = Alignment::Center,
    .children = {
        // Base child (Bottom-most)
        Container({
            .color = 0xFF333333,
            .width = 200.0f,
            .height = 200.0f
        }),
        // Overlapping child (Top-most)
        Container({
            .color = 0xFFFF0000,
            .width = 50.0f,
            .height = 50.0f
        })
    }
});
```

## Absolute Positioning

```cpp
Stack({
    .children = {
        // ... base elements ...

        // Absolutely positioned element
        Container({
            .position_type = ANUPositionTypeAbsolute,
            .position_top = 10.0f,
            .position_right = 10.0f,
            .child = Text("I am fixed to the top right!")
        })
    }
});
```

## Properties (`StackConfig`)

The `StackConfig` exposes **2 properties**:

1. **`alignment`** (`Alignment`): Determines how children are aligned within the stack if they are not explicitly absolutely positioned. For example, `Alignment::TopLeft`, `Alignment::Center`, `Alignment::BottomRight`. Defaults to `Alignment::TopLeft`.
2. **`children`** (`std::vector<WidgetPtr>`): The list of widgets to overlap. They are painted sequentially from back to front.
