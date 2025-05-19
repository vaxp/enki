#include "enki/widgets/tree_view.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/scroll_view.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/text.hpp"
#include "enki/tree/build_context.hpp"
#include "enki/rendering/canvas.hpp"
#include "enki/state/state.hpp"
#include <map>
#include <string>

namespace enki {

// ════════════════════════════════════════════════════════════════
// RenderTreeConnectorLine — paints the vertical connector line
// ════════════════════════════════════════════════════════════════

class RenderTreeLine : public RenderBox {
public:
    Color color;
    float line_width;
    RenderTreeLine(Color c, float lw, float indent_w) : color(c), line_width(lw) {
        ANUNodeStyleSetWidth(anu_node_, indent_w);
        ANUNodeStyleSetHeightPercent(anu_node_, 100.0f);
    }
    void paint(PaintContext& ctx) override {
        if (size_.height <= 0) return;
        float cx = ctx.offset.x + size_.width * 0.5f;
        Paint p;
        p.setColor(color);
        p.setStrokeWidth(line_width);
        ctx.canvas.drawLine({cx, ctx.offset.y}, {cx, ctx.offset.y + size_.height}, p);
    }
};

class TreeLineWidget : public SingleChildRenderObjectWidget {
public:
    Color color; float lw; float indent_w;
    TreeLineWidget(Color c, float lw, float iw) : color(c), lw(lw), indent_w(iw) {}
    std::unique_ptr<RenderObject> createRenderObject(BuildContext&) override {
        return std::make_unique<RenderTreeLine>(color, lw, indent_w);
    }
    void updateRenderObject(BuildContext&, RenderObject& ro) override {}
    std::string_view typeName() const override { return "TreeLineWidget"; }
};

// ════════════════════════════════════════════════════════════════
// TreeViewState
// ════════════════════════════════════════════════════════════════

class TreeViewState : public State {
    std::set<std::string> expanded_;
    std::set<std::string> selected_;

    void initState() override {
        State::initState();
        auto* w = static_cast<const TreeView*>(widget());
        // Init expanded state from TreeNodeData.initially_expanded
        initExpandedState(w->nodes);
        // Init selection
        if (w->selected_node_id.has_value())
            selected_.insert(*w->selected_node_id);
        selected_.insert(w->selected_node_ids.begin(), w->selected_node_ids.end());
    }

    void initExpandedState(const std::vector<TreeNodeData>& nodes) {
        for (auto& n : nodes) {
            if (n.initially_expanded) expanded_.insert(n.id);
            initExpandedState(n.children);
        }
    }

    WidgetPtr buildNode(const TreeNodeData& data, int depth, const TreeViewTheme& theme, const TreeView* w) {
        bool is_expanded = expanded_.count(data.id) > 0;
        bool is_selected = selected_.count(data.id) > 0;
        bool is_leaf     = data.children.empty() && !data.loading;

        // ── Row content ────────────────────────────────────────
        std::vector<WidgetPtr> row_children;

        // Indent spacers — one per depth level
        for (int i = 0; i < depth; ++i) {
            auto sp = container();
            sp->width(theme.indent_width);
            sp->height(StyleValue::percent(100.0f));
            row_children.push_back(sp);
        }

        // Arrow / toggle button
        {
            WidgetPtr arrow;
            if (!is_leaf || data.loading) {
                // Build a small rotatable arrow via container with text symbol
                auto arrow_sym = std::make_shared<Text>(is_expanded ? "▾" : "▸");
                arrow_sym->fontSize(theme.arrow_size)
                          .color(data.disabled ? theme.disabled_color : theme.arrow_color);

                auto arrow_wrap = container(arrow_sym);
                arrow_wrap->width(theme.indent_width);
                arrow_wrap->height(theme.node_height);
                arrow_wrap->align(Alignment::Center);

                if (!data.disabled && theme.animate_arrow) {
                    auto node_id = data.id;
                    auto det = std::make_shared<GestureDetector>(arrow_wrap);
                    det->hit_test_behavior = HitTestBehavior::Opaque;
                    det->cursor_type = SystemCursor::Pointer;
                    det->on_tap = [this, node_id, w]() {
                        bool expanding = expanded_.count(node_id) == 0;
                        setState([this, node_id, expanding]() {
                            if (expanding) expanded_.insert(node_id);
                            else           expanded_.erase(node_id);
                        });
                        if (expanding && w->on_node_expanded)   w->on_node_expanded(node_id);
                        if (!expanding && w->on_node_collapsed) w->on_node_collapsed(node_id);
                    };
                    arrow = det;
                } else {
                    arrow = arrow_wrap;
                }
            } else {
                // Leaf: empty space to align with non-leaf arrows
                auto sp = container();
                sp->width(theme.indent_width);
                sp->height(theme.node_height);
                arrow = sp;
            }
            row_children.push_back(arrow);
        }

        // Leading icon
        if (data.leading_icon) {
            auto c = container(data.leading_icon);
            c->margin(EdgeInsets::only(0, theme.leading_gap, 0, 0));
            row_children.push_back(c);
        }

        // Label (flex grow)
        auto label_item = std::make_shared<FlexItem>(data.label);
        label_item->flexGrow(1.0f).flexShrink(1.0f);
        row_children.push_back(label_item);

        // Trailing
        if (data.trailing) {
            auto c = container(data.trailing);
            c->margin(EdgeInsets::only(0, 0, 0, theme.trailing_gap));
            row_children.push_back(c);
        }

        // ── Build the row ──────────────────────────────────────
        auto row_content = std::make_shared<Row>(std::move(row_children));
        row_content->alignItems(Align::Center);
        row_content->height(theme.node_height);
        row_content->width(StyleValue::percent(100.0f));

        // ── Wrap in selection/hover decoration ─────────────────
        auto row_bg = container(row_content);
        row_bg->height(theme.node_height);
        row_bg->width(StyleValue::percent(100.0f));
        row_bg->padding(EdgeInsets::symmetric(theme.row_padding_vertical, theme.row_padding_horizontal));
        row_bg->borderRadius(theme.row_radius.top_left);

        if (is_selected) {
            row_bg->color(theme.selected_color);
        }

        WidgetPtr row_widget;
        if (!data.disabled) {
            auto node_id = data.id;
            auto det = std::make_shared<GestureDetector>(row_bg);
            det->hit_test_behavior = HitTestBehavior::Opaque;
            det->cursor_type = SystemCursor::Pointer;
            det->on_tap = [this, node_id, w]() {
                // Toggle expand on label click if configured
                if (w->toggle_on_label && expanded_.count(node_id) == 0) {
                    setState([this, node_id]{ expanded_.insert(node_id); });
                    if (w->on_node_expanded) w->on_node_expanded(node_id);
                } else if (w->toggle_on_label) {
                    setState([this, node_id]{ expanded_.erase(node_id); });
                    if (w->on_node_collapsed) w->on_node_collapsed(node_id);
                }
                // Selection
                setState([this, node_id, w]() {
                    if (w->multi_select) {
                        if (selected_.count(node_id)) selected_.erase(node_id);
                        else selected_.insert(node_id);
                        if (w->on_selection_changed) w->on_selection_changed(selected_);
                    } else {
                        selected_.clear();
                        selected_.insert(node_id);
                        if (w->on_node_selected) w->on_node_selected(node_id);
                    }
                });
            };
            if (w->on_node_context_menu) {
                auto ctx_cb = w->on_node_context_menu;
                det->on_secondary_tap = [ctx_cb, node_id]() { ctx_cb(node_id, {0,0}); };
            }
            row_widget = det;
        } else {
            row_widget = row_bg;
        }

        // ── Assemble node (row + expanded children) ────────────
        std::vector<WidgetPtr> node_col_children;
        node_col_children.push_back(row_widget);

        if (is_expanded && !data.children.empty()) {
            for (auto& child : data.children) {
                node_col_children.push_back(buildNode(child, depth + 1, theme, w));
            }
        }

        auto node_col = std::make_shared<Column>(std::move(node_col_children));
        node_col->width(StyleValue::percent(100.0f));
        return node_col;
    }

public:
    WidgetPtr build(BuildContext& ctx) override {
        auto* w = static_cast<const TreeView*>(widget());

        std::vector<WidgetPtr> all_nodes;
        for (auto& n : w->nodes)
            all_nodes.push_back(buildNode(n, 0, w->tree_theme, w));

        auto root_col = std::make_shared<Column>(std::move(all_nodes));
        root_col->width(StyleValue::percent(100.0f));
        root_col->flexShrink(0.0f);

        WidgetPtr content;
        if (w->list_padding != EdgeInsets{}) {
            auto pc = container(root_col);
            pc->padding(w->list_padding);
            content = pc;
        } else {
            content = root_col;
        }

        ScrollOptions opts;
        opts.direction = Axis::Vertical;
        opts.scroll_speed = w->scroll_speed;
        opts.show_scrollbar = true;
        opts.clamp_overscroll = (w->scroll_physics == ScrollPhysics::Clamped);
        return scrollView(opts, content);
    }
};

std::unique_ptr<State> TreeView::createState() {
    return std::make_unique<TreeViewState>();
}

} // namespace enki
