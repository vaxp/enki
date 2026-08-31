# Enki Web Technology

> A native web application runtime built into the Enki Framework — a lightweight, DE-independent alternative to Electron.

---

## Documentation Index

| Document | Description |
|---|---|
| [01 — Environment Setup](./01_environment_setup.md) | Prerequisites, CEF setup, and build configuration |
| [02 — Application Structure](./02_app_structure.md) | Project layout, manifest format, and entry points |
| [03 — Building Applications](./03_building_apps.md) | Step-by-step guide to building your first Enki web app |
| [04 — Native APIs Reference](./04_native_apis.md) | Complete reference for `window.enki.*` |
| [05 — Framework Developer Guide](./05_framework_developer.md) | How to build and configure the Web Host engine from source |

---

## What is Enki Web Host?

**Enki Web Host** is a native web application runtime embedded in the Enki Framework. It allows developers to build fully-featured desktop applications using standard web technologies — HTML, CSS, and JavaScript — while having direct access to the operating system through a set of native APIs exposed under `window.enki.*`.

Think of it as Electron, but:
- Built on top of **Enki's native rendering stack** (no GTK, no Qt)
- **No desktop environment dependency** — runs on any X11 Linux system
- Ships with **Chromium 144** via CEF (Chromium Embedded Framework)
- Exposes a clean, permission-gated **`window.enki.*` API surface**

---

## Quick Start

```bash
# Copy the starter template
cp -r web_technology/template/ my_app/

# Edit the manifest
nano my_app/enki.json

# Run your app
./build/examples/web_host_demo
```

---

## System Requirements

| Component | Requirement |
|---|---|
| OS | Linux (X11) |
| Compiler | GCC 13+ or Clang 16+ with C++20 |
| Build System | Meson ≥ 1.0 + Ninja |
| CEF | Included in `web_technology/cef_binary/` |
| Display | X11 (Wayland via XWayland also supported) |

---

*Enki Framework — MIT License*
