/// @file main.cpp
/// @brief ENKI Advanced ExpansionPanel Interactive Showcase Demo.

#include "enki/app/app.hpp"
#include "enki/widgets/expansion_panel.hpp"
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

class ExpansionPanelDemoState : public State {
private:
    std::shared_ptr<ExpansionPanelController> panel_ctrl_;
    bool is_radio_mode_ = true;
    std::string hud_msg_ = "Click any step panel header to expand, or click [Next Step] in the footer action bar.";

    // ── Step 1 Body: Compute & GPU ────────────────────────────────
    WidgetPtr buildStep1Body() {
        return column({
            .gap = StyleValue::point(8.0f),
            .children = {
                text("⚡ Node Sizing & Hardware Acceleration", { .color = 0xFFFFFFFF, .font_size = 13.5f, .font_weight = FontWeight::Bold }),
                text("Allocate compute instances for real-time Vulkan Skia compositing nodes.", { .color = 0xFF94A3B8, .font_size = 12.0f }),
                text("Selected: 16 vCPUs • 64GB ECC RAM • 1x NVIDIA H100 GPU", { .color = 0xFF38BDF8, .font_size = 12.5f, .font_weight = FontWeight::Bold })
            }
        });
    }

    // ── Step 2 Body: Network & Security ───────────────────────────
    WidgetPtr buildStep2Body() {
        return column({
            .gap = StyleValue::point(8.0f),
            .children = {
                text("🔒 Zero-Trust Virtual Private Cloud (VPC)", { .color = 0xFFFFFFFF, .font_size = 13.5f, .font_weight = FontWeight::Bold }),
                text("Deploy into a secure sub-network with hardware-enforced encryption keys and strict ingress rules.", { .color = 0xFF94A3B8, .font_size = 12.0f }),
                text("Selected: VPC-us-east-1a • Subnet 10.0.4.0/24 • Auto-Scaling enabled", { .color = 0xFF10B981, .font_size = 12.5f, .font_weight = FontWeight::Bold })
            }
        });
    }

    // ── Step 3 Body: Deployment Strategy ──────────────────────────
    WidgetPtr buildStep3Body() {
        return column({
            .gap = StyleValue::point(8.0f),
            .children = {
                text("🚀 Edge CDN & Global Load Balancing", { .color = 0xFFFFFFFF, .font_size = 13.5f, .font_weight = FontWeight::Bold }),
                text("Distribute assets across 240+ global POPs for ultra-low latency.", { .color = 0xFF94A3B8, .font_size = 12.0f }),
                text("Selected: Multi-Region Active-Active • Anycast Routing", { .color = 0xFFF59E0B, .font_size = 12.5f, .font_weight = FontWeight::Bold })
            }
        });
    }

public:
    void initState() override {
        State::initState();
        panel_ctrl_ = std::make_shared<ExpansionPanelController>();
    }

    WidgetPtr build(BuildContext&) override {
        return container({
            .color = 0xFF0B1120,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(24.0f),
            .child = column({
                .align_items = Align::Center,
                .gap = StyleValue::point(16.0f),
                .children = {
                    column({
                        .align_items = Align::Center,
                        .gap = StyleValue::point(4.0f),
                        .children = {
                            text("Advanced ExpansionPanel Component Suite", { .color = 0xFFFFFFFF, .font_size = 22.0f, .font_weight = FontWeight::Bold }),
                            text("Multi-step configuration wizard with accordion transitions, custom headers, and footer action bars", { .color = 0xFF94A3B8, .font_size = 13.0f })
                        }
                    }),
                    row({
                        .justify_content = Justify::Center,
                        .gap = StyleValue::point(10.0f),
                        .children = {
                            button(text(is_radio_mode_ ? "Mode: Single Expand (Radio)" : "Mode: Multi Expand (Checkbox)"), [this] {
                                is_radio_mode_ = !is_radio_mode_;
                                hud_msg_ = "Switched to " + std::string(is_radio_mode_ ? "Single Expand Mode" : "Multi Expand Mode");
                                setState([] {});
                            }),
                            button(text("Expand Step 2"), [this] {
                                panel_ctrl_->expand(1);
                                hud_msg_ = "Programmatically expanded Step 2 (Network & Security)";
                                setState([] {});
                            }),
                            button(text("Collapse All"), [this] {
                                panel_ctrl_->collapseAll();
                                hud_msg_ = "Collapsed all configuration steps.";
                                setState([] {});
                            })
                        }
                    }),
                    container({
                        .color = 0xFF1E293B,
                        .border_radius = BorderRadius::circular(12.0f),
                        .border = Border(0xFF334155, 1.0f),
                        .width = StyleValue::point(780.0f),
                        .padding = StyleInsets::all(24.0f),
                        .child = ExpansionPanelList {
                            .panels = {
                                ExpansionPanelItem("step1", "1. Compute Instances", buildStep1Body()),
                                ExpansionPanelItem("step2", "2. Network & Security", buildStep2Body()),
                                ExpansionPanelItem("step3", "3. Deployment Strategy", buildStep3Body())
                            },
                            .controller = panel_ctrl_,
                            .is_radio_mode = is_radio_mode_,
                            .on_panel_toggled = [this](int idx, bool expanded) {
                                hud_msg_ = "User " + std::string(expanded ? "expanded" : "collapsed") + " panel index " + std::to_string(idx);
                                setState([] {});
                            }
                        }
                    }),
                    container({
                        .color = 0xFF1E293B,
                        .border_radius = BorderRadius::circular(6.0f),
                        .border = Border(0xFF334155, 1.0f),
                        .width = StyleValue::point(780.0f),
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
    }
};

class ExpansionPanelDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<ExpansionPanelDemoState>();
    }
    std::string_view typeName() const override { return "ExpansionPanelDemoApp"; }
};

int main() {
    std::cout << "====================================================\n";
    std::cout << "  ENKI Engine — Advanced ExpansionPanel Demo\n";
    std::cout << "====================================================\n";

    AppConfig config;
    config.title       = "Enki Engine — Advanced ExpansionPanel Demo";
    config.width       = 1180;
    config.height      = 720;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 60;
    config.clear_color = 0xFF0B1120;

    return runApp(std::make_shared<ExpansionPanelDemoApp>(), config);
}
