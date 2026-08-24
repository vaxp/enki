/// @file main.cpp — Table standalone demo (basic Table widget only)
#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/table.hpp"
#include "enki/widgets/scroll_view.hpp"
#include "enki/state/state.hpp"

using namespace enki;

class TableDemoState : public State {
    static WidgetPtr cell(const std::string& s, bool hdr = false) {
        auto t = text({
            .text = s,
            .color = hdr ? 0xFFB0C4D8 : 0xFFE2E8F0,
            .font_size = hdr ? 12.0f : 13.0f,
            .font_weight = hdr ? FontWeight::Bold : FontWeight::Normal,
        });
        return container({
            .padding = StyleInsets::symmetric(10.0f, 12.0f),
            .child = t,
        });
    }
public:
    WidgetPtr build(BuildContext& ctx) override {
        std::vector<TableRow> rows;

        // Header
        rows.push_back(TableRow({
            cell("Rank", true), cell("Language", true), cell("Usage %", true),
            cell("Trend", true), cell("Type", true)
        }, BoxDecoration(0xFF1E2937)));

        const struct { const char* rank; const char* lang; const char* usage; const char* trend; const char* type; } kData[] = {
            {"1",  "Python",     "30.3%", "▲ +1.2", "Interpreted"},
            {"2",  "JavaScript", "28.1%", "▲ +0.5", "Interpreted"},
            {"3",  "Java",       "15.8%", "▼ -0.3", "Compiled"},
            {"4",  "C++",        "11.2%", "▲ +0.2", "Compiled"},
            {"5",  "TypeScript",  "9.9%", "▲ +1.8", "Transpiled"},
            {"6",  "Go",          "7.4%", "▲ +0.9", "Compiled"},
            {"7",  "Rust",        "5.2%", "▲ +1.1", "Compiled"},
            {"8",  "Kotlin",      "4.8%", "▲ +0.4", "Compiled"},
            {"9",  "Swift",       "3.9%", "— 0.0",  "Compiled"},
            {"10", "C#",          "8.7%", "▼ -0.1", "Compiled"},
        };

        for (int i = 0; i < 10; ++i) {
            const auto& d = kData[i];
            Color trend_color = std::string_view(d.trend).starts_with("▲") ? 0xFF10B981 :
                                std::string_view(d.trend).starts_with("▼") ? 0xFFEF4444 : 0xFF8B9BB4;
            auto trend_t = text({
                .text = d.trend,
                .color = trend_color,
                .font_size = 13.0f,
            });
            auto trend_wrap = container({
                .padding = StyleInsets::symmetric(10.0f, 12.0f),
                .child = trend_t,
            });

            rows.push_back(TableRow({
                cell(d.rank), cell(d.lang), cell(d.usage), trend_wrap, cell(d.type)
            }));
        }

        auto t_widget = Table {
            .rows = std::move(rows),
            .column_widths = {
                {0, FixedColumnWidth(40.0f)},
                {1, FlexColumnWidth(2.0f)},
                {2, FixedColumnWidth(70.0f)},
                {3, FixedColumnWidth(80.0f)},
                {4, FlexColumnWidth(1.2f)},
            },
            .border = TableBorder::symmetric(0x1AFFFFFF, 1.0f),
            .default_vertical_alignment = TableCellVerticalAlignment::Middle,
        };

        auto scroll = scrollView(
            ScrollOptions{.direction=Axis::Vertical,.show_scrollbar=true},
            t_widget
        );
        auto scroll_flex = flexItem({.flex_grow = 1.0f, .flex_shrink = 1.0f, .child = scroll});

        auto title = text({
            .text = "Table Demo — Programming Languages",
            .color = 0xFFFFFFFF,
            .font_size = 22.0f,
            .font_weight = FontWeight::Bold,
        });
        auto sub = text({
            .text = "FixedColumnWidth · FlexColumnWidth · TableBorder · per-row decoration",
            .color = 0xFF8B9BB4,
            .font_size = 12.0f,
        });
        auto hdr_col = column({
            .gap = StyleValue::point(4.0f),
            .children = {title, sub}
        });
        auto hdr = container({
            .color = 0xFF0D1117,
            .width = StyleValue::percent(100.0f),
            .padding = StyleInsets::symmetric(14.0f, 18.0f),
            .child = hdr_col,
        });

        auto root_col = column({
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .children = {hdr, scroll_flex},
        });
        auto root = container({
            .color = 0xFF0D1117,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .child = root_col,
        });
        return root;
    }
};

class TableDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override { return std::make_unique<TableDemoState>(); }
    std::string_view typeName() const override { return "TableDemoApp"; }
};

int main() {
    AppConfig cfg;
    cfg.title = "ENKI — Table Demo";
    cfg.width = 720; cfg.height = 600;
    cfg.resizable = true; cfg.vsync = false; cfg.target_fps = 0;
    cfg.show_performance_overlay = true;
    cfg.clear_color = 0xFF0D1117;
    return runApp(std::make_shared<TableDemoApp>(), cfg);
}
