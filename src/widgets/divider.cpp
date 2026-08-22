/// @file divider.cpp
/// @brief Advanced Divider & VerticalDivider implementation for ENKI Framework.

#include "enki/widgets/divider.hpp"
#include "enki/tree/build_context.hpp"
#include "enki/rendering/canvas.hpp"
#include "enki/rendering/paint.hpp"

#include <algorithm>
#include <cmath>

namespace enki {

// ════════════════════════════════════════════════════════════════
// Helpers
// ════════════════════════════════════════════════════════════════

/// Draw a dashed/dotted horizontal segment.
static void drawHDash(Canvas& canvas, float x0, float x1, float cy,
                      float thickness, float dash_len, float gap, bool dotted,
                      Color color, bool round_caps) {
    if (x0 >= x1) return;
    Paint p;
    p.setColor(color);
    p.setStyle(PaintStyle::Fill);
    p.setAntiAlias(true);

    float seg = dotted ? thickness : dash_len;
    float step = seg + gap;
    for (float x = x0; x < x1; x += step) {
        float xe = std::min(x + seg, x1);
        Rect r = Rect::fromLTWH(x, cy - thickness * 0.5f, xe - x, thickness);
        if (round_caps) {
            canvas.drawRRect(r, BorderRadius::circular(thickness * 0.5f), p);
        } else {
            canvas.drawRect(r, p);
        }
    }
}

/// Draw a dashed/dotted vertical segment.
static void drawVDash(Canvas& canvas, float cx, float y0, float y1,
                      float thickness, float dash_len, float gap, bool dotted,
                      Color color, bool round_caps) {
    if (y0 >= y1) return;
    Paint p;
    p.setColor(color);
    p.setStyle(PaintStyle::Fill);
    p.setAntiAlias(true);

    float seg = dotted ? thickness : dash_len;
    float step = seg + gap;
    for (float y = y0; y < y1; y += step) {
        float ye = std::min(y + seg, y1);
        Rect r = Rect::fromLTWH(cx - thickness * 0.5f, y, thickness, ye - y);
        if (round_caps) {
            canvas.drawRRect(r, BorderRadius::circular(thickness * 0.5f), p);
        } else {
            canvas.drawRect(r, p);
        }
    }
}

// ════════════════════════════════════════════════════════════════
// RenderDivider (Horizontal)
// ════════════════════════════════════════════════════════════════

class RenderDivider : public RenderBox {
public:
    DividerProps options;

    explicit RenderDivider(DividerProps opt) : options(std::move(opt)) {
        ANUNodeStyleSetWidthPercent(anu_node_, 100.0f);
        ANUNodeStyleSetHeight(anu_node_, options.height);
    }

    void updateOptions(const DividerProps& o) {
        if (options.height != o.height) {
            ANUNodeStyleSetHeight(anu_node_, o.height);
            markNeedsLayout();
        }
        options = o;
        markNeedsPaint();
    }

    void paint(PaintContext& ctx) override {
        if (options.thickness <= 0.0f) return;

        float cy   = ctx.offset.y + size_.height * 0.5f;
        float x0   = ctx.offset.x + options.indent;
        float x1   = ctx.offset.x + size_.width - options.end_indent;
        if (x0 >= x1) return;

        float t    = options.thickness;
        bool  rc   = options.round_caps;
        Color col  = options.color;

        // ── Label in center (horizontal only) ───────────────────
        float lbl_w = 0.0f;
        if (!options.label.empty()) {
            lbl_w = ctx.canvas.measureText(options.label, options.label_font_size) + options.label_padding * 2.0f;
        }
        float cx_mid = (x0 + x1) * 0.5f;
        float lbl_x0 = cx_mid - lbl_w * 0.5f;
        float lbl_x1 = cx_mid + lbl_w * 0.5f;

        // Left segment
        float seg_x1_left  = options.label.empty() ? x1 : lbl_x0;
        // Right segment
        float seg_x0_right = options.label.empty() ? x1 : lbl_x1;

        auto drawSegment = [&](float sx0, float sx1) {
            if (sx0 >= sx1) return;

            if (options.style == DividerStyle::Gradient || options.gradient_fade) {
                // Fade from transparent → color → transparent
                Paint gp;
                gp.setShader(Gradient::linear(
                    {sx0, cy}, {sx1, cy},
                    {0x00000000 | (col & 0x00FFFFFF), col, 0x00000000 | (col & 0x00FFFFFF)},
                    {0.0f, 0.5f, 1.0f}
                ));
                gp.setStyle(PaintStyle::Fill);
                gp.setAntiAlias(true);
                Rect r = Rect::fromLTWH(sx0, cy - t * 0.5f, sx1 - sx0, t);
                ctx.canvas.drawRect(r, gp);
            } else if (options.style == DividerStyle::Dashed) {
                drawHDash(ctx.canvas, sx0, sx1, cy, t, options.dash_length, options.dash_gap, false, col, rc);
            } else if (options.style == DividerStyle::Dotted) {
                drawHDash(ctx.canvas, sx0, sx1, cy, t, t, options.dash_gap, true, col, rc);
            } else {
                // Solid
                Paint sp;
                sp.setColor(col);
                sp.setStyle(PaintStyle::Fill);
                sp.setAntiAlias(true);
                Rect r = Rect::fromLTWH(sx0, cy - t * 0.5f, sx1 - sx0, t);
                if (rc) ctx.canvas.drawRRect(r, BorderRadius::circular(t * 0.5f), sp);
                else    ctx.canvas.drawRect(r, sp);
            }
        };

        drawSegment(x0, seg_x1_left);
        drawSegment(seg_x0_right, x1);

        // ── Center label ────────────────────────────────────────
        if (!options.label.empty()) {
            // Background pill behind text
            Rect bg_rect = Rect::fromLTWH(lbl_x0, cy - options.label_font_size * 0.85f,
                                          lbl_w, options.label_font_size * 1.6f);
            Paint bg_p; bg_p.setColor(options.label_bg_color);
            ctx.canvas.drawRRect(bg_rect, BorderRadius::circular(3.0f), bg_p);

            // Label text
            Paint tp; tp.setColor(options.label_color);
            float text_y = cy + options.label_font_size * 0.35f;
            float tw = ctx.canvas.measureText(options.label, options.label_font_size);
            ctx.canvas.drawText(options.label, {cx_mid - tw * 0.5f, text_y}, tp, options.label_font_size);
        }
    }
};

// ════════════════════════════════════════════════════════════════
// RenderVerticalDivider
// ════════════════════════════════════════════════════════════════

class RenderVerticalDivider : public RenderBox {
public:
    DividerProps options;

    explicit RenderVerticalDivider(DividerProps opt) : options(std::move(opt)) {
        ANUNodeStyleSetHeightPercent(anu_node_, 100.0f);
        ANUNodeStyleSetWidth(anu_node_, options.height);
    }

    void updateOptions(const DividerProps& o) {
        if (options.height != o.height) {
            ANUNodeStyleSetWidth(anu_node_, o.height);
            markNeedsLayout();
        }
        options = o;
        markNeedsPaint();
    }

    void paint(PaintContext& ctx) override {
        if (options.thickness <= 0.0f) return;

        float cx   = ctx.offset.x + size_.width * 0.5f;
        float y0   = ctx.offset.y + options.indent;
        float y1   = ctx.offset.y + size_.height - options.end_indent;
        if (y0 >= y1) return;

        float t   = options.thickness;
        bool  rc  = options.round_caps;
        Color col = options.color;

        auto drawSegment = [&](float sy0, float sy1) {
            if (sy0 >= sy1) return;

            if (options.style == DividerStyle::Gradient || options.gradient_fade) {
                Paint gp;
                gp.setShader(Gradient::linear(
                    {cx, sy0}, {cx, sy1},
                    {0x00000000 | (col & 0x00FFFFFF), col, 0x00000000 | (col & 0x00FFFFFF)},
                    {0.0f, 0.5f, 1.0f}
                ));
                gp.setStyle(PaintStyle::Fill);
                gp.setAntiAlias(true);
                Rect r = Rect::fromLTWH(cx - t * 0.5f, sy0, t, sy1 - sy0);
                ctx.canvas.drawRect(r, gp);
            } else if (options.style == DividerStyle::Dashed) {
                drawVDash(ctx.canvas, cx, sy0, sy1, t, options.dash_length, options.dash_gap, false, col, rc);
            } else if (options.style == DividerStyle::Dotted) {
                drawVDash(ctx.canvas, cx, sy0, sy1, t, t, options.dash_gap, true, col, rc);
            } else {
                // Solid
                Paint sp;
                sp.setColor(col);
                sp.setStyle(PaintStyle::Fill);
                sp.setAntiAlias(true);
                Rect r = Rect::fromLTWH(cx - t * 0.5f, sy0, t, sy1 - sy0);
                if (rc) ctx.canvas.drawRRect(r, BorderRadius::circular(t * 0.5f), sp);
                else    ctx.canvas.drawRect(r, sp);
            }
        };

        drawSegment(y0, y1);
    }
};

// ════════════════════════════════════════════════════════════════
// Widget implementations
// ════════════════════════════════════════════════════════════════

std::unique_ptr<RenderObject> DividerWidget::createRenderObject(BuildContext&) {
    return std::make_unique<RenderDivider>(options);
}

void DividerWidget::updateRenderObject(BuildContext&, RenderObject& ro) {
    static_cast<RenderDivider&>(ro).updateOptions(options);
}

std::unique_ptr<RenderObject> VerticalDividerWidget::createRenderObject(BuildContext&) {
    return std::make_unique<RenderVerticalDivider>(options);
}

void VerticalDividerWidget::updateRenderObject(BuildContext&, RenderObject& ro) {
    static_cast<RenderVerticalDivider&>(ro).updateOptions(options);
}

} // namespace enki
