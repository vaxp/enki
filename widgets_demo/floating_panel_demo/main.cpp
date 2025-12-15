/// @file main.cpp
/// @brief ENKI Advanced FloatingPanel Overlay Showcase Demo.

#include "enki/app/app.hpp"
#include "enki/widgets/floating_panel.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/slider.hpp"
#include "enki/widgets/switch.hpp"
#include "enki/widgets/stack.hpp"
#include "enki/state/state.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <iomanip>
#include <sstream>

using namespace enki;

class FloatingPanelDemoState : public State {
private:
    std::shared_ptr<FloatingPanelController> panel_ctrl_;

    float gain_val_ = 0.75f;
    float resonance_val_ = 0.45f;
    bool eq_active_ = true;
    bool limiter_active_ = false;

    float current_x_ = 280.0f;
    float current_y_ = 110.0f;
    float current_w_ = 440.0f;
    float current_h_ = 360.0f;
    std::string hud_status_ = "Panel active: Drag title bar to move, drag edges to resize.";

    WidgetPtr buildInspectorContent() {
        auto make_slider_row = [](std::string label, float val, auto on_chg) -> WidgetPtr {
            std::stringstream ss;
            ss << std::fixed << std::setprecision(2) << val;
            return column({
                .gap = StyleValue::point(4.0f),
                .children = {
                    row({
                        .justify_content = Justify::SpaceBetween,
                        .align_items = Align::Center,
                        .children = {
                            text(label, { .color = 0xFFCBD5E1, .font_size = 12.5f }),
                            text(ss.str(), { .color = 0xFF38BDF8, .font_size = 12.0f, .font_weight = FontWeight::Bold })
                        }
                    }),
                    Slider {
                        .value = val,
                        .on_change = on_chg,
                        .active_color = 0xFF38BDF8,
                        .inactive_color = 0xFF1E293B,
                        .min_value = 0.0f,
                        .max_value = 1.0f
                    }
                }
            });
        };

        auto make_switch_row = [](std::string label, bool val, auto on_chg) -> WidgetPtr {
            return row({
                .justify_content = Justify::SpaceBetween,
                .align_items = Align::Center,
                .children = {
                    text(label, { .color = 0xFFCBD5E1, .font_size = 12.5f }),
                    Switch {
                        .value = val,
                        .on_changed = on_chg,
                        .active_color = 0xFF10B981
                    }
                }
            });
        };

        return column({
            .gap = StyleValue::point(14.0f),
            .children = {
                text("AUDIO DSP MATRIX & EQUALIZER", {
                    .color = 0xFF64748B,
                    .font_size = 10.5f,
                    .font_weight = FontWeight::Bold
                }),
                make_slider_row("Master Output Gain", gain_val_, [this](float v) {
                    gain_val_ = v;
                    setState([] {});
                }),
                make_slider_row("Resonance Q-Factor", resonance_val_, [this](float v) {
                    resonance_val_ = v;
                    setState([] {});
                }),
                container({
                    .color = 0xFF1E293B,
                    .height = StyleValue::point(1.0f)
                }),
                make_switch_row("Parametric EQ Enable", eq_active_, [this](bool b) {
                    eq_active_ = b;
                    setState([] {});
                }),
                make_switch_row("Brickwall Peak Limiter", limiter_active_, [this](bool b) {
                    limiter_active_ = b;
                    setState([] {});
                })
            }
        });
    }

public:
    void initState() override {
        State::initState();
        panel_ctrl_ = std::make_shared<FloatingPanelController>();
    }

    WidgetPtr build(BuildContext&) override {
        // ── Main Page Background (Studio Workspace) ───────────────────
        auto header_toolbar = container({
            .color = 0xFF0F172A,
            .border = Border(0xFF1E293B, 1.0f),
            .padding = StyleInsets::symmetric(14.0f, 28.0f),
            .child = row({
                .justify_content = Justify::SpaceBetween,
                .align_items = Align::Center,
                .children = {
                    row({
                        .align_items = Align::Center,
                        .gap = StyleValue::point(12.0f),
                        .children = {
                            text("🎛️", { .color = 0xFF38BDF8, .font_size = 22.0f }),
                            text("ENKI STUDIO SUITE", {
                                .color = 0xFFF8FAFC,
                                .font_size = 16.0f,
                                .font_weight = FontWeight::Bold
                            })
                        }
                    }),
                    row({
                        .align_items = Align::Center,
                        .gap = StyleValue::point(10.0f),
                        .children = {
                            // Re-open / Show Panel Button
                            gestureDetector({
                                .child = container({
                                    .color = 0xFF1E293B,
                                    .border_radius = BorderRadius::circular(6.0f),
                                    .border = Border(0xFF334155, 1.0f),
                                    .padding = StyleInsets::symmetric(8.0f, 14.0f),
                                    .child = text("Open Floating HUD", {
                                        .color = 0xFFF8FAFC,
                                        .font_size = 12.5f,
                                        .font_weight = FontWeight::Bold
                                    })
                                }),
                                .cursor_type = SystemCursor::Pointer,
                                .on_tap_up = [this](const TapUpDetails&) {
                                    panel_ctrl_->show();
                                    panel_ctrl_->bringToFront();
                                }
                            }),
                            // Reset Position Button
                            gestureDetector({
                                .child = container({
                                    .color = 0x2238BDF8,
                                    .border_radius = BorderRadius::circular(6.0f),
                                    .padding = StyleInsets::symmetric(8.0f, 14.0f),
                                    .child = text("Reset Layout", {
                                        .color = 0xFF38BDF8,
                                        .font_size = 12.5f,
                                        .font_weight = FontWeight::Bold
                                    })
                                }),
                                .cursor_type = SystemCursor::Pointer,
                                .on_tap_up = [this](const TapUpDetails&) {
                                    panel_ctrl_->setPosition(280.0f, 110.0f);
                                    panel_ctrl_->setSize(440.0f, 360.0f);
                                    panel_ctrl_->restore();
                                    panel_ctrl_->show();
                                }
                            })
                        }
                    })
                }
            })
        });

        auto status_bar = container({
            .color = 0xFF0B0F19,
            .border_radius = BorderRadius::circular(10.0f),
            .border = Border(0xFF1E293B, 1.0f),
            .padding = StyleInsets::all(16.0f),
            .child = row({
                .justify_content = Justify::SpaceBetween,
                .align_items = Align::Center,
                .children = {
                    text(hud_status_, { .color = 0xFF10B981, .font_size = 13.0f }),
                    text("Pos: (" + std::to_string((int)current_x_) + ", " + std::to_string((int)current_y_) + ") • Size: " +
                         std::to_string((int)current_w_) + "×" + std::to_string((int)current_h_), {
                        .color = 0xFF64748B,
                        .font_size = 12.0f
                    })
                }
            })
        });

        auto workspace_body = container({
            .color = 0xFF080C14,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .child = column({
                .gap = StyleValue::point(24.0f),
                .children = {
                    header_toolbar,
                    container({
                        .padding = StyleInsets::symmetric(0.0f, 36.0f),
                        .child = status_bar
                    })
                }
            })
        });

        // ── FloatingPanel Declarative Overlay ─────────────────────────
        return FloatingPanel {
            .content = buildInspectorContent(),
            .body = workspace_body,
            .options = {
                .title = "DSP Audio Inspector",
                .icon = "🎛️",
                .initial_x = current_x_,
                .initial_y = current_y_,
                .initial_width = current_w_,
                .initial_height = current_h_,
                .allow_drag = true,
                .allow_resize = true,
                .allow_minimize = true,
                .allow_maximize = true,
                .allow_close = true,
                .on_moved = [this](float x, float y) {
                    current_x_ = x;
                    current_y_ = y;
                    hud_status_ = "Panel moved to (" + std::to_string((int)x) + ", " + std::to_string((int)y) + ")";
                    setState([] {});
                },
                .on_resized = [this](float w, float h) {
                    current_w_ = w;
                    current_h_ = h;
                    hud_status_ = "Panel resized to " + std::to_string((int)w) + " × " + std::to_string((int)h);
                    setState([] {});
                },
                .on_drag_update = [this](float x, float y) {
                    current_x_ = x;
                    current_y_ = y;
                },
                .on_resize_update = [this](float w, float h) {
                    current_w_ = w;
                    current_h_ = h;
                },
                .on_closed = [this] {
                    hud_status_ = "Floating HUD closed. Click 'Open Floating HUD' in header to reopen.";
                    setState([] {});
                }
            },
            .controller = panel_ctrl_
        };
    }
};

class FloatingPanelDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<FloatingPanelDemoState>();
    }
    [[nodiscard]] std::string_view typeName() const override { return "FloatingPanelDemoApp"; }
};

int main() {
    std::cout << "====================================================\n";
    std::cout << "  ENKI Engine — FloatingPanel Overlay Showcase Demo\n";
    std::cout << "====================================================\n";

    AppConfig config;
    config.title       = "Enki Engine — FloatingPanel Overlay Showcase Demo";
    config.width       = 1140;
    config.height      = 660;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 0;
    config.show_performance_overlay = true;
    config.clear_color = 0xFF080C14;

    return runApp(std::make_shared<FloatingPanelDemoApp>(), config);
}
