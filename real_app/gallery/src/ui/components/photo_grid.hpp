#pragma once
/// @file photo_grid.hpp
/// @brief Harmonious, balanced 2-column grid layout for displaying photo cards.

#include "ui/palette.hpp"
#include "ui/helpers.hpp"
#include "ui/components/photo_card.hpp"
#include "state/gallery_state.hpp"
#include "state/gallery_cubit.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include <vector>
#include <utility>

namespace enki::gallery {

inline WidgetPtr buildPhotoGrid(const GalleryState& state, GalleryCubit* cubit) {
    if (state.storage_status != PermissionStatus::Granted) {
        // Locked empty state
        return makeBox(
            column({
                .align_items = Align::Center,
                .children = {
                    text("[LOCKED]", { .color = Palette::amber, .font_size = 24.0f, .font_weight = FontWeight::Bold }),
                    sizedBox(0, 10.0f),
                    text("Storage Access Required", {
                        .color = Palette::text_white,
                        .font_size = 16.0f,
                        .font_weight = FontWeight::Bold
                    }),
                    sizedBox(0, 6.0f),
                    text("Tap 'Request Storage Access' above to scan photos on this device.", {
                        .color = Palette::text_muted,
                        .font_size = 13.0f
                    })
                }
            }),
            std::nullopt,
            BorderRadius::circular(14.0f),
            Border(Palette::border_subtle, 1.0f),
            StyleInsets::symmetric(40.0f, 20.0f)
        );
    }

    // Filter photos based on selected folder
    std::vector<std::pair<size_t, PhotoItem>> filtered;
    for (size_t i = 0; i < state.photos.size(); ++i) {
        const auto& p = state.photos[i];
        if (state.selected_folder == "All" ||
            (state.selected_folder == "Favorites" && p.is_favorite) ||
            p.folder_name == state.selected_folder)
        {
            filtered.emplace_back(i, p);
        }
    }

    if (filtered.empty()) {
        return makeBox(
            column({
                .align_items = Align::Center,
                .children = {
                    text("[EMPTY]", { .color = Palette::text_muted, .font_size = 20.0f }),
                    sizedBox(0, 8.0f),
                    text("No photos found in this directory.", {
                        .color = Palette::text_white,
                        .font_size = 14.0f,
                        .font_weight = FontWeight::Bold
                    })
                }
            }),
            std::nullopt,
            BorderRadius::circular(14.0f),
            Border(Palette::border_subtle, 1.0f),
            StyleInsets::symmetric(36.0f, 20.0f)
        );
    }

    // Two-column harmonious grid with balanced spacing via expanded flex items
    std::vector<WidgetPtr> grid_rows;
    for (size_t i = 0; i < filtered.size(); i += 2) {
        auto c1 = expanded(buildPhotoCard(filtered[i].first, filtered[i].second, cubit), 1.0f);
        if (i + 1 < filtered.size()) {
            auto c2 = expanded(buildPhotoCard(filtered[i + 1].first, filtered[i + 1].second, cubit), 1.0f);
            grid_rows.push_back(row({
                .align_items = Align::Stretch,
                .children = { c1, sizedBox(12.0f, 0), c2 }
            }));
        } else {
            // Single remaining card in the last row — balanced with an empty flex spacer
            auto placeholder = expanded(sizedBox(0, 0), 1.0f);
            grid_rows.push_back(row({
                .align_items = Align::Stretch,
                .children = { c1, sizedBox(12.0f, 0), placeholder }
            }));
        }
        grid_rows.push_back(sizedBox(0, 12.0f));
    }

    return column({
        .width = 100_pct,
        .children = grid_rows
    });
}

} // namespace enki::gallery
