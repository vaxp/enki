/// @file svg.cpp
/// @brief Vector SVG parser and hardware-accelerated rendering implementation using Skia.

#include "enki/rendering/svg.hpp"
#include "enki/rendering/canvas.hpp"
#include <include/core/SkCanvas.h>
#include <include/core/SkPaint.h>
#include <include/core/SkPath.h>
#include <include/core/SkMatrix.h>
#include <include/core/SkRect.h>
#include <include/core/SkShader.h>
#include <include/utils/SkParsePath.h>
#include <include/core/SkData.h>

#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <iostream>
#include <unordered_map>

namespace enki {

namespace {

std::string trim(std::string_view s) {
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front()))) s.remove_prefix(1);
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) s.remove_suffix(1);
    return std::string(s);
}

std::optional<std::string> getAttr(std::string_view tag, std::string_view attr) {
    std::string pattern = std::string(attr) + "=";
    size_t pos = tag.find(pattern);
    if (pos == std::string_view::npos) return std::nullopt;

    pos += pattern.size();
    if (pos >= tag.size()) return std::nullopt;

    char quote = tag[pos];
    if (quote != '"' && quote != '\'') return std::nullopt;

    size_t end_pos = tag.find(quote, pos + 1);
    if (end_pos == std::string_view::npos) return std::nullopt;

    return std::string(tag.substr(pos + 1, end_pos - pos - 1));
}

std::optional<Color> parseSvgColor(std::string_view str) {
    std::string s = trim(str);
    if (s.empty() || s == "none" || s == "transparent") return std::nullopt;

    // Standard Named Colors
    static const std::unordered_map<std::string, uint32_t> named = {
        {"black",   0xFF000000}, {"white",   0xFFFFFFFF}, {"red",     0xFFFF0000},
        {"green",   0xFF00FF00}, {"blue",    0xFF0000FF}, {"yellow",  0xFFFFFF00},
        {"cyan",    0xFF00FFFF}, {"magenta", 0xFFFF00FF}, {"gray",    0xFF808080},
        {"grey",    0xFF808080}, {"gold",    0xFFFFD700}, {"silver",  0xFFC0C0C0},
        {"orange",  0xFFFFA500}, {"purple",  0xFF800080}, {"pink",    0xFFFFC0CB}
    };

    auto it = named.find(s);
    if (it != named.end()) return Color(it->second);

    // Hex color: #RGB, #RRGGBB, #RRGGBBAA
    if (s.front() == '#') {
        std::string h = s.substr(1);
        if (h.size() == 3) {
            uint32_t r = std::stoul(std::string(2, h[0]), nullptr, 16);
            uint32_t g = std::stoul(std::string(2, h[1]), nullptr, 16);
            uint32_t b = std::stoul(std::string(2, h[2]), nullptr, 16);
            return Color((0xFF << 24) | (r << 16) | (g << 8) | b);
        } else if (h.size() == 6) {
            uint32_t val = std::stoul(h, nullptr, 16);
            return Color(0xFF000000 | val);
        } else if (h.size() == 8) {
            uint32_t val = std::stoul(h, nullptr, 16);
            // In web SVG, #RRGGBBAA has alpha at the end:
            uint32_t r = (val >> 24) & 0xFF;
            uint32_t g = (val >> 16) & 0xFF;
            uint32_t b = (val >> 8) & 0xFF;
            uint32_t a = val & 0xFF;
            return Color((a << 24) | (r << 16) | (g << 8) | b);
        }
    }

    // rgb(r, g, b) or rgba(r, g, b, a)
    if (s.rfind("rgb", 0) == 0) {
        size_t open_p = s.find('(');
        size_t close_p = s.find(')');
        if (open_p != std::string::npos && close_p != std::string::npos) {
            std::string inside = s.substr(open_p + 1, close_p - open_p - 1);
            std::replace(inside.begin(), inside.end(), ',', ' ');
            std::stringstream ss(inside);
            float r = 0, g = 0, b = 0, a = 1.0f;
            if (ss >> r >> g >> b) {
                if (ss >> a) {
                    if (a <= 1.0f) a *= 255.0f;
                } else {
                    a = 255.0f;
                }
                return Color((static_cast<uint32_t>(a) << 24) |
                             (static_cast<uint32_t>(r) << 16) |
                             (static_cast<uint32_t>(g) << 8)  |
                              static_cast<uint32_t>(b));
            }
        }
    }

    return std::nullopt;
}

} // namespace

struct SvgElement {
    SkPath path;
    std::optional<Color> fill;
    std::optional<Color> stroke;
    float stroke_width = 1.0f;
};

class SvgDocumentImpl : public SvgDocument {
public:
    SkRect view_box_ = SkRect::MakeEmpty();
    std::vector<SvgElement> elements_;

    bool isValid() const override {
        return !elements_.empty() || !view_box_.isEmpty();
    }

    Rect getBounds() const override {
        return Rect{view_box_.left(), view_box_.top(), view_box_.width(), view_box_.height()};
    }

    void drawElement(SkCanvas* sk_canvas, const SvgElement& el, const SkPath& path,
                     const Paint* override_paint, bool is_stroke) {
        if (override_paint) {
            SkPaint sk_p;
            sk_p.setAntiAlias(override_paint->isAntiAlias());
            sk_p.setColor(override_paint->getColor());
            sk_p.setStrokeWidth(override_paint->getStrokeWidth());
            if (override_paint->getShader()) {
                sk_p.setShader(sk_ref_sp(static_cast<SkShader*>(override_paint->getShader()->getNativeHandle())));
            }

            if (is_stroke) {
                sk_p.setStyle(SkPaint::kStroke_Style);
                if (sk_p.getStrokeWidth() <= 0.0f) sk_p.setStrokeWidth(el.stroke_width > 0.0f ? el.stroke_width : 2.0f);
            } else {
                switch (override_paint->getStyle()) {
                    case PaintStyle::Fill:          sk_p.setStyle(SkPaint::kFill_Style); break;
                    case PaintStyle::Stroke:        sk_p.setStyle(SkPaint::kStroke_Style); break;
                    case PaintStyle::StrokeAndFill: sk_p.setStyle(SkPaint::kStrokeAndFill_Style); break;
                }
            }
            sk_canvas->drawPath(path, sk_p);
            return;
        }

        if (is_stroke) {
            SkPaint p;
            p.setAntiAlias(true);
            p.setStyle(SkPaint::kStroke_Style);
            p.setStrokeWidth(el.stroke_width > 0.0f ? el.stroke_width : 2.0f);
            if (el.stroke.has_value()) {
                p.setColor(*el.stroke);
            } else if (el.fill.has_value()) {
                p.setColor(*el.fill);
            } else {
                p.setColor(0xFFFFFFFF);
            }
            sk_canvas->drawPath(path, p);
            return;
        }

        // 1. Draw Fill
        if (el.fill.has_value()) {
            SkPaint p;
            p.setAntiAlias(true);
            p.setStyle(SkPaint::kFill_Style);
            p.setColor(*el.fill);
            sk_canvas->drawPath(path, p);
        } else if (!el.stroke.has_value()) {
            // Default SVG spec: fill is black if neither fill nor stroke is defined
            SkPaint p;
            p.setAntiAlias(true);
            p.setStyle(SkPaint::kFill_Style);
            p.setColor(0xFFFFFFFF);
            sk_canvas->drawPath(path, p);
        }

        // 2. Draw Stroke
        if (el.stroke.has_value() && el.stroke_width > 0.0f) {
            SkPaint p;
            p.setAntiAlias(true);
            p.setStyle(SkPaint::kStroke_Style);
            p.setColor(*el.stroke);
            p.setStrokeWidth(el.stroke_width);
            sk_canvas->drawPath(path, p);
        }
    }

    void render(Canvas& canvas, const Rect& dst, SvgFit fit,
                const Paint* override_paint, bool is_stroke) override {
        SkCanvas* sk_canvas = static_cast<SkCanvas*>(canvas.getNativeHandle());
        if (!sk_canvas || elements_.empty()) return;

        float vw = view_box_.width() > 0.0f ? view_box_.width() : 100.0f;
        float vh = view_box_.height() > 0.0f ? view_box_.height() : 100.0f;

        SkMatrix m;
        if (fit == SvgFit::Stretch) {
            float sx = dst.width / vw;
            float sy = dst.height / vh;
            m.setScale(sx, sy);
            m.postTranslate(dst.x - view_box_.left() * sx, dst.y - view_box_.top() * sy);
        } else if (fit == SvgFit::Contain) {
            float s = std::min(dst.width / vw, dst.height / vh);
            float dx = dst.x + (dst.width - vw * s) * 0.5f - view_box_.left() * s;
            float dy = dst.y + (dst.height - vh * s) * 0.5f - view_box_.top() * s;
            m.setScale(s, s);
            m.postTranslate(dx, dy);
        } else if (fit == SvgFit::Cover) {
            float s = std::max(dst.width / vw, dst.height / vh);
            float dx = dst.x + (dst.width - vw * s) * 0.5f - view_box_.left() * s;
            float dy = dst.y + (dst.height - vh * s) * 0.5f - view_box_.top() * s;
            m.setScale(s, s);
            m.postTranslate(dx, dy);
        }

        sk_canvas->save();
        if (fit == SvgFit::Cover) {
            sk_canvas->clipRect(SkRect::MakeXYWH(dst.x, dst.y, dst.width, dst.height), true);
        }

        for (const auto& el : elements_) {
            SkPath transformed;
            el.path.transform(m, &transformed);
            drawElement(sk_canvas, el, transformed, override_paint, is_stroke);
        }

        sk_canvas->restore();
    }

    void renderNineSlice(Canvas& canvas, const Rect& dst, const SvgSlice& slice,
                         const Paint* override_paint, bool is_stroke) override {
        SkCanvas* sk_canvas = static_cast<SkCanvas*>(canvas.getNativeHandle());
        if (!sk_canvas || elements_.empty()) return;

        float vx = view_box_.left();
        float vy = view_box_.top();
        float vw = view_box_.width();
        float vh = view_box_.height();
        if (vw <= 0.0f || vh <= 0.0f) return;

        float sL = slice.left;
        float sR = slice.right;
        float sT = slice.top;
        float sB = slice.bottom;

        if (sL + sR >= vw || sT + sB >= vh || sL + sR >= dst.width || sT + sB >= dst.height) {
            render(canvas, dst, SvgFit::Stretch, override_paint, is_stroke);
            return;
        }

        float src_x[4] = { vx, vx + sL, vx + vw - sR, vx + vw };
        float src_y[4] = { vy, vy + sT, vy + vh - sB, vy + vh };

        float dst_x[4] = { dst.x, dst.x + sL, dst.right() - sR, dst.right() };
        float dst_y[4] = { dst.y, dst.y + sT, dst.bottom() - sB, dst.bottom() };

        for (int row = 0; row < 3; ++row) {
            for (int col = 0; col < 3; ++col) {
                float sx_min = src_x[col];
                float sx_max = src_x[col + 1];
                float sy_min = src_y[row];
                float sy_max = src_y[row + 1];
                float sw = sx_max - sx_min;
                float sh = sy_max - sy_min;

                float dx_min = dst_x[col];
                float dx_max = dst_x[col + 1];
                float dy_min = dst_y[row];
                float dy_max = dst_y[row + 1];
                float dw = dx_max - dx_min;
                float dh = dy_max - dy_min;

                if (sw <= 0.0f || sh <= 0.0f || dw <= 0.0f || dh <= 0.0f) continue;

                sk_canvas->save();
                sk_canvas->clipRect(SkRect::MakeLTRB(dx_min, dy_min, dx_max, dy_max), true);

                SkMatrix m;
                float scale_x = dw / sw;
                float scale_y = dh / sh;
                m.setScale(scale_x, scale_y);
                m.postTranslate(dx_min - sx_min * scale_x, dy_min - sy_min * scale_y);

                for (const auto& el : elements_) {
                    SkPath transformed;
                    el.path.transform(m, &transformed);
                    drawElement(sk_canvas, el, transformed, override_paint, is_stroke);
                }

                sk_canvas->restore();
            }
        }
    }
};

std::shared_ptr<SvgDocument> SvgDocument::parse(std::string_view input) {
    std::string s = trim(input);
    if (s.empty()) return nullptr;

    // 1. Check if input is a file path
    if (s.find('<') == std::string::npos && (s.ends_with(".svg") || s.find('/') != std::string::npos)) {
        std::ifstream file(s);
        if (file.is_open()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            s = trim(buffer.str());
        }
    }

    auto doc = std::make_shared<SvgDocumentImpl>();

    // 2. Direct SVG path format (e.g. "M 0 0 L 100 100 ... Z")
    if (s.find('<') == std::string::npos) {
        SkPath raw_path;
        if (SkParsePath::FromSVGString(s.c_str(), &raw_path)) {
            SvgElement el;
            el.path = raw_path;
            el.fill = Colors::White;
            doc->elements_.push_back(std::move(el));
            doc->view_box_ = raw_path.computeTightBounds();
            if (doc->view_box_.isEmpty()) {
                doc->view_box_ = SkRect::MakeWH(100.0f, 100.0f);
            }
            return doc;
        }
    }

    // 3. XML SVG document parsing
    // Extract viewBox
    size_t svg_pos = s.find("<svg");
    if (svg_pos != std::string::npos) {
        size_t svg_end = s.find('>', svg_pos);
        if (svg_end != std::string::npos) {
            std::string svg_header = s.substr(svg_pos, svg_end - svg_pos + 1);
            if (auto vb = getAttr(svg_header, "viewBox")) {
                std::string vb_str = *vb;
                std::replace(vb_str.begin(), vb_str.end(), ',', ' ');
                std::stringstream ss(vb_str);
                float x = 0, y = 0, w = 0, h = 0;
                if (ss >> x >> y >> w >> h) {
                    doc->view_box_ = SkRect::MakeXYWH(x, y, w, h);
                }
            } else {
                float w = 100.0f, h = 100.0f;
                if (auto sw = getAttr(svg_header, "width")) w = std::stof(*sw);
                if (auto sh = getAttr(svg_header, "height")) h = std::stof(*sh);
                doc->view_box_ = SkRect::MakeWH(w, h);
            }
        }
    }

    // Scan for tags: <path, <rect, <circle, <polygon
    size_t cur = 0;
    while (cur < s.size()) {
        size_t tag_start = s.find('<', cur);
        if (tag_start == std::string::npos) break;

        size_t tag_end = s.find('>', tag_start);
        if (tag_end == std::string::npos) break;

        std::string tag = s.substr(tag_start, tag_end - tag_start + 1);
        cur = tag_end + 1;

        if (tag.rfind("<path", 0) == 0) {
            if (auto d = getAttr(tag, "d")) {
                SkPath p;
                if (SkParsePath::FromSVGString(d->c_str(), &p)) {
                    SvgElement el;
                    el.path = std::move(p);
                    if (auto f = getAttr(tag, "fill")) el.fill = parseSvgColor(*f);
                    if (auto st = getAttr(tag, "stroke")) el.stroke = parseSvgColor(*st);
                    if (auto sw = getAttr(tag, "stroke-width")) {
                        try { el.stroke_width = std::stof(*sw); } catch (...) {}
                    }
                    doc->elements_.push_back(std::move(el));
                }
            }
        } else if (tag.rfind("<rect", 0) == 0) {
            float x = 0, y = 0, w = 0, h = 0, rx = 0, ry = 0;
            if (auto a = getAttr(tag, "x")) try { x = std::stof(*a); } catch (...) {}
            if (auto a = getAttr(tag, "y")) try { y = std::stof(*a); } catch (...) {}
            if (auto a = getAttr(tag, "width")) try { w = std::stof(*a); } catch (...) {}
            if (auto a = getAttr(tag, "height")) try { h = std::stof(*a); } catch (...) {}
            if (auto a = getAttr(tag, "rx")) try { rx = std::stof(*a); } catch (...) {}
            if (auto a = getAttr(tag, "ry")) try { ry = std::stof(*a); } catch (...) {}
            if (rx > 0.0f && ry == 0.0f) ry = rx;

            SkPath p;
            if (rx > 0.0f || ry > 0.0f) {
                p.addRoundRect(SkRect::MakeXYWH(x, y, w, h), rx, ry);
            } else {
                p.addRect(SkRect::MakeXYWH(x, y, w, h));
            }

            SvgElement el;
            el.path = std::move(p);
            if (auto f = getAttr(tag, "fill")) el.fill = parseSvgColor(*f);
            if (auto st = getAttr(tag, "stroke")) el.stroke = parseSvgColor(*st);
            if (auto sw = getAttr(tag, "stroke-width")) try { el.stroke_width = std::stof(*sw); } catch (...) {}
            doc->elements_.push_back(std::move(el));
        } else if (tag.rfind("<circle", 0) == 0) {
            float cx = 0, cy = 0, r = 0;
            if (auto a = getAttr(tag, "cx")) try { cx = std::stof(*a); } catch (...) {}
            if (auto a = getAttr(tag, "cy")) try { cy = std::stof(*a); } catch (...) {}
            if (auto a = getAttr(tag, "r")) try { r = std::stof(*a); } catch (...) {}

            SkPath p;
            p.addCircle(cx, cy, r);
            SvgElement el;
            el.path = std::move(p);
            if (auto f = getAttr(tag, "fill")) el.fill = parseSvgColor(*f);
            if (auto st = getAttr(tag, "stroke")) el.stroke = parseSvgColor(*st);
            if (auto sw = getAttr(tag, "stroke-width")) try { el.stroke_width = std::stof(*sw); } catch (...) {}
            doc->elements_.push_back(std::move(el));
        } else if (tag.rfind("<polygon", 0) == 0 || tag.rfind("<polyline", 0) == 0) {
            if (auto pts = getAttr(tag, "points")) {
                std::string pts_str = *pts;
                std::replace(pts_str.begin(), pts_str.end(), ',', ' ');
                std::stringstream ss(pts_str);
                SkPath p;
                float px = 0, py = 0;
                bool first = true;
                while (ss >> px >> py) {
                    if (first) {
                        p.moveTo(px, py);
                        first = false;
                    } else {
                        p.lineTo(px, py);
                    }
                }
                if (tag.rfind("<polygon", 0) == 0) p.close();

                SvgElement el;
                el.path = std::move(p);
                if (auto f = getAttr(tag, "fill")) el.fill = parseSvgColor(*f);
                if (auto st = getAttr(tag, "stroke")) el.stroke = parseSvgColor(*st);
                if (auto sw = getAttr(tag, "stroke-width")) try { el.stroke_width = std::stof(*sw); } catch (...) {}
                doc->elements_.push_back(std::move(el));
            }
        }
    }

    if (doc->view_box_.isEmpty()) {
        SkRect accum = SkRect::MakeEmpty();
        for (const auto& el : doc->elements_) {
            accum.join(el.path.computeTightBounds());
        }
        doc->view_box_ = accum.isEmpty() ? SkRect::MakeWH(100.0f, 100.0f) : accum;
    }

    return doc;
}

} // namespace enki
