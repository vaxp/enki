/// @file main.cpp — Table + DataTable Combined Demo
#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/table.hpp"
#include "enki/widgets/data_table.hpp"
#include "enki/state/state.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

using namespace enki;

// ── Sample data ────────────────────────────────────────────────
struct Employee {
    std::string name;
    std::string dept;
    int         salary;
    std::string status;
};

static const std::vector<Employee> kEmployees = {
    {"Alice Johnson",   "Engineering",   95000, "Active"},
    {"Bob Martinez",    "Design",        78000, "Active"},
    {"Carol Williams",  "Marketing",     82000, "On Leave"},
    {"David Lee",       "Engineering",  110000, "Active"},
    {"Emma Davis",      "HR",            65000, "Active"},
    {"Frank Wilson",    "Engineering",  120000, "Active"},
    {"Grace Taylor",    "Finance",       88000, "Inactive"},
    {"Henry Brown",     "Design",        72000, "Active"},
    {"Isabelle Jones",  "Marketing",     79000, "Active"},
    {"James Garcia",    "Finance",       92000, "Active"},
};

class TableDemoState : public State {
    int  tab_          = 0;   // 0=Table, 1=DataTable
    int  sort_col_     = -1;
    bool sort_asc_     = true;
    std::vector<bool> row_sel_;

    void initState() override {
        State::initState();
        row_sel_.assign(kEmployees.size(), false);
    }

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
        opts.padding       = EdgeInsets::symmetric(6.0f, 16.0f);
        opts.shadow_blur   = 0.0f;
        return button(lbl, [this, idx](){
            setState([this, idx]{ tab_ = idx; });
        }, opts);
    }

    static WidgetPtr cell(const std::string& s, bool bold = false, Color c = 0xFFE2E8F0) {
        return container({
            .padding = StyleInsets::symmetric(10.0f, 12.0f),
            .child = text(s, {
                .color = c,
                .font_size = 13.0f,
                .font_weight = bold ? FontWeight::Bold : FontWeight::Normal,
            })
        });
    }

    static WidgetPtr statusBadge(const std::string& s) {
        Color c = s == "Active"   ? 0xFF10B981 :
                  s == "On Leave" ? 0xFFF59E0B : 0xFF8B9BB4;
        return container({
            .color = c,
            .border_radius = BorderRadius::circular(10.0f),
            .padding = StyleInsets::symmetric(3.0f, 8.0f),
            .child = text(s, {
                .color = Colors::White,
                .font_size = 11.0f,
                .font_weight = FontWeight::Bold,
            })
        });
    }

    WidgetPtr buildBasicTable() {
        std::vector<TableRow> rows;

        // Header row
        rows.push_back(TableRow({
            cell("Name",       true, 0xFFB0C4D8),
            cell("Department", true, 0xFFB0C4D8),
            cell("Salary",     true, 0xFFB0C4D8),
            cell("Status",     true, 0xFFB0C4D8),
        }, BoxDecoration(0xFF1E2937)));

        for (auto& e : kEmployees) {
            rows.push_back(TableRow({
                cell(e.name),
                cell(e.dept),
                cell("$" + std::to_string(e.salary / 1000) + "K"),
                statusBadge(e.status),
            }));
        }

        return Table {
            .rows = std::move(rows),
            .column_widths = {
                {0, FlexColumnWidth(2.0f)},
                {1, FlexColumnWidth(1.5f)},
                {2, FixedColumnWidth(70.0f)},
                {3, FixedColumnWidth(90.0f)},
            },
            .border = TableBorder::symmetric(0x20FFFFFF, 1.0f),
            .default_vertical_alignment = TableCellVerticalAlignment::Middle,
        };
    }

    WidgetPtr buildDataTable() {
        // Sort employees if needed
        std::vector<Employee> sorted = kEmployees;
        if (sort_col_ >= 0) {
            std::sort(sorted.begin(), sorted.end(), [this](const Employee& a, const Employee& b){
                auto cmp = sort_asc_;
                if (sort_col_ == 0) return cmp ? a.name   < b.name   : a.name   > b.name;
                if (sort_col_ == 1) return cmp ? a.dept   < b.dept   : a.dept   > b.dept;
                if (sort_col_ == 2) return cmp ? a.salary < b.salary : a.salary > b.salary;
                return cmp ? a.status < b.status : a.status > b.status;
            });
        }

        std::vector<DataColumn> columns = {
            DataColumn(cell("Name",   true, 0xFFB0C4D8), true, [this](int c, bool a){ setState([this,c,a]{ sort_col_=c; sort_asc_=a; }); }),
            DataColumn(cell("Dept",   true, 0xFFB0C4D8), true, [this](int c, bool a){ setState([this,c,a]{ sort_col_=c; sort_asc_=a; }); }),
            DataColumn(cell("Salary", true, 0xFFB0C4D8), true, [this](int c, bool a){ setState([this,c,a]{ sort_col_=c; sort_asc_=a; }); }).asNumeric().fixedWidth(120.0f),
            DataColumn(cell("Status", true, 0xFFB0C4D8)).fixedWidth(110.0f),
        };

        std::vector<DataRow> rows;
        for (int i = 0; i < (int)sorted.size(); ++i) {
            const auto& e = sorted[i];
            DataRow row({
                DataCell(cell(e.name)),
                DataCell(cell(e.dept)),
                DataCell(cell("$" + std::to_string(e.salary / 1000) + "K")),
                DataCell(statusBadge(e.status)),
            });
            row.onTap([name = e.name]{ std::cout << "[DataTable] Row: " << name << "\n"; });
            rows.push_back(std::move(row));
        }

        DataTableTheme theme;
        theme.use_alternating_rows = true;
        theme.data_row_alt_color   = 0xFF161B22;
        theme.heading_row_color    = 0xFF1E2937;
        theme.data_row_height      = 44.0f;
        theme.divider_thickness    = 1.0f;
        theme.divider_color        = 0x20FFFFFF;
        theme.show_checkbox_column = true;

        return DataTable {
            .columns = std::move(columns),
            .rows = std::move(rows),
            .sort_column_index = sort_col_ >= 0 ? sort_col_ : 0,
            .sort_ascending = sort_asc_,
            .on_select_all = [](bool){ std::cout << "[DataTable] Select all toggled\n"; },
            .theme = theme,
        };
    }

public:
    WidgetPtr build(BuildContext& ctx) override {
        auto title = text("Table / DataTable Demo", {
            .color = 0xFFFFFFFF,
            .font_size = 22.0f,
            .font_weight = FontWeight::Bold,
        });

        auto sub = text(
            tab_ == 0 ? "Basic Table with column width strategies"
                      : "DataTable with sort, checkboxes, alternating rows", {
            .color = 0xFF8B9BB4,
            .font_size = 12.0f,
        });

        auto hdr = container({
            .color = 0xFF0D1117,
            .width = StyleValue::percent(100.0f),
            .padding = StyleInsets::symmetric(14.0f, 18.0f),
            .child = column({
                .gap = StyleValue::point(4.0f),
                .children = {title, sub}
            })
        });

        auto tabs_wrap = container({
            .color = 0xFF161B22,
            .width = StyleValue::percent(100.0f),
            .padding = StyleInsets::symmetric(8.0f, 12.0f),
            .child = row({
                .gap = StyleValue::point(6.0f),
                .children = {tabButton("Table", 0), tabButton("DataTable", 1)}
            })
        });

        WidgetPtr content = (tab_ == 0) ? buildBasicTable() : buildDataTable();

        return container({
            .color = 0xFF0D1117,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .child = column({
                .children = {
                    hdr,
                    tabs_wrap,
                    container({
                        .flex_grow = 1.0f,
                        .flex_shrink = 1.0f,
                        .child = content
                    })
                }
            })
        });
    }
};

class TableDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override { return std::make_unique<TableDemoState>(); }
    std::string_view typeName() const override { return "TableDemoApp"; }
};

int main() {
    AppConfig cfg;
    cfg.title = "ENKI — Table / DataTable Demo";
    cfg.width = 800; cfg.height = 680;
    cfg.resizable = true; cfg.vsync = false; cfg.target_fps = 0;
    cfg.show_performance_overlay = true;
    cfg.clear_color = 0xFF0D1117;
    return runApp(std::make_shared<TableDemoApp>(), cfg);
}
