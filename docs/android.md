# Android Support for enki

This document explains how to build `enki` for Android using the NDK and Meson.

---

## Prerequisites

| Tool | Minimum Version | Notes |
|------|----------------|-------|
| Android NDK | r27+ | Download from [developer.android.com/ndk/downloads](https://developer.android.com/ndk/downloads) |
| Meson | 1.3+ | `pip install meson` |
| Ninja | 1.11+ | `pip install ninja` |
| Skia (arm64 build) | chrome/m124+ | Built from source with the NDK (see below) |

---

## 1. Build Skia for Android arm64

```bash
cd core/skia

# Fetch the depot_tools sync scripts if not already present
python tools/git-sync-deps

# Generate the GN build config for Android arm64
bin/gn gen out/Release-android-arm64 --args='
  target_os="android"
  target_cpu="arm64"
  ndk="/path/to/android-ndk-r27"
  ndk_api=26
  is_official_build=true
  is_component_build=false
  skia_use_vulkan=false
  skia_use_gl=true
  skia_use_egl=true
  skia_use_fontconfig=false
  skia_use_freetype=true
  skia_use_expat=false
  skia_enable_skottie=true
  skia_enable_svg=true
'

ninja -C out/Release-android-arm64 skia skparagraph skshaper skunicode
```

---

## 2. Configure the cross-file

Edit `cross/android-arm64.ini` and set `ndk_root` to your NDK installation path:

```ini
[constants]
ndk_root  = '/home/user/android-ndk-r27'   # <-- change this
api_level = '26'
...
```

---

## 3. Build enki for Android

```bash
# Configure (from the enki root)
meson setup build-android \
  --cross-file cross/android-arm64.ini \
  --buildtype release \
  -Dprefix=/tmp/enki-android

# Build
ninja -C build-android
```

The output is a **shared library** (`libenki.so`) that you link into your
Android APK via the JNI bridge.

---

## 4. Application Entry Point

On Android you define `enki_android_main()` instead of `main()`:

```cpp
// my_app.cpp
#include <enki/app/app.hpp>
#include <enki/platform/platform.hpp>

extern "C" int enki_android_main(enki::Platform& platform) {
    auto app = enki::App::create(platform, MyRootWidget());

    while (platform.pollEvents()) {
        app->render();
    }

    return 0;
}
```

The `ANativeActivity_onCreate` entry point is provided automatically by
`android_app_glue.cpp`. Your APK manifest should declare:

```xml
<activity android:name="android.app.NativeActivity"
          android:label="My App">
  <meta-data android:name="android.app.lib_name"
             android:value="enki_myapp" />
</activity>
```

## 5. Android Permissions System

`enki` provides a unified, cross-platform runtime permissions API (`enki::Permissions`) designed specifically for Android NativeActivity apps with zero Java code required.

### 5.1 Declare Permissions in `AndroidManifest.xml`

Declare the permissions your application needs inside `<manifest>`:

```xml
<manifest xmlns:android="http://schemas.android.com/apk/res/android"
    package="com.example.enki_app">

    <!-- ── Network ─────────────────────────────────────────────── -->
    <uses-permission android:name="android.permission.INTERNET" />
    <uses-permission android:name="android.permission.ACCESS_NETWORK_STATE" />

    <!-- ── Storage & Media (Android 13+ API 33 & legacy) ───────── -->
    <!-- Android 13+ (API 33+) granular media access -->
    <uses-permission android:name="android.permission.READ_MEDIA_IMAGES" />
    <uses-permission android:name="android.permission.READ_MEDIA_VIDEO" />
    <uses-permission android:name="android.permission.READ_MEDIA_AUDIO" />
    <!-- Android 12 and below (API <= 32) fallback -->
    <uses-permission android:name="android.permission.READ_EXTERNAL_STORAGE" android:maxSdkVersion="32" />
    <uses-permission android:name="android.permission.WRITE_EXTERNAL_STORAGE" android:maxSdkVersion="29" />

    <!-- ── Notifications (Android 13+ API 33) ──────────────────── -->
    <uses-permission android:name="android.permission.POST_NOTIFICATIONS" />

    <!-- ── Location ────────────────────────────────────────────── -->
    <uses-permission android:name="android.permission.ACCESS_FINE_LOCATION" />
    <uses-permission android:name="android.permission.ACCESS_COARSE_LOCATION" />
    <!-- Optional: Background location (API 29+) -->
    <!-- <uses-permission android:name="android.permission.ACCESS_BACKGROUND_LOCATION" /> -->

    <!-- ── Hardware & Sensors ─────────────────────────────────── -->
    <uses-permission android:name="android.permission.CAMERA" />
    <uses-permission android:name="android.permission.RECORD_AUDIO" />
    <uses-permission android:name="android.permission.BODY_SENSORS" />

    <!-- ── Bluetooth (Android 12+ API 31 & legacy) ─────────────── -->
    <uses-permission android:name="android.permission.BLUETOOTH_SCAN"
                     android:usesPermissionFlags="neverForLocation" />
    <uses-permission android:name="android.permission.BLUETOOTH_CONNECT" />
    <uses-permission android:name="android.permission.BLUETOOTH" android:maxSdkVersion="30" />
    <uses-permission android:name="android.permission.BLUETOOTH_ADMIN" android:maxSdkVersion="30" />

    <!-- ── Contacts & Calendar ────────────────────────────────── -->
    <uses-permission android:name="android.permission.READ_CONTACTS" />
    <uses-permission android:name="android.permission.WRITE_CONTACTS" />
    <uses-permission android:name="android.permission.READ_CALENDAR" />
    <uses-permission android:name="android.permission.WRITE_CALENDAR" />

    <application ...>
        <activity android:name="android.app.NativeActivity" ...>
            <meta-data android:name="android.app.lib_name" android:value="enki_myapp" />
        </activity>
    </application>
</manifest>
```

### 5.2 C++ Usage Examples

#### Checking and Requesting a Single Permission

```cpp
#include <enki/platform/permissions.hpp>

// Synchronous check
if (enki::Permissions::isGranted(enki::Permission::Storage)) {
    loadUserMedia();
} else {
    // Request permission asynchronously
    enki::Permissions::request(enki::Permission::Storage, [](enki::PermissionStatus status) {
        if (status == enki::PermissionStatus::Granted) {
            loadUserMedia();
        } else if (status == enki::PermissionStatus::PermanentlyDenied) {
            // User selected "Don't ask again" — direct to system settings
            enki::Permissions::openAppSettings();
        }
    });
}
```

#### Requesting Multiple Permissions

```cpp
enki::Permissions::request({
    enki::Permission::Location,
    enki::Permission::Notifications,
    enki::Permission::Camera
}, [](const std::unordered_map<enki::Permission, enki::PermissionStatus>& results) {
    for (const auto& [perm, status] : results) {
        ENKI_ALOG("Permission %s: %s",
                  enki::Permissions::toString(perm),
                  enki::Permissions::toString(status));
    }
});
```

#### Showing Rationale

```cpp
if (enki::Permissions::shouldShowRationale(enki::Permission::Location)) {
    // Show your app's explanation dialog explaining why GPS is needed,
    // then call enki::Permissions::request(...)
}
```

### 5.3 Desktop Compatibility
On Windows and Linux, `enki::Permissions::check()` and `enki::Permissions::request()` automatically return `PermissionStatus::Granted`, allowing multiplatform applications to use the exact same code without `#ifdef` guards.

---

## File Map

| File | Purpose |
|------|---------|
| `include/enki/platform/permissions.hpp` | Cross-platform runtime permissions public API |
| `src/platform/permissions.cpp` | Permissions implementation & Desktop stubs |
| `include/enki/platform/android/android_platform.hpp` | Backend public interface & JNI permission helpers |
| `include/enki/platform/android/android_surface.hpp` | Surface (window) public interface |
| `src/platform/android/android_platform.cpp` | EGL init, ALooper, input, clipboard, JNI permissions |
| `src/platform/android/android_surface.cpp` | EGL surface lifecycle |
| `src/platform/android/android_app_glue.cpp` | `ANativeActivity_onCreate`, callbacks |
| `cross/android-arm64.ini` | Meson NDK cross-compilation toolchain |

