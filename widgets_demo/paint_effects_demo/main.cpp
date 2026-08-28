#include "enki/app/app.hpp"
#include "enki/widgets/clip.hpp"
#include "enki/widgets/paint_effects.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/stack.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/scroll_view.hpp"
#include "enki/state/state.hpp"

using namespace enki;

// ── Glass Showcase Section (Isolated State) ───────────────────────────
class GlassShowcaseState : public State {
    int glass_clicks_ = 0;
public:
    WidgetPtr build(BuildContext&) override {
        auto card2_title = text("2. Frosted Glass BackdropFilter (Real-time GPU Blur Over Shapes)", {
            .color = 0xFFEC4899,
            .font_size = 15.0f,
            .font_weight = FontWeight::Bold,
        });

        // Background colorful elements to be blurred through glass
        auto blob_pink = positioned({
            .child = container({
                .color = 0xFFEC4899,
                .border_radius = BorderRadius::circular(60.0f),
                .width = StyleValue::point(120.0f),
                .height = StyleValue::point(120.0f),
            }),
            .top = StyleValue::point(10.0f),
            .left = StyleValue::point(60.0f),
        });

        auto blob_cyan = positioned({
            .child = container({
                .color = 0xFF06B6D4,
                .border_radius = BorderRadius::circular(70.0f),
                .width = StyleValue::point(140.0f),
                .height = StyleValue::point(140.0f),
            }),
            .top = StyleValue::point(30.0f),
            .left = StyleValue::point(360.0f),
        });

        auto blob_amber = positioned({
            .child = container({
                .color = 0xFFF59E0B,
                .border_radius = BorderRadius::circular(55.0f),
                .width = StyleValue::point(110.0f),
                .height = StyleValue::point(110.0f),
            }),
            .top = StyleValue::point(15.0f),
            .right = StyleValue::point(80.0f),
        });

        auto blob_emerald = positioned({
            .child = container({
                .color = 0xFF10B981,
                .border_radius = BorderRadius::circular(45.0f),
                .width = StyleValue::point(90.0f),
                .height = StyleValue::point(90.0f),
            }),
            .bottom = StyleValue::point(15.0f),
            .left = StyleValue::point(210.0f),
        });

        auto blob_purple = positioned({
            .child = container({
                .color = 0xFF8B5CF6,
                .border_radius = BorderRadius::circular(50.0f),
                .width = StyleValue::point(100.0f),
                .height = StyleValue::point(100.0f),
            }),
            .right = StyleValue::point(220.0f),
            .bottom = StyleValue::point(15.0f),
        });

        auto makeGlassCard = [](std::shared_ptr<ImageFilter> filter, Color bg_color, Color border_color, Color text_color, const std::string& title, const std::string& sub, float w = 230.0f) {
            return clipRRect({
                .border_radius = BorderRadius::circular(16.0f),
                .child = backdropFilter({
                    .filter = filter,
                    .child = container({
                        .color = bg_color,
                        .border_radius = BorderRadius::circular(16.0f),
                        .border = Border(border_color, 1.5f),
                        .align = Alignment::Center,
                        .width = StyleValue::point(w),
                        .height = StyleValue::point(90.0f),
                        .padding = StyleInsets::all(10.0f),
                        .child = column({
                            .align_items = Align::Center,
                            .gap = StyleValue::point(4.0f),
                            .children = {
                                text(title, { .color = text_color, .font_size = 14.0f, .font_weight = FontWeight::Bold }),
                                text(sub, { .color = 0xFFE2E8F0, .font_size = 12.0f }),
                            }
                        })
                    })
                })
            });
        };

        WidgetPtr glass_card_1 = makeGlassCard(
            ImageFilter::blur(6.0f, 6.0f),
            0x25FFFFFF, 0x55FFFFFF, 0xFFFFFFFF,
            "Light Glass", "Blur: 6px", 230.0f
        );

        WidgetPtr glass_card_2 = makeGlassCard(
            ImageFilter::blur(16.0f, 16.0f),
            0x251E293B, 0x8838BDF8, 0xFF38BDF8,
            "✨ Glassmorphism", "Blur: 16px (Frosted)", 250.0f
        );

        WidgetPtr glass_card_3 = makeGlassCard(
            ImageFilter::blur(28.0f, 28.0f),
            0x300F172A, 0x88C084FC, 0xFFC084FC,
            "Deep Glass", "Blur: 28px", 230.0f
        );

        auto glass_cards_row = row({
            .justify_content = Justify::SpaceAround,
            .align_items = Align::Center,
            .width = StyleValue::percent(100.0f),
            .children = { glass_card_1, glass_card_2, glass_card_3 },
        });

        auto makeGlassBtn = [this](std::shared_ptr<ImageFilter> filter, Color text_color, const std::string& emoji, const std::string& label, Color normal_c, Color hover_c) {
            return clipRRect({
                .border_radius = BorderRadius::circular(24.0f),
                .child = backdropFilter({
                    .filter = filter,
                    .child = button({
                        .child = row({
                            .align_items = Align::Center,
                            .gap = StyleValue::point(8.0f),
                            .children = {
                                text(emoji, { .font_size = 14.0f }),
                                text(label, { .color = text_color, .font_size = 13.0f, .font_weight = FontWeight::Bold }),
                            }
                        }),
                        .on_pressed = [this] {
                            setState([this]{ glass_clicks_++; });
                        },
                        .normal_color = normal_c,
                        .hover_color  = hover_c,
                        .border_radius = 24.0f,
                        .padding = EdgeInsets::symmetric(10.0f, 22.0f),
                    })
                })
            });
        };

        WidgetPtr glass_btn_1 = makeGlassBtn(
            ImageFilter::blur(12.0f, 12.0f),
            0xFFFFFFFF, "✨", "Frosted White", 0x25FFFFFF, 0x45FFFFFF
        );

        WidgetPtr glass_btn_2 = makeGlassBtn(
            ImageFilter::blur(16.0f, 16.0f),
            0xFF38BDF8, "🚀", "Glass Action (" + std::to_string(glass_clicks_) + ")", 0x3038BDF8, 0x5538BDF8
        );

        WidgetPtr glass_btn_3 = makeGlassBtn(
            ImageFilter::blur(16.0f, 16.0f),
            0xFFF472B6, "💎", "Rose Neon Blur", 0x30EC4899, 0x55EC4899
        );

        auto glass_btns_row = row({
            .justify_content = Justify::Center,
            .align_items = Align::Center,
            .gap = StyleValue::point(18.0f),
            .width = StyleValue::percent(100.0f),
            .children = { glass_btn_1, glass_btn_2, glass_btn_3 },
        });

        auto glass_content = positioned({
            .child = column({
                .justify_content = Justify::SpaceBetween,
                .align_items = Align::Center,
                .gap = StyleValue::point(18.0f),
                .width = StyleValue::percent(100.0f),
                .children = { glass_cards_row, glass_btns_row },
            }),
            .top = StyleValue::point(20.0f),
            .right = StyleValue::point(0.0f),
            .left = StyleValue::point(0.0f),
        });

        auto blur_canvas = stack({
            .children = { blob_pink, blob_cyan, blob_amber, blob_emerald, blob_purple, glass_content },
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::point(200.0f),
        });

        return container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(12.0f),
            .border = Border(0xFF334155, 1.0f),
            .width = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(16.0f),
            .child = column({
                .gap = StyleValue::point(14.0f),
                .children = { card2_title, blur_canvas },
            }),
        });
    }
};

class GlassShowcaseWidget : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override { return std::make_unique<GlassShowcaseState>(); }
    std::string_view typeName() const override { return "GlassShowcaseWidget"; }
};

// ── ColorFiltered Section (Isolated State) ───────────────────────────
class ColorFilterSectionState : public State {
    int active_filter_idx_ = 0; // 0=Original, 1=Grayscale, 2=Sepia, 3=Invert, 4=Tint
public:
    WidgetPtr build(BuildContext&) override {
        auto card3_title = text("3. ColorFiltered Dynamic Filters", {
            .color = 0xFF10B981,
            .font_size = 15.0f,
            .font_weight = FontWeight::Bold,
        });

        std::shared_ptr<ColorFilter> active_filter = nullptr;
        std::string filter_name = "None (Original)";
        if (active_filter_idx_ == 1) { active_filter = ColorFilter::grayscale(); filter_name = "Grayscale"; }
        else if (active_filter_idx_ == 2) { active_filter = ColorFilter::sepia(); filter_name = "Sepia"; }
        else if (active_filter_idx_ == 3) { active_filter = ColorFilter::invert(); filter_name = "Invert"; }
        else if (active_filter_idx_ == 4) { active_filter = ColorFilter::tint(0xFF38BDF8); filter_name = "Tint (Sky Blue)"; }

        auto filter_content = container({
            .color = 0xFF4F46E5,
            .border_radius = BorderRadius::circular(10.0f),
            .padding = StyleInsets::all(16.0f),
            .child = column({
                .align_items = Align::Center,
                .gap = StyleValue::point(8.0f),
                .children = {
                    text("🎨 Live Filter Target", { .color = 0xFFFFFFFF, .font_size = 15.0f, .font_weight = FontWeight::Bold }),
                    text("Active: " + filter_name, { .color = 0xFFFCD34D, .font_size = 13.0f }),
                    text("Applies hardware color matrix transforms to entire subtrees.", { .color = 0xFFE0E7FF, .font_size = 12.0f })
                }
            })
        });

        WidgetPtr filtered_target = colorFiltered({
            .color_filter = active_filter,
            .child = filter_content,
        });

        auto makeFilterBtn = [this](int idx, const std::string& label) -> WidgetPtr {
            bool active = (active_filter_idx_ == idx);
            return button({
                .child = text(label, { .color = 0xFFFFFFFF, .font_size = 12.0f }),
                .on_pressed = [this, idx] {
                    setState([this, idx] { active_filter_idx_ = idx; });
                },
                .normal_color = active ? 0xFF3B82F6 : 0xFF334155,
                .hover_color  = active ? 0xFF60A5FA : 0xFF475569,
                .border_radius = 8.0f,
            });
        };

        auto btn_row = row({
            .justify_content = Justify::Center,
            .gap = StyleValue::point(10.0f),
            .children = {
                makeFilterBtn(0, "Original"),
                makeFilterBtn(1, "Grayscale"),
                makeFilterBtn(2, "Sepia"),
                makeFilterBtn(3, "Invert"),
                makeFilterBtn(4, "Tint Blue"),
            }
        });

        return container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(12.0f),
            .border = Border(0xFF334155, 1.0f),
            .width = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(16.0f),
            .child = column({
                .gap = StyleValue::point(14.0f),
                .children = { card3_title, btn_row, filtered_target },
            }),
        });
    }
};

class ColorFilterSectionWidget : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override { return std::make_unique<ColorFilterSectionState>(); }
    std::string_view typeName() const override { return "ColorFilterSectionWidget"; }
};

// ── Main App ─────────────────────────────────────────────────────────
class PaintEffectsDemoState : public State {
public:
    WidgetPtr build(BuildContext&) override {
        // ── Header with ShaderMask Gradient Title ─────────────────────
        auto title_text = text("🎨 ENKI Engine — Paint & Visual Effects Showcase", {
            .color = 0xFFFFFFFF,
            .font_size = 22.0f,
            .font_weight = FontWeight::Bold,
        });

        WidgetPtr gradient_title = shaderMask({
            .shader_callback = [](Rect bounds) {
                return Gradient::linear(
                    {bounds.x, bounds.y},
                    {bounds.x + bounds.width, bounds.y},
                    {0xFF38BDF8, 0xFF818CF8, 0xFFEC4899, 0xFFF59E0B}
                );
            },
            .blend_mode = BlendMode::SrcIn,
            .child = title_text,
        });

        auto subtitle = text("Section 12: ClipRect, ClipRRect, ClipOval, ClipPath, BackdropFilter, DecoratedBox, ShaderMask, ColorFiltered", {
            .color = 0xFF94A3B8,
            .font_size = 13.0f,
        });

        auto header = column({
            .align_items = Align::Center,
            .gap = StyleValue::point(6.0f),
            .children = { gradient_title, subtitle },
        });

        // ── Card 1: Clipping Primitives (Centered Content) ─────────────
        auto card1_title = text("1. Clipping Primitives (ClipRect, ClipRRect, ClipOval, ClipPath)", {
            .color = 0xFF38BDF8,
            .font_size = 15.0f,
            .font_weight = FontWeight::Bold,
        });

        WidgetPtr box_rect = clipRect({
            .child = container({
                .color = 0xFF3B82F6,
                .align = Alignment::Center,
                .width = StyleValue::point(110.0f),
                .height = StyleValue::point(75.0f),
                .child = text("ClipRect", { .color = 0xFFFFFFFF, .font_size = 13.0f, .font_weight = FontWeight::Bold }),
            }),
        });

        WidgetPtr box_rrect = clipRRect({
            .border_radius = BorderRadius::circular(20.0f),
            .child = container({
                .color = 0xFF10B981,
                .align = Alignment::Center,
                .width = StyleValue::point(110.0f),
                .height = StyleValue::point(75.0f),
                .child = text("ClipRRect", { .color = 0xFFFFFFFF, .font_size = 13.0f, .font_weight = FontWeight::Bold }),
            }),
        });

        WidgetPtr box_oval = clipOval({
            .child = container({
                .color = 0xFF8B5CF6,
                .align = Alignment::Center,
                .width = StyleValue::point(100.0f),
                .height = StyleValue::point(75.0f),
                .child = text("ClipOval", { .color = 0xFFFFFFFF, .font_size = 13.0f, .font_weight = FontWeight::Bold }),
            }),
        });

        WidgetPtr box_path = clipPath({
            .clipper = [](Size sz) {
                Path p;
                p.moveTo(sz.width * 0.5f, 0.0f);
                p.lineTo(sz.width, sz.height * 0.5f);
                p.lineTo(sz.width * 0.5f, sz.height);
                p.lineTo(0.0f, sz.height * 0.5f);
                p.close();
                return p;
            },
            .child = container({
                .color = 0xFFF59E0B,
                .align = Alignment::Center,
                .width = StyleValue::point(100.0f),
                .height = StyleValue::point(75.0f),
                .child = text("Diamond", { .color = 0xFFFFFFFF, .font_size = 12.0f, .font_weight = FontWeight::Bold }),
            }),
        });

        auto clip_row = row({
            .justify_content = Justify::SpaceAround,
            .align_items = Align::Center,
            .width = StyleValue::percent(100.0f),
            .children = { box_rect, box_rrect, box_oval, box_path },
        });

        auto clip_card = container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(12.0f),
            .border = Border(0xFF334155, 1.0f),
            .width = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(16.0f),
            .child = column({
                .gap = StyleValue::point(14.0f),
                .children = { card1_title, clip_row },
            }),
        });

        auto main_col = column({
            .align_items = Align::Center,
            .gap = StyleValue::point(18.0f),
            .width = StyleValue::percent(100.0f),
            .children = {
                header,
                clip_card,
                std::make_shared<GlassShowcaseWidget>(),
                std::make_shared<ColorFilterSectionWidget>()
            },
        });

        return container({
            .color = 0xFF0B1120,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(24.0f),
            .child = scrollView(main_col),
        });
    }
};

class PaintEffectsDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override { return std::make_unique<PaintEffectsDemoState>(); }
    std::string_view typeName() const override { return "PaintEffectsDemoApp"; }
};

int main() {
    AppConfig config;
    config.title       = "ENKI Engine — Paint & Visual Effects Demo";
    config.width       = 960;
    config.height      = 820;
    config.target_fps  = 0;
    config.vsync       = false;
    config.show_performance_overlay = true;
    config.clear_color = 0xFF0B1120;
    return runApp(std::make_shared<PaintEffectsDemoApp>(), config);
}
