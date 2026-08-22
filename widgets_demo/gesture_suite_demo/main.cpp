/// @file main.cpp
/// @brief ENKI Advanced Gestures & Interaction Interactive Showcase Demo.

#include "enki/app/app.hpp"
#include "enki/widgets/draggable.hpp"
#include "enki/widgets/dismissible.hpp"
#include "enki/widgets/focus.hpp"
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
#include <algorithm>

using namespace enki;

struct TaskCard {
    std::string id;
    std::string title;
    std::string tag;
    Color color;
};

class GestureSuiteDemoState : public State {
private:
    std::string hud_msg_ = "Drag tasks between Kanban columns. Swipe inbox rows left/right to dismiss. Click cards to test Focus.";

    // Kanban Tasks
    std::vector<TaskCard> todo_tasks_;
    std::vector<TaskCard> in_progress_tasks_;
    std::vector<TaskCard> done_tasks_;

    // Dismissible Inbox
    std::vector<std::string> inbox_items_;

    // Focus Nodes
    std::shared_ptr<FocusNode> focus_node_a_;
    std::shared_ptr<FocusNode> focus_node_b_;
    std::shared_ptr<FocusNode> focus_node_c_;
    std::string focused_card_name_ = "None";

    // ── Build Single Draggable Task Card ──────────────────────────
    WidgetPtr buildTaskCard(const TaskCard& t, const std::string& /*current_col*/) {
        auto tit = text(t.title, {
            .color = 0xFFFFFFFF,
            .font_size = 12.5f,
            .font_weight = FontWeight::Bold,
        });

        auto tag_t = text(t.tag, {
            .color = t.color,
            .font_size = 10.5f,
            .font_weight = FontWeight::Bold,
        });

        auto tag_box = container({
            .color = 0x3338BDF8,
            .border_radius = BorderRadius::circular(4.0f),
            .padding = StyleInsets::symmetric(2.0f, 6.0f),
            .child = tag_t,
        });

        auto row_c = row({
            .justify_content = Justify::SpaceBetween,
            .align_items = Align::Center,
            .children = {tit, tag_box},
        });

        auto drag_handle = text("⠿", {
            .color = 0xFF64748B,
            .font_size = 12.0f,
        });

        auto body_row = row({
            .align_items = Align::Center,
            .gap = StyleValue::point(8.0f),
            .children = {drag_handle, row_c},
        });

        auto c_box = container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(8.0f),
            .border = Border(0xFF334155, 1.0f),
            .width = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(10.0f),
            .child = body_row,
        });

        auto f_tit = text(t.title, {
            .color = 0xFFFFFFFF,
            .font_size = 12.5f,
            .font_weight = FontWeight::Bold,
        });

        auto f_tag = text(t.tag, {
            .color = t.color,
            .font_size = 10.5f,
            .font_weight = FontWeight::Bold,
        });

        auto f_tag_box = container({
            .color = 0x3338BDF8,
            .border_radius = BorderRadius::circular(4.0f),
            .padding = StyleInsets::symmetric(2.0f, 6.0f),
            .child = f_tag,
        });

        auto f_row = row({
            .justify_content = Justify::SpaceBetween,
            .align_items = Align::Center,
            .children = {f_tit, f_tag_box},
        });

        auto feedback_card = container({
            .color = 0xF01E293B,
            .border_radius = BorderRadius::circular(8.0f),
            .border = Border(0xFF38BDF8, 2.0f),
            .padding = StyleInsets::all(12.0f),
            .child = f_row,
        });

        return Draggable {
            .tag = "task_card",
            .preview_label = t.title,
            .data = t.id,
            .child = c_box,
            .feedback = feedback_card,
        };
    }

    // ── Build Kanban Column with DragTarget ────────────────────────
    WidgetPtr buildKanbanColumn(std::string title, std::string col_id,
                                const std::vector<TaskCard>& tasks, Color border_col) {
        auto col_title = text(title + " (" + std::to_string(tasks.size()) + ")", {
            .color = border_col,
            .font_size = 13.5f,
            .font_weight = FontWeight::Bold,
        });

        auto target = DragTarget {
            .accepted_tag = "task_card",
            .builder = [this, tasks, col_id, border_col](BuildContext&, bool is_hovered, const std::any&) -> WidgetPtr {
                std::vector<WidgetPtr> items;

                for (const auto& t : tasks) {
                    items.push_back(buildTaskCard(t, col_id));
                }

                if (tasks.empty()) {
                    auto empty_box = container({
                        .padding = StyleInsets::all(16.0f),
                        .child = text("Drop tasks here...", {
                            .color = 0xFF64748B,
                            .font_size = 11.5f,
                        }),
                    });
                    items.push_back(empty_box);
                }

                return container({
                    .color = is_hovered ? 0x3310B981 : 0xFF0F172A,
                    .border_radius = BorderRadius::circular(10.0f),
                    .border = Border(is_hovered ? 0xFF10B981 : 0xFF334155, is_hovered ? 2.0f : 1.0f),
                    .width = StyleValue::point(260.0f),
                    .padding = StyleInsets::all(12.0f),
                    .child = column({
                        .gap = StyleValue::point(8.0f),
                        .children = items,
                    }),
                });
            },
            .on_accept = [this, col_id](const std::any& data) {
                try {
                    std::string task_id = std::any_cast<std::string>(data);
                    moveTask(task_id, col_id);
                } catch (...) {}
            }
        };

        return column({
            .gap = StyleValue::point(8.0f),
            .children = {col_title, target},
        });
    }

    void moveTask(const std::string& task_id, const std::string& target_col) {
        TaskCard found_card;
        bool found = false;

        auto searchAndRemove = [&found_card, &found, task_id](std::vector<TaskCard>& list) {
            auto it = std::find_if(list.begin(), list.end(), [&task_id](const TaskCard& c) { return c.id == task_id; });
            if (it != list.end()) {
                found_card = *it;
                found = true;
                list.erase(it);
            }
        };

        searchAndRemove(todo_tasks_);
        searchAndRemove(in_progress_tasks_);
        searchAndRemove(done_tasks_);

        if (found) {
            if (target_col == "todo") todo_tasks_.push_back(found_card);
            else if (target_col == "progress") in_progress_tasks_.push_back(found_card);
            else if (target_col == "done") done_tasks_.push_back(found_card);

            hud_msg_ = "Moved [" + found_card.title + "] to column: " + target_col;
            setState([] {});
        }
    }

    // ── Build Dismissible Notification Item ───────────────────────
    WidgetPtr buildInboxItem(const std::string& msg, int idx) {
        auto c_box = container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(8.0f),
            .border = Border(0xFF334155, 1.0f),
            .width = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(12.0f),
            .child = row({
                .align_items = Align::Center,
                .gap = StyleValue::point(10.0f),
                .children = {
                    text("🔔", { .font_size = 14.0f }),
                    text(msg, { .color = 0xFFF1F5F9, .font_size = 13.0f }),
                }
            })
        });

        // Background left (Archive)
        auto arc_box = container({
            .color = 0x3310B981,
            .border_radius = BorderRadius::circular(8.0f),
            .padding = StyleInsets::symmetric(12.0f, 16.0f),
            .child = text("📥 Archive", {
                .color = 0xFF10B981,
                .font_size = 12.0f,
                .font_weight = FontWeight::Bold,
            })
        });

        // Background right (Delete)
        auto del_box = container({
            .color = 0x33EF4444,
            .border_radius = BorderRadius::circular(8.0f),
            .padding = StyleInsets::symmetric(12.0f, 16.0f),
            .child = text("🗑️ Delete", {
                .color = 0xFFEF4444,
                .font_size = 12.0f,
                .font_weight = FontWeight::Bold,
            })
        });

        return Dismissible {
            .child = c_box,
            .id = "inbox_" + std::to_string(idx),
            .background = arc_box,
            .secondary_background = del_box,
            .on_dismissed = [this, msg](DismissDirection dir) {
                hud_msg_ = (dir == DismissDirection::StartToEnd ? "Archived notification: " : "Deleted notification: ") + msg;
                setState([] {});
            },
        };
    }

public:
    void initState() override {
        State::initState();

        focus_node_a_ = std::make_shared<FocusNode>();
        focus_node_a_->debug_label = "Card A (Primary Server Node)";

        focus_node_b_ = std::make_shared<FocusNode>();
        focus_node_b_->debug_label = "Card B (Database Cluster)";

        focus_node_c_ = std::make_shared<FocusNode>();
        focus_node_c_->debug_label = "Card C (Edge Gateway)";

        todo_tasks_ = {
            {"t1", "Vulkan SPIR-V Shader Node", "GPU", 0xFF38BDF8},
            {"t2", "Memory Pool Allocator", "CORE", 0xFFF59E0B}
        };
        in_progress_tasks_ = {
            {"t3", "Wayland Buffer Protocol", "DISPLAY", 0xFF10B981}
        };
        done_tasks_ = {
            {"t4", "Anu Flexbox Layout Engine", "DONE", 0xFF8B5CF6}
        };

        inbox_items_ = {
            "Security audit notice for production cluster #4",
            "Automatic weekly backup completed successfully",
            "Continuous integration build passed for v1.0-rc3"
        };
    }

    WidgetPtr build(BuildContext&) override {
        // ── Main Page Header ──────────────────────────────────────────
        auto title_col = column({
            .align_items = Align::Center,
            .gap = StyleValue::point(6.0f),
            .children = {
                text("Advanced Gestures & Interaction Suite", { .color = 0xFFFFFFFF, .font_size = 22.0f, .font_weight = FontWeight::Bold }),
                text("Complete Category 9: Draggable (with floating cursor ghost), DragTarget, Dismissible, Focus, and FocusScope", { .color = 0xFF94A3B8, .font_size = 13.0f }),
            }
        });

        // ── 1. Kanban Drag & Drop Board (Draggable + DragTarget) ───────
        auto k1 = buildKanbanColumn("To Do", "todo", todo_tasks_, 0xFF38BDF8);
        auto k2 = buildKanbanColumn("In Progress", "progress", in_progress_tasks_, 0xFFF59E0B);
        auto k3 = buildKanbanColumn("Completed", "done", done_tasks_, 0xFF10B981);

        auto kanban_row = row({
            .justify_content = Justify::Center,
            .gap = StyleValue::point(16.0f),
            .children = {k1, k2, k3},
        });

        // ── 2. Swipeable Dismissible Inbox List ────────────────────────
        auto sec2_title = text("Swipe-to-Dismiss Notifications (Dismissible):", {
            .color = 0xFFCBD5E1,
            .font_size = 13.5f,
            .font_weight = FontWeight::Bold,
        });

        std::vector<WidgetPtr> inbox_rows = {sec2_title};
        for (size_t i = 0; i < inbox_items_.size(); ++i) {
            inbox_rows.push_back(buildInboxItem(inbox_items_[i], static_cast<int>(i)));
        }

        auto inbox_col = column({
            .gap = StyleValue::point(8.0f),
            .children = inbox_rows,
        });

        // ── 3. Focus & Keyboard Navigation Cards ──────────────────────
        auto makeFocusCard = [this](std::string name, std::shared_ptr<FocusNode> node) -> WidgetPtr {
            bool is_foc = node && node->has_focus;

            auto t = text(name + (is_foc ? " (FOCUSED ✓)" : " (Click to Focus)"), {
                .color = is_foc ? 0xFF38BDF8 : 0xFFCBD5E1,
                .font_size = 12.5f,
                .font_weight = FontWeight::Bold,
            });

            auto b = container({
                .color = is_foc ? 0xFF0C4A6E : 0xFF0F172A,
                .border_radius = BorderRadius::circular(8.0f),
                .border = Border(is_foc ? 0xFF38BDF8 : 0xFF334155, is_foc ? 2.0f : 1.0f),
                .padding = StyleInsets::symmetric(10.0f, 18.0f),
                .child = t,
            });

            return Focus {
                .child = b,
                .focus_node = node,
                .on_focus_change = [this, name](bool foc) {
                    if (foc) {
                        focused_card_name_ = name;
                        hud_msg_ = "Focus Changed: [" + name + "] is now actively FOCUSED!";
                    } else if (focused_card_name_ == name) {
                        focused_card_name_ = "None";
                    }
                    setState([] {});
                }
            };
        };

        auto fc1 = makeFocusCard("Server Node A", focus_node_a_);
        auto fc2 = makeFocusCard("Database Cluster B", focus_node_b_);
        auto fc3 = makeFocusCard("Edge Gateway C", focus_node_c_);

        auto focus_row = row({
            .justify_content = Justify::Center,
            .gap = StyleValue::point(14.0f),
            .children = {fc1, fc2, fc3},
        });

        // ── HUD / Status Box ──────────────────────────────────────────
        auto hud_box = container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(6.0f),
            .border = Border(0xFF334155, 1.0f),
            .padding = StyleInsets::symmetric(8.0f, 16.0f),
            .child = text("💡 " + hud_msg_, { .color = 0xFF38BDF8, .font_size = 12.5f }),
        });

        // ── Assemble Page Body ────────────────────────────────────────
        auto background_page = container({
            .color = 0xFF0B1120,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(24.0f),
            .child = column({
                .align_items = Align::Center,
                .gap = StyleValue::point(18.0f),
                .children = {title_col, kanban_row, inbox_col, focus_row, hud_box},
            }),
        });

        return DragOverlay {
            .child = background_page,
        };
    }
};

class GestureSuiteDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<GestureSuiteDemoState>();
    }
    std::string_view typeName() const override { return "GestureSuiteDemoApp"; }
};

int main() {
    std::cout << "====================================================\n";
    std::cout << "  ENKI Engine — Gestures & Interaction Suite Demo\n";
    std::cout << "====================================================\n";

    AppConfig config;
    config.title       = "Enki Engine — Gestures & Interaction Suite Demo";
    config.width       = 1180;
    config.height      = 720;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 0;
    config.show_performance_overlay = true;
    config.clear_color = 0xFF0B1120;

    return runApp(std::make_shared<GestureSuiteDemoApp>(), config);
}
