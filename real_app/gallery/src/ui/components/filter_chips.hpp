#pragma once
/// @file filter_chips.hpp
/// @brief Folder filter chips for gallery categorisation.

#include "ui/palette.hpp"
#include "ui/helpers.hpp"
#include "state/gallery_state.hpp"
#include "state/gallery_cubit.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/i18n/i18n.hpp"
#include "services/gallery_i18n.hpp"
#include <vector>
#include <string>
#include <algorithm>

namespace enki::gallery {

inline WidgetPtr buildFolderFilterChips(const GalleryState& state, GalleryCubit* cubit) {
    if (state.photos.empty()) return nullptr;

    std::vector<std::string> folders = { "All", "Favorites" };
    for (const auto& p : state.photos) {
        if (std::find(folders.begin(), folders.end(), p.folder_name) == folders.end()) {
            folders.push_back(p.folder_name);
        }
    }

    std::vector<WidgetPtr> chip_list;
    for (const auto& folder : folders) {
        bool active = (state.selected_folder == folder);

        std::string display_label = localizeFolderName(folder);

        auto chip = makeClickable(
            makeBox(
                text(display_label, {
                    .color = active ? Palette::text_white : Palette::text_muted,
                    .font_size = 13.0f,
                    .font_weight = active ? FontWeight::Bold : FontWeight::Normal
                }),
                active ? Palette::primary : Palette::bg_card,
                BorderRadius::circular(20.0f),
                Border(active ? Palette::primary_neon : Palette::border_subtle, 1.0f),
                StyleInsets::symmetric(6.0f, 14.0f)
            ),
            [cubit, folder]() { if (cubit) cubit->setFolder(folder); }
        );

        chip_list.push_back(chip);
        chip_list.push_back(sizedBox(8.0f, 0));
    }

    return row({
        .align_items = Align::Center,
        .children = chip_list
    });
}

} // namespace enki::gallery
