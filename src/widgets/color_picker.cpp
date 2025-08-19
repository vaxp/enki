/// @file color_picker.cpp
/// @brief Implementation of Advanced ColorPicker for ENKI Framework.

#include "enki/widgets/color_picker.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/rendering/canvas.hpp"
#include "enki/rendering/paint.hpp"
#include "enki/state/state.hpp"
#include "enki/tree/build_context.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <vector>

namespace enki {

// ════════════════════════════════════════════════════════════════
// Color Conversion Helpers
// ════════════════════════════════════════════════════════════════

static Color HSVtoRGB(float h, float s, float v, uint8_t a = 255) {
    while (h < 0.0f) h += 360.0f;
    while (h >= 360.0f) h -= 360.0f;
    s = std::clamp(s, 0.0f, 1.0f);
    v = std::clamp(v, 0.0f, 1.0f);

    float c = v * s;
    float x = c * (1.0f - std::fabs(std::fmod(h / 60.0f, 2.0f) - 1.0f));
    float m = v - c;

    float r_prime = 0, g_prime = 0, b_prime = 0;
    if (h < 60.0f) {
        r_prime = c; g_prime = x; b_prime = 0;
    } else if (h < 120.0f) {
        r_prime = x; g_prime = c; b_prime = 0;
    } else if (h < 180.0f) {
        r_prime = 0; g_prime = c; b_prime = x;
    } else if (h < 240.0f) {
        r_prime = 0; g_prime = x; b_prime = c;
    } else if (h < 300.0f) {
        r_prime = x; g_prime = 0; b_prime = c;
    } else {
        r_prime = c; g_prime = 0; b_prime = x;
    }

    uint8_t r = static_cast<uint8_t>(std::clamp((r_prime + m) * 255.0f, 0.0f, 255.0f));
    uint8_t g = static_cast<uint8_t>(std::clamp((g_prime + m) * 255.0f, 0.0f, 255.0f));
    uint8_t b = static_cast<uint8_t>(std::clamp((b_prime + m) * 255.0f, 0.0f, 255.0f));

    return makeColorARGB(a, r, g, b);
}

static void RGBtoHSV(Color c, float& h, float& s, float& v, uint8_t& a) {
    a = (c >> 24) & 0xFF;
    float r = ((c >> 16) & 0xFF) / 255.0f;
    float g = ((c >> 8) & 0xFF) / 255.0f;
    float b = (c & 0xFF) / 255.0f;

    float cmax = std::max({r, g, b});
    float cmin = std::min({r, g, b});
    float delta = cmax - cmin;

    // Value
    v = cmax;

    // Saturation
    s = (cmax > 0.0001f) ? (delta / cmax) : 0.0f;

    // Hue
    if (delta < 0.0001f) {
        h = 0.0f;
    } else if (cmax == r) {
        h = 60.0f * std::fmod(((g - b) / delta), 6.0f);
        if (h < 0.0f) h += 360.0f;
    } else if (cmax == g) {
        h = 60.0f * (((b - r) / delta) + 2.0f);
    } else {
        h = 60.0f * (((r - g) / delta) + 4.0f);
    }
}

static std::string colorToHex(Color c, bool with_alpha = true) {
    uint8_t a = (c >> 24) & 0xFF;
    uint8_t r = (c >> 16) & 0xFF;
    uint8_t g = (c >> 8) & 0xFF;
    uint8_t b = c & 0xFF;

    std::ostringstream ss;
    ss << "#"
       << std::hex << std::uppercase << std::setfill('0')
       << std::setw(2) << static_cast<int>(r)
       << std::setw(2) << static_cast<int>(g)
       << std::setw(2) << static_cast<int>(b);
    if (with_alpha && a < 255) {
        ss << std::setw(2) << static_cast<int>(a);
    }
    return ss.str();
}

// ════════════════════════════════════════════════════════════════
// RenderColorSVBox (2D Saturation-Value Canvas)
// ════════════════════════════════════════════════════════════════

class RenderColorSVBox : public RenderBox {
public:
    float hue = 0.0f;
    float saturation = 1.0f;
    float value = 1.0f;

    RenderColorSVBox(float h, float s, float v) : hue(h), saturation(s), value(v) {
        ANUNodeStyleSetWidth(anu_node_, 240.0f);
        ANUNodeStyleSetHeight(anu_node_, 130.0f);
    }

    void paint(PaintContext& ctx) override {
        Rect bounds = Rect::fromLTWH(ctx.offset.x, ctx.offset.y, size_.width, size_.height);
        BorderRadius rad = BorderRadius::circular(8.0f);

        // 1. Solid Pure Hue Background
        Paint hue_paint;
        hue_paint.setColor(HSVtoRGB(hue, 1.0f, 1.0f, 255));
        ctx.canvas.drawRRect(bounds, rad, hue_paint);

        // 2. Horizontal White-to-Transparent Gradient (Saturation)
        Paint sat_paint;
        sat_paint.setShader(Gradient::linear(
            Point(bounds.x, bounds.y),
            Point(bounds.x + bounds.width, bounds.y),
            {0xFFFFFFFF, 0x00FFFFFF}
        ));
        ctx.canvas.drawRRect(bounds, rad, sat_paint);

        // 3. Vertical Transparent-to-Black Gradient (Value / Brightness)
        Paint val_paint;
        val_paint.setShader(Gradient::linear(
            Point(bounds.x, bounds.y),
            Point(bounds.x, bounds.y + bounds.height),
            {0x00000000, 0xFF000000}
        ));
        ctx.canvas.drawRRect(bounds, rad, val_paint);

        // 4. Reticle Selector Circle
        float thumb_x = bounds.x + std::clamp(saturation * bounds.width, 4.0f, bounds.width - 4.0f);
        float thumb_y = bounds.y + std::clamp((1.0f - value) * bounds.height, 4.0f, bounds.height - 4.0f);
        Point reticle_pt(thumb_x, thumb_y);

        Paint ring_paint;
        ring_paint.setColor(0xFFFFFFFF);
        ring_paint.setStyle(PaintStyle::Stroke);
        ring_paint.setStrokeWidth(2.5f);
        ctx.canvas.drawCircle(reticle_pt, 7.0f, ring_paint);

        Paint shadow_ring;
        shadow_ring.setColor(0xFF000000);
        shadow_ring.setStyle(PaintStyle::Stroke);
        shadow_ring.setStrokeWidth(1.0f);
        ctx.canvas.drawCircle(reticle_pt, 8.5f, shadow_ring);
    }
};

class ColorSVBoxWidget : public SingleChildRenderObjectWidget {
public:
    float hue;
    float saturation;
    float value;

    ColorSVBoxWidget(float h, float s, float v)
        : SingleChildRenderObjectWidget(Key::none(), nullptr), hue(h), saturation(s), value(v) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext&) override {
        return std::make_unique<RenderColorSVBox>(hue, saturation, value);
    }

    void updateRenderObject(BuildContext&, RenderObject& ro) override {
        auto& r = static_cast<RenderColorSVBox&>(ro);
        r.hue = hue;
        r.saturation = saturation;
        r.value = value;
        r.markNeedsPaint();
    }

    [[nodiscard]] std::string_view typeName() const override { return "ColorSVBoxWidget"; }
};

// ════════════════════════════════════════════════════════════════
// RenderHueBar (Hue Rainbow Slider Bar)
// ════════════════════════════════════════════════════════════════

class RenderHueBar : public RenderBox {
public:
    float hue = 0.0f;

    RenderHueBar(float h) : hue(h) {
        ANUNodeStyleSetWidth(anu_node_, 240.0f);
        ANUNodeStyleSetHeight(anu_node_, 12.0f);
    }

    void paint(PaintContext& ctx) override {
        Rect bounds = Rect::fromLTWH(ctx.offset.x, ctx.offset.y, size_.width, size_.height);
        BorderRadius rad = BorderRadius::circular(6.0f);

        // Rainbow Gradient
        Paint bar_paint;
        bar_paint.setShader(Gradient::linear(
            Point(bounds.x, bounds.y),
            Point(bounds.x + bounds.width, bounds.y),
            {
                0xFFFF0000, // Red (0°)
                0xFFFFFF00, // Yellow (60°)
                0xFF00FF00, // Green (120°)
                0xFF00FFFF, // Cyan (180°)
                0xFF0000FF, // Blue (240°)
                0xFFFF00FF, // Magenta (300°)
                0xFFFF0000  // Red (360°)
            }
        ));
        ctx.canvas.drawRRect(bounds, rad, bar_paint);

        // Thumb indicator
        float thumb_x = bounds.x + std::clamp((hue / 360.0f) * bounds.width, 4.0f, bounds.width - 4.0f);
        Rect thumb_rect = Rect::fromLTWH(thumb_x - 4.0f, bounds.y - 2.0f, 8.0f, bounds.height + 4.0f);

        Paint thumb_paint;
        thumb_paint.setColor(0xFFFFFFFF);
        ctx.canvas.drawRRect(thumb_rect, BorderRadius::circular(3.0f), thumb_paint);

        Paint border_paint;
        border_paint.setColor(0xFF0F172A);
        border_paint.setStyle(PaintStyle::Stroke);
        border_paint.setStrokeWidth(1.5f);
        ctx.canvas.drawRRect(thumb_rect, BorderRadius::circular(3.0f), border_paint);
    }
};

class HueBarWidget : public SingleChildRenderObjectWidget {
public:
    float hue;
    HueBarWidget(float h) : SingleChildRenderObjectWidget(Key::none(), nullptr), hue(h) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext&) override {
        return std::make_unique<RenderHueBar>(hue);
    }

    void updateRenderObject(BuildContext&, RenderObject& ro) override {
        auto& r = static_cast<RenderHueBar&>(ro);
        r.hue = hue;
        r.markNeedsPaint();
    }

    [[nodiscard]] std::string_view typeName() const override { return "HueBarWidget"; }
};

// ════════════════════════════════════════════════════════════════
// RenderAlphaBar (Alpha Opacity Slider Bar)
// ════════════════════════════════════════════════════════════════

class RenderAlphaBar : public RenderBox {
public:
    Color color = 0xFF38BDF8;
    uint8_t alpha = 255;

    RenderAlphaBar(Color c, uint8_t a) : color(c), alpha(a) {
        ANUNodeStyleSetWidth(anu_node_, 240.0f);
        ANUNodeStyleSetHeight(anu_node_, 12.0f);
    }

    void paint(PaintContext& ctx) override {
        Rect bounds = Rect::fromLTWH(ctx.offset.x, ctx.offset.y, size_.width, size_.height);
        BorderRadius rad = BorderRadius::circular(6.0f);

        // Checkered Background
        Paint bg_paint;
        bg_paint.setColor(0xFF334155);
        ctx.canvas.drawRRect(bounds, rad, bg_paint);

        // Alpha Gradient from Transparent to Solid Color
        Paint alpha_paint;
        Color solid_c = color | 0xFF000000;
        Color trans_c = color & 0x00FFFFFF;
        alpha_paint.setShader(Gradient::linear(
            Point(bounds.x, bounds.y),
            Point(bounds.x + bounds.width, bounds.y),
            {trans_c, solid_c}
        ));
        ctx.canvas.drawRRect(bounds, rad, alpha_paint);

        // Thumb indicator
        float thumb_x = bounds.x + std::clamp((alpha / 255.0f) * bounds.width, 4.0f, bounds.width - 4.0f);
        Rect thumb_rect = Rect::fromLTWH(thumb_x - 4.0f, bounds.y - 2.0f, 8.0f, bounds.height + 4.0f);

        Paint thumb_paint;
        thumb_paint.setColor(0xFFFFFFFF);
        ctx.canvas.drawRRect(thumb_rect, BorderRadius::circular(3.0f), thumb_paint);

        Paint border_paint;
        border_paint.setColor(0xFF0F172A);
        border_paint.setStyle(PaintStyle::Stroke);
        border_paint.setStrokeWidth(1.5f);
        ctx.canvas.drawRRect(thumb_rect, BorderRadius::circular(3.0f), border_paint);
    }
};

class AlphaBarWidget : public SingleChildRenderObjectWidget {
public:
    Color color;
    uint8_t alpha;
    AlphaBarWidget(Color c, uint8_t a)
        : SingleChildRenderObjectWidget(Key::none(), nullptr), color(c), alpha(a) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext&) override {
        return std::make_unique<RenderAlphaBar>(color, alpha);
    }

    void updateRenderObject(BuildContext&, RenderObject& ro) override {
        auto& r = static_cast<RenderAlphaBar&>(ro);
        r.color = color;
        r.alpha = alpha;
        r.markNeedsPaint();
    }

    [[nodiscard]] std::string_view typeName() const override { return "AlphaBarWidget"; }
};

// ════════════════════════════════════════════════════════════════
// ColorPicker State
// ════════════════════════════════════════════════════════════════

class ColorPickerState : public State {
private:
    bool is_popup_open_ = false;
    ColorFormat current_format_ = ColorFormat::HEX;

    Color initial_color_ = 0xFF38BDF8;
    Color current_color_ = 0xFF38BDF8;

    float hue_ = 199.0f;
    float saturation_ = 0.77f;
    float value_ = 0.97f;
    uint8_t alpha_ = 255;

public:
    void initState() override {
        State::initState();
        auto* w = static_cast<const ColorPickerWidget*>(widget());
        initial_color_ = w->props.initial_color;
        current_color_ = initial_color_;
        current_format_ = w->props.default_format;

        RGBtoHSV(current_color_, hue_, saturation_, value_, alpha_);
        wireController();
    }

    void didUpdateWidget(const Widget& old) override {
        State::didUpdateWidget(old);
        wireController();
    }

    void wireController() {
        auto* w = static_cast<const ColorPickerWidget*>(widget());
        if (w->props.controller) {
            w->props.controller->set_color_fn = [this](Color c) {
                current_color_ = c;
                RGBtoHSV(c, hue_, saturation_, value_, alpha_);
                setState([] {});
            };
            w->props.controller->open_fn = [this] {
                is_popup_open_ = true;
                setState([] {});
            };
            w->props.controller->close_fn = [this] {
                is_popup_open_ = false;
                setState([] {});
            };
            w->props.controller->get_color_fn = [this] { return current_color_; };
            w->props.controller->get_hex_fn = [this] { return colorToHex(current_color_); };
        }
    }

    void updateColorFromHSV() {
        current_color_ = HSVtoRGB(hue_, saturation_, value_, alpha_);
        auto* w = static_cast<const ColorPickerWidget*>(widget());
        if (w->props.on_color_changed) {
            w->props.on_color_changed(current_color_);
        }
        setState([] {});
    }

    // ── Build 2D Saturation-Value Canvas ──────────────────────────
    WidgetPtr buildSVCanvas() {
        auto sv_render = std::make_shared<ColorSVBoxWidget>(hue_, saturation_, value_);

        auto gd = std::make_shared<GestureDetector>(sv_render);
        gd->cursor_type = SystemCursor::Crosshair;

        auto handle_touch = [this](Point local_pt) {
            saturation_ = std::clamp(local_pt.x / 240.0f, 0.0f, 1.0f);
            value_ = std::clamp(1.0f - (local_pt.y / 130.0f), 0.0f, 1.0f);
            updateColorFromHSV();
        };

        gd->on_tap_up = [handle_touch](const TapUpDetails& d) {
            handle_touch(d.local_position);
        };
        gd->on_pan_update = [handle_touch](const DragUpdateDetails& d) {
            handle_touch(d.local_position);
        };

        return gd;
    }

    // ── Build Hue Bar Slider ──────────────────────────────────────
    WidgetPtr buildHueSlider() {
        auto hue_render = std::make_shared<HueBarWidget>(hue_);

        auto gd = std::make_shared<GestureDetector>(hue_render);
        gd->cursor_type = SystemCursor::Pointer;

        auto handle_touch = [this](Point local_pt) {
            hue_ = std::clamp((local_pt.x / 240.0f) * 360.0f, 0.0f, 360.0f);
            updateColorFromHSV();
        };

        gd->on_tap_up = [handle_touch](const TapUpDetails& d) {
            handle_touch(d.local_position);
        };
        gd->on_pan_update = [handle_touch](const DragUpdateDetails& d) {
            handle_touch(d.local_position);
        };

        return gd;
    }

    // ── Build Alpha Bar Slider ────────────────────────────────────
    WidgetPtr buildAlphaSlider() {
        auto alpha_render = std::make_shared<AlphaBarWidget>(current_color_, alpha_);

        auto gd = std::make_shared<GestureDetector>(alpha_render);
        gd->cursor_type = SystemCursor::Pointer;

        auto handle_touch = [this](Point local_pt) {
            alpha_ = static_cast<uint8_t>(std::clamp((local_pt.x / 240.0f) * 255.0f, 0.0f, 255.0f));
            updateColorFromHSV();
        };

        gd->on_tap_up = [handle_touch](const TapUpDetails& d) {
            handle_touch(d.local_position);
        };
        gd->on_pan_update = [handle_touch](const DragUpdateDetails& d) {
            handle_touch(d.local_position);
        };

        return gd;
    }

    // ── Build Color Inputs & Format Switcher ──────────────────────
    WidgetPtr buildFormatInputs() {
        auto makeInputPill = [](std::string label, std::string val) -> WidgetPtr {
            auto l = text(label);
            l->fontSize(9.5f).bold().color(0xFF64748B);
            auto v = text(val);
            v->fontSize(11.5f).bold().color(0xFFFFFFFF);

            auto c = column(std::vector<WidgetPtr>{v, l});
            c->alignItems(Align::Center).gap(StyleValue::point(2.0f));

            auto b = container(c);
            b->color(0xFF0F172A).border(0xFF334155, 1.0f).borderRadius(4.0f).paddingSymmetric(4.0f, 6.0f);
            return b;
        };

        std::vector<WidgetPtr> fields;

        if (current_format_ == ColorFormat::HEX) {
            auto hex_txt = text(colorToHex(current_color_));
            hex_txt->fontSize(12.5f).bold().color(0xFFFFFFFF);
            auto hex_box = container(hex_txt);
            hex_box->color(0xFF0F172A).border(0xFF0284C7, 1.0f).borderRadius(6.0f).paddingSymmetric(6.0f, 14.0f);
            fields.push_back(hex_box);
        } else if (current_format_ == ColorFormat::RGBA) {
            uint8_t r = (current_color_ >> 16) & 0xFF;
            uint8_t g = (current_color_ >> 8) & 0xFF;
            uint8_t b = current_color_ & 0xFF;
            fields.push_back(makeInputPill("R", std::to_string(r)));
            fields.push_back(makeInputPill("G", std::to_string(g)));
            fields.push_back(makeInputPill("B", std::to_string(b)));
            fields.push_back(makeInputPill("A", std::to_string(alpha_)));
        } else {
            fields.push_back(makeInputPill("H", std::to_string(static_cast<int>(hue_)) + "°"));
            fields.push_back(makeInputPill("S", std::to_string(static_cast<int>(saturation_ * 100)) + "%"));
            fields.push_back(makeInputPill("V", std::to_string(static_cast<int>(value_ * 100)) + "%"));
        }

        // Format Switcher Button
        std::string mode_lbl = (current_format_ == ColorFormat::HEX) ? "HEX" :
                               (current_format_ == ColorFormat::RGBA) ? "RGBA" : "HSV";
        auto sw_txt = text(mode_lbl + " ⮂");
        sw_txt->fontSize(11.0f).bold().color(0xFF38BDF8);
        auto sw_box = container(sw_txt);
        sw_box->color(0xFF0F172A).borderRadius(4.0f).paddingSymmetric(6.0f, 8.0f);

        auto sw_gd = std::make_shared<GestureDetector>(sw_box);
        sw_gd->cursor_type = SystemCursor::Pointer;
        sw_gd->on_tap_up = [this](const TapUpDetails&) {
            if (current_format_ == ColorFormat::HEX) current_format_ = ColorFormat::RGBA;
            else if (current_format_ == ColorFormat::RGBA) current_format_ = ColorFormat::HSV;
            else current_format_ = ColorFormat::HEX;
            setState([] {});
        };

        fields.push_back(sw_gd);

        auto r = row(fields);
        r->justifyContent(Justify::SpaceBetween).alignItems(Align::Center).width(StyleValue::percent(100.0f));
        return r;
    }

    // ── Build Swatch Palette ──────────────────────────────────────
    WidgetPtr buildPalette(const ColorPickerWidget* w) {
        std::vector<WidgetPtr> swatch_boxes;
        for (Color sw_color : w->props.palette) {
            auto b = container();
            b->color(sw_color).borderRadius(10.0f).width(20.0f).height(20.0f).border(0xFF334155, 1.0f);

            auto gd = std::make_shared<GestureDetector>(b);
            gd->cursor_type = SystemCursor::Pointer;
            gd->on_tap_up = [this, sw_color](const TapUpDetails&) {
                current_color_ = sw_color;
                RGBtoHSV(current_color_, hue_, saturation_, value_, alpha_);
                updateColorFromHSV();
            };

            swatch_boxes.push_back(gd);
        }

        auto pal_row = row(swatch_boxes);
        pal_row->gap(StyleValue::point(6.0f)).justifyContent(Justify::Center).width(StyleValue::percent(100.0f));
        return pal_row;
    }

    // ── Build Color Comparison Swatch ─────────────────────────────
    WidgetPtr buildComparisonSwatch() {
        auto old_box = container();
        old_box->color(initial_color_).width(24.0f).height(24.0f).borderRadius(4.0f);

        auto new_box = container();
        new_box->color(current_color_).width(24.0f).height(24.0f).borderRadius(4.0f);

        auto arr = text("➔");
        arr->fontSize(11.0f).color(0xFF94A3B8);

        std::vector<WidgetPtr> items = {old_box, arr, new_box};
        auto r = row(items);
        r->gap(StyleValue::point(6.0f)).alignItems(Align::Center);
        return r;
    }

    // ── Build Full Color Picker Card ──────────────────────────────
    WidgetPtr buildColorPickerCard(const ColorPickerWidget* w) {
        const auto& opts = w->props;

        // Header Title & Comparison
        auto t_txt = text("🎨 Color Inspector");
        t_txt->fontSize(13.0f).bold().color(0xFFFFFFFF);

        std::vector<WidgetPtr> head_items = {t_txt};
        if (opts.show_comparison) {
            head_items.push_back(buildComparisonSwatch());
        }
        auto head_row = row(head_items);
        head_row->justifyContent(Justify::SpaceBetween).alignItems(Align::Center).width(StyleValue::percent(100.0f));

        std::vector<WidgetPtr> card_items = {
            head_row,
            buildSVCanvas(),
            buildHueSlider()
        };

        if (opts.enable_alpha) {
            card_items.push_back(buildAlphaSlider());
        }

        card_items.push_back(buildFormatInputs());

        if (opts.show_palette && !opts.palette.empty()) {
            auto div = container();
            div->color(0xFF334155).height(1.0f).width(StyleValue::percent(100.0f));
            card_items.push_back(div);
            card_items.push_back(buildPalette(w));
        }

        auto col = column(card_items);
        col->gap(StyleValue::point(10.0f)).width(240.0f);

        auto card_box = container(col);
        card_box->color(opts.background_color)
                .border(opts.border_color, 1.0f)
                .borderRadius(10.0f)
                .paddingAll(14.0f)
                .shadow(BoxShadow(0x99000000, {0.0f, 8.0f}, 24.0f));

        return card_box;
    }

    WidgetPtr build(BuildContext&) override {
        auto* w = static_cast<const ColorPickerWidget*>(widget());
        const auto& opts = w->props;

        if (opts.mode == ColorPickerMode::Inline) {
            return buildColorPickerCard(w);
        }

        // Input Popup Mode (Color Well Button)
        auto color_dot = container();
        color_dot->color(current_color_).borderRadius(6.0f).width(20.0f).height(20.0f).border(0xFF475569, 1.0f);

        auto hex_txt = text(colorToHex(current_color_));
        hex_txt->fontSize(12.5f).bold().color(0xFFFFFFFF);

        auto chev_txt = text(is_popup_open_ ? "⌃" : "⌄");
        chev_txt->fontSize(12.0f).color(0xFF94A3B8);

        std::vector<WidgetPtr> in_items = {color_dot, hex_txt, chev_txt};
        auto in_row = row(in_items);
        in_row->justifyContent(Justify::SpaceBetween)
              .alignItems(Align::Center)
              .width(StyleValue::percent(100.0f));

        auto input_box = container(in_row);
        input_box->color(0xFF1E293B)
                 .border(is_popup_open_ ? opts.active_color : opts.border_color, 1.0f)
                 .borderRadius(8.0f)
                 .paddingSymmetric(8.0f, 12.0f)
                 .width(180.0f);

        auto input_gd = std::make_shared<GestureDetector>(input_box);
        input_gd->cursor_type = SystemCursor::Pointer;
        input_gd->on_tap_up = [this](const TapUpDetails&) {
            is_popup_open_ = !is_popup_open_;
            setState([] {});
        };

        std::vector<WidgetPtr> col_items = {input_gd};
        if (is_popup_open_) {
            col_items.push_back(buildColorPickerCard(w));
        }

        auto full_col = column(col_items);
        full_col->gap(StyleValue::point(8.0f));
        return full_col;
    }
};

std::unique_ptr<State> ColorPickerWidget::createState() {
    return std::make_unique<ColorPickerState>();
}

} // namespace enki
