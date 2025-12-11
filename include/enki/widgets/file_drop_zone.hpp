#pragma once
/// @file file_drop_zone.hpp
/// @brief FileDropZone widget for ENKI Framework.
/// An interactive drag-and-drop surface accepting file drops from the OS with animated dashed neon borders.

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/rendering/color.hpp"
#include <string>
#include <vector>
#include <functional>
#include <memory>

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
