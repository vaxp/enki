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

    auto btnText = text(labelStr);
    btnText->fontSize(13.0f)
           .color(active ? 0xFFFFFFFF : 0xFF94A3B8);
    if (active) btnText->bold();

    return std::make_shared<Clickable>(Key::string(labelStr), dec, s, btnText, std::move(onClick));
}

// ════════════════════════════════════════════════════════════════
// Card / Container Helper
// ════════════════════════════════════════════════════════════════

inline std::shared_ptr<Container> typographyCard(std::string titleStr, std::string descStr, WidgetPtr content) {
    auto titleWidget = text(titleStr);
    titleWidget->fontSize(15.0f).bold().color(0xFFF1F5F9);

    auto descWidget = text(std::move(descStr));
    descWidget->fontSize(12.0f).color(0xFF64748B);

    auto headerCol = column({
        titleWidget,
        descWidget
    });

    auto cardCol = column({
        headerCol,
        content
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
    auto dispL = text("Display Large — 30px Bold Glow");
    dispL->fontSize(30.0f).bold().color(0xFF38BDF8).shadow(0x8038BDF8, {0, 0}, 12.0f);

    auto dispM = text("Headline Medium — 22px Bold");
    dispM->fontSize(22.0f).bold().color(0xFFFFFFFF);

    auto titleL = text("Title Large — 18px SemiBold Primary");
    titleL->fontSize(18.0f).fontWeight(FontWeight::SemiBold).color(0xFFE2E8F0);

    auto bodyL = text("Body Large — 15px Regular: Fast hardware accelerated text rendering via SkParagraph.");
    bodyL->fontSize(15.0f).color(0xFFCBD5E1);

    auto bodyM = text("Body Medium — 13px Muted: Clean typographic hierarchy with subpixel glyph positioning.");
    bodyM->fontSize(13.0f).color(0xFF94A3B8);

    auto caption = text("CAPTION / OVERLINE — 11px Bold Letter Spaced");
    caption->fontSize(11.0f).bold().letterSpacing(1.5f).color(0xFF818CF8);

    auto mono = text("Code Mono — const auto textNode = make_paragraph(); // 12px");
    mono->fontSize(12.0f).color(0xFF34D399);

    auto scaleCol = column({
        dispL,
        dispM,
        titleL,
        bodyL,
        bodyM,
        caption,
        mono
    });

    auto scaleCard = typographyCard(
        "1. Typographic Scale & Hierarchy",
        "From Display headlines to code tokens, crisp at any DPI scale",
        scaleCol
    );

    // 2. Font Weights Card
    auto w100 = text("Thin (100) — Elegant ultra-light weight");
    w100->fontSize(14.0f).fontWeight(FontWeight::Thin).color(0xFFE2E8F0);

    auto w300 = text("Light (300) — Subtly light text");
    w300->fontSize(14.0f).fontWeight(FontWeight::Light).color(0xFFE2E8F0);

    auto w400 = text("Regular (400) — Standard body text");
    w400->fontSize(14.0f).fontWeight(FontWeight::Regular).color(0xFFE2E8F0);

    auto w500 = text("Medium (500) — Slightly emphasized body");
    w500->fontSize(14.0f).fontWeight(FontWeight::Medium).color(0xFFE2E8F0);

    auto w600 = text("SemiBold (600) — Section headers & UI buttons");
    w600->fontSize(14.0f).fontWeight(FontWeight::SemiBold).color(0xFFE2E8F0);

    auto w700 = text("Bold (700) — Strong prominent titles");
    w700->fontSize(14.0f).fontWeight(FontWeight::Bold).color(0xFFE2E8F0);

    auto w900 = text("Black (900) — Maximum emphasis impactful headlines");
    w900->fontSize(14.0f).fontWeight(FontWeight::Black).color(0xFFE2E8F0);

    auto weightsCol = column({
        w100, w300, w400, w500, w600, w700, w900
    });

    auto weightsCard = typographyCard(
        "2. Complete Font Weight Range (100 to 900)",
        "Dynamic glyph selection backed by FontConfig on Linux",
        weightsCol
    );

    // 3. Decorations & Shadows Card
    auto dec1 = text("Solid Underline in Neon Cyan");
    dec1->fontSize(14.0f).color(0xFFFFFFFF).setStyle(
        TextStyle().setColor(0xFFFFFFFF).setFontSize(14.0f).setDecoration(TextDecoration::Underline, 0xFF38BDF8, TextDecorationStyle::Solid, 2.0f)
    );

    auto dec2 = text("Wavy Underline in Warning Amber");
    dec2->fontSize(14.0f).color(0xFFFFFFFF).setStyle(
        TextStyle().setColor(0xFFFFFFFF).setFontSize(14.0f).setDecoration(TextDecoration::Underline, 0xFFF59E0B, TextDecorationStyle::Wavy, 1.5f)
    );

    auto dec3 = text("Line Through / Strikethrough in Danger Rose");
    dec3->fontSize(14.0f).color(0xFFFFFFFF).setStyle(
        TextStyle().setColor(0xFF94A3B8).setFontSize(14.0f).setDecoration(TextDecoration::LineThrough, 0xFFF43F5E, TextDecorationStyle::Solid, 2.0f)
    );

    auto dec4 = text("Cyberpunk Glowing Neon Multi-Shadow");
    dec4->fontSize(16.0f).bold().color(0xFFEC4899)
        .shadow(0xFFFF007F, {0, 0}, 10.0f)
        .shadow(0xFF8B5CF6, {0, 4}, 16.0f);

    auto decCol = column({
        dec1, dec2, dec3, dec4
    });

    auto decCard = typographyCard(
        "3. Text Decorations & Glow Shadows",
        "Wavy, solid, strikethrough lines and multi-layered glow shadows",
        decCol
    );

    return column(Key::string("tab_hierarchy_view"), {
        scaleCard,
        weightsCard,
        decCard
    });
}

// ════════════════════════════════════════════════════════════════
// Tab 2: RichText & Inline Spans
// ════════════════════════════════════════════════════════════════

WidgetPtr buildRichTextView() {
    // 1. Code Syntax Highlighting Block
    auto codeSpan = span("", TextStyle().setColor(0xFFE2E8F0).setFontSize(13.0f), {
        span("// ENKI UI Tree Builder Example\n", TextStyle().setColor(0xFF64748B).italic().setFontSize(13.0f)),
        span("auto ", TextStyle().setColor(0xFF818CF8).bold().setFontSize(13.0f)),
        span("buildCard", TextStyle().setColor(0xFFFBBF24).bold().setFontSize(13.0f)),
        span("() -> ", TextStyle().setColor(0xFF94A3B8).setFontSize(13.0f)),
        span("WidgetPtr ", TextStyle().setColor(0xFF38BDF8).bold().setFontSize(13.0f)),
        span("{\n    ", TextStyle().setColor(0xFFE2E8F0).setFontSize(13.0f)),
        span("return ", TextStyle().setColor(0xFFF43F5E).bold().setFontSize(13.0f)),
        span("container", TextStyle().setColor(0xFF38BDF8).setFontSize(13.0f)),
        span("(\n        ", TextStyle().setColor(0xFFE2E8F0).setFontSize(13.0f)),
        span("text", TextStyle().setColor(0xFF38BDF8).setFontSize(13.0f)),
        span("(\"", TextStyle().setColor(0xFFE2E8F0).setFontSize(13.0f)),
        span("Hello SkParagraph Engine!", TextStyle().setColor(0xFF34D399).setFontSize(13.0f)),
        span("\")\n            ->", TextStyle().setColor(0xFFE2E8F0).setFontSize(13.0f)),
        span("fontSize", TextStyle().setColor(0xFFFBBF24).setFontSize(13.0f)),
        span("(", TextStyle().setColor(0xFFE2E8F0).setFontSize(13.0f)),
        span("18.0f", TextStyle().setColor(0xFFA78BFA).setFontSize(13.0f)),
        span(")\n            .", TextStyle().setColor(0xFFE2E8F0).setFontSize(13.0f)),
        span("bold", TextStyle().setColor(0xFFFBBF24).setFontSize(13.0f)),
        span("()\n    );\n}", TextStyle().setColor(0xFFE2E8F0).setFontSize(13.0f))
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
    auto articleSpan = span("ENKI Framework delivers ", TextStyle().setColor(0xFFCBD5E1).setFontSize(14.0f), {
        span("blazing-fast ", TextStyle().setColor(0xFF38BDF8).bold().setFontSize(14.0f)),
        span("desktop shell performance. Combining ", TextStyle().setColor(0xFFCBD5E1).setFontSize(14.0f)),
        span("Skia GPU Rasterization ", TextStyle().setColor(0xFF34D399).bold().setFontSize(14.0f)),
        span("with ", TextStyle().setColor(0xFFCBD5E1).setFontSize(14.0f)),
        span("Anu Flexbox Layout ", TextStyle().setColor(0xFFF59E0B).bold().setFontSize(14.0f)),
        span("and ", TextStyle().setColor(0xFFCBD5E1).setFontSize(14.0f)),
        span("HarfBuzz text shaping", TextStyle().setColor(0xFFEC4899).bold().italic().setFontSize(14.0f)),
        span(" allows building fluid 120 FPS Wayland/X11 desktops with zero stutter.", TextStyle().setColor(0xFFCBD5E1).setFontSize(14.0f))
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
    auto multiSpan = span("", TextStyle().setColor(0xFFFFFFFF).setFontSize(14.0f), {
        span("English: ", TextStyle().setColor(0xFF818CF8).bold().setFontSize(14.0f)),
        span("Modern Linux Desktop Shell\n", TextStyle().setColor(0xFFE2E8F0).setFontSize(14.0f)),
        span("Arabic (العربية): ", TextStyle().setColor(0xFF34D399).bold().setFontSize(14.0f)),
        span("محرك رسم فائق السرعة يدعم اللغة العربية بالكامل\n", TextStyle().setColor(0xFFF1F5F9).setFontSize(14.0f)),
        span("Japanese (日本語): ", TextStyle().setColor(0xFFFBBF24).bold().setFontSize(14.0f)),
        span("次世代のデスクトップ環境向けレンダリングエンジン\n", TextStyle().setColor(0xFFE2E8F0).setFontSize(14.0f)),
        span("Symbols & Emojis: ", TextStyle().setColor(0xFFEC4899).bold().setFontSize(14.0f)),
        span("⚡ 🚀 🎨 💎 🐧 🛡️ 🎧 🌌", TextStyle().setFontSize(16.0f))
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
        codeCard,
        articleCard,
        multiCard
    });
}

// ════════════════════════════════════════════════════════════════
// Tab 3: Wrapping, Overflow & Alignment
// ════════════════════════════════════════════════════════════════

WidgetPtr buildWrappingView() {
    std::string sampleLong = "The Anu Flexbox layout engine measures SkParagraph intrinsic geometry dynamically and wraps text smoothly across multiple lines.";

    // 1. Constrained Cards Comparison
    auto t1 = text(sampleLong);
    t1->fontSize(12.0f).color(0xFF94A3B8);
    auto h1 = text("Width: 200px");
    h1->fontSize(11.0f).bold().color(0xFF38BDF8);
    auto card1 = container(column({h1, t1}));
    card1->width(200.0f).color(0x200F172A).borderRadius(8.0f).paddingAll(10.0f).border(0x4038BDF8, 1.0f);

    auto t2 = text(sampleLong);
    t2->fontSize(12.0f).color(0xFF94A3B8);
    auto h2 = text("Width: 280px");
    h2->fontSize(11.0f).bold().color(0xFF34D399);
    auto card2 = container(column({h2, t2}));
    card2->width(280.0f).color(0x200F172A).borderRadius(8.0f).paddingAll(10.0f).border(0x4034D399, 1.0f).margin(EdgeInsets::only(0, 0, 0, 12.0f));

    auto t3 = text(sampleLong);
    t3->fontSize(12.0f).color(0xFF94A3B8);
    auto h3 = text("Width: 420px");
    h3->fontSize(11.0f).bold().color(0xFFFBBF24);
    auto card3 = container(column({h3, t3}));
    card3->width(420.0f).color(0x200F172A).borderRadius(8.0f).paddingAll(10.0f).border(0x40FBBF24, 1.0f).margin(EdgeInsets::only(0, 0, 0, 12.0f));

    auto rowWrapping = row({
        card1, card2, card3
    });

    auto wrapCard = typographyCard(
        "1. Dynamic Multiline Text Reflow",
        "Text adapts naturally to its parent container width constraint",
        rowWrapping
    );

    // 2. MaxLines and Ellipsis Card
    auto e1 = text("This is a single line that overflows and gets an ellipsis at the exact boundary of its container without breaking layout.");
    e1->fontSize(13.0f).maxLines(1).ellipsis().color(0xFFE2E8F0);
    auto eh1 = text("maxLines(1) + ellipsis()");
    eh1->fontSize(11.0f).bold().color(0xFF818CF8);
    auto e1Box = container(column({eh1, e1}));
    e1Box->width(400.0f).color(0x251E293B).borderRadius(8.0f).paddingAll(10.0f).margin(EdgeInsets::only(0, 0, 8.0f, 0));

    auto e2 = text("This is a two-line clamped paragraph. When text exceeds the second line, SkParagraph neatly truncates the string and appends a clean three-dot ellipsis character at the trailing end.");
    e2->fontSize(13.0f).maxLines(2).ellipsis().color(0xFFCBD5E1);
    auto eh2 = text("maxLines(2) + ellipsis()");
    eh2->fontSize(11.0f).bold().color(0xFF34D399);
    auto e2Box = container(column({eh2, e2}));
    e2Box->width(400.0f).color(0x251E293B).borderRadius(8.0f).paddingAll(10.0f);

    auto ellipsisCol = column({
        e1Box,
        e2Box
    });

    auto ellipsisCard = typographyCard(
        "2. Line Clamping & Ellipsis Truncation",
        "Clamps text precisely to N lines and adds ellipsis overflow",
        ellipsisCol
    );

    // 3. Text Alignments Card
    auto aLeft = text("Left Aligned Text: Elements align to the leading edge of the layout container.");
    aLeft->fontSize(12.0f).textAlign(TextAlign::Left).color(0xFF94A3B8);

    auto aCenter = text("Center Aligned Text: Symmetrically centered text for banners and titles.");
    aCenter->fontSize(12.0f).textAlign(TextAlign::Center).color(0xFF38BDF8);

    auto aRight = text("Right Aligned Text: Useful for timestamps, status values, and numerical columns.");
    aRight->fontSize(12.0f).textAlign(TextAlign::Right).color(0xFFFBBF24);

    auto alignCol = column({
        aLeft, aCenter, aRight
    });

    auto alignCard = typographyCard(
        "3. Text Alignments (Left, Center, Right, Justify)",
        "Horizontal justification and alignment inside any bounding box",
        alignCol
    );

    return column(Key::string("tab_wrapping_view"), {
        wrapCard,
        ellipsisCard,
        alignCard
    });
}

// ════════════════════════════════════════════════════════════════
// Tab 4: Real-World Desktop Shell UI Typography
// ════════════════════════════════════════════════════════════════

WidgetPtr buildShellShowcaseView() {
    // 1. Notification Toast Card
    auto notifIconText = text("🔔");
    notifIconText->fontSize(18.0f);
    auto notifIcon = container(notifIconText);
    notifIcon->size(38.0f, 38.0f)
             .color(0x303B82F6)
             .borderRadius(10.0f)
             .align(Alignment::Center);

    auto notifAppName = text("SYSTEM NOTIFICATION");
    notifAppName->fontSize(10.0f).bold().letterSpacing(1.0f).color(0xFF818CF8);

    auto notifTime = text("Just now");
    notifTime->fontSize(10.0f).color(0xFF64748B);

    auto notifHeaderRow = row({
        notifAppName,
        spacer(),
        notifTime
    });

    auto notifTitle = text("System Update 2.4.0 Available");
    notifTitle->fontSize(14.0f).bold().color(0xFFFFFFFF);

    auto notifBody = text("New Skia GPU pipeline and Wayland Fractional Scaling improvements are ready to install.");
    notifBody->fontSize(12.0f).color(0xFF94A3B8).maxLines(2).ellipsis();

    auto notifCol = column({
        notifHeaderRow,
        notifTitle,
        notifBody
    });
    notifCol->flexGrow(1.0f);

    auto notifBodyContainer = container(notifCol);
    notifBodyContainer->margin(EdgeInsets::only(0, 0, 0, 12.0f)).flexGrow(1.0f);

    auto notifContent = row({
        notifIcon,
        notifBodyContainer
    });
    notifContent->alignItems(Align::Center);

    auto notifCard = container(notifContent);
    notifCard->color(0xE61E293B)
             .borderRadius(14.0f)
             .border(0x40818CF8, 1.5f)
             .paddingAll(14.0f)
             .shadow(0x60000000, {0, 8}, 20.0f)
             .margin(EdgeInsets::only(0, 0, 14.0f, 0));

    // 2. Media Player Shell Card
    auto albumArtText = text("🎵");
    albumArtText->fontSize(24.0f);
    auto albumArt = container(albumArtText);
    albumArt->size(54.0f, 54.0f)
            .color(0x30EC4899)
            .borderRadius(12.0f)
            .border(0x50EC4899, 1.0f)
            .align(Alignment::Center);

    auto trackTitle = text("Resonance (Slowed + Reverb)");
    trackTitle->fontSize(15.0f).bold().color(0xFFFFFFFF).shadow(0x60EC4899, {0, 2}, 6.0f);

    auto trackArtist = text("HOME — Odyssey (Deluxe Remaster)");
    trackArtist->fontSize(12.0f).color(0xFFF472B6);

    auto trackTime = text("02:45 / 03:32");
    trackTime->fontSize(11.0f).color(0xFF94A3B8);

    auto mediaInfoCol = column({
        trackTitle,
        trackArtist,
        trackTime
    });
    mediaInfoCol->flexGrow(1.0f);

    auto mediaInfoContainer = container(mediaInfoCol);
    mediaInfoContainer->margin(EdgeInsets::only(0, 0, 0, 14.0f)).flexGrow(1.0f);

    auto mediaRow = row({
        albumArt,
        mediaInfoContainer
    });
    mediaRow->alignItems(Align::Center);

    auto mediaCard = container(mediaRow);
    mediaCard->color(0xE61E1B4B)
             .borderRadius(14.0f)
             .border(0x40EC4899, 1.5f)
             .paddingAll(14.0f)
             .shadow(0x60000000, {0, 8}, 20.0f)
             .margin(EdgeInsets::only(0, 0, 14.0f, 0));

    // 3. Quick System Monitor Card
    auto cpuLabel = text("CPU LOAD");
    cpuLabel->fontSize(10.0f).bold().letterSpacing(1.0f).color(0xFF64748B);
    auto cpuVal = text("18.4%");
    cpuVal->fontSize(20.0f).bold().color(0xFF34D399);
    auto cpuStat = container(column({cpuLabel, cpuVal}));
    cpuStat->color(0x250F172A).borderRadius(10.0f).paddingAll(12.0f).flexGrow(1.0f);

    auto ramLabel = text("MEMORY USED");
    ramLabel->fontSize(10.0f).bold().letterSpacing(1.0f).color(0xFF64748B);
    auto ramVal = text("4.2 / 32 GB");
    ramVal->fontSize(20.0f).bold().color(0xFF38BDF8);
    auto ramStat = container(column({ramLabel, ramVal}));
    ramStat->color(0x250F172A).borderRadius(10.0f).paddingAll(12.0f).margin(EdgeInsets::only(0, 0, 0, 10.0f)).flexGrow(1.0f);

    auto gpuLabel = text("GPU ENGINE");
    gpuLabel->fontSize(10.0f).bold().letterSpacing(1.0f).color(0xFF64748B);
    auto gpuVal = text("Vulkan 120 FPS");
    gpuVal->fontSize(18.0f).bold().color(0xFFFBBF24);
    auto gpuStat = container(column({gpuLabel, gpuVal}));
    gpuStat->color(0x250F172A).borderRadius(10.0f).paddingAll(12.0f).margin(EdgeInsets::only(0, 0, 0, 10.0f)).flexGrow(1.0f);

    auto statsRow = row({
        cpuStat, ramStat, gpuStat
    });

    auto sysCard = typographyCard(
        "3. Real-Time Shell Metrics & Status Cards",
        "High-contrast statistical typography for dock, panels, and taskbars",
        statsRow
    );

    return column(Key::string("tab_shell_view"), {
        notifCard,
        mediaCard,
        sysCard
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
        auto title = text("⚡ ENKI ENGINE — ADVANCED TYPOGRAPHY DEMO");
        title->fontSize(18.0f).bold().color(0xFFFFFFFF).shadow(0x8038BDF8, {0, 0}, 8.0f);

        auto sub = text("SkParagraph Text Layout + HarfBuzz Font Shaping + Anu Flexbox Integration");
        sub->fontSize(12.0f).color(0xFF818CF8);

        auto badgeText = text("● SKPARAGRAPH GPU READY");
        badgeText->fontSize(10.0f).bold().color(0xFF34D399);

        auto badge = container(badgeText);
        badge->color(0x2010B981)
             .borderRadius(20.0f)
             .border(0x5010B981, 1.0f)
             .paddingSymmetric(4.0f, 12.0f);

        auto titleCol = column({
            title,
            sub
        });

        auto titleRow = row({
            titleCol,
            spacer(),
            badge
        });
        titleRow->alignItems(Align::Center);

        // 2. Tab Navigation Buttons
        auto tabs = row(Key::string("tabs_navigation_row"), {
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
            titleRow,
            tabsBox,
            bodyContent
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
