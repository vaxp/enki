#pragma once
/// @file project_template.hpp
/// @brief Scaffolding generator for new clean-architecture ENKI applications.

#include "core/env.hpp"
#include "core/terminal.hpp"
#include <string>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace enki::cli {

class ProjectTemplate {
public:
    static bool create(const fs::path& repo_root, const std::string& app_name) {
        fs::path app_dir = repo_root / "real_app" / app_name;
        std::error_code ec;

        if (fs::exists(app_dir, ec)) {
            Terminal::fail("Creation Aborted", "Directory already exists: " + Env::pathToUtf8(app_dir));
            return false;
        }

        Terminal::header("Scaffolding New ENKI Application: " + app_name);

        // 1. Create directories
        std::vector<fs::path> subdirs = {
            app_dir,
            app_dir / "src",
            app_dir / "src" / "models",
            app_dir / "src" / "services",
            app_dir / "src" / "state",
            app_dir / "src" / "ui",
            app_dir / "src" / "ui" / "components",
            app_dir / "src" / "ui" / "views",
            app_dir / "assets"
        };

        for (const auto& d : subdirs) {
            fs::create_directories(d, ec);
            Terminal::ok("Created directory", Env::pathToUtf8(d.lexically_relative(repo_root)));
        }

        // 2. Generate meson.build
        fs::path meson_file = app_dir / "meson.build";
        std::ofstream m_ofs(meson_file);
        if (m_ofs.is_open()) {
            m_ofs << "# ============================================================\n";
            m_ofs << "# ENKI " << app_name << " Application Build\n";
            m_ofs << "# ============================================================\n\n";
            m_ofs << app_name << "_sources = files(\n";
            m_ofs << "  'src/main.cpp',\n";
            m_ofs << ")\n\n";
            m_ofs << app_name << "_inc = include_directories('src')\n\n";
            m_ofs << "if is_windows\n";
            m_ofs << "  configure_file(\n";
            m_ofs << "    input: meson.project_source_root() / 'core/Skia-Windows/out/Release-x64/icudtl.dat',\n";
            m_ofs << "    output: 'icudtl.dat',\n";
            m_ofs << "    copy: true\n";
            m_ofs << "  )\n";
            m_ofs << "endif\n\n";
            m_ofs << "if is_android\n";
            m_ofs << "  shared_library('enki_" << app_name << "',\n";
            m_ofs << "    sources: " << app_name << "_sources,\n";
            m_ofs << "    include_directories: [" << app_name << "_inc],\n";
            m_ofs << "    dependencies: [enki_dep],\n";
            m_ofs << "    name_prefix: 'lib',\n";
            m_ofs << "    link_args: ['-u', 'ANativeActivity_onCreate'],\n";
            m_ofs << "    install: true,\n";
            m_ofs << "  )\n";
            m_ofs << "else\n";
            m_ofs << "  executable('enki_" << app_name << "',\n";
            m_ofs << "    sources: " << app_name << "_sources,\n";
            m_ofs << "    include_directories: [" << app_name << "_inc],\n";
            m_ofs << "    dependencies: [enki_dep],\n";
            m_ofs << "    install: true,\n";
            m_ofs << "  )\n";
            m_ofs << "endif\n";
            m_ofs.close();
            Terminal::ok("Generated build target", "meson.build");
        }

        // 3. Generate starter main.cpp
        fs::path main_cpp = app_dir / "src" / "main.cpp";
        std::ofstream main_ofs(main_cpp);
        if (main_ofs.is_open()) {
            main_ofs << "/// @file main.cpp\n";
            main_ofs << "/// @brief Starter template for " << app_name << " built with ENKI framework.\n\n";
            main_ofs << "#include \"enki/enki.hpp\"\n";
            main_ofs << "#include <iostream>\n\n";
            main_ofs << "using namespace enki;\n\n";
            main_ofs << "class " << app_name << "App : public StatelessWidget {\n";
            main_ofs << "public:\n";
            main_ofs << "    WidgetPtr build(BuildContext& ctx) override {\n";
            main_ofs << "        auto title = text(\"" << app_name << "\", {\n";
            main_ofs << "            .color = 0xFFFFFFFF,\n";
            main_ofs << "            .font_size = 28.0f,\n";
            main_ofs << "            .font_weight = FontWeight::Bold\n";
            main_ofs << "        });\n\n";
            main_ofs << "        auto subtitle = text(\"Built with high-performance C++20 ENKI Engine\", {\n";
            main_ofs << "            .color = 0xFF94A3B8,\n";
            main_ofs << "            .font_size = 14.0f\n";
            main_ofs << "        });\n\n";
            main_ofs << "        auto card = container({\n";
            main_ofs << "            .color = 0xFF1E293B,\n";
            main_ofs << "            .border_radius = BorderRadius::circular(16.0f),\n";
            main_ofs << "            .border = Border(0xFF334155, 1.2f),\n";
            main_ofs << "            .padding = StyleInsets::all(20.0f),\n";
            main_ofs << "            .child = column({\n";
            main_ofs << "                .align_items = Align::Center,\n";
            main_ofs << "                .children = { title, sizedBox(0, 8.0f), subtitle }\n";
            main_ofs << "            })\n";
            main_ofs << "        });\n\n";
            main_ofs << "        auto content = container({\n";
            main_ofs << "            .color = 0xFF0B0F19,\n";
            main_ofs << "            .align = Alignment::Center,\n";
            main_ofs << "            .padding = StyleInsets::all(16.0f),\n";
            main_ofs << "            .child = card\n";
            main_ofs << "        });\n\n";
            main_ofs << "        return windowFrame(WindowFrameProps{\n";
            main_ofs << "            .content = content,\n";
            main_ofs << "            .title = \"" << app_name << "\",\n";
            main_ofs << "            .titlebar_style = TitleBarStyle::VAXPOS\n";
            main_ofs << "        });\n";
            main_ofs << "    }\n";
            main_ofs << "    std::string_view typeName() const override { return \"" << app_name << "App\"; }\n";
            main_ofs << "};\n\n";
            main_ofs << "int main() {\n";
            main_ofs << "    AppConfig config;\n";
            main_ofs << "    config.title       = \"" << app_name << "\";\n";
            main_ofs << "    config.app_id      = \"org.enki." << app_name << "\";\n";
            main_ofs << "    config.width       = 460;\n";
            main_ofs << "    config.height      = 840;\n";
            main_ofs << "    config.resizable   = true;\n";
            main_ofs << "    config.clear_color = 0xFF0B0F19;\n\n";
            main_ofs << "    return runApp(std::make_shared<" << app_name << "App>(), config);\n";
            main_ofs << "}\n";
            main_ofs.close();
            Terminal::ok("Generated starter code", "src/main.cpp");
        }

        // 4. Generate AndroidManifest.xml
        fs::path manifest_path = app_dir / "AndroidManifest.xml";
        std::ofstream manifest_ofs(manifest_path);
        if (manifest_ofs.is_open()) {
            manifest_ofs << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
            manifest_ofs << "<manifest xmlns:android=\"http://schemas.android.com/apk/res/android\"\n";
            manifest_ofs << "    package=\"org.enki." << app_name << "\">\n\n";
            manifest_ofs << "    <uses-permission android:name=\"android.permission.INTERNET\" />\n";
            manifest_ofs << "    <uses-permission android:name=\"android.permission.ACCESS_NETWORK_STATE\" />\n\n";
            manifest_ofs << "    <application\n";
            manifest_ofs << "        android:label=\"" << app_name << "\"\n";
            manifest_ofs << "        android:hasCode=\"false\">\n\n";
            manifest_ofs << "        <activity\n";
            manifest_ofs << "            android:name=\"android.app.NativeActivity\"\n";
            manifest_ofs << "            android:label=\"" << app_name << "\"\n";
            manifest_ofs << "            android:configChanges=\"orientation|keyboardHidden|screenSize\"\n";
            manifest_ofs << "            android:exported=\"true\">\n";
            manifest_ofs << "            <meta-data\n";
            manifest_ofs << "                android:name=\"android.app.lib_name\"\n";
            manifest_ofs << "                android:value=\"enki_" << app_name << "\" />\n";
            manifest_ofs << "            <intent-filter>\n";
            manifest_ofs << "                <action android:name=\"android.intent.action.MAIN\" />\n";
            manifest_ofs << "                <category android:name=\"android.intent.category.LAUNCHER\" />\n";
            manifest_ofs << "            </intent-filter>\n";
            manifest_ofs << "        </activity>\n";
            manifest_ofs << "    </application>\n";
            manifest_ofs << "</manifest>\n";
            manifest_ofs.close();
            Terminal::ok("Generated manifest", "AndroidManifest.xml");
        }

        // 5. Register in real_app/meson.build
        fs::path real_app_meson = repo_root / "real_app" / "meson.build";
        std::string subdir_line = "subdir('" + app_name + "')\n";
        bool already_registered = false;

        std::ifstream r_ifs(real_app_meson);
        if (r_ifs.is_open()) {
            std::string line;
            while (std::getline(r_ifs, line)) {
                if (line.find("subdir('" + app_name + "')") != std::string::npos) {
                    already_registered = true;
                    break;
                }
            }
            r_ifs.close();
        }

        if (!already_registered) {
            std::ofstream r_ofs(real_app_meson, std::ios::app);
            if (r_ofs.is_open()) {
                r_ofs << subdir_line;
                r_ofs.close();
                Terminal::ok("Registered in", "real_app/meson.build");
            }
        }

        std::cout << "\n" << Terminal::style("• Application '" + app_name + "' created successfully!", Color::BrightGreen, true) << "\n";
        std::cout << "  To run on Windows:\n";
        std::cout << "    " << Terminal::style("enki run " + app_name + " desktop", Color::BrightCyan) << "\n";
        std::cout << "  To run on Android:\n";
        std::cout << "    " << Terminal::style("enki run " + app_name + " android", Color::BrightCyan) << "\n\n";

        return true;
    }
};

} // namespace enki::cli
