#pragma once
/// @file photo_card.hpp
/// @brief Grid cell rendering lightweight thumbnail, metadata and favorite action.

#include "models/photo_item.hpp"
#include "ui/palette.hpp"
#include "ui/helpers.hpp"
#include "state/gallery_cubit.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/image.hpp"
#include "enki/widgets/text.hpp"
#include "enki/i18n/i18n.hpp"
#include "services/gallery_i18n.hpp"
#include <iostream>

namespace enki::gallery {

inline WidgetPtr buildPhotoCard(size_t photo_idx, const PhotoItem& photo, GalleryCubit* cubit) {
    // 1. Grid image uses exclusively lightweight thumbnail
    WidgetPtr img_box = nullptr;
    if (!photo.thumb_path.empty()) {
        img_box = image({
            .source_path = photo.thumb_path,
            .width = 100_pct,
            .height = StyleValue::point(145.0f),
            .fit = BoxFit::Cover,
            .border_radius = BorderRadius::only(12.0f, 12.0f, 0.0f, 0.0f)
        });
    } else {
        img_box = makeBox(
            column({
                .align_items = Align::Center,
                .children = {
                    text("...", { .color = Palette::text_muted, .font_size = 16.0f, .font_weight = FontWeight::Bold })
                }
            }),
            0x251E293B,
            BorderRadius::only(12.0f, 12.0f, 0.0f, 0.0f),
            std::nullopt,
            StyleInsets::symmetric(55.0f, 10.0f),
            100_pct,
            StyleValue::point(145.0f)
        );
    }

    std::string card_title = truncateString(photo.title, 16);
    auto title = text(card_title, {
        .color = Palette::text_white,
        .font_size = 12.0f,
        .font_weight = FontWeight::Bold
    });

    std::string card_folder = truncateString(localizeFolderName(photo.folder_name), 12);
    auto meta = text(card_folder + " • " + photo.file_size, {
        .color = Palette::text_muted,
        .font_size = 11.0f
    });

    std::string fav_tag = (photo.is_favorite ? "★ " : "☆ ") + std::string(tr("card.fav"));
    auto fav_btn = makeClickable(
        makeBox(
            text(fav_tag, {
                .color = photo.is_favorite ? Palette::rose : Palette::primary_neon,
                .font_size = 11.0f,
                .font_weight = FontWeight::Bold
            }),
            photo.is_favorite ? 0x30EF4444 : 0x2038BDF8,
            BorderRadius::circular(6.0f),
            Border(photo.is_favorite ? Palette::rose : Palette::border_subtle, 1.0f),
            StyleInsets::symmetric(2.0f, 6.0f)
        ),
        [cubit, photo_idx]() {
            std::cout << "[PhotoCard] Fav button clicked for photo " << photo_idx << "\n";
            if (cubit) cubit->toggleFavorite(photo_idx);
        }
    );

    auto footer = row({
        .justify_content = Justify::SpaceBetween,
        .align_items = Align::Center,
        .children = { meta, fav_btn }
    });

    // Clicking anywhere on the photo thumbnail or title triggers openFullscreen
    auto clickable_header = makeClickable(
        column({
            .children = {
                img_box,
                makeBox(
                    column({
                        .align_items = Align::Start,
                        .children = { title, sizedBox(0, 4.0f) }
                    }),
                    std::nullopt,
                    std::nullopt,
                    std::nullopt,
                    StyleInsets::only(8.0f, 8.0f, 0.0f, 8.0f)
                )
            }
        }),
        [cubit, photo_idx]() {
            std::cout << "[PhotoCard] Clicked photo index " << photo_idx << " -> opening fullscreen viewer\n";
            if (cubit) {
                cubit->openFullscreen(static_cast<int>(photo_idx));
            }
        }
    );

    auto footer_box = makeBox(
        footer,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        StyleInsets::only(4.0f, 8.0f, 8.0f, 8.0f)
    );

    return makeBox(
        column({
            .children = { clickable_header, footer_box }
        }),
        Palette::bg_card,
        BorderRadius::circular(12.0f),
        Border(Palette::border_subtle, 1.0f),
        std::nullopt,
        100_pct
    );
}

} // namespace enki::gallery
