#pragma once
/// @file app_bar.hpp
/// @brief Header component for ENKI Gallery.

#include "ui/palette.hpp"
#include "ui/helpers.hpp"
#include "state/gallery_state.hpp"
#include "state/gallery_cubit.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/i18n/i18n.hpp"
#include "enki/i18n/locale.hpp"

namespace enki::gallery {

inline WidgetPtr buildAppBar(const GalleryState& state, GalleryCubit* cubit) {
    auto title = text(std::string(tr("gallery.title")), {
        .color = Palette::text_white,
        .font_size = 18.0f,
        .font_weight = FontWeight::Bold
    });

    std::string count_str = trPlural("photos_count", state.photos.size());
    std::string sub_str = tr("gallery.subtitle", {{"count", count_str}});
    auto subtitle = text(sub_str, {
        .color = Palette::text_muted,
        .font_size = 12.0f
    });

    auto title_col = column({
        .children = { title, subtitle }
    });

    bool is_ar = (I18n::currentLocale().language == "ar");
    auto lang_btn = makeClickable(
        makeBox(
            text(is_ar ? "🌐 English" : "🌐 عربي", {
                .color = Palette::text_white,
                .font_size = 12.0f,
                .font_weight = FontWeight::SemiBold
            }),
            is_ar ? 0x3038BDF8 : 0x2510B981,
            BorderRadius::circular(8.0f),
            Border(is_ar ? Palette::primary_neon : Palette::emerald, 1.0f),
            StyleInsets::symmetric(6.0f, 10.0f)
        ),
        []() {
            if (I18n::currentLocale().language == "ar") {
                I18n::setLocale(Locale("en", "US"));
            } else {
                I18n::setLocale(Locale("ar", "IQ"));
            }
        }
    );

    auto refresh_btn = makeClickable(
        makeBox(
            text(std::string(tr("gallery.rescan")), {
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

    auto actions = row({
        .align_items = Align::Center,
        .children = { lang_btn, sizedBox(8.0f, 0), refresh_btn }
    });

    return makeBox(
        row({
            .justify_content = Justify::SpaceBetween,
            .align_items = Align::Center,
            .children = { title_col, actions }
        }),
        Palette::bg_card,
        BorderRadius::circular(14.0f),
        Border(Palette::border_subtle, 1.0f),
        StyleInsets::all(14.0f)
    );
}

} // namespace enki::gallery
