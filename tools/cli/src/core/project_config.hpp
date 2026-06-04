#pragma once
/// @file project_config.hpp
/// @brief ENKI application discovery and configuration model.

#include "core/env.hpp"
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace enki::cli {

struct AppMetadata {
    std::string name;
    std::string app_id;          // e.g. "org.enki.gallery"
    std::string version = "1.0.0";
    std::string title;
    fs::path dir_path;
    bool has_assets = false;
    std::vector<std::string> permissions; // e.g. "STORAGE", "INTERNET"
};

class ProjectConfig {
public:
    static std::vector<AppMetadata> discoverApps(const fs::path& repo_root) {
        std::vector<AppMetadata> apps;
        fs::path real_app_dir = repo_root / "real_app";
        std::error_code ec;

        if (!fs::exists(real_app_dir, ec) || !fs::is_directory(real_app_dir, ec)) {
            return apps;
        }

        for (const auto& entry : fs::directory_iterator(real_app_dir, ec)) {
            if (!entry.is_directory(ec)) continue;

            std::string app_name = Env::pathToUtf8(entry.path().filename());
            fs::path meson_file = entry.path() / "meson.build";
            if (!fs::exists(meson_file, ec)) continue;

            AppMetadata app;
            app.name = app_name;
            app.dir_path = entry.path();
            app.app_id = "org.enki." + app_name;
            app.title = "ENKI " + app_name;
            app.has_assets = fs::exists(entry.path() / "assets", ec);

            if (app_name == "gallery") {
                app.title = "ENKI Gallery";
                app.permissions = {
                    "INTERNET",
                    "ACCESS_NETWORK_STATE",
                    "READ_EXTERNAL_STORAGE",
                    "WRITE_EXTERNAL_STORAGE",
                    "MANAGE_EXTERNAL_STORAGE",
                    "READ_MEDIA_IMAGES",
                    "READ_MEDIA_VIDEO"
                };
            } else if (app_name == "calc") {
                app.title = "ENKI Calculator";
            } else if (app_name == "counter") {
                app.title = "ENKI Counter";
            }

            apps.push_back(std::move(app));
        }

        return apps;
    }

    static std::optional<AppMetadata> findApp(const fs::path& repo_root, const std::string& name) {
        auto all = discoverApps(repo_root);
        for (const auto& app : all) {
            if (app.name == name) return app;
        }
        return std::nullopt;
    }
};

} // namespace enki::cli
