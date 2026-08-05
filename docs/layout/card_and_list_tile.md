# Card & ListTile

## Card

### Usage

```cpp
#include "enki/widgets/card.hpp"

auto my_card = Card({
    .color = 0xFF1E293B,
    .border_radius = 16.0f,
    .elevation = 8.0f,
    .padding = {16, 16, 16, 16}
}, Text("This is inside a card!"));
```

### Properties (`CardConfig`)

The `CardConfig` exposes **7 properties**:

1. **`color`** (`Color`): Background color of the card. Defaults to `0xFF1E293B`.
2. **`border_radius`** (`float`): Radius of the corners. Defaults to `16.0f`.
3. **`elevation`** (`float`): Size of the drop shadow. Defaults to `8.0f`.
4. **`shadow_color`** (`Color`): Shadow color. Defaults to `0x80000000` (semi-transparent black).
5. **`shadow_dx`** (`float`): Shadow X offset. Defaults to `0.0f`.
6. **`shadow_dy`** (`float`): Shadow Y offset. Defaults to `4.0f`.
7. **`padding`** (`EdgeInsets`): Padding inside the card. Defaults to `{16, 16, 16, 16}`.

## ListTile

### Usage

```cpp
#include "enki/widgets/list_tile.hpp"

auto my_tile = ListTile({
    .leading = Icon(Icons::Settings),
    .title = Text("Settings"),
    .subtitle = Text("Configure your app"),
    .trailing = Icon(Icons::ChevronRight),
    .on_tap = []() { std::cout << "Tapped!" << std::endl; },
    .content_padding = {16, 12, 16, 12}
});
```

### Properties (`ListTileConfig`)

The `ListTileConfig` exposes **8 properties**:

1. **`leading`** (`WidgetPtr`): A widget to display before the title (e.g., an Avatar or Icon).
2. **`title`** (`WidgetPtr`): The primary content of the list tile (usually a `Text` widget).
3. **`subtitle`** (`WidgetPtr`): Additional content displayed below the title.
4. **`trailing`** (`WidgetPtr`): A widget to display after the title (e.g., a Switch, Checkbox, or Chevron).
5. **`on_tap`** (`std::function<void()>`): Callback function executed when the tile is clicked or tapped.
6. **`content_padding`** (`EdgeInsets`): Padding inside the tile. Defaults to `{16, 12, 16, 12}`. Note that `left` and `right` paddings distance the inner items from the tile's edges.
7. **`hover_color`** (`Color`): Background color when hovered. Defaults to `0x0A000000`.
8. **`pressed_color`** (`Color`): Background color when pressed. Defaults to `0x14000000`.
