# Avatar

> A specialized user profile picture widget supporting automatic circular clipping, image loading, fallback text initials, status badges, and overlapping `AvatarGroup` layouts.

- **Header File**: `#include "enki/widgets/avatar.hpp"`
- **C++ Class**: `enki::AvatarWidget`
- **Declarative Struct**: `enki::Avatar` (converts implicitly to `WidgetPtr`)
- **Props Struct**: `enki::AvatarProps`
- **Group Struct**: `enki::AvatarGroup` (`enki::AvatarGroupWidget`)
- **Underlying Engine**: Skia circular canvas clipping + Stack status badge overlays

---

## Overview

`Avatar` provides standard user profile representation. If an image path is provided, it loads and renders the profile picture clipped into a circle. If the image is omitted or missing, it automatically displays user initials centered over a colored circular background. It also includes built-in online status badges and shadow borders.

The companion `AvatarGroup` widget organizes multiple avatars in a horizontally stacked, overlapping sequence.

---

## C++ API Definition

### Declarative Struct (`Avatar`)
```cpp
namespace enki {

struct Avatar {
    Key                    key                = Key::none();
    float                  radius             = 24.0f;
    Color                  background_color   = 0xFF38BDF8; // Default blue
    Color                  text_color         = 0xFFFFFFFF; // White
    std::string            initials           = "";
    std::string            image_path         = "";
    std::shared_ptr<Image> image_data         = nullptr;
    
    // Border Styling
    float                  border_width       = 0.0f;
    Color                  border_color       = 0xFFFFFFFF;
    
    // Online / Activity Status Badge
    bool                   show_badge         = false;
    Color                  badge_color        = 0xFF10B981; // Green
    float                  badge_size         = 12.0f;
    float                  badge_border_width = 2.0f;
    Color                  badge_border_color = 0xFFFFFFFF;
    
    // Elevation Shadow
    float                  shadow_blur        = 0.0f;
    Color                  shadow_color       = 0x40000000;

    operator WidgetPtr() const;
};

} // namespace enki
```

### Declarative Struct (`AvatarGroup`)
```cpp
namespace enki {

struct AvatarGroup {
    Key                    key         = Key::none();
    std::vector<WidgetPtr> avatars;
    float                  spacing     = -12.0f; // Negative spacing creates overlap
    size_t                 max_avatars = 4;

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference (`Avatar`)

| Property | Type | Default | Description |
|---|---|---|---|
| `radius` | `float` | `24.0f` | Radius of the avatar circle (diameter is `2 * radius`). |
| `initials` | `std::string` | `""` | 1 or 2 letter initials to display if no image is available. |
| `image_path` | `std::string` | `""` | Filepath or asset path to user's photo. |
| `background_color` | `Color` | `0xFF38BDF8` | Background fill behind initials. |
| `text_color` | `Color` | `0xFFFFFFFF` | Font color of the initials text. |
| `border_width` | `float` | `0.0f` | Thickness of circular border outline. |
| `border_color` | `Color` | `0xFFFFFFFF` | Color of circular border outline. |
| `show_badge` | `bool` | `false` | Displays a live status badge dot at bottom-right. |
| `badge_color` | `Color` | `0xFF10B981` | Color of status badge (e.g. green for online, amber for away). |
| `badge_size` | `float` | `12.0f` | Diameter of the status badge dot. |
| `shadow_blur` | `float` | `0.0f` | Drop shadow blur radius for elevation. |

---

## Code Examples (From `widgets_demo/avatar_demo/main.cpp`)

### 1. Initials Avatar with Online Badge
```cpp
#include "enki/widgets/avatar.hpp"

using namespace enki;

WidgetPtr buildMemberAvatar() {
    return Avatar {
        .radius = 28.0f,
        .initials = "EJ",
        .background_color = 0xFF8B5CF6, // Purple
        .show_badge = true,
        .badge_color = 0xFF10B981,      // Online green
    };
}
```

### 2. Photo Avatar with Border and Elevation
```cpp
auto profileAvatar = Avatar {
    .radius = 36.0f,
    .image_path = "assets/user_photo.png",
    .border_width = 2.5f,
    .border_color = 0xFF38BDF8,
    .shadow_blur = 8.0f,
};
```

### 3. Overlapping Avatar Group (Team List)
```cpp
auto teamStack = AvatarGroup {
    .spacing = -14.0f,
    .max_avatars = 5,
    .avatars = {
        Avatar { .initials = "AL", .background_color = 0xFFEC4899 },
        Avatar { .initials = "BK", .background_color = 0xFF3B82F6 },
        Avatar { .initials = "CM", .background_color = 0xFF10B981 },
        Avatar { .initials = "DN", .background_color = 0xFFF59E0B },
    }
};
```

---

## See Also
- [**Badge**](./badge.md) — Universal notification and counter badge widget.
- [**Image**](./image.md) — Raw image loading and scaling primitive.
