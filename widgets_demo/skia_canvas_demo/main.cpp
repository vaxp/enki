/// @file main.cpp
/// @brief Interactive Showcase Demo for ENKI SkiaCanvas Widget (Roadmap v0.2.0 - Section 16).
///
/// Demonstrates:
///   1. Direct Raw SkCanvas* painting (Radar Sweep Scanner with phosphor trail & blips)
///   2. High-level Enki Canvas painting (Dynamic Bezier Wave Visualizer with gradients)
///   3. Composite Child + Background & Foreground Painting (Cyber HUD Card)
///   4. Interactive Click / Hit-testing Canvas (Touch Ripples & Particle Rings)
///
/// @copyright ENKI Framework — MIT License

#include "enki/app/app.hpp"
#include "enki/widgets/skia_canvas.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/state/state.hpp"
#include "enki/animation/ticker.hpp"
#include "enki/animation/animation_controller.hpp"

#include <include/core/SkCanvas.h>
#include <include/core/SkPaint.h>
#include <include/core/SkPath.h>
#include <include/core/SkColor.h>

#include <cmath>
#include <vector>
#include <string>
#include <iostream>

using namespace enki;

class SkiaCanvasDemoState : public State {
private:
    std::shared_ptr<AnimationController> radar_anim_;
    std::shared_ptr<AnimationController> wave_anim_;
    std::unique_ptr<Ticker>              ticker_;

    // Interactive ripple state
    struct Ripple {
        Point center;
        float radius = 0.0f;
        float alpha  = 1.0f;
        Color color  = 0xFF38BDF8;
    };
    std::vector<Ripple> ripples_;

    float wave_frequency_ = 2.0f;
    float wave_amplitude_ = 28.0f;

public:
    void initState() override {
        State::initState();

        // 1. Radar continuous rotation controller (2.2s per sweep)
        radar_anim_ = std::make_shared<AnimationController>(std::chrono::milliseconds(2200));
        radar_anim_->setRepeats(true);
        radar_anim_->forward();

        // 2. Wave visualizer continuous phase controller (1.6s period)
        wave_anim_ = std::make_shared<AnimationController>(std::chrono::milliseconds(1600));
        wave_anim_->setRepeats(true);
        wave_anim_->forward();

        // 3. Global Ticker to advance animation controllers on every frame
        ticker_ = createTicker([this]() {
            if (radar_anim_) radar_anim_->tick();
            if (wave_anim_) wave_anim_->tick();

            // Smoothly expand and fade active ripples without triggering widget rebuilds
            if (!ripples_.empty()) {
                for (auto it = ripples_.begin(); it != ripples_.end();) {
                    it->radius += 1.4f;
                    it->alpha  -= 0.02f;
                    if (it->alpha <= 0.0f) {
                        it = ripples_.erase(it);
                    } else {
                        ++it;
                    }
                }
            }
        });
        ticker_->start();
    }

    void dispose() override {
        if (ticker_) {
            ticker_->stop();
            ticker_.reset();
        }
        if (radar_anim_) radar_anim_->dispose();
        if (wave_anim_) wave_anim_->dispose();
        State::dispose();
    }

    WidgetPtr build(BuildContext&) override {
        // ── Title & Subtitle ──────────────────────────────────────────
        auto title = text("SkiaCanvas Widget Showcase", {
            .color = 0xFFF8FAFC,
            .font_size = 24.0f,
            .font_weight = FontWeight::Bold,
        });

        auto subtitle = text("Declarative 2D Drawing Surface for ENKI (C++20 designated initializers) — Roadmap v0.2.0 Category 16", {
            .color = 0xFF94A3B8,
            .font_size = 13.0f,
        });

        auto header = column({
            .align_items = Align::Center,
            .gap = StyleValue::point(6.0f),
            .children = {title, subtitle},
        });

        // ══════════════════════════════════════════════════════════════
        // Card 1: Direct SkCanvas* Radar Scanner
        // ══════════════════════════════════════════════════════════════
        auto radar_title = text("🎯 Radar Scanner (Direct SkCanvas* API)", {
            .color = 0xFF38BDF8,
            .font_size = 14.0f,
            .font_weight = FontWeight::Bold,
        });

        auto radar_widget = skiaCanvas({
            .skia_painter = [this](SkCanvas* sk, Size size) {
                const float cx = size.width * 0.5f;
                const float cy = size.height * 0.5f;
                const float radius = std::min(cx, cy) - 10.0f;

                // 1. Background grid rings
                SkPaint ring_paint;
                ring_paint.setAntiAlias(true);
                ring_paint.setStyle(SkPaint::kStroke_Style);
                ring_paint.setStrokeWidth(1.2f);
                ring_paint.setColor(0xFF1E293B);

                for (float r = radius * 0.25f; r <= radius; r += radius * 0.25f) {
                    sk->drawCircle(cx, cy, r, ring_paint);
                }

                // Crosshairs
                ring_paint.setColor(0xFF1E3A5F);
                sk->drawLine(cx - radius, cy, cx + radius, cy, ring_paint);
                sk->drawLine(cx, cy - radius, cx, cy + radius, ring_paint);

                // 2. Rotating Phosphor Sweep Line & Fan Wedge
                float progress = radar_anim_ ? radar_anim_->value() : 0.0f;
                float angle = progress * 2.0f * 3.14159265f;

                // Glowing outer ring
                SkPaint outer_paint;
                outer_paint.setAntiAlias(true);
                outer_paint.setStyle(SkPaint::kStroke_Style);
                outer_paint.setStrokeWidth(2.5f);
                outer_paint.setColor(0xFF0284C7);
                sk->drawCircle(cx, cy, radius, outer_paint);

                // Rotating sweep ray
                float ray_x = cx + radius * std::cos(angle);
                float ray_y = cy + radius * std::sin(angle);

                SkPaint ray_paint;
                ray_paint.setAntiAlias(true);
                ray_paint.setStrokeWidth(2.0f);
                ray_paint.setColor(0xFF38BDF8);
                sk->drawLine(cx, cy, ray_x, ray_y, ray_paint);

                // Sweep fan gradient wedge (simulating phosphor trail)
                SkPath fan;
                fan.moveTo(cx, cy);
                const int steps = 18;
                for (int i = 0; i <= steps; ++i) {
                    float a = angle - (float(i) / float(steps)) * 0.75f;
                    float px = cx + radius * std::cos(a);
                    float py = cy + radius * std::sin(a);
                    fan.lineTo(px, py);
                }
                fan.close();

                SkPaint fan_paint;
                fan_paint.setAntiAlias(true);
                fan_paint.setStyle(SkPaint::kFill_Style);
                fan_paint.setColor(0x3338BDF8);
                sk->drawPath(fan, fan_paint);

                // Blip targets
                SkPaint blip_paint;
                blip_paint.setAntiAlias(true);
                blip_paint.setStyle(SkPaint::kFill_Style);
                blip_paint.setColor(0xFF22C55E);

                float blip1_x = cx + radius * 0.55f * std::cos(1.2f);
                float blip1_y = cy + radius * 0.55f * std::sin(1.2f);
                sk->drawCircle(blip1_x, blip1_y, 4.0f, blip_paint);

                float blip2_x = cx + radius * 0.80f * std::cos(3.8f);
                float blip2_y = cy + radius * 0.80f * std::sin(3.8f);
                sk->drawCircle(blip2_x, blip2_y, 3.5f, blip_paint);
            },
            .repaint = radar_anim_,
            .width = 260.0f,
            .height = 240.0f,
            .clip_behavior = Clip::AntiAlias,
            .clip_radius = BorderRadius::circular(12.0f),
        });

        auto radar_card = container({
            .color = 0xFF0F172A,
            .border_radius = BorderRadius::circular(16.0f),
            .border = Border(0xFF334155, 1.0f),
            .padding = StyleInsets::all(16.0f),
            .child = column({
                .align_items = Align::Center,
                .gap = StyleValue::point(12.0f),
                .children = {radar_title, radar_widget},
            }),
        });

        // ══════════════════════════════════════════════════════════════
        // Card 2: High-level Canvas Bezier Wave Visualizer
        // ══════════════════════════════════════════════════════════════
        auto wave_title = text("🌊 Dynamic Wave Visualizer (Enki Canvas API)", {
            .color = 0xFFA855F7,
            .font_size = 14.0f,
            .font_weight = FontWeight::Bold,
        });

        auto wave_widget = skiaCanvas({
            .painter = [this](Canvas& canvas, Size size) {
                float progress = wave_anim_ ? wave_anim_->value() : 0.0f;
                float phase = progress * 2.0f * 3.14159265f;

                Path wave_path;
                wave_path.moveTo(0.0f, size.height * 0.5f);

                const int points = 60;
                for (int i = 0; i <= points; ++i) {
                    float x = (float(i) / float(points)) * size.width;
                    float norm_x = float(i) / float(points);
                    float y = size.height * 0.5f +
                              std::sin(norm_x * wave_frequency_ * 2.0f * 3.14159265f + phase) * wave_amplitude_ +
                              std::cos(norm_x * 4.0f + phase * 1.5f) * (wave_amplitude_ * 0.3f);
                    wave_path.lineTo(x, y);
                }

                // 1. Draw glowing wave stroke
                Paint stroke_paint;
                stroke_paint.setColor(0xFFA855F7);
                stroke_paint.setStyle(PaintStyle::Stroke);
                stroke_paint.setStrokeWidth(3.0f);
                stroke_paint.setAntiAlias(true);
                canvas.drawPath(wave_path, stroke_paint);

                // 2. Filled area below wave with linear gradient
                Path area_path;
                area_path.moveTo(0.0f, size.height * 0.5f);
                for (int i = 0; i <= points; ++i) {
                    float x = (float(i) / float(points)) * size.width;
                    float norm_x = float(i) / float(points);
                    float y = size.height * 0.5f +
                              std::sin(norm_x * wave_frequency_ * 2.0f * 3.14159265f + phase) * wave_amplitude_ +
                              std::cos(norm_x * 4.0f + phase * 1.5f) * (wave_amplitude_ * 0.3f);
                    area_path.lineTo(x, y);
                }
                area_path.lineTo(size.width, size.height);
                area_path.lineTo(0.0f, size.height);
                area_path.close();

                Paint fill_paint;
                fill_paint.setStyle(PaintStyle::Fill);
                fill_paint.setAntiAlias(true);
                fill_paint.setShader(Gradient::linear(
                    Point(0.0f, size.height * 0.3f),
                    Point(0.0f, size.height),
                    {0x55A855F7, 0x00A855F7}
                ));
                canvas.drawPath(area_path, fill_paint);
            },
            .repaint = wave_anim_,
            .width = 300.0f,
            .height = 240.0f,
            .clip_behavior = Clip::AntiAlias,
            .clip_radius = BorderRadius::circular(12.0f),
        });

        auto wave_card = container({
            .color = 0xFF0F172A,
            .border_radius = BorderRadius::circular(16.0f),
            .border = Border(0xFF334155, 1.0f),
            .padding = StyleInsets::all(16.0f),
            .child = column({
                .align_items = Align::Center,
                .gap = StyleValue::point(12.0f),
                .children = {wave_title, wave_widget},
            }),
        });

        // ══════════════════════════════════════════════════════════════
        // Card 3: Composite Child + Background & Foreground Canvas
        // ══════════════════════════════════════════════════════════════
        auto comp_title = text("🛡️ HUD Card (Background & Foreground Layers)", {
            .color = 0xFFF59E0B,
            .font_size = 14.0f,
            .font_weight = FontWeight::Bold,
        });

        auto composite_card = skiaCanvas({
            // Background: Cyberpunk diagonal scan grid
            .painter = [](Canvas& canvas, Size size) {
                Paint grid_paint;
                grid_paint.setColor(0x1A38BDF8);
                grid_paint.setStrokeWidth(1.0f);
                grid_paint.setStyle(PaintStyle::Stroke);
                grid_paint.setAntiAlias(true);

                const float spacing = 18.0f;
                for (float x = 0; x < size.width + size.height; x += spacing) {
                    canvas.drawLine(Point(x, 0.0f), Point(x - size.height, size.height), grid_paint);
                }
            },
            // Foreground: Cyber HUD Corner Brackets & Reticle
            .foreground_painter = [](Canvas& canvas, Size size) {
                Paint bracket_paint;
                bracket_paint.setColor(0xFFF59E0B);
                bracket_paint.setStrokeWidth(2.5f);
                bracket_paint.setStyle(PaintStyle::Stroke);
                bracket_paint.setAntiAlias(true);

                const float len = 16.0f;
                // Top-Left
                canvas.drawLine(Point(6.0f, 6.0f), Point(6.0f + len, 6.0f), bracket_paint);
                canvas.drawLine(Point(6.0f, 6.0f), Point(6.0f, 6.0f + len), bracket_paint);

                // Top-Right
                canvas.drawLine(Point(size.width - 6.0f, 6.0f), Point(size.width - 6.0f - len, 6.0f), bracket_paint);
                canvas.drawLine(Point(size.width - 6.0f, 6.0f), Point(size.width - 6.0f, 6.0f + len), bracket_paint);

                // Bottom-Left
                canvas.drawLine(Point(6.0f, size.height - 6.0f), Point(6.0f + len, size.height - 6.0f), bracket_paint);
                canvas.drawLine(Point(6.0f, size.height - 6.0f), Point(6.0f, size.height - 6.0f - len), bracket_paint);

                // Bottom-Right
                canvas.drawLine(Point(size.width - 6.0f, size.height - 6.0f), Point(size.width - 6.0f - len, size.height - 6.0f), bracket_paint);
                canvas.drawLine(Point(size.width - 6.0f, size.height - 6.0f), Point(size.width - 6.0f, size.height - 6.0f - len), bracket_paint);
            },
            // Child: Interactive content centered inside the canvas
            .child = container({
                .padding = StyleInsets::symmetric(24.0f, 20.0f),
                .child = column({
                    .align_items = Align::Center,
                    .gap = StyleValue::point(8.0f),
                    .children = {
                        text("VAXP SECURE PROTOCOL", {
                            .color = 0xFFF59E0B,
                            .font_size = 12.0f,
                            .font_weight = FontWeight::Bold,
                        }),
                        text("QUANTUM ENGINE: ONLINE", {
                            .color = 0xFFF8FAFC,
                            .font_size = 16.0f,
                            .font_weight = FontWeight::Bold,
                        }),
                        text("Layered Skia Custom Rendering", {
                            .color = 0xFF64748B,
                            .font_size = 12.0f,
                        }),
                    }
                })
            }),
            .width = 300.0f,
            .height = 240.0f,
            .clip_behavior = Clip::AntiAlias,
            .clip_radius = BorderRadius::circular(16.0f),
        });

        auto comp_card_container = container({
            .color = 0xFF0F172A,
            .border_radius = BorderRadius::circular(16.0f),
            .border = Border(0xFF334155, 1.0f),
            .padding = StyleInsets::all(16.0f),
            .child = column({
                .align_items = Align::Center,
                .gap = StyleValue::point(12.0f),
                .children = {comp_title, composite_card},
            }),
        });

        // ══════════════════════════════════════════════════════════════
        // Card 4: Interactive Click & Ripple Canvas
        // ══════════════════════════════════════════════════════════════
        auto ripple_title = text("✨ Interactive Canvas (Click to spawn ripples)", {
            .color = 0xFF10B981,
            .font_size = 14.0f,
            .font_weight = FontWeight::Bold,
        });

        auto ripple_inner_canvas = skiaCanvas({
            .painter = [this](Canvas& canvas, Size size) {
                Paint bg;
                bg.setColor(0xFF0F172A);
                canvas.drawRect(Rect::fromLTWH(0, 0, size.width, size.height), bg);

                // Draw center prompt if empty
                if (ripples_.empty()) {
                    Paint prompt_p;
                    prompt_p.setColor(0xFF64748B);
                    float tw = canvas.measureText("Click Anywhere Here", 13.0f, nullptr, true);
                    canvas.drawText("Click Anywhere Here", Point(size.width * 0.5f - tw * 0.5f, size.height * 0.5f), prompt_p, 13.0f, nullptr, true);
                }

                // Render active ripples
                for (const auto& r : ripples_) {
                    Paint p;
                    Color c = r.color;
                    uint8_t a = static_cast<uint8_t>(std::clamp(r.alpha, 0.0f, 1.0f) * 255.0f);
                    p.setColor((static_cast<uint32_t>(a) << 24) | (c & 0x00FFFFFF));
                    p.setStyle(PaintStyle::Stroke);
                    p.setStrokeWidth(2.5f);
                    p.setAntiAlias(true);
                    canvas.drawCircle(r.center, r.radius, p);
                    canvas.drawCircle(r.center, r.radius * 0.5f, p);
                }
            },
            .repaint = wave_anim_,
            .width = 300.0f,
            .height = 240.0f,
            .clip_behavior = Clip::AntiAlias,
            .clip_radius = BorderRadius::circular(12.0f),
        });

        auto interactive_canvas = gestureDetector({
            .child = ripple_inner_canvas,
            .on_tap_down = [this](const TapDownDetails& d) {
                ripples_.push_back({
                    .center = Point(d.local_position.x, d.local_position.y),
                    .radius = 12.0f,
                    .alpha  = 1.0f,
                    .color  = (ripples_.size() % 2 == 0) ? 0xFF10B981 : 0xFF38BDF8
                });
                if (ripples_.size() > 16) ripples_.erase(ripples_.begin());
            }
        });

        auto ripple_card = container({
            .color = 0xFF0F172A,
            .border_radius = BorderRadius::circular(16.0f),
            .border = Border(0xFF334155, 1.0f),
            .padding = StyleInsets::all(16.0f),
            .child = column({
                .align_items = Align::Center,
                .gap = StyleValue::point(12.0f),
                .children = {ripple_title, interactive_canvas},
            }),
        });

        // ── Layout Grid of 4 Cards ────────────────────────────────────
        auto row_top = row({
            .justify_content = Justify::Center,
            .align_items = Align::Center,
            .gap = StyleValue::point(20.0f),
            .children = {radar_card, wave_card},
        });

        auto row_bottom = row({
            .justify_content = Justify::Center,
            .align_items = Align::Center,
            .gap = StyleValue::point(20.0f),
            .children = {comp_card_container, ripple_card},
        });

        auto content_col = column({
            .align_items = Align::Center,
            .gap = StyleValue::point(24.0f),
            .children = {header, row_top, row_bottom},
        });

        return container({
            .color = 0xFF090D16,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(24.0f),
            .child = content_col,
        });
    }
};

class SkiaCanvasDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<SkiaCanvasDemoState>();
    }
    std::string_view typeName() const override { return "SkiaCanvasDemoApp"; }
};

int main() {
    std::cout << "====================================================\n";
    std::cout << "  ENKI Engine — SkiaCanvas Showcase Demo\n";
    std::cout << "  Roadmap v0.2.0 | Section 16 Media & Canvas\n";
    std::cout << "====================================================\n";

    AppConfig config;
    config.title       = "Enki Engine — SkiaCanvas Showcase Demo";
    config.width       = 1080;
    config.height      = 780;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 0;
    config.show_performance_overlay = true;
    config.clear_color = 0xFF090D16;

    return runApp(std::make_shared<SkiaCanvasDemoApp>(), config);
}
