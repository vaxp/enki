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
    return text({
        .text = s,
        .color = 0xFF64748B,
        .font_size = 11.5f,
        .font_weight = FontWeight::Bold
    });
}

// ── Demo content row label ───────────────────────────────────────────────────
static WidgetPtr rowLabel(const std::string& s) {
    return text({
        .text = s,
        .color = 0xFFCBD5E1,
        .font_size = 13.0f
    });
}

// ── Card wrapper ─────────────────────────────────────────────────────────────
static WidgetPtr card(std::vector<WidgetPtr> items, float w = 560.0f) {
    return container({
        .color = 0xFF1E293B,
        .border_radius = BorderRadius::circular(12.0f),
        .width = StyleValue::point(w),
        .padding = StyleInsets::all(20.0f),
        .child = column({
            .gap = StyleValue::point(2.0f),
            .children = std::move(items)
        })
    });
}

class DividerDemoWidget : public StatelessWidget {
public:
    std::string_view typeName() const override { return "DividerDemoWidget"; }

    WidgetPtr build(BuildContext&) override {
        return container({
            .color = 0xFF0B1120,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(28.0f),
            .child = column({
                .align_items = Align::Center,
                .gap = StyleValue::point(20.0f),
                .children = {
                    // ── Page header ──────────────────────────────────────────────────────
                    column({
                        .align_items = Align::Center,
                        .gap = StyleValue::point(4.0f),
                        .children = {
                            text("Advanced Divider & VerticalDivider Suite", {
                                .color = 0xFFFFFFFF,
                                .font_size = 22.0f,
                                .font_weight = FontWeight::Bold
                            }),
                            text("Solid · Dashed · Dotted · Gradient · Center Label · Round Caps", {
                                .color = 0xFF94A3B8,
                                .font_size = 13.0f
                            })
                        }
                    }),
                    
                    // ═══════════════════════════════════════
                    // Card 1 — Horizontal Dividers
                    // ═══════════════════════════════════════
                    card({
                        sectionLabel("HORIZONTAL DIVIDERS"),
                        Divider { .height = 20.0f },
                        rowLabel("Default Solid (1px slate)"),      Divider { .thickness = 1.0f, .color = 0xFF334155 },
                        rowLabel("Solid Accent (2px sky-400)"),     Divider { .thickness = 2.0f, .color = 0xFF38BDF8 },
                        rowLabel("Dashed (slate, 8/5)"),            Divider { .thickness = 1.5f, .color = 0xFF64748B, .style = DividerStyle::Dashed, .dash_length = 8.0f, .dash_gap = 5.0f },
                        rowLabel("Dotted (amber, 3px round)"),      Divider { .thickness = 3.0f, .color = 0xFFF59E0B, .style = DividerStyle::Dotted, .dash_gap = 5.0f },
                        rowLabel("Gradient Fade (violet)"),         Divider { .thickness = 2.0f, .color = 0xFF8B5CF6, .style = DividerStyle::Gradient },
                        rowLabel("Gradient Fade + indent (emerald)"), Divider { .thickness = 2.5f, .indent = 40.0f, .end_indent = 40.0f, .color = 0xFF10B981, .style = DividerStyle::Gradient },
                        rowLabel("Dashed Round Caps (pink, 4px)"),  Divider { .thickness = 4.0f, .color = 0xFFEC4899, .style = DividerStyle::Dashed, .dash_length = 10.0f, .dash_gap = 6.0f, .round_caps = true },
                        rowLabel("Center Label \"OR\""),            Divider { .thickness = 1.0f, .color = 0xFF475569, .label = "OR", .label_font_size = 11.0f, .label_color = 0xFF94A3B8, .label_bg_color = 0xFF1E293B },
                        rowLabel("Dashed + Label \"Section Break\""), Divider { .thickness = 1.0f, .color = 0xFF38BDF8, .style = DividerStyle::Dashed, .dash_length = 6.0f, .dash_gap = 4.0f, .label = "Section Break", .label_font_size = 10.5f, .label_color = 0xFF38BDF8, .label_bg_color = 0xFF1E293B },
                    }),
                    
                    // ═══════════════════════════════════════
                    // Card 2 — Vertical Dividers side-by-side
                    // ═══════════════════════════════════════
                    card({
                        sectionLabel("VERTICAL DIVIDERS (height = parent height)"),
                        row({
                            .align_items = Align::Stretch,
                            .gap = StyleValue::point(8.0f),
                            .children = {
                                container({
                                    .padding = StyleInsets::symmetric(8.0f, 6.0f),
                                    .child = column({
                                        .align_items = Align::Center, .gap = StyleValue::point(6.0f),
                                        .children = {
                                            text("Default", { .color = 0xFF64748B, .font_size = 10.0f }),
                                            VerticalDivider { .height = 20.0f, .thickness = 1.0f, .color = 0xFF334155 }
                                        }
                                    }),
                                }),
                                container({
                                    .padding = StyleInsets::symmetric(8.0f, 6.0f),
                                    .child = column({
                                        .align_items = Align::Center, .gap = StyleValue::point(6.0f),
                                        .children = {
                                            text("Sky-400", { .color = 0xFF64748B, .font_size = 10.0f }),
                                            VerticalDivider { .height = 20.0f, .thickness = 2.0f, .color = 0xFF38BDF8 }
                                        }
                                    }),
                                }),
                                container({
                                    .padding = StyleInsets::symmetric(8.0f, 6.0f),
                                    .child = column({
                                        .align_items = Align::Center, .gap = StyleValue::point(6.0f),
                                        .children = {
                                            text("Dashed", { .color = 0xFF64748B, .font_size = 10.0f }),
                                            VerticalDivider { .height = 20.0f, .thickness = 1.5f, .color = 0xFF8B5CF6, .style = DividerStyle::Dashed, .dash_length = 8.0f, .dash_gap = 4.0f }
                                        }
                                    }),
                                }),
                                container({
                                    .padding = StyleInsets::symmetric(8.0f, 6.0f),
                                    .child = column({
                                        .align_items = Align::Center, .gap = StyleValue::point(6.0f),
                                        .children = {
                                            text("Dotted", { .color = 0xFF64748B, .font_size = 10.0f }),
                                            VerticalDivider { .height = 20.0f, .thickness = 3.0f, .color = 0xFFF59E0B, .style = DividerStyle::Dotted, .dash_gap = 5.0f }
                                        }
                                    }),
                                }),
                                container({
                                    .padding = StyleInsets::symmetric(8.0f, 6.0f),
                                    .child = column({
                                        .align_items = Align::Center, .gap = StyleValue::point(6.0f),
                                        .children = {
                                            text("Gradient", { .color = 0xFF64748B, .font_size = 10.0f }),
                                            VerticalDivider { .height = 20.0f, .thickness = 2.0f, .color = 0xFF10B981, .style = DividerStyle::Gradient }
                                        }
                                    }),
                                }),
                                container({
                                    .padding = StyleInsets::symmetric(8.0f, 6.0f),
                                    .child = column({
                                        .align_items = Align::Center, .gap = StyleValue::point(6.0f),
                                        .children = {
                                            text("Rounded", { .color = 0xFF64748B, .font_size = 10.0f }),
                                            VerticalDivider { .height = 20.0f, .thickness = 4.0f, .color = 0xFFEC4899, .style = DividerStyle::Dashed, .dash_length = 10.0f, .dash_gap = 5.0f, .round_caps = true }
                                        }
                                    }),
                                })
                            }
                        })
                    })
                }
            })
        });
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
