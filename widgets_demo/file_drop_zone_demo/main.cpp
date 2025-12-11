#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/file_drop_zone.hpp"
#include "enki/state/state.hpp"
#include <iostream>

using namespace enki;

class FileDropZoneDemoState : public State {
    std::vector<std::string> dropped_files_;

public:
    WidgetPtr build(BuildContext&) override {
        auto title = text("FileDropZone Interactive Demo", {
            .color = 0xFFFFFFFF,
            .font_size = 22.0f,
            .font_weight = FontWeight::Bold
        });
        auto subtitle = text("Native Wayland (wl_data_device) & X11 (XDnD) Real File Ingestion", {
            .color = 0xFF00E5FF,
            .font_size = 13.0f,
            .font_weight = FontWeight::Medium
        });

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

        std::vector<WidgetPtr> file_entries;
        if (dropped_files_.empty()) {
            file_entries.push_back(text("Status: Waiting for file drop... Drag any file from desktop/file manager into the zone.", {
                .color = 0xFF94A3B8,
                .font_size = 13.0f,
            }));
        } else {
            file_entries.push_back(text("Received Files (" + std::to_string(dropped_files_.size()) + "):", {
                .color = 0xFF10B981,
                .font_size = 14.0f,
                .font_weight = FontWeight::Bold
            }));
            for (size_t i = 0; i < dropped_files_.size(); ++i) {
                file_entries.push_back(text(std::to_string(i + 1) + ". " + dropped_files_[i], {
                    .color = 0xFF38BDF8,
                    .font_size = 12.5f,
                }));
            }
        }

        auto files_col = column(FlexboxProps{
            .align_items = Align::Start,
            .gap = StyleValue::point(6.0f),
            .children = file_entries
        });

        auto main_col = column(FlexboxProps{
            .align_items = Align::Center,
            .gap = StyleValue::point(20.0f),
            .children = {title, subtitle, dz, files_col}
        });

        return container(ContainerProps{
            .color = 0xFF0B1320,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .padding = StyleInsets::all(36.0f),
            .child = main_col
        });
    }
};

class FileDropZoneDemoApp : public StatefulWidget {
public:
    std::string_view typeName() const override { return "FileDropZoneDemoApp"; }
    std::unique_ptr<State> createState() override { return std::make_unique<FileDropZoneDemoState>(); }
};

int main() {
    std::cout << "=== ENKI FileDropZone Standalone Demo (Native DnD) ===\n";
    std::cout << "[ENKI] Drag and drop any real file from your desktop/file manager into the drop zone.\n";
    AppConfig config;
    config.title = "ENKI — FileDropZone (Native DnD)";
    config.width = 760;
    config.height = 460;
    config.resizable = true;
    config.vsync = false;
    config.target_fps = 0;
    config.show_performance_overlay = true;
    config.clear_color = 0xFF0B1320;

    return runApp(std::make_shared<FileDropZoneDemoApp>(), config);
}
