#pragma once
/// @file android_detector.hpp
/// @brief Auto-discovery of Android SDK, NDK, Build-Tools, Platforms, and ADB.

#include "core/env.hpp"
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>

namespace enki::cli {

struct AndroidToolchain {
    bool found = false;
    fs::path sdk_root;
    fs::path ndk_root;
    fs::path java_home;
    fs::path adb_path;
    fs::path aapt2_path;
    fs::path zipalign_path;
    fs::path apksigner_path;
    fs::path android_jar_path;
    fs::path debug_keystore;
    fs::path libcxx_shared;
    std::string build_tools_version;
    std::string ndk_version;
    std::string platform_version;
};

class AndroidDetector {
public:
    static AndroidToolchain detect() {
        AndroidToolchain tc;
        std::error_code ec;

        // 1. Locate Android SDK Root
        std::string sdk_env = Env::get("ANDROID_HOME");
        if (sdk_env.empty()) sdk_env = Env::get("ANDROID_SDK_ROOT");
        if (sdk_env.empty()) {
#if defined(_WIN32)
            std::string local_app_data = Env::get("LOCALAPPDATA");
            if (!local_app_data.empty()) {
                sdk_env = local_app_data + "\\Android\\Sdk";
            }
#else
            std::string home = Env::get("HOME");
            if (!home.empty()) {
                sdk_env = home + "/Android/Sdk";
            }
#endif
        }

        if (sdk_env.empty() || !fs::exists(Env::stringToPath(sdk_env), ec)) {
            return tc;
        }
        tc.sdk_root = Env::stringToPath(sdk_env);

        // 2. Locate ADB
        fs::path adb = tc.sdk_root / "platform-tools" / (
#if defined(_WIN32)
            "adb.exe"
#else
            "adb"
#endif
        );
        if (fs::exists(adb, ec)) {
            tc.adb_path = adb;
        } else {
            auto which_adb = Env::which("adb");
            if (which_adb) tc.adb_path = *which_adb;
        }

        // 3. Locate Build-Tools (Find highest version folder)
        fs::path bt_dir = tc.sdk_root / "build-tools";
        if (fs::exists(bt_dir, ec)) {
            std::vector<fs::path> versions;
            for (const auto& entry : fs::directory_iterator(bt_dir, ec)) {
                if (entry.is_directory(ec)) versions.push_back(entry.path());
            }
            std::sort(versions.begin(), versions.end());
            if (!versions.empty()) {
                fs::path latest_bt = versions.back();
                tc.build_tools_version = Env::pathToUtf8(latest_bt.filename());

                fs::path aapt2 = latest_bt / (
#if defined(_WIN32)
                    "aapt2.exe"
#else
                    "aapt2"
#endif
                );
                if (fs::exists(aapt2, ec)) tc.aapt2_path = aapt2;

                fs::path zipalign = latest_bt / (
#if defined(_WIN32)
                    "zipalign.exe"
#else
                    "zipalign"
#endif
                );
                if (fs::exists(zipalign, ec)) tc.zipalign_path = zipalign;

                fs::path apksigner = latest_bt / (
#if defined(_WIN32)
                    "apksigner.bat"
#else
                    "apksigner"
#endif
                );
                if (fs::exists(apksigner, ec)) tc.apksigner_path = apksigner;
            }
        }

        // 4. Locate android.jar (Find latest platform)
        fs::path platforms_dir = tc.sdk_root / "platforms";
        if (fs::exists(platforms_dir, ec)) {
            std::vector<fs::path> plats;
            for (const auto& entry : fs::directory_iterator(platforms_dir, ec)) {
                if (entry.is_directory(ec) && fs::exists(entry.path() / "android.jar", ec)) {
                    plats.push_back(entry.path());
                }
            }
            std::sort(plats.begin(), plats.end());
            if (!plats.empty()) {
                tc.platform_version = Env::pathToUtf8(plats.back().filename());
                tc.android_jar_path = plats.back() / "android.jar";
            }
        }

        // 5. Locate NDK
        std::string ndk_env = Env::get("ANDROID_NDK_HOME");
        if (ndk_env.empty()) ndk_env = Env::get("ANDROID_NDK_ROOT");
        if (!ndk_env.empty() && fs::exists(Env::stringToPath(ndk_env), ec)) {
            tc.ndk_root = Env::stringToPath(ndk_env);
            tc.ndk_version = Env::pathToUtf8(tc.ndk_root.filename());
        } else {
            fs::path ndk_dir = tc.sdk_root / "ndk";
            if (fs::exists(ndk_dir, ec)) {
                std::vector<fs::path> ndks;
                for (const auto& entry : fs::directory_iterator(ndk_dir, ec)) {
                    if (entry.is_directory(ec)) ndks.push_back(entry.path());
                }
                std::sort(ndks.begin(), ndks.end());
                if (!ndks.empty()) {
                    tc.ndk_root = ndks.back();
                    tc.ndk_version = Env::pathToUtf8(tc.ndk_root.filename());
                }
            }
        }

        // 6. Locate JAVA_HOME / JBR
        std::string jh = Env::get("JAVA_HOME");
        if (!jh.empty() && fs::exists(Env::stringToPath(jh), ec)) {
            tc.java_home = Env::stringToPath(jh);
        } else {
            std::vector<std::string> java_candidates = {
                "C:\\Program Files\\Android\\Android Studio\\jbr",
                "C:\\Program Files\\Java\\jdk-21",
                "C:\\Program Files\\Java\\jdk-17",
                "C:\\Program Files\\Eclipse Adoptium\\jdk-17-hotspot",
                "/usr/lib/jvm/default-java"
            };
            for (const auto& jc : java_candidates) {
                if (fs::exists(Env::stringToPath(jc), ec)) {
                    tc.java_home = Env::stringToPath(jc);
                    break;
                }
            }
        }

        // 7. Locate Debug Keystore
        std::string user_prof = Env::get("USERPROFILE");
        if (user_prof.empty()) user_prof = Env::get("HOME");
        if (!user_prof.empty()) {
            fs::path ks = Env::stringToPath(user_prof) / ".android" / "debug.keystore";
            if (fs::exists(ks, ec)) {
                tc.debug_keystore = ks;
            }
        }

        // 8. Locate libc++_shared.so
        if (!tc.ndk_root.empty()) {
            std::vector<fs::path> libcxx_paths = {
                tc.ndk_root / "toolchains" / "llvm" / "prebuilt" / "windows-x86_64" / "sysroot" / "usr" / "lib" / "aarch64-linux-android" / "libc++_shared.so",
                tc.ndk_root / "toolchains" / "llvm" / "prebuilt" / "linux-x86_64" / "sysroot" / "usr" / "lib" / "aarch64-linux-android" / "libc++_shared.so",
                tc.ndk_root / "sources" / "cxx-stl" / "llvm-libc++" / "libs" / "arm64-v8a" / "libc++_shared.so"
            };
            for (const auto& lp : libcxx_paths) {
                if (fs::exists(lp, ec)) {
                    tc.libcxx_shared = lp;
                    break;
                }
            }
        }

        tc.found = !tc.adb_path.empty() && !tc.aapt2_path.empty() && !tc.android_jar_path.empty();
        return tc;
    }
};

} // namespace enki::cli
