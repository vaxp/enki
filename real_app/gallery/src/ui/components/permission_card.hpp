#pragma once
/// @file permission_card.hpp
/// @brief Status and request card for runtime storage permission.

#include "ui/palette.hpp"
#include "ui/helpers.hpp"
#include "state/gallery_state.hpp"
#include "state/gallery_cubit.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include <vector>

namespace enki::gallery {

inline WidgetPtr buildPermissionStatusCard(const GalleryState& state, GalleryCubit* cubit) {
    Color card_bg   = Palette::bg_card;
    Color border_c  = Palette::border_subtle;
    Color badge_c   = Palette::amber;
    std::string badge_str;
    std::string icon_str;
    WidgetPtr action_btn = nullptr;

    if (state.storage_status == PermissionStatus::Granted) {
        card_bg   = 0x1810B981;
        border_c  = 0x4010B981;
        badge_c   = Palette::emerald;
        badge_str = "Storage Permission: GRANTED";
        icon_str  = "[OK]";
    } else if (state.storage_status == PermissionStatus::PermanentlyDenied) {
        card_bg   = 0x18EF4444;
        border_c  = 0x40EF4444;
        badge_c   = Palette::rose;
        badge_str = "Storage Permission: PERMANENTLY DENIED";
        icon_str  = "[X]";

        action_btn = makeClickable(
            makeBox(
                text("Open App Settings", {
                    .color = Palette::text_white,
                    .font_size = 13.0f,
                    .font_weight = FontWeight::Bold
                }),
                Palette::rose,
                BorderRadius::circular(8.0f),
                std::nullopt,
                StyleInsets::symmetric(8.0f, 16.0f)
            ),
            [cubit]() { if (cubit) cubit->openAppSettings(); }
        );
    } else {
        card_bg   = 0x18F59E0B;
        border_c  = 0x40F59E0B;
        badge_c   = Palette::amber;
        badge_str = "Storage Permission: REQUIRED";
        icon_str  = "[!]";

        action_btn = makeClickable(
            makeBox(
                text("Request Storage Access", {
                    .color = Palette::text_white,
                    .font_size = 13.0f,
                    .font_weight = FontWeight::Bold
                }),
                Palette::primary,
                BorderRadius::circular(8.0f),
                std::nullopt,
                StyleInsets::symmetric(8.0f, 16.0f)
            ),
            [cubit]() { if (cubit) cubit->requestStoragePermission(); }
        );
    }

    auto badge = makeBox(
        text(icon_str + " " + badge_str, {
            .color = badge_c,
            .font_size = 12.0f,
            .font_weight = FontWeight::Bold
        }),
        (badge_c & 0x00FFFFFF) | 0x25000000,
        BorderRadius::circular(6.0f),
        Border(badge_c, 1.0f),
        StyleInsets::symmetric(4.0f, 8.0f)
    );

    auto desc = text(state.status_message, {
        .color = Palette::text_white,
        .font_size = 13.0f
    });

    std::vector<WidgetPtr> col_items = { badge, sizedBox(0, 6.0f), desc };
    if (action_btn) {
        col_items.push_back(sizedBox(0, 10.0f));
        col_items.push_back(action_btn);
    }

    return makeBox(
        column({
            .align_items = Align::Start,
            .children = col_items
        }),
        card_bg,
        BorderRadius::circular(12.0f),
        Border(border_c, 1.2f),
        StyleInsets::all(14.0f)
    );
}

} // namespace enki::gallery
