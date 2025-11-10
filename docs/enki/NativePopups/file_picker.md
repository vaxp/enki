# FilePicker

> An advanced native desktop filesystem dialog widget built on NativePopup, supporting real-time file and folder browsing, extension filtering, quick bookmarks, multi-selection, and file save workflows.

- **Header File**: `#include "enki/widgets/file_picker.hpp"`
- **C++ Class**: `enki::FilePickerWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::FilePicker` (converts implicitly to `WidgetPtr`)
- **Props Struct**: `enki::FilePickerProps`
- **Result Payload**: `enki::FilePickerResult`
- **Filter Model**: `enki::FileFilter`
- **Modes Enum**: `enki::FilePickerMode` (`OpenFile`, `OpenMultipleFiles`, `SelectFolder`, `SaveFile`)
- **Static Launcher**: `FilePicker::show(ctx, file_picker)`

---

## Overview

`FilePicker` spawns a standalone desktop modal window (`NativePopup`) providing an interactive graphical filesystem navigator. It supports real-time directory tree traversal, quick-access location bookmarks, file extension filters (e.g. `*.png`, `*.json`), filename search, and file creation for save workflows.

---

## C++ API Definition

### Modes & Data Models
```cpp
namespace enki {

enum class FilePickerMode {
    OpenFile,           ///< Select a single existing file
    OpenMultipleFiles,  ///< Select one or more existing files
    SelectFolder,       ///< Select a directory folder
    SaveFile            ///< Specify a file name and directory destination to write
};

struct FileFilter {
    std::string              name;        ///< Human label (e.g. "Images (*.png, *.jpg)")
    std::vector<std::string> extensions;  ///< Extensions without dot (e.g. {"png", "jpg"})

    FileFilter(std::string name = "All Files (*.*)", std::vector<std::string> extensions = {});
};

struct FilePickerResult {
    bool                     canceled = true;    ///< True if dialog was closed without choosing
    std::vector<std::string> selected_paths;     ///< Absolute filesystem paths selected
};

} // namespace enki
```

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct FilePicker {
    WidgetPtr                                   child             = nullptr;
    std::function<void(const FilePickerResult&)> on_result        = nullptr;

    FilePickerMode                              mode              = FilePickerMode::OpenFile;
    std::string                                 initial_directory = "";
    std::string                                 default_filename  = "";
    std::vector<FileFilter>                     filters;

    Size                                        window_size       = Size{740.0f, 520.0f};

    Color                                       background_color  = 0xFA1F242C;
    Color                                       sidebar_color     = 0xFA161B22;
    Color                                       border_color      = 0xFF363B42;
    Color                                       accent_color      = 0xFF38BDF8;

    float                                       border_radius     = 12.0f;
    float                                       elevation         = 16.0f;

    static std::shared_ptr<NativePopup> show(BuildContext& context, const FilePicker& fp);

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `child` | `WidgetPtr` | `nullptr` | Trigger button or widget clicked to open the picker. |
| `on_result` | `Function(FilePickerResult)`| `nullptr` | Callback receiving selected paths or cancel state. |
| `mode` | `FilePickerMode` | `OpenFile` | Operation mode (`OpenFile`, `OpenMultipleFiles`, `SelectFolder`, `SaveFile`). |
| `initial_directory` | `std::string` | `""` | Starting filesystem directory (defaults to user's `$HOME`). |
| `default_filename` | `std::string` | `""` | Pre-populated filename in `SaveFile` mode. |
| `filters` | `std::vector<FileFilter>` | `{}` | File extension filters displayed in the dropdown. |
| `window_size` | `Size` | `{740, 520}` | Window size of the native popup dialog. |

---

## Code Examples (From `widgets_demo/file_picker_demo/main.cpp`)

### 1. Open File Dialog with Image Filters
```cpp
#include "enki/widgets/file_picker.hpp"
#include "enki/widgets/button.hpp"

using namespace enki;

WidgetPtr buildImageImportButton(auto onImageSelected) {
    return FilePicker {
        .child = button(text("📂 Import Image..."), nullptr),
        .on_result = [onImageSelected](const FilePickerResult& res) {
            if (!res.canceled && !res.selected_paths.empty()) {
                onImageSelected(res.selected_paths[0]);
            }
        },
        .mode = FilePickerMode::OpenFile,
        .filters = {
            FileFilter("Image Files (*.png, *.jpg)", {"png", "jpg", "jpeg"}),
            FileFilter("All Files (*.*)", {})
        }
    };
}
```

### 2. Save File Dialog
```cpp
auto exportButton = FilePicker {
    .child = button(text("💾 Export Project..."), nullptr),
    .on_result = [](const FilePickerResult& res) {
        if (!res.canceled && !res.selected_paths.empty()) {
            std::cout << "Saving project to: " << res.selected_paths[0] << "\n";
        }
    },
    .mode = FilePickerMode::SaveFile,
    .default_filename = "project_manifest.json",
    .filters = {
        FileFilter("JSON Configuration (*.json)", {"json"})
    }
};
```

### 3. Imperative Launcher (`FilePicker::show`)
```cpp
void promptSaveLocation(BuildContext& ctx) {
    FilePicker::show(ctx, FilePicker {
        .mode = FilePickerMode::SelectFolder,
        .on_result = [](const FilePickerResult& res) {
            if (!res.canceled) {
                std::cout << "Selected output directory: " << res.selected_paths[0] << "\n";
            }
        }
    });
}
```

---

## See Also
- [**Popup**](./popup.md) — Base native floating surface engine.
- [**Dialog**](../Overlays/dialog.md) — In-tree modal dialogs.
