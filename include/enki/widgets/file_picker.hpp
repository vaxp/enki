#pragma once
/// @file file_picker.hpp
/// @brief Advanced Native FilePicker dialog widget built on NativePopup.
///
/// FilePicker spawns a standalone desktop modal window (NativePopup) allowing
/// real-time system filesystem browsing, file/folder selection, extension filtering,
/// quick location bookmarks, and file save dialogs.
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/rendering/color.hpp"
#include "enki/shell/native_popup.hpp"
#include "enki/shell/shell_types.hpp"

#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace enki {

/// Operation mode for FilePicker dialog
enum class FilePickerMode {
    OpenFile,
    OpenMultipleFiles,
    SelectFolder,
    SaveFile
};

/// File extension filter definition
struct FileFilter {
    std::string name;                          ///< Human-readable label (e.g., "Images (*.png, *.jpg)")
    std::vector<std::string> extensions;       ///< List of extensions without dot (e.g. {"png", "jpg", "jpeg"})

    FileFilter(std::string name = "All Files (*.*)", std::vector<std::string> extensions = {})
        : name(std::move(name)), extensions(std::move(extensions)) {}
};

/// Result payload returned from FilePicker completion
struct FilePickerResult {
    bool canceled = true;                      ///< True if closed without selection
    std::vector<std::string> selected_paths;   ///< Absolute paths of selected files/folders
};

/// Configuration options for FilePicker styling and behavior
struct FilePickerProps {
    WidgetPtr child = nullptr;
    std::function<void(const FilePickerResult&)> on_result = nullptr;

    FilePickerMode mode           = FilePickerMode::OpenFile;
    std::string initial_directory = "";        ///< Initial directory path (defaults to $HOME)
    std::string default_filename  = "";        ///< Default filename for SaveFile mode

    std::vector<FileFilter> filters;           ///< Allowed file extension filters

    Size window_size              = Size{740.0f, 520.0f};

    Color background_color        = 0xFA1F242C; ///< ARGB main panel background
    Color sidebar_color           = 0xFA161B22; ///< ARGB quick access sidebar background
    Color border_color            = 0xFF363B42; ///< Outer border stroke color
    Color accent_color            = 0xFF38BDF8; ///< Primary selection & action button color
    Color text_color              = 0xFFF0F6FC; ///< Primary text color
    Color subtext_color           = 0xFF8B949E; ///< Secondary text / metadata color

    float border_radius           = 12.0f;
    float elevation               = 16.0f;
    Color shadow_color            = 0x60000000;
};

/// @brief FilePicker widget wrapping a target child widget or static show helper.
class FilePickerWidget : public StatefulWidget {
public:
    FilePickerProps props;

    FilePickerWidget() = default;
    FilePickerWidget(FilePickerProps p)
        : props(std::move(p)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "FilePicker"; }

    /// Static launcher to open FilePicker dialog programmatically from anywhere.
    static std::shared_ptr<NativePopup> show(
        BuildContext& context,
        FilePickerProps props
    );
};

// ── Declarative Proxy Struct (C++20 Designated Initializers) ───────────────

struct FilePicker {
    WidgetPtr child = nullptr;
    std::function<void(const FilePickerResult&)> on_result = nullptr;
    FilePickerMode mode = FilePickerMode::OpenFile;
    std::string initial_directory = "";
    std::string default_filename = "";
    std::vector<FileFilter> filters;
    Size window_size = Size{740.0f, 520.0f};
    Color background_color = 0xFA1F242C;
    Color sidebar_color = 0xFA161B22;
    Color border_color = 0xFF363B42;
    Color accent_color = 0xFF38BDF8;
    Color text_color = 0xFFF0F6FC;
    Color subtext_color = 0xFF8B949E;
    float border_radius = 12.0f;
    float elevation = 16.0f;
    Color shadow_color = 0x60000000;

    operator WidgetPtr() const {
        FilePickerProps p;
        p.child = child;
        p.on_result = on_result;
        p.mode = mode;
        p.initial_directory = initial_directory;
        p.default_filename = default_filename;
        p.filters = filters;
        p.window_size = window_size;
        p.background_color = background_color;
        p.sidebar_color = sidebar_color;
        p.border_color = border_color;
        p.accent_color = accent_color;
        p.text_color = text_color;
        p.subtext_color = subtext_color;
        p.border_radius = border_radius;
        p.elevation = elevation;
        p.shadow_color = shadow_color;
        return std::make_shared<FilePickerWidget>(std::move(p));
    }
    
    static std::shared_ptr<NativePopup> show(BuildContext& context, const FilePicker& fp) {
        FilePickerProps p;
        p.child = fp.child;
        p.on_result = fp.on_result;
        p.mode = fp.mode;
        p.initial_directory = fp.initial_directory;
        p.default_filename = fp.default_filename;
        p.filters = fp.filters;
        p.window_size = fp.window_size;
        p.background_color = fp.background_color;
        p.sidebar_color = fp.sidebar_color;
        p.border_color = fp.border_color;
        p.accent_color = fp.accent_color;
        p.text_color = fp.text_color;
        p.subtext_color = fp.subtext_color;
        p.border_radius = fp.border_radius;
        p.elevation = fp.elevation;
        p.shadow_color = fp.shadow_color;
        return FilePickerWidget::show(context, std::move(p));
    }
};

} // namespace enki
