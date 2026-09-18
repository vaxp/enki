#pragma once
/// @file wasm_detector.hpp
/// @brief Auto-discovery of Emscripten SDK (emcc, em++, emsdk) and WebAssembly toolchains.

#include "core/env.hpp"
#include "core/process.hpp"
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>

namespace enki::cli {

struct WasmToolchain {
    bool found = false;
    fs::path emsdk_root;
    fs::path emcc_path;
    fs::path empp_path;
    std::string emcc_version;
    bool has_skia_wasm = false;
};

class WasmDetector {
public:
    static WasmToolchain detect(const fs::path& repo_root = {}) {
        WasmToolchain tc;
        std::error_code ec;

        // 1. Check EMSDK environment variable
        std::string emsdk_env = Env::get("EMSDK");
        if (!emsdk_env.empty()) {
            tc.emsdk_root = Env::stringToPath(emsdk_env);
        } else {
            // Check common emsdk paths
#if defined(_WIN32)
            std::vector<std::string> candidates = {
                "C:\\emsdk",
                Env::get("USERPROFILE") + "\\emsdk",
                Env::get("LOCALAPPDATA") + "\\emsdk"
            };
#else
            std::vector<std::string> candidates = {
                Env::get("HOME") + "/emsdk",
                "/opt/emsdk",
                "/usr/lib/emscripten"
            };
#endif
            for (const auto& c : candidates) {
                if (!c.empty() && fs::exists(Env::stringToPath(c), ec)) {
                    tc.emsdk_root = Env::stringToPath(c);
                    break;
                }
            }
        }

        // 2. Locate emcc and em++
        auto findInPathOrDir = [&](const std::string& name) -> fs::path {
            auto in_path = Env::which(name);
            if (in_path) return *in_path;

            if (!tc.emsdk_root.empty()) {
                fs::path p = tc.emsdk_root / "upstream" / "emscripten" / (
#if defined(_WIN32)
                    name + ".bat"
#else
                    name
#endif
                );
                if (fs::exists(p, ec)) return p;
            }
            return {};
        };

        tc.emcc_path = findInPathOrDir("emcc");
        tc.empp_path = findInPathOrDir("em++");

        if (!tc.emcc_path.empty() && !tc.empp_path.empty()) {
            tc.found = true;
            // Query emcc version
            auto res = Process::run("\"" + Env::pathToUtf8(tc.emcc_path) + "\" --version");
            if (res.success()) {
                auto first_newline = res.stdout_str.find('\n');
                tc.emcc_version = (first_newline != std::string::npos)
                    ? res.stdout_str.substr(0, first_newline)
                    : res.stdout_str;
                // Trim trailing carriage return
                while (!tc.emcc_version.empty() && (tc.emcc_version.back() == '\r' || tc.emcc_version.back() == ' ')) {
                    tc.emcc_version.pop_back();
                }
            }
        }

        // 3. Check Skia Wasm build in repository
        if (!repo_root.empty()) {
            fs::path skia_wasm = repo_root / "core" / "skia" / "out" / "Release-wasm" / "libskia.a";
            tc.has_skia_wasm = fs::exists(skia_wasm, ec);
        }

        return tc;
    }
};

} // namespace enki::cli
