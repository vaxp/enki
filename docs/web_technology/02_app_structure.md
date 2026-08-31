# 02 — Application Structure

Every Enki web application follows a simple, well-defined directory layout built around a single manifest file: `enki.json`.

---

## Directory Layout

```
my_app/
├── enki.json              ← Application manifest (required)
└── src/
    ├── index.html         ← Entry point HTML
    ├── main.js            ← Application JavaScript
    └── styles/
        └── app.css        ← Application styles
```

This is the minimal structure. You may add any additional directories (`assets/`, `components/`, `pages/`, etc.) — Enki places no restrictions on the internal layout beyond the manifest and the entry HTML file.

---

## The Manifest: `enki.json`

The manifest is the single source of truth for your application. It describes the app identity, window configuration, and permission grants.

### Full Schema

```json
{
  "name": "My App",
  "version": "1.0.0",
  "description": "A brief description of my application",
  "entry": "src/index.html",

  "window": {
    "title": "My App",
    "width": 1280,
    "height": 800,
    "resizable": true,
    "fullscreen": false
  },

  "devtools": false,

  "permissions": [
    "fs.read",
    "fs.write",
    "dialog",
    "clipboard",
    "notifications",
    "shell.open_external",
    "system.info"
  ]
}
```

### Field Reference

#### Top-Level

| Field | Type | Required | Description |
|---|---|---|---|
| `name` | `string` | ✅ | Application identifier and display name |
| `version` | `string` | ✅ | Semantic version string |
| `description` | `string` | ❌ | Short description |
| `entry` | `string` | ✅ | Relative path to the HTML entry point |
| `devtools` | `boolean` | ❌ | Open Chrome DevTools on startup (default: `false`) |
| `permissions` | `string[]` | ❌ | List of native API permissions to grant |

#### `window` Object

| Field | Type | Default | Description |
|---|---|---|---|
| `title` | `string` | `name` | OS window title |
| `width` | `integer` | `1280` | Initial window width in pixels |
| `height` | `integer` | `800` | Initial window height in pixels |
| `resizable` | `boolean` | `true` | Allow the user to resize the window |
| `fullscreen` | `boolean` | `false` | Start in fullscreen mode |

#### Permission Tokens

| Token | Grants access to |
|---|---|
| `fs.read` | `enki.fs.readFile`, `enki.fs.exists`, `enki.fs.listDir` |
| `fs.write` | `enki.fs.writeFile`, `enki.fs.mkdir`, `enki.fs.remove` |
| `dialog` | `enki.dialog.openFile`, `enki.dialog.saveFile`, `enki.dialog.confirm`, `enki.dialog.message` |
| `clipboard` | `enki.clipboard.read`, `enki.clipboard.write` |
| `notifications` | `enki.notification.show` |
| `shell.open_external` | `enki.shell.openExternal` |
| `system.info` | `enki.system.platform`, `.arch`, `.hostname`, `.memory`, `.cpuCount` |

> **Note:** `enki.window.*` and `enki.path.*` are always available and require no permission.

---

## Entry Point: `index.html`

The entry HTML file is a standard HTML5 document. The `window.enki` object is injected automatically before any script runs — no imports or special setup required.

```html
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>My App</title>
    <link rel="stylesheet" href="styles/app.css">
</head>
<body>

    <div id="titlebar">
        <span id="app-title">My App</span>
        <div id="window-controls">
            <button onclick="enki.window.minimize()">−</button>
            <button onclick="enki.window.maximize()">□</button>
            <button onclick="enki.window.close()">×</button>
        </div>
    </div>

    <div id="content">
        <!-- Your application UI goes here -->
    </div>

    <script src="main.js"></script>
</body>
</html>
```

---

## Custom Titlebar

Because Enki Web Host creates a native OS window, you can opt to hide the default titlebar and implement a fully custom one in HTML/CSS. The `window.enki.window.*` API gives you full control over minimize, maximize, and close actions.

See [03 — Building Applications](./03_building_apps.md) for a complete custom titlebar example.

---

## Multiple Pages / SPA

Enki Web Host runs a full Chromium browser engine, so any approach to page navigation works:

- **Multi-page**: simple `<a href="page2.html">` links between HTML files
- **Single-Page Application (SPA)**: use any JS router (Vue Router, React Router, etc.)
- **Hash routing**: `window.location.hash` changes work natively

There are no restrictions on which JavaScript frameworks you can use. React, Vue, Svelte, Vanilla JS — all work out of the box.
