#!/usr/bin/env python3
"""
package_calculator_apk.py — Packages the ENKI Calculator app into a signed APK
and deploys it to a connected Android device/emulator.

Usage:
    python3 scripts/package_calculator_apk.py
"""

import os
import sys
import subprocess
import zipfile
import shutil

# ── SDK / NDK paths ──────────────────────────────────────────────────────────
SDK_ROOT     = r"C:\Users\x\AppData\Local\Android\Sdk"
NDK_ROOT     = r"C:\Users\x\AppData\Local\Android\Sdk\ndk\30.0.16248370"
JAVA_HOME    = r"C:\Program Files\Android\Android Studio\jbr"
BUILD_TOOLS  = os.path.join(SDK_ROOT, "build-tools", "36.0.0")
PLATFORMS    = os.path.join(SDK_ROOT, "platforms", "android-37.0")
PLATFORM_TOOLS = os.path.join(SDK_ROOT, "platform-tools")

AAPT2       = os.path.join(BUILD_TOOLS, "aapt2.exe")
ZIPALIGN    = os.path.join(BUILD_TOOLS, "zipalign.exe")
APKSIGNER   = os.path.join(BUILD_TOOLS, "apksigner.bat")
ANDROID_JAR = os.path.join(PLATFORMS, "android.jar")
ADB         = os.path.join(PLATFORM_TOOLS, "adb.exe")
KEYSTORE    = os.path.expanduser(r"~\.android\debug.keystore")

LIBCXX_SO = os.path.join(
    NDK_ROOT,
    "toolchains", "llvm", "prebuilt", "windows-x86_64",
    "sysroot", "usr", "lib", "aarch64-linux-android",
    "libc++_shared.so"
)

# ── Project paths ─────────────────────────────────────────────────────────────
PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
BUILD_DIR    = os.path.join(PROJECT_ROOT, "build-android")
SO_PATH      = os.path.join(BUILD_DIR, "real_app", "calculator", "libenki_calculator.so")
STAGING_DIR  = os.path.join(BUILD_DIR, "apk_staging_calculator")

os.environ["JAVA_HOME"] = JAVA_HOME
os.environ["PATH"] = os.path.join(JAVA_HOME, "bin") + ";" + os.environ["PATH"]

# ── AndroidManifest.xml ───────────────────────────────────────────────────────
MANIFEST = """\
<?xml version="1.0" encoding="utf-8"?>
<manifest xmlns:android="http://schemas.android.com/apk/res/android"
    package="org.enki.quantum_calculator"
    android:versionCode="1"
    android:versionName="1.0">

    <uses-sdk android:minSdkVersion="26" android:targetSdkVersion="34" />
    <uses-feature android:glEsVersion="0x00030000" android:required="true" />

    <application
        android:label="ENKI Calculator"
        android:hasCode="false">
        <activity
            android:name="android.app.NativeActivity"
            android:label="ENKI Calculator"
            android:launchMode="singleTask"
            android:clearTaskOnLaunch="false"
            android:alwaysRetainTaskState="true"
            android:configChanges="orientation|keyboardHidden|screenSize|screenLayout|smallestScreenSize|uiMode"
            android:exported="true">
            <meta-data
                android:name="android.app.lib_name"
                android:value="enki_calculator" />
            <intent-filter>
                <action android:name="android.intent.action.MAIN" />
                <category android:name="android.intent.category.LAUNCHER" />
            </intent-filter>
        </activity>
    </application>
</manifest>
"""

def main():
    print("=== Packaging ENKI Calculator App for Android ===")

    # ── Sanity check ─────────────────────────────────────────────────────────
    if not os.path.isfile(SO_PATH):
        print(f"ERROR: {SO_PATH} not found.")
        print("Run:  ninja -C build-android real_app/calculator/libenki_calculator.so")
        sys.exit(1)

    os.makedirs(STAGING_DIR, exist_ok=True)

    # ── [1/5] Write manifest and link base APK ────────────────────────────────
    manifest_path = os.path.join(STAGING_DIR, "AndroidManifest.xml")
    with open(manifest_path, "w", encoding="utf-8") as f:
        f.write(MANIFEST)

    base_apk = os.path.join(STAGING_DIR, "base.apk")
    print("[1/5] Linking base APK with aapt2...")
    subprocess.check_call([
        AAPT2, "link",
        "-o", base_apk,
        "-I", ANDROID_JAR,
        "--manifest", manifest_path,
    ])

    # ── [2/5] Inject native libraries ────────────────────────────────────────
    unaligned_apk = os.path.join(STAGING_DIR, "unaligned.apk")
    shutil.copyfile(base_apk, unaligned_apk)

    print("[2/5] Adding native libraries into APK (arm64-v8a)...")
    with zipfile.ZipFile(unaligned_apk, "a", compression=zipfile.ZIP_DEFLATED) as zf:
        zf.write(SO_PATH,    "lib/arm64-v8a/libenki_calculator.so")
        zf.write(LIBCXX_SO,  "lib/arm64-v8a/libc++_shared.so")
        print(f"  Added libenki_calculator.so ({os.path.getsize(SO_PATH):,} bytes)")
        print(f"  Added libc++_shared.so      ({os.path.getsize(LIBCXX_SO):,} bytes)")

    # ── [3/5] Align ───────────────────────────────────────────────────────────
    aligned_apk = os.path.join(STAGING_DIR, "aligned.apk")
    if os.path.exists(aligned_apk):
        os.remove(aligned_apk)

    print("[3/5] Aligning APK with zipalign (4-byte alignment)...")
    subprocess.check_call(
        [ZIPALIGN, "-v", "-p", "4", unaligned_apk, aligned_apk],
        stdout=subprocess.DEVNULL,
    )

    # ── [4/5] Sign ────────────────────────────────────────────────────────────
    final_apk = os.path.join(BUILD_DIR, "enki_calculator.apk")
    if os.path.exists(final_apk):
        os.remove(final_apk)

    print("[4/5] Signing APK with apksigner...")
    subprocess.check_call([
        APKSIGNER, "sign",
        "--ks",          KEYSTORE,
        "--ks-pass",     "pass:android",
        "--ks-key-alias","androiddebugkey",
        "--key-pass",    "pass:android",
        "--out",         final_apk,
        aligned_apk,
    ])
    print(f"  Signed APK: {final_apk} ({os.path.getsize(final_apk):,} bytes)")

    # ── [5/5] Deploy ──────────────────────────────────────────────────────────
    print("[5/5] Deploying to Android device...")
    subprocess.check_call([ADB, "install", "-r", final_apk])
    subprocess.check_call([ADB, "logcat", "-c"])
    subprocess.check_call([
        ADB, "shell", "am", "start",
        "-n", "org.enki.quantum_calculator/android.app.NativeActivity",
    ])

    print("=== Deployment Complete! ENKI Calculator launched on Android! ===")

if __name__ == "__main__":
    main()
