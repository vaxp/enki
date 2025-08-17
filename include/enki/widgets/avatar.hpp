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
// Avatar Options
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
// Avatar Widget
// ════════════════════════════════════════════════════════════════

class Avatar : public StatelessWidget {
public:
    AvatarProps options;

    Avatar(AvatarProps opt = AvatarProps()) : options(std::move(opt)) {}
    explicit Avatar(std::string initials, AvatarProps opt = AvatarProps()) : options(std::move(opt)) {
        options.initials = std::move(initials);
    }
    Avatar(Key k, AvatarProps opt) : StatelessWidget(std::move(k)), options(std::move(opt)) {}
    
    [[nodiscard]] std::string_view typeName() const override { return "Avatar"; }
    WidgetPtr build(BuildContext& ctx) override;
};

// ════════════════════════════════════════════════════════════════
// AvatarGroup Widget
// ════════════════════════════════════════════════════════════════

struct AvatarGroupProps {
    Key key = Key::none();
    std::vector<WidgetPtr> avatars;
    float                  spacing = -10.0f; // Negative spacing makes them overlap
    size_t                 max_avatars = 4;
};

class AvatarGroup : public StatelessWidget {
public:
    std::vector<WidgetPtr> avatars;
    float                  spacing = -10.0f; // Negative spacing makes them overlap
    size_t                 max_avatars = 4;
    
    AvatarGroup(std::vector<WidgetPtr> avatars, float spacing = -12.0f, size_t max_avatars = 4)
        : avatars(std::move(avatars)), spacing(spacing), max_avatars(max_avatars) {}
    
    AvatarGroup(Key k, std::vector<WidgetPtr> avatars, float spacing, size_t max_avatars)
        : StatelessWidget(std::move(k)), avatars(std::move(avatars)), spacing(spacing), max_avatars(max_avatars) {}

    [[nodiscard]] std::string_view typeName() const override { return "AvatarGroup"; }
    WidgetPtr build(BuildContext& ctx) override;
};

// ════════════════════════════════════════════════════════════════
// Fluent Factories
// ════════════════════════════════════════════════════════════════

inline std::shared_ptr<Avatar> avatar(AvatarProps options = AvatarProps()) {
    return std::make_shared<Avatar>(options.key, std::move(options));
}

inline std::shared_ptr<Avatar> avatar(std::string initials, AvatarProps options = AvatarProps()) {
    options.initials = std::move(initials);
    return std::make_shared<Avatar>(options.key, std::move(options));
}

inline std::shared_ptr<AvatarGroup> avatarGroup(std::vector<WidgetPtr> avatars, float spacing = -12.0f, size_t max = 4) {
    return std::make_shared<AvatarGroup>(std::move(avatars), spacing, max);
}

inline std::shared_ptr<AvatarGroup> avatarGroup(std::initializer_list<WidgetPtr> avatars, float spacing = -12.0f, size_t max = 4) {
    return std::make_shared<AvatarGroup>(std::vector<WidgetPtr>(avatars), spacing, max);
}

inline std::shared_ptr<AvatarGroup> avatarGroup(AvatarGroupProps props) {
    return std::make_shared<AvatarGroup>(std::move(props.key), std::move(props.avatars), props.spacing, props.max_avatars);
}

} // namespace enki
