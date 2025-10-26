# 01 — Environment Setup

This guide covers everything you need to get Enki Web Host running on your Linux system.

---

## Prerequisites

### System Packages

```bash
# Debian / Ubuntu
sudo apt install \
    build-essential \
    cmake \
    ninja-build \
    meson \
    python3 \
    pkg-config \
    libx11-dev \
    libxcomposite-dev \
    libxrandr-dev \
    libxdamage-dev \
    libxfixes-dev \
    libxi-dev \
    libxtst-dev \
    libnss3-dev \
    libatk-bridge2.0-dev \
    libcups2-dev \
    libdrm-dev \
    libgbm-dev \
    libasound2-dev

# Arch Linux
sudo pacman -S base-devel cmake ninja meson python libx11 nss atk cups libdrm mesa alsa-lib
```

### Compiler

A C++20-capable compiler is required:

```bash
# Check GCC version (must be ≥ 13)
gcc --version

# Check Clang version (must be ≥ 16)
clang --version
```

---

## Getting the CEF Binary

The Enki Web Host is powered by **CEF (Chromium Embedded Framework)**. The binary distribution must be placed inside the `web_technology/cef_binary/` directory.

### Verified Compatible Version

| Field | Value |
|---|---|
| CEF Version | `144.0.34+g8fc21c8` |
| Chromium | `144.0.7559.261` |
| Platform | `linux64_minimal` |
| Size | ~392 MB |

### Download

```bash
cd /path/to/enki/web_technology

# Download the minimal distribution
wget "https://cef-builds.spotifycdn.com/cef_binary_144.0.34+g8fc21c8+chromium-144.0.7559.261_linux64_minimal.tar.bz2"

# Extract into cef_binary/
mkdir -p cef_binary
tar -xjf cef_binary_*.tar.bz2 -C cef_binary/
```

The resulting layout should be:
```
web_technology/
└── cef_binary/
    └── cef_binary_144.0.34+g8fc21c8+chromium-144.0.7559.261_linux64_minimal/
        ├── include/
        ├── Release/
        │   ├── libcef.so          ← Main CEF library
        │   ├── libEGL.so
        │   ├── libGLESv2.so
        │   └── ...
        └── Resources/
            ├── icudtl.dat         ← ICU internationalization data
            ├── resources.pak
            ├── chrome_100_percent.pak
            ├── chrome_200_percent.pak
            └── locales/
```

### Build the CEF C++ Wrapper

The `libcef_dll_wrapper` static library must be compiled from source:

```bash
cd web_technology/cef_binary/cef_binary_144.0.34+g8fc21c8+chromium-144.0.7559.261_linux64_minimal

mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --target libcef_dll_wrapper -j$(nproc)
```

This produces `build/libcef_dll_wrapper/libcef_dll_wrapper.a` which Meson picks up automatically.

---

## Building Enki with Web Technology Support

From the root of the Enki repository:

```bash
# Configure the build (web_technology is included by default)
meson setup build_dir

# Build everything
ninja -C build_dir

# Or build only the web host demo
ninja -C build_dir examples/web_host_demo
```

### Verify the Build

```bash
ls -lh build_dir/examples/web_host_demo
ls -lh build_dir/web_technology/subprocess/enki_cef_subprocess
ls -lh build_dir/web_technology/libenki_web.a
```

All three artifacts must exist for the runtime to work correctly.

---

## Running the Demo

```bash
cd /path/to/enki

# Run the built-in template application
./build_dir/examples/web_host_demo
```

A native X11 window should appear displaying the Enki Web starter template.

---

## Troubleshooting

### `Invalid file descriptor to ICU data`

The CEF runtime cannot locate `icudtl.dat`. Make sure the `Resources/` directory is accessible. The runtime searches the following paths automatically:

1. Next to the executable
2. `web_technology/cef_binary/.../Resources/`
3. Absolute path (fallback)

If the issue persists, copy the Resources manually:

```bash
cp -r web_technology/cef_binary/.../Resources/* build_dir/examples/
```

### `Opening in existing browser session`

This means a stale CEF cache directory exists at `~/.cache/enki_web/`. Remove it:

```bash
rm -rf ~/.cache/enki_web
```

### Window does not appear

Ensure you are running under X11. If using Wayland, enable XWayland compatibility:

```bash
DISPLAY=:0 ./build_dir/examples/web_host_demo
```
