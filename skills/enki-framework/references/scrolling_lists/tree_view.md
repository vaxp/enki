# TreeView

> A high-performance hierarchical tree view widget for desktop IDEs, file managers, scene graphs, and nested organizational structures, supporting arbitrary recursion depth, connector lines, multi-selection, and asynchronous lazy loading.

- **Header File**: `#include "enki/widgets/tree_view.hpp"`
- **C++ Class**: `enki::TreeViewWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::TreeView` (converts implicitly to `WidgetPtr`)
- **Props Struct**: `enki::TreeViewProps`
- **Node Descriptor**: `enki::TreeNodeData`
- **Visual Theme**: `enki::TreeViewTheme`

---

## Overview

`TreeView` is an essential component for complex desktop software (such as project file explorers, object inspectors, or settings trees). It manages expand/collapse state per node with smooth arrow rotations, draws optional tree connector branches (`show_lines = true`), supports single or multi-selection, and features an asynchronous callback hook (`on_children_requested`) for dynamically loading subtrees on demand (e.g. reading directories from disk).

---

## C++ API Definition

### Node Data Model (`TreeNodeData`)
```cpp
namespace enki {

struct TreeNodeData {
    std::string               id;                 ///< Unique ID across the tree
    WidgetPtr                 label;              ///< Displayed text/content widget
    WidgetPtr                 leading_icon;       ///< Optional icon (folder, file, etc.)
    WidgetPtr                 trailing;           ///< Optional trailing widget (badge, count)

    std::vector<TreeNodeData> children;           ///< Child subtrees (empty = leaf)

    bool                      initially_expanded = false;
    bool                      selectable         = true;
    bool                      checkable          = false;
    bool                      checked            = false;
    bool                      loading            = false;
    bool                      disabled           = false;
    void*                     user_data          = nullptr;

    TreeNodeData() = default;
    TreeNodeData(std::string id, WidgetPtr label, std::vector<TreeNodeData> children = {});
    TreeNodeData(std::string id, WidgetPtr icon, WidgetPtr label, std::vector<TreeNodeData> children = {});

    // Fluent Setters
    TreeNodeData& expand(bool v = true);
    TreeNodeData& withTrailing(WidgetPtr w);
    TreeNodeData& withCheck(bool v = false);
    TreeNodeData& disable(bool v = true);
    TreeNodeData& setLoading(bool v = true);
    TreeNodeData& setUserData(void* d);
};

} // namespace enki
```

### Visual Theme (`TreeViewTheme`)
```cpp
namespace enki {

struct TreeViewTheme {
    float        indent_width   = 20.0f;     ///< Pixels indented per depth tier
    float        node_height    = 32.0f;     ///< Row height

    Color        hover_color    = 0x0DFFFFFF;
    Color        selected_color = 0x1A2563EB;
    Color        focused_color  = 0x1A58A6FF;
    Color        line_color     = 0x33FFFFFF; ///< Hierarchy connector lines
    Color        arrow_color    = 0xFFAAAAAA;

    bool         show_lines     = false;     ///< Draws vertical/horizontal branching lines
    float        line_width     = 1.0f;
    float        arrow_size     = 12.0f;
    bool         animate_arrow  = true;

    BorderRadius row_radius     = BorderRadius::circular(4.0f);
};

} // namespace enki
```

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct TreeView {
    Key                                  key                   = Key::none();
    std::vector<TreeNodeData>            nodes;

    bool                                 multi_select          = false;
    std::optional<std::string>           selected_node_id;
    std::set<std::string>                selected_node_ids;

    bool                                 toggle_on_arrow       = true;
    bool                                 toggle_on_label       = false;

    TreeViewTheme                        tree_theme;
    ScrollPhysics                        scroll_physics        = ScrollPhysics::Clamped;

    std::function<void(const std::string& id)>                     on_node_expanded    = nullptr;
    std::function<void(const std::string& id)>                     on_node_collapsed   = nullptr;
    std::function<void(const std::string& id)>                     on_node_selected    = nullptr;
    std::function<void(const std::set<std::string>& ids)>          on_selection_changed= nullptr;
    std::function<void(const std::string& id, bool checked)>       on_node_checked     = nullptr;
    std::function<void(const std::string& id,
                       std::function<void(std::vector<TreeNodeData>)> resolve)>
                                                                   on_children_requested = nullptr;

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `nodes` | `std::vector<TreeNodeData>` | `{}` | Root-level tree nodes hierarchy. |
| `tree_theme` | `TreeViewTheme` | `{}` | Sizing, connector lines, and color styling. |
| `multi_select` | `bool` | `false` | Enables selecting multiple nodes with Ctrl/Cmd or checkboxes. |
| `toggle_on_arrow` | `bool` | `true` | Expanding triggers when clicking the chevron arrow. |
| `toggle_on_label` | `bool` | `false` | Expanding triggers when clicking anywhere on the node row. |
| `on_node_selected`| `Function(string)` | `nullptr` | Callback when user selects a node. |
| `on_children_requested`| `Function(id, resolve)`| `nullptr` | Async resolver for lazy loading subdirectories on click. |

---

## Code Examples (From `widgets_demo/tree_view_demo/main.cpp`)

### 1. IDE Project File Browser with Connector Lines
```cpp
#include "enki/widgets/tree_view.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildProjectTree() {
    std::vector<TreeNodeData> rootNodes = {
        TreeNodeData("src", text("📁"), text("src"), {
            TreeNodeData("main_cpp", text("📄"), text("main.cpp")),
            TreeNodeData("app_cpp",  text("📄"), text("app.cpp")),
        }).expand(),
        TreeNodeData("include", text("📁"), text("include"), {
            TreeNodeData("app_hpp", text("📄"), text("app.hpp")),
        }),
        TreeNodeData("build_cfg", text("⚙️"), text("meson.build")),
    };

    return TreeView {
        .nodes = std::move(rootNodes),
        .tree_theme = {
            .indent_width = 18.0f,
            .node_height = 28.0f,
            .show_lines = true, // Draw IDE connector lines
            .line_color = 0x26FFFFFF,
        },
        .on_node_selected = [](const std::string& id) {
            std::cout << "Opened file in editor: " << id << "\n";
        }
    };
}
```

### 2. Lazy Asynchronous Subtree Loading
```cpp
auto lazyTree = TreeView {
    .nodes = {
        TreeNodeData("remote_drive", text("☁️"), text("Cloud Drive"))
            .setLoading(false)
    },
    .on_children_requested = [](const std::string& parentId, auto resolve) {
        // Fetch remote children over network/filesystem asynchronously
        std::vector<TreeNodeData> fetchedChildren = {
            TreeNodeData(parentId + "/file1", text("📄"), text("RemoteDoc.pdf")),
            TreeNodeData(parentId + "/file2", text("📄"), text("Data.xlsx")),
        };
        resolve(fetchedChildren);
    }
};
```

---

## See Also
- [**ListView**](./list_view.md) — 1D flat list.
- [**ListTile**](./list_tile.md) — Standard list row component.
- [**ScrollView**](./scroll_view.md) — Base scrollable viewport.
