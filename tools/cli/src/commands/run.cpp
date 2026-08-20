#include "commands/run.hpp"
#include "core/project_config.hpp"
#include "core/terminal.hpp"
#include "core/process.hpp"
#include "toolchains/msvc_detector.hpp"
#include "toolchains/android_detector.hpp"
#include "packaging/android_packager.hpp"

namespace enki::cli {

int RunCommand::execute(const fs::path& repo_root, const std::vector<std::string>& args) {
    std::string app_name;
    std::string target = "desktop";
    std::string device_id = "";
    bool stream_logs = true;

    // Parse arguments
    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "android" || args[i] == "--android") {
            target = "android";
        } else if (args[i] == "desktop" || args[i] == "--desktop" || args[i] == "windows" || args[i] == "linux") {
            target = "desktop";
        } else if (args[i] == "drm" || args[i] == "--drm" || args[i] == "kms") {
            target = "drm";
        } else if (args[i] == "wayland" || args[i] == "--wayland") {
            target = "wayland";
        } else if (args[i] == "x11" || args[i] == "--x11") {
            target = "x11";
        } else if (args[i] == "--device" && i + 1 < args.size()) {
            device_id = args[++i];
            if (device_id == "drm") {
                target = "drm";
            } else if (device_id == "desktop") {
                target = "desktop";
            } else {
                target = "android";
            }
        } else if (args[i] == "--no-logs") {
            stream_logs = false;
        } else if (app_name.empty() && args[i][0] != '-') {
            app_name = args[i];
        }
    }

    auto apps = ProjectConfig::discoverApps(repo_root);
    if (apps.empty()) {
        Terminal::fail("No Apps Found", "No apps found in real_app/. Create one with 'enki create <name>'.");
        return 1;
    }

    if (app_name.empty()) {
        Terminal::header("Select an application to run:");
        for (const auto& a : apps) {
            std::cout << "  • " << Terminal::style(a.name, Color::BrightCyan) << " (" << a.title << ")\n";
        }
        std::cout << "\nUsage: enki run <app_name> [desktop|drm|wayland|x11|android]\n";
        return 1;
    }

    auto app_opt = ProjectConfig::findApp(repo_root, app_name);
    if (!app_opt) {
        Terminal::fail("Application Not Found", "Could not find app '" + app_name + "' in real_app/");
        return 1;
    }
    const auto& app = *app_opt;

    Terminal::banner();
    std::string target_label = "Desktop";
    if (target == "android") target_label = "Android";
    else if (target == "drm") target_label = "Linux DRM/KMS (Direct Scanout)";
    else if (target == "wayland") target_label = "Linux Wayland";
    else if (target == "x11") target_label = "Linux X11";

    Terminal::header("Running " + app.title + " on " + target_label);
    std::cout << "\n";

    if (target == "android") {
        auto android_tc = AndroidDetector::detect();
        if (!android_tc.found) {
            Terminal::fail("Android Toolchain Incomplete", "Run 'enki doctor' to diagnose missing tools.");
            return 1;
        }
        if (AndroidPackager::buildAndDeploy(app, android_tc, repo_root, device_id, stream_logs)) {
            return 0;
        }
        return 1;
    } else {
        // Desktop / Linux Target
        std::error_code ec;
#if defined(_WIN32)
        fs::path build_dir = repo_root / "build-Win";
        if (!fs::exists(build_dir, ec)) {
            build_dir = repo_root / "build";
        }
        std::string exe_name = "enki_" + app.name + ".exe";
#else
        fs::path build_dir = repo_root / "build";
        if (!fs::exists(build_dir, ec)) {
            build_dir = repo_root / "build-linux";
        }
        std::string exe_name = "enki_" + app.name;
#endif

        if (!fs::exists(build_dir, ec)) {
            Terminal::warn("Build Directory Missing", "Configuring build directory with Meson...");
            auto meson_path = Env::findMeson();
            std::string meson_bin = meson_path ? Env::pathToUtf8(*meson_path) : "meson";
            std::string setup_cmd = "\"" + meson_bin + "\" setup \"" + Env::pathToUtf8(build_dir) + "\"";
            auto setup_res = Process::run(setup_cmd, Env::pathToUtf8(repo_root), true);
            if (!setup_res.success()) {
                Terminal::fail("Build Configuration Failed", "Run 'meson setup build' manually.");
                return 1;
            }
        }

        Terminal::step(1, 2, "Building Desktop executable (" + app.name + ")");
        std::string ninja_target = "real_app/" + app.name + "/" + exe_name;
        auto ninja_path = Env::findNinja();
        std::string ninja_bin = ninja_path ? Env::pathToUtf8(*ninja_path) : "ninja";
        std::string ninja_cmd = "\"" + ninja_bin + "\" -C \"" + Env::pathToUtf8(build_dir) + "\" " + ninja_target;

#if defined(_WIN32)
        auto msvc = MsvcDetector::detect();
        ninja_cmd = MsvcDetector::wrapCommand(ninja_cmd, msvc);
#endif

        auto ninja_res = Process::run(ninja_cmd);
        if (!ninja_res.success()) {
            Terminal::fail("Build Failed", ninja_res.stderr_str.empty() ? ninja_res.stdout_str : ninja_res.stderr_str);
            return 1;
        }

        Terminal::step(2, 2, "Launching " + app.title);
        fs::path exe_path = build_dir / "real_app" / app.name / exe_name;
        if (!fs::exists(exe_path, ec)) {
            Terminal::fail("Executable Not Found", Env::pathToUtf8(exe_path));
            return 1;
        }

        Terminal::ok("Process Started", Env::pathToUtf8(exe_path));
        std::cout << "\n" << Terminal::style("=== Application Output ===", Color::BrightCyan, true) << "\n";

        std::vector<std::pair<std::string, std::string>> env_vars;
        if (target == "drm") {
            env_vars.push_back({"ENKI_BACKEND", "drm"});
        } else if (target == "wayland") {
            env_vars.push_back({"ENKI_BACKEND", "wayland"});
        } else if (target == "x11") {
            env_vars.push_back({"ENKI_BACKEND", "x11"});
        }

        Process::run("\"" + Env::pathToUtf8(exe_path) + "\"", "", true, nullptr, env_vars);
        return 0;
    }
}

} // namespace enki::cli
