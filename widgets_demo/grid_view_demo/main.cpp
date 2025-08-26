/// @file main.cpp
/// @brief GridView + GridTile Demo

#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/grid_view.hpp"
#include "enki/widgets/grid_tile.hpp"
#include "enki/state/state.hpp"
#include <iostream>

using namespace enki;

// Fake "photo" tile using gradient-colored containers
static const Color kPalette[] = {
    0xFF2563EB, 0xFF10B981, 0xFFF59E0B, 0xFFEF4444,
    0xFF8B5CF6, 0xFF0EA5E9, 0xFFEC4899, 0xFF14B8A6,
    0xFFF97316, 0xFF6366F1,
};

static WidgetPtr fakePhoto(int i) {
    Color c = kPalette[i % 10];
    return container({
        .color = c,
        .align = Alignment::Center,
        .width = StyleValue::percent(100.0f),
        .height = StyleValue::percent(100.0f),
        .child = text(std::to_string(i + 1), {
            .color = Colors::White,
            .font_size = 24.0f,
            .font_weight = FontWeight::Bold,
        })
    });
}

class GridDemoState : public State {
    int tab_ = 2;  // 0=3-col fixed, 1=2-col fixed, 2=max-extent responsive

    WidgetPtr tabButton(const std::string& label, int idx) {
        bool active = (tab_ == idx);
        auto lbl = text(label, {
            .color = active ? 0xFFFFFFFF : 0xFF8B9BB4,
            .font_size = 13.0f,
        });
        ButtonProps opts;
        opts.normal_color  = active ? 0xFF2563EB : 0xFF161B22;
        opts.hover_color   = active ? 0xFF3B82F6 : 0xFF1E2937;
        opts.pressed_color = active ? 0xFF1D4ED8 : 0xFF0D1117;
        opts.border_radius = 6.0f;
        opts.padding       = EdgeInsets::symmetric(6.0f, 14.0f);
        opts.shadow_blur   = 0.0f;
        return button(lbl, [this, idx](){
            setState([this, idx]{ tab_ = idx; });
        }, opts);
    }

public:
    WidgetPtr build(BuildContext& ctx) override {
        auto header_title = text("GridView + GridTile Demo", {
            .color = 0xFFFFFFFF,
            .font_size = 24.0f,
            .font_weight = FontWeight::Bold,
        });

        std::string desc;
        if (tab_ == 0)      desc = "3 columns, aspect ratio 1:1";
        else if (tab_ == 1) desc = "2 columns, aspect ratio 4:3";
        else                desc = "Max-extent 160px — auto responsive columns";

        auto header_sub = text(desc, {
            .color = 0xFF8B9BB4,
            .font_size = 12.0f,
        });

        auto header = container({
            .color = 0xFF0D1117,
            .width = StyleValue::percent(100.0f),
            .padding = StyleInsets::symmetric(16.0f, 20.0f),
            .child = column({
                .gap = StyleValue::point(4.0f),
                .children = {header_title, header_sub}
            })
        });

        auto tab_wrap = container({
            .color = 0xFF161B22,
            .width = StyleValue::percent(100.0f),
            .padding = StyleInsets::symmetric(8.0f, 12.0f),
            .child = row({
                .gap = StyleValue::point(6.0f),
                .children = {
                    tabButton("3 Cols", 0),
                    tabButton("2 Cols", 1),
                    tabButton("Responsive", 2),
                }
            })
        });

        // Grid content
        WidgetPtr grid;
        if (tab_ == 0) {
            grid = GridView {
                .item_count = 30,
                .item_builder = [](int i) -> WidgetPtr {
                    return GridTile {
                        .child = fakePhoto(i),
                        .footer = GridTileBar {
                            .title = text("Item " + std::to_string(i + 1), { .color = 0xFFFFFFFF, .font_size = 12.0f }),
                        }
                    };
                },
                .fixed_delegate = SliverGridDelegateFixedCount(3, 4.0f, 4.0f, 1.0f),
                .list_padding = EdgeInsets::all(4.0f),
            };
        } else if (tab_ == 1) {
            grid = GridView {
                .item_count = 20,
                .item_builder = [](int i) -> WidgetPtr {
                    return GridTile {
                        .child = fakePhoto(i),
                        .footer = GridTileBar {
                            .title = text("Photo " + std::to_string(i + 1), { .color = 0xFFFFFFFF, .font_size = 13.0f }),
                            .subtitle = text("Subtitle " + std::to_string(i + 1), { .color = 0xFFB0C4D8, .font_size = 11.0f }),
                        }
                    };
                },
                .fixed_delegate = SliverGridDelegateFixedCount(2, 8.0f, 8.0f, 4.0f / 3.0f),
                .list_padding = EdgeInsets::all(8.0f),
            };
        } else {
            grid = GridView {
                .item_count = 25,
                .item_builder = [](int i) -> WidgetPtr {
                    return container({
                        .color = kPalette[i % 10],
                        .border_radius = BorderRadius::circular(8.0f),
                        .align = Alignment::Center,
                        .width = StyleValue::percent(100.0f),
                        .height = StyleValue::percent(100.0f),
                        .child = text(std::to_string(i + 1), {
                            .color = Colors::White,
                            .font_size = 28.0f,
                            .font_weight = FontWeight::Bold,
                        })
                    });
                },
                .max_delegate = SliverGridDelegateMaxExtent(160.0f, 8.0f, 8.0f, 1.0f),
                .use_max_extent_delegate = true,
                .list_padding = EdgeInsets::all(8.0f),
            };
        }

        return container({
            .color = 0xFF0D1117,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .child = column({
                .children = {
                    header,
                    tab_wrap,
                    container({
                        .flex_grow = 1.0f,
                        .flex_shrink = 1.0f,
                        .child = grid
                    })
                }
            })
        });
    }
};

class GridDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override { return std::make_unique<GridDemoState>(); }
    std::string_view typeName() const override { return "GridDemoApp"; }
};

int main() {
    AppConfig cfg;
    cfg.title      = "ENKI — GridView + GridTile Demo";
    cfg.width      = 640;
    cfg.height     = 700;
    cfg.resizable  = true;
    cfg.vsync      = false;
    cfg.target_fps = 0;
    cfg.show_performance_overlay = true;
    cfg.clear_color = 0xFF0D1117;
    return runApp(std::make_shared<GridDemoApp>(), cfg);
}
