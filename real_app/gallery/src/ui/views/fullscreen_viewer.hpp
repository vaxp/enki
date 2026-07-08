#pragma once
/// @file fullscreen_viewer.hpp
/// @brief Fullscreen immersive viewer for original full-resolution photos.

#include "models/photo_item.hpp"
#include "ui/palette.hpp"
#include "ui/helpers.hpp"
#include "state/gallery_state.hpp"
#include "state/gallery_cubit.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/image.hpp"
#include "enki/widgets/text.hpp"
#include "enki/i18n/i18n.hpp"
#include "services/gallery_i18n.hpp"
#include <string>
#include <iostream>

namespace enki::gallery {

inline WidgetPtr buildFullscreenViewer(const GalleryState& state, GalleryCubit* cubit) {
    if (!state.active_fullscreen_index.has_value()) return nullptr;

    int idx = *state.active_fullscreen_index;
    if (idx < 0 || idx >= static_cast<int>(state.photos.size())) return nullptr;

    const auto& photo = state.photos[idx];

    // ── 1. Top Floating Header Bar ─────────────────────────────────
    auto back_btn = makeClickable(
        makeBox(
            text(std::string(tr("viewer.back")), {
                .color = Palette::text_white,
                .font_size = 14.0f,
                .font_weight = FontWeight::Bold
            }),
            Palette::bg_card_hover,
            BorderRadius::circular(20.0f),
            Border(Palette::border_bright, 1.2f),
            StyleInsets::symmetric(8.0f, 16.0f)
        ),
        [cubit]() {
            std::cout << "[FullscreenViewer] Back tapped -> closing fullscreen\n";
            if (cubit) cubit->closeFullscreen();
        }
    );

    std::string count_str = std::to_string(idx + 1) + " / " + std::to_string(state.photos.size());
    auto count_text = text(count_str, {
        .color = Palette::primary_neon,
        .font_size = 14.0f,
        .font_weight = FontWeight::Bold
    });

    std::string fav_label = photo.is_favorite ? std::string(tr("viewer.saved")) : std::string(tr("viewer.favorite"));
    auto fav_btn = makeClickable(
        makeBox(
            text(fav_label, {
                .color = photo.is_favorite ? Palette::rose : Palette::text_white,
                .font_size = 12.0f,
                .font_weight = FontWeight::Bold
            }),
            photo.is_favorite ? 0x40EF4444 : Palette::bg_card_hover,
            BorderRadius::circular(20.0f),
            Border(photo.is_favorite ? Palette::rose : Palette::border_subtle, 1.2f),
            StyleInsets::symmetric(8.0f, 14.0f)
        ),
        [cubit, idx]() {
            std::cout << "[FullscreenViewer] Favorite toggled for photo " << idx << "\n";
            if (cubit) cubit->toggleFavorite(static_cast<size_t>(idx));
        }
    );

    auto top_bar = makeBox(
        row({
            .justify_content = Justify::SpaceBetween,
            .align_items = Align::Center,
            .children = { back_btn, count_text, fav_btn }
        }),
        0xEE0B0F19,
        BorderRadius::circular(14.0f),
        Border(Palette::border_subtle, 1.0f),
        StyleInsets::symmetric(10.0f, 14.0f),
        100_pct
    );

    // ── 2. Full-Resolution Original Photo (Centered in middle of screen) ──
    auto full_image = image({
        .source_path = photo.path,
        .width = 100_pct,
        .height = 100_pct,
        .fit = BoxFit::Contain,
        .alignment = Alignment::Center
    });

    auto image_area = expanded(
        container({
            .color = 0xFF050811, // clean backdrop
            .border_radius = BorderRadius::circular(14.0f),
            .border = Border(Palette::border_subtle, 1.0f),
            .align = Alignment::Center,
            .width = 100_pct,
            .height = 100_pct,
            .padding = StyleInsets::all(8.0f),
            .child = full_image
        }),
        1.0f
    );

    // ── 3. Floating Bottom Navigation Bar ──────────────────────────
    auto prev_btn = makeClickable(
        makeBox(
            text(std::string(tr("viewer.prev")), {
                .color = Palette::text_white,
                .font_size = 13.0f,
                .font_weight = FontWeight::Bold
            }),
            Palette::bg_card_hover,
            BorderRadius::circular(16.0f),
            Border(Palette::border_subtle, 1.0f),
            StyleInsets::symmetric(8.0f, 16.0f)
        ),
        [cubit]() {
            std::cout << "[FullscreenViewer] Prev tapped\n";
            if (cubit) cubit->prevPhoto();
        }
    );

    std::string short_title = truncateString(photo.title, 20);
    auto title_display = text(short_title, {
        .color = Palette::text_white,
        .font_size = 13.0f,
        .font_weight = FontWeight::Bold
    });

    std::string short_folder = truncateString(localizeFolderName(photo.folder_name), 14);
    auto meta_display = text(short_folder + " • " + photo.file_size, {
        .color = Palette::text_muted,
        .font_size = 11.0f
    });

    auto info_center = column({
        .align_items = Align::Center,
        .children = { title_display, sizedBox(0, 2.0f), meta_display }
    });

    auto next_btn = makeClickable(
        makeBox(
            text(std::string(tr("viewer.next")), {
                .color = Palette::text_white,
                .font_size = 13.0f,
                .font_weight = FontWeight::Bold
            }),
            Palette::bg_card_hover,
            BorderRadius::circular(16.0f),
            Border(Palette::border_subtle, 1.0f),
            StyleInsets::symmetric(8.0f, 16.0f)
        ),
        [cubit]() {
            std::cout << "[FullscreenViewer] Next tapped\n";
            if (cubit) cubit->nextPhoto();
        }
    );

    auto bottom_bar = makeBox(
        row({
            .justify_content = Justify::SpaceBetween,
            .align_items = Align::Center,
            .children = { prev_btn, sizedBox(8.0f, 0), expanded(info_center, 1.0f), sizedBox(8.0f, 0), next_btn }
        }),
        0xEE0B0F19,
        BorderRadius::circular(14.0f),
        Border(Palette::border_subtle, 1.0f),
        StyleInsets::symmetric(10.0f, 14.0f),
        100_pct
    );

    // ── 4. Fullscreen Scaffold with Top & Bottom pinned ────────────
    return makeBox(
        column({
            .justify_content = Justify::SpaceBetween,
            .width = 100_pct,
            .height = 100_pct,
            .children = {
                top_bar,
                sizedBox(0, 8.0f),
                image_area,
                sizedBox(0, 8.0f),
                bottom_bar
            }
        }),
        Palette::fullscreen_bg,
        std::nullopt,
        std::nullopt,
        StyleInsets::all(14.0f),
        100_pct,
        100_pct
    );
}

} // namespace enki::gallery
