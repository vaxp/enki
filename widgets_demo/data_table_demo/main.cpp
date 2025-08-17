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
        auto lbl = std::make_shared<Text>(label);
        lbl->fontSize(13.0f).color(active ? 0xFFFFFFFF : 0xFF8B9BB4);
        ButtonProps opts;
        opts.normal_color  = active ? 0xFF2563EB : 0xFF161B22;
        opts.hover_color   = active ? 0xFF3B82F6 : 0xFF1E2937;
        opts.pressed_color = active ? 0xFF1D4ED8 : 0xFF0D1117;
        opts.border_radius = 6.0f;
        opts.padding       = EdgeInsets::symmetric(6.0f, 16.0f);
        opts.shadow_blur   = 0.0f;
        return std::make_shared<Button>(lbl, [this, idx](){
            setState([this, idx]{ tab_ = idx; });
        }, opts);
    }

    static WidgetPtr cell(const std::string& s, bool bold = false, Color c = 0xFFE2E8F0) {
        auto t = std::make_shared<Text>(s);
        t->fontSize(13.0f).color(c);
        if (bold) t->bold();
        auto wrap = container(t);
        wrap->padding(EdgeInsets::symmetric(10.0f, 12.0f));
        return wrap;
    }

    static WidgetPtr statusBadge(const std::string& s) {
        Color c = s == "Active"   ? 0xFF10B981 :
                  s == "On Leave" ? 0xFFF59E0B : 0xFF8B9BB4;
        auto t = std::make_shared<Text>(s);
        t->fontSize(11.0f).bold().color(Colors::White);
        auto b = container(t);
        b->color(c);
        b->borderRadius(10.0f);
        b->padding(EdgeInsets::symmetric(3.0f, 8.0f));
        return b;
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

        auto t_widget = table(std::move(rows));
        t_widget->columnWidths({
                {0, FlexColumnWidth(2.0f)},
                {1, FlexColumnWidth(1.5f)},
                {2, FixedColumnWidth(70.0f)},
                {3, FixedColumnWidth(90.0f)},
            });
        t_widget->border(TableBorder::symmetric(0x20FFFFFF, 1.0f));
        t_widget->defaultVerticalAlignment(TableCellVerticalAlignment::Middle);
        return t_widget;
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
            DataColumn(cell("Name",   true, 0xFFB0C4D8))
                .onSort([this](int c, bool a){ setState([this,c,a]{ sort_col_=c; sort_asc_=a; }); }),
            DataColumn(cell("Dept",   true, 0xFFB0C4D8))
                .onSort([this](int c, bool a){ setState([this,c,a]{ sort_col_=c; sort_asc_=a; }); }),
            DataColumn(cell("Salary", true, 0xFFB0C4D8))
                .asNumeric()
                .fixedWidth(120.0f)
                .onSort([this](int c, bool a){ setState([this,c,a]{ sort_col_=c; sort_asc_=a; }); }),
            DataColumn(cell("Status", true, 0xFFB0C4D8))
                .fixedWidth(110.0f),
        };

        std::vector<DataRow> rows;
        for (int i = 0; i < (int)sorted.size(); ++i) {
            const auto& e = sorted[i];
            rows.push_back(DataRow({
                DataCell(cell(e.name)),
                DataCell(cell(e.dept)),
                DataCell(cell("$" + std::to_string(e.salary / 1000) + "K")),
                DataCell(statusBadge(e.status)),
            })
            .onTap([name = e.name]{ std::cout << "[DataTable] Row: " << name << "\n"; }));
        }

        DataTableTheme theme;
        theme.use_alternating_rows = true;
        theme.data_row_alt_color   = 0xFF161B22;
        theme.heading_row_color    = 0xFF1E2937;
        theme.data_row_height      = 44.0f;
        theme.divider_thickness    = 1.0f;
        theme.divider_color        = 0x20FFFFFF;
        theme.show_checkbox_column = true;

        auto dt = dataTable(std::move(columns), std::move(rows));
        dt->sortColumnIndex(sort_col_ >= 0 ? sort_col_ : 0);
        dt->sortAscending(sort_asc_);
        dt->withTheme(theme);
        dt->onSelectAll([](bool){ std::cout << "[DataTable] Select all toggled\n"; });
        return dt;
    }

public:
    WidgetPtr build(BuildContext& ctx) override {
        auto title = std::make_shared<Text>("Table / DataTable Demo");
        title->fontSize(22.0f).bold().color(0xFFFFFFFF);
        auto sub = std::make_shared<Text>(
            tab_ == 0 ? "Basic Table with column width strategies"
                      : "DataTable with sort, checkboxes, alternating rows");
        sub->fontSize(12.0f).color(0xFF8B9BB4);
        auto hdr_col = column({title, sub});
        hdr_col->gap(StyleValue::point(4.0f));
        auto hdr = container(hdr_col);
        hdr->padding(EdgeInsets::symmetric(14.0f, 18.0f));
        hdr->color(0xFF0D1117);
        hdr->width(StyleValue::percent(100.0f));

        auto tabs = row({tabButton("Table", 0), tabButton("DataTable", 1)});
        tabs->gap(StyleValue::point(6.0f));
        tabs->padding(StyleInsets::symmetric(8.0f, 12.0f));
        auto tabs_wrap = container(tabs);
        tabs_wrap->color(0xFF161B22);
        tabs_wrap->width(StyleValue::percent(100.0f));

        WidgetPtr content = (tab_ == 0) ? buildBasicTable() : buildDataTable();
        auto content_flex = std::make_shared<FlexItem>(content);
        content_flex->flexGrow(1.0f).flexShrink(1.0f);

        auto root_col = column({hdr, tabs_wrap, content_flex});
        root_col->width(StyleValue::percent(100.0f));
        root_col->height(StyleValue::percent(100.0f));
        auto root = container(root_col);
        root->color(0xFF0D1117);
        root->width(StyleValue::percent(100.0f));
        root->height(StyleValue::percent(100.0f));
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
    cfg.title = "ENKI — Table / DataTable Demo";
    cfg.width = 800; cfg.height = 680;
    cfg.resizable = true; cfg.vsync = false; cfg.target_fps = 0;
    cfg.show_performance_overlay = true;
    cfg.clear_color = 0xFF0D1117;
    return runApp(std::make_shared<TableDemoApp>(), cfg);
}
