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
        auto title = text("Advanced Chip & ChipGroup Suite", {
            .color = 0xFFFFFFFF,
            .font_size = 22.0f,
            .font_weight = FontWeight::Bold,
        });

        auto sub = text("Enterprise tags & interactive tokens (Category 2. Basic UI), FilterChips, ChoiceChips, Deletable InputChips, and Live Status badges", {
            .color = 0xFF94A3B8,
            .font_size = 13.0f,
        });

        auto title_col = column({
            .align_items = Align::Center,
            .gap = StyleValue::point(6.0f),
            .children = {title, sub},
        });

        // ── Card 1: Multi-Selection Filter Chips ──────────────────────
        auto f_title = text("🔍 Technology Filter Chips (Multi-Select & Auto-Wrap)", {
            .color = 0xFF38BDF8,
            .font_size = 14.5f,
            .font_weight = FontWeight::Bold,
        });

        std::vector<WidgetPtr> filter_chip_widgets;
        for (const auto& f : all_filters_) {
            bool is_sel = std::find(active_filters_.begin(), active_filters_.end(), f) != active_filters_.end();
            filter_chip_widgets.push_back(Chip {
                .type = ChipType::Filter,
                .label = f,
                .selected = is_sel,
                .on_selected = [this, f](bool sel) {
                    if (sel) {
                        active_filters_.push_back(f);
                    } else {
                        active_filters_.erase(std::remove(active_filters_.begin(), active_filters_.end(), f), active_filters_.end());
                    }
                    hud_msg_ = "Filter toggled: " + f + (sel ? " (ENABLED)" : " (DISABLED)");
                    setState([] {});
                },
            });
        }

        auto filters_group = ChipGroup {
            .chips = std::move(filter_chip_widgets),
        };

        auto f_card = container({
            .color = 0xFF0F172A,
            .border_radius = BorderRadius::circular(12.0f),
            .border = Border(0xFF334155, 1.0f),
            .width = StyleValue::point(530.0f),
            .padding = StyleInsets::all(18.0f),
            .child = column({
                .gap = StyleValue::point(12.0f),
                .children = {f_title, filters_group},
            }),
        });

        // ── Card 2: Single-Choice Priority Chips ──────────────────────
        auto c_title = text("🎯 Priority Segment Choice Chips (Single-Select)", {
            .color = 0xFF10B981,
            .font_size = 14.5f,
            .font_weight = FontWeight::Bold,
        });

        std::vector<std::string> priorities = {"Low", "Medium", "High", "Critical"};
        std::vector<WidgetPtr> choice_chip_widgets;

        for (const auto& prio : priorities) {
            bool is_active = (selected_priority_ == prio);
            Color sel_col = (prio == "Critical") ? 0xFFDC2626 : (prio == "High" ? 0xFFF59E0B : 0xFF0284C7);
            choice_chip_widgets.push_back(Chip {
                .type = ChipType::Choice,
                .label = prio,
                .selected = is_active,
                .selected_color = sel_col,
                .on_selected = [this, prio](bool) {
                    selected_priority_ = prio;
                    hud_msg_ = "Priority changed to: " + prio;
                    setState([] {});
                },
            });
        }

        auto choice_group = ChipGroup {
            .chips = std::move(choice_chip_widgets),
        };

        auto c_card = container({
            .color = 0xFF0F172A,
            .border_radius = BorderRadius::circular(12.0f),
            .border = Border(0xFF334155, 1.0f),
            .width = StyleValue::point(530.0f),
            .padding = StyleInsets::all(18.0f),
            .child = column({
                .gap = StyleValue::point(12.0f),
                .children = {c_title, choice_group},
            }),
        });

        // ── Card 3: Deletable Input Chips (Tags & Entities) ───────────
        auto in_title = text("👤 Email Recipients & Input Tags (Deletable)", {
            .color = 0xFFF59E0B,
            .font_size = 14.5f,
            .font_weight = FontWeight::Bold,
        });

        std::vector<WidgetPtr> input_chip_widgets;
        for (const auto& rec : recipients_) {
            input_chip_widgets.push_back(Chip {
                .type = ChipType::Input,
                .label = rec,
                .avatar_icon = "📧",
                .deletable = true,
                .on_deleted = [this, rec] {
                    recipients_.erase(std::remove(recipients_.begin(), recipients_.end(), rec), recipients_.end());
                    hud_msg_ = "Removed recipient tag: " + rec;
                    setState([] {});
                },
            });
        }

        // Add Tag Button
        input_chip_widgets.push_back(Chip {
            .type = ChipType::Action,
            .label = "+ Add Tag",
            .avatar_icon = "➕",
            .on_tap = [this] {
                std::string new_tag = "user-" + std::to_string(recipients_.size() + 1) + "@enki.io";
                recipients_.push_back(new_tag);
                hud_msg_ = "Added new recipient tag: " + new_tag;
                setState([] {});
            },
        });

        auto input_group = ChipGroup {
            .chips = std::move(input_chip_widgets),
        };

        auto in_card = container({
            .color = 0xFF0F172A,
            .border_radius = BorderRadius::circular(12.0f),
            .border = Border(0xFF334155, 1.0f),
            .width = StyleValue::point(530.0f),
            .padding = StyleInsets::all(18.0f),
            .child = column({
                .gap = StyleValue::point(12.0f),
                .children = {in_title, input_group},
            }),
        });

        // ── Card 4: Action Chips & Live Status Badges ─────────────────
        auto st_title = text("⚡ Action Chips & Live System Status", {
            .color = 0xFF8B5CF6,
            .font_size = 14.5f,
            .font_weight = FontWeight::Bold,
        });

        auto st_chip1 = Chip {
            .type = ChipType::Status,
            .label = "600+ FPS Active",
            .pulsing_dot = true,
            .status_color = 0xFF10B981,
        };

        auto st_chip2 = Chip {
            .type = ChipType::Status,
            .label = "Wayland 1.22",
            .pulsing_dot = true,
            .status_color = 0xFF38BDF8,
        };

        auto act_chip1 = Chip {
            .type = ChipType::Action,
            .label = "Run Benchmark",
            .avatar_icon = "⚡",
            .on_tap = [this] {
                hud_msg_ = "🚀 Benchmark Action Triggered — Running at max hardware throughput!";
                setState([] {});
            },
        };

        auto act_chip2 = Chip {
            .type = ChipType::Action,
            .label = "Save Config",
            .avatar_icon = "💾",
            .on_tap = [this] {
                hud_msg_ = "💾 Configuration successfully saved to disk.";
                setState([] {});
            },
        };

        std::vector<WidgetPtr> status_chips = {st_chip1, st_chip2, act_chip1, act_chip2};
        auto status_group = ChipGroup {
            .chips = std::move(status_chips),
        };

        auto st_card = container({
            .color = 0xFF0F172A,
            .border_radius = BorderRadius::circular(12.0f),
            .border = Border(0xFF334155, 1.0f),
            .width = StyleValue::point(530.0f),
            .padding = StyleInsets::all(18.0f),
            .child = column({
                .gap = StyleValue::point(12.0f),
                .children = {st_title, status_group},
            }),
        });

        // ── Grid 2x2 of Cards ─────────────────────────────────────────
        auto row1 = row({
            .justify_content = Justify::Center,
            .gap = StyleValue::point(20.0f),
            .children = {f_card, c_card},
        });

        auto row2 = row({
            .justify_content = Justify::Center,
            .gap = StyleValue::point(20.0f),
            .children = {in_card, st_card},
        });

        // ── HUD / Status Box ──────────────────────────────────────────
        auto hud_box = container({
            .color = 0xFF1E293B,
            .border_radius = BorderRadius::circular(6.0f),
            .border = Border(0xFF334155, 1.0f),
            .width = StyleValue::point(1080.0f),
            .padding = StyleInsets::symmetric(8.0f, 16.0f),
            .child = row({
                .children = {
                    text("💡 " + hud_msg_, {
                        .color = 0xFF38BDF8,
                        .font_size = 12.5f,
                    }),
                },
            }),
        });

        // ── Assemble Page Body ────────────────────────────────────────
        return container({
            .color = 0xFF0B1120,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(24.0f),
            .child = column({
                .align_items = Align::Center,
                .gap = StyleValue::point(18.0f),
                .children = {title_col, row1, row2, hud_box},
            }),
        });
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
