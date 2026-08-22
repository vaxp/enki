/// @file main.cpp
/// @brief ListTile Widget Demo — showcases all ListTile variants.

#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/scroll_view.hpp"
#include "enki/widgets/divider.hpp"
#include "enki/widgets/list_tile.hpp"
#include "enki/state/state.hpp"
#include <iostream>

using namespace enki;

class ListTileDemoState : public State {
    int selected_index_ = -1;

    static WidgetPtr makeIcon(Color color, char letter) {
        return container({
            .color = color,
            .border_radius = BorderRadius::circular(18.0f),
            .align = Alignment::Center,
            .width = StyleValue::point(36.0f),
            .height = StyleValue::point(36.0f),
            .child = text(std::string(1, letter), {
                .color = Colors::White,
                .font_size = 16.0f,
                .font_weight = FontWeight::Bold,
            })
        });
    }

    static WidgetPtr chevron() {
        return text("›", {
            .color = 0xFF606060,
            .font_size = 20.0f,
        });
    }

    static WidgetPtr badge(const std::string& label, Color bg) {
        return container({
            .color = bg,
            .border_radius = BorderRadius::circular(10.0f),
            .padding = StyleInsets::symmetric(2.0f, 7.0f),
            .child = text(label, {
                .color = Colors::White,
                .font_size = 11.0f,
                .font_weight = FontWeight::Bold,
            })
        });
    }

public:
    WidgetPtr build(BuildContext& ctx) override {
        // ── Section label helper ───────────────────────────────
        auto section = [](const std::string& s) -> WidgetPtr {
            return container({
                .width = StyleValue::percent(100.0f),
                .padding = StyleInsets::symmetric(8.0f, 16.0f),
                .child = text(s, {
                    .color = 0xFF8B9BB4,
                    .font_size = 11.0f,
                    .font_weight = FontWeight::Bold,
                    .letter_spacing = 1.2f,
                })
            });
        };

        // ── Divider helper ─────────────────────────────────────
        auto divline = []() -> WidgetPtr {
            return container({
                .color = 0x1AFFFFFF,
                .height = StyleValue::point(1.0f),
                .margin = StyleInsets::only(0, 0, 0, 72.0f),
            });
        };

        std::vector<WidgetPtr> items;

        // ── Section 1: Basic tiles ─────────────────────────────
        items.push_back(section("BASIC"));

        items.push_back(ListTile {
            .leading = makeIcon(0xFF2563EB, 'A'),
            .title = text("Simple Title", { .color = 0xFFE2E8F0, .font_size = 15.0f }),
            .on_tap = [this](){ setState([this]{ selected_index_ = 0; }); }
        });
        items.push_back(divline());

        items.push_back(ListTile {
            .leading = makeIcon(0xFF10B981, 'B'),
            .title = text("With Subtitle", { .color = 0xFFE2E8F0, .font_size = 15.0f }),
            .subtitle = text("Secondary description text", { .color = 0xFF8B9BB4, .font_size = 13.0f }),
            .trailing = chevron(),
            .on_tap = [this](){ setState([this]{ selected_index_ = 1; }); }
        });
        items.push_back(divline());

        items.push_back(ListTile {
            .leading = makeIcon(0xFFF59E0B, 'C'),
            .title = text("With Badge Trailing", { .color = 0xFFE2E8F0, .font_size = 15.0f }),
            .subtitle = text("Notifications: 5", { .color = 0xFF8B9BB4, .font_size = 13.0f }),
            .trailing = badge("5", 0xFFEF4444),
            .on_tap = [this](){ setState([this]{ selected_index_ = 2; }); }
        });

        // ── Section 2: Selected ────────────────────────────────
        items.push_back(section("SELECTION"));

        for (int i = 0; i < 3; ++i) {
            items.push_back(ListTile {
                .leading = makeIcon(0xFF8B5CF6, '0' + i + 1),
                .title = text("Selectable Item " + std::to_string(i + 1), { .color = 0xFFE2E8F0, .font_size = 15.0f }),
                .selected = (selected_index_ == i + 10),
                .on_tap = [this, i](){ setState([this, i]{ selected_index_ = i + 10; }); },
                .selected_color = 0x1A8B5CF6,
            });
            items.push_back(divline());
        }

        // ── Section 3: Dense / Compact ─────────────────────────
        items.push_back(section("COMPACT (DENSE)"));

        for (int i = 1; i <= 4; ++i) {
            items.push_back(ListTile {
                .leading = makeIcon(0xFF0EA5E9, '0' + i),
                .title = text("Dense Item " + std::to_string(i), { .color = 0xFFE2E8F0, .font_size = 13.0f }),
                .on_tap = [i](){ std::cout << "Dense " << i << "\n"; },
                .visual_density = VisualDensity::Compact,
            });
        }

        // ── Section 4: Disabled ────────────────────────────────
        items.push_back(section("DISABLED"));

        items.push_back(ListTile {
            .leading = makeIcon(0xFF475569, 'D'),
            .title = text("Disabled Tile", { .color = 0xFF475569, .font_size = 15.0f }),
            .subtitle = text("This tile cannot be interacted with", { .color = 0xFF334155, .font_size = 13.0f }),
            .enabled = false,
        });

        // ── Layout ────────────────────────────────────────────
        auto content_col = column({
            .flex_shrink = 0.0f,
            .width = StyleValue::percent(100.0f),
            .children = std::move(items),
        });

        auto scroll = scrollView(
            ScrollOptions{.direction = Axis::Vertical, .show_scrollbar = true},
            content_col
        );

        auto header = container({
            .color = 0xFF0D1117,
            .width = StyleValue::percent(100.0f),
            .padding = StyleInsets::symmetric(16.0f, 20.0f),
            .child = column({
                .gap = StyleValue::point(4.0f),
                .children = {
                    text("ListTile Demo", { .color = 0xFFFFFFFF, .font_size = 26.0f, .font_weight = FontWeight::Bold }),
                    text("leading · title · subtitle · trailing · hover · ripple · selection · dense · disabled", {
                        .color = 0xFF8B9BB4,
                        .font_size = 12.0f,
                    }),
                }
            })
        });

        // Status bar
        auto status_bar = container({
            .color = 0xFF161B22,
            .width = StyleValue::percent(100.0f),
            .padding = StyleInsets::symmetric(8.0f, 20.0f),
            .child = text(selected_index_ >= 0
                          ? "Selected index: " + std::to_string(selected_index_)
                          : "Tap any tile to select", {
                .color = 0xFF8B9BB4,
                .font_size = 13.0f,
            })
        });

        return container({
            .color = 0xFF0D1117,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .child = column({
                .height = StyleValue::percent(100.0f),
                .children = {
                    header,
                    status_bar,
                    flexItem(
                        { .flex_grow = 1.0f, .flex_shrink = 1.0f },
                        scroll
                    )
                }
            })
        });
    }
};

class ListTileDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<ListTileDemoState>();
    }
    std::string_view typeName() const override { return "ListTileDemoApp"; }
};

int main() {
    std::cout << "╔══════════════════════════════════════╗\n";
    std::cout << "║  ENKI Engine — ListTile Demo          ║\n";
    std::cout << "╚══════════════════════════════════════╝\n";

    AppConfig cfg;
    cfg.title      = "ENKI — ListTile Demo";
    cfg.width      = 480;
    cfg.height     = 700;
    cfg.resizable  = true;
    cfg.vsync      = false;
    cfg.target_fps = 0;
    cfg.show_performance_overlay = true;
    cfg.clear_color = 0xFF0D1117;

    return runApp(std::make_shared<ListTileDemoApp>(), cfg);
}
