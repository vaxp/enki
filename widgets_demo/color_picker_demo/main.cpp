/// @file main.cpp
/// @brief ENKI Advanced ColorPicker Interactive Showcase Demo.

#include "enki/app/app.hpp"
#include "enki/widgets/color_picker.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/state/state.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <memory>

using namespace enki;

class ColorPickerDemoState : public State {
private:
    std::string hud_msg_ = "Adjust colors via the 2D Canvas & Hue Slider on the right, or click the Color Wells on the left to see live theme updates.";

    Color brand_color_ = 0xFF38BDF8;      // Sky 400
    Color accent_color_ = 0xFF8B5CF6;     // Violet 500
    Color surface_color_ = 0xFF1E293B;    // Slate 800

public:
    WidgetPtr build(BuildContext&) override {
        return container({
            .color = 0xFF0B1120,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(24.0f),
            .child = column({
                .align_items = Align::Center,
                .gap = StyleValue::point(20.0f),
                .children = {
                    // ── Main Page Header ──────────────────────────────────────────
                    column({
                        .align_items = Align::Center,
                        .gap = StyleValue::point(6.0f),
                        .children = {
                            text("Advanced ColorPicker & Theme Studio", {
                                .color = 0xFFFFFFFF,
                                .font_size = 22.0f,
                                .font_weight = FontWeight::Bold
                            }),
                            text("Enterprise color inspector (Category 3. Input / Forms), 2D Saturation-Value plane, Hue & Alpha sliders, HEX/RGBA/HSV formats, and Palettes", {
                                .color = 0xFF94A3B8,
                                .font_size = 13.0f
                            })
                        }
                    }),

                    // ── Side-by-Side Main Sections ────────────────────────────────
                    row({
                        .justify_content = Justify::Center,
                        .align_items = Align::Start,
                        .gap = StyleValue::point(20.0f),
                        .children = {
                            // ── Left Column: Theme Palette Settings (Input Popups) ────────
                            container({
                                .color = 0xFF0F172A,
                                .border_radius = BorderRadius::circular(12.0f),
                                .border = Border(0xFF334155, 1.0f),
                                .width = StyleValue::point(320.0f),
                                .padding = StyleInsets::all(20.0f),
                                .child = column({
                                    .gap = StyleValue::point(10.0f),
                                    .children = {
                                        text("🎨 Theme Design Tokens", { .color = 0xFF38BDF8, .font_size = 15.0f, .font_weight = FontWeight::Bold }),
                                        text("Primary Brand Color:", { .color = 0xFF94A3B8, .font_size = 11.5f, .font_weight = FontWeight::Bold }),
                                        ColorPicker {
                                            .initial_color = brand_color_,
                                            .on_color_changed = [this](Color c) {
                                                brand_color_ = c;
                                                hud_msg_ = "Primary Brand Color updated to: " + std::to_string(c);
                                                setState([] {});
                                            }
                                        },
                                        text("Accent & Glow Color:", { .color = 0xFF94A3B8, .font_size = 11.5f, .font_weight = FontWeight::Bold }),
                                        ColorPicker {
                                            .initial_color = accent_color_,
                                            .on_color_changed = [this](Color c) {
                                                accent_color_ = c;
                                                hud_msg_ = "Accent Color updated to: " + std::to_string(c);
                                                setState([] {});
                                            }
                                        }
                                    }
                                })
                            }),

                            // ── Center Column: Live Inline Color Studio ───────────────────
                            container({
                                .color = 0xFF0F172A,
                                .border_radius = BorderRadius::circular(12.0f),
                                .border = Border(0xFF334155, 1.0f),
                                .width = StyleValue::point(320.0f),
                                .padding = StyleInsets::all(20.0f),
                                .child = column({
                                    .align_items = Align::Center,
                                    .gap = StyleValue::point(10.0f),
                                    .children = {
                                        text("🖌️ Color Studio (Inline)", { .color = 0xFF10B981, .font_size = 15.0f, .font_weight = FontWeight::Bold }),
                                        ColorPicker {
                                            .mode = ColorPickerMode::Inline,
                                            .initial_color = brand_color_,
                                            .enable_alpha = true,
                                            .on_color_changed = [this](Color c) {
                                                brand_color_ = c;
                                                hud_msg_ = "Studio Color changed via 2D Canvas";
                                                setState([] {});
                                            }
                                        }
                                    }
                                })
                            }),

                            // ── Right Column: Live Dynamic Preview UI ─────────────────────
                            container({
                                .color = surface_color_,
                                .border_radius = BorderRadius::circular(12.0f),
                                .border = Border(brand_color_, 1.5f),
                                .box_shadow = { BoxShadow(brand_color_ & 0x66FFFFFF, {0.0f, 6.0f}, 20.0f) },
                                .width = StyleValue::point(320.0f),
                                .padding = StyleInsets::all(20.0f),
                                .child = column({
                                    .gap = StyleValue::point(14.0f),
                                    .children = {
                                        text("✨ Live Dynamic Theme Preview", { .color = 0xFFF59E0B, .font_size = 15.0f, .font_weight = FontWeight::Bold }),
                                        text("Enterprise Dashboard Pro", { .color = brand_color_, .font_size = 14.0f, .font_weight = FontWeight::Bold }),
                                        text("This card dynamically reacts to the selected colors, borders, and accent glow.", { .color = 0xFFCBD5E1, .font_size = 11.5f }),
                                        row({
                                            .align_items = Align::Center,
                                            .gap = StyleValue::point(10.0f),
                                            .children = {
                                                container({
                                                    .color = brand_color_,
                                                    .border_radius = BorderRadius::circular(6.0f),
                                                    .padding = StyleInsets::symmetric(8.0f, 16.0f),
                                                    .child = text("🚀 Launch Mission", { .color = 0xFFFFFFFF, .font_size = 12.0f, .font_weight = FontWeight::Bold })
                                                }),
                                                container({
                                                    .color = 0x338B5CF6,
                                                    .border_radius = BorderRadius::circular(12.0f),
                                                    .border = Border(accent_color_, 1.0f),
                                                    .padding = StyleInsets::symmetric(3.0f, 8.0f),
                                                    .child = text("PRO FEATURE", { .color = accent_color_, .font_size = 10.0f, .font_weight = FontWeight::Bold })
                                                })
                                            }
                                        })
                                    }
                                })
                            })
                        }
                    }),

                    // ── HUD / Status Box ──────────────────────────────────────────
                    container({
                        .color = 0xFF1E293B,
                        .border_radius = BorderRadius::circular(6.0f),
                        .border = Border(0xFF334155, 1.0f),
                        .width = StyleValue::point(960.0f),
                        .padding = StyleInsets::symmetric(8.0f, 16.0f),
                        .child = row({
                            .children = {
                                text("💡 " + hud_msg_, { .color = 0xFF38BDF8, .font_size = 12.5f })
                            }
                        })
                    })
                }
            })
        });
    }
};

class ColorPickerDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<ColorPickerDemoState>();
    }
    std::string_view typeName() const override { return "ColorPickerDemoApp"; }
};

int main() {
    std::cout << "====================================================\n";
    std::cout << "  ENKI Engine — Advanced ColorPicker Demo\n";
    std::cout << "====================================================\n";

    AppConfig config;
    config.title       = "Enki Engine — Advanced ColorPicker Demo";
    config.width       = 1240;
    config.height      = 760;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 60;
    config.clear_color = 0xFF0B1120;

    return runApp(std::make_shared<ColorPickerDemoApp>(), config);
}
