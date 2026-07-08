#define _CRT_SECURE_NO_WARNINGS
/// @file main.cpp
/// @brief ENKI Real Photo Gallery — Entry Point & Responsive App Scaffold.

#include "enki/app/app.hpp"
#include "enki/platform/platform.hpp"
#include "enki/state/bloc_provider.hpp"
#include "enki/state/bloc_builder.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/scroll_view.hpp"
#include "enki/widgets/window_frame.hpp"
#include "enki/widgets/titlebar.hpp"

#include "state/gallery_cubit.hpp"
#include "ui/palette.hpp"
#include "ui/helpers.hpp"
#include "ui/components/app_bar.hpp"
#include "ui/components/permission_card.hpp"
#include "ui/components/filter_chips.hpp"
#include "ui/components/photo_grid.hpp"
#include "ui/views/fullscreen_viewer.hpp"

#include "services/gallery_i18n.hpp"
#include "enki/i18n/i18n.hpp"
#include "enki/i18n/locale.hpp"
#include "enki/state/state.hpp"

#include <iostream>
#include <memory>
#include <vector>

using namespace enki;
using namespace enki::gallery;

WidgetPtr buildGalleryUI(const GalleryState& state, GalleryCubit* cubit) {
    // ── 1. Fullscreen Viewer Mode ─────────────────────────────────
    // When a photo is opened, display the immersive viewer directly
    if (state.active_fullscreen_index.has_value()) {
        auto viewer = buildFullscreenViewer(state, cubit);
        if (viewer) return viewer;
    }

    // ── 2. Main Gallery Scrollable Grid Body ──────────────────────
    std::vector<WidgetPtr> content_items;
    content_items.push_back(buildAppBar(state, cubit));
    content_items.push_back(sizedBox(0, 12.0f));
    content_items.push_back(buildPermissionStatusCard(state, cubit));
    content_items.push_back(sizedBox(0, 14.0f));

    auto chips = buildFolderFilterChips(state, cubit);
    if (chips) {
        content_items.push_back(chips);
        content_items.push_back(sizedBox(0, 14.0f));
    }

    content_items.push_back(buildPhotoGrid(state, cubit));
    content_items.push_back(sizedBox(0, 30.0f));

    auto scrollable_content = scrollView({
        .child = makeBox(
            column({ .children = std::move(content_items) }),
            std::nullopt,
            std::nullopt,
            std::nullopt,
            StyleInsets::all(14.0f)
        )
    });

    return makeBox(
        scrollable_content,
        Palette::bg_dark,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        100_pct,
        100_pct
    );
}

class GalleryPage : public StatelessWidget {
public:
    WidgetPtr build(BuildContext& ctx) override {
        auto* cubit = BlocProvider<GalleryCubit>::tryOf(ctx);
        return bloc_builder<GalleryCubit>(
            [cubit](BuildContext&, const GalleryState& state) {
                return buildGalleryUI(state, cubit);
            }
        );
    }
    std::string_view typeName() const override { return "GalleryPage"; }
};

class GalleryAppState : public State {
    std::shared_ptr<GalleryCubit> cubit_;
    SlotId locale_sub_ = 0;

public:
    void initState() override {
        State::initState();
        cubit_ = std::make_shared<GalleryCubit>();
        locale_sub_ = I18n::onLocaleChanged().connect([this](const Locale&) {
            setState([]{});
        });
    }

    void dispose() override {
        if (locale_sub_ != 0) {
            I18n::onLocaleChanged().disconnect(locale_sub_);
            locale_sub_ = 0;
        }
        State::dispose();
    }

    WidgetPtr build(BuildContext&) override {
        auto page = bloc_provider_value<GalleryCubit>(
            cubit_,
            std::make_shared<GalleryPage>()
        );

        return windowFrame(WindowFrameProps{
            .content = page,
            .title = std::string(tr("gallery.window_title")),
            .border_radius = 12.0f,
            .border_color = 0x4038BDF8,
            .border_width = 1.5f,
            .background_color = 0xFF0B0F19,
            .titlebar_background_color = 0xFF1E293B,
            .titlebar_inactive_background_color = 0xFF1E293B,
            .titlebar_style = TitleBarStyle::VAXPOS,
        });
    }
};

class GalleryApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<GalleryAppState>();
    }
    std::string_view typeName() const override { return "GalleryApp"; }
};

int main() {
    std::cout << "[GalleryApp] Launching ENKI Gallery App...\n";

    // ── 1. Initialize Declarative Localization ────────────────────
    initGalleryTranslations();

    // Default to Arabic (Iraq) to showcase native RTL & Arabic typography on startup:
    I18n::setLocale(Locale("ar", "IQ"));

    AppConfig config;
    config.title       = "ENKI Gallery";
    config.app_id      = "org.enki.gallery";
    config.width       = 460;
    config.height      = 840;
    config.resizable   = true;
    config.enable_csd  = true;
    config.clear_color = 0xFF0B0F19;

    return runApp(std::make_shared<GalleryApp>(), config);
}
