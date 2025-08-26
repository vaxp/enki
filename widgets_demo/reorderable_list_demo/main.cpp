/// @file main.cpp
/// @brief ENKI Advanced ReorderableList Interactive Showcase Demo.

#include "enki/app/app.hpp"
#include "enki/widgets/reorderable_list.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/state/state.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <memory>

using namespace enki;

struct TaskItem {
    std::string icon;
    std::string title;
    std::string tag;
    Color       tag_color;
};

// ── Build a single task card widget ────────────────────────────────────────
static WidgetPtr buildTaskCard(const TaskItem& t, int rank, float card_w) {
    return container({
        .color = 0xFF1E293B,
        .border_radius = BorderRadius::circular(10.0f),
        .border = Border(0xFF334155, 1.0f),
        .width = StyleValue::point(card_w),
        .padding = StyleInsets::symmetric(14.0f, 16.0f),
        .child = row({
            .justify_content = Justify::SpaceBetween,
            .align_items = Align::Center,
            .width = StyleValue::percent(100.0f),
            .children = {
                row({
                    .align_items = Align::Center,
                    .gap = StyleValue::point(8.0f),
                    .children = {
                        container({
                            .color = 0x330284C7,
                            .border_radius = BorderRadius::circular(6.0f),
                            .padding = StyleInsets::symmetric(3.0f, 7.0f),
                            .child = text("#" + std::to_string(rank), { .color = 0xFF38BDF8, .font_size = 11.5f, .font_weight = FontWeight::Bold })
                        }),
                        row({
                            .align_items = Align::Center,
                            .gap = StyleValue::point(8.0f),
                            .children = {
                                text(t.icon, { .font_size = 14.0f }),
                                text(t.title, { .color = 0xFFFFFFFF, .font_size = 13.0f, .font_weight = FontWeight::Bold })
                            }
                        })
                    }
                }),
                row({
                    .align_items = Align::Center,
                    .gap = StyleValue::point(8.0f),
                    .children = {
                        container({
                            .color = 0x22000000 | (t.tag_color & 0x00FFFFFF),
                            .border_radius = BorderRadius::circular(4.0f),
                            .padding = StyleInsets::symmetric(2.0f, 6.0f),
                            .child = text(t.tag, { .color = t.tag_color, .font_size = 10.5f, .font_weight = FontWeight::Bold })
                        }),
                        ReorderableDragHandle {}
                    }
                })
            }
        })
    });
}

// ── Demo State ──────────────────────────────────────────────────────────────

class ReorderableListDemoState : public State {
    std::string hud_msg_ = "Drag and drop any task card up or down to reorder.";

    std::vector<TaskItem> tasks_ = {
        {"🚀", "Direct Skia Shader Rendering Pipeline",      "CRITICAL", 0xFFDC2626},
        {"⚡", "Wayland Buffer Sync & Presentation Engine",  "HIGH",     0xFFF59E0B},
        {"🎨", "Flexbox Layout & Anura Layout Nodes",        "CORE",     0xFF0284C7},
        {"⌨️", "XKB Keyboard & Pointer Input Manager",      "INPUT",    0xFF10B981},
        {"📦", "Widget Reconciliation Tree & State Cache",   "STABLE",   0xFF8B5CF6},
    };

public:
    WidgetPtr build(BuildContext&) override {
        const float LIST_W     = 620.0f;
        const float ITEM_H     = 52.0f;   // must match card's actual rendered height
        const float ITEM_GAP   = 10.0f;
        const float CARD_W     = LIST_W;

        std::vector<WidgetPtr> cards;
        for (int i = 0; i < (int)tasks_.size(); ++i) {
            cards.push_back(buildTaskCard(tasks_[i], i + 1, CARD_W));
        }

        return container({
            .color = 0xFF0B1120,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(28.0f),
            .child = column({
                .align_items = Align::Center,
                .gap = StyleValue::point(20.0f),
                .children = {
                    column({
                        .align_items = Align::Center,
                        .gap = StyleValue::point(5.0f),
                        .children = {
                            text("Advanced ReorderableList Suite", { .color = 0xFFFFFFFF, .font_size = 22.0f, .font_weight = FontWeight::Bold }),
                            text("600+ FPS Floating Drag — Drop Slot Indicators — Live Reorder Callbacks", { .color = 0xFF94A3B8, .font_size = 12.5f })
                        }
                    }),
                    container({
                        .color = 0xFF0F172A,
                        .border_radius = BorderRadius::circular(14.0f),
                        .border = Border(0xFF334155, 1.0f),
                        .width = StyleValue::point(LIST_W + 48.0f),
                        .padding = StyleInsets::all(24.0f),
                        .child = column({
                            .gap = StyleValue::point(14.0f),
                            .children = {
                                text("📋  Sprint Priority Backlog  (Drag Any Row to Reorder)", { .color = 0xFF38BDF8, .font_size = 14.5f, .font_weight = FontWeight::Bold }),
                                ReorderableList {
                                    .children = std::move(cards),
                                    .item_height = ITEM_H,
                                    .gap = ITEM_GAP,
                                    .width = LIST_W,
                                    .on_reorder = [this](int old_idx, int new_idx) {
                                        auto item = tasks_[old_idx];
                                        tasks_.erase(tasks_.begin() + old_idx);
                                        tasks_.insert(tasks_.begin() + new_idx, item);
                                        hud_msg_ = "✨ Moved '" + item.title + "'  #" + std::to_string(old_idx + 1)
                                                 + "  ➔  #" + std::to_string(new_idx + 1);
                                        setState([]{});
                                    }
                                }
                            }
                        })
                    }),
                    container({
                        .color = 0xFF1E293B,
                        .border_radius = BorderRadius::circular(6.0f),
                        .border = Border(0xFF334155, 1.0f),
                        .width = StyleValue::point(LIST_W + 48.0f),
                        .padding = StyleInsets::symmetric(10.0f, 18.0f),
                        .child = text("💡  " + hud_msg_, { .color = 0xFF38BDF8, .font_size = 12.0f })
                    })
                }
            })
        });
    }
};

class ReorderableListDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<ReorderableListDemoState>();
    }
    std::string_view typeName() const override { return "ReorderableListDemoApp"; }
};

int main() {
    std::cout << "====================================================\n";
    std::cout << "  ENKI Engine — Advanced ReorderableList Demo\n";
    std::cout << "====================================================\n";

    AppConfig cfg;
    cfg.title       = "Enki — Advanced ReorderableList Demo";
    cfg.width       = 900;
    cfg.height      = 700;
    cfg.resizable   = true;
    cfg.vsync       = false;
    cfg.target_fps  = 60;
    cfg.clear_color = 0xFF0B1120;

    return runApp(std::make_shared<ReorderableListDemoApp>(), cfg);
}
