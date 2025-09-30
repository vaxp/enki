/// @file data_grid.cpp
/// @brief Implementation of Advanced DataGrid widget.

#include "enki/widgets/data_grid.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/platform/platform.hpp"
#include "enki/platform/clipboard.hpp"
#include "enki/state/state.hpp"
#include "enki/tree/element.hpp"
#include "enki/tree/build_context.hpp"
#include "enki/rendering/canvas.hpp"
#include "enki/app/app.hpp"

#include <include/core/SkCanvas.h>
#include <include/core/SkPaint.h>
#include <include/core/SkPath.h>
#include <include/core/SkRRect.h>
#include <include/core/SkFontMgr.h>
#include <include/ports/SkFontMgr_fontconfig.h>
#include <modules/skparagraph/include/FontCollection.h>
#include <modules/skparagraph/include/Paragraph.h>
#include <modules/skparagraph/include/ParagraphBuilder.h>
#include <modules/skparagraph/include/ParagraphStyle.h>
#include <modules/skparagraph/include/TextStyle.h>
#include <modules/skparagraph/include/DartTypes.h>

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cctype>
#include <cmath>

namespace enki {

static sk_sp<skia::textlayout::FontCollection> getDataGridFontCollection() {
    static sk_sp<skia::textlayout::FontCollection> s_fc = []() {
        auto m = SkFontMgr_New_FontConfig(nullptr);
        if (!m) m = SkFontMgr::RefDefault();
        auto fc = sk_make_sp<skia::textlayout::FontCollection>();
        fc->setDefaultFontManager(m);
        fc->enableFontFallback();
        return fc;
    }();
    return s_fc;
}

// ════════════════════════════════════════════════════════════════
// DataGridController Implementation
// ════════════════════════════════════════════════════════════════

void DataGridController::applyFilterAndSort() {
    filtered_row_indices_.clear();
    std::string lfilter = global_filter_;
    std::transform(lfilter.begin(), lfilter.end(), lfilter.begin(), ::tolower);

    // 1. Filter
    for (size_t i = 0; i < raw_rows_.size(); ++i) {
        const auto& row = raw_rows_[i];
        if (lfilter.empty()) {
            filtered_row_indices_.push_back(i);
        } else {
            bool matches = false;
            for (const auto& col : columns_) {
                if (!col.visible || !col.filterable) continue;
                std::string cell_val = row.get(col.key);
                std::transform(cell_val.begin(), cell_val.end(), cell_val.begin(), ::tolower);
                if (cell_val.find(lfilter) != std::string::npos) {
                    matches = true;
                    break;
                }
            }
            if (matches) {
                filtered_row_indices_.push_back(i);
            }
        }
    }

    // 2. Multi-column Sort
    if (!sort_rules_.empty()) {
        std::stable_sort(filtered_row_indices_.begin(), filtered_row_indices_.end(),
            [this](size_t a_idx, size_t b_idx) {
                const auto& row_a = raw_rows_[a_idx];
                const auto& row_b = raw_rows_[b_idx];

                for (const auto& rule : sort_rules_) {
                    if (rule.direction == DataGridSortDirection::None) continue;

                    std::string val_a = row_a.get(rule.column_key);
                    std::string val_b = row_b.get(rule.column_key);

                    if (val_a == val_b) continue;

                    // Numeric comparison test
                    try {
                        size_t pos_a = 0, pos_b = 0;
                        double num_a = std::stod(val_a, &pos_a);
                        double num_b = std::stod(val_b, &pos_b);
                        if (pos_a == val_a.length() && pos_b == val_b.length()) {
                            return (rule.direction == DataGridSortDirection::Ascending) ? (num_a < num_b) : (num_a > num_b);
                        }
                    } catch (...) {}

                    // String comparison fallback
                    bool asc = (rule.direction == DataGridSortDirection::Ascending);
                    return asc ? (val_a < val_b) : (val_a > val_b);
                }
                return a_idx < b_idx;
            });
    }
}

void DataGridController::toggleSort(const std::string& col_key, bool multi_sort) {
    if (!multi_sort) {
        DataGridSortDirection current_dir = getSortDirection(col_key);
        sort_rules_.clear();
        if (current_dir == DataGridSortDirection::None) {
            sort_rules_.push_back({col_key, DataGridSortDirection::Ascending});
        } else if (current_dir == DataGridSortDirection::Ascending) {
            sort_rules_.push_back({col_key, DataGridSortDirection::Descending});
        }
    } else {
        bool found = false;
        for (auto it = sort_rules_.begin(); it != sort_rules_.end(); ++it) {
            if (it->column_key == col_key) {
                if (it->direction == DataGridSortDirection::Ascending) {
                    it->direction = DataGridSortDirection::Descending;
                } else {
                    sort_rules_.erase(it);
                }
                found = true;
                break;
            }
        }
        if (!found) {
            sort_rules_.push_back({col_key, DataGridSortDirection::Ascending});
        }
    }
    applyFilterAndSort();
}

DataGridSortDirection DataGridController::getSortDirection(const std::string& col_key) const {
    for (const auto& rule : sort_rules_) {
        if (rule.column_key == col_key) return rule.direction;
    }
    return DataGridSortDirection::None;
}

int DataGridController::getSortPriority(const std::string& col_key) const {
    for (size_t i = 0; i < sort_rules_.size(); ++i) {
        if (sort_rules_[i].column_key == col_key) return static_cast<int>(i + 1);
    }
    return 0;
}

void DataGridController::selectRow(const std::string& row_id, bool select, bool clear_others) {
    if (clear_others) selected_row_ids_.clear();
    if (select) {
        selected_row_ids_.insert(row_id);
    } else {
        selected_row_ids_.erase(row_id);
    }
}

void DataGridController::toggleRowSelection(const std::string& row_id, bool clear_others) {
    bool is_sel = isRowSelected(row_id);
    selectRow(row_id, !is_sel, clear_others);
}

void DataGridController::selectAll(bool select) {
    if (select) {
        for (size_t idx : filtered_row_indices_) {
            selected_row_ids_.insert(raw_rows_[idx].id);
        }
    } else {
        selected_row_ids_.clear();
    }
}

bool DataGridController::isRowSelected(const std::string& row_id) const {
    return selected_row_ids_.find(row_id) != selected_row_ids_.end();
}

bool DataGridController::isAllSelected() const {
    if (filtered_row_indices_.empty()) return false;
    for (size_t idx : filtered_row_indices_) {
        if (!isRowSelected(raw_rows_[idx].id)) return false;
    }
    return true;
}

bool DataGridController::isPartiallySelected() const {
    if (selected_row_ids_.empty() || isAllSelected()) return false;
    return true;
}

int DataGridController::getTotalPages() const {
    if (page_size_ <= 0 || filtered_row_indices_.empty()) return 1;
    return static_cast<int>(std::ceil(static_cast<double>(filtered_row_indices_.size()) / page_size_));
}

void DataGridController::setPage(int page) {
    int max_p = std::max(0, getTotalPages() - 1);
    current_page_ = std::clamp(page, 0, max_p);
}

std::string DataGridController::exportToCsv(bool selected_only) const {
    std::ostringstream ss;

    // Header line
    bool first = true;
    for (const auto& col : columns_) {
        if (!col.visible) continue;
        if (!first) ss << ",";
        ss << "\"" << col.title << "\"";
        first = false;
    }
    ss << "\n";

    // Rows
    for (size_t idx : filtered_row_indices_) {
        const auto& row = raw_rows_[idx];
        if (selected_only && !isRowSelected(row.id)) continue;

        first = true;
        for (const auto& col : columns_) {
            if (!col.visible) continue;
            if (!first) ss << ",";
            std::string cell_val = row.get(col.key);
            // Escape inner quotes
            size_t pos = 0;
            while ((pos = cell_val.find('"', pos)) != std::string::npos) {
                cell_val.replace(pos, 1, "\"\"");
                pos += 2;
            }
            ss << "\"" << cell_val << "\"";
            first = false;
        }
        ss << "\n";
    }

    return ss.str();
}

// ════════════════════════════════════════════════════════════════
// Custom RenderBox for DataGrid Table View
// ════════════════════════════════════════════════════════════════

class RenderDataGridBox : public RenderBox {
public:
    std::shared_ptr<DataGridController> controller;
    DataGridProps options;

    int hovered_row_index = -1;
    int hovered_col_header = -1;
    int hovered_col_divider = -1; // Index of column divider being hovered

    RenderDataGridBox(std::shared_ptr<DataGridController> ctrl, DataGridProps opt)
        : controller(std::move(ctrl)), options(std::move(opt)) {
        updateFlexboxStyle();
    }

    float computeGridHeight() const {
        float h = 0.0f;
        if (options.show_header) h += options.header_height;

        int page_sz = controller->getPageSize();
        size_t count = controller->getFilteredIndices().size();
        size_t rows_on_page = (page_sz > 0) ? std::min(static_cast<size_t>(page_sz), count) : count;
        if (rows_on_page == 0) {
            h += options.row_height * 2.5f;
        } else {
            h += static_cast<float>(rows_on_page) * options.row_height;
        }

        if (options.show_summary_footer && options.summary_calculator) {
            h += options.footer_height;
        }
        return h;
    }

    void updateFlexboxStyle() {
        FlexboxStyle st;
        st.height = StyleValue::point(computeGridHeight());
        applyFlexboxStyle(anuNode(), st);
    }

    float getCheckboxWidth() const {
        return (options.selection_mode == DataGridSelectionMode::RowMultiple) ? 40.0f : 0.0f;
    }

    float computeTotalGridWidth() const {
        float w = getCheckboxWidth();
        for (const auto& col : controller->getColumns()) {
            if (col.visible) w += col.width;
        }
        return w;
    }

    void paint(PaintContext& ctx) override {
        SkCanvas* sk_canvas = static_cast<SkCanvas*>(ctx.canvas.getNativeHandle());
        if (!sk_canvas || size_.width <= 0 || size_.height <= 0) return;

        auto fc = getDataGridFontCollection();
        if (!fc) return;

        SkRect grid_bounds = SkRect::MakeXYWH(ctx.offset.x, ctx.offset.y, size_.width, size_.height);
        SkRRect grid_rrect;
        grid_rrect.setRectXY(grid_bounds, options.border_radius, options.border_radius);

        // 1. Grid Background
        SkPaint bg_paint;
        bg_paint.setAntiAlias(true);
        bg_paint.setColor(options.background_color);
        sk_canvas->drawRRect(grid_rrect, bg_paint);

        sk_canvas->save();
        sk_canvas->clipRRect(grid_rrect, true);

        const auto& columns = controller->getColumns();
        const auto& raw_rows = controller->getRawRows();
        const auto& filtered_indices = controller->getFilteredIndices();
        float cb_w = getCheckboxWidth();

        // 2. Paint Header Row
        float cur_y = ctx.offset.y;
        if (options.show_header) {
            SkRect hdr_rect = SkRect::MakeXYWH(ctx.offset.x, cur_y, size_.width, options.header_height);
            SkPaint hdr_bg;
            hdr_bg.setColor(options.header_bg_color);
            sk_canvas->drawRect(hdr_rect, hdr_bg);

            float col_x = ctx.offset.x;

            // Header Select-All Checkbox
            if (cb_w > 0) {
                float cb_cx = col_x + cb_w * 0.5f;
                float cb_cy = cur_y + options.header_height * 0.5f;
                float cb_sz = 14.0f;
                SkRRect cb_box;
                cb_box.setRectXY(SkRect::MakeXYWH(cb_cx - cb_sz * 0.5f, cb_cy - cb_sz * 0.5f, cb_sz, cb_sz), 3.0f, 3.0f);

                bool all_sel = controller->isAllSelected();
                bool part_sel = controller->isPartiallySelected();

                SkPaint cb_paint;
                cb_paint.setAntiAlias(true);
                cb_paint.setColor(all_sel || part_sel ? 0xFF38BDF8 : 0xFF334155);
                sk_canvas->drawRRect(cb_box, cb_paint);

                if (all_sel) {
                    SkPaint chk_paint;
                    chk_paint.setAntiAlias(true);
                    chk_paint.setColor(0xFF0F172A);
                    chk_paint.setStyle(SkPaint::kStroke_Style);
                    chk_paint.setStrokeWidth(1.8f);
                    SkPath p;
                    p.moveTo(cb_cx - 3.5f, cb_cy);
                    p.lineTo(cb_cx - 1.0f, cb_cy + 3.0f);
                    p.lineTo(cb_cx + 4.0f, cb_cy - 2.5f);
                    sk_canvas->drawPath(p, chk_paint);
                } else if (part_sel) {
                    SkPaint dash_paint;
                    dash_paint.setAntiAlias(true);
                    dash_paint.setColor(0xFF0F172A);
                    dash_paint.setStrokeWidth(2.0f);
                    sk_canvas->drawLine(cb_cx - 3.5f, cb_cy, cb_cx + 3.5f, cb_cy, dash_paint);
                }

                col_x += cb_w;
            }

            // Column Headers
            for (size_t i = 0; i < columns.size(); ++i) {
                const auto& col = columns[i];
                if (!col.visible) continue;

                skia::textlayout::ParagraphStyle p_style;
                p_style.setTextAlign(col.align == DataGridAlign::Right ? skia::textlayout::TextAlign::kRight :
                                    (col.align == DataGridAlign::Center ? skia::textlayout::TextAlign::kCenter : skia::textlayout::TextAlign::kLeft));

                auto b_hdr = skia::textlayout::ParagraphBuilder::make(p_style, fc);
                skia::textlayout::TextStyle t_hdr;
                t_hdr.setFontSize(12.5f);
                t_hdr.setColor(static_cast<SkColor>(options.header_text_color));
                b_hdr->pushStyle(t_hdr);

                std::string header_title = col.title;
                auto dir = controller->getSortDirection(col.key);
                if (dir == DataGridSortDirection::Ascending) header_title += " ▲";
                else if (dir == DataGridSortDirection::Descending) header_title += " ▼";

                b_hdr->addText(header_title.c_str(), header_title.length());
                auto p_hdr = b_hdr->Build();
                p_hdr->layout(col.width - 16.0f);

                float p_y = cur_y + (options.header_height - p_hdr->getHeight()) * 0.5f;
                p_hdr->paint(sk_canvas, col_x + 8.0f, p_y);

                // Divider Line
                SkPaint div_paint;
                div_paint.setColor(options.border_color);
                sk_canvas->drawLine(col_x + col.width, cur_y, col_x + col.width, cur_y + options.header_height, div_paint);

                col_x += col.width;
            }

            // Header Bottom Divider
            SkPaint h_div;
            h_div.setColor(options.border_color);
            sk_canvas->drawLine(ctx.offset.x, cur_y + options.header_height, ctx.offset.x + size_.width, cur_y + options.header_height, h_div);

            cur_y += options.header_height;
        }

        // 3. Paint Rows (Paged slice)
        int page = controller->getCurrentPage();
        int page_sz = controller->getPageSize();
        size_t start_idx = (page_sz > 0) ? static_cast<size_t>(page * page_sz) : 0;
        size_t end_idx = (page_sz > 0) ? std::min(start_idx + page_sz, filtered_indices.size()) : filtered_indices.size();

        if (filtered_indices.empty()) {
            skia::textlayout::ParagraphStyle p_style;
            p_style.setTextAlign(skia::textlayout::TextAlign::kCenter);
            auto b_empty = skia::textlayout::ParagraphBuilder::make(p_style, fc);
            skia::textlayout::TextStyle t_empty;
            t_empty.setFontSize(13.0f);
            t_empty.setColor(0xFF64748B);
            b_empty->pushStyle(t_empty);
            b_empty->addText("No matching records found", 25);
            auto p_empty = b_empty->Build();
            p_empty->layout(size_.width);
            p_empty->paint(sk_canvas, ctx.offset.x, cur_y + options.row_height * 0.7f);
            cur_y += options.row_height * 2.5f;
        }

        for (size_t r = start_idx; r < end_idx; ++r) {
            size_t row_idx = filtered_indices[r];
            const auto& row = raw_rows[row_idx];
            bool is_sel = controller->isRowSelected(row.id);
            bool is_hov = (hovered_row_index == static_cast<int>(r));

            // Row Background
            SkRect r_rect = SkRect::MakeXYWH(ctx.offset.x, cur_y, size_.width, options.row_height);
            SkPaint r_bg;
            if (is_sel) {
                r_bg.setColor(options.row_selected_color);
            } else if (is_hov) {
                r_bg.setColor(options.row_hover_color);
            } else if (options.zebra_stripes && (r % 2 == 1)) {
                r_bg.setColor(options.zebra_row_bg_color);
            } else {
                r_bg.setColor(options.row_bg_color);
            }
            sk_canvas->drawRect(r_rect, r_bg);

            float col_x = ctx.offset.x;

            // Row Checkbox
            if (cb_w > 0) {
                float cb_cx = col_x + cb_w * 0.5f;
                float cb_cy = cur_y + options.row_height * 0.5f;
                float cb_sz = 14.0f;
                SkRRect cb_box;
                cb_box.setRectXY(SkRect::MakeXYWH(cb_cx - cb_sz * 0.5f, cb_cy - cb_sz * 0.5f, cb_sz, cb_sz), 3.0f, 3.0f);

                SkPaint cb_paint;
                cb_paint.setAntiAlias(true);
                cb_paint.setColor(is_sel ? 0xFF38BDF8 : 0xFF334155);
                sk_canvas->drawRRect(cb_box, cb_paint);

                if (is_sel) {
                    SkPaint chk_paint;
                    chk_paint.setAntiAlias(true);
                    chk_paint.setColor(0xFF0F172A);
                    chk_paint.setStyle(SkPaint::kStroke_Style);
                    chk_paint.setStrokeWidth(1.8f);
                    SkPath p;
                    p.moveTo(cb_cx - 3.5f, cb_cy);
                    p.lineTo(cb_cx - 1.0f, cb_cy + 3.0f);
                    p.lineTo(cb_cx + 4.0f, cb_cy - 2.5f);
                    sk_canvas->drawPath(p, chk_paint);
                }

                col_x += cb_w;
            }

            // Row Cells
            for (size_t c = 0; c < columns.size(); ++c) {
                const auto& col = columns[c];
                if (!col.visible) continue;

                auto it = row.cells.find(col.key);
                if (it != row.cells.end()) {
                    const auto& cell = it->second;

                    if (col.cell_type == DataGridCellType::Badge && cell.badge_bg != 0) {
                        // Badge Render
                        skia::textlayout::ParagraphStyle p_style;
                        p_style.setTextAlign(skia::textlayout::TextAlign::kCenter);
                        auto b_badge = skia::textlayout::ParagraphBuilder::make(p_style, fc);
                        skia::textlayout::TextStyle t_badge;
                        t_badge.setFontSize(11.0f);
                        t_badge.setColor(static_cast<SkColor>(cell.badge_fg != 0 ? cell.badge_fg : 0xFFFFFFFF));
                        b_badge->pushStyle(t_badge);
                        b_badge->addText(cell.display_text.c_str(), cell.display_text.length());
                        auto p_badge = b_badge->Build();
                        p_badge->layout(col.width);

                        float bw = p_badge->getMaxIntrinsicWidth() + 12.0f;
                        float bh = p_badge->getHeight() + 4.0f;
                        float bx = col_x + 8.0f;
                        float by = cur_y + (options.row_height - bh) * 0.5f;

                        SkRRect badge_rrect;
                        badge_rrect.setRectXY(SkRect::MakeXYWH(bx, by, bw, bh), 4.0f, 4.0f);
                        SkPaint badge_paint;
                        badge_paint.setAntiAlias(true);
                        badge_paint.setColor(cell.badge_bg);
                        sk_canvas->drawRRect(badge_rrect, badge_paint);

                        p_badge->paint(sk_canvas, bx + 6.0f, by + 2.0f);
                    } else if (col.cell_type == DataGridCellType::Progress) {
                        // Progress Bar Render
                        float pb_x = col_x + 8.0f;
                        float pb_w = col.width - 50.0f;
                        float pb_h = 6.0f;
                        float pb_y = cur_y + (options.row_height - pb_h) * 0.5f;

                        SkRRect track;
                        track.setRectXY(SkRect::MakeXYWH(pb_x, pb_y, pb_w, pb_h), 3.0f, 3.0f);
                        SkPaint track_paint;
                        track_paint.setColor(0xFF334155);
                        sk_canvas->drawRRect(track, track_paint);

                        float fill_w = pb_w * std::clamp(cell.progress, 0.0f, 1.0f);
                        if (fill_w > 0) {
                            SkRRect fill_r;
                            fill_r.setRectXY(SkRect::MakeXYWH(pb_x, pb_y, fill_w, pb_h), 3.0f, 3.0f);
                            SkPaint fill_paint;
                            fill_paint.setColor(0xFF38BDF8);
                            sk_canvas->drawRRect(fill_r, fill_paint);
                        }

                        // Progress label
                        skia::textlayout::ParagraphStyle p_style;
                        auto b_plab = skia::textlayout::ParagraphBuilder::make(p_style, fc);
                        skia::textlayout::TextStyle t_plab;
                        t_plab.setFontSize(11.0f);
                        t_plab.setColor(static_cast<SkColor>(options.text_color));
                        b_plab->pushStyle(t_plab);
                        b_plab->addText(cell.display_text.c_str(), cell.display_text.length());
                        auto p_plab = b_plab->Build();
                        p_plab->layout(40.0f);
                        p_plab->paint(sk_canvas, pb_x + pb_w + 6.0f, cur_y + (options.row_height - p_plab->getHeight()) * 0.5f);
                    } else {
                        // Standard Text Render
                        skia::textlayout::ParagraphStyle p_style;
                        p_style.setTextAlign(col.align == DataGridAlign::Right ? skia::textlayout::TextAlign::kRight :
                                            (col.align == DataGridAlign::Center ? skia::textlayout::TextAlign::kCenter : skia::textlayout::TextAlign::kLeft));

                        auto b_cell = skia::textlayout::ParagraphBuilder::make(p_style, fc);
                        skia::textlayout::TextStyle t_cell;
                        t_cell.setFontSize(12.5f);
                        t_cell.setColor(static_cast<SkColor>(options.text_color));
                        b_cell->pushStyle(t_cell);
                        b_cell->addText(cell.display_text.c_str(), cell.display_text.length());
                        auto p_cell = b_cell->Build();
                        p_cell->layout(col.width - 16.0f);

                        float p_y = cur_y + (options.row_height - p_cell->getHeight()) * 0.5f;
                        p_cell->paint(sk_canvas, col_x + 8.0f, p_y);
                    }
                }

                // Vertical Column Divider
                SkPaint div_paint;
                div_paint.setColor(0x33334155);
                sk_canvas->drawLine(col_x + col.width, cur_y, col_x + col.width, cur_y + options.row_height, div_paint);

                col_x += col.width;
            }

            // Horizontal Row Divider
            SkPaint r_div;
            r_div.setColor(0x2B334155);
            sk_canvas->drawLine(ctx.offset.x, cur_y + options.row_height, ctx.offset.x + size_.width, cur_y + options.row_height, r_div);

            cur_y += options.row_height;
        }

        // 4. Summary Footer (if enabled)
        if (options.show_summary_footer && options.summary_calculator) {
            SkRect foot_rect = SkRect::MakeXYWH(ctx.offset.x, cur_y, size_.width, options.footer_height);
            SkPaint foot_bg;
            foot_bg.setColor(options.footer_bg_color);
            sk_canvas->drawRect(foot_rect, foot_bg);

            float col_x = ctx.offset.x + cb_w;
            for (const auto& col : columns) {
                if (!col.visible) continue;

                std::string summary_str = options.summary_calculator(col.key, raw_rows);
                if (!summary_str.empty()) {
                    skia::textlayout::ParagraphStyle p_style;
                    p_style.setTextAlign(col.align == DataGridAlign::Right ? skia::textlayout::TextAlign::kRight :
                                        (col.align == DataGridAlign::Center ? skia::textlayout::TextAlign::kCenter : skia::textlayout::TextAlign::kLeft));

                    auto b_sum = skia::textlayout::ParagraphBuilder::make(p_style, fc);
                    skia::textlayout::TextStyle t_sum;
                    t_sum.setFontSize(12.0f);
                    t_sum.setColor(0xFF38BDF8);
                    b_sum->pushStyle(t_sum);
                    b_sum->addText(summary_str.c_str(), summary_str.length());
                    auto p_sum = b_sum->Build();
                    p_sum->layout(col.width - 16.0f);

                    float p_y = cur_y + (options.footer_height - p_sum->getHeight()) * 0.5f;
                    p_sum->paint(sk_canvas, col_x + 8.0f, p_y);
                }
                col_x += col.width;
            }

            SkPaint f_div;
            f_div.setColor(options.border_color);
            sk_canvas->drawLine(ctx.offset.x, cur_y, ctx.offset.x + size_.width, cur_y, f_div);
        }

        sk_canvas->restore();

        // 5. Outer Border
        if (options.show_borders) {
            SkPaint border_paint;
            border_paint.setAntiAlias(true);
            border_paint.setStyle(SkPaint::kStroke_Style);
            border_paint.setStrokeWidth(1.0f);
            border_paint.setColor(options.border_color);
            sk_canvas->drawRRect(grid_rrect, border_paint);
        }
    }

    [[nodiscard]] bool hitTestSelf(Point localPoint) const override {
        return localPoint.x >= 0 && localPoint.x <= size_.width &&
               localPoint.y >= 0 && localPoint.y <= size_.height;
    }

    // Returns divider index (0..columns.size()-1) or -1
    int getHitDivider(float local_x, float local_y) const {
        if (local_y > options.header_height) return -1;
        float cur_x = getCheckboxWidth();
        const auto& columns = controller->getColumns();
        for (size_t i = 0; i < columns.size(); ++i) {
            if (!columns[i].visible) continue;
            cur_x += columns[i].width;
            if (std::abs(local_x - cur_x) <= 5.0f && columns[i].resizable) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

    // Returns column index clicked in header or -1
    int getHitColumnHeader(float local_x, float local_y) const {
        if (local_y > options.header_height) return -1;
        float cur_x = getCheckboxWidth();
        if (local_x < cur_x) return -99; // Checkbox header
        const auto& columns = controller->getColumns();
        for (size_t i = 0; i < columns.size(); ++i) {
            if (!columns[i].visible) continue;
            if (local_x >= cur_x && local_x < cur_x + columns[i].width) {
                return static_cast<int>(i);
            }
            cur_x += columns[i].width;
        }
        return -1;
    }

    // Returns visible row index or -1
    int getHitRowIndex(float local_y) const {
        if (local_y < options.header_height) return -1;
        float rel_y = local_y - options.header_height;
        int r = static_cast<int>(rel_y / options.row_height);
        int page_sz = controller->getPageSize();
        int page = controller->getCurrentPage();
        size_t start_idx = (page_sz > 0) ? static_cast<size_t>(page * page_sz) : 0;
        size_t end_idx = (page_sz > 0) ? std::min(start_idx + page_sz, controller->getFilteredIndices().size()) : controller->getFilteredIndices().size();
        size_t target_r = start_idx + r;
        if (target_r < end_idx) return static_cast<int>(target_r);
        return -1;
    }
};

static RenderDataGridBox* findDataGridBox(RenderObject* ro) {
    if (!ro) return nullptr;
    if (auto* box = dynamic_cast<RenderDataGridBox*>(ro)) return box;
    for (auto* child : ro->children()) {
        if (auto* found = findDataGridBox(child)) return found;
    }
    return nullptr;
}

class RenderDataGridWidget : public SingleChildRenderObjectWidget {
public:
    std::shared_ptr<DataGridController> controller;
    DataGridProps options;
    int hovered_row_index;
    int hovered_col_header;
    int hovered_col_divider;

    RenderDataGridWidget(std::shared_ptr<DataGridController> ctrl, DataGridProps opt,
                         int h_row, int h_hdr, int h_div)
        : SingleChildRenderObjectWidget(Key::none()), controller(std::move(ctrl)),
          options(std::move(opt)), hovered_row_index(h_row),
          hovered_col_header(h_hdr), hovered_col_divider(h_div) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext&) override {
        auto ro = std::make_unique<RenderDataGridBox>(controller, options);
        ro->hovered_row_index = hovered_row_index;
        ro->hovered_col_header = hovered_col_header;
        ro->hovered_col_divider = hovered_col_divider;
        return ro;
    }

    void updateRenderObject(BuildContext&, RenderObject& renderObject) override {
        if (auto* ro = dynamic_cast<RenderDataGridBox*>(&renderObject)) {
            ro->controller = controller;
            ro->options = options;
            ro->hovered_row_index = hovered_row_index;
            ro->hovered_col_header = hovered_col_header;
            ro->hovered_col_divider = hovered_col_divider;
            ro->updateFlexboxStyle();
            ro->markNeedsPaint();
        }
    }

    [[nodiscard]] std::string_view typeName() const override { return "RenderDataGridWidget"; }
};

// ════════════════════════════════════════════════════════════════
// DataGrid State Implementation
// ════════════════════════════════════════════════════════════════

class DataGridState : public State {
private:
    std::shared_ptr<DataGridController> controller_;
    int hovered_row_index_ = -1;
    int hovered_col_header_ = -1;
    int hovered_col_divider_ = -1;

    // Column Resizing Drag State
    bool is_resizing_column_ = false;
    int resize_col_idx_ = -1;
    float resize_start_x_ = 0.0f;
    float resize_start_w_ = 0.0f;

    SlotId key_down_conn_ = 0;

    void handleKey(int key, int mods) {
        bool ctrl = (mods & 2) != 0 || (mods & 4) != 0;

        // Ctrl+C: Copy Selected Rows as CSV
        if ((ctrl && (key == 'c' || key == 'C' || key == 0x63 || key == 0x43 || key == 54)) || (key == 0x03)) {
            if (Platform::instance()) {
                std::string csv_data = controller_->exportToCsv(true);
                if (!csv_data.empty()) {
                    ClipboardData data;
                    data.setText(csv_data);
                    Platform::instance()->setClipboardData(data);
                    Platform::instance()->setClipboardText(csv_data);
                }
            }
        } else if (ctrl && (key == 'a' || key == 'A' || key == 0x61 || key == 0x41 || key == 38)) {
            controller_->selectAll(true);
            setState([] {});
        }
    }

public:
    void initState() override {
        State::initState();
        auto* dg = static_cast<const DataGridWidget*>(widget());
        controller_ = dg->props.controller;
        if (!controller_) {
            controller_ = std::make_shared<DataGridController>();
        }

        if (Platform::instance()) {
            key_down_conn_ = Platform::instance()->onKeyDown().connect([this](int key, int mods) {
                handleKey(key, mods);
            });
        }
    }

    void didUpdateWidget(const Widget& old_widget) override {
        State::didUpdateWidget(old_widget);
        auto* dg = static_cast<const DataGridWidget*>(widget());
        controller_ = dg->props.controller;
        if (!controller_) {
            controller_ = std::make_shared<DataGridController>();
        }
    }

    void dispose() override {
        if (Platform::instance() && key_down_conn_) {
            Platform::instance()->onKeyDown().disconnect(key_down_conn_);
        }
        State::dispose();
    }

    WidgetPtr build(BuildContext&) override {
        auto* dg = static_cast<const DataGridWidget*>(widget());

        auto grid_render = std::make_shared<RenderDataGridWidget>(
            controller_, dg->props, hovered_row_index_,
            hovered_col_header_, hovered_col_divider_
        );

        auto detector = gestureDetector({
            .child = grid_render,
            .hit_test_behavior = HitTestBehavior::Opaque,
            .cursor_type = (hovered_col_divider_ >= 0 || is_resizing_column_) ?
                            SystemCursor::ResizeHorizontal : SystemCursor::Default,
            .on_tap_down = [this, dg](const TapDownDetails& e) {
                if (auto* ro = context().element()->findRenderObject()) {
                    if (auto* box = findDataGridBox(ro)) {
                        int div = box->getHitDivider(e.local_position.x, e.local_position.y);
                        if (div >= 0) return; // Ignore tap if starting divider resize

                        int col_hdr = box->getHitColumnHeader(e.local_position.x, e.local_position.y);
                        if (col_hdr == -99) { // Header select-all checkbox
                            controller_->selectAll(!controller_->isAllSelected());
                            if (dg->props.on_selection_changed) {
                                dg->props.on_selection_changed(controller_->getSelectedRowIds());
                            }
                            setState([] {});
                            return;
                        } else if (col_hdr >= 0) { // Header sort tap
                            const auto& cols = controller_->getColumns();
                            if (static_cast<size_t>(col_hdr) < cols.size() && cols[col_hdr].sortable) {
                                controller_->toggleSort(cols[col_hdr].key);
                                if (dg->props.on_sort_changed) {
                                    dg->props.on_sort_changed(cols[col_hdr].key, controller_->getSortDirection(cols[col_hdr].key));
                                }
                                setState([] {});
                                return;
                            }
                        }

                        int hit_r = box->getHitRowIndex(e.local_position.y);
                        if (hit_r >= 0) {
                            const auto& filtered = controller_->getFilteredIndices();
                            if (static_cast<size_t>(hit_r) < filtered.size()) {
                                const auto& row = controller_->getRawRows()[filtered[hit_r]];
                                if (dg->props.selection_mode == DataGridSelectionMode::RowMultiple) {
                                    controller_->toggleRowSelection(row.id);
                                } else if (dg->props.selection_mode == DataGridSelectionMode::RowSingle) {
                                    controller_->selectRow(row.id, true, true);
                                }
                                if (dg->props.on_selection_changed) {
                                    dg->props.on_selection_changed(controller_->getSelectedRowIds());
                                }
                                if (dg->props.on_row_tap) {
                                    dg->props.on_row_tap(row.id);
                                }
                                setState([] {});
                            }
                        }
                    }
                }
            },
            .on_pan_start = [this](const DragStartDetails& e) {
                if (auto* ro = context().element()->findRenderObject()) {
                    if (auto* box = findDataGridBox(ro)) {
                        int div = box->getHitDivider(e.local_position.x, e.local_position.y);
                        if (div >= 0) {
                            is_resizing_column_ = true;
                            resize_col_idx_ = div;
                            resize_start_x_ = e.global_position.x;
                            resize_start_w_ = controller_->getColumns()[div].width;
                            setState([] {});
                        }
                    }
                }
            },
            .on_pan_update = [this](const DragUpdateDetails& e) {
                if (!is_resizing_column_ || resize_col_idx_ < 0) return;
                float delta = e.global_position.x - resize_start_x_;
                controller_->setColumnWidth(resize_col_idx_, resize_start_w_ + delta);
                setState([] {});
            },
            .on_pan_end = [this](const DragEndDetails&) {
                if (is_resizing_column_) {
                    is_resizing_column_ = false;
                    resize_col_idx_ = -1;
                    setState([] {});
                }
            },
            .on_hover_exit = [this](const PointerEvent&) {
                setState([this] {
                    hovered_row_index_ = -1;
                    hovered_col_header_ = -1;
                    hovered_col_divider_ = -1;
                });
            },
            .on_hover_move = [this](const PointerEvent& e) {
                if (is_resizing_column_) return;
                if (auto* ro = context().element()->findRenderObject()) {
                    if (auto* box = findDataGridBox(ro)) {
                        int div = box->getHitDivider(e.localPosition.x, e.localPosition.y);
                        int col = box->getHitColumnHeader(e.localPosition.x, e.localPosition.y);
                        int row = box->getHitRowIndex(e.localPosition.y);

                        if (div != hovered_col_divider_ || col != hovered_col_header_ || row != hovered_row_index_) {
                            hovered_col_divider_ = div;
                            hovered_col_header_ = col;
                            hovered_row_index_ = row;
                            setState([] {});
                        }
                    }
                }
            },
        });

        std::vector<WidgetPtr> main_items = {detector};

        // ── Integrated Pagination Toolbar ─────────────────────────────
        if (dg->props.show_pagination && controller_->getTotalPages() > 1) {
            int cur_p = controller_->getCurrentPage();
            int tot_p = controller_->getTotalPages();
            size_t tot_items = controller_->getTotalFilteredCount();

            int start_item = cur_p * controller_->getPageSize() + 1;
            int end_item = std::min(static_cast<size_t>((cur_p + 1) * controller_->getPageSize()), tot_items);

            std::ostringstream ss_info;
            ss_info << "Showing " << start_item << "–" << end_item << " of " << tot_items << " entries";
            auto info_txt = text({
                .text = ss_info.str(),
                .color = 0xFF94A3B8,
                .font_size = 12.0f,
            });

            // First [⏮]
            auto btn_first = button(text("⏮"), [this] {
                controller_->setPage(0);
                setState([] {});
            });

            // Prev [◀]
            auto btn_prev = button(text("◀"), [this] {
                controller_->prevPage();
                setState([] {});
            });

            // Page label [ Page 1 of 5 ]
            std::ostringstream ss_page;
            ss_page << "Page " << (cur_p + 1) << " of " << tot_p;
            auto page_lbl = text({
                .text = ss_page.str(),
                .color = 0xFFF1F5F9,
                .font_size = 12.0f,
            });

            // Next [▶]
            auto btn_next = button(text("▶"), [this] {
                controller_->nextPage();
                setState([] {});
            });

            // Last [⏭]
            auto btn_last = button(text("⏭"), [this, tot_p] {
                controller_->setPage(tot_p - 1);
                setState([] {});
            });

            std::vector<WidgetPtr> nav_items = {btn_first, btn_prev, page_lbl, btn_next, btn_last};
            auto nav_row = row({
                .align_items = Align::Center,
                .gap = StyleValue::point(6.0f),
                .children = std::move(nav_items),
            });

            auto bar_row = row({
                .justify_content = Justify::SpaceBetween,
                .align_items = Align::Center,
                .children = { info_txt, nav_row },
            });

            auto pag_container = container({
                .color = 0xFF1E293B,
                .border_radius = BorderRadius::circular(6.0f),
                .border = Border(0xFF334155, 1.0f),
                .padding = StyleInsets::symmetric(6.0f, 10.0f),
                .child = bar_row,
            });

            main_items.push_back(pag_container);
        }

        auto full_col = column({
            .gap = StyleValue::point(8.0f),
            .children = std::move(main_items),
        });

        return full_col;
    }
};

std::unique_ptr<State> DataGridWidget::createState() {
    return std::make_unique<DataGridState>();
}

} // namespace enki
