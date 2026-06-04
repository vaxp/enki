#pragma once
/// @file android_packager.hpp
/// @brief Native automated packaging, signing, and deployment engine for Android APKs.

#include "toolchains/android_detector.hpp"
#include "core/project_config.hpp"
#include "core/terminal.hpp"
#include "core/process.hpp"
#include "core/env.hpp"
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace enki::cli {

class AndroidPackager {
public:
    static std::string extractPackageName(const fs::path& manifest_path, const std::string& fallback) {
        std::ifstream ifs(manifest_path);
        if (!ifs.is_open()) return fallback;
        std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        auto pos = content.find("package=\"");
        if (pos != std::string::npos) {
            auto end = content.find("\"", pos + 9);
            if (end != std::string::npos) {
                return content.substr(pos + 9, end - (pos + 9));
            }
        }
        return fallback;
    }

    static bool buildAndDeploy(
        const AppMetadata& app,
        const AndroidToolchain& tc,
        const fs::path& repo_root,
        const std::string& target_device_id = "",
        bool stream_logs = true
    ) {
        if (!tc.found) {
            Terminal::fail("Android Toolchain Missing", "Run 'enki doctor' for installation steps.");
            return false;
        }

        fs::path build_dir = repo_root / "build-android";
        std::error_code ec;
        if (!fs::exists(build_dir, ec)) {
            Terminal::fail("Build Directory Missing", "build-android directory does not exist.");
            return false;
        }

        // 1. Build C++ Shared Library using Ninja
        Terminal::step(1, 6, "Compiling C++ shared library (" + app.name + ")");
        std::string ninja_target = "real_app/" + app.name + "/libenki_" + app.name + ".so";
        auto ninja_path = Env::findNinja();
        std::string ninja_bin = ninja_path ? Env::pathToUtf8(*ninja_path) : "ninja";
        std::string ninja_cmd = "\"" + ninja_bin + "\" -C \"" + Env::pathToUtf8(build_dir) + "\" " + ninja_target;
        auto ninja_res = Process::run(ninja_cmd);
        if (!ninja_res.success()) {
            Terminal::fail("Ninja Build Failed", ninja_res.stderr_str.empty() ? ninja_res.stdout_str : ninja_res.stderr_str);
            return false;
        }

        // 2. Prepare AndroidManifest.xml (Use project manifest if present, else generate)
        Terminal::step(2, 6, "Configuring AndroidManifest & linking base APK");
        fs::path staging_dir = build_dir / ("apk_staging_" + app.name);
        fs::create_directories(staging_dir, ec);

        fs::path manifest_path = staging_dir / "AndroidManifest.xml";
        fs::path app_manifest = app.dir_path / "AndroidManifest.xml";
        fs::path app_android_manifest = app.dir_path / "android" / "AndroidManifest.xml";

        if (fs::exists(app_manifest, ec)) {
            fs::copy_file(app_manifest, manifest_path, fs::copy_options::overwrite_existing, ec);
            Terminal::ok("Using App Manifest", Env::pathToUtf8(app_manifest.lexically_relative(repo_root)));
        } else if (fs::exists(app_android_manifest, ec)) {
            fs::copy_file(app_android_manifest, manifest_path, fs::copy_options::overwrite_existing, ec);
            Terminal::ok("Using App Manifest", Env::pathToUtf8(app_android_manifest.lexically_relative(repo_root)));
        } else {
            std::ofstream manifest_ofs(manifest_path);
            if (!manifest_ofs.is_open()) {
                Terminal::fail("Manifest Generation Failed", "Could not write " + Env::pathToUtf8(manifest_path));
                return false;
            }

            manifest_ofs << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
            manifest_ofs << "<manifest xmlns:android=\"http://schemas.android.com/apk/res/android\"\n";
            manifest_ofs << "    package=\"" << app.app_id << "\"\n";
            manifest_ofs << "    android:versionCode=\"1\"\n";
            manifest_ofs << "    android:versionName=\"" << app.version << "\">\n";
            manifest_ofs << "    <uses-sdk android:minSdkVersion=\"26\" android:targetSdkVersion=\"34\" />\n";
            manifest_ofs << "    <uses-feature android:glEsVersion=\"0x00030000\" android:required=\"true\" />\n";
            
            for (const auto& perm : app.permissions) {
                manifest_ofs << "    <uses-permission android:name=\"android.permission." << perm << "\" />\n";
            }

            manifest_ofs << "    <application android:label=\"" << app.title << "\" android:hasCode=\"false\">\n";
            manifest_ofs << "        <activity android:name=\"android.app.NativeActivity\"\n";
            manifest_ofs << "                  android:exported=\"true\"\n";
            manifest_ofs << "                  android:configChanges=\"orientation|keyboardHidden|screenSize\">\n";
            manifest_ofs << "            <meta-data android:name=\"android.app.lib_name\" android:value=\"enki_" << app.name << "\" />\n";
            manifest_ofs << "            <intent-filter>\n";
            manifest_ofs << "                <action android:name=\"android.intent.action.MAIN\" />\n";
            manifest_ofs << "                <category android:name=\"android.intent.category.LAUNCHER\" />\n";
            manifest_ofs << "            </intent-filter>\n";
            manifest_ofs << "        </activity>\n";
            manifest_ofs << "    </application>\n";
            manifest_ofs << "</manifest>\n";
            manifest_ofs.close();
            Terminal::ok("Generated Manifest", "Default NativeActivity template");
        }

        std::string final_package = extractPackageName(manifest_path, app.app_id);

        // 3. Link Base APK via AAPT2
        fs::path base_apk = staging_dir / "base.apk";
        fs::remove(base_apk, ec);

        std::string aapt_cmd = "\"" + Env::pathToUtf8(tc.aapt2_path) + "\" link"
            + " -o \"" + Env::pathToUtf8(base_apk) + "\""
            + " -I \"" + Env::pathToUtf8(tc.android_jar_path) + "\""
            + " --manifest \"" + Env::pathToUtf8(manifest_path) + "\""
            + " --min-sdk-version 26 --target-sdk-version 34";
        
        auto aapt_res = Process::run(aapt_cmd);
        if (!aapt_res.success()) {
            Terminal::fail("AAPT2 Link Failed", aapt_res.stdout_str + aapt_res.stderr_str);
            return false;
        }

        // 4. Inject Native Libraries (.so) into APK (arm64-v8a)
        Terminal::step(3, 6, "Injecting native binaries into APK (arm64-v8a)");
        fs::path unaligned_apk = staging_dir / "unaligned.apk";
        fs::copy_file(base_apk, unaligned_apk, fs::copy_options::overwrite_existing, ec);

        fs::path so_path = build_dir / "real_app" / app.name / ("libenki_" + app.name + ".so");
        if (!fs::exists(so_path, ec)) {
            Terminal::fail("Missing native library", Env::pathToUtf8(so_path));
            return false;
        }

        fs::path libcxx = tc.libcxx_shared;
        if (libcxx.empty() || !fs::exists(libcxx, ec)) {
            libcxx = repo_root / "cross" / "android-sysroot" / "libc++_shared.so";
        }

#if defined(_WIN32)
        std::string ps_cmd = "powershell -NoProfile -Command \""
            "Add-Type -AssemblyName System.IO.Compression, System.IO.Compression.FileSystem; "
            "$zip = [System.IO.Compression.ZipFile]::Open('" + Env::pathToUtf8(unaligned_apk) + "', [System.IO.Compression.ZipArchiveMode]::Update); "
            "[System.IO.Compression.ZipFileExtensions]::CreateEntryFromFile($zip, '" + Env::pathToUtf8(so_path) + "', 'lib/arm64-v8a/" + so_path.filename().string() + "'); ";
        if (fs::exists(libcxx, ec)) {
            ps_cmd += "[System.IO.Compression.ZipFileExtensions]::CreateEntryFromFile($zip, '" + Env::pathToUtf8(libcxx) + "', 'lib/arm64-v8a/libc++_shared.so'); ";
        }
        ps_cmd += "$zip.Dispose()\"";
        auto inject_res = Process::run(ps_cmd);
        if (!inject_res.success()) {
            Terminal::fail("Native Library Injection Failed", inject_res.stderr_str.empty() ? inject_res.stdout_str : inject_res.stderr_str);
            return false;
        }
#else
        fs::path lib_dir = staging_dir / "lib" / "arm64-v8a";
        fs::create_directories(lib_dir, ec);
        fs::copy_file(so_path, lib_dir / so_path.filename(), fs::copy_options::overwrite_existing, ec);
        if (fs::exists(libcxx, ec)) {
            fs::copy_file(libcxx, lib_dir / "libc++_shared.so", fs::copy_options::overwrite_existing, ec);
        }
        std::string zip_cmd = "cd \"" + Env::pathToUtf8(staging_dir) + "\" && zip -u unaligned.apk lib/arm64-v8a/*";
        Process::run(zip_cmd);
#endif

        // 5. Align APK via zipalign (4-byte boundary)
        Terminal::step(4, 6, "Aligning APK (4-byte alignment)");
        fs::path aligned_apk = staging_dir / "aligned.apk";
        fs::remove(aligned_apk, ec);

        std::string zipalign_cmd = "\"" + Env::pathToUtf8(tc.zipalign_path) + "\" -v -p 4 \""
            + Env::pathToUtf8(unaligned_apk) + "\" \"" + Env::pathToUtf8(aligned_apk) + "\"";
        auto align_res = Process::run(zipalign_cmd);
        if (!align_res.success()) {
            Terminal::fail("APK Zipalign Failed", align_res.stderr_str.empty() ? align_res.stdout_str : align_res.stderr_str);
            return false;
        }

        // 6. Sign APK via apksigner
        Terminal::step(5, 6, "Signing APK with debug keystore");
        fs::path final_apk = build_dir / ("enki_" + app.name + ".apk");
        fs::remove(final_apk, ec);

        fs::path keystore = tc.debug_keystore;
        if (keystore.empty() || !fs::exists(keystore, ec)) {
            Terminal::fail("Debug Keystore Missing", "Could not locate ~/.android/debug.keystore");
            return false;
        }

        std::string sign_cmd = "\"" + Env::pathToUtf8(tc.apksigner_path) + "\" sign"
            + " --ks \"" + Env::pathToUtf8(keystore) + "\""
            + " --ks-pass pass:android --ks-key-alias androiddebugkey --key-pass pass:android"
            + " --out \"" + Env::pathToUtf8(final_apk) + "\" \"" + Env::pathToUtf8(aligned_apk) + "\"";

#if defined(_WIN32)
        if (!tc.java_home.empty()) {
            std::string java_bin = Env::pathToUtf8(tc.java_home / "bin");
            sign_cmd = "cmd.exe /s /c \"set \"JAVA_HOME=" + Env::pathToUtf8(tc.java_home) + "\" && set \"PATH=" + java_bin + ";%PATH%\" && " + sign_cmd + "\"";
        }
#endif
        auto sign_res = Process::run(sign_cmd);
        if (!sign_res.success()) {
            Terminal::fail("APK Signing Failed", sign_res.stderr_str.empty() ? sign_res.stdout_str : sign_res.stderr_str);
            return false;
        }

        // 7. Deploy & Run
        Terminal::step(6, 6, "Deploying and Launching on Android device");
        std::string adb_prefix = "\"" + Env::pathToUtf8(tc.adb_path) + "\"";
        if (!target_device_id.empty() && target_device_id != "desktop") {
            adb_prefix += " -s " + target_device_id;
        }

        std::string install_cmd = adb_prefix + " install -r \"" + Env::pathToUtf8(final_apk) + "\"";
        auto install_res = Process::run(install_cmd);
        if (!install_res.success()) {
            Terminal::fail("ADB Install Failed", install_res.stdout_str + install_res.stderr_str);
            return false;
        }

        std::string launch_cmd = adb_prefix + " shell am start -n " + final_package + "/android.app.NativeActivity";
        Process::run(launch_cmd);

        Terminal::ok("Application Deployed & Running", final_package);

        if (stream_logs) {
            std::cout << "\n" << Terminal::style("=== Live Application Log Stream (Ctrl+C to stop) ===", Color::BrightCyan, true) << "\n";
            Process::run(adb_prefix + " logcat -c");
            std::string log_cmd = adb_prefix + " logcat -v time -s ENKI:V GalleryApp:V GalleryCubit:V GalleryWorker:V FullscreenViewer:V PhotoCard:V AndroidPlatform:V";
            Process::run(log_cmd, "", true);
        }

        return true;
    }
};

} // namespace enki::cli
