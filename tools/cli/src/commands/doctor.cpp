#include "commands/doctor.hpp"
#include "toolchains/msvc_detector.hpp"
#include "toolchains/android_detector.hpp"
#include "toolchains/device_manager.hpp"
#include "core/terminal.hpp"
#include "core/process.hpp"

namespace enki::cli {

int DoctorCommand::execute(const fs::path& repo_root, const std::vector<std::string>&) {
    Terminal::banner();
    Terminal::header("Checking ENKI Development Environment Diagnostics");
    std::cout << "\n";

    int passed = 0;
    int total = 0;

    // 1. C++ Compiler
    total++;
#if defined(_WIN32)
    auto msvc = MsvcDetector::detect();
    if (msvc.found) {
        Terminal::ok("C++ Compiler (MSVC)", msvc.version);
        passed++;
    } else {
        Terminal::warn("C++ Compiler (MSVC)", "Not found. Install Visual Studio 2022 C++ tools.");
    }
#else
    auto gcc = Env::which("g++");
    auto clang = Env::which("clang++");
    if (clang) {
        Terminal::ok("C++ Compiler (Clang)", Env::pathToUtf8(*clang));
        passed++;
    } else if (gcc) {
        Terminal::ok("C++ Compiler (GCC)", Env::pathToUtf8(*gcc));
        passed++;
    } else {
        Terminal::fail("C++ Compiler", "Neither g++ nor clang++ found in PATH.");
    }
#endif

    // 2. Build System (Meson & Ninja)
    total++;
    auto ninja_bin = Env::findNinja();
    auto meson_bin = Env::findMeson();
    if (ninja_bin && meson_bin) {
        Terminal::ok("Build Tools (Ninja & Meson)", "Ninja (" + Env::pathToUtf8(ninja_bin->filename()) + ") & Meson ready");
        passed++;
    } else if (ninja_bin) {
        Terminal::ok("Build Tools (Ninja)", Env::pathToUtf8(*ninja_bin));
        passed++;
    } else {
        Terminal::fail("Build Tools", "Install Ninja and Meson build systems.");
    }

    // 3. Skia Engine Binaries
    total++;
    std::error_code ec;
#if defined(_WIN32)
    fs::path skia_win = repo_root / "core" / "Skia-Windows" / "out" / "Release-x64" / "skia.lib";
    if (fs::exists(skia_win, ec)) {
        Terminal::ok("Skia Engine (Desktop)", "core/Skia-Windows (Release-x64)");
        passed++;
    } else {
        Terminal::fail("Skia Engine (Desktop)", "skia.lib missing in core/Skia-Windows");
    }
#else
    Terminal::ok("Skia Engine (Desktop)", "System/bundled Skia");
    passed++;
#endif

    // 4. Android SDK & Build-Tools
    total++;
    auto android_tc = AndroidDetector::detect();
    if (android_tc.found) {
        std::string details = "SDK: " + Env::pathToUtf8(android_tc.sdk_root)
            + " | Build-tools: " + android_tc.build_tools_version
            + " | " + android_tc.platform_version;
        Terminal::ok("Android SDK Tools", details);
        passed++;
    } else if (!android_tc.sdk_root.empty()) {
        Terminal::warn("Android SDK Tools", "SDK found at " + Env::pathToUtf8(android_tc.sdk_root) + " but build-tools or android.jar missing");
    } else {
        Terminal::warn("Android SDK Tools", "Android SDK not found. Set ANDROID_HOME or install via Android Studio.");
    }

    // 5. Android NDK
    total++;
    if (!android_tc.ndk_root.empty()) {
        Terminal::ok("Android NDK", android_tc.ndk_version + " (" + Env::pathToUtf8(android_tc.ndk_root) + ")");
        passed++;
    } else {
        Terminal::warn("Android NDK", "NDK not detected. Set ANDROID_NDK_HOME if building for Android.");
    }

    // 6. Connected Devices / Emulators
    total++;
    auto devices = DeviceManager::listDevices(android_tc);
    int android_count = 0;
    for (const auto& dev : devices) {
        if (dev.type != DeviceType::Desktop) android_count++;
    }

    if (android_count > 0) {
        std::string d_str = std::to_string(devices.size()) + " target(s) available (1 Desktop, "
            + std::to_string(android_count) + " Android device/emulator)";
        Terminal::ok("Target Devices", d_str);
        passed++;
    } else {
        Terminal::ok("Target Devices", "Host Desktop available. (No Android devices connected)");
        passed++;
    }

    std::cout << "\n";
    if (passed == total) {
        std::cout << Terminal::style("• All " + std::to_string(total) + " environment checks passed! Ready to build and run.", Color::BrightGreen, true) << "\n\n";
    } else {
        std::cout << Terminal::style("• " + std::to_string(passed) + " of " + std::to_string(total) + " checks passed. Check warnings above.", Color::BrightYellow, true) << "\n\n";
    }

    return 0;
}

} // namespace enki::cli
