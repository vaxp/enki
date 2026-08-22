/// @file main.cpp
/// @brief ENKI Advanced SplitView Interactive Showcase Demo.

#include "enki/app/app.hpp"
#include "enki/widgets/split_view.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/scroll_view.hpp"
#include "enki/state/state.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <memory>

using namespace enki;

class SplitViewDemoState : public State {
private:
    std::shared_ptr<SplitViewController> split_ctrl_;
    SplitOrientation current_orientation_ = SplitOrientation::Horizontal;
    std::string hud_msg_ = "Drag the divider handle (⋮) to resize panes in real-time, or double-click it to reset.";

    // ── 1. Left Pane: Project Explorer ────────────────────────────
    WidgetPtr buildExplorerPane() {
        return container({
            .color = 0xFF0F172A,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(24.0f),
            .child = column({
                .gap = StyleValue::point(8.0f),
                .children = {
                    text("📁 Workspace Project Explorer", { .color = 0xFFFFFFFF, .font_size = 13.5f, .font_weight = FontWeight::Bold }),
                    text("  ├─ 📂 src/widgets/split_view.cpp", { .color = 0xFF38BDF8, .font_size = 12.0f }),
                    text("  ├─ 📂 include/enki/widgets/split_view.hpp", { .color = 0xFF94A3B8, .font_size = 12.0f }),
                    text("  ├─ 📂 core/skia_compositor.cpp", { .color = 0xFF94A3B8, .font_size = 12.0f }),
                    text("  └─ 📄 CMakeLists.txt", { .color = 0xFF94A3B8, .font_size = 12.0f })
                }
            })
        });
    }

    // ── 2. Right Pane: Code Editor ────────────────────────────────
    WidgetPtr buildEditorPane() {
        return container({
            .color = 0xFF1E293B,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(24.0f),
            .child = column({
                .gap = StyleValue::point(16.0f),
                .children = {
                    text("</> Code Editor: split_view.cpp", { .color = 0xFFE2E8F0, .font_size = 13.5f, .font_weight = FontWeight::Bold }),
                    text("class SplitView : public StatefulWidget {\n"
                         "public:\n"
                         "    void handleDrag(float delta) {\n"
                         "        float new_split = current_split + (delta / total_width);\n"
                         "        new_split = std::clamp(new_split, min_ratio, max_ratio);\n"
                         "        if (new_split != current_split) {\n"
                         "            current_split = new_split;\n"
                         "            requestLayout();\n"
                         "        }\n"
                         "    }\n"
                         "};", { .color = 0xFF10B981, .font_size = 11.5f })
                }
            })
        });
    }

public:
    void initState() override {
        State::initState();
        split_ctrl_ = std::make_shared<SplitViewController>();
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
                            text("Advanced SplitView Component Suite", { .color = 0xFFFFFFFF, .font_size = 22.0f, .font_weight = FontWeight::Bold }),
                            text("Adjustable multi-pane layout with fluid drag re-sizing, double-click reset, and programmable ratio limits.", { .color = 0xFF94A3B8, .font_size = 13.0f })
                        }
                    }),
                    row({
                        .justify_content = Justify::Center,
                        .gap = StyleValue::point(10.0f),
                        .children = {
                            button(text("Toggle Orientation"), [this] {
                                current_orientation_ = (current_orientation_ == SplitOrientation::Horizontal)
                                    ? SplitOrientation::Vertical
                                    : SplitOrientation::Horizontal;
                                hud_msg_ = "Orientation changed to: " + std::string(current_orientation_ == SplitOrientation::Horizontal ? "Horizontal" : "Vertical");
                                setState([] {});
                            }),
                            button(text("Set Split to 20%"), [this] {
                                split_ctrl_->setRatio(0.2f);
                                hud_msg_ = "Programmatically set split ratio to 20% (0.2)";
                                setState([] {});
                            }),
                            button(text("Set Split to 80%"), [this] {
                                split_ctrl_->setRatio(0.8f);
                                hud_msg_ = "Programmatically set split ratio to 80% (0.8)";
                                setState([] {});
                            }),
                            button(text("Reset Default (50%)"), [this] {
                                split_ctrl_->reset();
                                hud_msg_ = "Reset split ratio to default (50%)";
                                setState([] {});
                            })
                        }
                    }),
                    container({
                        .color = 0xFF1E293B,
                        .border_radius = BorderRadius::circular(12.0f),
                        .border = Border(0xFF334155, 1.0f),
                        .clip_content = true,
                        .width = StyleValue::point(960.0f),
                        .height = StyleValue::point(420.0f),
                        .child = SplitView {
                            .leading = buildExplorerPane(),
                            .trailing = buildEditorPane(),
                            .options = {
                                .orientation = current_orientation_,
                                .initial_ratio = 0.5f,
                                .handle_thickness = 8.0f,
                                .handle_color = 0xFF38BDF8,
                                .background_color = 0xFF0F172A,
                                .on_split_changed = [this](float ratio) {
                                    hud_msg_ = "User dragging divider: Split ratio is now " + std::to_string(static_cast<int>(ratio * 100)) + "%";
                                    setState([] {});
                                }
                            },
                            .controller = split_ctrl_
                        }
                    }),
                    container({
                        .color = 0xFF1E293B,
                        .border_radius = BorderRadius::circular(6.0f),
                        .border = Border(0xFF334155, 1.0f),
                        .width = StyleValue::point(960.0f),
                        .padding = StyleInsets::symmetric(8.0f, 16.0f),
                        .child = text("💡 " + hud_msg_, { .color = 0xFF38BDF8, .font_size = 12.5f })
                    })
                }
            })
        });
    }
};

class SplitViewDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<SplitViewDemoState>();
    }
    std::string_view typeName() const override { return "SplitViewDemoApp"; }
};

int main() {
    std::cout << "====================================================\n";
    std::cout << "  ENKI Engine — Advanced SplitView Component Demo\n";
    std::cout << "====================================================\n";

    AppConfig config;
    config.title       = "Enki Engine — Advanced SplitView Component Demo";
    config.width       = 1180;
    config.height      = 720;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 60;
    config.clear_color = 0xFF0B1120;

    return runApp(std::make_shared<SplitViewDemoApp>(), config);
}
