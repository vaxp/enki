#pragma once
/// @file linux_detector.hpp
/// @brief Auto-discovery of Linux compilers, graphics backends, Skia, and DRM permissions.

#include "core/env.hpp"
#include "core/process.hpp"
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <sstream>

#if !defined(_WIN32)
#include <unistd.h>
#include <grp.h>
#include <sys/types.h>
#endif

namespace enki::cli {

struct LinuxInfo {
    bool is_linux = false;
    std::string distro;
    std::string kernel;
    std::string compiler;
    std::string compiler_version;

    bool has_wayland = false;
    bool has_x11     = false;
    bool has_drm     = false;
    bool has_egl     = false;

    bool has_skia    = false;
    std::string skia_path;

    std::string active_session; // "Wayland", "X11", "Console (DRM/KMS)"

    bool in_input_group  = false;
    bool in_video_group  = false;
    bool in_render_group = false;
};

class LinuxDetector {
public:
    static LinuxInfo detect(const fs::path& repo_root = "") {
        LinuxInfo info;

#if !defined(_WIN32) && !defined(__APPLE__) && !defined(__ANDROID__)
        info.is_linux = true;

        // 1. Distro Name
        std::ifstream os_rel("/etc/os-release");
        if (os_rel.is_open()) {
            std::string line;
            while (std::getline(os_rel, line)) {
                if (line.rfind("PRETTY_NAME=", 0) == 0) {
                    std::string val = line.substr(12);
                    if (!val.empty() && val.front() == '"') val = val.substr(1);
                    if (!val.empty() && val.back() == '"') val.pop_back();
                    info.distro = val;
                    break;
                }
            }
            os_rel.close();
        }
        if (info.distro.empty()) info.distro = "Linux Generic";

        // 2. Kernel
        auto uname_res = Process::run("uname -r");
        if (uname_res.success()) {
            std::string k = uname_res.stdout_str;
            while (!k.empty() && (k.back() == '\n' || k.back() == '\r' || k.back() == ' ')) k.pop_back();
            info.kernel = k;
        }

        // 3. Compiler
        auto clang_path = Env::which("clang++");
        auto gcc_path   = Env::which("g++");
        if (clang_path) {
            info.compiler = "Clang";
            auto res = Process::run("\"" + Env::pathToUtf8(*clang_path) + "\" --version");
            if (res.success()) {
                std::istringstream iss(res.stdout_str);
                std::string first_line;
                std::getline(iss, first_line);
                info.compiler_version = first_line;
            }
        } else if (gcc_path) {
            info.compiler = "GCC";
            auto res = Process::run("\"" + Env::pathToUtf8(*gcc_path) + "\" --version");
            if (res.success()) {
                std::istringstream iss(res.stdout_str);
                std::string first_line;
                std::getline(iss, first_line);
                info.compiler_version = first_line;
            }
        }

        // 4. Graphics Subsystems (via pkg-config)
        info.has_wayland = (Process::run("pkg-config --exists wayland-client").exit_code == 0);
        info.has_x11     = (Process::run("pkg-config --exists x11 xrandr").exit_code == 0);
        info.has_drm     = (Process::run("pkg-config --exists libdrm gbm libinput").exit_code == 0);
        info.has_egl     = (Process::run("pkg-config --exists egl").exit_code == 0);

        // 5. Skia Engine
        fs::path root = repo_root.empty() ? Env::findEnkiRepoRoot() : repo_root;
        fs::path skia_file = root / "core" / "skia" / "out" / "Release-x64" / "libskia.a";
        std::error_code ec;
        if (fs::exists(skia_file, ec)) {
            info.has_skia = true;
            info.skia_path = "core/skia/out/Release-x64/libskia.a";
        }

        // 6. Active Session
        std::string wayland_disp = Env::get("WAYLAND_DISPLAY");
        std::string x11_disp     = Env::get("DISPLAY");
        if (!wayland_disp.empty()) {
            info.active_session = "Wayland (" + wayland_disp + ")";
        } else if (!x11_disp.empty()) {
            info.active_session = "X11 (" + x11_disp + ")";
        } else {
            info.active_session = "Direct Console (DRM/KMS)";
        }

        // 7. Group Permissions (video, render, input)
        int ngroups = getgroups(0, nullptr);
        if (ngroups > 0) {
            std::vector<gid_t> groups(ngroups);
            if (getgroups(ngroups, groups.data()) != -1) {
                for (gid_t gid : groups) {
                    struct group* grp = getgrgid(gid);
                    if (grp && grp->gr_name) {
                        std::string gname = grp->gr_name;
                        if (gname == "input")  info.in_input_group = true;
                        if (gname == "video")  info.in_video_group = true;
                        if (gname == "render") info.in_render_group = true;
                    }
                }
            }
        }
#endif
        return info;
    }
};

} // namespace enki::cli
