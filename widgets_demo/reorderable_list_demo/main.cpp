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
    // Rank badge
    auto rank_txt = text("#" + std::to_string(rank));
    rank_txt->fontSize(11.5f).bold().color(0xFF38BDF8);
    auto rank_box = container(rank_txt);
    rank_box->color(0x330284C7).borderRadius(6.0f).paddingSymmetric(3.0f, 7.0f);

    // Icon + title
    auto ic  = text(t.icon); ic->fontSize(14.0f);
    auto ttl = text(t.title); ttl->fontSize(13.0f).bold().color(0xFFFFFFFF);
    auto title_row = row(std::vector<WidgetPtr>{ic, ttl});
    title_row->gap(StyleValue::point(8.0f)).alignItems(Align::Center);

    // Tag badge
    auto tag_txt = text(t.tag); tag_txt->fontSize(10.5f).bold().color(t.tag_color);
    auto tag_box = container(tag_txt);
    tag_box->color(0x22000000 | (t.tag_color & 0x00FFFFFF)).borderRadius(4.0f).paddingSymmetric(2.0f, 6.0f);

    // Drag handle
    auto handle = reorderableDragHandle();

    auto left  = row(std::vector<WidgetPtr>{rank_box, title_row});
    left->gap(StyleValue::point(8.0f)).alignItems(Align::Center);

    auto right = row(std::vector<WidgetPtr>{tag_box, handle});
    right->gap(StyleValue::point(8.0f)).alignItems(Align::Center);

    auto card_row = row(std::vector<WidgetPtr>{left, right});
    card_row->justifyContent(Justify::SpaceBetween).alignItems(Align::Center).width(StyleValue::percent(100.0f));

    auto card = container(card_row);
    card->color(0xFF1E293B)
        .border(0xFF334155, 1.0f)
        .borderRadius(10.0f)
        .paddingSymmetric(14.0f, 16.0f)
        .width(StyleValue::point(card_w));
    return card;
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

        // ── Header ──────────────────────────────────────────────
        auto title = text("Advanced ReorderableList Suite");
        title->fontSize(22.0f).bold().color(0xFFFFFFFF);
        auto sub = text("600+ FPS Floating Drag — Drop Slot Indicators — Live Reorder Callbacks");
        sub->fontSize(12.5f).color(0xFF94A3B8);
        auto hdr = column(std::vector<WidgetPtr>{title, sub});
        hdr->alignItems(Align::Center).gap(StyleValue::point(5.0f));

        // ── Task cards ──────────────────────────────────────────
        std::vector<WidgetPtr> cards;
        for (int i = 0; i < (int)tasks_.size(); ++i) {
            cards.push_back(buildTaskCard(tasks_[i], i + 1, CARD_W));
        }

        // ── Reorderable list ─────────────────────────────────────
        ReorderableListOptions opts;
        opts.width       = LIST_W;
        opts.item_height = ITEM_H;
        opts.gap         = ITEM_GAP;
        opts.on_reorder  = [this](int old_idx, int new_idx) {
            auto item = tasks_[old_idx];
            tasks_.erase(tasks_.begin() + old_idx);
            tasks_.insert(tasks_.begin() + new_idx, item);
            hud_msg_ = "✨ Moved '" + item.title + "'  #" + std::to_string(old_idx + 1)
                     + "  ➔  #" + std::to_string(new_idx + 1);
            setState([]{});
        };

        auto rlist = std::make_shared<ReorderableList>(cards, opts);

        auto board_lbl = text("📋  Sprint Priority Backlog  (Drag Any Row to Reorder)");
        board_lbl->fontSize(14.5f).bold().color(0xFF38BDF8);

        auto board_col = column(std::vector<WidgetPtr>{board_lbl, rlist});
        board_col->gap(StyleValue::point(14.0f));

        auto board = container(board_col);
        board->color(0xFF0F172A)
              .border(0xFF334155, 1.0f)
              .borderRadius(14.0f)
              .paddingAll(24.0f)
              .width(LIST_W + 48.0f);

        // ── HUD ─────────────────────────────────────────────────
        auto hud_txt = text("💡  " + hud_msg_);
        hud_txt->fontSize(12.0f).color(0xFF38BDF8);
        auto hud = container(hud_txt);
        hud->color(0xFF1E293B)
            .border(0xFF334155, 1.0f)
            .borderRadius(6.0f)
            .paddingSymmetric(10.0f, 18.0f)
            .width(LIST_W + 48.0f);

        // ── Page ─────────────────────────────────────────────────
        auto page = column(std::vector<WidgetPtr>{hdr, board, hud});
        page->gap(StyleValue::point(20.0f)).alignItems(Align::Center);

        auto bg = container(page);
        bg->color(0xFF0B1120)
           .paddingAll(28.0f)
           .width(StyleValue::percent(100.0f))
           .height(StyleValue::percent(100.0f));

        return bg;
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
