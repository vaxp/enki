#include "commands/doctor.hpp"
#include "toolchains/msvc_detector.hpp"
#include "toolchains/linux_detector.hpp"
#include "toolchains/android_detector.hpp"
#include "toolchains/wasm_detector.hpp"
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
    auto linux_info = LinuxDetector::detect(repo_root);
    if (!linux_info.compiler.empty()) {
        Terminal::ok("C++ Compiler (" + linux_info.compiler + ")",
                     linux_info.compiler_version.empty() ? linux_info.compiler : linux_info.compiler_version);
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
    if (linux_info.has_skia) {
        Terminal::ok("Skia Engine (Desktop)", linux_info.skia_path);
        passed++;
    } else {
        Terminal::ok("Skia Engine (Desktop)", "System/bundled Skia");
        passed++;
    }

    // 3b. Linux Display Backends
    total++;
    std::string backends;
    if (linux_info.has_wayland) backends += "Wayland ";
    if (linux_info.has_x11)     backends += "X11 ";
    if (linux_info.has_drm)     backends += "DRM/KMS ";
    if (linux_info.has_egl)     backends += "EGL ";
    if (!backends.empty()) {
        Terminal::ok("Linux Display Backends", backends + "[Active: " + linux_info.active_session + "]");
        passed++;
    } else {
        Terminal::warn("Linux Display Backends", "No standard display backends detected via pkg-config");
    }

    // 3c. Linux Direct KMS permissions check
    if (linux_info.has_drm) {
        total++;
        if (linux_info.in_input_group) {
            Terminal::ok("Direct KMS Permissions", "User belongs to 'input' group (Bare-metal TTY ready)");
            passed++;
        } else {
            Terminal::warn("Direct KMS Permissions", "User not in 'input' group. For bare-metal TTY without sudo, run: sudo usermod -aG input,video $USER");
        }
    }
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

    // 5b. WebAssembly / Emscripten Toolchain
    total++;
    auto wasm_tc = WasmDetector::detect(repo_root);
    if (wasm_tc.found) {
        std::string info = wasm_tc.emcc_version.empty() ? "Emscripten SDK ready" : wasm_tc.emcc_version;
        if (!wasm_tc.has_skia_wasm) {
            info += " | Skia Wasm: missing core/skia/out/Release-wasm/libskia.a";
            Terminal::warn("WebAssembly Toolchain", info);
        } else {
            Terminal::ok("WebAssembly Toolchain", info + " (Skia Wasm ready)");
            passed++;
        }
    } else {
        Terminal::warn("WebAssembly Toolchain", "Emscripten SDK not detected. Set EMSDK or install from https://emscripten.org");
    }

    // 6. Connected Devices / Emulators
    total++;
    auto devices = DeviceManager::listDevices(android_tc);
    int android_count = 0;
    for (const auto& dev : devices) {
        if (dev.type == DeviceType::AndroidEmulator || dev.type == DeviceType::AndroidPhysical) android_count++;
    }

    if (android_count > 0) {
        std::string d_str = std::to_string(devices.size()) + " targets available (Desktop, Web Browser, "
            + std::to_string(android_count) + " Android device/emulator)";
        Terminal::ok("Target Devices", d_str);
        passed++;
    } else {
        Terminal::ok("Target Devices", "Host Desktop & Web Browser available. (No Android devices connected)");
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
