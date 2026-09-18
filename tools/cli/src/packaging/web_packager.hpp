#pragma once
/// @file web_packager.hpp
/// @brief WebAssembly Packager, HTML Shell generator, and Local Dev Server for Enki.

#include "core/project_config.hpp"
#include "core/terminal.hpp"
#include "core/process.hpp"
#include "core/env.hpp"
#include "toolchains/wasm_detector.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <thread>
#include <chrono>
#if defined(_WIN32)
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

namespace enki::cli {

class WebPackager {
public:
    static int findAvailablePort(int start_port) {
        int port = start_port;
        for (int i = 0; i < 50; ++i, ++port) {
#if defined(_WIN32)
            SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock == INVALID_SOCKET) continue;
            sockaddr_in addr{};
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
            addr.sin_port = htons(port);
            if (bind(sock, (sockaddr*)&addr, sizeof(addr)) == 0) {
                closesocket(sock);
                return port;
            }
            closesocket(sock);
#else
            int sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock < 0) continue;
            sockaddr_in addr{};
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
            addr.sin_port = htons(port);
            if (bind(sock, (sockaddr*)&addr, sizeof(addr)) == 0) {
                close(sock);
                return port;
            }
            close(sock);
#endif
        }
        return start_port;
    }

    static bool buildAndServe(
        const AppMetadata& app,
        const WasmToolchain& tc,
        const fs::path& repo_root,
        int port = 8080,
        bool auto_open = true)
    {
        std::error_code ec;
        fs::path build_dir = repo_root / "build-wasm";
        fs::path cross_file = repo_root / "cross" / "wasm.ini";

        // Ensure Emscripten and Node binaries are in PATH and EMSDK is set
        if (!tc.emsdk_root.empty()) {
            fs::path em_bin = tc.emsdk_root / "upstream" / "emscripten";
            std::string cur_path = Env::get("PATH");
            std::string path_prefix = Env::pathToUtf8(em_bin) + ":" + Env::pathToUtf8(tc.emsdk_root);
            for (const auto& entry : fs::directory_iterator(tc.emsdk_root / "node", ec)) {
                if (entry.is_directory(ec)) {
                    fs::path node_bin = entry.path() / "bin";
                    if (fs::exists(node_bin, ec)) {
                        path_prefix = Env::pathToUtf8(node_bin) + ":" + path_prefix;
                        break;
                    }
                }
            }
            if (cur_path.find(Env::pathToUtf8(em_bin)) == std::string::npos) {
                Env::set("PATH", path_prefix + ":" + cur_path);
            }
            Env::set("EMSDK", Env::pathToUtf8(tc.emsdk_root));
        }

        Terminal::step(1, 4, "Configuring WebAssembly build directory (Meson)...");

        if (!fs::exists(build_dir / "build.ninja", ec)) {
            std::string setup_cmd = "meson setup \"" + Env::pathToUtf8(build_dir) + "\""
                                  + " --cross-file \"" + Env::pathToUtf8(cross_file) + "\""
                                  + " -Dbuild_tests=false -Dbuild_examples=false";
            auto setup_res = Process::run(setup_cmd);
            if (!setup_res.success()) {
                Terminal::fail("Meson Setup Failed", setup_res.stderr_str);
                return false;
            }
        }

        Terminal::step(2, 4, "Compiling " + app.title + " to WebAssembly (.wasm / .js)...");
        std::string exe_target = "real_app/" + app.name + "/enki_" + app.name + ".js";
        std::string build_cmd = "ninja -C \"" + Env::pathToUtf8(build_dir) + "\" " + exe_target;
        auto build_res = Process::run(build_cmd);
        if (!build_res.success()) {
            Terminal::fail("Ninja Compilation Failed", build_res.stderr_str);
            return false;
        }

        Terminal::step(3, 4, "Generating Modern HTML5 Shell for " + app.title + "...");
        fs::path html_path = build_dir / "real_app" / app.name / "index.html";
        if (!generateHtmlShell(html_path, app)) {
            Terminal::fail("HTML Generation Failed", "Could not generate " + Env::pathToUtf8(html_path));
            return false;
        }

        port = findAvailablePort(port);
        Terminal::step(4, 4, "Starting Local Development Web Server on port " + std::to_string(port) + "...");
        fs::path serve_dir = build_dir / "real_app" / app.name;
        std::string url = "http://localhost:" + std::to_string(port);

        std::cout << "\n";
        Terminal::ok("WebAssembly App Ready", "Serving at " + Terminal::style(url, Color::BrightCyan, true));
        std::cout << "  • Press " << Terminal::style("Ctrl+C", Color::Bold) << " to stop the server\n\n";

        if (auto_open) {
            openBrowser(url);
        }

        // Launch Python http.server
        std::string server_cmd = "python3 -m http.server " + std::to_string(port)
                               + " --directory \"" + Env::pathToUtf8(serve_dir) + "\"";
        system(server_cmd.c_str());

        return true;
    }

private:
    static bool generateHtmlShell(const fs::path& dest_path, const AppMetadata& app) {
        std::error_code ec;
        fs::create_directories(dest_path.parent_path(), ec);

        std::ofstream out(dest_path);
        if (!out.is_open()) return false;

        out << R"html(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
    <title>)html" << app.title << R"html( — ENKI WebAssembly</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        html, body {
            width: 100%;
            height: 100%;
            background-color: #0F172A;
            color: #F8FAFC;
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Inter, sans-serif;
            overflow: hidden;
            display: flex;
            align-items: center;
            justify-content: center;
        }
        #canvas-container {
            position: relative;
            width: 100%;
            height: 100%;
            display: flex;
            align-items: center;
            justify-content: center;
        }
        canvas#canvas {
            display: block;
            width: 100%;
            height: 100%;
            outline: none;
            background-color: #0F172A;
        }
        #loading-overlay {
            position: absolute;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            gap: 16px;
            background: rgba(15, 23, 42, 0.95);
            width: 100%;
            height: 100%;
            z-index: 100;
            transition: opacity 0.3s ease-out;
        }
        .spinner {
            width: 48px;
            height: 48px;
            border: 4px solid rgba(0, 229, 255, 0.2);
            border-top-color: #00E5FF;
            border-radius: 50%;
            animation: spin 0.8s linear infinite;
        }
        @keyframes spin {
            to { transform: rotate(360deg); }
        }
        .loading-title {
            font-size: 1.25rem;
            font-weight: 600;
            letter-spacing: -0.02em;
            color: #FFFFFF;
        }
        .loading-subtitle {
            font-size: 0.875rem;
            color: #94A3B8;
        }
        /* Hidden input for mobile keyboard / IME bridge */
        #enki-ime-bridge {
            position: absolute;
            left: -9999px;
            top: -9999px;
            opacity: 0;
            pointer-events: none;
        }
    </style>
</head>
<body>
    <div id="canvas-container">
        <div id="loading-overlay">
            <div class="spinner"></div>
            <div class="loading-title">ENKI Native Engine</div>
            <div class="loading-subtitle">Launching )html" << app.title << R"html( on WebAssembly...</div>
        </div>
        <canvas id="canvas" oncontextmenu="event.preventDefault()" tabindex="-1"></canvas>
        <input type="text" id="enki-ime-bridge" autocomplete="off" autocorrect="off" autocapitalize="off" spellcheck="false" />
    </div>

    <script>
        var Module = {
            preRun: [],
            postRun: [function() {
                var overlay = document.getElementById("loading-overlay");
                if (overlay) {
                    overlay.style.opacity = "0";
                    setTimeout(function() { overlay.style.display = "none"; }, 300);
                }
            }],
            canvas: (function() {
                var canvas = document.getElementById("canvas");
                return canvas;
            })(),
            setStatus: function(text) {
                if (text) {
                    console.log("[ENKI Wasm Status] " + text);
                    var sub = document.querySelector(".loading-subtitle");
                    if (sub && text !== "Running...") sub.innerText = text;
                }
            },
            onAbort: function(what) {
                console.error("[ENKI Wasm Abort] " + what);
                var sub = document.querySelector(".loading-subtitle");
                if (sub) {
                    sub.innerText = "Runtime Error: " + what;
                    sub.style.color = "#EF4444";
                }
            },
            totalDependencies: 0,
            monitorRunDependencies: function(left) {
                this.totalDependencies = Math.max(this.totalDependencies, left);
            }
        };
        window.addEventListener("error", function(e) {
            console.error("[ENKI Window Error]", e);
            var sub = document.querySelector(".loading-subtitle");
            if (sub) {
                sub.innerText = "Error: " + (e.message || e);
                sub.style.color = "#EF4444";
            }
        });
    </script>
    <script async src="enki_)html" << app.name << R"html(.js"></script>
</body>
</html>)html";
        return true;
    }

    static void openBrowser(const std::string& url) {
#if defined(_WIN32)
        std::string cmd = "start \"\" \"" + url + "\"";
#elif defined(__APPLE__)
        std::string cmd = "open \"" + url + "\"";
#else
        std::string cmd = "xdg-open \"" + url + "\" 2>/dev/null &";
#endif
        system(cmd.c_str());
    }
};

} // namespace enki::cli
