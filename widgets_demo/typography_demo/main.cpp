/// @file main.cpp
/// @brief Interactive Visual Showcase for Section 17 Typography — Extended.
/// Features:
///   1. SelectableText with mouse drag selection, word/all selection, and copy button
///   2. Marquee ticker tape with live velocity controls, pause-on-hover, and direction toggle
///   3. Multi-language CodeBlock (C++, JSON, Python) with theme switcher, line highlighting & copy button

#include "enki/app/app.hpp"
#include "enki/widgets/selectable_text.hpp"
#include "enki/widgets/marquee.hpp"
#include "enki/widgets/code_block.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/scroll_view.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/text.hpp"
#include "enki/platform/platform.hpp"
#include "enki/state/state.hpp"
#include <iostream>
#include <string>
#include <vector>

using namespace enki;

// ════════════════════════════════════════════════════════════════
// 1. Interactive Selectable Text Component
// ════════════════════════════════════════════════════════════════

class InteractiveSelectableSection : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override;
    std::string_view typeName() const override { return "InteractiveSelectableSection"; }
};

class InteractiveSelectableSectionState : public State {
    std::string sample_text_ =
        "ENKI is an ultra-fast, native Linux GUI framework built in pure modern C++20.\n"
        "You can click and drag across this paragraph to highlight any words, double-click "
        "to select a word, triple-click to select all text, and copy to your system clipboard.";

    TextSelection current_sel_ = TextSelection::empty();
    std::string copy_status_ = "Copy Selected";

public:
    WidgetPtr build(BuildContext& ctx) override {
        std::string metrics_str = "No active selection";
        if (current_sel_.isValid() && !current_sel_.isCollapsed()) {
            metrics_str = "Selected: [" + std::to_string(current_sel_.start()) + " -> " +
                          std::to_string(current_sel_.end()) + "] (" +
                          std::to_string(current_sel_.end() - current_sel_.start()) + " chars)";
        }

        auto header_row = row({
            .justify_content = Justify::SpaceBetween,
            .align_items = Align::Center,
            .children = {
                text("2. SelectableText (Mouse Drag, Word Select & Copy)", {
                    .color = 0xFFE2E8F0,
                    .font_size = 16.0f,
                    .font_weight = FontWeight::SemiBold,
                }),
                container({
                    .color = 0x2038BDF8,
                    .border_radius = BorderRadius::circular(12.0f),
                    .padding = StyleInsets::symmetric(4.0f, 10.0f),
                    .child = text(metrics_str, {
                        .color = 0xFF38BDF8,
                        .font_size = 11.0f,
                        .font_weight = FontWeight::Medium,
                    }),
                }),
            },
        });

        auto selectable_widget = SelectableText {
            .text = sample_text_,
            .style = TextStyle{
                .color = 0xFFCBD5E1,
                .font_size = 14.0f,
                .height = 1.6f,
            },
            .selection_color = 0x6038BDF8,
            .on_selection_changed = [this](TextSelection sel) {
                current_sel_ = sel;
                copy_status_ = "Copy Selected";
            },
        };

        auto selectable_box = container({
            .color = 0xFF0F172A,
            .border_radius = BorderRadius::circular(8.0f),
            .padding = StyleInsets::all(16.0f),
            .child = selectable_widget,
        });

        auto copy_btn = button(
            text(copy_status_, {
                .color = 0xFFFFFFFF,
                .font_size = 12.0f,
                .font_weight = FontWeight::SemiBold,
            }),
            [this]() {
                if (current_sel_.isValid() && !current_sel_.isCollapsed()) {
                    size_t s = current_sel_.start();
                    size_t e = std::min(current_sel_.end(), sample_text_.length());
                    if (s < e) {
                        std::string sel_text = sample_text_.substr(s, e - s);
                        if (auto* p = Platform::instance()) {
                            p->setClipboardText(sel_text);
                        }
                        setState([this]() { copy_status_ = "✓ Copied!"; });
                    }
                }
            },
            ButtonProps{
                .normal_color = 0xFF2563EB,
                .hover_color = 0xFF3B82F6,
                .border_radius = 6.0f,
                .padding = EdgeInsets::symmetric(6.0f, 14.0f),
                .shadow_blur = 0.0f,
            }
        );

        auto action_row = row({
            .gap = StyleValue::point(10.0f),
            .children = {
                copy_btn,
                text("Tip: Select any words with mouse, then click Copy or paste in another app.", {
                    .color = 0xFF64748B,
                    .font_size = 12.0f,
                }),
            },
        });

        return container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(12.0f),
            .padding = StyleInsets::all(20.0f),
            .child = column({
                .gap = StyleValue::point(16.0f),
                .children = {
                    header_row,
                    selectable_box,
                    action_row,
                },
            }),
        });
    }
};

std::unique_ptr<State> InteractiveSelectableSection::createState() {
    return std::make_unique<InteractiveSelectableSectionState>();
}

// ════════════════════════════════════════════════════════════════
// 2. Interactive CodeBlock Component
// ════════════════════════════════════════════════════════════════

class InteractiveCodeBlockSection : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override;
    std::string_view typeName() const override { return "InteractiveCodeBlockSection"; }
};

class InteractiveCodeBlockSectionState : public State {
    std::string current_language_ = "cpp";
    int current_theme_idx_ = 0;

    CodeTheme getActiveTheme() const {
        switch (current_theme_idx_) {
            case 0: return CodeTheme::oneDark();
            case 1: return CodeTheme::dracula();
            case 2: return CodeTheme::vsCodeDark();
            case 3: return CodeTheme::githubDark();
        }
        return CodeTheme::oneDark();
    }

    std::string getCodeForLanguage() const {
        if (current_language_ == "cpp") {
            return 
                "#include <enki/enki.hpp>\n"
                "#include <enki/widgets/selectable_text.hpp>\n"
                "#include <enki/widgets/marquee.hpp>\n"
                "#include <enki/widgets/code_block.hpp>\n\n"
                "// Declarative C++20 GUI Component\n"
                "WidgetPtr buildDashboard() {\n"
                "    return CustomScrollView {\n"
                "        .slivers = {\n"
                "            SliverAppBar {\n"
                "                .title = text(\"Typography Showcase\"),\n"
                "                .pinned = true,\n"
                "                .expanded_height = 180.0f,\n"
                "            },\n"
                "            SliverToBoxAdapter {\n"
                "                .child = SelectableText {\n"
                "                    .text = \"High performance native desktop GUI.\",\n"
                "                    .selection_color = 0x6038BDF8,\n"
                "                },\n"
                "            },\n"
                "        },\n"
                "    };\n"
                "}\n";
        } else if (current_language_ == "json") {
            return 
                "{\n"
                "  \"framework\": \"ENKI\",\n"
                "  \"version\": \"0.2.0\",\n"
                "  \"graphics\": \"Skia / SkParagraph\",\n"
                "  \"layout\": \"Anu Flexbox\",\n"
                "  \"platform\": [\"Wayland\", \"X11\", \"EGL\"],\n"
                "  \"features\": {\n"
                "    \"declarative_syntax\": true,\n"
                "    \"syntax_highlighter\": true,\n"
                "    \"smooth_scrolling\": true\n"
                "  }\n"
                "}\n";
        } else {
            return 
                "# Python Integration Script\n"
                "import enki_core as enki\n\n"
                "def render_status(app_name, uptime):\n"
                "    \"\"\"Generates formatted status report\"\"\"\n"
                "    print(f'Starting {app_name} on Linux Native...')\n"
                "    return {'status': 'ONLINE', 'uptime_sec': uptime}\n\n"
                "if __name__ == '__main__':\n"
                "    render_status('ENKI Shell', 3600)\n";
        }
    }

public:
    WidgetPtr build(BuildContext& ctx) override {
        auto lang_btn = [this](std::string name, std::string lang_id) {
            bool is_active = (current_language_ == lang_id);
            return button(
                text(name, {
                    .color = is_active ? 0xFFFFFFFF : 0xFF94A3B8,
                    .font_size = 12.0f,
                    .font_weight = is_active ? FontWeight::Bold : FontWeight::Normal,
                }),
                [this, lang_id]() {
                    setState([this, lang_id]() { current_language_ = lang_id; });
                },
                ButtonProps{
                    .normal_color = is_active ? 0xFF2563EB : 0x15FFFFFF,
                    .hover_color = is_active ? 0xFF3B82F6 : 0x25FFFFFF,
                    .border_radius = 6.0f,
                    .padding = EdgeInsets::symmetric(6.0f, 14.0f),
                    .shadow_blur = 0.0f,
                }
            );
        };

        auto theme_btn = [this](std::string name, int idx) {
            bool is_active = (current_theme_idx_ == idx);
            return button(
                text(name, {
                    .color = is_active ? 0xFFFFFFFF : 0xFF94A3B8,
                    .font_size = 12.0f,
                    .font_weight = is_active ? FontWeight::Bold : FontWeight::Normal,
                }),
                [this, idx]() {
                    setState([this, idx]() { current_theme_idx_ = idx; });
                },
                ButtonProps{
                    .normal_color = is_active ? 0xFF7C3AED : 0x15FFFFFF,
                    .hover_color = is_active ? 0xFF8B5CF6 : 0x25FFFFFF,
                    .border_radius = 6.0f,
                    .padding = EdgeInsets::symmetric(6.0f, 12.0f),
                    .shadow_blur = 0.0f,
                }
            );
        };

        auto theme_selector = row({
            .gap = StyleValue::point(6.0f),
            .children = {
                theme_btn("OneDark", 0),
                theme_btn("Dracula", 1),
                theme_btn("VSCode", 2),
                theme_btn("GitHub", 3),
            },
        });

        auto code_header = row({
            .justify_content = Justify::SpaceBetween,
            .align_items = Align::Center,
            .children = {
                text("3. CodeBlock (Syntax Highlighting & Copy)", {
                    .color = 0xFFE2E8F0,
                    .font_size = 16.0f,
                    .font_weight = FontWeight::SemiBold,
                }),
                theme_selector,
            },
        });

        auto lang_row = row({
            .gap = StyleValue::point(8.0f),
            .children = {
                lang_btn("C++", "cpp"),
                lang_btn("JSON", "json"),
                lang_btn("Python", "python"),
            },
        });

        auto code_widget = CodeBlock {
            .key = Key::string(current_language_ + "_" + std::to_string(current_theme_idx_)),
            .code = getCodeForLanguage(),
            .language = current_language_,
            .show_line_numbers = true,
            .show_copy_button = true,
            .show_header = true,
            .highlighted_lines = {5, 6},
            .theme = getActiveTheme(),
            .font_size = 13.0f,
        };

        return container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(12.0f),
            .padding = StyleInsets::all(20.0f),
            .child = column({
                .gap = StyleValue::point(16.0f),
                .children = {
                    code_header,
                    lang_row,
                    WidgetPtr(code_widget),
                },
            }),
        });
    }
};

std::unique_ptr<State> InteractiveCodeBlockSection::createState() {
    return std::make_unique<InteractiveCodeBlockSectionState>();
}

// ════════════════════════════════════════════════════════════════
// 3. Main Typography Demo App
// ════════════════════════════════════════════════════════════════

class TypographyDemoAppState : public State {
    float marquee_speed_ = 60.0f;
    MarqueeDirection marquee_dir_ = MarqueeDirection::RightToLeft;
    bool pause_on_hover_ = true;

public:
    WidgetPtr build(BuildContext& ctx) override {
        // ── Top Header ──────────────────────────────────────────
        auto top_badge = container({
            .color = 0x2038BDF8,
            .border_radius = BorderRadius::circular(12.0f),
            .padding = StyleInsets::symmetric(4.0f, 10.0f),
            .child = text("ENKI v0.2.0", {
                .color = 0xFF38BDF8,
                .font_size = 12.0f,
                .font_weight = FontWeight::Bold,
            }),
        });

        auto title_col = column({
            .gap = StyleValue::point(4.0f),
            .children = {
                text("Typography & Code Showcase", {
                    .color = 0xFFF8FAFC,
                    .font_size = 22.0f,
                    .font_weight = FontWeight::Bold,
                }),
                text("Section 17: SelectableText, Marquee & CodeBlock", {
                    .color = 0xFF94A3B8,
                    .font_size = 13.0f,
                }),
            },
        });

        auto header = container({
            .color = 0xFF0F172A,
            .border = Border(0xFF1E293B, 1.0f),
            .padding = StyleInsets::symmetric(24.0f, 32.0f),
            .child = row({
                .justify_content = Justify::SpaceBetween,
                .align_items = Align::Center,
                .children = {
                    title_col,
                    top_badge,
                },
            }),
        });

        // ── 1. Marquee Section (Ticker Tape) ────────────────────
        auto marquee_pill = container({
            .color = 0x2010B981,
            .border_radius = BorderRadius::circular(12.0f),
            .padding = StyleInsets::symmetric(4.0f, 10.0f),
            .child = text(pause_on_hover_ ? "Pause on Hover: ON" : "Pause on Hover: OFF", {
                .color = 0xFF10B981,
                .font_size = 11.0f,
                .font_weight = FontWeight::SemiBold,
            }),
        });

        auto marquee_header = row({
            .justify_content = Justify::SpaceBetween,
            .align_items = Align::Center,
            .children = {
                text("1. Marquee (Auto-Scrolling Ticker)", {
                    .color = 0xFFE2E8F0,
                    .font_size = 16.0f,
                    .font_weight = FontWeight::SemiBold,
                }),
                marquee_pill,
            },
        });

        auto marquee_box = container({
            .color = 0xFF0F172A,
            .border_radius = BorderRadius::circular(8.0f),
            .padding = StyleInsets::symmetric(12.0f, 16.0f),
            .child = Marquee {
                .text = "⚡ BREAKING: ENKI GUI Framework releases next-generation Typography widgets with Skia SkParagraph and C++20 declarative syntax!  •  NASDAQ: ENKI +14.2%  •  GitHub Stars surging!  •  ",
                .style = TextStyle{
                    .color = 0xFF38BDF8,
                    .font_size = 14.0f,
                    .font_weight = FontWeight::Medium,
                },
                .velocity = marquee_speed_,
                .blank_space = 60.0f,
                .direction = marquee_dir_,
                .pause_on_hover = pause_on_hover_,
                .fading_edge_length = 32.0f,
            },
        });

        auto marquee_card = container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(12.0f),
            .padding = StyleInsets::all(20.0f),
            .child = column({
                .gap = StyleValue::point(16.0f),
                .children = {
                    marquee_header,
                    marquee_box,
                },
            }),
        });

        // ── 2. SelectableText Section ───────────────────────────
        auto selectable_card = std::make_shared<InteractiveSelectableSection>();

        // ── 3. CodeBlock Section ────────────────────────────────
        auto code_card = std::make_shared<InteractiveCodeBlockSection>();

        // ── Main Page Scroll Layout ─────────────────────────────
        auto body_content = column({
            .gap = StyleValue::point(24.0f),
            .children = {
                marquee_card,
                selectable_card,
                code_card,
            },
        });

        auto scroll_content = ScrollView {
            .child = body_content,
        };

        auto body_container = container({
            .padding = StyleInsets::all(24.0f),
            .flex_grow = 1.0f,
            .child = scroll_content,
        });

        return container({
            .color = 0xFF0B0F17,
            .child = column({
                .children = {
                    header,
                    body_container,
                },
            }),
        });
    }
};

class TypographyDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override { return std::make_unique<TypographyDemoAppState>(); }
    std::string_view typeName() const override { return "TypographyDemoApp"; }
};

int main() {
    AppConfig cfg;
    cfg.title      = "ENKI — Typography Extended Demo";
    cfg.width      = 960;
    cfg.height     = 820;
    cfg.resizable  = true;
    cfg.vsync      = false;
    cfg.clear_color = 0xFF0B0F17;
    cfg.target_fps  = 0; // Uncapped max speed
    cfg.show_performance_overlay = false; // Display real-time FPS & Frame Time HUD
    return runApp(std::make_shared<TypographyDemoApp>(), cfg);
}
