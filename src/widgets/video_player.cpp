/// @file video_player.cpp
/// @brief Implementation of VideoPlayerWidget and RenderVideoPlayer.
/// @copyright ENKI Framework — MIT License

#include "enki/widgets/video_player.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/rendering/canvas.hpp"
#include "enki/animation/ticker.hpp"
#include "enki/platform/input.hpp"

#include <include/core/SkCanvas.h>
#include <include/core/SkImage.h>
#include <include/core/SkPaint.h>
#include <include/core/SkPixmap.h>
#include <include/core/SkSamplingOptions.h>
#include <include/core/SkRRect.h>
#include <include/core/SkFont.h>

#include <cmath>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace enki {

namespace {

std::string formatTime(double seconds) {
    if (seconds < 0.0) seconds = 0.0;
    int total = static_cast<int>(seconds);
    int m = total / 60;
    int s = total % 60;
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << m << ":"
        << std::setfill('0') << std::setw(2) << s;
    return oss.str();
}

} // namespace

// ════════════════════════════════════════════════════════════════
// RenderVideoPlayer
// ════════════════════════════════════════════════════════════════

class RenderVideoPlayer : public RenderBox {
public:
    VideoPlayerProps props_;
    std::shared_ptr<video::VideoController> controller_;
    std::unique_ptr<Ticker> ticker_;
    sk_sp<SkImage>          cached_sk_image_;
    double                  cached_frame_pts_{-1.0};
    bool                    owns_controller_{false};

    // HUD interaction & auto-hide
    std::chrono::steady_clock::time_point last_interaction_time_;
    std::chrono::steady_clock::time_point last_scrub_seek_time_;
    float                   controls_alpha_{1.0f};
    bool                    is_mouse_over_{false};
    bool                    is_scrubbing_{false};
    double                  scrub_target_sec_{0.0};
    bool                    show_speed_menu_{false};
    static constexpr float  kSpeedOptions[5] = {0.5f, 1.0f, 1.25f, 1.5f, 2.0f};
    Point                   mouse_pos_{0.0f, 0.0f};

    explicit RenderVideoPlayer(VideoPlayerProps props)
        : props_(std::move(props)) {
        initController();
        applyStyleToNode();
        last_interaction_time_ = std::chrono::steady_clock::now();
        startTicker();
    }

    ~RenderVideoPlayer() override {
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
            controller_ = std::make_shared<video::VideoController>();
            owns_controller_ = true;
            if (!props_.source.empty()) {
                controller_->open(props_.source);
                controller_->setLooping(props_.looping);
                controller_->setVolume(props_.volume);
                controller_->setPlaybackSpeed(props_.playback_speed);
                if (props_.auto_play) {
                    controller_->play();
                }
            }
        }
    }

    void startTicker() {
        ticker_ = createTicker([this]() {
            // Check auto-hide for HUD
            auto now = std::chrono::steady_clock::now();
            double idle_sec = std::chrono::duration<double>(now - last_interaction_time_).count();

            if (controller_ && controller_->isPlaying() && idle_sec > 2.2) {
                controls_alpha_ = std::max(0.0f, controls_alpha_ - 0.05f);
            } else {
                controls_alpha_ = std::min(1.0f, controls_alpha_ + 0.08f);
            }

            markNeedsPaint();
        });
        ticker_->start();
    }

    void update(const VideoPlayerProps& new_props) {
        bool layout_changed = (props_.width != new_props.width ||
                               props_.height != new_props.height ||
                               props_.min_width != new_props.min_width ||
                               props_.min_height != new_props.min_height ||
                               props_.max_width != new_props.max_width ||
                               props_.max_height != new_props.max_height);

        if (props_.source != new_props.source || props_.controller != new_props.controller) {
            if (owns_controller_ && controller_) {
                controller_->stop();
            }
            props_ = new_props;
            initController();
        } else {
            bool speed_changed = (props_.playback_speed != new_props.playback_speed);
            bool volume_changed = (props_.volume != new_props.volume);
            bool looping_changed = (props_.looping != new_props.looping);
            props_ = new_props;
            if (controller_) {
                if (looping_changed) controller_->setLooping(props_.looping);
                if (volume_changed) controller_->setVolume(props_.volume);
                if (speed_changed) controller_->setPlaybackSpeed(props_.playback_speed);
            }
        }

        if (layout_changed) {
            applyStyleToNode();
            markNeedsLayout();
        }
        markNeedsPaint();
    }

    void applyStyleToNode() {
        if (!anu_node_) return;

        if (props_.width.has_value()) {
            if (props_.width->isPercent()) ANUNodeStyleSetWidthPercent(anu_node_, props_.width->value);
            else if (props_.width->isAuto()) ANUNodeStyleSetWidthAuto(anu_node_);
            else ANUNodeStyleSetWidth(anu_node_, props_.width->value);
        } else {
            ANUNodeStyleSetWidthAuto(anu_node_);
        }

        if (props_.height.has_value()) {
            if (props_.height->isPercent()) ANUNodeStyleSetHeightPercent(anu_node_, props_.height->value);
            else if (props_.height->isAuto()) ANUNodeStyleSetHeightAuto(anu_node_);
            else ANUNodeStyleSetHeight(anu_node_, props_.height->value);
        } else {
            ANUNodeStyleSetHeightAuto(anu_node_);
        }

        if (props_.min_width.has_value()) {
            if (props_.min_width->isPercent()) ANUNodeStyleSetMinWidthPercent(anu_node_, props_.min_width->value);
            else ANUNodeStyleSetMinWidth(anu_node_, props_.min_width->value);
        }
        if (props_.min_height.has_value()) {
            if (props_.min_height->isPercent()) ANUNodeStyleSetMinHeightPercent(anu_node_, props_.min_height->value);
            else ANUNodeStyleSetMinHeight(anu_node_, props_.min_height->value);
        }

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

        context.canvas.save();
        Rect bounds = Rect::fromLTWH(context.offset.x, context.offset.y, size_.width, size_.height);

        // Clip rounded corners
        if (props_.border_radius != BorderRadius::zero()) {
            context.canvas.clipRRect(bounds, props_.border_radius);
        } else {
            context.canvas.clipRect(bounds);
        }

        // Draw background
        Paint bg_paint;
        bg_paint.setStyle(PaintStyle::Fill);
        bg_paint.setColor(props_.background_color);
        context.canvas.drawRect(bounds, bg_paint);

        // Fetch synchronized video frame
        if (controller_) {
            auto frame = controller_->getNextRenderFrame();
            if (frame && !frame->rgba_data.empty() && frame->pts_sec != cached_frame_pts_) {
                SkImageInfo info = SkImageInfo::Make(
                    frame->width, frame->height,
                    kRGBA_8888_SkColorType, kPremul_SkAlphaType
                );
                SkPixmap pixmap(info, frame->rgba_data.data(), frame->width * 4);
                cached_sk_image_ = SkImage::MakeRasterCopy(pixmap);
                cached_frame_pts_ = frame->pts_sec;
            }
        }

        // Render current video frame via Skia GPU
        if (cached_sk_image_) {
            SkCanvas* sk_canvas = static_cast<SkCanvas*>(context.canvas.getNativeHandle());
            if (sk_canvas) {
                float img_w = float(cached_sk_image_->width());
                float img_h = float(cached_sk_image_->height());

                Rect dst = calculateDestRect(img_w, img_h, bounds);
                SkRect src_sk = SkRect::MakeWH(img_w, img_h);
                SkRect dst_sk = SkRect::MakeXYWH(dst.x, dst.y, dst.width, dst.height);

                SkPaint paint;
                paint.setAntiAlias(true);

                sk_canvas->drawImageRect(
                    cached_sk_image_.get(), src_sk, dst_sk,
                    SkSamplingOptions(SkFilterMode::kLinear),
                    &paint, SkCanvas::kFast_SrcRectConstraint
                );
            }
        }

        // Render Futuristic Glassmorphic HUD overlay
        if (props_.show_controls && controls_alpha_ > 0.01f) {
            paintHudControls(context.canvas, bounds);
        }

        context.canvas.restore();
    }

    [[nodiscard]] bool hitTestSelf(Point localPoint) const override {
        return localPoint.x >= 0.0f && localPoint.x <= size_.width &&
               localPoint.y >= 0.0f && localPoint.y <= size_.height;
    }

    void handlePointerDown(const PointerEvent& e) override {
        last_interaction_time_ = std::chrono::steady_clock::now();
        controls_alpha_ = 1.0f;

        if (!controller_) return;

        float mx = e.localPosition.x;
        float my = e.localPosition.y;

        float bar_h = 44.0f;
        float bar_w = std::min(size_.width - 24.0f, 720.0f);
        float bar_x = (size_.width - bar_w) * 0.5f;
        float bar_y = size_.height - bar_h - 12.0f;

        float speed_btn_w = 46.0f;
        float speed_btn_h = 24.0f;
        float speed_btn_x = bar_x + bar_w - speed_btn_w - 10.0f;
        float speed_btn_y = bar_y + (bar_h - speed_btn_h) * 0.5f;

        float time_w = 90.0f;
        float time_x = speed_btn_x - time_w - 8.0f;

        float track_x = bar_x + 48.0f;
        float track_w = time_x - track_x - 12.0f;

        // 1. Check speed dropdown menu click (when open)
        if (show_speed_menu_) {
            float menu_w = 64.0f;
            float item_h = 26.0f;
            float menu_h = item_h * 5 + 8.0f;
            float menu_x = speed_btn_x + (speed_btn_w - menu_w) * 0.5f;
            float menu_y = bar_y - menu_h - 6.0f;
            Rect menu_rect{menu_x, menu_y, menu_w, menu_h};

            if (menu_rect.contains(Point(mx, my))) {
                for (int i = 0; i < 5; ++i) {
                    float iy = menu_y + 4.0f + i * item_h;
                    Rect item_r{menu_x + 4.0f, iy, menu_w - 8.0f, item_h};
                    if (item_r.contains(Point(mx, my))) {
                        if (controller_) {
                            controller_->setPlaybackSpeed(kSpeedOptions[i]);
                        }
                        show_speed_menu_ = false;
                        markNeedsPaint();
                        return;
                    }
                }
                return;
            } else {
                show_speed_menu_ = false;
                markNeedsPaint();
            }
        }

        // 2. Check speed button toggle hit
        Rect speed_btn_rect{speed_btn_x, speed_btn_y, speed_btn_w, speed_btn_h};
        if (speed_btn_rect.contains(Point(mx, my))) {
            show_speed_menu_ = !show_speed_menu_;
            markNeedsPaint();
            return;
        }

        // 3. Check scrubber progress track hit
        if (mx >= track_x - 10.0f && mx <= track_x + track_w + 10.0f && my >= bar_y && my <= bar_y + bar_h) {
            is_scrubbing_ = true;
            seekFromScrubber(mx, track_x, track_w, true);
            markNeedsPaint();
            return;
        }

        // 4. Check HUD Play/Pause button hit
        Rect play_btn_rect{bar_x + 4.0f, bar_y + 4.0f, 40.0f, 38.0f};
        if (play_btn_rect.contains(Point(mx, my))) {
            controller_->togglePlay();
            markNeedsPaint();
            return;
        }

        // 5. Check Center Play button hit (when paused)
        if (!controller_->isPlaying()) {
            float cx = size_.width * 0.5f;
            float cy = size_.height * 0.5f;
            float dx = mx - cx;
            float dy = my - cy;
            if (dx * dx + dy * dy <= 48.0f * 48.0f) {
                controller_->play();
                markNeedsPaint();
                return;
            }
        }

        // 6. Click elsewhere on video: toggle play/pause
        controller_->togglePlay();
        markNeedsPaint();
    }

    void handlePointerMove(const PointerEvent& e) override {
        last_interaction_time_ = std::chrono::steady_clock::now();
        controls_alpha_ = 1.0f;
        mouse_pos_ = e.localPosition;

        if (is_scrubbing_ && controller_) {
            float bar_w = std::min(size_.width - 24.0f, 720.0f);
            float bar_x = (size_.width - bar_w) * 0.5f;
            float speed_btn_w = 46.0f;
            float speed_btn_x = bar_x + bar_w - speed_btn_w - 10.0f;
            float time_w = 90.0f;
            float time_x = speed_btn_x - time_w - 8.0f;
            float track_x = bar_x + 48.0f;
            float track_w = time_x - track_x - 12.0f;
            seekFromScrubber(e.localPosition.x, track_x, track_w, false);
        }
        markNeedsPaint();
    }

    void handlePointerUp(const PointerEvent& e) override {
        if (is_scrubbing_ && controller_) {
            is_scrubbing_ = false;
            float bar_w = std::min(size_.width - 24.0f, 720.0f);
            float bar_x = (size_.width - bar_w) * 0.5f;
            float speed_btn_w = 46.0f;
            float speed_btn_x = bar_x + bar_w - speed_btn_w - 10.0f;
            float time_w = 90.0f;
            float time_x = speed_btn_x - time_w - 8.0f;
            float track_x = bar_x + 48.0f;
            float track_w = time_x - track_x - 12.0f;
            seekFromScrubber(e.localPosition.x, track_x, track_w, true);
            markNeedsPaint();
        }
    }

    void handlePointerEnter(const PointerEvent&) override {
        is_mouse_over_ = true;
        last_interaction_time_ = std::chrono::steady_clock::now();
        controls_alpha_ = 1.0f;
        markNeedsPaint();
    }

    void handlePointerExit(const PointerEvent&) override {
        is_mouse_over_ = false;
        if (is_scrubbing_) {
            is_scrubbing_ = false;
        }
    }

    [[nodiscard]] SystemCursor cursor() const override {
        return is_scrubbing_ ? SystemCursor::ResizeHorizontal : SystemCursor::Pointer;
    }

private:
    void seekFromScrubber(float mx, float track_x, float track_w, bool force_immediate) {
        if (track_w <= 0.0f || !controller_) return;
        float fraction = std::clamp((mx - track_x) / track_w, 0.0f, 1.0f);
        double duration = controller_->getDuration();
        if (duration <= 0.0) return;

        scrub_target_sec_ = fraction * duration;

        auto now = std::chrono::steady_clock::now();
        double ms_since = std::chrono::duration<double, std::milli>(now - last_scrub_seek_time_).count();

        if (force_immediate || ms_since >= 60.0) {
            last_scrub_seek_time_ = now;
            controller_->seek(scrub_target_sec_);
        }
    }

    Rect calculateDestRect(float src_w, float src_h, const Rect& container) const {
        if (src_w <= 0.0f || src_h <= 0.0f) return container;

        float c_w = container.width;
        float c_h = container.height;

        switch (props_.fit) {
            case BoxFit::Fill:
                return container;

            case BoxFit::Contain: {
                float scale = std::min(c_w / src_w, c_h / src_h);
                float w = src_w * scale;
                float h = src_h * scale;
                float x = container.x + (c_w - w) * 0.5f;
                float y = container.y + (c_h - h) * 0.5f;
                return Rect{x, y, w, h};
            }

            case BoxFit::Cover: {
                float scale = std::max(c_w / src_w, c_h / src_h);
                float w = src_w * scale;
                float h = src_h * scale;
                float x = container.x + (c_w - w) * 0.5f;
                float y = container.y + (c_h - h) * 0.5f;
                return Rect{x, y, w, h};
            }

            case BoxFit::FitWidth: {
                float scale = c_w / src_w;
                float w = c_w;
                float h = src_h * scale;
                float y = container.y + (c_h - h) * 0.5f;
                return Rect{container.x, y, w, h};
            }

            case BoxFit::FitHeight: {
                float scale = c_h / src_h;
                float w = src_w * scale;
                float h = c_h;
                float x = container.x + (c_w - w) * 0.5f;
                return Rect{x, container.y, w, h};
            }

            default:
                return container;
        }
    }

    void paintHudControls(Canvas& canvas, const Rect& bounds) {
        uint8_t alpha = static_cast<uint8_t>(controls_alpha_ * 255.0f);

        double cur_pos = is_scrubbing_ ? scrub_target_sec_ : (controller_ ? controller_->getCurrentPosition() : 0.0);
        double duration = controller_ ? controller_->getDuration() : 0.0;
        bool is_playing = controller_ && controller_->isPlaying();

        // 1. Center Floating Play Button (when paused)
        if (!is_playing) {
            float cx = bounds.x + bounds.width * 0.5f;
            float cy = bounds.y + bounds.height * 0.5f;
            float radius = 32.0f;

            Paint circle_paint;
            circle_paint.setStyle(PaintStyle::Fill);
            circle_paint.setAntiAlias(true);
            circle_paint.setColor((static_cast<uint32_t>(alpha * 0.75f) << 24) | 0x000E1626);
            canvas.drawCircle(Point(cx, cy), radius, circle_paint);

            Paint ring_paint;
            ring_paint.setStyle(PaintStyle::Stroke);
            ring_paint.setStrokeWidth(2.0f);
            ring_paint.setAntiAlias(true);
            ring_paint.setColor((static_cast<uint32_t>(alpha) << 24) | 0x0000E5FF);
            canvas.drawCircle(Point(cx, cy), radius, ring_paint);

            // Play Triangle
            Path tri;
            tri.moveTo(cx - 8.0f, cy - 12.0f);
            tri.lineTo(cx + 14.0f, cy);
            tri.lineTo(cx - 8.0f, cy + 12.0f);
            tri.close();

            Paint tri_paint;
            tri_paint.setStyle(PaintStyle::Fill);
            tri_paint.setAntiAlias(true);
            tri_paint.setColor((static_cast<uint32_t>(alpha) << 24) | 0x00FFFFFF);
            canvas.drawPath(tri, tri_paint);
        }

        // 2. Glassmorphic Bottom Control Bar
        float bar_h = 44.0f;
        float bar_w = std::min(bounds.width - 24.0f, 720.0f);
        float bar_x = bounds.x + (bounds.width - bar_w) * 0.5f;
        float bar_y = bounds.y + bounds.height - bar_h - 12.0f;

        Rect bar_rect{bar_x, bar_y, bar_w, bar_h};

        // Glass background
        Paint bar_bg;
        bar_bg.setStyle(PaintStyle::Fill);
        bar_bg.setAntiAlias(true);
        bar_bg.setColor((static_cast<uint32_t>(alpha * 0.88f) << 24) | 0x000B1320);
        canvas.drawRRect(bar_rect, BorderRadius::circular(14.0f), bar_bg);

        // Glass border
        Paint bar_border;
        bar_border.setStyle(PaintStyle::Stroke);
        bar_border.setStrokeWidth(1.2f);
        bar_border.setAntiAlias(true);
        bar_border.setColor((static_cast<uint32_t>(alpha * 0.35f) << 24) | 0x0000E5FF);
        canvas.drawRRect(bar_rect, BorderRadius::circular(14.0f), bar_border);

        // Play / Pause Icon on Left
        float play_x = bar_x + 22.0f;
        float play_y = bar_y + bar_h * 0.5f;
        Paint icon_paint;
        icon_paint.setStyle(PaintStyle::Fill);
        icon_paint.setAntiAlias(true);
        icon_paint.setColor((static_cast<uint32_t>(alpha) << 24) | 0x0000E5FF);

        if (is_playing) {
            // Pause bars
            canvas.drawRect(Rect{play_x - 6.0f, play_y - 7.0f, 4.0f, 14.0f}, icon_paint);
            canvas.drawRect(Rect{play_x + 2.0f, play_y - 7.0f, 4.0f, 14.0f}, icon_paint);
        } else {
            // Play triangle
            Path p;
            p.moveTo(play_x - 5.0f, play_y - 7.0f);
            p.lineTo(play_x + 7.0f, play_y);
            p.lineTo(play_x - 5.0f, play_y + 7.0f);
            p.close();
            canvas.drawPath(p, icon_paint);
        }

        // Layout Metrics for Right Elements
        float speed_btn_w = 46.0f;
        float speed_btn_h = 24.0f;
        float speed_btn_x = bar_x + bar_w - speed_btn_w - 10.0f;
        float speed_btn_y = bar_y + (bar_h - speed_btn_h) * 0.5f;

        float time_w = 90.0f;
        float time_x = speed_btn_x - time_w - 8.0f;

        // Progress Bar Track
        float track_x = bar_x + 48.0f;
        float track_w = time_x - track_x - 12.0f;
        float track_y = bar_y + bar_h * 0.5f - 2.0f;
        float track_h = 4.0f;

        Paint track_bg;
        track_bg.setStyle(PaintStyle::Fill);
        track_bg.setAntiAlias(true);
        track_bg.setColor((static_cast<uint32_t>(alpha * 0.3f) << 24) | 0x00FFFFFF);
        canvas.drawRRect(Rect{track_x, track_y, track_w, track_h}, BorderRadius::circular(2.0f), track_bg);

        // Played progress fill
        float progress = (duration > 0.0) ? float(cur_pos / duration) : 0.0f;
        progress = std::clamp(progress, 0.0f, 1.0f);

        Paint track_fill;
        track_fill.setStyle(PaintStyle::Fill);
        track_fill.setAntiAlias(true);
        track_fill.setColor((static_cast<uint32_t>(alpha) << 24) | 0x0000E5FF);
        canvas.drawRRect(Rect{track_x, track_y, track_w * progress, track_h}, BorderRadius::circular(2.0f), track_fill);

        // Scrubber handle dot
        Paint thumb_paint;
        thumb_paint.setStyle(PaintStyle::Fill);
        thumb_paint.setAntiAlias(true);
        thumb_paint.setColor((static_cast<uint32_t>(alpha) << 24) | 0x00FFFFFF);
        float thumb_r = is_scrubbing_ ? 7.5f : 5.5f;
        canvas.drawCircle(Point(track_x + track_w * progress, track_y + track_h * 0.5f), thumb_r, thumb_paint);

        if (is_scrubbing_) {
            Paint halo;
            halo.setStyle(PaintStyle::Stroke);
            halo.setStrokeWidth(2.0f);
            halo.setColor((static_cast<uint32_t>(alpha) << 24) | 0x0000E5FF);
            canvas.drawCircle(Point(track_x + track_w * progress, track_y + track_h * 0.5f), 12.0f, halo);
        }

        // Time Label: "01:24 / 04:12"
        std::string time_str = formatTime(cur_pos) + " / " + formatTime(duration);
        Paint text_paint;
        text_paint.setColor((static_cast<uint32_t>(alpha) << 24) | 0x00CBD5E1);
        text_paint.setAntiAlias(true);

        canvas.drawText(
            time_str,
            Point(time_x, bar_y + bar_h * 0.5f + 4.0f),
            text_paint,
            11.0f,
            nullptr,
            false
        );

        // Playback Speed Button Pill
        Rect speed_btn_rect{speed_btn_x, speed_btn_y, speed_btn_w, speed_btn_h};
        Paint speed_btn_bg;
        speed_btn_bg.setStyle(PaintStyle::Fill);
        speed_btn_bg.setAntiAlias(true);
        speed_btn_bg.setColor((static_cast<uint32_t>(alpha * (show_speed_menu_ ? 0.45f : 0.20f)) << 24) | 0x0000E5FF);
        canvas.drawRRect(speed_btn_rect, BorderRadius::circular(6.0f), speed_btn_bg);

        Paint speed_btn_border;
        speed_btn_border.setStyle(PaintStyle::Stroke);
        speed_btn_border.setStrokeWidth(1.0f);
        speed_btn_border.setAntiAlias(true);
        speed_btn_border.setColor((static_cast<uint32_t>(alpha * 0.6f) << 24) | 0x0000E5FF);
        canvas.drawRRect(speed_btn_rect, BorderRadius::circular(6.0f), speed_btn_border);

        float cur_speed = controller_ ? controller_->getPlaybackSpeed() : 1.0f;
        char speed_buf[16];
        if (std::abs(cur_speed - std::round(cur_speed)) < 0.01f) {
            std::snprintf(speed_buf, sizeof(speed_buf), "%.1fx", cur_speed);
        } else {
            std::snprintf(speed_buf, sizeof(speed_buf), "%.2fx", cur_speed);
        }

        Paint speed_btn_text;
        speed_btn_text.setColor((static_cast<uint32_t>(alpha) << 24) | 0x0038BDF8);
        speed_btn_text.setAntiAlias(true);
        canvas.drawText(
            speed_buf,
            Point(speed_btn_x + 9.0f, speed_btn_y + speed_btn_h * 0.5f + 3.5f),
            speed_btn_text,
            10.5f,
            nullptr,
            false
        );

        // 3. Floating Speed Dropdown Menu (Glass Card)
        if (show_speed_menu_) {
            float menu_w = 64.0f;
            float item_h = 26.0f;
            float menu_h = item_h * 5 + 8.0f;
            float menu_x = speed_btn_x + (speed_btn_w - menu_w) * 0.5f;
            float menu_y = bar_y - menu_h - 6.0f;
            Rect menu_rect{menu_x, menu_y, menu_w, menu_h};

            // Glass menu card
            Paint menu_bg;
            menu_bg.setStyle(PaintStyle::Fill);
            menu_bg.setAntiAlias(true);
            menu_bg.setColor((static_cast<uint32_t>(alpha * 0.95f) << 24) | 0x00070D18);
            canvas.drawRRect(menu_rect, BorderRadius::circular(10.0f), menu_bg);

            Paint menu_border;
            menu_border.setStyle(PaintStyle::Stroke);
            menu_border.setStrokeWidth(1.2f);
            menu_border.setAntiAlias(true);
            menu_border.setColor((static_cast<uint32_t>(alpha * 0.8f) << 24) | 0x0000E5FF);
            canvas.drawRRect(menu_rect, BorderRadius::circular(10.0f), menu_border);

            for (int i = 0; i < 5; ++i) {
                float s = kSpeedOptions[i];
                float iy = menu_y + 4.0f + i * item_h;
                Rect item_r{menu_x + 4.0f, iy, menu_w - 8.0f, item_h};
                bool is_cur = (std::abs(cur_speed - s) < 0.05f);

                if (is_cur) {
                    Paint sel_bg;
                    sel_bg.setStyle(PaintStyle::Fill);
                    sel_bg.setAntiAlias(true);
                    sel_bg.setColor((static_cast<uint32_t>(alpha * 0.35f) << 24) | 0x0000E5FF);
                    canvas.drawRRect(item_r, BorderRadius::circular(6.0f), sel_bg);
                }

                char opt_buf[16];
                if (std::abs(s - std::round(s)) < 0.01f) {
                    std::snprintf(opt_buf, sizeof(opt_buf), "%.1fx", s);
                } else {
                    std::snprintf(opt_buf, sizeof(opt_buf), "%.2fx", s);
                }

                Paint item_text;
                item_text.setAntiAlias(true);
                item_text.setColor(is_cur ? ((static_cast<uint32_t>(alpha) << 24) | 0x0000E5FF)
                                          : ((static_cast<uint32_t>(alpha) << 24) | 0x0094A3B8));

                canvas.drawText(
                    opt_buf,
                    Point(item_r.x + 16.0f, item_r.y + item_h * 0.5f + 4.0f),
                    item_text,
                    11.0f,
                    nullptr,
                    false
                );
            }
        }
    }
};

// ════════════════════════════════════════════════════════════════
// VideoPlayerWidget Implementations
// ════════════════════════════════════════════════════════════════

std::unique_ptr<RenderObject> VideoPlayerWidget::createRenderObject(BuildContext&) {
    return std::make_unique<RenderVideoPlayer>(props);
}

void VideoPlayerWidget::updateRenderObject(BuildContext&, RenderObject& renderObject) {
    auto& r = static_cast<RenderVideoPlayer&>(renderObject);
    r.update(props);
}

VideoPlayerProps::operator WidgetPtr() const {
    return std::make_shared<VideoPlayerWidget>(*this);
}

} // namespace enki
