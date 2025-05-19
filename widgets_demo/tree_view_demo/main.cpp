/// @file main.cpp — TreeView Demo: File Browser simulation
#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/tree_view.hpp"
#include "enki/state/state.hpp"
#include <iostream>

using namespace enki;

static WidgetPtr folder(Color c = 0xFFF59E0B) {
    auto t = std::make_shared<Text>("📁");
    t->fontSize(14.0f);
    return t;
}
static WidgetPtr file_icon() {
    auto t = std::make_shared<Text>("📄");
    t->fontSize(14.0f);
    return t;
}

class TreeViewDemoState : public State {
    std::string last_action_ = "No action yet";

public:
    WidgetPtr build(BuildContext& ctx) override {
        // ── Build tree data ────────────────────────────────────
        std::vector<TreeNodeData> nodes = {
            TreeNodeData("home", folder(), std::make_shared<Text>("Home", TextStyle{.color=0xFFE2E8F0,.font_size=14.0f}), {
                TreeNodeData("docs", folder(), std::make_shared<Text>("Documents", TextStyle{.color=0xFFE2E8F0,.font_size=14.0f}), {
                    TreeNodeData("resume", file_icon(), std::make_shared<Text>("Resume.pdf", TextStyle{.color=0xFFB0C4D8,.font_size=13.0f})),
                    TreeNodeData("cover",  file_icon(), std::make_shared<Text>("CoverLetter.docx", TextStyle{.color=0xFFB0C4D8,.font_size=13.0f})),
                    TreeNodeData("notes",  file_icon(), std::make_shared<Text>("Notes.md", TextStyle{.color=0xFFB0C4D8,.font_size=13.0f})),
                }).expand(),
                TreeNodeData("pics", folder(0xFF2563EB), std::make_shared<Text>("Pictures", TextStyle{.color=0xFFE2E8F0,.font_size=14.0f}), {
                    TreeNodeData("vac", folder(), std::make_shared<Text>("Vacation 2024", TextStyle{.color=0xFFE2E8F0,.font_size=14.0f}), {
                        TreeNodeData("p1", file_icon(), std::make_shared<Text>("IMG_001.jpg", TextStyle{.color=0xFFB0C4D8,.font_size=13.0f})),
                        TreeNodeData("p2", file_icon(), std::make_shared<Text>("IMG_002.jpg", TextStyle{.color=0xFFB0C4D8,.font_size=13.0f})),
                    }),
                    TreeNodeData("pf", folder(), std::make_shared<Text>("Profile Photos", TextStyle{.color=0xFFE2E8F0,.font_size=14.0f})),
                }),
                TreeNodeData("dl", folder(0xFF10B981), std::make_shared<Text>("Downloads", TextStyle{.color=0xFFE2E8F0,.font_size=14.0f}), {
                    TreeNodeData("f1", file_icon(), std::make_shared<Text>("setup.exe", TextStyle{.color=0xFFB0C4D8,.font_size=13.0f})),
                    TreeNodeData("f2", file_icon(), std::make_shared<Text>("archive.zip", TextStyle{.color=0xFFB0C4D8,.font_size=13.0f})),
                }),
            }).expand(),
            TreeNodeData("dev", folder(0xFF8B5CF6), std::make_shared<Text>("Projects", TextStyle{.color=0xFFE2E8F0,.font_size=14.0f}), {
                TreeNodeData("enki_proj", folder(0xFF2563EB), std::make_shared<Text>("enki", TextStyle{.color=0xFFE2E8F0,.font_size=14.0f}), {
                    TreeNodeData("inc", folder(), std::make_shared<Text>("include", TextStyle{.color=0xFFE2E8F0,.font_size=14.0f})),
                    TreeNodeData("src2", folder(), std::make_shared<Text>("src", TextStyle{.color=0xFFE2E8F0,.font_size=14.0f})),
                    TreeNodeData("mb", file_icon(), std::make_shared<Text>("meson.build", TextStyle{.color=0xFFB0C4D8,.font_size=13.0f})),
                }),
                TreeNodeData("web_proj", folder(), std::make_shared<Text>("webapp", TextStyle{.color=0xFFE2E8F0,.font_size=14.0f})),
            }),
            TreeNodeData("trash", std::make_shared<Text>("🗑", TextStyle{.font_size=14.0f}),
                         std::make_shared<Text>("Trash", TextStyle{.color=0xFF8B9BB4,.font_size=14.0f}), {}).disable(),
        };

        // ── Build TreeView ─────────────────────────────────────
        auto tree = treeView(std::move(nodes));
        tree->showLines(true);
        tree->indentWidth(18.0f);
        tree->nodeHeight(30.0f);
        tree->onNodeSelected([this](const std::string& id){
            setState([this, id]{ last_action_ = "Selected: " + id; });
            std::cout << "[TreeView] Selected: " << id << "\n";
        });
        tree->onNodeExpanded([this](const std::string& id){
            setState([this, id]{ last_action_ = "Expanded: " + id; });
        });
        tree->onNodeCollapsed([this](const std::string& id){
            setState([this, id]{ last_action_ = "Collapsed: " + id; });
        });
        tree->paddingAll(8.0f);

        auto tree_flex = std::make_shared<FlexItem>(tree);
        tree_flex->flexGrow(1.0f).flexShrink(1.0f);

        // Header
        auto title = std::make_shared<Text>("TreeView Demo — File Browser");
        title->fontSize(22.0f).bold().color(0xFFFFFFFF);
        auto sub = std::make_shared<Text>("Expand/collapse · Selection · Connector lines · Disabled nodes");
        sub->fontSize(12.0f).color(0xFF8B9BB4);
        auto hdr_col = column({title, sub});
        hdr_col->gap(StyleValue::point(4.0f));
        auto hdr = container(hdr_col);
        hdr->padding(EdgeInsets::symmetric(14.0f, 18.0f));
        hdr->color(0xFF0D1117);
        hdr->width(StyleValue::percent(100.0f));

        // Status bar
        auto status = std::make_shared<Text>(last_action_);
        status->fontSize(12.0f).color(0xFF8B9BB4);
        auto status_bar = container(status);
        status_bar->padding(EdgeInsets::symmetric(7.0f, 18.0f));
        status_bar->color(0xFF161B22);
        status_bar->width(StyleValue::percent(100.0f));

        auto root_col = column({hdr, status_bar, tree_flex});
        root_col->width(StyleValue::percent(100.0f));
        root_col->height(StyleValue::percent(100.0f));

        auto root = container(root_col);
        root->color(0xFF0D1117);
        root->width(StyleValue::percent(100.0f));
        root->height(StyleValue::percent(100.0f));
        return root;
    }
};

class TreeViewDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override { return std::make_unique<TreeViewDemoState>(); }
    std::string_view typeName() const override { return "TreeViewDemoApp"; }
};

int main() {
    AppConfig cfg;
    cfg.title = "ENKI — TreeView Demo";
    cfg.width = 420; cfg.height = 680;
    cfg.resizable = true; cfg.vsync = false; cfg.target_fps = 0;
    cfg.show_performance_overlay = true;
    cfg.clear_color = 0xFF0D1117;
    return runApp(std::make_shared<TreeViewDemoApp>(), cfg);
}
