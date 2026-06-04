#pragma once
/// @file device_manager.hpp
/// @brief Cross-platform target device discovery (Desktop, Android Emulators, Hardware phones).

#include "toolchains/android_detector.hpp"
#include "core/process.hpp"
#include <string>
#include <vector>
#include <sstream>

namespace enki::cli {

enum class DeviceType {
    Desktop,
    AndroidEmulator,
    AndroidPhysical
};

struct TargetDevice {
    std::string id;
    std::string name;
    DeviceType type;
    std::string status; // "device", "offline", "unauthorized", "ready"
    bool is_default = false;
};

class DeviceManager {
public:
    static std::vector<TargetDevice> listDevices(const AndroidToolchain& android_tc) {
        std::vector<TargetDevice> devices;

        // 1. Host Desktop is always available
#if defined(_WIN32)
        devices.push_back({
            .id = "desktop",
            .name = "Windows Desktop (x64)",
            .type = DeviceType::Desktop,
            .status = "ready",
            .is_default = true
        });
#elif defined(__APPLE__)
        devices.push_back({
            .id = "desktop",
            .name = "macOS Desktop",
            .type = DeviceType::Desktop,
            .status = "ready",
            .is_default = true
        });
#else
        devices.push_back({
            .id = "desktop",
            .name = "Linux Desktop",
            .type = DeviceType::Desktop,
            .status = "ready",
            .is_default = true
        });
#endif

        // 2. Query ADB devices if ADB exists
        if (!android_tc.adb_path.empty()) {
            std::string cmd = "\"" + Env::pathToUtf8(android_tc.adb_path) + "\" devices -l";
            auto res = Process::run(cmd);
            if (res.success()) {
                std::istringstream stream(res.stdout_str);
                std::string line;
                bool first_line = true;

                while (std::getline(stream, line)) {
                    while (!line.empty() && (line.back() == '\r' || line.back() == '\n')) line.pop_back();
                    if (line.empty()) continue;
                    if (first_line && line.find("List of devices") != std::string::npos) {
                        first_line = false;
                        continue;
                    }

                    std::istringstream line_stream(line);
                    std::string dev_id, dev_status;
                    line_stream >> dev_id >> dev_status;

                    if (dev_id.empty() || dev_status.empty()) continue;

                    // Parse model name if present
                    std::string model = dev_id;
                    size_t model_pos = line.find("model:");
                    if (model_pos != std::string::npos) {
                        size_t end_model = line.find(' ', model_pos);
                        model = line.substr(model_pos + 6, (end_model == std::string::npos ? line.length() : end_model) - (model_pos + 6));
                    }

                    bool is_emu = (dev_id.find("emulator") != std::string::npos);
                    std::string display_name = (is_emu ? "Android Emulator (" : "Android Device (") + model + ")";

                    devices.push_back({
                        .id = dev_id,
                        .name = display_name,
                        .type = is_emu ? DeviceType::AndroidEmulator : DeviceType::AndroidPhysical,
                        .status = dev_status,
                        .is_default = false
                    });
                }
            }
        }

        return devices;
    }
};

} // namespace enki::cli
