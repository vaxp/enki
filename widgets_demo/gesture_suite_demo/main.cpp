/// @file main.cpp
/// @brief ENKI Advanced Gestures & Interaction Suite Interactive Showcase Demo.

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
    std::vector<TaskCard> todo_tasks_;
    std::vector<TaskCard> in_progress_tasks_;
    std::vector<TaskCard> done_tasks_;

    std::vector<std::string> inbox_items_;
    std::string hud_msg_ = "Drag task cards between Kanban columns (floats with mouse cursor!), swipe notifications, or test Focus!";
    std::string focused_card_name_ = "None";

    std::shared_ptr<FocusNode> focus_node_a_;
    std::shared_ptr<FocusNode> focus_node_b_;
    std::shared_ptr<FocusNode> focus_node_c_;

    // ── Build Single Kanban Task Card ─────────────────────────────
    WidgetPtr buildTaskCard(const TaskCard& t, const std::string& current_col) {
        auto tit = text(t.title);
        tit->fontSize(12.5f).bold().color(0xFFFFFFFF);

        auto tag_txt = text(t.tag);
        tag_txt->fontSize(10.5f).bold().color(t.color);
        auto tag_box = container(tag_txt);
        tag_box->color(0x2238BDF8).borderRadius(4.0f).paddingSymmetric(2.0f, 6.0f);

        std::vector<WidgetPtr> h_items = {tit, tag_box};
        auto h_row = row(h_items);
        h_row->justifyContent(Justify::SpaceBetween).alignItems(Align::Center);

        // Move buttons inside card for easy 1-click or drag testing
        auto makeMoveBtn = [this, t, current_col](std::string label, std::string target_col) -> WidgetPtr {
            auto b_txt = text(label);
            b_txt->fontSize(10.5f).color(0xFF94A3B8);
            auto b_box = container(b_txt);
            b_box->color(0xFF0F172A).borderRadius(4.0f).paddingSymmetric(2.0f, 6.0f);

            auto gd = std::make_shared<GestureDetector>(b_box);
            gd->cursor_type = SystemCursor::Pointer;
            gd->on_tap_up = [this, t, target_col](const TapUpDetails&) {
                moveTask(t.id, target_col);
            };
            return gd;
        };

        std::vector<WidgetPtr> quick_btns;
        if (current_col != "todo") quick_btns.push_back(makeMoveBtn("◀ To Do", "todo"));
        if (current_col != "progress") quick_btns.push_back(makeMoveBtn("In Progress", "progress"));
        if (current_col != "done") quick_btns.push_back(makeMoveBtn("Done ▶", "done"));

        auto btns_row = row(quick_btns);
        btns_row->gap(StyleValue::point(6.0f)).justifyContent(Justify::End);

        std::vector<WidgetPtr> card_col_items = {h_row, btns_row};
        auto card_col = column(card_col_items);
        card_col->gap(StyleValue::point(8.0f));

        auto c_box = container(card_col);
        c_box->color(0xFF1E293B)
             .border(0xFF334155, 1.0f)
             .borderRadius(8.0f)
             .paddingAll(10.0f)
             .width(StyleValue::percent(100.0f))
             .shadow(BoxShadow(0x66000000, {0.0f, 2.0f}, 6.0f));

        // Floating feedback card that tracks the cursor in DragOverlay
        auto f_tit = text(t.title);
        f_tit->fontSize(12.5f).bold().color(0xFFFFFFFF);
        auto f_tag = text(t.tag);
        f_tag->fontSize(10.5f).bold().color(t.color);
        auto f_tag_box = container(f_tag);
        f_tag_box->color(0x3338BDF8).borderRadius(4.0f).paddingSymmetric(2.0f, 6.0f);

        std::vector<WidgetPtr> f_items = {f_tit, f_tag_box};
        auto f_row = row(f_items);
        f_row->justifyContent(Justify::SpaceBetween).alignItems(Align::Center);

        auto feedback_card = container(f_row);
        feedback_card->color(0xF01E293B)
                     .border(0xFF38BDF8, 2.0f)
                     .borderRadius(8.0f)
                     .paddingAll(12.0f);

        return draggable("task_card", t.id, c_box, feedback_card, nullptr, t.title);
    }

    // ── Build Kanban Column with DragTarget ────────────────────────
    WidgetPtr buildKanbanColumn(std::string title, std::string col_id,
                                const std::vector<TaskCard>& tasks, Color border_col) {
        auto col_title = text(title + " (" + std::to_string(tasks.size()) + ")");
        col_title->fontSize(13.5f).bold().color(border_col);

        auto target = dragTarget([this, tasks, col_id, border_col](BuildContext&, bool is_hovered, const std::any&) -> WidgetPtr {
            std::vector<WidgetPtr> items;

            for (const auto& t : tasks) {
                items.push_back(buildTaskCard(t, col_id));
            }

            if (tasks.empty()) {
                auto empty_txt = text("Drop tasks here...");
                empty_txt->fontSize(11.5f).color(0xFF64748B);
                auto empty_box = container(empty_txt);
                empty_box->paddingAll(16.0f);
                items.push_back(empty_box);
            }

            auto col_tasks = column(items);
            col_tasks->gap(StyleValue::point(8.0f)).width(StyleValue::percent(100.0f));

            auto box = container(col_tasks);
            box->color(is_hovered ? 0x3310B981 : 0xFF0F172A)
               .border(is_hovered ? 0xFF10B981 : 0xFF334155, is_hovered ? 2.0f : 1.0f)
               .borderRadius(10.0f)
               .paddingAll(12.0f)
               .width(260.0f);

            return box;
        }, [this, col_id](const std::any& data) {
            try {
                std::string task_id = std::any_cast<std::string>(data);
                moveTask(task_id, col_id);
            } catch (...) {}
        }, "task_card");

        std::vector<WidgetPtr> col_items = {col_title, target};
        auto full_col = column(col_items);
        full_col->gap(StyleValue::point(8.0f));

        return full_col;
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

            hud_msg_ = "Moved task [" + found_card.title + "] to " + target_col + " column!";
            setState([] {});
        }
    }

    // ── Build Swipeable Inbox Item ────────────────────────────────
    WidgetPtr buildInboxItem(const std::string& msg, int idx) {
        auto msg_txt = text("📩 " + msg);
        msg_txt->fontSize(12.5f).color(0xFFFFFFFF);

        auto hint_txt = text("Swipe ➔ to Archive, 🠔 to Delete");
        hint_txt->fontSize(11.0f).color(0xFF64748B);

        std::vector<WidgetPtr> txt_items = {msg_txt, hint_txt};
        auto txt_col = column(txt_items);
        txt_col->gap(StyleValue::point(2.0f));

        auto c_box = container(txt_col);
        c_box->color(0xFF1E293B)
             .border(0xFF334155, 1.0f)
             .borderRadius(8.0f)
             .paddingAll(12.0f)
             .width(StyleValue::percent(100.0f));

        // Background left (Archive)
        auto arc_txt = text("📥 Archive");
        arc_txt->fontSize(12.0f).bold().color(0xFF10B981);
        auto arc_box = container(arc_txt);
        arc_box->color(0x3310B981).borderRadius(8.0f).paddingSymmetric(12.0f, 16.0f);

        // Background right (Delete)
        auto del_txt = text("🗑️ Delete");
        del_txt->fontSize(12.0f).bold().color(0xFFEF4444);
        auto del_box = container(del_txt);
        del_box->color(0x33EF4444).borderRadius(8.0f).paddingSymmetric(12.0f, 16.0f);

        DismissibleProps d_opts;
        d_opts.background = arc_box;
        d_opts.secondary_background = del_box;
        d_opts.on_dismissed = [this, msg](DismissDirection dir) {
            hud_msg_ = (dir == DismissDirection::StartToEnd ? "Archived notification: " : "Deleted notification: ") + msg;
            setState([] {});
        };

        return dismissible("inbox_" + std::to_string(idx), c_box, d_opts);
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
        auto title = text("Advanced Gestures & Interaction Suite");
        title->fontSize(22.0f).bold().color(0xFFFFFFFF);

        auto sub = text("Complete Category 9: Draggable (with floating cursor ghost), DragTarget, Dismissible, Focus, and FocusScope");
        sub->fontSize(13.0f).color(0xFF94A3B8);

        std::vector<WidgetPtr> title_items = {title, sub};
        auto title_col = column(title_items);
        title_col->alignItems(Align::Center).gap(StyleValue::point(6.0f));

        // ── 1. Kanban Drag & Drop Board (Draggable + DragTarget) ───────
        auto k1 = buildKanbanColumn("To Do", "todo", todo_tasks_, 0xFF38BDF8);
        auto k2 = buildKanbanColumn("In Progress", "progress", in_progress_tasks_, 0xFFF59E0B);
        auto k3 = buildKanbanColumn("Completed", "done", done_tasks_, 0xFF10B981);

        std::vector<WidgetPtr> kanban_cols = {k1, k2, k3};
        auto kanban_row = row(kanban_cols);
        kanban_row->gap(StyleValue::point(16.0f)).justifyContent(Justify::Center);

        // ── 2. Swipeable Dismissible Inbox List ────────────────────────
        auto sec2_title = text("Swipe-to-Dismiss Notifications (Dismissible):");
        sec2_title->fontSize(13.5f).bold().color(0xFFCBD5E1);

        std::vector<WidgetPtr> inbox_rows = {sec2_title};
        for (size_t i = 0; i < inbox_items_.size(); ++i) {
            inbox_rows.push_back(buildInboxItem(inbox_items_[i], static_cast<int>(i)));
        }

        auto inbox_col = column(inbox_rows);
        inbox_col->gap(StyleValue::point(8.0f)).width(820.0f);

        // ── 3. Focus & Keyboard Navigation Cards ──────────────────────
        auto makeFocusCard = [this](std::string name, std::shared_ptr<FocusNode> node) -> WidgetPtr {
            bool is_foc = node && node->has_focus;

            auto t = text(name + (is_foc ? " (FOCUSED ✓)" : " (Click to Focus)"));
            t->fontSize(12.5f).bold().color(is_foc ? 0xFF38BDF8 : 0xFFCBD5E1);

            auto b = container(t);
            b->color(is_foc ? 0xFF0C4A6E : 0xFF0F172A)
             .border(is_foc ? 0xFF38BDF8 : 0xFF334155, is_foc ? 2.0f : 1.0f)
             .borderRadius(8.0f)
             .paddingSymmetric(10.0f, 18.0f);

            auto f_widget = focus(b, node);
            f_widget->on_focus_change = [this, name](bool foc) {
                if (foc) {
                    focused_card_name_ = name;
                    hud_msg_ = "Focus Changed: [" + name + "] is now actively FOCUSED!";
                } else if (focused_card_name_ == name) {
                    focused_card_name_ = "None";
                }
                setState([] {});
            };
            return f_widget;
        };

        auto fc1 = makeFocusCard("Server Node A", focus_node_a_);
        auto fc2 = makeFocusCard("Database Cluster B", focus_node_b_);
        auto fc3 = makeFocusCard("Edge Gateway C", focus_node_c_);

        std::vector<WidgetPtr> focus_cards = {fc1, fc2, fc3};
        auto focus_row = row(focus_cards);
        focus_row->gap(StyleValue::point(14.0f)).justifyContent(Justify::Center);

        // ── HUD / Status Box ──────────────────────────────────────────
        auto hud_txt = text("💡 " + hud_msg_);
        hud_txt->fontSize(12.5f).color(0xFF38BDF8);

        auto hud_row = row(std::vector<WidgetPtr>{hud_txt});
        auto hud_box = container(hud_row);
        hud_box->color(0xFF1E293B)
               .borderRadius(6.0f)
               .border(0xFF334155, 1.0f)
               .paddingSymmetric(8.0f, 16.0f)
               .width(820.0f);

        // ── Assemble Page Body ────────────────────────────────────────
        std::vector<WidgetPtr> page_items = {title_col, kanban_row, inbox_col, focus_row, hud_box};
        auto page_col = column(page_items);
        page_col->gap(StyleValue::point(18.0f)).alignItems(Align::Center);

        auto background_page = container(page_col);
        background_page->color(0xFF0B1120)
                       .paddingAll(24.0f)
                       .width(StyleValue::percent(100.0f))
                       .height(StyleValue::percent(100.0f));

        return dragOverlay(background_page);
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
