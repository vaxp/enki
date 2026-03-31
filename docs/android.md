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

---

## Architecture

```
ANativeActivity_onCreate()          [android_app_glue.cpp]
  └─ Platform::setAndroidActivity()
  └─ Platform::create()
       └─ AndroidPlatformBackend::init()
            ├─ EGL (shared context — survives rotation)
            ├─ ALooper (main thread event pump)
            └─ AConfiguration (DPI, locale)
  └─ enki_android_main()            [your application]
       └─ Window::create()
            └─ AndroidSurface::init()
                 └─ registers with backend
                      │
                      ├─ onNativeWindowCreated()  → createEGLSurface()
                      ├─ onNativeWindowDestroyed() → destroyEGLSurface()
                      └─ pollEvents()  → touch / key dispatch
```

### Key Design Decisions

- **EGL context is shared** — Only the EGL *surface* is destroyed/recreated on
  screen rotation, keeping GPU resources (textures, shaders, Skia GrContext)
  alive across configuration changes.
- **Single surface** — Android always has one full-screen window; there is no
  window manager. `AndroidSurface` is always fullscreen.
- **JNI clipboard** — Text clipboard is bridged to Android's `ClipboardManager`
  via JNI. Other MIME types fall back to an in-process buffer.
- **No cursor / drag-and-drop** — These are silently no-ops since Android has
  no desktop-style mouse or DnD protocol.
- **Strict isolation** — Every Android-specific code path is wrapped in
  `#if defined(__ANDROID__)` / `#elif defined(_WIN32)` / `#else` chains.
  Linux (X11/Wayland) and Windows builds are **completely unaffected**.

---

## File Map

| File | Purpose |
|------|---------|
| `include/enki/platform/android/android_platform.hpp` | Backend public interface |
| `include/enki/platform/android/android_surface.hpp` | Surface (window) public interface |
| `src/platform/android/android_platform.cpp` | EGL init, ALooper, input, clipboard |
| `src/platform/android/android_surface.cpp` | EGL surface lifecycle |
| `src/platform/android/android_app_glue.cpp` | `ANativeActivity_onCreate`, callbacks |
| `cross/android-arm64.ini` | Meson NDK cross-compilation toolchain |
