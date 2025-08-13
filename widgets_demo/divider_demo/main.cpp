/// @file main.cpp
/// @brief ENKI Advanced Divider & VerticalDivider Showcase Demo.

#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/divider.hpp"
#include "enki/state/state.hpp"

#include <iostream>
using namespace enki;

// ── Section heading helper ───────────────────────────────────────────────────
static WidgetPtr sectionLabel(const std::string& s) {
    auto t = text(s); t->fontSize(11.5f).bold().color(0xFF64748B);
    return t;
}

// ── Demo content row label ───────────────────────────────────────────────────
static WidgetPtr rowLabel(const std::string& s) {
    auto t = text(s); t->fontSize(13.0f).color(0xFFCBD5E1);
    return t;
}

// ── Card wrapper ─────────────────────────────────────────────────────────────
static WidgetPtr card(std::vector<WidgetPtr> items, float w = 560.0f) {
    auto col = column(items);
    col->gap(StyleValue::point(2.0f));
    auto c = container(col);
    c->color(0xFF1E293B).borderRadius(12.0f).paddingAll(20.0f).width(w);
    return c;
}

class DividerDemoWidget : public StatelessWidget {
public:
    std::string_view typeName() const override { return "DividerDemoWidget"; }

    WidgetPtr build(BuildContext&) override {

        // ── Page header ──────────────────────────────────────────────────────
        auto title = text("Advanced Divider & VerticalDivider Suite");
        title->fontSize(22.0f).bold().color(0xFFFFFFFF);
        auto sub = text("Solid · Dashed · Dotted · Gradient · Center Label · Round Caps");
        sub->fontSize(13.0f).color(0xFF94A3B8);
        auto hdr = column(std::vector<WidgetPtr>{title, sub});
        hdr->alignItems(Align::Center).gap(StyleValue::point(4.0f));

        // ═══════════════════════════════════════
        // Card 1 — Horizontal Dividers
        // ═══════════════════════════════════════

        // Solid
        DividerOptions solid;
        solid.color     = 0xFF334155;
        solid.thickness = 1.0f;

        // Solid thick accent
        DividerOptions accent;
        accent.color     = 0xFF38BDF8;
        accent.thickness = 2.0f;

        // Dashed slate
        DividerOptions dashed;
        dashed.style      = DividerStyle::Dashed;
        dashed.color      = 0xFF64748B;
        dashed.thickness  = 1.5f;
        dashed.dash_length = 8.0f;
        dashed.dash_gap    = 5.0f;

        // Dotted amber
        DividerOptions dotted;
        dotted.style      = DividerStyle::Dotted;
        dotted.color      = 0xFFF59E0B;
        dotted.thickness  = 3.0f;
        dotted.dash_gap   = 5.0f;

        // Gradient fade violet
        DividerOptions grad;
        grad.style      = DividerStyle::Gradient;
        grad.color      = 0xFF8B5CF6;
        grad.thickness  = 2.0f;

        // Gradient fade emerald with indent
        DividerOptions grad2;
        grad2.style      = DividerStyle::Gradient;
        grad2.color      = 0xFF10B981;
        grad2.thickness  = 2.5f;
        grad2.indent     = 40.0f;
        grad2.end_indent = 40.0f;

        // Dashed round caps
        DividerOptions dashed_rc;
        dashed_rc.style       = DividerStyle::Dashed;
        dashed_rc.color       = 0xFFEC4899;
        dashed_rc.thickness   = 4.0f;
        dashed_rc.dash_length = 10.0f;
        dashed_rc.dash_gap    = 6.0f;
        dashed_rc.round_caps  = true;

        // With center label
        DividerOptions with_label;
        with_label.color          = 0xFF475569;
        with_label.thickness      = 1.0f;
        with_label.label          = "OR";
        with_label.label_color    = 0xFF94A3B8;
        with_label.label_font_size = 11.0f;
        with_label.label_bg_color  = 0xFF1E293B;

        DividerOptions with_label2;
        with_label2.style          = DividerStyle::Dashed;
        with_label2.color          = 0xFF38BDF8;
        with_label2.thickness      = 1.0f;
        with_label2.label          = "Section Break";
        with_label2.label_color    = 0xFF38BDF8;
        with_label2.label_font_size = 10.5f;
        with_label2.label_bg_color  = 0xFF1E293B;
        with_label2.dash_length     = 6.0f;
        with_label2.dash_gap        = 4.0f;

        auto h_card = card({
            sectionLabel("HORIZONTAL DIVIDERS"),
            divider({.height=20}),
            rowLabel("Default Solid (1px slate)"),      divider(solid),
            rowLabel("Solid Accent (2px sky-400)"),     divider(accent),
            rowLabel("Dashed (slate, 8/5)"),            divider(dashed),
            rowLabel("Dotted (amber, 3px round)"),      divider(dotted),
            rowLabel("Gradient Fade (violet)"),         divider(grad),
            rowLabel("Gradient Fade + indent (emerald)"), divider(grad2),
            rowLabel("Dashed Round Caps (pink, 4px)"),  divider(dashed_rc),
            rowLabel("Center Label \"OR\""),            divider(with_label),
            rowLabel("Dashed + Label \"Section Break\""), divider(with_label2),
        });

        // ═══════════════════════════════════════
        // Card 2 — Vertical Dividers side-by-side
        // ═══════════════════════════════════════

        auto makeVPanel = [](const std::string& lbl, WidgetPtr vd) {
            auto t = text(lbl); t->fontSize(10.0f).color(0xFF64748B);
            auto col = column(std::vector<WidgetPtr>{t, vd});
            col->alignItems(Align::Center).gap(StyleValue::point(6.0f));
            auto c = container(col);
            c->paddingSymmetric(8.0f, 6.0f);
            return c;
        };

        DividerOptions vs; vs.color = 0xFF334155; vs.thickness = 1.0f; vs.height = 20.0f;

        DividerOptions vsolid; vsolid.color = 0xFF38BDF8; vsolid.thickness = 2.0f; vsolid.height = 20.0f;

        DividerOptions vdashed; vdashed.style = DividerStyle::Dashed; vdashed.color = 0xFF8B5CF6;
        vdashed.thickness = 1.5f; vdashed.dash_length = 8.0f; vdashed.dash_gap = 4.0f; vdashed.height = 20.0f;

        DividerOptions vdotted; vdotted.style = DividerStyle::Dotted; vdotted.color = 0xFFF59E0B;
        vdotted.thickness = 3.0f; vdotted.dash_gap = 5.0f; vdotted.height = 20.0f;

        DividerOptions vgrad; vgrad.style = DividerStyle::Gradient; vgrad.color = 0xFF10B981;
        vgrad.thickness = 2.0f; vgrad.height = 20.0f;

        DividerOptions vrc; vrc.style = DividerStyle::Dashed; vrc.color = 0xFFEC4899;
        vrc.thickness = 4.0f; vrc.dash_length = 10.0f; vrc.dash_gap = 5.0f;
        vrc.round_caps = true; vrc.height = 20.0f;

        auto v_row = row(std::vector<WidgetPtr>{
            makeVPanel("Default",  verticalDivider(vs)),
            makeVPanel("Sky-400",  verticalDivider(vsolid)),
            makeVPanel("Dashed",   verticalDivider(vdashed)),
            makeVPanel("Dotted",   verticalDivider(vdotted)),
            makeVPanel("Gradient", verticalDivider(vgrad)),
            makeVPanel("Rounded",  verticalDivider(vrc)),
        });
        v_row->gap(StyleValue::point(8.0f)).alignItems(Align::Stretch);

        auto vcard = card({
            sectionLabel("VERTICAL DIVIDERS (height = parent height)"),
            v_row,
        });

        // ── Page layout ──────────────────────────────────────────────────────
        auto page = column(std::vector<WidgetPtr>{hdr, h_card, vcard});
        page->gap(StyleValue::point(20.0f)).alignItems(Align::Center);

        auto bg = container(page);
        bg->color(0xFF0B1120).paddingAll(28.0f)
           .width(StyleValue::percent(100.0f))
           .height(StyleValue::percent(100.0f));
        return bg;
    }
};

int main() {
    std::cout << "====================================================\n";
    std::cout << "  ENKI — Advanced Divider & VerticalDivider Demo\n";
    std::cout << "====================================================\n";

    AppConfig cfg;
    cfg.title       = "Enki — Divider & VerticalDivider Demo";
    cfg.width       = 680;
    cfg.height      = 820;
    cfg.resizable   = true;
    cfg.vsync       = false;
    cfg.target_fps  = 60;
    cfg.clear_color = 0xFF0B1120;

    return runApp(std::make_shared<DividerDemoWidget>(), cfg);
}
