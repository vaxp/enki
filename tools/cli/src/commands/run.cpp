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
        } else if (args[i] == "desktop" || args[i] == "--desktop" || args[i] == "windows") {
            target = "desktop";
        } else if (args[i] == "--device" && i + 1 < args.size()) {
            device_id = args[++i];
            target = "android";
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
        std::cout << "\nUsage: enki run <app_name> [desktop|android]\n";
        return 1;
    }

    auto app_opt = ProjectConfig::findApp(repo_root, app_name);
    if (!app_opt) {
        Terminal::fail("Application Not Found", "Could not find app '" + app_name + "' in real_app/");
        return 1;
    }
    const auto& app = *app_opt;

    Terminal::banner();
    Terminal::header("Running " + app.title + " on " + (target == "android" ? "Android" : "Desktop"));
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
        // Desktop Target
        fs::path build_dir = repo_root / "build-Win";
        std::error_code ec;
        if (!fs::exists(build_dir, ec)) {
            Terminal::fail("Build Directory Missing", "build-Win directory does not exist.");
            return 1;
        }

        Terminal::step(1, 2, "Building Desktop executable (" + app.name + ")");
        std::string ninja_target = "real_app/" + app.name + "/enki_" + app.name + ".exe";
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
        fs::path exe_path = build_dir / "real_app" / app.name / ("enki_" + app.name + ".exe");
        if (!fs::exists(exe_path, ec)) {
            Terminal::fail("Executable Not Found", Env::pathToUtf8(exe_path));
            return 1;
        }

        Terminal::ok("Process Started", Env::pathToUtf8(exe_path));
        std::cout << "\n" << Terminal::style("=== Application Output ===", Color::BrightCyan, true) << "\n";
        Process::run("\"" + Env::pathToUtf8(exe_path) + "\"", "", true);
        return 0;
    }
}

} // namespace enki::cli
