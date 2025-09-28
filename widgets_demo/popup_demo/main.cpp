/// @file main.cpp
/// @brief ENKI Universal Popup Widget Interactive Showcase.
/// Demonstrates 12-direction placements, cursor tracking, dismiss controls, and custom Skia rendering.

#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/popup.hpp"
#include "enki/widgets/button.hpp"
#include "enki/state/state.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <memory>

using namespace enki;

// ── Custom Interactive Button (BarButton style as in desktop_shell_demo) ───

class DemoButton : public StatefulWidget {
public:
    std::string icon;
    std::string label;
    Color bg_color;
    Color hover_color;

    DemoButton(std::string icon, std::string label, Color bg = 0xFF1E293B, Color hov = 0xFF334155)
        : icon(std::move(icon)), label(std::move(label)), bg_color(bg), hover_color(hov) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "DemoButton"; }
};

class DemoButtonState : public State {
private:
    bool hovered_ = false;

public:
    WidgetPtr build(BuildContext& ctx) override {
        auto* btn = static_cast<const DemoButton*>(widget());

        std::vector<WidgetPtr> items;
        if (!btn->icon.empty()) {
            auto icon_t = text({
                .text = btn->icon,
                .color = 0xFFFFFFFF,
                .font_size = 13.0f,
                .font_weight = FontWeight::Bold,
            });
            items.push_back(icon_t);
        }
        if (!btn->label.empty()) {
            auto label_t = text({
                .text = btn->label,
                .color = 0xFFFFFFFF,
                .font_size = 13.0f,
                .font_weight = FontWeight::Bold,
            });
            items.push_back(label_t);
        }

        auto content_row = row({
            .align_items = Align::Center,
            .gap = StyleValue::point(8.0f),
            .children = std::move(items),
        });

        auto box = container(content_row);
        box->color(hovered_ ? btn->hover_color : btn->bg_color)
           .borderRadius(8.0f)
           .border(0xFF475569, 1.0f)
           .paddingSymmetric(10.0f, 16.0f);

        return gestureDetector({
            .child = box,
            .hit_test_behavior = HitTestBehavior::Opaque,
            .cursor_type       = SystemCursor::Pointer,
            .on_hover_enter = [this](const PointerEvent&) {
                setState([this] { hovered_ = true; });
            },
            .on_hover_exit = [this](const PointerEvent&) {
                setState([this] { hovered_ = false; });
            },
        });
    }
};

std::unique_ptr<State> DemoButton::createState() {
    return std::make_unique<DemoButtonState>();
}

// ── Main Popup Showcase Application ───────────────────────────────

class PopupDemoState : public State {
public:
    WidgetPtr build(BuildContext& ctx) override {
        // 1. TopCenter Default Popup
        auto top_btn = std::make_shared<DemoButton>("✨", "TopCenter Popup");

        auto top_popup = Popup {
            .child = top_btn,
            .builder = [](BuildContext&, std::shared_ptr<NativePopup>) {
                return column({
                    .children = {
                        text("Hello from TopCenter!", { .color = 0xFFF8FAFC, .font_size = 13.0f, .font_weight = FontWeight::Bold }),
                        text("This is the default popup behavior.", { .color = 0xFF94A3B8, .font_size = 11.0f })
                    }
                });
            },
            .options = {
                .placement = PopupPlacement::TopCenter
            }
        };

        // 2. BottomRight Placement Popup
        auto bottom_btn = std::make_shared<DemoButton>("🔽", "BottomRight Popup", 0xFF1E293B, 0xFF334155);

        auto bottom_popup = Popup {
            .child = bottom_btn,
            .builder = [](BuildContext&, std::shared_ptr<NativePopup>) {
                return column({
                    .children = {
                        text("BottomRight Popup Card", { .color = 0xFF10B981, .font_size = 13.0f, .font_weight = FontWeight::Bold }),
                        text("Aligned to bottom right corner with smooth drop shadows.", { .color = 0xFFCBD5E1, .font_size = 11.0f })
                    }
                });
            },
            .options = {
                .placement = PopupPlacement::BottomRight,
                .background_color = 0xFA1E293B,
                .border_color = 0xFF10B981,
                .content_size = Size{220.0f, 85.0f}
            }
        };

        // 3. Follow Cursor Popup (Hover Trigger)
        auto hover_btn = std::make_shared<DemoButton>("🎯", "Hover (FollowCursor)", 0xFF1E293B, 0xFF334155);

        auto cursor_popup = Popup {
            .child = hover_btn,
            .builder = [](BuildContext&, std::shared_ptr<NativePopup>) {
                return column({
                    .children = {
                        text("Tracking Cursor Point", { .color = 0xFFF59E0B, .font_size = 12.0f, .font_weight = FontWeight::Bold }),
                        text("Dynamically spawns near pointer position.", { .color = 0xFF94A3B8, .font_size = 11.0f })
                    }
                });
            },
            .options = {
                .placement = PopupPlacement::FollowCursor,
                .trigger = PopupTrigger::Hover,
                .background_color = 0xFA0F172A,
                .border_color = 0xFFF59E0B,
                .content_size = Size{200.0f, 65.0f}
            }
        };

        // 4. Center Screen Modal Popup
        auto modal_btn = std::make_shared<DemoButton>("🔲", "Center Screen Modal", 0xFF3B82F6, 0xFF60A5FA);

        auto modal_popup = Popup {
            .child = modal_btn,
            .builder = [](BuildContext& ctx, std::shared_ptr<NativePopup> popup_instance) {
                return column({
                    .gap = StyleValue::point(16.0f),
                    .children = {
                        text("Center Screen Modal", { .color = 0xFFFFFFFF, .font_size = 15.0f, .font_weight = FontWeight::Bold }),
                        text("This popup ignores the anchor position and is centered on the current window screen.", { .color = 0xFFE2E8F0, .font_size = 12.0f }),
                        row({
                            .justify_content = Justify::End,
                            .children = {
                                button(text("Close Modal"), [popup_instance]() {
                                    if (popup_instance) popup_instance->close();
                                })
                            }
                        })
                    }
                });
            },
            .options = {
                .placement = PopupPlacement::CenterScreen,
                .background_color = 0xEA0F172A,
                .border_color = 0xFF3B82F6,
                .padding = EdgeInsets::all(20.0f),
                .content_size = Size{300.0f, 150.0f}
            }
        };

        // Assemble Demo Layout
        return container({
            .color = 0xFF0B1120,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .child = column({
                .align_items = Align::Center,
                .children = {
                    container({
                        .padding = StyleInsets::only(0.0f, 40.0f, 40.0f, 40.0f),
                        .child = column({
                            .align_items = Align::Center,
                            .gap = StyleValue::point(8.0f),
                            .children = {
                                text("Universal Popup Subsystem", { .color = 0xFFFFFFFF, .font_size = 24.0f, .font_weight = FontWeight::Bold }),
                                text("Context menus, tooltips, and floating windows with multi-directional anchoring.", { .color = 0xFF94A3B8, .font_size = 14.0f })
                            }
                        })
                    }),
                    container({
                        .padding = StyleInsets::only(40.0f, 40.0f, 40.0f, 40.0f),
                        .child = wrap({
                            .justify_content = Justify::Center,
                            .align_items = Align::Center,
                            .gap = StyleValue::point(16.0f),
                            .children = {
                                top_popup,
                                bottom_popup,
                                cursor_popup,
                                modal_popup
                            }
                        })
                    })
                }
            })
        });
    }
};

class PopupDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<PopupDemoState>();
    }
    std::string_view typeName() const override { return "PopupDemoApp"; }
};

int main() {
    std::cout << "====================================================\n";
    std::cout << "  ENKI Engine — Universal Popup Widget Demo\n";
    std::cout << "====================================================\n";

    AppConfig config;
    config.title       = "Enki Engine — Popup Demo";
    config.width       = 950;
    config.height      = 450;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 0;
    config.show_performance_overlay = true;
    config.clear_color = 0xFF0F172A;

    return runApp(std::make_shared<PopupDemoApp>(), config);
}
