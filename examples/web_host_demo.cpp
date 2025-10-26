/// @file web_host_demo.cpp
/// @brief Enki Web Host Demo — runs an HTML/CSS/JS application hosted inside Enki.
///
/// @copyright ENKI Framework — MIT License

#include <web_technology/EnkiWebHost.hpp>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

int main(int argc, char* argv[])
{
    std::cout << "========================================\n";
    std::cout << "    Enki Web Host (Electron Alternative)\n";
    std::cout << "========================================\n";

    enki::web::EnkiWebHost host(argc, argv);

    // Look for template enki.json
    fs::path template_config = "web_technology/template/enki.json";
    if (fs::exists(template_config)) {
        host.load_config(template_config.string());
    } else {
        // Fallback default config
        enki::web::AppConfig cfg;
        cfg.name = "Enki Demo App";
        cfg.entry = "web_technology/template/src/index.html";
        cfg.window.title = "Enki Web Host Demo";
        cfg.window.width = 1280;
        cfg.window.height = 800;
        host.set_config(cfg);
    }

    return host.run();
}
