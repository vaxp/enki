/// @file audio_waveform.cpp
/// @brief Implementation of AudioWaveformWidget and luxurious RenderAudioWaveform visualizers.
/// @copyright ENKI Framework — MIT License

#include "enki/widgets/audio_waveform.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/rendering/canvas.hpp"
#include "enki/animation/ticker.hpp"

#include <include/core/SkCanvas.h>
#include <include/core/SkPaint.h>
#include <include/core/SkRRect.h>

#include <algorithm>
#include <cmath>
#include <deque>

namespace enki {

namespace {
constexpr float kPi = 3.14159265358979323846f;
}

// ════════════════════════════════════════════════════════════════
// RenderAudioWaveform
// ════════════════════════════════════════════════════════════════

class RenderAudioWaveform : public RenderBox {
public:
    AudioWaveformProps props_;
    std::shared_ptr<audio::AudioController> controller_;
    std::unique_ptr<Ticker> ticker_;
    std::vector<float> peak_caps_;
    std::deque<float>  scrolling_history_;
    float              pulse_rotation_ = 0.0f;
    bool               owns_controller_{false};

    explicit RenderAudioWaveform(AudioWaveformProps props)
        : props_(std::move(props)) {
        initController();
        applyStyleToNode();
        startTicker();
    }

    ~RenderAudioWaveform() override {
        if (ticker_) {
            ticker_->stop();
            ticker_.reset();
        }
        if (owns_controller_ && controller_) {
            controller_->stop();
        }
    }

    void initController() {
        if (props_.controller) {
            controller_ = props_.controller;
            owns_controller_ = false;
        } else {
            controller_ = std::make_shared<audio::AudioController>();
            owns_controller_ = true;
            if (props_.auto_start) {
                if (props_.type == AudioWaveformType::Music) {
                    controller_->startSystemAudio();
                } else {
                    controller_->startMicrophone();
                }
            }
        }
    }

    void startTicker() {
        ticker_ = createTicker([this]() {
            pulse_rotation_ += 0.015f;
            if (pulse_rotation_ > 2.0f * kPi) pulse_rotation_ -= 2.0f * kPi;
            markNeedsPaint();
        });
        ticker_->start();
    }

    void update(const AudioWaveformProps& new_props) {
        bool layout_changed = (props_.width != new_props.width ||
                               props_.height != new_props.height ||
                               props_.min_width != new_props.min_width ||
                               props_.min_height != new_props.min_height ||
                               props_.max_width != new_props.max_width ||
                               props_.max_height != new_props.max_height);

        if (props_.controller != new_props.controller || props_.type != new_props.type) {
            if (owns_controller_ && controller_) {
                controller_->stop();
            }
            props_ = new_props;
            initController();
        } else {
            props_ = new_props;
        }

        if (layout_changed) {
            applyStyleToNode();
            markNeedsLayout();
        }
        markNeedsPaint();
    }

    void applyStyleToNode() {
        if (!anu_node_) return;

        // Width
        if (props_.width.has_value()) {
            if (props_.width->isPercent()) ANUNodeStyleSetWidthPercent(anu_node_, props_.width->value);
            else if (props_.width->isAuto()) ANUNodeStyleSetWidthAuto(anu_node_);
            else ANUNodeStyleSetWidth(anu_node_, props_.width->value);
        } else {
            ANUNodeStyleSetWidthAuto(anu_node_);
        }

        // Height
        if (props_.height.has_value()) {
            if (props_.height->isPercent()) ANUNodeStyleSetHeightPercent(anu_node_, props_.height->value);
            else if (props_.height->isAuto()) ANUNodeStyleSetHeightAuto(anu_node_);
            else ANUNodeStyleSetHeight(anu_node_, props_.height->value);
        } else {
            ANUNodeStyleSetHeightAuto(anu_node_);
        }

        // Min Width & Height
        if (props_.min_width.has_value()) {
            if (props_.min_width->isPercent()) ANUNodeStyleSetMinWidthPercent(anu_node_, props_.min_width->value);
            else ANUNodeStyleSetMinWidth(anu_node_, props_.min_width->value);
        }
        if (props_.min_height.has_value()) {
            if (props_.min_height->isPercent()) ANUNodeStyleSetMinHeightPercent(anu_node_, props_.min_height->value);
            else ANUNodeStyleSetMinHeight(anu_node_, props_.min_height->value);
        }

        // Max Width & Height
        if (props_.max_width.has_value()) {
            if (props_.max_width->isPercent()) ANUNodeStyleSetMaxWidthPercent(anu_node_, props_.max_width->value);
            else ANUNodeStyleSetMaxWidth(anu_node_, props_.max_width->value);
        }
        if (props_.max_height.has_value()) {
            if (props_.max_height->isPercent()) ANUNodeStyleSetMaxHeightPercent(anu_node_, props_.max_height->value);
            else ANUNodeStyleSetMaxHeight(anu_node_, props_.max_height->value);
        }
    }

    void paint(PaintContext& context) override {
        if (size_.width <= 0.0f || size_.height <= 0.0f) return;

        const bool needs_clip = (props_.clip_behavior != Clip::None);
        if (needs_clip) {
            context.canvas.save();
            Rect bounds = Rect::fromLTWH(context.offset.x, context.offset.y, size_.width, size_.height);
            if (props_.clip_radius != BorderRadius::zero()) {
                context.canvas.clipRRect(bounds, props_.clip_radius);
            } else {
                context.canvas.clipRect(bounds);
            }
        }

        // Isolate canvas coordinates to local (0, 0)
        context.canvas.save();
        context.canvas.translate(context.offset.x, context.offset.y);

        // Fetch audio data
        std::vector<float> data;
        if (controller_ && controller_->isRunning()) {
            if (props_.style == WaveformStyle::ScrollingLive || props_.style == WaveformStyle::LiveOscilloscope) {
                data = controller_->getWaveform(props_.bands * 2);
            } else {
                data = controller_->getBands(props_.bands, props_.decay_rate, props_.sensitivity);
            }
        } else if (!props_.amplitudes.empty()) {
            data = props_.amplitudes;
        } else {
            data.resize(props_.bands, 0.06f);
        }

        switch (props_.style) {
            case WaveformStyle::SegmentedLed:
                paintSegmentedLed(context.canvas, data);
                break;
            case WaveformStyle::RadialArc:
                paintRadialArc(context.canvas, data);
                break;
            case WaveformStyle::LayeredAurora:
                paintLayeredAurora(context.canvas, data);
                break;
            case WaveformStyle::SymmetricWings:
                paintSymmetricWings(context.canvas, data);
                break;
            case WaveformStyle::SpectrumBars:
                paintSpectrumBars(context.canvas, data);
                break;
            case WaveformStyle::FluidWave:
                paintFluidWave(context.canvas, data);
                break;
            case WaveformStyle::VoiceBars:
                paintVoiceBars(context.canvas, data);
                break;
            case WaveformStyle::ScrollingLive:
                paintScrollingLive(context.canvas);
                break;
            case WaveformStyle::LiveOscilloscope:
                paintOscilloscope(context.canvas, data);
                break;
            case WaveformStyle::StereoBands:
                paintStereoBands(context.canvas, data);
                break;
        }

        context.canvas.restore();

        if (needs_clip) {
            context.canvas.restore();
        }
    }

private:
    // ════════════════════════════════════════════════════════════════
    // 1. Segmented LED Studio Equalizer with Glass Floor Reflection
    // ════════════════════════════════════════════════════════════════
    void paintSegmentedLed(Canvas& canvas, const std::vector<float>& bands) {
        const size_t count = bands.size();
        if (count == 0) return;

        const float reflection_ratio = props_.show_reflection ? 0.22f : 0.0f;
        const float usable_h = size_.height * (1.0f - reflection_ratio) - 8.0f;
        const float baseline_y = size_.height * (1.0f - reflection_ratio);

        float total_gaps = float(count - 1) * props_.bar_gap;
        float bar_w = props_.bar_width;
        if (bar_w * float(count) + total_gaps > size_.width) {
            bar_w = std::max(2.0f, (size_.width - total_gaps) / float(count));
        }

        float total_w = float(count) * bar_w + total_gaps;
        float start_x = (size_.width - total_w) * 0.5f;

        if (peak_caps_.size() != count) {
            peak_caps_.resize(count, 0.0f);
        }

        constexpr int num_segments = 16;
        const float seg_gap = 1.5f;
        const float seg_h = std::max(1.8f, (usable_h - float(num_segments - 1) * seg_gap) / float(num_segments));

        Color c_base = props_.primary_color;
        Color c_mid  = props_.secondary_color.value_or(0xFFA855F7);
        Color c_high = props_.accent_color.value_or(0xFFF43F5E);

        for (size_t i = 0; i < count; ++i) {
            float val = std::clamp(bands[i], 0.0f, 1.0f);
            int active_segs = std::max(1, static_cast<int>(val * float(num_segments)));

            float x = start_x + float(i) * (bar_w + props_.bar_gap);

            // Draw LED ladder upward from baseline
            for (int s = 0; s < active_segs; ++s) {
                float y = baseline_y - float(s + 1) * (seg_h + seg_gap);

                Paint p;
                p.setStyle(PaintStyle::Fill);
                p.setAntiAlias(true);

                // Multi-tier color threshold
                if (s >= 13) p.setColor(c_high);
                else if (s >= 8) p.setColor(c_mid);
                else p.setColor(c_base);

                canvas.drawRRect(Rect{x, y, bar_w, seg_h}, props_.bar_radius, p);
            }

            // Peak cap floating segment
            if (props_.show_peaks) {
                float current_h = float(active_segs) * (seg_h + seg_gap);
                if (current_h >= peak_caps_[i]) {
                    peak_caps_[i] = current_h;
                } else {
                    peak_caps_[i] = std::max(0.0f, peak_caps_[i] - (props_.decay_rate * 0.35f * usable_h));
                }

                float peak_y = baseline_y - peak_caps_[i] - seg_h - seg_gap;
                if (peak_y >= 0.0f) {
                    Paint peak_p;
                    peak_p.setStyle(PaintStyle::Fill);
                    peak_p.setAntiAlias(true);
                    peak_p.setColor(c_high);
                    canvas.drawRRect(Rect{x, peak_y, bar_w, seg_h}, props_.bar_radius, peak_p);
                }
            }

            // Glass floor reflection
            if (props_.show_reflection) {
                int reflect_segs = std::min(active_segs, 7);
                for (int s = 0; s < reflect_segs; ++s) {
                    float y = baseline_y + 3.0f + float(s) * (seg_h + seg_gap);
                    float alpha_factor = std::max(0.0f, 0.28f * (1.0f - (float(s) / float(reflect_segs))));

                    Paint rp;
                    rp.setStyle(PaintStyle::Fill);
                    rp.setAntiAlias(true);

                    Color base_c = (s >= 5) ? c_mid : c_base;
                    uint8_t a = static_cast<uint8_t>(alpha_factor * 255.0f);
                    rp.setColor((static_cast<uint32_t>(a) << 24) | (base_c & 0x00FFFFFF));

                    canvas.drawRRect(Rect{x, y, bar_w, seg_h}, props_.bar_radius, rp);
                }
            }
        }
    }

    // ════════════════════════════════════════════════════════════════
    // 2. Radial Arc 360° Circular Sound Reactor
    // ════════════════════════════════════════════════════════════════
    void paintRadialArc(Canvas& canvas, const std::vector<float>& bands) {
        const size_t count = bands.size();
        if (count == 0) return;

        const float cx = size_.width * 0.5f;
        const float cy = size_.height * 0.5f;
        const float bass_pulse = controller_ ? controller_->getBassEnergy() : 0.0f;

        const float inner_r = std::min(cx, cy) * 0.40f + (bass_pulse * 6.0f);
        const float max_bar_len = std::min(cx, cy) * 0.50f;

        // Center Pulsing Core Glow
        Paint core_paint;
        core_paint.setStyle(PaintStyle::Fill);
        core_paint.setAntiAlias(true);
        core_paint.setShader(Gradient::radial(
            Point(cx, cy), inner_r,
            {(props_.primary_color & 0x00FFFFFF) | 0x44000000, 0x00000000}
        ));
        canvas.drawCircle(Point(cx, cy), inner_r * 1.3f, core_paint);

        // Core Ring
        Paint ring_paint;
        ring_paint.setStyle(PaintStyle::Stroke);
        ring_paint.setStrokeWidth(2.0f);
        ring_paint.setAntiAlias(true);
        ring_paint.setColor(props_.primary_color);
        canvas.drawCircle(Point(cx, cy), inner_r, ring_paint);

        // 360-degree Radiating Spikes
        const float angle_step = (2.0f * kPi) / float(count);

        for (size_t i = 0; i < count; ++i) {
            float angle = pulse_rotation_ + float(i) * angle_step;
            float val = std::clamp(bands[i], 0.0f, 1.0f);
            float len = std::max(props_.min_bar_height, val * max_bar_len);

            float cos_a = std::cos(angle);
            float sin_a = std::sin(angle);

            Point p_start(cx + inner_r * cos_a, cy + inner_r * sin_a);
            Point p_end(cx + (inner_r + len) * cos_a, cy + (inner_r + len) * sin_a);

            Paint spike_paint;
            spike_paint.setStyle(PaintStyle::Stroke);
            spike_paint.setStrokeWidth(props_.bar_width);
            spike_paint.setAntiAlias(true);

            // Interpolate color along circle
            float ratio = float(i) / float(count);
            Color c = (ratio < 0.5f) ? props_.primary_color : props_.secondary_color.value_or(0xFFA855F7);
            spike_paint.setColor(c);

            canvas.drawLine(p_start, p_end, spike_paint);

            // Floating bead dot at tip
            if (props_.show_peaks && val > 0.3f) {
                Paint dot_paint;
                dot_paint.setStyle(PaintStyle::Fill);
                dot_paint.setAntiAlias(true);
                dot_paint.setColor(props_.accent_color.value_or(0xFFF43F5E));
                Point tip(cx + (inner_r + len + 4.0f) * cos_a, cy + (inner_r + len + 4.0f) * sin_a);
                canvas.drawCircle(tip, 2.2f, dot_paint);
            }
        }
    }

    // ════════════════════════════════════════════════════════════════
    // 3. Layered Aurora (Triple Translucent Neon Splines)
    // ════════════════════════════════════════════════════════════════
    void paintLayeredAurora(Canvas& canvas, const std::vector<float>& bands) {
        const size_t count = bands.size();
        if (count < 2) return;

        const float step_x = size_.width / float(count - 1);
        const float max_h = size_.height * 0.82f;

        // Wave Layer 1 (Deep Purple / Magenta - Back)
        drawAuroraLayer(canvas, bands, 0.70f, 0xFFA855F7, 0x22EC4899, step_x, max_h * 0.75f, 1.4f);

        // Wave Layer 2 (Electric Cyan / Blue - Mid)
        drawAuroraLayer(canvas, bands, 0.88f, 0xFF00E5FF, 0x330284C7, step_x, max_h * 0.90f, 2.0f);

        // Wave Layer 3 (Neon Teal / White Glow - Front)
        drawAuroraLayer(canvas, bands, 1.05f, 0xFF34D399, 0x4410B981, step_x, max_h, 2.8f);
    }

    void drawAuroraLayer(Canvas& canvas, const std::vector<float>& bands, float scale,
                         Color stroke_color, Color fill_color, float step_x, float max_h, float stroke_w) {
        const size_t count = bands.size();

        std::vector<Point> points;
        points.reserve(count);
        for (size_t i = 0; i < count; ++i) {
            float val = std::clamp(bands[i] * scale, 0.0f, 1.0f);
            float y = size_.height - std::max(props_.min_bar_height, val * max_h);
            points.push_back(Point(float(i) * step_x, y));
        }

        // Spline Path
        Path stroke_path;
        stroke_path.moveTo(points[0].x, points[0].y);
        for (size_t i = 0; i < count - 1; ++i) {
            Point p0 = points[i];
            Point p1 = points[i + 1];
            Point cp1(p0.x + (p1.x - p0.x) * 0.5f, p0.y);
            Point cp2(p0.x + (p1.x - p0.x) * 0.5f, p1.y);
            stroke_path.cubicTo(cp1.x, cp1.y, cp2.x, cp2.y, p1.x, p1.y);
        }

        // Fill Path
        Path fill_path;
        fill_path.moveTo(0.0f, size_.height);
        fill_path.lineTo(points[0].x, points[0].y);
        for (size_t i = 0; i < count - 1; ++i) {
            Point p0 = points[i];
            Point p1 = points[i + 1];
            Point cp1(p0.x + (p1.x - p0.x) * 0.5f, p0.y);
            Point cp2(p0.x + (p1.x - p0.x) * 0.5f, p1.y);
            fill_path.cubicTo(cp1.x, cp1.y, cp2.x, cp2.y, p1.x, p1.y);
        }
        fill_path.lineTo(size_.width, size_.height);
        fill_path.close();

        Paint fill_paint;
        fill_paint.setStyle(PaintStyle::Fill);
        fill_paint.setAntiAlias(true);
        fill_paint.setShader(Gradient::linear(
            Point(0.0f, size_.height * 0.25f),
            Point(0.0f, size_.height),
            {fill_color, 0x00000000}
        ));
        canvas.drawPath(fill_path, fill_paint);

        Paint stroke_paint;
        stroke_paint.setStyle(PaintStyle::Stroke);
        stroke_paint.setStrokeWidth(stroke_w);
        stroke_paint.setAntiAlias(true);
        stroke_paint.setColor(stroke_color);
        canvas.drawPath(stroke_path, stroke_paint);
    }

    // ════════════════════════════════════════════════════════════════
    // 4. Symmetric Cyber Wings (Center-Outward Mirrored Spectrum)
    // ════════════════════════════════════════════════════════════════
    void paintSymmetricWings(Canvas& canvas, const std::vector<float>& bands) {
        const size_t count = bands.size();
        if (count == 0) return;

        const float cx = size_.width * 0.5f;
        const float cy = size_.height * 0.5f;
        const float half_w = size_.width * 0.48f;

        const size_t wing_bars = count / 2;
        if (wing_bars == 0) return;

        float bar_w = props_.bar_width;
        float gap = props_.bar_gap;
        float total_wing = float(wing_bars) * (bar_w + gap);
        if (total_wing > half_w) {
            bar_w = std::max(2.0f, (half_w - float(wing_bars - 1) * gap) / float(wing_bars));
        }

        for (size_t i = 0; i < wing_bars; ++i) {
            // Bass in center (i=0), treble outward
            float val = std::clamp(bands[i], 0.0f, 1.0f);
            float h = std::max(props_.min_bar_height, val * (size_.height * 0.44f));

            // Right wing
            float rx = cx + 8.0f + float(i) * (bar_w + gap);
            // Left wing
            float lx = cx - 8.0f - float(i + 1) * bar_w - float(i) * gap;

            Paint p;
            p.setStyle(PaintStyle::Fill);
            p.setAntiAlias(true);
            p.setShader(Gradient::linear(
                Point(0.0f, cy - h),
                Point(0.0f, cy + h),
                {props_.secondary_color.value_or(0xFFA855F7), props_.primary_color}
            ));

            canvas.drawRRect(Rect{rx, cy - h, bar_w, h * 2.0f}, BorderRadius::circular(bar_w * 0.5f), p);
            canvas.drawRRect(Rect{lx, cy - h, bar_w, h * 2.0f}, BorderRadius::circular(bar_w * 0.5f), p);
        }
    }

    // ════════════════════════════════════════════════════════════════
    // 5. Classic Spectrum Bars
    // ════════════════════════════════════════════════════════════════
    void paintSpectrumBars(Canvas& canvas, const std::vector<float>& bands) {
        const size_t count = bands.size();
        if (count == 0) return;

        float total_gaps = float(count - 1) * props_.bar_gap;
        float bar_w = props_.bar_width;
        if (bar_w * float(count) + total_gaps > size_.width) {
            bar_w = std::max(1.5f, (size_.width - total_gaps) / float(count));
        }

        float total_w = float(count) * bar_w + total_gaps;
        float start_x = (size_.width - total_w) * 0.5f;

        if (peak_caps_.size() != count) {
            peak_caps_.resize(count, 0.0f);
        }

        const float max_h = size_.height - 6.0f;

        for (size_t i = 0; i < count; ++i) {
            float val = std::clamp(bands[i], 0.0f, 1.0f);
            float bar_h = std::max(props_.min_bar_height, val * max_h);

            float x = start_x + float(i) * (bar_w + props_.bar_gap);
            float y = size_.height - bar_h;

            Paint bar_paint;
            bar_paint.setStyle(PaintStyle::Fill);
            bar_paint.setAntiAlias(true);

            if (props_.secondary_color.has_value()) {
                bar_paint.setShader(Gradient::linear(
                    Point(x, size_.height),
                    Point(x, y),
                    {props_.primary_color, *props_.secondary_color}
                ));
            } else {
                bar_paint.setColor(props_.primary_color);
            }

            canvas.drawRRect(Rect{x, y, bar_w, bar_h}, props_.bar_radius, bar_paint);

            if (props_.show_peaks) {
                if (bar_h >= peak_caps_[i]) {
                    peak_caps_[i] = bar_h;
                } else {
                    peak_caps_[i] = std::max(0.0f, peak_caps_[i] - (props_.decay_rate * 0.35f * max_h));
                }

                float peak_y = size_.height - peak_caps_[i] - 3.0f;
                if (peak_y >= 0.0f) {
                    Paint peak_paint;
                    peak_paint.setStyle(PaintStyle::Fill);
                    peak_paint.setAntiAlias(true);
                    peak_paint.setColor(props_.secondary_color.value_or(0xFFF8FAFC));
                    canvas.drawRRect(Rect{x, peak_y, bar_w, 2.0f}, BorderRadius::circular(1.0f), peak_paint);
                }
            }
        }
    }

    // ════════════════════════════════════════════════════════════════
    // 6. Fluid Wave
    // ════════════════════════════════════════════════════════════════
    void paintFluidWave(Canvas& canvas, const std::vector<float>& bands) {
        const size_t count = bands.size();
        if (count < 2) return;

        const float step_x = size_.width / float(count - 1);
        const float max_h = size_.height * 0.85f;

        std::vector<Point> points;
        points.reserve(count);
        for (size_t i = 0; i < count; ++i) {
            float val = std::clamp(bands[i], 0.0f, 1.0f);
            float y = size_.height - std::max(props_.min_bar_height, val * max_h);
            points.push_back(Point(float(i) * step_x, y));
        }

        Path stroke_path;
        stroke_path.moveTo(points[0].x, points[0].y);
        for (size_t i = 0; i < count - 1; ++i) {
            Point p0 = points[i];
            Point p1 = points[i + 1];
            Point cp1(p0.x + (p1.x - p0.x) * 0.5f, p0.y);
            Point cp2(p0.x + (p1.x - p0.x) * 0.5f, p1.y);
            stroke_path.cubicTo(cp1.x, cp1.y, cp2.x, cp2.y, p1.x, p1.y);
        }

        Path fill_path;
        fill_path.moveTo(0.0f, size_.height);
        fill_path.lineTo(points[0].x, points[0].y);
        for (size_t i = 0; i < count - 1; ++i) {
            Point p0 = points[i];
            Point p1 = points[i + 1];
            Point cp1(p0.x + (p1.x - p0.x) * 0.5f, p0.y);
            Point cp2(p0.x + (p1.x - p0.x) * 0.5f, p1.y);
            fill_path.cubicTo(cp1.x, cp1.y, cp2.x, cp2.y, p1.x, p1.y);
        }
        fill_path.lineTo(size_.width, size_.height);
        fill_path.close();

        Paint fill_paint;
        fill_paint.setStyle(PaintStyle::Fill);
        fill_paint.setAntiAlias(true);
        Color top_c = props_.secondary_color.value_or(props_.primary_color);
        fill_paint.setShader(Gradient::linear(
            Point(0.0f, size_.height * 0.2f),
            Point(0.0f, size_.height),
            {(top_c & 0x00FFFFFF) | 0x66000000, (props_.primary_color & 0x00FFFFFF) | 0x05000000}
        ));
        canvas.drawPath(fill_path, fill_paint);

        Paint stroke_paint;
        stroke_paint.setStyle(PaintStyle::Stroke);
        stroke_paint.setStrokeWidth(2.5f);
        stroke_paint.setAntiAlias(true);
        stroke_paint.setColor(props_.primary_color);
        canvas.drawPath(stroke_path, stroke_paint);
    }

    // ════════════════════════════════════════════════════════════════
    // 7. Voice Activity Bars (Mic)
    // ════════════════════════════════════════════════════════════════
    void paintVoiceBars(Canvas& canvas, const std::vector<float>& bands) {
        const size_t count = bands.size();
        if (count == 0) return;

        float total_gaps = float(count - 1) * props_.bar_gap;
        float bar_w = props_.bar_width;
        if (bar_w * float(count) + total_gaps > size_.width) {
            bar_w = std::max(2.0f, (size_.width - total_gaps) / float(count));
        }

        float total_w = float(count) * bar_w + total_gaps;
        float start_x = (size_.width - total_w) * 0.5f;
        float center_y = size_.height * 0.5f;

        for (size_t i = 0; i < count; ++i) {
            float val = std::clamp(bands[i], 0.0f, 1.0f);
            float half_h = std::max(props_.min_bar_height * 0.5f, val * (size_.height * 0.45f));

            float x = start_x + float(i) * (bar_w + props_.bar_gap);
            float y = center_y - half_h;
            float h = half_h * 2.0f;

            Paint p;
            p.setStyle(PaintStyle::Fill);
            p.setAntiAlias(true);
            p.setColor(props_.primary_color);

            canvas.drawRRect(Rect{x, y, bar_w, h}, BorderRadius::circular(bar_w * 0.5f), p);
        }
    }

    // ════════════════════════════════════════════════════════════════
    // 8. Scrolling Live Tape (Mic)
    // ════════════════════════════════════════════════════════════════
    void paintScrollingLive(Canvas& canvas) {
        float vol = controller_ ? controller_->getVolume() : 0.0f;
        scrolling_history_.push_back(vol);

        const size_t max_history = static_cast<size_t>(size_.width / (props_.bar_width + props_.bar_gap)) + 1;
        while (scrolling_history_.size() > max_history) {
            scrolling_history_.pop_front();
        }

        float center_y = size_.height * 0.5f;
        float x = size_.width - props_.bar_width;

        for (auto it = scrolling_history_.rbegin(); it != scrolling_history_.rend(); ++it) {
            if (x < 0.0f) break;

            float val = std::clamp(*it, 0.0f, 1.0f);
            float half_h = std::max(props_.min_bar_height * 0.5f, val * (size_.height * 0.45f));

            Paint p;
            p.setStyle(PaintStyle::Fill);
            p.setAntiAlias(true);
            p.setColor(props_.primary_color);

            canvas.drawRRect(Rect{x, center_y - half_h, props_.bar_width, half_h * 2.0f},
                             BorderRadius::circular(props_.bar_width * 0.5f), p);

            x -= (props_.bar_width + props_.bar_gap);
        }
    }

    // ════════════════════════════════════════════════════════════════
    // 9. Real-Time Oscilloscope Wave (Mic)
    // ════════════════════════════════════════════════════════════════
    void paintOscilloscope(Canvas& canvas, const std::vector<float>& wave) {
        if (wave.size() < 2) return;

        Path path;
        const float step_x = size_.width / float(wave.size() - 1);
        const float center_y = size_.height * 0.5f;
        const float amp_scale = size_.height * 0.42f;

        path.moveTo(0.0f, center_y + wave[0] * amp_scale);
        for (size_t i = 1; i < wave.size(); ++i) {
            path.lineTo(float(i) * step_x, center_y + wave[i] * amp_scale);
        }

        Paint p;
        p.setStyle(PaintStyle::Stroke);
        p.setStrokeWidth(2.0f);
        p.setAntiAlias(true);
        p.setColor(props_.primary_color);

        canvas.drawPath(path, p);
    }

    // ════════════════════════════════════════════════════════════════
    // 10. Stereo Bands (L/R)
    // ════════════════════════════════════════════════════════════════
    void paintStereoBands(Canvas& canvas, const std::vector<float>& bands) {
        const size_t half = bands.size() / 2;
        if (half == 0) return;

        std::vector<float> left(bands.begin(), bands.begin() + half);
        std::vector<float> right(bands.begin() + half, bands.end());

        canvas.save();
        paintSegmentedLed(canvas, left);
        canvas.restore();

        canvas.save();
        canvas.translate(size_.width * 0.52f, 0.0f);
        paintSegmentedLed(canvas, right);
        canvas.restore();
    }
};

// ════════════════════════════════════════════════════════════════
// AudioWaveformWidget Implementations
// ════════════════════════════════════════════════════════════════

std::unique_ptr<RenderObject> AudioWaveformWidget::createRenderObject(BuildContext&) {
    return std::make_unique<RenderAudioWaveform>(props);
}

void AudioWaveformWidget::updateRenderObject(BuildContext&, RenderObject& renderObject) {
    auto& r = static_cast<RenderAudioWaveform&>(renderObject);
    r.update(props);
}

AudioWaveformProps::operator WidgetPtr() const {
    return std::make_shared<AudioWaveformWidget>(*this);
}

} // namespace enki
