/// @file main.cpp
/// @brief ENKI Advanced Chip & ChipGroup Interactive Showcase Demo.

#include "enki/app/app.hpp"
#include "enki/widgets/chip.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/state/state.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <memory>

using namespace enki;

class ChipDemoState : public State {
private:
    std::string hud_msg_ = "Interact with FilterChips to toggle tags, click 'X' on InputChips to delete tags, or pick a Choice priority.";

    // Active Filter tags
    std::vector<std::string> all_filters_ = {"C++20", "Wayland", "Skia Shaders", "Flexbox Layout", "Desktop GUI"};
    std::vector<std::string> active_filters_ = {"C++20", "Skia Shaders"};

    // Choice Priority
    std::string selected_priority_ = "High";

    // Deletable Input Tag List
    std::vector<std::string> recipients_ = {"alex@enki.dev", "team-core@deepmind.org", "sarah.ui@enki.io"};

public:
    WidgetPtr build(BuildContext&) override {
        // ── Main Page Header ──────────────────────────────────────────
        auto title = text("Advanced Chip & ChipGroup Suite");
        title->fontSize(22.0f).bold().color(0xFFFFFFFF);

        auto sub = text("Enterprise tags & interactive tokens (Category 2. Basic UI), FilterChips, ChoiceChips, Deletable InputChips, and Live Status badges");
        sub->fontSize(13.0f).color(0xFF94A3B8);

        std::vector<WidgetPtr> title_items = {title, sub};
        auto title_col = column(title_items);
        title_col->alignItems(Align::Center).gap(StyleValue::point(6.0f));

        // ── Card 1: Multi-Selection Filter Chips ──────────────────────
        auto f_title = text("🔍 Technology Filter Chips (Multi-Select & Auto-Wrap)");
        f_title->fontSize(14.5f).bold().color(0xFF38BDF8);

        std::vector<std::shared_ptr<Chip>> filter_chip_widgets;
        for (const auto& f : all_filters_) {
            bool is_sel = std::find(active_filters_.begin(), active_filters_.end(), f) != active_filters_.end();
            auto fc = filterChip(f, is_sel, [this, f](bool sel) {
                if (sel) {
                    active_filters_.push_back(f);
                } else {
                    active_filters_.erase(std::remove(active_filters_.begin(), active_filters_.end(), f), active_filters_.end());
                }
                hud_msg_ = "Filter toggled: " + f + (sel ? " (ENABLED)" : " (DISABLED)");
                setState([] {});
            });
            filter_chip_widgets.push_back(fc);
        }

        auto filters_group = chipGroup(filter_chip_widgets);

        std::vector<WidgetPtr> f_items = {f_title, filters_group};
        auto f_col = column(f_items);
        f_col->gap(StyleValue::point(12.0f));

        auto f_card = container(f_col);
        f_card->color(0xFF0F172A).border(0xFF334155, 1.0f).borderRadius(12.0f).paddingAll(18.0f).width(530.0f);

        // ── Card 2: Single-Choice Priority Chips ──────────────────────
        auto c_title = text("🎯 Priority Segment Choice Chips (Single-Select)");
        c_title->fontSize(14.5f).bold().color(0xFF10B981);

        std::vector<std::string> priorities = {"Low", "Medium", "High", "Critical"};
        std::vector<std::shared_ptr<Chip>> choice_chip_widgets;

        for (const auto& prio : priorities) {
            bool is_active = (selected_priority_ == prio);
            auto cc = choiceChip(prio, is_active, [this, prio](bool) {
                selected_priority_ = prio;
                hud_msg_ = "Priority changed to: " + prio;
                setState([] {});
            });
            if (is_active) {
                cc->options.selected_color = (prio == "Critical") ? 0xFFDC2626 : (prio == "High" ? 0xFFF59E0B : 0xFF0284C7);
            }
            choice_chip_widgets.push_back(cc);
        }

        auto choice_group = chipGroup(choice_chip_widgets);

        std::vector<WidgetPtr> c_items = {c_title, choice_group};
        auto c_col = column(c_items);
        c_col->gap(StyleValue::point(12.0f));

        auto c_card = container(c_col);
        c_card->color(0xFF0F172A).border(0xFF334155, 1.0f).borderRadius(12.0f).paddingAll(18.0f).width(530.0f);

        // ── Card 3: Deletable Input Chips (Tags & Entities) ───────────
        auto in_title = text("👤 Email Recipients & Input Tags (Deletable)");
        in_title->fontSize(14.5f).bold().color(0xFFF59E0B);

        std::vector<std::shared_ptr<Chip>> input_chip_widgets;
        for (const auto& rec : recipients_) {
            auto ic = inputChip(rec, [this, rec] {
                recipients_.erase(std::remove(recipients_.begin(), recipients_.end(), rec), recipients_.end());
                hud_msg_ = "Removed recipient tag: " + rec;
                setState([] {});
            }, "📧");
            input_chip_widgets.push_back(ic);
        }

        // Add Tag Button
        auto add_chip = actionChip("+ Add Tag", [this] {
            std::string new_tag = "user-" + std::to_string(recipients_.size() + 1) + "@enki.io";
            recipients_.push_back(new_tag);
            hud_msg_ = "Added new recipient tag: " + new_tag;
            setState([] {});
        }, "➕");

        input_chip_widgets.push_back(add_chip);

        auto input_group = chipGroup(input_chip_widgets);

        std::vector<WidgetPtr> in_items = {in_title, input_group};
        auto in_col = column(in_items);
        in_col->gap(StyleValue::point(12.0f));

        auto in_card = container(in_col);
        in_card->color(0xFF0F172A).border(0xFF334155, 1.0f).borderRadius(12.0f).paddingAll(18.0f).width(530.0f);

        // ── Card 4: Action Chips & Live Status Badges ─────────────────
        auto st_title = text("⚡ Action Chips & Live System Status");
        st_title->fontSize(14.5f).bold().color(0xFF8B5CF6);

        auto st_chip1 = statusChip("600+ FPS Active", 0xFF10B981, true);
        auto st_chip2 = statusChip("Wayland 1.22", 0xFF38BDF8, true);

        auto act_chip1 = actionChip("Run Benchmark", [this] {
            hud_msg_ = "🚀 Benchmark Action Triggered — Running at max hardware throughput!";
            setState([] {});
        }, "⚡");

        auto act_chip2 = actionChip("Save Config", [this] {
            hud_msg_ = "💾 Configuration successfully saved to disk.";
            setState([] {});
        }, "💾");

        std::vector<std::shared_ptr<Chip>> status_chips = {st_chip1, st_chip2, act_chip1, act_chip2};
        auto status_group = chipGroup(status_chips);

        std::vector<WidgetPtr> st_items = {st_title, status_group};
        auto st_col = column(st_items);
        st_col->gap(StyleValue::point(12.0f));

        auto st_card = container(st_col);
        st_card->color(0xFF0F172A).border(0xFF334155, 1.0f).borderRadius(12.0f).paddingAll(18.0f).width(530.0f);

        // ── Grid 2x2 of Cards ─────────────────────────────────────────
        std::vector<WidgetPtr> row1_cards = {f_card, c_card};
        auto row1 = row(row1_cards);
        row1->gap(StyleValue::point(20.0f)).justifyContent(Justify::Center);

        std::vector<WidgetPtr> row2_cards = {in_card, st_card};
        auto row2 = row(row2_cards);
        row2->gap(StyleValue::point(20.0f)).justifyContent(Justify::Center);

        // ── HUD / Status Box ──────────────────────────────────────────
        auto hud_txt = text("💡 " + hud_msg_);
        hud_txt->fontSize(12.5f).color(0xFF38BDF8);

        auto hud_row = row(std::vector<WidgetPtr>{hud_txt});
        auto hud_box = container(hud_row);
        hud_box->color(0xFF1E293B)
               .borderRadius(6.0f)
               .border(0xFF334155, 1.0f)
               .paddingSymmetric(8.0f, 16.0f)
               .width(1080.0f);

        // ── Assemble Page Body ────────────────────────────────────────
        std::vector<WidgetPtr> page_items = {title_col, row1, row2, hud_box};
        auto page_col = column(page_items);
        page_col->gap(StyleValue::point(18.0f)).alignItems(Align::Center);

        auto background_page = container(page_col);
        background_page->color(0xFF0B1120)
                       .paddingAll(24.0f)
                       .width(StyleValue::percent(100.0f))
                       .height(StyleValue::percent(100.0f));

        return background_page;
    }
};

class ChipDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<ChipDemoState>();
    }
    std::string_view typeName() const override { return "ChipDemoApp"; }
};

int main() {
    std::cout << "====================================================\n";
    std::cout << "  ENKI Engine — Advanced Chip Demo\n";
    std::cout << "====================================================\n";

    AppConfig config;
    config.title       = "Enki Engine — Advanced Chip Demo";
    config.width       = 1240;
    config.height      = 740;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 60;
    config.clear_color = 0xFF0B1120;

    return runApp(std::make_shared<ChipDemoApp>(), config);
}
