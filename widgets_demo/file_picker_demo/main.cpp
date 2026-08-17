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
        // Title & Description Header
        auto title = text("Advanced Native FilePicker (NativePopup)");
        title->fontSize(24.0f).bold().color(0xFFFFFFFF);

        auto sub = text("Click buttons below to launch standalone desktop file/folder picker dialogs");
        sub->fontSize(14.0f).color(0xFF94A3B8);

        std::vector<WidgetPtr> t_children = {title, sub};
        auto titleCol = column(t_children);
        titleCol->alignItems(Align::Center).margin(StyleInsets::only(0, 0, 40.0f, 0));

        // 1. Open File Button
        auto open_btn_text = text("📂 Open File...");
        open_btn_text->fontSize(14.0f).color(0xFFFFFFFF).bold();

        FilePickerOptions open_opt;
        open_opt.mode = FilePickerMode::OpenFile;

        auto open_picker = filePicker(
            button(open_btn_text, nullptr),
            [this](const FilePickerResult& res) {
                if (!res.canceled && !res.selected_paths.empty()) {
                    setState([this, res]() {
                        selected_file_path_ = "Opened File: " + res.selected_paths[0];
                    });
                    std::cout << "[FilePicker] Selected file: " << res.selected_paths[0] << "\n";
                }
            },
            open_opt
        );

        // 2. Select Folder Button
        auto folder_btn_text = text("📁 Select Folder...");
        folder_btn_text->fontSize(14.0f).color(0xFFFFFFFF).bold();

        FilePickerOptions folder_opt;
        folder_opt.mode = FilePickerMode::SelectFolder;

        auto folder_picker = filePicker(
            button(folder_btn_text, nullptr),
            [this](const FilePickerResult& res) {
                if (!res.canceled && !res.selected_paths.empty()) {
                    setState([this, res]() {
                        selected_file_path_ = "Selected Folder: " + res.selected_paths[0];
                    });
                    std::cout << "[FilePicker] Selected folder: " << res.selected_paths[0] << "\n";
                }
            },
            folder_opt
        );

        // 3. Save File Button
        auto save_btn_text = text("💾 Save File As...");
        save_btn_text->fontSize(14.0f).color(0xFFFFFFFF).bold();

        FilePickerOptions save_opt;
        save_opt.mode = FilePickerMode::SaveFile;
        save_opt.default_filename = "export_project.json";

        auto save_picker = filePicker(
            button(save_btn_text, nullptr),
            [this](const FilePickerResult& res) {
                if (!res.canceled && !res.selected_paths.empty()) {
                    setState([this, res]() {
                        selected_file_path_ = "Save Destination: " + res.selected_paths[0];
                    });
                    std::cout << "[FilePicker] Save destination: " << res.selected_paths[0] << "\n";
                }
            },
            save_opt
        );

        // Result Card
        auto res_title = text("Current Selection Payload:");
        res_title->fontSize(13.0f).bold().color(0xFF38BDF8);

        auto res_path_txt = text(selected_file_path_);
        res_path_txt->fontSize(13.0f).color(0xFFF1F5F9);

        auto res_col = column({res_title, res_path_txt});
        res_col->gap(StyleValue::point(6.0f));

        auto res_box = container(res_col);
        res_box->color(0xFF1E293B)
               .borderRadius(10.0f)
               .border(0xFF334155, 1.0f)
               .paddingAll(20.0f)
               .margin(StyleInsets::only(30.0f, 0, 0, 0))
               .width(550.0f);

        // Layout rows
        std::vector<WidgetPtr> r_children = {open_picker, folder_picker, save_picker};
        auto buttonsRow = row(r_children);
        buttonsRow->justifyContent(Justify::Center).alignItems(Align::Center).gap(30_px);

        std::vector<WidgetPtr> m_children = {titleCol, buttonsRow, res_box};
        auto mainCol = column(m_children);
        mainCol->alignItems(Align::Center).justifyContent(Justify::Center);

        auto appRoot = container(mainCol);
        appRoot->color(0xFF0F172A)
               .paddingAll(40.0f)
               .flexGrow(1.0f);

        return appRoot;
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
