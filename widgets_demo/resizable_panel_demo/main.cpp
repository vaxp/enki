/// @file main.cpp
/// @brief ENKI Advanced ResizablePanel & Floating Tool Window Interactive Showcase Demo.

#include "enki/app/app.hpp"
#include "enki/widgets/resizable_panel.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/state/state.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <memory>

using namespace enki;

class ResizablePanelDemoState : public State {
private:
    std::shared_ptr<ResizablePanelController> panel_ctrl_;
    std::string hud_msg_ = "Drag the top title bar to MOVE the tool window, or drag the bottom-right corner grip (◢) to RESIZE it!";

    // ── Build Inspector Window Content ────────────────────────────
    WidgetPtr buildInspectorContent() {
        return column({
            .gap = StyleValue::point(10.0f),
            .children = {
                text("⚡ Real-Time Vulkan Shader Pipeline Inspector", { .color = 0xFF38BDF8, .font_size = 13.0f, .font_weight = FontWeight::Bold }),
                text("• Render Target: 1920x1080 (Skia VK_FORMAT_R8G8B8A8_UNORM)", { .color = 0xFF94A3B8, .font_size = 11.5f }),
                text("• Frame Latency: 4.2ms • VRAM Usage: 184MB / 8192MB", { .color = 0xFF10B981, .font_size = 11.5f }),
                text("• Active Pipeline: SPIR-V Fragment Stage (Optimization: O3)", { .color = 0xFFCBD5E1, .font_size = 11.5f }),
                button(text("Recompile Shader"), [this] {
                    hud_msg_ = "Action: SPIR-V Shader pipeline recompiled successfully (0 warnings).";
                    setState([] {});
                })
            }
        });
    }

public:
    void initState() override {
        State::initState();
        panel_ctrl_ = std::make_shared<ResizablePanelController>();
    }

    WidgetPtr build(BuildContext&) override {
        auto background_page = container({
            .color = 0xFF0B1120,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(24.0f),
            .child = column({
                .align_items = Align::Center,
                .gap = StyleValue::point(18.0f),
                .children = {
                    column({
                        .align_items = Align::Center,
                        .gap = StyleValue::point(6.0f),
                        .children = {
                            text("Advanced ResizablePanel Component Suite", { .color = 0xFFFFFFFF, .font_size = 22.0f, .font_weight = FontWeight::Bold }),
                            text("Floating window & dockable tool panel (Category 10. Advanced / Data UI), drag-to-move, corner grip resizing, and minimize/maximize", { .color = 0xFF94A3B8, .font_size = 13.0f })
                        }
                    }),
                    row({
                        .justify_content = Justify::Center,
                        .gap = StyleValue::point(10.0f),
                        .children = {
                            button(text("Preset: Compact (380x240)"), [this] {
                                panel_ctrl_->setSize(380.0f, 240.0f);
                                hud_msg_ = "Resized window to Compact (380x240).";
                                setState([] {});
                            }),
                            button(text("Preset: Medium (520x340)"), [this] {
                                panel_ctrl_->setSize(520.0f, 340.0f);
                                hud_msg_ = "Resized window to Medium (520x340).";
                                setState([] {});
                            }),
                            button(text("Preset: Large (680x420)"), [this] {
                                panel_ctrl_->setSize(680.0f, 420.0f);
                                hud_msg_ = "Resized window to Large (680x420).";
                                setState([] {});
                            }),
                            button(text("Reset Position & Size"), [this] {
                                panel_ctrl_->reset();
                                hud_msg_ = "Reset window to default position (240, 120) and size (460x320).";
                                setState([] {});
                            })
                        }
                    }),
                    container({
                        .color = 0xFF0F172A,
                        .border_radius = BorderRadius::circular(12.0f),
                        .border = Border(0xFF334155, 1.0f),
                        .width = StyleValue::point(960.0f),
                        .height = StyleValue::point(380.0f),
                        .padding = StyleInsets::all(40.0f),
                        .child = column({
                            .align_items = Align::Center,
                            .gap = StyleValue::point(6.0f),
                            .children = {
                                text("🖥️ Workspace Canvas — Floating Tool Window Layered on Top", { .color = 0xFF64748B, .font_size = 14.0f, .font_weight = FontWeight::Bold }),
                                text("Drag the window around this desktop surface or resize it dynamically.", { .color = 0xFF475569, .font_size = 12.5f })
                            }
                        })
                    }),
                    container({
                        .color = 0xFF1E293B,
                        .border_radius = BorderRadius::circular(6.0f),
                        .border = Border(0xFF334155, 1.0f),
                        .width = StyleValue::point(960.0f),
                        .padding = StyleInsets::symmetric(8.0f, 16.0f),
                        .child = row({
                            .children = {
                                text("💡 " + hud_msg_, { .color = 0xFF38BDF8, .font_size = 12.5f })
                            }
                        })
                    })
                }
            })
        });

        return resizablePanel({
            .child = buildInspectorContent(),
            .body = background_page,
            .options = {
                .title = "Vulkan Shader Inspector",
                .icon = "⚡",
                .initial_x = 340.0f,
                .initial_y = 190.0f,
                .initial_width = 480.0f,
                .initial_height = 260.0f,
                .on_resized = [this](float w, float h) {
                    hud_msg_ = "Window Resized: " + std::to_string(static_cast<int>(w)) + "px × " + std::to_string(static_cast<int>(h)) + "px";
                    setState([] {});
                },
                .on_moved = [this](float x, float y) {
                    hud_msg_ = "Window Position: (" + std::to_string(static_cast<int>(x)) + ", " + std::to_string(static_cast<int>(y)) + ")";
                    setState([] {});
                }
            },
            .controller = panel_ctrl_
        });
    }
};

class ResizablePanelDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<ResizablePanelDemoState>();
    }
    std::string_view typeName() const override { return "ResizablePanelDemoApp"; }
};

int main() {
    std::cout << "====================================================\n";
    std::cout << "  ENKI Engine — Advanced ResizablePanel Demo\n";
    std::cout << "====================================================\n";

    AppConfig config;
    config.title       = "Enki Engine — Advanced ResizablePanel Demo";
    config.width       = 1180;
    config.height      = 720;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 60;
    config.clear_color = 0xFF0B1120;

    return runApp(std::make_shared<ResizablePanelDemoApp>(), config);
}
