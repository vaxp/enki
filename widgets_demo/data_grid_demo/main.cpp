/// @file main.cpp
/// @brief ENKI Advanced DataGrid Interactive Showcase Demo.

#include "enki/app/app.hpp"
#include "enki/widgets/data_grid.hpp"
#include "enki/widgets/search_field.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/state/state.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <iomanip>
#include <sstream>

using namespace enki;

static std::vector<DataGridColumn> buildGridColumns() {
    std::vector<DataGridColumn> cols;

    DataGridColumn col_id("id", "ID", 70.0f, DataGridAlign::Center);
    col_id.sortable = true;
    cols.push_back(col_id);

    DataGridColumn col_name("name", "Employee Name", 170.0f, DataGridAlign::Left);
    col_name.sortable = true;
    cols.push_back(col_name);

    DataGridColumn col_dept("dept", "Department", 130.0f, DataGridAlign::Left);
    col_dept.sortable = true;
    cols.push_back(col_dept);

    DataGridColumn col_role("role", "Title / Role", 150.0f, DataGridAlign::Left);
    col_role.sortable = true;
    cols.push_back(col_role);

    DataGridColumn col_status("status", "Status", 110.0f, DataGridAlign::Center);
    col_status.cell_type = DataGridCellType::Badge;
    col_status.sortable = true;
    cols.push_back(col_status);

    DataGridColumn col_perf("perf", "Performance", 140.0f, DataGridAlign::Left);
    col_perf.cell_type = DataGridCellType::Progress;
    col_perf.sortable = true;
    cols.push_back(col_perf);

    DataGridColumn col_sal("salary", "Salary (USD)", 120.0f, DataGridAlign::Right);
    col_sal.sortable = true;
    cols.push_back(col_sal);

    DataGridColumn col_loc("loc", "Location", 120.0f, DataGridAlign::Left);
    col_loc.sortable = true;
    cols.push_back(col_loc);

    return cols;
}

static std::vector<DataGridRow> generateMockRows() {
    std::vector<DataGridRow> rows;

    struct EmpData {
        std::string id, name, dept, role, status, salary, loc;
        Color status_bg;
        float perf;
        std::string perf_str;
    };

    std::vector<EmpData> data = {
        {"#1001", "Alexander Vance", "AI Research", "Principal Architect", "Active", "$ 195,000", "San Francisco", 0x2E10B981, 0.96f, "96%"},
        {"#1002", "Elena Rostova", "Engineering", "Lead C++ Systems", "Active", "$ 175,000", "Zurich", 0x2E10B981, 0.92f, "92%"},
        {"#1003", "Marcus Chen", "Product Design", "Design Director", "Remote", "$ 160,000", "Tokyo", 0x2E38BDF8, 0.88f, "88%"},
        {"#1004", "Sophia Al-Mansoor", "Security & Kernel", "Chief Security Eng", "Active", "$ 185,000", "Dubai", 0x2E10B981, 0.95f, "95%"},
        {"#1005", "David Kim", "Engineering", "Graphics & Vulkan", "On Leave", "$ 155,000", "Seoul", 0x2EF59E0B, 0.84f, "84%"},
        {"#1006", "Clara Dubois", "Cloud & Infra", "DevOps Specialist", "Active", "$ 148,000", "Paris", 0x2E10B981, 0.89f, "89%"},
        {"#1007", "Liam O'Connor", "Engineering", "Wayland & Linux Lead", "Active", "$ 170,000", "Dublin", 0x2E10B981, 0.94f, "94%"},
        {"#1008", "Amina Zahra", "AI Research", "NLP Research Scientist", "Remote", "$ 165,000", "London", 0x2E38BDF8, 0.91f, "91%"},
        {"#1009", "Hiroshi Tanaka", "Product Design", "Senior UI/UX Designer", "Active", "$ 138,000", "Tokyo", 0x2E10B981, 0.86f, "86%"},
        {"#1010", "Oliver Smith", "Finance", "Senior Quant Analyst", "Active", "$ 152,000", "New York", 0x2E10B981, 0.87f, "87%"},
        {"#1011", "Isabella Rossi", "Engineering", "Compiler & Anu Layout", "Remote", "$ 162,000", "Milan", 0x2E38BDF8, 0.93f, "93%"},
        {"#1012", "Noah Becker", "Security & Kernel", "Memory Safety Auditor", "Active", "$ 158,000", "Berlin", 0x2E10B981, 0.90f, "90%"}
    };

    for (const auto& e : data) {
        DataGridRow r(e.id);
        r.set("id", e.id)
         .set("name", e.name)
         .set("dept", e.dept)
         .set("role", e.role)
         .setBadge("status", e.status, e.status_bg, 0xFFFFFFFF)
         .setProgress("perf", e.perf, e.perf_str)
         .set("salary", e.salary)
         .set("loc", e.loc);
        rows.push_back(r);
    }

    return rows;
}

class DataGridDemoState : public State {
private:
    std::shared_ptr<DataGridController> grid_ctrl_;
    std::shared_ptr<SearchFieldController> search_ctrl_;
    std::string hud_message_ = "Hover column edges to resize. Click headers to sort. Ctrl+C to copy selected rows as CSV.";

public:
    void initState() override {
        State::initState();
        grid_ctrl_ = std::make_shared<DataGridController>(buildGridColumns(), generateMockRows());
        grid_ctrl_->setPageSize(8); // Show 8 items per page
        search_ctrl_ = std::make_shared<SearchFieldController>();
    }

    WidgetPtr build(BuildContext&) override {
        // Main Header
        auto title = text("Advanced Enterprise DataGrid Suite");
        title->fontSize(22.0f).bold().color(0xFFFFFFFF);

        auto sub = text("Live column drag-resizing, multi-column sorting, row selection, pagination, summary aggregations, and CSV export");
        sub->fontSize(13.0f).color(0xFF94A3B8);

        std::vector<WidgetPtr> title_items = {title, sub};
        auto title_col = column(title_items);
        title_col->alignItems(Align::Center);

        // ── Grid Toolbar (Search Bar + Action Buttons) ────────────────
        SearchFieldOptions search_opts;
        search_opts.placeholder = "Filter by name, dept, role, status, salary...";
        search_opts.size = SearchFieldSize::Small;
        search_opts.on_changed = [this](std::string_view q) {
            grid_ctrl_->setGlobalFilter(q);
            setState([] {});
        };

        auto search_widget = searchField(search_ctrl_, search_opts);
        auto search_box = container(search_widget);
        search_box->width(420.0f);

        // Action Buttons
        auto btn_add = button(text("+ Add Employee"), [this] {
            static int s_counter = 1013;
            std::string new_id = "#" + std::to_string(s_counter++);
            DataGridRow r(new_id);
            r.set("id", new_id)
             .set("name", "New Employee " + new_id)
             .set("dept", "Engineering")
             .set("role", "Software Engineer")
             .setBadge("status", "Active", 0x2E10B981, 0xFFFFFFFF)
             .setProgress("perf", 0.85f, "85%")
             .set("salary", "$ 140,000")
             .set("loc", "Remote");
            grid_ctrl_->addRow(r);
            hud_message_ = "Added new employee " + new_id;
            setState([] {});
        });

        auto btn_del = button(text("- Delete Selected"), [this] {
            const auto& sel = grid_ctrl_->getSelectedRowIds();
            if (sel.empty()) {
                hud_message_ = "No rows selected to delete!";
            } else {
                size_t count = sel.size();
                for (const auto& id : sel) {
                    grid_ctrl_->removeRow(id);
                }
                hud_message_ = "Deleted " + std::to_string(count) + " selected rows.";
            }
            setState([] {});
        });

        auto btn_export = button(text("📊 Export CSV"), [this] {
            std::string csv = grid_ctrl_->exportToCsv(false);
            if (Platform::instance()) {
                ClipboardData data;
                data.setText(csv);
                Platform::instance()->setClipboardData(data);
                Platform::instance()->setClipboardText(csv);
            }
            hud_message_ = "Exported " + std::to_string(grid_ctrl_->getTotalFilteredCount()) + " rows to CSV and copied to Clipboard!";
            setState([] {});
        });

        std::vector<WidgetPtr> tool_actions = {btn_add, btn_del, btn_export};
        auto tool_act_row = row(tool_actions);
        tool_act_row->gap(StyleValue::point(8.0f)).alignItems(Align::Center);

        std::vector<WidgetPtr> toolbar_items = {search_box, tool_act_row};
        auto toolbar_row = row(toolbar_items);
        toolbar_row->justifyContent(Justify::SpaceBetween)
                   .alignItems(Align::Center);

        // ── DataGrid Options & Summary Calculator ─────────────────────
        DataGridOptions grid_opts;
        grid_opts.selection_mode = DataGridSelectionMode::RowMultiple;
        grid_opts.show_pagination = true;
        grid_opts.show_summary_footer = true;
        grid_opts.row_height = 36.0f;
        grid_opts.header_height = 38.0f;
        grid_opts.footer_height = 36.0f;
        grid_opts.summary_calculator = [](const std::string& col_key, const std::vector<DataGridRow>& rows) -> std::string {
            if (col_key == "id") return "Total:";
            if (col_key == "name") return std::to_string(rows.size()) + " Employees";
            if (col_key == "perf") {
                float total_p = 0.0f;
                int count = 0;
                for (const auto& r : rows) {
                    auto it = r.cells.find("perf");
                    if (it != r.cells.end()) {
                        total_p += it->second.progress;
                        count++;
                    }
                }
                if (count > 0) {
                    std::ostringstream ss;
                    ss << "Avg: " << std::fixed << std::setprecision(1) << (total_p / count * 100.0f) << "%";
                    return ss.str();
                }
            }
            if (col_key == "salary") {
                long long sum = 0;
                for (const auto& r : rows) {
                    std::string s = r.get("salary");
                    std::string clean = "";
                    for (char c : s) if (std::isdigit(c)) clean += c;
                    if (!clean.empty()) sum += std::stoll(clean);
                }
                std::ostringstream ss;
                ss << "$ " << (sum / 1000) << ",000";
                return ss.str();
            }
            return "";
        };

        grid_opts.on_selection_changed = [this](const std::set<std::string>& selected) {
            hud_message_ = "Selected " + std::to_string(selected.size()) + " rows. Press Ctrl+C to copy as CSV.";
            setState([] {});
        };

        auto grid_widget = dataGrid(grid_ctrl_, grid_opts);

        // ── HUD / Status Bar ──────────────────────────────────────────
        auto hud_txt = text("💡 " + hud_message_);
        hud_txt->fontSize(12.0f).color(0xFF38BDF8);

        std::vector<WidgetPtr> hud_items = {hud_txt};
        auto hud_row = row(hud_items);

        auto hud_box = container(hud_row);
        hud_box->paddingSymmetric(4.0f, 8.0f);

        // Main Card Stack
        std::vector<WidgetPtr> card_items = {toolbar_row, grid_widget, hud_box};
        auto card_col = column(card_items);
        card_col->gap(StyleValue::point(10.0f));

        auto grid_card = container(card_col);
        grid_card->color(0xFF1E293B)
                 .borderRadius(10.0f)
                 .border(0xFF334155, 1.0f)
                 .paddingAll(16.0f)
                 .width(1080.0f);

        std::vector<WidgetPtr> root_items = {title_col, grid_card};
        auto root_col = column(root_items);
        root_col->gap(StyleValue::point(14.0f))
                .alignItems(Align::Center);

        auto app_root = container(root_col);
        app_root->color(0xFF0B1120)
                .paddingAll(16.0f)
                .flexGrow(1.0f);

        return app_root;
    }
};

class DataGridDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<DataGridDemoState>();
    }
    std::string_view typeName() const override { return "DataGridDemoApp"; }
};

int main() {
    std::cout << "====================================================\n";
    std::cout << "  ENKI Engine — Advanced DataGrid Widget Demo\n";
    std::cout << "====================================================\n";

    AppConfig config;
    config.title       = "Enki Engine — Advanced DataGrid Demo";
    config.width       = 1160;
    config.height      = 720;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 60;
    config.clear_color = 0xFF0B1120;

    return runApp(std::make_shared<DataGridDemoApp>(), config);
}
