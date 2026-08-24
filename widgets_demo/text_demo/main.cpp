/// @file main.cpp
/// @brief ENKI Typography & Text Widgets Interactive Showcase (SkParagraph + Anu Flexbox).
/// Demonstrates Text, RichText, InlineSpans, Font Weights, Decorations, Shadows, Wrapping, and Shell UI.

#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/state/state.hpp"
#include "enki/rendering/canvas.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <memory>

using namespace enki;

// ════════════════════════════════════════════════════════════════
// Interactive Clickable Button Widget
// ════════════════════════════════════════════════════════════════

class RenderClickable : public RenderDecoratedBox {
public:
    std::function<void()> on_click_;
    bool is_hovered_ = false;
    bool is_pressed_ = false;

    RenderClickable(BoxDecoration dec, FlexboxStyle style, std::function<void()> onClick)
        : RenderDecoratedBox(std::move(dec), std::move(style)), on_click_(std::move(onClick)) {}

    void handlePointerEnter(const PointerEvent&) override {
        is_hovered_ = true;
        markNeedsPaint();
    }

    void handlePointerExit(const PointerEvent&) override {
        is_hovered_ = false;
        is_pressed_ = false;
        markNeedsPaint();
    }

    void handlePointerDown(const PointerEvent&) override {
        is_pressed_ = true;
        markNeedsPaint();
    }

    void handlePointerUp(const PointerEvent&) override {
        if (is_pressed_ && on_click_) {
            on_click_();
        }
        is_pressed_ = false;
        markNeedsPaint();
    }

    SystemCursor cursor() const override { return SystemCursor::Pointer; }
};

class Clickable : public SingleChildRenderObjectWidget {
public:
    BoxDecoration         decoration;
    FlexboxStyle          style;
    std::function<void()> on_click;

    Clickable(Key key, BoxDecoration dec, FlexboxStyle s, WidgetPtr child, std::function<void()> onClick)
        : SingleChildRenderObjectWidget(std::move(key), std::move(child)),
          decoration(std::move(dec)), style(std::move(s)), on_click(std::move(onClick)) {}

    std::unique_ptr<RenderObject> createRenderObject(BuildContext&) override {
        return std::make_unique<RenderClickable>(decoration, style, on_click);
    }

    void updateRenderObject(BuildContext&, RenderObject& ro) override {
        if (auto* rc = dynamic_cast<RenderClickable*>(&ro)) {
            rc->setDecoration(decoration);
            rc->setStyle(style);
            rc->on_click_ = on_click;
            rc->markNeedsPaint();
            rc->markNeedsLayout();
        }
    }

    std::string_view typeName() const override { return "Clickable"; }
};

inline WidgetPtr tabButton(std::string labelStr, bool active, std::function<void()> onClick) {
    BoxDecoration dec;
    dec.border_radius = BorderRadius::circular(10.0f);
    dec.border = Border(active ? 0xFF818CF8 : 0x30FFFFFF, active ? 1.5f : 1.0f);

    if (active) {
        dec.gradient = GradientConfig::linear({0xFF4F46E5, 0xFF6366F1});
        dec.box_shadow = { BoxShadow(0x604F46E5, {0, 4}, 12, 0) };
    } else {
        dec.color = 0x251E293B;
    }

    FlexboxStyle s;
    s.padding = StyleInsets::symmetric(8.0f, 18.0f);
    s.margin = StyleInsets::only(0, 10.0f, 0, 0);

    auto btnText = text({
        .text = labelStr,
        .color = active ? 0xFFFFFFFF : 0xFF94A3B8,
        .font_size = 13.0f,
        .font_weight = active ? FontWeight::Bold : FontWeight::Normal,
    });

    return std::make_shared<Clickable>(Key::string(labelStr), dec, s, btnText, std::move(onClick));
}

// ════════════════════════════════════════════════════════════════
// Card / Container Helper
// ════════════════════════════════════════════════════════════════

inline std::shared_ptr<Container> typographyCard(std::string titleStr, std::string descStr, WidgetPtr content) {
    auto titleWidget = text({
        .text = titleStr,
        .color = 0xFFF1F5F9,
        .font_size = 15.0f,
        .font_weight = FontWeight::Bold,
    });

    auto descWidget = text({
        .text = std::move(descStr),
        .color = 0xFF64748B,
        .font_size = 12.0f,
    });

    auto headerCol = column({
        .children = { titleWidget, descWidget }
    });

    auto cardCol = column({
        .children = { headerCol, content }
    });

    auto c = container(Key::string("card_" + titleStr), cardCol);
    c->color(0x301E293B)
     .borderRadius(14.0f)
     .border(0x40334155, 1.0f)
     .paddingAll(16.0f)
     .margin(EdgeInsets::only(0, 0, 14.0f, 0))
     .shadow(0x30000000, {0, 4}, 10.0f);

    return c;
}

// ════════════════════════════════════════════════════════════════
// Tab 1: Typography Scale, Weights & Styles
// ════════════════════════════════════════════════════════════════

WidgetPtr buildHierarchyView() {
    // 1. Typographic Scale Card
    auto dispL = text({
        .text = "Display Large — 30px Bold Glow",
        .color = 0xFF38BDF8,
        .font_size = 30.0f,
        .font_weight = FontWeight::Bold,
        .shadows = { BoxShadow(0x8038BDF8, {0, 0}, 12.0f) },
    });

    auto dispM = text({
        .text = "Headline Medium — 22px Bold",
        .color = 0xFFFFFFFF,
        .font_size = 22.0f,
        .font_weight = FontWeight::Bold,
    });

    auto titleL = text({
        .text = "Title Large — 18px SemiBold Primary",
        .color = 0xFFE2E8F0,
        .font_size = 18.0f,
        .font_weight = FontWeight::SemiBold,
    });

    auto bodyL = text({
        .text = "Body Large — 15px Regular: Fast hardware accelerated text rendering via SkParagraph.",
        .color = 0xFFCBD5E1,
        .font_size = 15.0f,
    });

    auto bodyM = text({
        .text = "Body Medium — 13px Muted: Clean typographic hierarchy with subpixel glyph positioning.",
        .color = 0xFF94A3B8,
        .font_size = 13.0f,
    });

    auto caption = text({
        .text = "CAPTION / OVERLINE — 11px Bold Letter Spaced",
        .color = 0xFF818CF8,
        .font_size = 11.0f,
        .font_weight = FontWeight::Bold,
        .letter_spacing = 1.5f,
    });

    auto mono = text({
        .text = "Code Mono — const auto textNode = make_paragraph(); // 12px",
        .color = 0xFF34D399,
        .font_size = 12.0f,
    });

    auto scaleCol = column({
        .children = { dispL, dispM, titleL, bodyL, bodyM, caption, mono }
    });

    auto scaleCard = typographyCard(
        "1. Typographic Scale & Hierarchy",
        "From Display headlines to code tokens, crisp at any DPI scale",
        scaleCol
    );

    // 2. Font Weights Card
    auto w100 = text({ .text = "Thin (100) — Elegant ultra-light weight", .color = 0xFFE2E8F0, .font_size = 14.0f, .font_weight = FontWeight::Thin });
    auto w300 = text({ .text = "Light (300) — Subtly light text", .color = 0xFFE2E8F0, .font_size = 14.0f, .font_weight = FontWeight::Light });
    auto w400 = text({ .text = "Regular (400) — Standard body text", .color = 0xFFE2E8F0, .font_size = 14.0f, .font_weight = FontWeight::Regular });
    auto w500 = text({ .text = "Medium (500) — Slightly emphasized body", .color = 0xFFE2E8F0, .font_size = 14.0f, .font_weight = FontWeight::Medium });
    auto w600 = text({ .text = "SemiBold (600) — Section headers & UI buttons", .color = 0xFFE2E8F0, .font_size = 14.0f, .font_weight = FontWeight::SemiBold });
    auto w700 = text({ .text = "Bold (700) — Strong prominent titles", .color = 0xFFE2E8F0, .font_size = 14.0f, .font_weight = FontWeight::Bold });
    auto w900 = text({ .text = "Black (900) — Maximum emphasis impactful headlines", .color = 0xFFE2E8F0, .font_size = 14.0f, .font_weight = FontWeight::Black });

    auto weightsCol = column({
        .children = { w100, w300, w400, w500, w600, w700, w900 }
    });

    auto weightsCard = typographyCard(
        "2. Complete Font Weight Range (100 to 900)",
        "Dynamic glyph selection backed by FontConfig on Linux",
        weightsCol
    );

    // 3. Decorations & Shadows Card
    auto dec1 = text({
        .text = "Solid Underline in Neon Cyan",
        .style = TextStyle{
            .color = 0xFFFFFFFF,
            .font_size = 14.0f,
            .decoration = TextDecoration::Underline,
            .decoration_color = 0xFF38BDF8,
            .decoration_style = TextDecorationStyle::Solid,
            .decoration_thickness = 2.0f,
        }
    });

    auto dec2 = text({
        .text = "Wavy Underline in Warning Amber",
        .style = TextStyle{
            .color = 0xFFFFFFFF,
            .font_size = 14.0f,
            .decoration = TextDecoration::Underline,
            .decoration_color = 0xFFF59E0B,
            .decoration_style = TextDecorationStyle::Wavy,
            .decoration_thickness = 1.5f,
        }
    });

    auto dec3 = text({
        .text = "Line Through / Strikethrough in Danger Rose",
        .style = TextStyle{
            .color = 0xFF94A3B8,
            .font_size = 14.0f,
            .decoration = TextDecoration::LineThrough,
            .decoration_color = 0xFFF43F5E,
            .decoration_style = TextDecorationStyle::Solid,
            .decoration_thickness = 2.0f,
        }
    });

    auto dec4 = text({
        .text = "Cyberpunk Glowing Neon Multi-Shadow",
        .color = 0xFFEC4899,
        .font_size = 16.0f,
        .font_weight = FontWeight::Bold,
        .shadows = {
            BoxShadow(0xFFFF007F, {0, 0}, 10.0f),
            BoxShadow(0xFF8B5CF6, {0, 4}, 16.0f)
        }
    });

    auto decCol = column({
        .children = { dec1, dec2, dec3, dec4 }
    });

    auto decCard = typographyCard(
        "3. Text Decorations & Glow Shadows",
        "Wavy, solid, strikethrough lines and multi-layered glow shadows",
        decCol
    );

    return column(Key::string("tab_hierarchy_view"), {
        .children = { scaleCard, weightsCard, decCard }
    });
}

// ════════════════════════════════════════════════════════════════
// Tab 2: RichText & Inline Spans
// ════════════════════════════════════════════════════════════════

WidgetPtr buildRichTextView() {
    // 1. Code Syntax Highlighting Block
    auto codeSpan = span({
        .text = "",
        .style = TextStyle{ .color = 0xFFE2E8F0, .font_size = 13.0f },
        .children = {
            span("// ENKI UI Tree Builder Example\n", TextStyle{ .color = 0xFF64748B, .font_size = 13.0f, .font_style = FontStyle::Italic }),
            span("auto ", TextStyle{ .color = 0xFF818CF8, .font_size = 13.0f, .font_weight = FontWeight::Bold }),
            span("buildCard", TextStyle{ .color = 0xFFFBBF24, .font_size = 13.0f, .font_weight = FontWeight::Bold }),
            span("() -> ", TextStyle{ .color = 0xFF94A3B8, .font_size = 13.0f }),
            span("WidgetPtr ", TextStyle{ .color = 0xFF38BDF8, .font_size = 13.0f, .font_weight = FontWeight::Bold }),
            span("{\n    ", TextStyle{ .color = 0xFFE2E8F0, .font_size = 13.0f }),
            span("return ", TextStyle{ .color = 0xFFF43F5E, .font_size = 13.0f, .font_weight = FontWeight::Bold }),
            span("container", TextStyle{ .color = 0xFF38BDF8, .font_size = 13.0f }),
            span("(\n        ", TextStyle{ .color = 0xFFE2E8F0, .font_size = 13.0f }),
            span("text", TextStyle{ .color = 0xFF38BDF8, .font_size = 13.0f }),
            span("({ .text = \"Hello SkParagraph Engine!\", .font_size = 18.0f })\n", TextStyle{ .color = 0xFF34D399, .font_size = 13.0f }),
            span("    );\n}", TextStyle{ .color = 0xFFE2E8F0, .font_size = 13.0f })
        }
    });

    auto codeWidget = richText(codeSpan);
    auto codeBox = container(codeWidget);
    codeBox->color(0xFF0F172A)
           .borderRadius(10.0f)
           .border(0xFF1E293B, 1.0f)
           .paddingAll(14.0f);

    auto codeCard = typographyCard(
        "1. Real-Time RichText Syntax Highlighting",
        "Multiple inline spans nested inside a single SkParagraph instance",
        codeBox
    );

    // 2. Formatted Article / Mixed Badges Card
    auto articleSpan = span({
        .text = "ENKI Framework delivers ",
        .style = TextStyle{ .color = 0xFFCBD5E1, .font_size = 14.0f },
        .children = {
            span("blazing-fast ", TextStyle{ .color = 0xFF38BDF8, .font_size = 14.0f, .font_weight = FontWeight::Bold }),
            span("desktop shell performance. Combining ", TextStyle{ .color = 0xFFCBD5E1, .font_size = 14.0f }),
            span("Skia GPU Rasterization ", TextStyle{ .color = 0xFF34D399, .font_size = 14.0f, .font_weight = FontWeight::Bold }),
            span("with ", TextStyle{ .color = 0xFFCBD5E1, .font_size = 14.0f }),
            span("Anu Flexbox Layout ", TextStyle{ .color = 0xFFF59E0B, .font_size = 14.0f, .font_weight = FontWeight::Bold }),
            span("and ", TextStyle{ .color = 0xFFCBD5E1, .font_size = 14.0f }),
            span("HarfBuzz text shaping", TextStyle{ .color = 0xFFEC4899, .font_size = 14.0f, .font_weight = FontWeight::Bold, .font_style = FontStyle::Italic }),
            span(" allows building fluid 120 FPS Wayland/X11 desktops with zero stutter.", TextStyle{ .color = 0xFFCBD5E1, .font_size = 14.0f })
        }
    });

    auto articleWidget = richText(articleSpan);
    auto articleBox = container(articleWidget);
    articleBox->paddingAll(12.0f)
              .color(0x203B82F6)
              .borderRadius(10.0f)
              .border(0x403B82F6, 1.0f);

    auto articleCard = typographyCard(
        "2. Mixed Typography Paragraph",
        "Rich text with inline colored emphasis, bolding, and italics",
        articleBox
    );

    // 3. BiDi & Unicode Multilingual Card
    auto multiSpan = span({
        .text = "",
        .style = TextStyle{ .color = 0xFFFFFFFF, .font_size = 14.0f },
        .children = {
            span("English: ", TextStyle{ .color = 0xFF818CF8, .font_size = 14.0f, .font_weight = FontWeight::Bold }),
            span("Modern Linux Desktop Shell\n", TextStyle{ .color = 0xFFE2E8F0, .font_size = 14.0f }),
            span("Arabic (العربية): ", TextStyle{ .color = 0xFF34D399, .font_size = 14.0f, .font_weight = FontWeight::Bold }),
            span("محرك رسم فائق السرعة يدعم اللغة العربية بالكامل\n", TextStyle{ .color = 0xFFF1F5F9, .font_size = 14.0f }),
            span("Japanese (日本語): ", TextStyle{ .color = 0xFFFBBF24, .font_size = 14.0f, .font_weight = FontWeight::Bold }),
            span("次世代のデスクトップ環境向けレンダリングエンジン\n", TextStyle{ .color = 0xFFE2E8F0, .font_size = 14.0f }),
            span("Symbols & Emojis: ", TextStyle{ .color = 0xFFEC4899, .font_size = 14.0f, .font_weight = FontWeight::Bold }),
            span("⚡ 🚀 🎨 💎 🐧 🛡️ 🎧 🌌", TextStyle{ .font_size = 16.0f })
        }
    });

    auto multiWidget = richText(multiSpan);
    auto multiBox = container(multiWidget);
    multiBox->paddingAll(12.0f)
            .color(0x201E293B)
            .borderRadius(10.0f)
            .border(0x40334155, 1.0f);

    auto multiCard = typographyCard(
        "3. International Unicode & BiDi Text",
        "Seamless shaping for Arabic, Asian, Latin scripts and emojis",
        multiBox
    );

    return column(Key::string("tab_richtext_view"), {
        .children = { codeCard, articleCard, multiCard }
    });
}

// ════════════════════════════════════════════════════════════════
// Tab 3: Wrapping, Overflow & Alignment
// ════════════════════════════════════════════════════════════════

WidgetPtr buildWrappingView() {
    std::string sampleLong = "The Anu Flexbox layout engine measures SkParagraph intrinsic geometry dynamically and wraps text smoothly across multiple lines.";

    // 1. Constrained Cards Comparison
    auto t1 = text({ .text = sampleLong, .color = 0xFF94A3B8, .font_size = 12.0f });
    auto h1 = text({ .text = "Width: 200px", .color = 0xFF38BDF8, .font_size = 11.0f, .font_weight = FontWeight::Bold });
    auto card1 = container(column({ .children = { h1, t1 } }));
    card1->width(200.0f).color(0x200F172A).borderRadius(8.0f).paddingAll(10.0f).border(0x4038BDF8, 1.0f);

    auto t2 = text({ .text = sampleLong, .color = 0xFF94A3B8, .font_size = 12.0f });
    auto h2 = text({ .text = "Width: 280px", .color = 0xFF34D399, .font_size = 11.0f, .font_weight = FontWeight::Bold });
    auto card2 = container(column({ .children = { h2, t2 } }));
    card2->width(280.0f).color(0x200F172A).borderRadius(8.0f).paddingAll(10.0f).border(0x4034D399, 1.0f).margin(EdgeInsets::only(0, 0, 0, 12.0f));

    auto t3 = text({ .text = sampleLong, .color = 0xFF94A3B8, .font_size = 12.0f });
    auto h3 = text({ .text = "Width: 420px", .color = 0xFFFBBF24, .font_size = 11.0f, .font_weight = FontWeight::Bold });
    auto card3 = container(column({ .children = { h3, t3 } }));
    card3->width(420.0f).color(0x200F172A).borderRadius(8.0f).paddingAll(10.0f).border(0x40FBBF24, 1.0f).margin(EdgeInsets::only(0, 0, 0, 12.0f));

    auto rowWrapping = row({
        .children = { card1, card2, card3 }
    });

    auto wrapCard = typographyCard(
        "1. Dynamic Multiline Text Reflow",
        "Text adapts naturally to its parent container width constraint",
        rowWrapping
    );

    // 2. MaxLines and Ellipsis Card
    auto e1 = text({
        .text = "This is a single line that overflows and gets an ellipsis at the exact boundary of its container without breaking layout.",
        .color = 0xFFE2E8F0,
        .font_size = 13.0f,
        .overflow = TextOverflow::Ellipsis,
        .max_lines = 1,
    });
    auto eh1 = text({ .text = "maxLines(1) + ellipsis()", .color = 0xFF818CF8, .font_size = 11.0f, .font_weight = FontWeight::Bold });
    auto e1Box = container(column({ .children = { eh1, e1 } }));
    e1Box->width(400.0f).color(0x251E293B).borderRadius(8.0f).paddingAll(10.0f).margin(EdgeInsets::only(0, 0, 8.0f, 0));

    auto e2 = text({
        .text = "This is a two-line clamped paragraph. When text exceeds the second line, SkParagraph neatly truncates the string and appends a clean three-dot ellipsis character at the trailing end.",
        .color = 0xFFCBD5E1,
        .font_size = 13.0f,
        .overflow = TextOverflow::Ellipsis,
        .max_lines = 2,
    });
    auto eh2 = text({ .text = "maxLines(2) + ellipsis()", .color = 0xFF34D399, .font_size = 11.0f, .font_weight = FontWeight::Bold });
    auto e2Box = container(column({ .children = { eh2, e2 } }));
    e2Box->width(400.0f).color(0x251E293B).borderRadius(8.0f).paddingAll(10.0f);

    auto ellipsisCol = column({
        .children = { e1Box, e2Box }
    });

    auto ellipsisCard = typographyCard(
        "2. Line Clamping & Ellipsis Truncation",
        "Clamps text precisely to N lines and adds ellipsis overflow",
        ellipsisCol
    );

    // 3. Text Alignments Card
    auto aLeft = text({
        .text = "Left Aligned Text: Elements align to the leading edge of the layout container.",
        .color = 0xFF94A3B8,
        .font_size = 12.0f,
        .text_align = TextAlign::Left,
    });

    auto aCenter = text({
        .text = "Center Aligned Text: Symmetrically centered text for banners and titles.",
        .color = 0xFF38BDF8,
        .font_size = 12.0f,
        .text_align = TextAlign::Center,
    });

    auto aRight = text({
        .text = "Right Aligned Text: Useful for timestamps, status values, and numerical columns.",
        .color = 0xFFFBBF24,
        .font_size = 12.0f,
        .text_align = TextAlign::Right,
    });

    auto alignCol = column({
        .children = { aLeft, aCenter, aRight }
    });

    auto alignCard = typographyCard(
        "3. Text Alignments (Left, Center, Right, Justify)",
        "Horizontal justification and alignment inside any bounding box",
        alignCol
    );

    return column(Key::string("tab_wrapping_view"), {
        .children = { wrapCard, ellipsisCard, alignCard }
    });
}

// ════════════════════════════════════════════════════════════════
// Tab 4: Real-World Desktop Shell UI Typography
// ════════════════════════════════════════════════════════════════

WidgetPtr buildShellShowcaseView() {
    // 1. Notification Toast Card
    auto notifIconText = text({ .text = "🔔", .font_size = 18.0f });
    auto notifIcon = container(notifIconText);
    notifIcon->size(38.0f, 38.0f)
             .color(0x303B82F6)
             .borderRadius(10.0f)
             .align(Alignment::Center);

    auto notifAppName = text({
        .text = "SYSTEM NOTIFICATION",
        .color = 0xFF818CF8,
        .font_size = 10.0f,
        .font_weight = FontWeight::Bold,
        .letter_spacing = 1.0f,
    });

    auto notifTime = text({ .text = "Just now", .color = 0xFF64748B, .font_size = 10.0f });

    auto notifHeaderRow = row({
        .children = { notifAppName, spacer(), notifTime }
    });

    auto notifTitle = text({
        .text = "System Update 2.4.0 Available",
        .color = 0xFFFFFFFF,
        .font_size = 14.0f,
        .font_weight = FontWeight::Bold,
    });

    auto notifBody = text({
        .text = "New Skia GPU pipeline and Wayland Fractional Scaling improvements are ready to install.",
        .color = 0xFF94A3B8,
        .font_size = 12.0f,
        .overflow = TextOverflow::Ellipsis,
        .max_lines = 2,
    });

    auto notifCol = column({
        .flex_grow = 1.0f,
        .children = { notifHeaderRow, notifTitle, notifBody }
    });

    auto notifBodyContainer = container(notifCol);
    notifBodyContainer->margin(EdgeInsets::only(0, 0, 0, 12.0f)).flexGrow(1.0f);

    auto notifContent = row({
        .align_items = Align::Center,
        .children = { notifIcon, notifBodyContainer }
    });

    auto notifCard = container(notifContent);
    notifCard->color(0xE61E293B)
             .borderRadius(14.0f)
             .border(0x40818CF8, 1.5f)
             .paddingAll(14.0f)
             .shadow(0x60000000, {0, 8}, 20.0f)
             .margin(EdgeInsets::only(0, 0, 14.0f, 0));

    // 2. Media Player Shell Card
    auto albumArtText = text({ .text = "🎵", .font_size = 24.0f });
    auto albumArt = container(albumArtText);
    albumArt->size(54.0f, 54.0f)
            .color(0x30EC4899)
            .borderRadius(12.0f)
            .border(0x50EC4899, 1.0f)
            .align(Alignment::Center);

    auto trackTitle = text({
        .text = "Resonance (Slowed + Reverb)",
        .color = 0xFFFFFFFF,
        .font_size = 15.0f,
        .font_weight = FontWeight::Bold,
        .shadows = { BoxShadow(0x60EC4899, {0, 2}, 6.0f) },
    });

    auto trackArtist = text({ .text = "HOME — Odyssey (Deluxe Remaster)", .color = 0xFFF472B6, .font_size = 12.0f });

    auto trackTime = text({ .text = "02:45 / 03:32", .color = 0xFF94A3B8, .font_size = 11.0f });

    auto mediaInfoCol = column({
        .flex_grow = 1.0f,
        .children = { trackTitle, trackArtist, trackTime }
    });

    auto mediaInfoContainer = container(mediaInfoCol);
    mediaInfoContainer->margin(EdgeInsets::only(0, 0, 0, 14.0f)).flexGrow(1.0f);

    auto mediaRow = row({
        .align_items = Align::Center,
        .children = { albumArt, mediaInfoContainer }
    });

    auto mediaCard = container(mediaRow);
    mediaCard->color(0xE61E1B4B)
             .borderRadius(14.0f)
             .border(0x40EC4899, 1.5f)
             .paddingAll(14.0f)
             .shadow(0x60000000, {0, 8}, 20.0f)
             .margin(EdgeInsets::only(0, 0, 14.0f, 0));

    // 3. Quick System Monitor Card
    auto cpuLabel = text({ .text = "CPU LOAD", .color = 0xFF64748B, .font_size = 10.0f, .font_weight = FontWeight::Bold, .letter_spacing = 1.0f });
    auto cpuVal = text({ .text = "18.4%", .color = 0xFF34D399, .font_size = 20.0f, .font_weight = FontWeight::Bold });
    auto cpuStat = container(column({ .children = { cpuLabel, cpuVal } }));
    cpuStat->color(0x250F172A).borderRadius(10.0f).paddingAll(12.0f).flexGrow(1.0f);

    auto ramLabel = text({ .text = "MEMORY USED", .color = 0xFF64748B, .font_size = 10.0f, .font_weight = FontWeight::Bold, .letter_spacing = 1.0f });
    auto ramVal = text({ .text = "4.2 / 32 GB", .color = 0xFF38BDF8, .font_size = 20.0f, .font_weight = FontWeight::Bold });
    auto ramStat = container(column({ .children = { ramLabel, ramVal } }));
    ramStat->color(0x250F172A).borderRadius(10.0f).paddingAll(12.0f).margin(EdgeInsets::only(0, 0, 0, 10.0f)).flexGrow(1.0f);

    auto gpuLabel = text({ .text = "GPU ENGINE", .color = 0xFF64748B, .font_size = 10.0f, .font_weight = FontWeight::Bold, .letter_spacing = 1.0f });
    auto gpuVal = text({ .text = "Vulkan 120 FPS", .color = 0xFFFBBF24, .font_size = 18.0f, .font_weight = FontWeight::Bold });
    auto gpuStat = container(column({ .children = { gpuLabel, gpuVal } }));
    gpuStat->color(0x250F172A).borderRadius(10.0f).paddingAll(12.0f).margin(EdgeInsets::only(0, 0, 0, 10.0f)).flexGrow(1.0f);

    auto statsRow = row({
        .children = { cpuStat, ramStat, gpuStat }
    });

    auto sysCard = typographyCard(
        "3. Real-Time Shell Metrics & Status Cards",
        "High-contrast statistical typography for dock, panels, and taskbars",
        statsRow
    );

    return column({
        .children = { notifCard, mediaCard, sysCard }
    });
}

// ════════════════════════════════════════════════════════════════
// Root Text Demo App State & Application Widget
// ════════════════════════════════════════════════════════════════

class TextDemoState : public State {
public:
    int current_tab_ = 0; // 0: Hierarchy, 1: RichText, 2: Wrapping, 3: Shell Showcase

    WidgetPtr build(BuildContext& ctx) override {
        // 1. Header Bar
        auto title = text({
            .text = "⚡ ENKI ENGINE — ADVANCED TYPOGRAPHY DEMO",
            .color = 0xFFFFFFFF,
            .font_size = 18.0f,
            .font_weight = FontWeight::Bold,
            .shadows = { BoxShadow(0x8038BDF8, {0, 0}, 8.0f) },
        });

        auto sub = text({
            .text = "SkParagraph Text Layout + HarfBuzz Font Shaping + Anu Flexbox Integration",
            .color = 0xFF818CF8,
            .font_size = 12.0f,
        });

        auto badgeText = text({
            .text = "● SKPARAGRAPH GPU READY",
            .color = 0xFF34D399,
            .font_size = 10.0f,
            .font_weight = FontWeight::Bold,
        });

        auto badge = container(badgeText);
        badge->color(0x2010B981)
             .borderRadius(20.0f)
             .border(0x5010B981, 1.0f)
             .paddingSymmetric(4.0f, 12.0f);

        auto titleCol = column({
            .children = { title, sub }
        });

        auto titleRow = row({
            .align_items = Align::Center,
            .children = { titleCol, spacer(), badge }
        });

        auto tabs = row({
            .children = {
                tabButton("🔤 1. Hierarchy & Styles", current_tab_ == 0, [this] {
                    setState([this] { current_tab_ = 0; });
                }),
                tabButton("🌈 2. RichText & Spans", current_tab_ == 1, [this] {
                    setState([this] { current_tab_ = 1; });
                }),
                tabButton("📐 3. Wrapping & Overflow", current_tab_ == 2, [this] {
                    setState([this] { current_tab_ = 2; });
                }),
                tabButton("🖥️ 4. Shell UI Showcase", current_tab_ == 3, [this] {
                    setState([this] { current_tab_ = 3; });
                })
            },
            .key = Key::string("tabs_navigation_row")
        });

        // 3. Tab Body Content
        WidgetPtr bodyContent = nullptr;
        if (current_tab_ == 0) {
            bodyContent = buildHierarchyView();
        } else if (current_tab_ == 1) {
            bodyContent = buildRichTextView();
        } else if (current_tab_ == 2) {
            bodyContent = buildWrappingView();
        } else {
            bodyContent = buildShellShowcaseView();
        }

        // 4. Main Column Layout
        auto tabsBox = container(tabs);
        tabsBox->margin(EdgeInsets::symmetric(14.0f, 0));

        auto mainCol = column({
            .children = { titleRow, tabsBox, bodyContent }
        });

        auto appRoot = container(mainCol);
        appRoot->color(0xFF0B0F19)
               .paddingAll(20.0f)
               .flexGrow(1.0f);

        return appRoot;
    }
};

class TextDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<TextDemoState>();
    }
    std::string_view typeName() const override { return "TextDemoApp"; }
};

int main() {
    std::cout << "================================================\n";
    std::cout << "  ENKI Engine — Typography & Text Widgets Demo   \n";
    std::cout << "  SkParagraph + Anu Flexbox Interactive Showcase \n";
    std::cout << "================================================\n";

    AppConfig config;
    config.title       = "Enki Engine — Typography & SkParagraph Demo";
    config.width       = 1080;
    config.height      = 680;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 0; // Uncapped max speed
    config.show_performance_overlay = true; // Display real-time FPS & Frame Time HUD
    config.clear_color = 0xFF0B0F19;

    return runApp(std::make_shared<TextDemoApp>(), config);
}
