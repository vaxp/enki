# FileDropZone

> An interactive drag-and-drop surface accepting file drops from the OS with animated dashed neon borders.

- **Header File**: `#include "enki/widgets/file_drop_zone.hpp"`
- **C++ Class**: `enki::FileDropZoneWidget` (inherits from `enki::SingleChildRenderObjectWidget`)
- **Declarative Helper**: `enki::fileDropZone(FileDropZoneProps props)` (returns `enki::WidgetPtr`)
- **Render Object**: `enki::RenderFileDropZone`
- **Underlying Mechanism**: Directly hooks into native Wayland (`wl_data_device`) and X11 (`XDnD`) protocols, reading `DataOffer` URIs with real-time conveyor dashed Skia path animation.

---

## Overview

`FileDropZone` is a high-performance drag-and-drop area for desktop applications. When files are dragged from desktop managers (such as the VAXP-OS file manager) over the window, the widget lights up its border, animates an orbiting conveyor dash pattern, parses the incoming `text/uri-list` payload into real file paths, validates file extensions, and invokes `on_files_dropped`.

---

## C++ API Definition

### Struct Definition (`enki/widgets/file_drop_zone.hpp`)
```cpp
namespace enki {

struct FileDropZoneProps {
    std::vector<std::string>                    allowed_extensions; // empty = allow all
    std::string                                 prompt_text = "Drag & Drop files here";
    std::string                                 sub_text = "or click to select from files";
    std::string                                 icon = "📥";
    WidgetPtr                                   child = nullptr;

    float                                       width = 360.0f;
    float                                       height = 160.0f;
    float                                       border_radius = 14.0f;

    Color                                       idle_border_color = 0x4D00E5FF;
    Color                                       hover_border_color = 0xFF00E5FF;
    Color                                       idle_background = 0x400A101D;
    Color                                       hover_background = 0x3300E5FF;
    Color                                       text_color = 0xFFE2E8F0;

    std::function<void(const std::vector<std::string>&)> on_files_dropped;

    operator WidgetPtr() const;
};

class FileDropZoneWidget : public SingleChildRenderObjectWidget {
public:
    FileDropZoneProps props;

    explicit FileDropZoneWidget(FileDropZoneProps p)
        : SingleChildRenderObjectWidget(Key::none(), nullptr), props(std::move(p)) {}

    [[nodiscard]] std::string_view typeName() const override { return "FileDropZone"; }
    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
};

inline WidgetPtr fileDropZone(FileDropZoneProps props) {
    return std::make_shared<FileDropZoneWidget>(std::move(props));
}

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `allowed_extensions` | `std::vector<std::string>` | `{}` | Allowed file suffixes (e.g. `{".mp4", ".png"}`). Empty allows any file. |
| `prompt_text` | `std::string` | `"Drag & Drop files here"` | Main headline text centered in the drop zone. |
| `sub_text` | `std::string` | `"or click to select from files"` | Secondary hint or file format description. |
| `icon` | `std::string` | `"📥"` | Glyph or emoji rendered above the text. |
| `child` | `WidgetPtr` | `nullptr` | Optional custom widget content inside the drop area. |
| `width` | `float` | `360.0f` | Box width in pixels. |
| `height` | `float` | `160.0f` | Box height in pixels. |
| `border_radius` | `float` | `14.0f` | Corner rounding radius of the drop card. |
| `idle_border_color` | `Color` | `0x4D00E5FF` | Dashed border color when resting. |
| `hover_border_color`| `Color` | `0xFF00E5FF` | Glowing border color when dragging over. |
| `idle_background` | `Color` | `0x400A101D` | Surface fill color when resting. |
| `hover_background`| `Color` | `0x3300E5FF` | Glowing background tint when dragging over. |
| `text_color` | `Color` | `0xFFE2E8F0` | Primary prompt text color. |
| `on_files_dropped` | `std::function<void(...)>` | `nullptr` | Callback delivered with full absolute paths of dropped files. |

---

## Code Examples (From `widgets_demo/file_drop_zone_demo/main.cpp`)

### 1. Real Desktop File Drop Surface (Native Wayland / X11)
```cpp
auto dz = fileDropZone({
    .prompt_text = "Drag & Drop Any Files Here",
    .sub_text = "Accepts real files dragged from desktop or file manager",
    .width = 560.0f,
    .height = 180.0f,
    .on_files_dropped = [this](const std::vector<std::string>& files) {
        std::cout << "\n==================================================" << std::endl;
        std::cout << ">>> [FileDropZone] REAL FILE DROP RECEIVED (" << files.size() << " files):" << std::endl;
        for (size_t i = 0; i < files.size(); ++i) {
            std::cout << "    [" << (i + 1) << "] " << files[i] << std::endl;
            dropped_files_.push_back(files[i]);
        }
        std::cout << "==================================================\n" << std::endl;
        setState([]{});
    },
});
```

### 2. Media Filtered Drop Area
```cpp
auto mediaDropZone = fileDropZone({
    .allowed_extensions = {".mp4", ".mkv", ".wav", ".png", ".flac"},
    .prompt_text = "Drop Media Files Here",
    .sub_text = "Supported formats: MP4, MKV, WAV, PNG, FLAC",
    .width = 480.0f,
    .height = 150.0f,
    .on_files_dropped = [](const std::vector<std::string>& media_files) {
        // Load media files into pipeline
    },
});
```
