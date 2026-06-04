#pragma once
/// @file app_bar.hpp
/// @brief Header component for ENKI Gallery.

#include "ui/palette.hpp"
#include "ui/helpers.hpp"
#include "state/gallery_state.hpp"
#include "state/gallery_cubit.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"

namespace enki::gallery {

inline WidgetPtr buildAppBar(const GalleryState& state, GalleryCubit* cubit) {
    auto title = text("Device Gallery", {
        .color = Palette::text_white,
        .font_size = 18.0f,
        .font_weight = FontWeight::Bold
    });

    std::string sub_str = "Loaded: " + std::to_string(state.photos.size()) + " files from storage";
    auto subtitle = text(sub_str, {
        .color = Palette::text_muted,
        .font_size = 12.0f
    });

    auto title_col = column({
        .children = { title, subtitle }
    });

    auto refresh_btn = makeClickable(
        makeBox(
            text("Rescan Storage", {
                .color = Palette::primary_neon,
                .font_size = 12.0f,
                .font_weight = FontWeight::SemiBold
            }),
            Palette::bg_card_hover,
            BorderRadius::circular(8.0f),
            Border(Palette::border_subtle, 1.0f),
            StyleInsets::symmetric(6.0f, 12.0f)
        ),
        [cubit]() { if (cubit) cubit->checkAndSyncStorage(); }
    );

    return makeBox(
        row({
            .justify_content = Justify::SpaceBetween,
            .align_items = Align::Center,
            .children = { title_col, refresh_btn }
        }),
        Palette::bg_card,
        BorderRadius::circular(14.0f),
        Border(Palette::border_subtle, 1.0f),
        StyleInsets::all(14.0f)
    );
}

} // namespace enki::gallery
