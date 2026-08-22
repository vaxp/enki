#pragma once
/// @file avatar.hpp
/// @brief Avatar and AvatarGroup widgets for displaying user profiles.
///
/// Features:
///   - Automatic clipping to circles or rounded squares.
///   - Fallback to text initials if no image is provided.
///   - Image support (Network via Memory or Local Files).
///   - Badges for online status.
///   - AvatarGroup for overlapping multiple avatars.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/widgets/image.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/container.hpp"
#include <string>
#include <vector>
#include <memory>
#include <optional>

namespace enki {

// ════════════════════════════════════════════════════════════════
// Avatar Props
// ════════════════════════════════════════════════════════════════

struct AvatarProps {
    Key                  key                = Key::none();
    float                radius             = 24.0f;
    Color                background_color   = 0xFF38BDF8; // Default blue
    Color                text_color         = 0xFFFFFFFF; // Default white
    std::string          initials           = "";
    std::string          image_path         = "";         // Local or Network URL (handled by Image layer)
    std::shared_ptr<Image> image_data       = nullptr;    // Pre-loaded image data
    
    // Borders
    float                border_width       = 0.0f;
    Color                border_color       = 0xFFFFFFFF;
    
    // Status Badge
    bool                 show_badge         = false;
    Color                badge_color        = 0xFF10B981; // Default green (online)
    float                badge_size         = 12.0f;
    float                badge_border_width = 2.0f;
    Color                badge_border_color = 0xFFFFFFFF; // Usually matches app background
    
    // Shadow
    float                shadow_blur        = 0.0f;
    Color                shadow_color       = 0x40000000;
};

// ════════════════════════════════════════════════════════════════
// Avatar Implementation Widget
// ════════════════════════════════════════════════════════════════

class AvatarWidget : public StatelessWidget {
public:
    AvatarProps options;

    AvatarWidget() = default;
    explicit AvatarWidget(AvatarProps opt) : StatelessWidget(opt.key), options(std::move(opt)) {}
    AvatarWidget(Key k, AvatarProps opt) : StatelessWidget(std::move(k)), options(std::move(opt)) {}
    
    [[nodiscard]] std::string_view typeName() const override { return "Avatar"; }
    WidgetPtr build(BuildContext& ctx) override;
};

// ════════════════════════════════════════════════════════════════
// Declarative Avatar Struct (C++20 Designated Initializers)
// ════════════════════════════════════════════════════════════════

struct Avatar {
    Key                  key                = Key::none();
    float                radius             = 24.0f;
    Color                background_color   = 0xFF38BDF8; // Default blue
    Color                text_color         = 0xFFFFFFFF; // Default white
    std::string          initials           = "";
    std::string          image_path         = "";
    std::shared_ptr<Image> image_data       = nullptr;
    
    // Borders
    float                border_width       = 0.0f;
    Color                border_color       = 0xFFFFFFFF;
    
    // Status Badge
    bool                 show_badge         = false;
    Color                badge_color        = 0xFF10B981;
    float                badge_size         = 12.0f;
    float                badge_border_width = 2.0f;
    Color                badge_border_color = 0xFFFFFFFF;
    
    // Shadow
    float                shadow_blur        = 0.0f;
    Color                shadow_color       = 0x40000000;

    operator WidgetPtr() const {
        AvatarProps p;
        p.key = key;
        p.radius = radius;
        p.background_color = background_color;
        p.text_color = text_color;
        p.initials = initials;
        p.image_path = image_path;
        p.image_data = image_data;
        p.border_width = border_width;
        p.border_color = border_color;
        p.show_badge = show_badge;
        p.badge_color = badge_color;
        p.badge_size = badge_size;
        p.badge_border_width = badge_border_width;
        p.badge_border_color = badge_border_color;
        p.shadow_blur = shadow_blur;
        p.shadow_color = shadow_color;
        return std::make_shared<AvatarWidget>(key, std::move(p));
    }
};

// ════════════════════════════════════════════════════════════════
// AvatarGroup Implementation Widget
// ════════════════════════════════════════════════════════════════

class AvatarGroupWidget : public StatelessWidget {
public:
    std::vector<WidgetPtr> avatars;
    float                  spacing = -12.0f;
    size_t                 max_avatars = 4;
    
    AvatarGroupWidget() = default;
    AvatarGroupWidget(std::vector<WidgetPtr> avatars, float spacing = -12.0f, size_t max_avatars = 4)
        : avatars(std::move(avatars)), spacing(spacing), max_avatars(max_avatars) {}
    AvatarGroupWidget(Key k, std::vector<WidgetPtr> avatars, float spacing, size_t max_avatars)
        : StatelessWidget(std::move(k)), avatars(std::move(avatars)), spacing(spacing), max_avatars(max_avatars) {}

    [[nodiscard]] std::string_view typeName() const override { return "AvatarGroup"; }
    WidgetPtr build(BuildContext& ctx) override;
};

// ════════════════════════════════════════════════════════════════
// Declarative AvatarGroup Struct (C++20 Designated Initializers)
// ════════════════════════════════════════════════════════════════

struct AvatarGroup {
    Key key = Key::none();
    std::vector<WidgetPtr> avatars;
    float                  spacing = -12.0f; // Negative spacing makes them overlap
    size_t                 max_avatars = 4;

    operator WidgetPtr() const {
        return std::make_shared<AvatarGroupWidget>(key, avatars, spacing, max_avatars);
    }
};

} // namespace enki
