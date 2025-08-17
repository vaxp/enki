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
            TreeNodeData("home", folder(), text("Home", { .color = 0xFFE2E8F0, .font_size = 14.0f }), {
                TreeNodeData("docs", folder(), text("Documents", { .color = 0xFFE2E8F0, .font_size = 14.0f }), {
                    TreeNodeData("resume", file_icon(), text("Resume.pdf", { .color = 0xFFB0C4D8, .font_size = 13.0f })),
                    TreeNodeData("cover",  file_icon(), text("CoverLetter.docx", { .color = 0xFFB0C4D8, .font_size = 13.0f })),
                    TreeNodeData("notes",  file_icon(), text("Notes.md", { .color = 0xFFB0C4D8, .font_size = 13.0f })),
                }).expand(),
                TreeNodeData("pics", folder(0xFF2563EB), text("Pictures", { .color = 0xFFE2E8F0, .font_size = 14.0f }), {
                    TreeNodeData("vac", folder(), text("Vacation 2024", { .color = 0xFFE2E8F0, .font_size = 14.0f }), {
                        TreeNodeData("p1", file_icon(), text("IMG_001.jpg", { .color = 0xFFB0C4D8, .font_size = 13.0f })),
                        TreeNodeData("p2", file_icon(), text("IMG_002.jpg", { .color = 0xFFB0C4D8, .font_size = 13.0f })),
                    }),
                    TreeNodeData("pf", folder(), text("Profile Photos", { .color = 0xFFE2E8F0, .font_size = 14.0f })),
                }),
                TreeNodeData("dl", folder(0xFF10B981), text("Downloads", { .color = 0xFFE2E8F0, .font_size = 14.0f }), {
                    TreeNodeData("f1", file_icon(), text("setup.exe", { .color = 0xFFB0C4D8, .font_size = 13.0f })),
                    TreeNodeData("f2", file_icon(), text("archive.zip", { .color = 0xFFB0C4D8, .font_size = 13.0f })),
                }),
            }).expand(),
            TreeNodeData("dev", folder(0xFF8B5CF6), text("Projects", { .color = 0xFFE2E8F0, .font_size = 14.0f }), {
                TreeNodeData("enki_proj", folder(0xFF2563EB), text("enki", { .color = 0xFFE2E8F0, .font_size = 14.0f }), {
                    TreeNodeData("inc", folder(), text("include", { .color = 0xFFE2E8F0, .font_size = 14.0f })),
                    TreeNodeData("src2", folder(), text("src", { .color = 0xFFE2E8F0, .font_size = 14.0f })),
                    TreeNodeData("mb", file_icon(), text("meson.build", { .color = 0xFFB0C4D8, .font_size = 13.0f })),
                }),
                TreeNodeData("web_proj", folder(), text("webapp", { .color = 0xFFE2E8F0, .font_size = 14.0f })),
            }),
            TreeNodeData("trash", text("🗑", { .font_size = 14.0f }),
                         text("Trash", { .color = 0xFF8B9BB4, .font_size = 14.0f }), {}).disable(),
        };

        // ── Build TreeView ─────────────────────────────────────
        return container({
            .color = 0xFF0D1117,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .child = column({
                .children = {
                    container({
                        .color = 0xFF0D1117,
                        .width = StyleValue::percent(100.0f),
                        .padding = StyleInsets::symmetric(14.0f, 18.0f),
                        .child = column({
                            .gap = StyleValue::point(4.0f),
                            .children = {
                                text("TreeView Demo — File Browser", { .color = 0xFFFFFFFF, .font_size = 22.0f, .font_weight = FontWeight::Bold }),
                                text("Expand/collapse · Selection · Connector lines · Disabled nodes", { .color = 0xFF8B9BB4, .font_size = 12.0f })
                            }
                        })
                    }),
                    container({
                        .color = 0xFF161B22,
                        .width = StyleValue::percent(100.0f),
                        .padding = StyleInsets::symmetric(7.0f, 18.0f),
                        .child = text(last_action_, { .color = 0xFF8B9BB4, .font_size = 12.0f })
                    }),
                    flexItem(
                        { .flex_grow = 1.0f, .flex_shrink = 1.0f },
                        container({
                            .padding = StyleInsets::all(8.0f),
                            .child = treeView({
                                .nodes = std::move(nodes),
                                .tree_theme = {
                                    .indent_width = 18.0f,
                                    .node_height = 30.0f,
                                    .show_lines = true
                                },
                                .on_node_expanded = [this](const std::string& id) {
                                    setState([this, id]{ last_action_ = "Expanded: " + id; });
                                },
                                .on_node_collapsed = [this](const std::string& id) {
                                    setState([this, id]{ last_action_ = "Collapsed: " + id; });
                                },
                                .on_node_selected = [this](const std::string& id) {
                                    setState([this, id]{ last_action_ = "Selected: " + id; });
                                    std::cout << "[TreeView] Selected: " << id << "\\n";
                                }
                            })
                        })
                    )
                }
            })
        });
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
