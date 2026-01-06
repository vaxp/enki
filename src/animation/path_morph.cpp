/// @file path_morph.cpp
/// @brief Vector SVG path morphing engine implementation using Skia.

#include "enki/animation/path_morph.hpp"
#include <include/core/SkPath.h>
#include <include/core/SkPathMeasure.h>
#include <include/utils/SkParsePath.h>
#include <include/core/SkCanvas.h>
#include <include/core/SkPaint.h>
#include <include/core/SkMatrix.h>
#include <cmath>
#include <algorithm>
#include <unordered_map>
#include <mutex>
#include <string>

namespace enki {

namespace {

std::vector<Point> sampleSkPath(const SkPath& path, size_t count) {
    std::vector<Point> points;
    points.reserve(count);

    if (path.isEmpty() || count < 2) {
        return points;
    }

    // Pass 1: Compute total path length across all contours
    SkPathMeasure measure(path, false);
    std::vector<float> contour_lengths;
    float total_length = 0.0f;

    do {
        float len = measure.getLength();
        contour_lengths.push_back(len);
        total_length += len;
    } while (measure.nextContour());

    if (total_length <= 0.0f) {
        SkRect b = path.getBounds();
        for (size_t i = 0; i < count; ++i) {
            points.push_back({b.centerX(), b.centerY()});
        }
        return points;
    }

    // Pass 2: Single-pass linear sampling across contours without resetting measure
    measure.setPath(&path, false);
    size_t current_contour_idx = 0;
    float current_contour_start = 0.0f;
    float current_contour_len = contour_lengths.empty() ? 0.0f : contour_lengths[0];

    for (size_t i = 0; i < count; ++i) {
        float target_d = (static_cast<float>(i) / static_cast<float>(count - 1)) * total_length;
        if (target_d >= total_length) target_d = total_length - 0.001f;

        while (current_contour_idx + 1 < contour_lengths.size() &&
               target_d > current_contour_start + current_contour_len) {
            current_contour_start += current_contour_len;
            current_contour_idx++;
            measure.nextContour();
            current_contour_len = contour_lengths[current_contour_idx];
        }

        float dist_in_contour = std::clamp(target_d - current_contour_start, 0.0f, current_contour_len);
        SkPoint pos{0, 0};
        bool ok = measure.getPosTan(dist_in_contour, &pos, nullptr);
        (void)ok;
        points.push_back({pos.x(), pos.y()});
    }

    return points;
}

struct SampledSvgEntry {
    std::vector<Point> points;
    Rect bounds{0, 0, 100, 100};
    bool is_closed = false;
};

static std::unordered_map<std::string, SampledSvgEntry> s_svg_sample_cache;
static std::mutex s_svg_sample_mutex;

static bool getOrSampleSvg(std::string_view svg_str, size_t count, SampledSvgEntry& out_entry) {
    std::string key;
    key.reserve(svg_str.size() + 16);
    key.append(svg_str);
    key.push_back('#');
    key.append(std::to_string(count));

    {
        std::lock_guard<std::mutex> lock(s_svg_sample_mutex);
        auto it = s_svg_sample_cache.find(key);
        if (it != s_svg_sample_cache.end()) {
            out_entry = it->second;
            return true;
        }
    }

    SkPath sk_path;
    std::string str(svg_str);
    if (!SkParsePath::FromSVGString(str.c_str(), &sk_path)) {
        return false;
    }

    SampledSvgEntry entry;
    entry.points = sampleSkPath(sk_path, count);
    SkRect b = sk_path.getBounds();
    if (b.isEmpty()) {
        b = SkRect::MakeWH(100.0f, 100.0f);
    }
    entry.bounds = Rect{b.left(), b.top(), b.width(), b.height()};
    entry.is_closed = (svg_str.find('Z') != std::string_view::npos ||
                       svg_str.find('z') != std::string_view::npos);

    {
        std::lock_guard<std::mutex> lock(s_svg_sample_mutex);
        s_svg_sample_cache.emplace(std::move(key), entry);
    }

    out_entry = std::move(entry);
    return true;
}

} // namespace

PathMorph::PathMorph(std::string_view from_svg_path, std::string_view to_svg_path, size_t sample_points) {
    size_t count = std::clamp(sample_points, static_cast<size_t>(10), static_cast<size_t>(300));
    SampledSvgEntry entry_a, entry_b;
    if (getOrSampleSvg(from_svg_path, count, entry_a) && getOrSampleSvg(to_svg_path, count, entry_b)) {
        points_a_ = std::move(entry_a.points);
        points_b_ = std::move(entry_b.points);
        is_closed_ = entry_a.is_closed && entry_b.is_closed;

        float min_x = std::min(entry_a.bounds.x, entry_b.bounds.x);
        float min_y = std::min(entry_a.bounds.y, entry_b.bounds.y);
        float max_x = std::max(entry_a.bounds.right(), entry_b.bounds.right());
        float max_y = std::max(entry_a.bounds.bottom(), entry_b.bounds.bottom());
        bounds_union_ = Rect{min_x, min_y, std::max(1.0f, max_x - min_x), std::max(1.0f, max_y - min_y)};
        is_valid_ = (points_a_.size() == count && points_b_.size() == count);
    }
}

PathMorph::PathMorph(const Path& from_path, const Path& to_path, size_t sample_points) {
    const auto* sk_a = static_cast<const SkPath*>(from_path.getNativeHandle());
    const auto* sk_b = static_cast<const SkPath*>(to_path.getNativeHandle());

    if (sk_a && sk_b) {
        resample(sk_a, sk_b, sample_points);
    }
}

void PathMorph::resample(const void* sk_path_a_ptr, const void* sk_path_b_ptr, size_t samples) {
    const auto& sk_a = *static_cast<const SkPath*>(sk_path_a_ptr);
    const auto& sk_b = *static_cast<const SkPath*>(sk_path_b_ptr);

    size_t count = std::clamp(samples, static_cast<size_t>(10), static_cast<size_t>(300));

    points_a_ = sampleSkPath(sk_a, count);
    points_b_ = sampleSkPath(sk_b, count);

    if (points_a_.size() == count && points_b_.size() == count) {
        is_valid_ = true;

        SkPathMeasure ma(sk_a, false);
        SkPathMeasure mb(sk_b, false);
        is_closed_ = ma.isClosed() && mb.isClosed();

        SkRect ba = sk_a.getBounds();
        SkRect bb = sk_b.getBounds();
        SkRect bu;
        bu.join(ba);
        bu.join(bb);
        if (bu.isEmpty()) {
            bu = SkRect::MakeWH(100.0f, 100.0f);
        }
        bounds_union_ = Rect{bu.left(), bu.top(), bu.width(), bu.height()};
    }
}

std::shared_ptr<Path> PathMorph::evaluate(float t) const {
    if (!is_valid_ || points_a_.empty() || points_b_.empty()) {
        return std::make_shared<Path>();
    }

    float clamped_t = std::clamp(t, 0.0f, 1.0f);
    auto res = std::make_shared<Path>();

    size_t n = points_a_.size();
    for (size_t i = 0; i < n; ++i) {
        float x = (1.0f - clamped_t) * points_a_[i].x + clamped_t * points_b_[i].x;
        float y = (1.0f - clamped_t) * points_a_[i].y + clamped_t * points_b_[i].y;

        if (i == 0) {
            res->moveTo(x, y);
        } else {
            res->lineTo(x, y);
        }
    }

    if (is_closed_) {
        res->close();
    }

    return res;
}

void PathMorph::render(Canvas& canvas, const Rect& dst, float t, const Paint& paint, bool is_stroke) const {
    if (!is_valid_ || dst.size().isEmpty()) return;

    auto morphed_path = evaluate(t);
    if (!morphed_path) return;

    auto* sk_canvas = static_cast<SkCanvas*>(canvas.getNativeHandle());
    if (!sk_canvas) return;

    const auto* sk_path = static_cast<const SkPath*>(morphed_path->getNativeHandle());
    if (!sk_path) return;

    sk_canvas->save();

    // Scale and center into dst
    float bw = std::max(1.0f, bounds_union_.width);
    float bh = std::max(1.0f, bounds_union_.height);

    float sx = dst.width / bw;
    float sy = dst.height / bh;
    float scale = std::min(sx, sy);

    float ox = dst.x + (dst.width - bw * scale) * 0.5f;
    float oy = dst.y + (dst.height - bh * scale) * 0.5f;

    sk_canvas->translate(ox, oy);
    sk_canvas->scale(scale, scale);
    sk_canvas->translate(-bounds_union_.x, -bounds_union_.y);

    SkPaint sk_paint;
    sk_paint.setAntiAlias(true);
    sk_paint.setColor(paint.getColor());
    if (is_stroke) {
        sk_paint.setStyle(SkPaint::kStroke_Style);
        float sw = paint.getStrokeWidth();
        sk_paint.setStrokeWidth(sw > 0.0f ? sw / scale : 2.0f / scale);
        sk_paint.setStrokeCap(SkPaint::kRound_Cap);
        sk_paint.setStrokeJoin(SkPaint::kRound_Join);
    } else {
        sk_paint.setStyle(SkPaint::kFill_Style);
    }

    sk_canvas->drawPath(*sk_path, sk_paint);
    sk_canvas->restore();
}

} // namespace enki
