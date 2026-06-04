#include "commands/devices.hpp"
#include "toolchains/android_detector.hpp"
#include "toolchains/device_manager.hpp"
#include "core/terminal.hpp"
#include <iomanip>

namespace enki::cli {

int DevicesCommand::execute(const fs::path&, const std::vector<std::string>&) {
    auto android_tc = AndroidDetector::detect();
    auto devices = DeviceManager::listDevices(android_tc);

    Terminal::header("Connected Target Devices & Platforms");
    std::cout << "\n";

    auto pad = [](std::string s, size_t w) {
        if (s.length() < w) s.append(w - s.length(), ' ');
        return s;
    };

    std::cout << "  "
              << Terminal::style(pad("Device ID", 20), Color::Bold)
              << Terminal::style(pad("Name / Model", 34), Color::Bold)
              << Terminal::style(pad("Platform", 18), Color::Bold)
              << Terminal::style("Status", Color::Bold) << "\n";
    std::cout << "  " << std::string(80, '-') << "\n";

    for (const auto& dev : devices) {
        std::string platform_str;
        Color status_color = Color::Green;

        if (dev.type == DeviceType::Desktop) {
            platform_str = "Desktop";
            status_color = Color::BrightGreen;
        } else if (dev.type == DeviceType::AndroidEmulator) {
            platform_str = "Android (Emu)";
            status_color = (dev.status == "device") ? Color::BrightGreen : Color::Yellow;
        } else {
            platform_str = "Android (Phone)";
            status_color = (dev.status == "device") ? Color::BrightGreen : Color::Yellow;
        }

        std::cout << "  "
                  << Terminal::style(pad(dev.id, 20), Color::BrightCyan)
                  << pad(dev.name, 34)
                  << pad(platform_str, 18)
                  << Terminal::style(dev.status, status_color) << "\n";
    }

    std::cout << "\n" << Terminal::style("• To target a specific device, run: enki run <app> --device <device_id>", Color::Gray) << "\n\n";
    return 0;
}

} // namespace enki::cli
