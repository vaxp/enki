# 03 — Building Applications

A complete walkthrough for building your first desktop application with Enki Web Host.

---

## Step 1: Copy the Starter Template

The repository ships with a ready-to-run template application:

```bash
cp -r web_technology/template/ my_first_app/
cd my_first_app/
```

The template contains:

```
my_first_app/
├── enki.json
└── src/
    ├── index.html
    ├── main.js
    └── styles/app.css
```

---

## Step 2: Configure `enki.json`

Open `enki.json` and customise it for your app:

```json
{
  "name": "My First App",
  "version": "1.0.0",
  "description": "My first Enki web application",
  "entry": "src/index.html",

  "window": {
    "title": "My First App",
    "width": 1280,
    "height": 800,
    "resizable": true
  },

  "permissions": [
    "fs.read",
    "fs.write",
    "dialog",
    "notifications",
    "system.info"
  ]
}
```

---

## Step 3: Build Your UI

### `src/index.html`

```html
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>My First App</title>
    <link rel="stylesheet" href="styles/app.css">
</head>
<body>

    <!-- Custom titlebar — full CSS control -->
    <div id="titlebar" style="-webkit-app-region: drag;">
        <span class="title">My First App</span>
        <div class="controls" style="-webkit-app-region: no-drag;">
            <button onclick="enki.window.minimize()">&#x2212;</button>
            <button onclick="enki.window.maximize()">&#x25A1;</button>
            <button class="close-btn" onclick="enki.window.close()">&#x2715;</button>
        </div>
    </div>

    <!-- Application content -->
    <main>
        <h1>Hello from Enki Web!</h1>

        <div class="button-row">
            <button id="btn-open">📂 Open File</button>
            <button id="btn-notify">🔔 Notify</button>
            <button id="btn-info">💻 System Info</button>
        </div>

        <pre id="output">Output will appear here...</pre>
    </main>

    <script src="main.js"></script>
</body>
</html>
```

### `src/main.js`

```javascript
// window.enki is available automatically — no imports needed.

// ── Open File ────────────────────────────────────────────────────
document.getElementById('btn-open').addEventListener('click', async () => {
    const file = await enki.dialog.openFile({
        title: 'Choose a file',
        filters: [
            { name: 'Text', extensions: ['txt', 'md', 'json'] },
            { name: 'All Files', extensions: ['*'] }
        ]
    });

    if (!file) return; // user cancelled

    const content = await enki.fs.readFile(file.path, 'utf8');
    document.getElementById('output').textContent = content;
    enki.window.setTitle(file.name + ' — My First App');
});

// ── Desktop Notification ─────────────────────────────────────────
document.getElementById('btn-notify').addEventListener('click', () => {
    enki.notification.show({
        title: 'Hello!',
        body: 'This notification was sent from your Enki app.'
    });
});

// ── System Information ───────────────────────────────────────────
document.getElementById('btn-info').addEventListener('click', () => {
    const info = {
        platform:  enki.system.platform(),
        arch:      enki.system.arch(),
        hostname:  enki.system.hostname(),
        memory_gb: (enki.system.memory() / 1024 / 1024 / 1024).toFixed(1),
        cpus:      enki.system.cpuCount()
    };

    document.getElementById('output').textContent = JSON.stringify(info, null, 2);
});
```

### `src/styles/app.css`

```css
*, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }

:root {
    --bg:       #0f0f1a;
    --surface:  #1a1a2e;
    --border:   #2a2a4a;
    --accent:   #6c63ff;
    --text:     #e0e0ff;
    --radius:   8px;
}

body {
    background: var(--bg);
    color: var(--text);
    font-family: 'Inter', 'Segoe UI', sans-serif;
    height: 100vh;
    display: flex;
    flex-direction: column;
    overflow: hidden;
}

/* Titlebar */
#titlebar {
    display: flex;
    align-items: center;
    justify-content: space-between;
    padding: 0 16px;
    height: 40px;
    background: var(--surface);
    border-bottom: 1px solid var(--border);
    user-select: none;
}
#titlebar .controls button {
    width: 28px; height: 28px;
    border: none; border-radius: 50%;
    background: var(--border);
    color: var(--text);
    cursor: pointer;
    margin-left: 6px;
    font-size: 13px;
}
#titlebar .controls .close-btn { background: #c0392b; }

/* Content */
main {
    flex: 1;
    padding: 40px;
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 24px;
}

h1 { font-size: 2rem; font-weight: 700; }

.button-row { display: flex; gap: 12px; }

button {
    padding: 10px 20px;
    background: var(--accent);
    color: #fff;
    border: none;
    border-radius: var(--radius);
    font-size: 14px;
    cursor: pointer;
    transition: opacity 0.2s;
}
button:hover { opacity: 0.85; }

#output {
    width: 100%;
    max-width: 600px;
    padding: 16px;
    background: var(--surface);
    border: 1px solid var(--border);
    border-radius: var(--radius);
    font-family: 'JetBrains Mono', monospace;
    font-size: 13px;
    min-height: 120px;
    white-space: pre-wrap;
}
```

---

## Step 4: Run Your Application

From the root of the Enki repository:

```bash
./build/examples/web_host_demo
```

The runtime automatically looks for `web_technology/template/enki.json` relative to the binary, or you can point it to your own app directory.

---

## Using a JavaScript Framework

Enki Web Host runs a full Chromium engine. You can use any JS framework:

### React (with CDN, no build step)

```html
<script src="https://unpkg.com/react@18/umd/react.development.js"></script>
<script src="https://unpkg.com/react-dom@18/umd/react-dom.development.js"></script>
<script src="https://unpkg.com/@babel/standalone/babel.min.js"></script>

<script type="text/babel">
    function App() {
        const [title, setTitle] = React.useState('');

        async function openFile() {
            const f = await enki.dialog.openFile();
            if (f) setTitle(f.name);
        }

        return (
            <div>
                <h1>{title || 'No file selected'}</h1>
                <button onClick={openFile}>Open File</button>
            </div>
        );
    }

    ReactDOM.createRoot(document.getElementById('root')).render(<App />);
</script>
```

### Vue 3 (with CDN)

```html
<script src="https://unpkg.com/vue@3/dist/vue.global.js"></script>
<script>
    const { createApp, ref } = Vue;

    createApp({
        setup() {
            const fileContent = ref('');

            async function openFile() {
                const file = await enki.dialog.openFile();
                if (file) {
                    fileContent.value = await enki.fs.readFile(file.path, 'utf8');
                }
            }

            return { fileContent, openFile };
        }
    }).mount('#app');
</script>
```

### Using a Bundler (Vite, Webpack, etc.)

Since Enki serves local HTML files, you can also pre-build your app with Vite or any other bundler and point `entry` in `enki.json` to the build output:

```json
{
  "entry": "dist/index.html"
}
```

```bash
# Build with Vite
npm run build

# Run with Enki
./build/examples/web_host_demo
```

---

## Inter-Process Communication (IPC)

All `window.enki.*` calls are implemented as synchronous-looking async functions backed by V8 native bindings. Under the hood:

1. Your JS calls `enki.fs.readFile(path)`
2. A V8 handler sends an IPC message to the Enki browser process
3. The browser process executes the native C++ implementation
4. The result is returned as a resolved `Promise`

No `preload.js` or `contextBridge` setup is needed — the APIs are injected automatically before your page script runs.
