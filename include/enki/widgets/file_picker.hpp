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
class FilePicker : public StatefulWidget {
public:
    FilePickerProps props;

    FilePicker(FilePickerProps p)
        : props(std::move(p)) {}

    [[nodiscard]] std::unique_ptr<State> createState() override;
    [[nodiscard]] std::string_view typeName() const override { return "FilePicker"; }

    /// Static launcher to open FilePicker dialog programmatically from anywhere.
    static std::shared_ptr<NativePopup> show(
        BuildContext& context,
        FilePickerProps props
    );
};

// ── Factory Helpers ────────────────────────────────────────────────

inline WidgetPtr filePicker(FilePickerProps props) {
    return std::make_shared<FilePicker>(std::move(props));
}

} // namespace enki
