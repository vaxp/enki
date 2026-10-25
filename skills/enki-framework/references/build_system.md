# ENKI Build System & CLI Reference

This guide provides everything an AI assistant or developer needs to configure, build, run, and package ENKI C++20 applications across Desktop, Embedded (DRM/KMS), Android, and WebAssembly.

---

## 1. ENKI CLI Toolchain

ENKI provides a native CLI tool (`./enki` or `enki` if installed in PATH) modeled after modern reactive toolchains.

### Core Commands

| Command | Usage | Description |
|---|---|---|
| `enki doctor` | `enki doctor` | Validates environment: C++20 compiler (GCC 13+, Clang 16+, MSVC 2022), Meson, Ninja, Skia, display servers (Wayland/X11), Android NDK, Emscripten. |
| `enki devices` | `enki devices` | Discovers connected display surfaces (X11 screens, Wayland compositors, DRM/KMS cards, ADB Android devices). |
| `enki create` | `enki create <project_name>` | Scaffolds a new ENKI C++20 project with clean architecture, BLoC state directory, assets, and Meson setup. |
| `enki run` | `enki run <target> [desktop\|drm\|android]` | Builds and executes an application immediately on the selected platform target. |
| `enki clean` | `enki clean` | Removes build directories and cache artifacts (`build/`, `build-wasm/`, `subprojects/`). |

---

## 2. Meson Build System Configuration

ENKI uses **Meson + Ninja** as its primary build system.

### Compiling an ENKI Application

In your project's `meson.build`:

```meson
project('my_enki_app', 'cpp',
  version: '1.0.0',
  default_options: [
    'cpp_std=c++20',
    'warning_level=2',
    'werror=false',
    'optimization=3'
  ]
)

# 1. Dependency Resolution
# Option A: System or pkg-config installed ENKI
enki_dep = dependency('enki', required: false)

# Option B: Subproject / Monorepo Fallback
if not enki_dep.found()
  enki_proj = subproject('enki')
  enki_dep = enki_proj.get_variable('enki_dep')
endif

# 2. Executable Target
executable('my_enki_app',
  sources: [
    'src/main.cpp',
    'src/state/app_cubit.cpp'
  ],
  include_directories: [
    include_directories('src')
  ],
  dependencies: [
    enki_dep
  ],
  install: true
)
```

### Framework Build Options (`meson_options.txt`)

When configuring the ENKI framework directly:

```bash
meson setup build \
  -Denable_x11=true \
  -Denable_wayland=true \
  -Denable_drm=false \
  -Denable_audio=true \
  -Denable_video=true \
  -Denable_webview=false \
  -Denable_demos=true \
  -Dbuildtype=release

ninja -C build
```

| Option | Default | Description |
|---|---|---|
| `enable_x11` | `true` | Compile native X11 / Xlib backend with EGL / GLX. |
| `enable_wayland` | `true` | Compile Wayland backend with `xdg-shell`, `wlr-layer-shell`, `xdg-decoration`. |
| `enable_drm` | `false` | Bare-metal DRM/KMS direct scanout (GBM + libinput + libudev) for kiosks. |
| `enable_audio` | `true` | Audio waveform & FFT spectrum analyzer (PulseAudio). |
| `enable_video` | `true` | Hardware-accelerated video player (FFmpeg + VA-API). |
| `enable_webview`| `false`| Chromium Embedded Framework (CEF) host runtime. |
| `enable_demos`  | `true` | Compile all 100+ widget demo executables. |

---

## 3. Platform Cross-Compilation

### A. Android (arm64-v8a)
```bash
meson setup build-android --cross-file cross/android-arm64.ini
ninja -C build-android
python3 scripts/package_gallery_apk.py  # or package_counter_apk.py
```

### B. WebAssembly (WASM via Emscripten)
```bash
source /path/to/emsdk/emsdk_env.sh
meson setup build-wasm --cross-file cross/wasm.ini
ninja -C build-wasm
python3 -m http.server -d build-wasm/real_app/calculator
```

---

## 4. Headless Testing without Display Server

ENKI state logic (`Cubit`, `Bloc`, `Stream`) and widget trees can run in completely headless environments for CI/CD unit testing:

```bash
ninja -C build test
```
No X11, Wayland, or GPU context is required for Cubit or element reconciliation tests.
