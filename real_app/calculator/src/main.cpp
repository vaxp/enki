/// @file main.cpp
/// @brief ENKI Calculator — Real World Showcase Application.
/// Features SkSL live shader injection, vector SVG HUD frames, 9-slice borders,
/// particle celebration burst, full scientific calculation engine, and VAXPOS CSD.

#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/window_frame.hpp"
#include "enki/widgets/titlebar.hpp"
#include "enki/widgets/scroll_view.hpp"
#include "enki/widgets/stack.hpp"
#include "enki/widgets/particle_emitter.hpp"
#include "enki/state/state.hpp"

#include "calculator_engine.hpp"
#include "calculator_theme.hpp"

#include <iostream>
#include <memory>
#include <vector>
#include <string>

using namespace enki;
using namespace enki::calc;

enum class ButtonType {
    Digit,
    Operator,
    Action,
    Scientific,
    Equals
};

class CalcButton : public StatefulWidget {
public:
    std::string label;
    ButtonType type;
    std::function<void()> on_tap;
    float flex_val;

    CalcButton(std::string label, ButtonType type, std::function<void()> on_tap, float flex_val = 1.0f)
        : label(std::move(label)), type(type), on_tap(std::move(on_tap)), flex_val(flex_val) {}

    std::unique_ptr<State> createState() override;
    std::string_view typeName() const override { return "CalcButton"; }
};

class CalcButtonState : public State {
public:
    bool is_hovered = false;
    bool is_pressed = false;

    WidgetPtr build(BuildContext&) override {
        auto* btn = static_cast<const CalcButton*>(widget());
        BoxDecoration dec;
        dec.border_radius = BorderRadius::circular(12.0f);

        Color text_color = colors::KeyNumText;
        float font_size = 20.0f;
        FontWeight weight = FontWeight::SemiBold;

        switch (btn->type) {
            case ButtonType::Digit:
                if (is_pressed) {
                    dec.color = 0x66334155;
                    dec.border = Border(0xFF38BDF8, 1.5f);
                    dec.box_shadow = { BoxShadow::glow(0x6038BDF8, 10.0f) };
                } else if (is_hovered) {
                    dec.color = 0x4D334155;
                    dec.border = Border(0x8038BDF8, 1.0f);
                    dec.box_shadow = { BoxShadow::glow(0x3538BDF8, 8.0f) };
                } else {
                    dec.color = colors::KeyNumBg;
                    dec.border = Border(colors::KeyNumBorder, 1.0f);
                    dec.box_shadow = { BoxShadow(0x30000000, {0.0f, 2.0f}, 4.0f) };
                }
                text_color = is_hovered ? 0xFFFFFFFF : colors::KeyNumText;
                font_size = 20.0f;
                break;

            case ButtonType::Operator:
                if (is_pressed) {
                    dec.color = 0x884338CA;
                    dec.border = Border(0xFF818CF8, 1.5f);
                    dec.box_shadow = { BoxShadow::glow(0x806366F1, 14.0f) };
                } else if (is_hovered) {
                    dec.color = 0x55312E81;
                    dec.border = Border(0xCC818CF8, 1.5f);
                    dec.box_shadow = { BoxShadow::glow(0x506366F1, 12.0f) };
                } else {
                    dec.color = colors::KeyOpBg;
                    dec.border = Border(colors::KeyOpBorder, 1.0f);
                    dec.box_shadow = { BoxShadow::glow(0x286366F1, 8.0f) };
                }
                text_color = is_hovered ? 0xFFBAE6FD : colors::KeyOpText;
                font_size = 22.0f;
                weight = FontWeight::Bold;
                break;

            case ButtonType::Action:
                dec.background_shader = SHADER_CLEAR_KEY; // Live SkSL pulse
                if (is_pressed) {
                    dec.border = Border(0xFFFFFFFF, 2.0f);
                    dec.box_shadow = { BoxShadow::glow(0xAAEF4444, 20.0f) };
                } else if (is_hovered) {
                    dec.border = Border(0xFFF87171, 1.5f);
                    dec.box_shadow = { BoxShadow::glow(0x80EF4444, 16.0f) };
                } else {
                    dec.border = Border(colors::KeyActionBorder, 1.0f);
                    dec.box_shadow = { BoxShadow::glow(0x40EF4444, 10.0f) };
                }
                text_color = colors::KeyActionText;
                font_size = 17.0f;
                weight = FontWeight::Bold;
                break;

            case ButtonType::Scientific:
                if (is_pressed) {
                    dec.color = 0x6620334D;
                    dec.border = Border(0xFF00E5FF, 1.5f);
                    dec.box_shadow = { BoxShadow::glow(0x6000E5FF, 10.0f) };
                } else if (is_hovered) {
                    dec.color = 0x4420334D;
                    dec.border = Border(0x9938BDF8, 1.0f);
                    dec.box_shadow = { BoxShadow::glow(0x3538BDF8, 8.0f) };
                } else {
                    dec.color = colors::KeySciBg;
                    dec.border = Border(colors::KeySciBorder, 1.0f);
                    dec.box_shadow = { BoxShadow(0x20000000, {0.0f, 2.0f}, 4.0f) };
                }
                text_color = is_hovered ? 0xFFE2E8F0 : colors::KeySciText;
                font_size = 15.0f;
                weight = FontWeight::Medium;
                break;

            case ButtonType::Equals:
                dec.background_shader = SHADER_EQUALS_KEY; // Live high-energy SkSL core!
                if (is_pressed) {
                    dec.border = Border(0xFFFFFFFF, 2.0f);
                    dec.box_shadow = {
                        BoxShadow::glow(0xFF00E5FF, 26.0f),
                        BoxShadow::glow(0xCCBD00FF, 32.0f),
                        BoxShadow::standard(0x80000000, 12.0f, 6.0f)
                    };
                } else if (is_hovered) {
                    dec.border = Border(0xFFFFFFFF, 2.0f);
                    dec.box_shadow = {
                        BoxShadow::glow(0xB000E5FF, 22.0f),
                        BoxShadow::glow(0x90BD00FF, 28.0f),
                        BoxShadow::standard(0x60000000, 10.0f, 4.0f)
                    };
                } else {
                    dec.border = Border(0x90FFFFFF, 1.5f);
                    dec.box_shadow = {
                        BoxShadow::glow(0x8000E5FF, 16.0f),
                        BoxShadow::glow(0x60BD00FF, 22.0f),
                        BoxShadow::standard(0x60000000, 10.0f, 4.0f)
                    };
                }
                text_color = colors::KeyEqualsText;
                font_size = 24.0f;
                weight = FontWeight::Bold;
                break;
        }

        StyleInsets content_margin = is_pressed ? StyleInsets::only(1.0f, 0.0f, 0.0f, 0.0f) : StyleInsets{};

        auto content = container({
            .color = dec.color,
            .border_radius = dec.border_radius,
            .border = dec.border,
            .box_shadow = dec.box_shadow,
            .background_shader = dec.background_shader,
            .align = Alignment::Center,
            .width = 100_pct,
            .height = (btn->type == ButtonType::Scientific) ? 42_px : 54_px,
            .margin = content_margin,
            .child = text(btn->label, {
                .color = text_color,
                .font_size = font_size,
                .font_weight = weight,
            })
        });

        GestureDetectorProps gprops;
        gprops.child = content;
        gprops.cursor_type = SystemCursor::Pointer;
        gprops.on_hover_enter = [this](const PointerEvent&) {
            setState([this]() { is_hovered = true; });
        };
        gprops.on_hover_exit = [this](const PointerEvent&) {
            setState([this]() { is_hovered = false; is_pressed = false; });
        };
        gprops.on_tap_down = [this](const TapDownDetails&) {
            setState([this]() { is_pressed = true; });
        };
        gprops.on_tap_up = [this](const TapUpDetails&) {
            setState([this]() { is_pressed = false; });
        };
        gprops.on_tap_cancel = [this]() {
            setState([this]() { is_pressed = false; });
        };
        gprops.on_tap = [btn]() {
            if (btn->on_tap) btn->on_tap();
        };

        auto gd = gestureDetector(std::move(gprops));

        return container({
            .flex = btn->flex_val,
            .child = gd
        });
    }
};

inline std::unique_ptr<State> CalcButton::createState() {
    return std::make_unique<CalcButtonState>();
}

class SmallPill : public StatefulWidget {
public:
    std::string label;
    std::function<void()> on_tap;
    bool is_highlighted = false;

    SmallPill(std::string label, std::function<void()> on_tap, bool is_highlighted = false)
        : label(std::move(label)), on_tap(std::move(on_tap)), is_highlighted(is_highlighted) {}

    std::unique_ptr<State> createState() override;
    std::string_view typeName() const override { return "SmallPill"; }
};

class SmallPillState : public State {
public:
    bool is_hovered = false;
    bool is_pressed = false;

    WidgetPtr build(BuildContext&) override {
        auto* pill = static_cast<const SmallPill*>(widget());

        Color bg = pill->is_highlighted ? (is_hovered ? 0x60F59E0B : 0x40F59E0B)
                                        : (is_pressed ? 0x44334155 : (is_hovered ? 0x33334155 : 0x201E293B));
        Border border = pill->is_highlighted ? Border(is_hovered ? 0xFFFCD34D : 0x80F59E0B, 1.0f)
                                             : Border(is_hovered ? 0x8038BDF8 : 0x30475569, 1.0f);
        Color txt_col = pill->is_highlighted ? 0xFFFCD34D : (is_hovered ? 0xFF38BDF8 : 0xFF94A3B8);

        auto content = container({
            .color = bg,
            .border_radius = BorderRadius::circular(8.0f),
            .border = border,
            .padding = StyleInsets::symmetric(5.0f, 10.0f),
            .child = text(pill->label, {
                .color = txt_col,
                .font_size = 11.5f,
                .font_weight = FontWeight::SemiBold
            })
        });

        GestureDetectorProps gprops;
        gprops.child = content;
        gprops.cursor_type = SystemCursor::Pointer;
        gprops.on_hover_enter = [this](const PointerEvent&) {
            setState([this]() { is_hovered = true; });
        };
        gprops.on_hover_exit = [this](const PointerEvent&) {
            setState([this]() { is_hovered = false; is_pressed = false; });
        };
        gprops.on_tap_down = [this](const TapDownDetails&) {
            setState([this]() { is_pressed = true; });
        };
        gprops.on_tap_up = [this](const TapUpDetails&) {
            setState([this]() { is_pressed = false; });
        };
        gprops.on_tap_cancel = [this]() {
            setState([this]() { is_pressed = false; });
        };
        gprops.on_tap = [pill]() {
            if (pill->on_tap) pill->on_tap();
        };

        return gestureDetector(std::move(gprops));
    }
};

inline std::unique_ptr<State> SmallPill::createState() {
    return std::make_unique<SmallPillState>();
}

class CalculatorAppState : public State {
public:
    CalculatorEngine engine;
    bool show_history = false;
    bool trigger_particles = false;
    int64_t particle_seed = 0;

    CalculatorAppState() = default;

    WidgetPtr buildButton(const std::string& label, ButtonType type, std::function<void()> on_tap, float flex_val = 1.0f) {
        return std::make_shared<CalcButton>(label, type, [this, on_tap]() {
            on_tap();
            setState([](){});
        }, flex_val);
    }

    WidgetPtr buildSmallPill(const std::string& label, std::function<void()> cb, bool is_highlighted = false) {
        return std::make_shared<SmallPill>(label, [this, cb]() {
            cb();
            setState([](){});
        }, is_highlighted);
    }

    WidgetPtr buildScreen() {
        // Dynamic font size scaling for long numbers
        std::string main_txt = engine.displayMain();
        float font_size = 38.0f;
        if (main_txt.length() > 14) font_size = 24.0f;
        else if (main_txt.length() > 10) font_size = 30.0f;

        Color main_color = engine.hasError() ? 0xFFF87171 : colors::AccentCyan;

        // Top line: Badges & Expression
        auto top_row = row({
            .justify_content = Justify::SpaceBetween,
            .align_items = Align::Center,
            .width = 100_pct,
            .children = {
                // Left indicators (DEG/RAD, MEM)
                row({
                    .align_items = Align::Center,
                    .gap = StyleValue::point(8.0f),
                    .children = {
                        // DEG / RAD Clickable Pill
                        gestureDetector(GestureDetectorProps{
                            .child = container({
                                .color = 0x3338BDF8,
                                .border_radius = BorderRadius::circular(6.0f),
                                .border = Border(0x6638BDF8, 1.0f),
                                .padding = StyleInsets::symmetric(3.0f, 8.0f),
                                .child = text(engine.angleModeStr(), {
                                    .color = 0xFF38BDF8,
                                    .font_size = 11.0f,
                                    .font_weight = FontWeight::Bold
                                })
                            }),
                            .cursor_type = SystemCursor::Pointer,
                            .on_tap = [this]() {
                                engine.toggleAngleMode();
                                setState([](){});
                            },
                        }),
                        // MEM Indicator
                        container({
                            .color = engine.hasMemory() ? 0x40F59E0B : 0x15FFFFFF,
                            .border_radius = BorderRadius::circular(6.0f),
                            .border = Border(engine.hasMemory() ? 0x80F59E0B : 0x25FFFFFF, 1.0f),
                            .padding = StyleInsets::symmetric(3.0f, 6.0f),
                            .child = text("MEM", {
                                .color = engine.hasMemory() ? 0xFFFBBF24 : 0x50FFFFFF,
                                .font_size = 10.0f,
                                .font_weight = FontWeight::SemiBold
                            })
                        })
                    }
                }),
                // Right Expression Tape
                text(engine.displayExpression().empty() ? " " : engine.displayExpression(), {
                    .color = 0xFF94A3B8,
                    .font_size = 14.0f,
                    .font_weight = FontWeight::Medium
                })
            }
        });

        // Hero Main Number Display
        auto main_row = row({
            .justify_content = Justify::End,
            .align_items = Align::Center,
            .width = 100_pct,
            .margin = StyleInsets::only(10.0f, 0.0f, 0.0f, 0.0f),
            .children = {
                text(main_txt, {
                    .color = main_color,
                    .font_size = font_size,
                    .font_weight = FontWeight::Bold,
                })
            }
        });

        auto screen_content = column({
            .justify_content = Justify::SpaceBetween,
            .width = 100_pct,
            .height = 100_pct,
            .children = { top_row, main_row }
        });

        // Framed inside Vector SVG HUD Frame
        auto screen_box = container({
            .border_svg = SVG_HUD_SCREEN_FRAME,
            .svg_fit = SvgFit::Stretch,
            .width = 100_pct,
            .height = 126_px,
            .padding = StyleInsets::all(16.0f),
            .child = screen_content
        });

        // Screen framed in Vector SVG HUD Frame, with layered celebratory particle burst
        return stack({
            screen_box,
            particleEmitter(ParticleEmitterProps{
                .preset = ParticlePreset::NeonSparks,
                .active = trigger_particles,
                .key = Key::value(particle_seed)
            })
        });
    }

    WidgetPtr buildToolBar() {
        return row({
            .justify_content = Justify::SpaceBetween,
            .align_items = Align::Center,
            .width = 100_pct,
            .margin = StyleInsets::only(14.0f, 0.0f, 10.0f, 0.0f),
            .children = {
                // Memory Buttons (MC, MR, M+, M-)
                row({
                    .gap = StyleValue::point(6.0f),
                    .children = {
                        buildSmallPill("MC", [this]() { engine.memoryClear(); }),
                        buildSmallPill("MR", [this]() { engine.memoryRecall(); }),
                        buildSmallPill("M+", [this]() { engine.memoryAdd(); }),
                        buildSmallPill("M-", [this]() { engine.memorySubtract(); }),
                    }
                }),

                // History Toggle Button with Hover
                buildSmallPill(std::string("📜 ") + std::to_string(engine.history().size()), [this]() {
                    show_history = !show_history;
                    setState([](){});
                }, show_history)
            }
        });
    }

    WidgetPtr buildScientificGrid() {
        return column({
            .gap = StyleValue::point(8.0f),
            .margin = StyleInsets::only(0.0f, 0.0f, 10.0f, 0.0f),
            .children = {
                row({
                    .gap = StyleValue::point(8.0f),
                    .children = {
                        buildButton("sin", ButtonType::Scientific, [this]() { engine.inputFunction("sin"); }),
                        buildButton("cos", ButtonType::Scientific, [this]() { engine.inputFunction("cos"); }),
                        buildButton("tan", ButtonType::Scientific, [this]() { engine.inputFunction("tan"); }),
                        buildButton("π", ButtonType::Scientific, [this]() { engine.inputConstant("pi"); }),
                        buildButton("e", ButtonType::Scientific, [this]() { engine.inputConstant("e"); }),
                    }
                }),
                row({
                    .gap = StyleValue::point(8.0f),
                    .children = {
                        buildButton("√", ButtonType::Scientific, [this]() { engine.inputFunction("sqrt"); }),
                        buildButton("x²", ButtonType::Scientific, [this]() { engine.inputFunction("sqr"); }),
                        buildButton("^", ButtonType::Scientific, [this]() { engine.inputOperator('^'); }),
                        buildButton("(", ButtonType::Scientific, [this]() { engine.inputParenthesis('('); }),
                        buildButton(")", ButtonType::Scientific, [this]() { engine.inputParenthesis(')'); }),
                    }
                }),
                row({
                    .gap = StyleValue::point(8.0f),
                    .children = {
                        buildButton("ln", ButtonType::Scientific, [this]() { engine.inputFunction("ln"); }),
                        buildButton("log", ButtonType::Scientific, [this]() { engine.inputFunction("log"); }),
                        buildButton("1/x", ButtonType::Scientific, [this]() { engine.inputFunction("inv"); }),
                        buildButton("n!", ButtonType::Scientific, [this]() { engine.inputFunction("fact"); }),
                        buildButton("|x|", ButtonType::Scientific, [this]() { engine.inputFunction("abs"); }),
                    }
                })
            }
        });
    }

    WidgetPtr buildMainKeypad() {
        return column({
            .gap = StyleValue::point(10.0f),
            .children = {
                // Row 1: AC, DEL, %, ÷
                row({
                    .gap = StyleValue::point(10.0f),
                    .children = {
                        buildButton("AC", ButtonType::Action, [this]() {
                            engine.clearAll();
                            trigger_particles = false;
                        }),
                        buildButton("DEL", ButtonType::Scientific, [this]() {
                            engine.backspace();
                        }),
                        buildButton("%", ButtonType::Operator, [this]() {
                            engine.inputOperator('%');
                        }),
                        buildButton("÷", ButtonType::Operator, [this]() {
                            engine.inputOperator('/');
                        })
                    }
                }),

                // Row 2: 7, 8, 9, ×
                row({
                    .gap = StyleValue::point(10.0f),
                    .children = {
                        buildButton("7", ButtonType::Digit, [this]() { engine.inputDigit('7'); }),
                        buildButton("8", ButtonType::Digit, [this]() { engine.inputDigit('8'); }),
                        buildButton("9", ButtonType::Digit, [this]() { engine.inputDigit('9'); }),
                        buildButton("×", ButtonType::Operator, [this]() { engine.inputOperator('*'); })
                    }
                }),

                // Row 3: 4, 5, 6, −
                row({
                    .gap = StyleValue::point(10.0f),
                    .children = {
                        buildButton("4", ButtonType::Digit, [this]() { engine.inputDigit('4'); }),
                        buildButton("5", ButtonType::Digit, [this]() { engine.inputDigit('5'); }),
                        buildButton("6", ButtonType::Digit, [this]() { engine.inputDigit('6'); }),
                        buildButton("−", ButtonType::Operator, [this]() { engine.inputOperator('-'); })
                    }
                }),

                // Row 4: 1, 2, 3, +
                row({
                    .gap = StyleValue::point(10.0f),
                    .children = {
                        buildButton("1", ButtonType::Digit, [this]() { engine.inputDigit('1'); }),
                        buildButton("2", ButtonType::Digit, [this]() { engine.inputDigit('2'); }),
                        buildButton("3", ButtonType::Digit, [this]() { engine.inputDigit('3'); }),
                        buildButton("+", ButtonType::Operator, [this]() { engine.inputOperator('+'); })
                    }
                }),

                // Row 5: ±, 0, ., =
                row({
                    .gap = StyleValue::point(10.0f),
                    .children = {
                        buildButton("±", ButtonType::Digit, [this]() { engine.toggleSign(); }),
                        buildButton("0", ButtonType::Digit, [this]() { engine.inputDigit('0'); }),
                        buildButton(".", ButtonType::Digit, [this]() { engine.inputDecimal(); }),
                        buildButton("=", ButtonType::Equals, [this]() {
                            bool ok = engine.evaluate();
                            if (ok) {
                                trigger_particles = true;
                                particle_seed++;
                            }
                        })
                    }
                })
            }
        });
    }

    WidgetPtr buildHistoryView() {
        std::vector<WidgetPtr> items;

        if (engine.history().empty()) {
            items.push_back(container({
                .align = Alignment::Center,
                .padding = StyleInsets::all(40_px),
                .child = text("No calculations yet.", {
                    .color = 0xFF64748B,
                    .font_size = 14.0f
                })
            }));
        } else {
            for (size_t i = engine.history().size(); i > 0; --i) {
                size_t idx = i - 1;
                const auto& h = engine.history()[idx];

                auto card = gestureDetector(GestureDetectorProps{
                    .child = container({
                        .color = 0x251E293B,
                        .border_radius = BorderRadius::circular(10.0f),
                        .border = Border(0x30475569, 1.0f),
                        .padding = StyleInsets::all(12_px),
                        .margin = StyleInsets::only(0.0f, 0.0f, 8.0f, 0.0f),
                        .child = column({
                            .gap = StyleValue::point(4.0f),
                            .children = {
                                text(h.expression, {
                                    .color = 0xFF94A3B8,
                                    .font_size = 13.0f
                                }),
                                text("= " + h.result, {
                                    .color = colors::AccentCyan,
                                    .font_size = 18.0f,
                                    .font_weight = FontWeight::Bold
                                })
                            }
                        })
                    }),
                    .cursor_type = SystemCursor::Pointer,
                    .on_tap = [this, idx]() {
                        engine.recallHistory(idx);
                        show_history = false;
                        setState([](){});
                    },
                });

                items.push_back(card);
            }

            // Clear History Button
            items.push_back(gestureDetector(GestureDetectorProps{
                .child = container({
                    .color = 0x30EF4444,
                    .border_radius = BorderRadius::circular(8.0f),
                    .border = Border(0x60EF4444, 1.0f),
                    .align = Alignment::Center,
                    .padding = StyleInsets::symmetric(8.0f, 16.0f),
                    .margin = StyleInsets::only(12.0f, 0.0f, 0.0f, 0.0f),
                    .child = text("Clear History", {
                        .color = 0xFFFCA5A5,
                        .font_size = 13.0f,
                        .font_weight = FontWeight::Bold
                    })
                }),
                .cursor_type = SystemCursor::Pointer,
                .on_tap = [this]() {
                    engine.clearHistory();
                    setState([](){});
                },
            }));
        }

        return container({
            .color = 0x4D000000,
            .border_radius = BorderRadius::circular(14.0f),
            .border = Border(0x3038BDF8, 1.0f),
            .height = 360_px,
            .padding = StyleInsets::all(14_px),
            .child = scrollView(column({
                .width = 100_pct,
                .children = std::move(items)
            }))
        });
    }

    WidgetPtr build(BuildContext&) override {
        // App Body
        auto content_col = column({
            .width = 100_pct,
            .children = {
                buildScreen(),
                buildToolBar(),
                show_history ? buildHistoryView() : column({
                    .width = 100_pct,
                    .children = {
                        buildScientificGrid(),
                        buildMainKeypad()
                    }
                })
            }
        });

        auto app_body = container({
            .color = colors::WindowBg,
            .width = 100_pct,
            .height = 100_pct,
            .padding = StyleInsets::all(16_px),
            .child = content_col
        });

        // Wrap the entire app in WindowFrame (CSD) with glowing SkSL neon border_shader
        return windowFrame(WindowFrameProps{
            .content = app_body,
            .title = "QUANTUM PRECISION",
            .border_radius = 16.0f,
            .border_width = 2.0f,
            .background_color = 0x4D000000,
            .background_shader = SHADER_WINDOW_BACKGROUND, // Ambient holographic aurora
            .border_shader = SHADER_WINDOW_BORDER,         // Rotating dual-neon stroke
            .titlebar_background_color = 0x4D000000,
            .titlebar_inactive_background_color = 0x4D000000,
            .titlebar_style = TitleBarStyle::VAXPOS,
        });
    }
};

class CalculatorApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<CalculatorAppState>();
    }
    std::string_view typeName() const override { return "CalculatorApp"; }
};

int main() {
    std::cout << "================================================\n";
    std::cout << "  ENKI Engine — Calculator    \n";
    std::cout << "  Real-World Showcase Application (CSD + Shaders)\n";
    std::cout << "================================================\n";

    AppConfig config;
    config.title       = "ENKI Calculator";
    config.width       = 460;
    config.height      = 720;
    config.resizable   = true;
    config.enable_csd  = true; // Enable Client-Side Decorations
    config.app_id      = "org.enki.quantum_calculator";
    config.vsync       = false;
    config.target_fps  = 60;
    config.show_performance_overlay = false;
    config.clear_color = 0x0000004D; // Translucent dark glass native surface

    return runApp(std::make_shared<CalculatorApp>(), config);
}
