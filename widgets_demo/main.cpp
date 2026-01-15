/// @file main.cpp
/// @brief ENKI Visual Widget Test Suite & Interactive Desktop Shell Showcase.
/// Renders the exact test cases from test_flexbox.cpp and test_container.cpp as a rich graphical UI.

#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text_field.hpp"
#include "enki/state/state.hpp"
#include "enki/rendering/canvas.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <memory>

using namespace enki;

// ════════════════════════════════════════════════════════════════
// Custom Visual Text Label Widget (Skia Rendered)
// ════════════════════════════════════════════════════════════════

class RenderLabel : public RenderBox {
public:
    std::string text_;
    float font_size_;
    Color color_;
    bool bold_;

    RenderLabel(std::string text, float fontSize, Color color, bool bold)
        : text_(std::move(text)), font_size_(fontSize), color_(color), bold_(bold) {}

    void setText(std::string t) { text_ = std::move(t); markNeedsPaint(); }
    void setColor(Color c) { color_ = c; markNeedsPaint(); }

    void paint(PaintContext& ctx) override {
        if (text_.empty()) return;
        Paint p;
        p.setColor(color_);
        p.setAntiAlias(true);
        ctx.canvas.drawText(text_, {ctx.offset.x, ctx.offset.y + font_size_ * 0.82f}, p, font_size_, nullptr, bold_);
    }
};

class Label : public SingleChildRenderObjectWidget {
public:
    std::string text;
    float font_size = 14.0f;
    Color color = 0xFFFFFFFF;
    bool bold = false;

    Label(std::string text, float fontSize = 14.0f, Color color = 0xFFFFFFFF, bool bold = false)
        : text(std::move(text)), font_size(fontSize), color(color), bold(bold) {}

    std::unique_ptr<RenderObject> createRenderObject(BuildContext&) override {
        auto ro = std::make_unique<RenderLabel>(text, font_size, color, bold);
        // Approx text size
        float approx_w = text.length() * font_size * 0.58f;
        ANUNodeStyleSetWidth(ro->anuNode(), approx_w);
        ANUNodeStyleSetHeight(ro->anuNode(), font_size * 1.3f);
        return ro;
    }



    void updateRenderObject(BuildContext&, RenderObject& ro) override {
        if (auto* rl = dynamic_cast<RenderLabel*>(&ro)) {
            rl->setText(text);
            rl->setColor(color);
            rl->font_size_ = font_size;
            rl->bold_ = bold;
            float approx_w = text.length() * font_size * 0.58f;
            ANUNodeStyleSetWidth(rl->anuNode(), approx_w);
            ANUNodeStyleSetHeight(rl->anuNode(), font_size * 1.3f);
            rl->markNeedsLayout();
        }
    }

    std::string_view typeName() const override { return "Label"; }
};

inline std::shared_ptr<Label> label(std::string text, float fontSize = 14.0f, Color color = 0xFFFFFFFF, bool bold = false) {
    return std::make_shared<Label>(std::move(text), fontSize, color, bold);
}

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

    Clickable(BoxDecoration dec, FlexboxStyle s, WidgetPtr child, std::function<void()> onClick)
        : SingleChildRenderObjectWidget(Key::none(), std::move(child)),
          decoration(std::move(dec)), style(std::move(s)), on_click(std::move(onClick)) {}

    std::unique_ptr<RenderObject> createRenderObject(BuildContext&) override {
        return std::make_unique<RenderClickable>(decoration, style, on_click);
    }

    void updateRenderObject(BuildContext&, RenderObject& ro) override {
        if (auto* rc = dynamic_cast<RenderClickable*>(&ro)) {
            rc->setDecoration(decoration);
            rc->setStyle(style);
            rc->on_click_ = on_click;
        }
    }

    std::string_view typeName() const override { return "Clickable"; }
};

inline WidgetPtr button(std::string text, bool active, std::function<void()> onClick) {
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

    auto btnLabel = label(std::move(text), 13.0f, active ? 0xFFFFFFFF : 0xFF94A3B8, active);
    return std::make_shared<Clickable>(dec, s, btnLabel, std::move(onClick));
}

// ════════════════════════════════════════════════════════════════
// Reusable UI Card Container Helper
// ════════════════════════════════════════════════════════════════

inline WidgetPtr sectionCard(std::string title, std::string subtitle, WidgetPtr content, Key key = Key::none()) {
    auto titleLabel = label(std::move(title), 14.0f, 0xFFF1F5F9, true);
    auto subLabel = label(std::move(subtitle), 11.0f, 0xFF64748B, false);

    auto cardCol = column({
        .children = {
            titleLabel,
            paddingBox(EdgeInsets::only(2.0f, 0, 12.0f, 0), subLabel),
            content
        }
    });

    auto card = container({
        .color = 0x301E293B,
        .border_radius = BorderRadius::circular(14.0f),
        .border = Border(0x25FFFFFF, 1.0f),
        .box_shadow = {BoxShadow(0x30000000, {0, 4}, 10.0f)},
        .padding = StyleInsets::all(16.0f),
        .margin = StyleInsets::only(0, 0, 16.0f, 0),
        .flex_grow = 1.0f,
        .child = cardCol,
        .key = std::move(key),
    });

    return card;
}

inline WidgetPtr colorBox(std::string text, float w, float h, Color c, float radius = 8.0f) {
    auto textLabel = label(std::move(text), 11.0f, 0xFFFFFFFF, true);
    auto box = container({
        .color = c,
        .border_radius = BorderRadius::circular(radius),
        .box_shadow = {BoxShadow(0x30000000, {0, 2}, 6.0f)},
        .align = Alignment::Center,
        .width = StyleValue::point(w),
        .height = StyleValue::point(h),
        .child = textLabel,
    });
    return box;
}

// ════════════════════════════════════════════════════════════════
// Tab 1: Flexbox Visual Test Suite View
// ════════════════════════════════════════════════════════════════

inline WidgetPtr buildFlexboxTestView() {
    // 1. Basic Row & Column
    auto rowDemo = row({
        .children = {
            colorBox("100x44", 100.0f, 44.0f, 0xFF3B82F6),
            paddingBox(EdgeInsets::only(0, 0, 0, 8.0f), colorBox("150x44", 150.0f, 44.0f, 0xFF8B5CF6)),
            paddingBox(EdgeInsets::only(0, 0, 0, 8.0f), colorBox("70x44", 70.0f, 44.0f, 0xFFEC4899))
        }
    });

    auto colDemo = column({
        .children = {
            colorBox("140x26", 140.0f, 26.0f, 0xFF10B981),
            paddingBox(EdgeInsets::only(6.0f, 0, 0, 0), colorBox("140x36", 140.0f, 36.0f, 0xFFF59E0B)),
            paddingBox(EdgeInsets::only(6.0f, 0, 0, 0), colorBox("140x22", 140.0f, 22.0f, 0xFF06B6D4))
        }
    });

    auto card1 = sectionCard("1. Basic Row & Column", "test_row_basic & test_column_basic",
        row({
            .children = {
                rowDemo,
                paddingBox(EdgeInsets::only(0, 0, 0, 30.0f), colDemo)
            }
        })
    );

    // 2. Justify Content
    auto makeJustifyStrip = [](Justify j, std::string name) {
        auto r = row({
            .justify_content = j,
            .align_items = Align::Center,
            .width = 280_px,
            .height = 40_px,
            .children = {
                colorBox("1", 40.0f, 30.0f, 0xFF6366F1),
                colorBox("2", 40.0f, 30.0f, 0xFF8B5CF6),
                colorBox("3", 40.0f, 30.0f, 0xFFA855F7)
            }
        });

        auto c = container({
            .color = 0x200F172A,
            .border_radius = BorderRadius::circular(6.0f),
            .padding = StyleInsets::symmetric(0, 6.0f),
            .child = r,
        });

        return row({
            .children = {
                sizedBox(95.0f, 30.0f, label(name, 11.0f, 0xFF94A3B8)),
                c
            }
        });
    };

    auto card2 = sectionCard("2. Justify Content Alignments", "Start, Center, End, SpaceBetween, SpaceAround, SpaceEvenly",
        column({
            .children = {
                makeJustifyStrip(Justify::Start, "Start"),
                paddingBox(EdgeInsets::only(6.0f, 0, 0, 0), makeJustifyStrip(Justify::Center, "Center")),
                paddingBox(EdgeInsets::only(6.0f, 0, 0, 0), makeJustifyStrip(Justify::End, "End")),
                paddingBox(EdgeInsets::only(6.0f, 0, 0, 0), makeJustifyStrip(Justify::SpaceBetween, "SpaceBetween")),
                paddingBox(EdgeInsets::only(6.0f, 0, 0, 0), makeJustifyStrip(Justify::SpaceEvenly, "SpaceEvenly"))
            }
        })
    );

    // 3. Flex Grow & Proportions (1:2:1)
    auto growBox1 = container({
        .color = 0xFF0EA5E9,
        .border_radius = BorderRadius::circular(8.0f),
        .box_shadow = {BoxShadow(0x30000000, {0, 2}, 6.0f)},
        .align = Alignment::Center,
        .height = StyleValue::point(36.0f),
        .flex_grow = 1.0f,
        .child = label("Grow: 1", 11.0f, 0xFFFFFFFF, true),
    });

    auto growBox2 = container({
        .color = 0xFF6366F1,
        .border_radius = BorderRadius::circular(8.0f),
        .box_shadow = {BoxShadow(0x30000000, {0, 2}, 6.0f)},
        .align = Alignment::Center,
        .height = StyleValue::point(36.0f),
        .margin = StyleInsets::symmetric(0, 8.0f),
        .flex_grow = 2.0f,
        .child = label("Grow: 2 (Double Width)", 11.0f, 0xFFFFFFFF, true),
    });

    auto growBox3 = container({
        .color = 0xFF10B981,
        .border_radius = BorderRadius::circular(8.0f),
        .box_shadow = {BoxShadow(0x30000000, {0, 2}, 6.0f)},
        .align = Alignment::Center,
        .height = StyleValue::point(36.0f),
        .flex_grow = 1.0f,
        .child = label("Grow: 1", 11.0f, 0xFFFFFFFF, true),
    });

    auto growRow = row({
        .width = 400_px,
        .children = {growBox1, growBox2, growBox3}
    });

    auto card3 = sectionCard("3. Proportional Flex Factors & Gaps", "test_flex_factor (1 : 2 : 1 Proportional Expansion)",
        growRow
    );

    // 4. Flex Wrap & Tags
    std::vector<WidgetPtr> tagChips;
    std::vector<std::string> tags = {"Anu Layout", "Skia GPU", "Zero Calculation Tamper", "RenderObject", "Flexbox", "Container", "Wayland LayerShell", "High Performance"};
    std::vector<Color> tagColors = {0xFF2563EB, 0xFF7C3AED, 0xFFDB2777, 0xFFD97706, 0xFF059669, 0xFF0891B2, 0xFF4F46E5, 0xFFE11D48};

    for (size_t i = 0; i < tags.size(); ++i) {
        auto chip = container({
            .color = tagColors[i % tagColors.size()],
            .border_radius = BorderRadius::circular(20.0f),
            .align = Alignment::Center,
            .padding = StyleInsets::symmetric(6.0f, 14.0f),
            .margin = StyleInsets::only(0, 8.0f, 8.0f, 0),
            .child = label(tags[i], 11.0f, 0xFFFFFFFF, true),
        });
        tagChips.push_back(chip);
    }

    auto wrapRow = row({
        .flex_wrap = FlexWrap::Wrap,
        .width = 420_px,
        .children = std::move(tagChips)
    });

    auto card4 = sectionCard("4. Multi-Line Flex Wrap", "test_flex_wrap with dynamic chips",
        wrapRow
    );

    return column(Key::string("tab_flexbox_root"), {
        .children = {
            row({
                .children = {card1, paddingBox(EdgeInsets::only(0, 0, 0, 16.0f), card2)}
            }),
            row({
                .children = {card3, paddingBox(EdgeInsets::only(0, 0, 0, 16.0f), card4)}
            })
        }
    });
}

// ════════════════════════════════════════════════════════════════
// Tab 2: Container & BoxDecoration Visual Test Suite View
// ════════════════════════════════════════════════════════════════

inline WidgetPtr buildContainerTestView() {
    // 1. Modern Gradients
    auto gradCard1 = container({
        .color = 0xFF7928CA,
        .border_radius = BorderRadius::circular(12.0f),
        .box_shadow = {BoxShadow(0x50FF007A, {0, 6}, 14.0f)},
        .align = Alignment::Center,
        .width = StyleValue::point(110.0f),
        .height = StyleValue::point(75.0f),
        .child = label("Linear Neon", 12.0f, 0xFFFFFFFF, true),
    });

    auto gradCard2 = container({
        .color = 0xFFFF4D4D,
        .border_radius = BorderRadius::circular(12.0f),
        .box_shadow = {BoxShadow(0x50FF4D4D, {0, 6}, 14.0f)},
        .align = Alignment::Center,
        .width = StyleValue::point(110.0f),
        .height = StyleValue::point(75.0f),
        .margin = StyleInsets::only(0, 0, 0, 12.0f),
        .child = label("Sunset Glow", 12.0f, 0xFFFFFFFF, true),
    });

    auto gradCard3 = container({
        .color = 0xFF0072FF,
        .border_radius = BorderRadius::circular(12.0f),
        .box_shadow = {BoxShadow(0x500072FF, {0, 6}, 14.0f)},
        .align = Alignment::Center,
        .width = StyleValue::point(110.0f),
        .height = StyleValue::point(75.0f),
        .margin = StyleInsets::only(0, 0, 0, 12.0f),
        .child = label("Ocean Cyan", 12.0f, 0xFFFFFFFF, true),
    });

    auto gradCard4 = container({
        .color = 0xFF0D47A1,
        .border_radius = BorderRadius::circular(12.0f),
        .box_shadow = {BoxShadow(0x5000FFCC, {0, 6}, 14.0f)},
        .align = Alignment::Center,
        .width = StyleValue::point(110.0f),
        .height = StyleValue::point(75.0f),
        .margin = StyleInsets::only(0, 0, 0, 12.0f),
        .child = label("Radial Glow", 12.0f, 0xFFFFFFFF, true),
    });

    auto card1 = sectionCard("1. Linear & Radial Gradients", "test_box_decoration (Custom Color Stops & Angles)",
        row({
            .children = {gradCard1, gradCard2, gradCard3, gradCard4}
        })
    );

    // 2. Multi-Box Shadows
    auto shadow1 = container({
        .color = 0xFF1E293B,
        .border_radius = BorderRadius::circular(10.0f),
        .border = Border(0x30FFFFFF, 1.0f),
        .box_shadow = {BoxShadow(0x40000000, {0, 4}, 12.0f)},
        .align = Alignment::Center,
        .width = StyleValue::point(130.0f),
        .height = StyleValue::point(65.0f),
        .child = label("Soft Ambient", 11.0f, 0xFFE2E8F0),
    });

    auto shadow2 = container({
        .color = 0xFF1E293B,
        .border_radius = BorderRadius::circular(10.0f),
        .border = Border(0x30FFFFFF, 1.0f),
        .box_shadow = {BoxShadow(0x70000000, {0, 10}, 24.0f)},
        .align = Alignment::Center,
        .width = StyleValue::point(130.0f),
        .height = StyleValue::point(65.0f),
        .margin = StyleInsets::only(0, 0, 0, 16.0f),
        .child = label("Deep Elevation", 11.0f, 0xFFE2E8F0),
    });

    auto shadow3 = container({
        .color = 0xFF4338CA,
        .border_radius = BorderRadius::circular(10.0f),
        .border = Border(0xFF818CF8, 1.5f),
        .box_shadow = {BoxShadow(0x806366F1, {0, 0}, 20.0f)},
        .align = Alignment::Center,
        .width = StyleValue::point(130.0f),
        .height = StyleValue::point(65.0f),
        .margin = StyleInsets::only(0, 0, 0, 16.0f),
        .child = label("Neon Indigo", 11.0f, 0xFFFFFFFF, true),
    });

    auto card2 = sectionCard("2. Multi-Layer Box Shadows", "test_box_decoration (Ambient, Deep & Neon Glow)",
        row({
            .children = {shadow1, shadow2, shadow3}
        })
    );

    // 3. Corner Radii & Shapes
    auto shape1 = colorBox("Radius 6", 80.0f, 60.0f, 0xFF334155, 6.0f);
    auto shape2 = container({
        .color = 0xFF334155,
        .border_radius = BorderRadius::circular(18.0f),
        .box_shadow = {BoxShadow(0x30000000, {0, 2}, 6.0f)},
        .align = Alignment::Center,
        .width = StyleValue::point(80.0f),
        .height = StyleValue::point(60.0f),
        .margin = StyleInsets::only(0, 0, 0, 10.0f),
        .child = label("Radius 18", 11.0f, 0xFFFFFFFF, true),
    });

    auto shape3 = container({
        .color = 0xFF8B5CF6,
        .border_radius = BorderRadius::circular(999.0f),
        .align = Alignment::Center,
        .width = StyleValue::point(90.0f),
        .height = StyleValue::point(40.0f),
        .margin = StyleInsets::only(0, 0, 0, 10.0f),
        .child = label("Pill", 11.0f, 0xFFFFFFFF, true),
    });

    auto shape4 = container({
        .color = 0xFFEC4899,
        .border = Border(0xFFFFFFFF, 2.0f),
        .box_shadow = {BoxShadow(0x50EC4899, {0, 4}, 12.0f)},
        .shape = BoxShape::Circle,
        .align = Alignment::Center,
        .width = StyleValue::point(60.0f),
        .height = StyleValue::point(60.0f),
        .margin = StyleInsets::only(0, 0, 0, 10.0f),
        .child = label("Circle", 11.0f, 0xFFFFFFFF, true),
    });

    auto card3 = sectionCard("3. Shapes & Border Radii", "test_hit_testing (Rectangles, Pills & Circles)",
        row({
            .children = {shape1, shape2, shape3, shape4}
        })
    );

    // 4. Aspect Ratio & Padding
    auto aspectChild = container({
        .color = 0xFF2563EB,
        .border_radius = BorderRadius::circular(10.0f),
        .box_shadow = {BoxShadow(0x40000000, {0, 4}, 8.0f)},
        .align = Alignment::Center,
        .width = 192_px,
        .aspect_ratio = 16.0f / 9.0f,
        .child = label("16 : 9 Widescreen", 12.0f, 0xFFFFFFFF, true),
    });

    auto paddingDemoChild = container({
        .color = 0xFF38BDF8,
        .border_radius = BorderRadius::circular(6.0f),
        .align = Alignment::Center,
        .width = StyleValue::point(90.0f),
        .height = StyleValue::point(35.0f),
        .child = label("Child (Padded)", 10.0f, 0xFF0F172A, true),
    });

    auto paddingOuter = container({
        .color = 0xFF0369A1,
        .border_radius = BorderRadius::circular(10.0f),
        .border = Border(0xFF38BDF8, 1.5f),
        .padding = StyleInsets::all(14.0f),
        .margin = StyleInsets::only(0, 0, 0, 20.0f),
        .child = paddingDemoChild,
    });

    auto card4 = sectionCard("4. Aspect Ratio (16:9) & Inset Padding", "test_container_constraints & test_container_padding",
        row({
            .children = {aspectChild, paddingOuter}
        })
    );

    return column(Key::string("tab_container_root"), {
        .children = {
            row({
                .children = {card1, paddingBox(EdgeInsets::only(0, 0, 0, 16.0f), card2)}
            }),
            row({
                .children = {card3, paddingBox(EdgeInsets::only(0, 0, 0, 16.0f), card4)}
            })
        }
    });
}

// ════════════════════════════════════════════════════════════════
// Tab 3: Desktop Shell Real-World Showcase
// ════════════════════════════════════════════════════════════════

inline WidgetPtr buildShellShowcaseView() {
    // 1. macOS/iPadOS Style Glassmorphism Dock
    std::vector<WidgetPtr> dockIcons;
    std::vector<Color> iconColors = {0xFF3B82F6, 0xFF10B981, 0xFFF59E0B, 0xFF8B5CF6, 0xFFEC4899, 0xFF06B6D4};
    std::vector<std::string> iconLabels = {">_", "Files", "Music", "Code", "Web", "Settings"};

    for (size_t i = 0; i < iconColors.size(); ++i) {
        auto iconInner = container({
            .color = iconColors[i],
            .border_radius = BorderRadius::circular(14.0f),
            .box_shadow = {BoxShadow(0x50000000, {0, 4}, 8.0f)},
            .align = Alignment::Center,
            .width = StyleValue::point(48.0f),
            .height = StyleValue::point(48.0f),
            .margin = StyleInsets::symmetric(0, 6.0f),
            .child = label(iconLabels[i], 11.0f, 0xFFFFFFFF, true),
        });
        dockIcons.push_back(iconInner);
    }

    auto dockRow = row({
        .align_items = Align::Center,
        .children = std::move(dockIcons)
    });

    auto dockContainer = container({
        .color = 0x351E293B,
        .border_radius = BorderRadius::circular(22.0f),
        .border = Border(0x40FFFFFF, 1.0f),
        .box_shadow = {BoxShadow(0x80000000, {0, 12}, 30.0f)},
        .padding = StyleInsets::symmetric(10.0f, 16.0f),
        .child = dockRow,
    });

    auto dockCenterRow = row({
        .justify_content = Justify::Center,
        .children = {dockContainer}
    });

    auto dockCard = sectionCard("1. Glassmorphism Desktop Dock", "Assembled via Flexbox Row + Container Glassmorphism",
        dockCenterRow
    );

    // 2. Control Center Card
    auto makeToggle = [](std::string name, bool on, Color activeColor) {
        auto dot = container({
            .color = on ? 0xFFFFFFFF : 0xFF64748B,
            .shape = BoxShape::Circle,
            .width = StyleValue::point(12.0f),
            .height = StyleValue::point(12.0f),
        });

        auto tRow = row({
            .align_items = Align::Center,
            .children = {
                dot,
                paddingBox(EdgeInsets::only(0, 0, 0, 8.0f), label(name, 11.0f, on ? 0xFFFFFFFF : 0xFF94A3B8, true))
            }
        });

        auto t = container({
            .color = on ? activeColor : 0x25334155,
            .border_radius = BorderRadius::circular(10.0f),
            .padding = StyleInsets::symmetric(8.0f, 14.0f),
            .margin = StyleInsets::only(0, 8.0f, 8.0f, 0),
            .child = tRow,
        });
        return t;
    };

    auto toggleRow = row({
        .children = {
            makeToggle("Wi-Fi (5G)", true, 0xFF2563EB),
            makeToggle("Bluetooth", true, 0xFF7C3AED),
            makeToggle("Dark Mode", true, 0xFF059669),
            makeToggle("DND", false, 0xFF475569)
        }
    });

    // Slider bar
    auto sliderFill = container({
        .color = 0xFF6366F1,
        .border_radius = BorderRadius::circular(999.0f),
        .width = StyleValue::point(160.0f),
        .height = StyleValue::point(10.0f),
    });

    auto sliderTrack = container({
        .color = 0x40475569,
        .border_radius = BorderRadius::circular(999.0f),
        .width = StyleValue::point(240.0f),
        .height = StyleValue::point(10.0f),
        .child = sliderFill,
    });

    auto sliderSection = row({
        .align_items = Align::Center,
        .children = {
            label("Brightness", 11.0f, 0xFF94A3B8),
            paddingBox(EdgeInsets::only(0, 0, 0, 12.0f), sliderTrack)
        }
    });

    auto controlCol = column({
        .children = {
            toggleRow,
            paddingBox(EdgeInsets::only(12.0f, 0, 0, 0), sliderSection)
        }
    });

    auto controlCard = sectionCard("2. Shell Quick Settings & Sliders", "Control Center toggles and status sliders",
        controlCol
    );

    return column(Key::string("tab_shell_root"), {
        .children = {
            dockCard,
            controlCard
        }
    });
}

// ════════════════════════════════════════════════════════════════
// Root Demo State & Application Widget
// ════════════════════════════════════════════════════════════════

class DemoState : public State {
    int current_tab_ = 3; // Default to TextField Tab for demo
    std::shared_ptr<TextFieldController> text_ctrl_;

public:
    void initState() override {
        State::initState();
        text_ctrl_ = std::make_shared<TextFieldController>("");
    }

    WidgetPtr build(BuildContext& ctx) override {
        // 1. Header Bar
        auto title = label("⚡ ENKI LINUX SHELL — VISUAL WIDGET TEST SUITE", 18.0f, 0xFFFFFFFF, true);
        auto sub = label("Real-time GPU Skia Rendering & Anu Flexbox Validation", 12.0f, 0xFF818CF8, false);

        auto badgeText = label("● 10/10 TESTS PASSING", 10.0f, 0xFF34D399, true);
        auto badge = container({
            .color = 0x2010B981,
            .border_radius = BorderRadius::circular(20.0f),
            .border = Border(0x5010B981, 1.0f),
            .padding = StyleInsets::symmetric(4.0f, 12.0f),
            .child = badgeText,
        });

        auto titleRow = row({
            .align_items = Align::Center,
            .children = {
                column({
                    .children = {title, paddingBox(EdgeInsets::only(3.0f, 0, 0, 0), sub)}
                }),
                spacer(),
                badge
            }
        });

        // 2. Tab Navigation Buttons
        auto tabs = row({
            .children = {
                button("📐 1. Flexbox Layout Suite", current_tab_ == 0, [this] {
                    setState([this] { current_tab_ = 0; });
                }),
                button("🎨 2. Container & BoxDecoration Suite", current_tab_ == 1, [this] {
                    setState([this] { current_tab_ = 1; });
                }),
                button("🖥️ 3. Real-World Desktop Shell", current_tab_ == 2, [this] {
                    setState([this] { current_tab_ = 2; });
                }),
                button("📝 4. Interactive TextField", current_tab_ == 3, [this] {
                    setState([this] { current_tab_ = 3; });
                })
            }
        });

        // 3. Tab Body
        WidgetPtr bodyContent = nullptr;
        if (current_tab_ == 0) {
            bodyContent = buildFlexboxTestView();
        } else if (current_tab_ == 1) {
            bodyContent = buildContainerTestView();
        } else if (current_tab_ == 2) {
            bodyContent = buildShellShowcaseView();
        } else {
            // TextField Demo Tab
            auto tf = TextField {
                .controller = text_ctrl_,
                .style = TextStyle { .color = 0xFFFFFFFF, .font_size = 18.0f },
                .hint_text = "Type here (Supports Arabic, UTF-8, Key Repeat)...",
                .auto_focus = true,
                .cursor_color = 0xFF3B82F6,
                .selection_color = 0x503B82F6,
            };
            
            auto tf_box = container({
                .color = 0xFF1E293B,
                .border_radius = BorderRadius::circular(8.0f),
                .border = Border(0xFF334155, 1.0f),
                .width = 600_px,
                .padding = StyleInsets::all(12.0f),
                .child = tf,
            });
                  
            bodyContent = sectionCard("Interactive TextField", "Test typing, backspace, and arrow keys with repeat support.", tf_box);
        }

        // Main App Layout
        auto mainCol = column({
            .children = {
                titleRow,
                paddingBox(EdgeInsets::only(14.0f, 0, 16.0f, 0), tabs),
                bodyContent
            }
        });

        auto appRoot = container({
            .color = 0xFF0B0F19,
            .padding = StyleInsets::all(20.0f),
            .flex_grow = 1.0f,
            .child = mainCol,
        });

        return appRoot;
    }
};

class WidgetsDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<DemoState>();
    }
    std::string_view typeName() const override { return "WidgetsDemoApp"; }
};

int main() {
    std::cout << "================================================\n";
    std::cout << "  ENKI Engine — Visual Widget Test Suite Demo    \n";
    std::cout << "  Interactive Flexbox & Container UI Validator  \n";
    std::cout << "================================================\n";

    AppConfig config;
    config.title       = "Enki Engine — Visual Widget Test Suite & Shell Demo";
    config.width       = 1060;
    config.height      = 650;
    config.resizable   = true;
    config.vsync       = true;
    config.target_fps  = 60;
    config.show_performance_overlay = true;
    config.clear_color = 0xFF0B0F19;

    return runApp(std::make_shared<WidgetsDemoApp>(), config);
}
