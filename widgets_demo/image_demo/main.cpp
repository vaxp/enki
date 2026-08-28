/// @file main.cpp
/// @brief ENKI Image Engine & BoxFit Interactive Showcase Demo
///
/// Features demonstrated:
///   1. Realtime BoxFit Mode Switcher (Cover, Contain, Fill, FitWidth, FitHeight, None, ScaleDown).
///   2. Multi-asset Gallery loading real PNG assets from disk (0.png, 1.png, 2.png, 12.png, vaxp.png).
///   3. Circular Clipping (BoxShape::Circle) for Profile Avatars with Status Badges.
///   4. Smooth Rounded Corners with dynamic BorderRadius.
///   5. Live Color Tinting with BlendMode and Opacity.
///   6. Responsive Image Gallery Grid.

#include "enki/app/app.hpp"
#include "enki/state/state.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/stack.hpp"
#include "enki/widgets/image.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <sstream>

using namespace enki;

// ════════════════════════════════════════════════════════════════
// Design System & Palette
// ════════════════════════════════════════════════════════════════

namespace Style {
    constexpr Color bg_dark       = 0xFF0A0E1A;
    constexpr Color bg_card       = 0xFF141B2D;
    constexpr Color bg_card_light = 0xFF1E293B;
    constexpr Color border_subtle = 0x3038BDF8;
    constexpr Color border_bright = 0x9038BDF8;
    constexpr Color primary       = 0xFF6366F1;
    constexpr Color primary_light = 0xFF818CF8;
    constexpr Color cyan_neon     = 0xFF00E5FF;
    constexpr Color purple_neon   = 0xFFC084FC;
    constexpr Color emerald       = 0xFF10B981;
    constexpr Color rose          = 0xFFF43F5E;
    constexpr Color amber         = 0xFFF59E0B;
    constexpr Color text_white    = 0xFFFFFFFF;
    constexpr Color text_muted    = 0xFF94A3B8;
}

// ════════════════════════════════════════════════════════════════
// Image Demo State & App
// ════════════════════════════════════════════════════════════════

class ImageDemoApp : public StatefulWidget {
public:
    std::string_view typeName() const override { return "ImageDemoApp"; }
    std::unique_ptr<State> createState() override;
};

class ImageDemoState : public State {
public:
    BoxFit current_fit = BoxFit::Cover;
    std::string current_image = "assets/0.png";
    BoxShape current_shape = BoxShape::Rectangle;
    float current_radius = 20.0f;
    float current_opacity = 1.0f;
    bool enable_tint = false;
    Color tint_color = 0x5500E5FF; // Soft cyan tint

    // Asset list
    const std::vector<std::string> assets = {
        "assets/0.png",
        "assets/1.png",
        "assets/2.png",
        "assets/12.png",
        "assets/vaxp.png"
    };

    WidgetPtr build(BuildContext& context) override {
        WidgetPtr header = buildHeader();
        WidgetPtr ctrl = buildControlPanel();
        WidgetPtr hero = buildHeroPreviewCard();
        WidgetPtr gall = buildGalleryAndAvatarsPanel();

        auto body_row = row({
            .align_items = Align::Start,
            .children = {
                ctrl,
                sizedBox(14.0f, 0),
                hero,
                sizedBox(14.0f, 0),
                gall,
            }
        });

        auto content = container({
            .color = Style::bg_dark,
            .width = StyleValue::point(1080.0f),
            .height = StyleValue::point(660.0f),
            .padding = StyleInsets::all(18.0f),
            .child = column({
                .children = {
                    header,
                    sizedBox(0, 14.0f),
                    body_row,
                }
            }),
        });

        return content;
    }

private:
    // ── 1. Top Header ──────────────────────────────────────────

    WidgetPtr buildHeader() {
        auto title_t = text({
            .text = "ENKI ⚡ Image Engine & BoxFit Showcase",
            .color = Style::text_white,
            .font_size = 22.0f,
            .font_weight = FontWeight::Bold,
        });

        auto subtitle_t = text({
            .text = "Hardware-Accelerated Skia Pipeline • Zero-Copy ImageCache • Realtime Geometric Clipping",
            .color = Style::text_muted,
            .font_size = 12.0f,
        });

        WidgetPtr badge1 = buildBadge("Skia GL Pipeline", Style::cyan_neon);
        WidgetPtr badge2 = buildBadge("Thread-safe LRU Cache", Style::primary_light);
        WidgetPtr badge3 = buildBadge("Instant BoxFit Math", Style::emerald);

        auto badges_row = row({
            .children = {
                badge1,
                sizedBox(8.0f, 0),
                badge2,
                sizedBox(8.0f, 0),
                badge3,
            }
        });

        WidgetPtr title_col = column({
            .children = {
                title_t,
                sizedBox(0, 4.0f),
                subtitle_t,
            }
        });

        auto header_row = row({
            .justify_content = Justify::SpaceBetween,
            .align_items = Align::Center,
            .children = {
                title_col,
                badges_row,
            }
        });

        auto header_box = container({
            .color = Style::bg_card,
            .border_radius = BorderRadius::circular(14.0f),
            .border = Border(Style::border_subtle, 1.0f),
            .padding = StyleInsets::symmetric(14.0f, 18.0f),
            .child = header_row,
        });

        return header_box;
    }

    WidgetPtr buildBadge(const std::string& label, Color color) {
        auto t = text({
            .text = label,
            .color = color,
            .font_size = 11.0f,
            .font_weight = FontWeight::Bold,
        });

        auto b = container({
            .color = 0x20000000 | (color & 0x00FFFFFF),
            .border_radius = BorderRadius::circular(20.0f),
            .border = Border(color, 1.0f),
            .padding = StyleInsets::symmetric(4.0f, 10.0f),
            .child = t,
        });
        return b;
    }

    // ── 2. Left Control Panel ──────────────────────────────────

    WidgetPtr buildControlPanel() {
        auto title = text({
            .text = "⚙️ BoxFit Switcher",
            .color = Style::text_white,
            .font_size = 14.0f,
            .font_weight = FontWeight::Bold,
        });

        std::vector<WidgetPtr> fit_buttons;
        fit_buttons.push_back(title);
        fit_buttons.push_back(sizedBox(0, 10.0f));

        const std::vector<std::pair<BoxFit, std::string>> fits = {
            {BoxFit::Cover, "Cover (Fill & Crop)"},
            {BoxFit::Contain, "Contain (Letterbox)"},
            {BoxFit::Fill, "Fill (Stretch XY)"},
            {BoxFit::FitWidth, "FitWidth (Span X)"},
            {BoxFit::FitHeight, "FitHeight (Span Y)"},
            {BoxFit::None, "None (1:1 Native)"},
            {BoxFit::ScaleDown, "ScaleDown (Smart)"}
        };

        for (const auto& [fit_mode, label] : fits) {
            bool active = (current_fit == fit_mode);
            auto btn = buildInteractiveButton(label, active, [this, fit_mode]() {
                setState([this, fit_mode]() {
                    current_fit = fit_mode;
                });
            });
            fit_buttons.push_back(btn);
            fit_buttons.push_back(sizedBox(0, 6.0f));
        }

        fit_buttons.push_back(sizedBox(0, 8.0f));
        auto img_title = text({
            .text = "🖼️ Source Asset",
            .color = Style::text_white,
            .font_size = 14.0f,
            .font_weight = FontWeight::Bold,
        });
        fit_buttons.push_back(img_title);
        fit_buttons.push_back(sizedBox(0, 8.0f));

        // Asset selector wrap layout
        std::vector<WidgetPtr> asset_btns;
        for (size_t i = 0; i < assets.size(); ++i) {
            bool active = (current_image == assets[i]);
            std::string short_name = "Img " + std::to_string(i + 1);
            if (assets[i].find("vaxp") != std::string::npos) short_name = "Logo";

            auto t = text({
                .text = short_name,
                .color = active ? Style::text_white : Style::text_muted,
                .font_size = 11.0f,
                .font_weight = FontWeight::Bold,
            });

            auto b = container({
                .color = active ? Style::primary : Style::bg_card_light,
                .border_radius = BorderRadius::circular(8.0f),
                .border = Border(active ? Style::cyan_neon : Style::border_subtle, 1.0f),
                .padding = StyleInsets::symmetric(6.0f, 10.0f),
                .child = t,
            });

            auto gd = gestureDetector({
                .child = b,
                .on_tap = [this, idx = i]() {
                    setState([this, idx]() {
                        current_image = assets[idx];
                    });
                },
            });
            asset_btns.push_back(gd);
        }

        auto asset_wrap = wrap({
            .row_gap = StyleValue::point(6.0f),
            .column_gap = StyleValue::point(6.0f),
            .children = std::move(asset_btns),
        });
        fit_buttons.push_back(asset_wrap);

        auto panel = container({
            .color = Style::bg_card,
            .border_radius = BorderRadius::circular(14.0f),
            .border = Border(Style::border_subtle, 1.0f),
            .width = StyleValue::point(240.0f),
            .height = StyleValue::point(540.0f),
            .padding = StyleInsets::all(14.0f),
            .child = column({
                .children = std::move(fit_buttons)
            }),
        });
        return panel;
    }

    WidgetPtr buildInteractiveButton(const std::string& label, bool active, std::function<void()> onClick) {
        auto t = text({
            .text = label,
            .color = active ? Style::text_white : Style::text_muted,
            .font_size = 12.0f,
            .font_weight = FontWeight::Bold,
        });

        auto btn = container({
            .color = active ? Style::primary : Style::bg_card_light,
            .border_radius = BorderRadius::circular(8.0f),
            .border = Border(active ? Style::cyan_neon : Style::border_subtle, 1.0f),
            .padding = StyleInsets::symmetric(8.0f, 12.0f),
            .child = t,
        });

        return gestureDetector({
            .child = btn,
            .on_tap = std::move(onClick),
        });
    }

    // ── 3. Center Hero Preview Card ────────────────────────────

    WidgetPtr buildHeroPreviewCard() {
        auto img = image({
            .source_path = current_image,
            .fit = current_fit,
            .alignment = Alignment::Center,
            .border_radius = BorderRadius::circular(current_radius),
            .shape = current_shape,
            .tint_color = enable_tint ? std::optional<Color>(tint_color) : std::nullopt,
            .blend_mode = BlendMode::SrcIn,
            .opacity = current_opacity,
        });

        auto img_holder = container({
            .color = 0xFF0F1422,
            .border_radius = BorderRadius::circular(current_radius),
            .border = Border(Style::border_bright, 1.5f),
            .width = StyleValue::point(452.0f),
            .height = StyleValue::point(320.0f),
            .child = img,
        });

        std::string fit_str = "Cover";
        if (current_fit == BoxFit::Contain) fit_str = "Contain";
        else if (current_fit == BoxFit::Fill) fit_str = "Fill";
        else if (current_fit == BoxFit::FitWidth) fit_str = "FitWidth";
        else if (current_fit == BoxFit::FitHeight) fit_str = "FitHeight";
        else if (current_fit == BoxFit::None) fit_str = "None (1:1)";
        else if (current_fit == BoxFit::ScaleDown) fit_str = "ScaleDown";

        WidgetPtr fit_chip = buildBadge("Mode: " + fit_str, Style::cyan_neon);
        WidgetPtr asset_chip = buildBadge(current_image, Style::primary_light);

        auto chips_row = row({
            .children = { fit_chip, sizedBox(8.0f, 0), asset_chip }
        });

        WidgetPtr shape_rect_btn = buildInteractiveButton("Rectangle", current_shape == BoxShape::Rectangle, [this]() {
            setState([this]() {
                current_shape = BoxShape::Rectangle;
                current_radius = 20.0f;
            });
        });

        WidgetPtr shape_circle_btn = buildInteractiveButton("Circle Clip", current_shape == BoxShape::Circle, [this]() {
            setState([this]() {
                current_shape = BoxShape::Circle;
            });
        });

        WidgetPtr tint_toggle_btn = buildInteractiveButton(enable_tint ? "Tint: ON" : "Tint: OFF", enable_tint, [this]() {
            setState([this]() {
                enable_tint = !enable_tint;
            });
        });

        WidgetPtr opacity_toggle_btn = buildInteractiveButton(current_opacity < 1.0f ? "Alpha: 60%" : "Alpha: 100%", current_opacity < 1.0f, [this]() {
            setState([this]() {
                current_opacity = (current_opacity < 1.0f) ? 1.0f : 0.60f;
            });
        });

        auto toggles_row = row({
            .children = {
                shape_rect_btn,
                sizedBox(6.0f, 0),
                shape_circle_btn,
                sizedBox(6.0f, 0),
                tint_toggle_btn,
                sizedBox(6.0f, 0),
                opacity_toggle_btn,
            }
        });

        auto card = container({
            .color = Style::bg_card,
            .border_radius = BorderRadius::circular(14.0f),
            .border = Border(Style::border_subtle, 1.0f),
            .width = StyleValue::point(480.0f),
            .height = StyleValue::point(540.0f),
            .padding = StyleInsets::all(14.0f),
            .child = column({
                .children = {
                    chips_row,
                    sizedBox(0, 10.0f),
                    img_holder,
                    sizedBox(0, 14.0f),
                    toggles_row,
                }
            }),
        });

        return card;
    }

    // ── 4. Right Panel: Avatars & Gallery Grid ─────────────────

    WidgetPtr buildGalleryAndAvatarsPanel() {
        auto title1 = text({
            .text = "👤 Circular Avatars (Stack)",
            .color = Style::text_white,
            .font_size = 13.0f,
            .font_weight = FontWeight::Bold,
        });

        auto avatar1 = image({
            .source_path = "assets/1.png",
            .width = StyleValue::point(52.0f),
            .height = StyleValue::point(52.0f),
            .fit = BoxFit::Cover,
            .shape = BoxShape::Circle,
        });
        auto avatar1_box = container({
            .border_radius = BorderRadius::circular(26.0f),
            .border = Border(Style::primary_light, 2.0f),
            .child = avatar1,
        });

        auto online_dot = container({
            .color = Style::emerald,
            .border_radius = BorderRadius::circular(6.0f),
            .border = Border(0xFF0A0E1A, 2.0f),
            .width = StyleValue::point(12.0f),
            .height = StyleValue::point(12.0f),
        });

        auto pos_dot = Positioned {
            .child = online_dot,
            .right = StyleValue::point(0.0f),
            .bottom = StyleValue::point(0.0f),
        };

        auto av1_stack = Stack {
            .width = StyleValue::point(52.0f),
            .height = StyleValue::point(52.0f),
            .children = {
                avatar1_box,
                pos_dot
            }
        };

        auto avatar2 = image({
            .source_path = "assets/2.png",
            .width = StyleValue::point(52.0f),
            .height = StyleValue::point(52.0f),
            .fit = BoxFit::Cover,
            .shape = BoxShape::Circle,
        });
        auto avatar2_box = container({
            .border_radius = BorderRadius::circular(26.0f),
            .border = Border(Style::purple_neon, 2.0f),
            .child = avatar2,
        });

        auto avatar3 = image({
            .source_path = "assets/12.png",
            .width = StyleValue::point(52.0f),
            .height = StyleValue::point(52.0f),
            .fit = BoxFit::Cover,
            .shape = BoxShape::Circle,
        });
        auto avatar3_box = container({
            .border_radius = BorderRadius::circular(26.0f),
            .border = Border(Style::cyan_neon, 2.0f),
            .child = avatar3,
        });

        WidgetPtr w_av1 = av1_stack;
        WidgetPtr w_av2 = avatar2_box;
        WidgetPtr w_av3 = avatar3_box;

        auto avatars_row = row({
            .children = {
                w_av1,
                sizedBox(12.0f, 0),
                w_av2,
                sizedBox(12.0f, 0),
                w_av3,
            }
        });

        auto title2 = text({
            .text = "🎨 Multi-Asset Responsive Grid",
            .color = Style::text_white,
            .font_size = 13.0f,
            .font_weight = FontWeight::Bold,
        });

        WidgetPtr card1 = buildGalleryCard("assets/1.png", "Neon Cyber", Style::cyan_neon);
        WidgetPtr card2 = buildGalleryCard("assets/2.png", "Cosmic Core", Style::purple_neon);
        WidgetPtr card3 = buildGalleryCard("assets/3.png", "Digital Void", Style::amber);
        WidgetPtr card4 = buildGalleryCard("assets/12.png", "Aura Engine", Style::emerald);

        auto grid_row1 = row({
            .children = { card1, sizedBox(10.0f, 0), card2 }
        });
        auto grid_row2 = row({
            .children = { card3, sizedBox(10.0f, 0), card4 }
        });

        WidgetPtr w_title1 = title1;
        WidgetPtr w_title2 = title2;
        WidgetPtr w_avatars_row = avatars_row;
        WidgetPtr w_grid1 = grid_row1;
        WidgetPtr w_grid2 = grid_row2;

        auto panel = container({
            .color = Style::bg_card,
            .border_radius = BorderRadius::circular(14.0f),
            .border = Border(Style::border_subtle, 1.0f),
            .width = StyleValue::point(290.0f),
            .height = StyleValue::point(540.0f),
            .padding = StyleInsets::all(14.0f),
            .child = column({
                .children = {
                    w_title1,
                    sizedBox(0, 10.0f),
                    w_avatars_row,
                    sizedBox(0, 18.0f),
                    w_title2,
                    sizedBox(0, 10.0f),
                    w_grid1,
                    sizedBox(0, 8.0f),
                    w_grid2,
                }
            }),
        });

        return panel;
    }

    WidgetPtr buildGalleryCard(const std::string& asset_path, const std::string& label, Color accent) {
        auto img = image({
            .source_path = asset_path,
            .width = StyleValue::point(114.0f),
            .height = StyleValue::point(68.0f),
            .fit = BoxFit::Cover,
            .border_radius = BorderRadius::circular(8.0f),
        });

        auto t = text({
            .text = label,
            .color = Style::text_white,
            .font_size = 10.0f,
            .font_weight = FontWeight::Bold,
        });

        WidgetPtr w_img = img;
        WidgetPtr w_t = t;

        auto card = container({
            .color = Style::bg_card_light,
            .border_radius = BorderRadius::circular(10.0f),
            .border = Border(accent, 1.0f),
            .width = StyleValue::point(126.0f),
            .height = StyleValue::point(106.0f),
            .padding = StyleInsets::all(6.0f),
            .child = column({
                .children = {
                    w_img,
                    sizedBox(0, 4.0f),
                    w_t,
                }
            }),
        });

        return gestureDetector({
            .child = card,
            .on_tap = [this, asset_path]() {
                setState([this, asset_path]() {
                    current_image = asset_path;
                });
            },
        });
    }
};

std::unique_ptr<State> ImageDemoApp::createState() {
    return std::make_unique<ImageDemoState>();
}

// ════════════════════════════════════════════════════════════════
// Main Entrypoint
// ════════════════════════════════════════════════════════════════

int main(int argc, char** argv) {
    AppConfig config;
    config.title                    = "ENKI — Image & BoxFit Interactive Showcase";
    config.width                    = 1080;
    config.height                   = 660;
    config.window_mode              = WindowMode::Normal;
    config.vsync                    = false;
    config.target_fps               = 0;
    config.show_performance_overlay = true;
    config.clear_color              = Style::bg_dark;

    return runApp(std::make_shared<ImageDemoApp>(), config);
}
