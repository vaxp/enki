/// @file main.cpp — GridTile standalone demo
#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/grid_tile.hpp"
#include "enki/widgets/scroll_view.hpp"
#include "enki/state/state.hpp"

using namespace enki;

static const Color kColors[] = {
    0xFF2563EB, 0xFF10B981, 0xFFF59E0B, 0xFFEF4444, 0xFF8B5CF6, 0xFF0EA5E9
};

static WidgetPtr swatch(int i) {
    return container({
        .color = kColors[i % 6],
        .width = StyleValue::percent(100.0f),
        .height = StyleValue::percent(100.0f),
    });
}

class GridTileDemoState : public State {
public:
    WidgetPtr build(BuildContext& ctx) override {
        auto hdr_wrap = container({
            .color = 0xFF0D1117,
            .width = StyleValue::percent(100.0f),
            .padding = StyleInsets::symmetric(16.0f, 20.0f),
            .child = text("GridTile Demo", {
                .color = 0xFFFFFFFF,
                .font_size = 24.0f,
                .font_weight = FontWeight::Bold,
            })
        });

        // 2×3 manual grid using Wrap
        std::vector<WidgetPtr> tiles;
        for (int i = 0; i < 6; ++i) {
            auto tile = GridTile {
                .child = swatch(i),
                .header = GridTileBar {
                    .trailing = text("★", { .color = 0xFFFCD34D, .font_size = 14.0f }),
                    .background_color = 0x99000000,
                },
                .footer = GridTileBar {
                    .title = text("Tile " + std::to_string(i + 1), { .color = 0xFFFFFFFF, .font_size = 13.0f }),
                    .subtitle = text("Header + Footer overlay", { .color = 0xFFCCCCCC, .font_size = 11.0f }),
                }
            };

            auto cell = container({
                .aspect_ratio = 1.0f,
                .flex_grow = 0.0f,
                .flex_shrink = 0.0f,
                .flex_basis = StyleValue::percent(50.0f),
                .child = tile
            });

            tiles.push_back(cell);
        }

        auto grid = wrap({
            .flex_shrink = 0.0f,
            .row_gap = StyleValue::point(4.0f),
            .column_gap = StyleValue::point(4.0f),
            .width = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(8.0f),
            .children = std::move(tiles),
        });

        auto scroll = scrollView(
            ScrollOptions{.direction = Axis::Vertical, .show_scrollbar = true},
            grid
        );

        return container({
            .color = 0xFF0D1117,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .child = column({
                .children = {
                    hdr_wrap,
                    container({
                        .flex_grow = 1.0f,
                        .flex_shrink = 1.0f,
                        .child = scroll
                    })
                }
            })
        });
    }
};

class GridTileDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override { return std::make_unique<GridTileDemoState>(); }
    std::string_view typeName() const override { return "GridTileDemoApp"; }
};

int main() {
    AppConfig cfg;
    cfg.title = "ENKI — GridTile Demo";
    cfg.width = 640; cfg.height = 700;
    cfg.resizable = true; cfg.vsync = false; cfg.target_fps = 0;
    cfg.show_performance_overlay = true;
    cfg.clear_color = 0xFF0D1117;
    return runApp(std::make_shared<GridTileDemoApp>(), cfg);
}
