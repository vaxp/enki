#pragma once
/// @file msvc_detector.hpp
/// @brief Auto-discovery of MSVC, Visual Studio, and Windows SDK toolchains.

#include "core/env.hpp"
#include "core/process.hpp"
#include <string>
#include <vector>
#include <filesystem>
#include <optional>

namespace enki::cli {

struct MsvcInfo {
    bool found = false;
    std::string version;
    std::string installation_path;
    std::string vcvars_path;
};

class MsvcDetector {
public:
    static MsvcInfo detect() {
        MsvcInfo info;
#if defined(_WIN32)
        // 1. First check if cl.exe is already in PATH
        auto cl_path = Env::which("cl");
        if (cl_path) {
            info.found = true;
            info.version = "In PATH (" + Env::pathToUtf8(*cl_path) + ")";
            return info;
        }

        // 2. Query vswhere.exe
        std::string prog_files_x86 = Env::get("ProgramFiles(x86)");
        std::string vswhere_path;
        if (!prog_files_x86.empty()) {
            vswhere_path = prog_files_x86 + "\\Microsoft Visual Studio\\Installer\\vswhere.exe";
        } else {
            vswhere_path = "C:\\Program Files (x86)\\Microsoft Visual Studio\\Installer\\vswhere.exe";
        }

        std::error_code ec;
        if (fs::exists(Env::stringToPath(vswhere_path), ec)) {
            std::string cmd = "\"" + vswhere_path + "\" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath";
            auto res = Process::run(cmd);
            if (res.success() && !res.stdout_str.empty()) {
                std::string install_dir = res.stdout_str;
                while (!install_dir.empty() && (install_dir.back() == '\r' || install_dir.back() == '\n' || install_dir.back() == ' ')) {
                    install_dir.pop_back();
                }

                if (!install_dir.empty() && fs::exists(Env::stringToPath(install_dir), ec)) {
                    info.installation_path = install_dir;

                    // Locate vcvars64.bat
                    fs::path vcvars = Env::stringToPath(install_dir) / "VC" / "Auxiliary" / "Build" / "vcvars64.bat";
                    if (fs::exists(vcvars, ec)) {
                        info.found = true;
                        info.vcvars_path = Env::pathToUtf8(vcvars);
                        info.version = "Visual Studio (via " + info.vcvars_path + ")";
                        return info;
                    }
                }
            }
        }

        // 3. Fallback to standard 2022 paths
        std::vector<std::string> standard_vcvars = {
            "C:\\Program Files (x86)\\Microsoft Visual Studio\\2022\\BuildTools\\VC\\Auxiliary\\Build\\vcvars64.bat",
            "C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Auxiliary\\Build\\vcvars64.bat",
            "C:\\Program Files\\Microsoft Visual Studio\\2022\\Professional\\VC\\Auxiliary\\Build\\vcvars64.bat",
            "C:\\Program Files\\Microsoft Visual Studio\\2022\\Enterprise\\VC\\Auxiliary\\Build\\vcvars64.bat"
        };
        for (const auto& p : standard_vcvars) {
            if (fs::exists(Env::stringToPath(p), ec)) {
                info.found = true;
                info.vcvars_path = p;
                info.version = "Visual Studio 2022";
                return info;
            }
        }
#endif
        return info;
    }

    /// @brief Wrap a command to execute within the MSVC environment if needed
    static std::string wrapCommand(const std::string& cmd, const MsvcInfo& info) {
#if defined(_WIN32)
        if (Env::which("cl")) {
            return cmd; // Already configured
        }
        if (!info.vcvars_path.empty()) {
            return "cmd.exe /c \"call \"" + info.vcvars_path + "\" && " + cmd + "\"";
        }
#endif
        return cmd;
    }
};

} // namespace enki::cli
