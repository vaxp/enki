/// @file main.cpp
/// @brief ENKI Advanced FilePicker Widget Interactive Showcase.
/// Demonstrates native compositor file picker dialogs, directory navigation, quick access, and open/save modes.

#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/file_picker.hpp"
#include "enki/state/state.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <memory>

using namespace enki;

class FilePickerDemoState : public State {
private:
    std::string selected_file_path_ = "No file selected";

public:
    WidgetPtr build(BuildContext& ctx) override {
        return container({
            .color = 0xFF0F172A,
            .padding = StyleInsets::all(40.0f),
            .child = column({
                .justify_content = Justify::Center,
                .align_items = Align::Center,
                .children = {
                    // Title & Description Header
                    column({
                        .align_items = Align::Center,
                        .children = {
                            text("Advanced Native FilePicker (NativePopup)", {
                                .color = 0xFFFFFFFF,
                                .font_size = 24.0f,
                                .font_weight = FontWeight::Bold
                            }),
                            text("Click buttons below to launch standalone desktop file/folder picker dialogs", {
                                .color = 0xFF94A3B8,
                                .font_size = 14.0f
                            })
                        }
                    }),

                    // Layout rows for pickers
                    container({
                        .margin = StyleInsets::only(40.0f, 0.0f, 0.0f, 0.0f),
                        .child = row({
                            .justify_content = Justify::Center,
                            .align_items = Align::Center,
                            .gap = StyleValue::point(30.0f),
                            .children = {
                                // 1. Open File Button
                                FilePicker {
                                    .child = button(text("📂 Open File...", { .color = 0xFFFFFFFF, .font_size = 14.0f, .font_weight = FontWeight::Bold }), nullptr),
                                    .on_result = [this](const FilePickerResult& res) {
                                        if (!res.canceled && !res.selected_paths.empty()) {
                                            setState([this, res]() {
                                                selected_file_path_ = "Opened File: " + res.selected_paths[0];
                                            });
                                            std::cout << "[FilePicker] Selected file: " << res.selected_paths[0] << "\n";
                                        }
                                    },
                                    .mode = FilePickerMode::OpenFile
                                },

                                // 2. Select Folder Button
                                FilePicker {
                                    .child = button(text("📁 Select Folder...", { .color = 0xFFFFFFFF, .font_size = 14.0f, .font_weight = FontWeight::Bold }), nullptr),
                                    .on_result = [this](const FilePickerResult& res) {
                                        if (!res.canceled && !res.selected_paths.empty()) {
                                            setState([this, res]() {
                                                selected_file_path_ = "Selected Folder: " + res.selected_paths[0];
                                            });
                                            std::cout << "[FilePicker] Selected folder: " << res.selected_paths[0] << "\n";
                                        }
                                    },
                                    .mode = FilePickerMode::SelectFolder
                                },

                                // 3. Save File Button
                                FilePicker {
                                    .child = button(text("💾 Save File As...", { .color = 0xFFFFFFFF, .font_size = 14.0f, .font_weight = FontWeight::Bold }), nullptr),
                                    .on_result = [this](const FilePickerResult& res) {
                                        if (!res.canceled && !res.selected_paths.empty()) {
                                            setState([this, res]() {
                                                selected_file_path_ = "Save Destination: " + res.selected_paths[0];
                                            });
                                            std::cout << "[FilePicker] Save destination: " << res.selected_paths[0] << "\n";
                                        }
                                    },
                                    .mode = FilePickerMode::SaveFile,
                                    .default_filename = "export_project.json"
                                }
                            }
                        })
                    }),

                    // Result Card
                    container({
                        .color = 0xFF1E293B,
                        .border_radius = BorderRadius::circular(10.0f),
                        .border = Border(0xFF334155, 1.0f),
                        .width = StyleValue::point(550.0f),
                        .padding = StyleInsets::all(20.0f),
                        .margin = StyleInsets::only(30.0f, 0.0f, 0.0f, 0.0f),
                        .child = column({
                            .gap = StyleValue::point(6.0f),
                            .children = {
                                text("Current Selection Payload:", { .color = 0xFF38BDF8, .font_size = 13.0f, .font_weight = FontWeight::Bold }),
                                text(selected_file_path_, { .color = 0xFFF1F5F9, .font_size = 13.0f })
                            }
                        })
                    })
                }
            })
        });
    }
};

class FilePickerDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<FilePickerDemoState>();
    }
    std::string_view typeName() const override { return "FilePickerDemoApp"; }
};

int main() {
    std::cout << "====================================================\n";
    std::cout << "  ENKI Engine — FilePicker Widget Demo (NativePopup)\n";
    std::cout << "====================================================\n";

    AppConfig config;
    config.title       = "Enki Engine — FilePicker Demo";
    config.width       = 850;
    config.height      = 450;
    config.resizable   = true;
    config.vsync       = false;
    config.target_fps  = 60;
    config.clear_color = 0xFF0F172A;

    return runApp(std::make_shared<FilePickerDemoApp>(), config);
}
