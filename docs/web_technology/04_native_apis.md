# 04 — Native APIs Reference

Complete reference for the `window.enki.*` API surface available in every Enki web application.

> All APIs return `Promise` unless otherwise noted. Synchronous variants are marked with `[sync]`.

---

## `enki.fs` — File System

**Required permission:** `"fs.read"` for read operations, `"fs.write"` for write operations.

### `enki.fs.readFile(path, encoding?)`

Reads the contents of a file.

```javascript
const content = await enki.fs.readFile('/home/user/notes.txt', 'utf8');
console.log(content); // string
```

| Parameter | Type | Description |
|---|---|---|
| `path` | `string` | Absolute path to the file |
| `encoding` | `string?` | `"utf8"` (default) or `"binary"` |

**Returns:** `Promise<string>`

---

### `enki.fs.writeFile(path, content)`

Writes content to a file. Creates the file if it does not exist.

```javascript
await enki.fs.writeFile('/home/user/output.txt', 'Hello, World!');
```

| Parameter | Type | Description |
|---|---|---|
| `path` | `string` | Absolute path to the file |
| `content` | `string` | Content to write |

**Returns:** `Promise<void>`

---

### `enki.fs.exists(path)`

Checks whether a path exists on the filesystem.

```javascript
const exists = await enki.fs.exists('/home/user/config.json');
if (exists) { /* ... */ }
```

**Returns:** `Promise<boolean>`

---

### `enki.fs.listDir(path)`

Lists the entries of a directory.

```javascript
const entries = await enki.fs.listDir('/home/user/Documents');
// [{ name: "file.txt", isDir: false }, { name: "subdir", isDir: true }, ...]
```

**Returns:** `Promise<Array<{ name: string, isDir: boolean }>>`

---

### `enki.fs.mkdir(path)`

Creates a directory (and any missing parent directories).

```javascript
await enki.fs.mkdir('/home/user/my_app/data');
```

**Returns:** `Promise<void>`

---

### `enki.fs.remove(path)`

Removes a file or directory (recursive).

```javascript
await enki.fs.remove('/home/user/tmp/old_file.txt');
```

**Returns:** `Promise<void>`

---

## `enki.dialog` — Native Dialogs

**Required permission:** `"dialog"`

### `enki.dialog.openFile(options?)`

Opens a native file picker dialog.

```javascript
const file = await enki.dialog.openFile({
    title: 'Select a file',
    filters: [
        { name: 'Images', extensions: ['png', 'jpg', 'gif'] },
        { name: 'All Files', extensions: ['*'] }
    ]
});

if (file) {
    console.log(file.name);  // "photo.jpg"
    console.log(file.path);  // "/home/user/pictures/photo.jpg"
}
```

**Returns:** `Promise<{ name: string, path: string } | null>`

---

### `enki.dialog.saveFile(options?)`

Opens a native save-file dialog.

```javascript
const savePath = await enki.dialog.saveFile({
    title: 'Save As',
    default_name: 'document.txt',
    filters: [{ name: 'Text', extensions: ['txt'] }]
});

if (savePath) {
    await enki.fs.writeFile(savePath, content);
}
```

**Returns:** `Promise<string | null>`

---

### `enki.dialog.confirm(message, title?)`

Shows a native Yes/No confirmation dialog.

```javascript
const confirmed = await enki.dialog.confirm(
    'Are you sure you want to delete this file?',
    'Confirm Delete'
);

if (confirmed) { /* proceed */ }
```

**Returns:** `Promise<boolean>`

---

### `enki.dialog.message(message, title?)`

Shows a native informational message box.

```javascript
await enki.dialog.message('File saved successfully!', 'Success');
```

**Returns:** `Promise<void>`

---

## `enki.window` — Window Control

**Required permission:** None (always available)

### `enki.window.setTitle(title)`

Sets the OS window title.

```javascript
enki.window.setTitle('My App — Unsaved Changes');
```

---

### `enki.window.setSize(width, height)`

Resizes the window.

```javascript
enki.window.setSize(1920, 1080);
```

---

### `enki.window.minimize()`

Minimizes the window to the taskbar.

---

### `enki.window.maximize()`

Maximizes the window.

---

### `enki.window.restore()`

Restores the window from maximized or minimized state.

---

### `enki.window.close()`

Closes the window and exits the application.

---

## `enki.notification` — Desktop Notifications

**Required permission:** `"notifications"`

### `enki.notification.show(options)`

Displays a native desktop notification.

```javascript
enki.notification.show({
    title: 'Download Complete',
    body: 'Your file has been downloaded successfully.'
});
```

| Parameter | Type | Description |
|---|---|---|
| `title` | `string` | Notification title |
| `body` | `string` | Notification body text |

**Returns:** `void`

---

## `enki.clipboard` — Clipboard

**Required permission:** `"clipboard"`

### `enki.clipboard.read()`

Reads plain text from the system clipboard.

```javascript
const text = await enki.clipboard.read();
console.log(text);
```

**Returns:** `Promise<string>`

---

### `enki.clipboard.write(text)`

Writes plain text to the system clipboard.

```javascript
await enki.clipboard.write('Copied to clipboard!');
```

**Returns:** `Promise<void>`

---

## `enki.shell` — Shell Integration

**Required permission:** `"shell.open_external"`

### `enki.shell.openExternal(url)`

Opens a URL or file path in the system's default application (browser, file manager, etc.).

```javascript
// Open a URL in the default browser
enki.shell.openExternal('https://github.com/enki-framework/enki');

// Open a directory in the file manager
enki.shell.openExternal('/home/user/Documents');
```

**Returns:** `void`

---

## `enki.system` — System Information

**Required permission:** `"system.info"`

### `enki.system.platform()` `[sync]`

Returns the operating system identifier.

```javascript
enki.system.platform(); // "linux"
```

**Returns:** `string`

---

### `enki.system.arch()` `[sync]`

Returns the CPU architecture.

```javascript
enki.system.arch(); // "x86_64"
```

**Returns:** `string`

---

### `enki.system.hostname()` `[sync]`

Returns the machine hostname.

```javascript
enki.system.hostname(); // "my-desktop"
```

**Returns:** `string`

---

### `enki.system.memory()` `[sync]`

Returns total physical memory in bytes.

```javascript
const gb = enki.system.memory() / 1024 / 1024 / 1024;
console.log(gb.toFixed(1) + ' GB'); // "16.0 GB"
```

**Returns:** `number`

---

### `enki.system.cpuCount()` `[sync]`

Returns the number of logical CPU cores.

```javascript
enki.system.cpuCount(); // 12
```

**Returns:** `number`

---

## `enki.path` — Path Utilities

**Required permission:** None (always available)

All `enki.path` functions are synchronous.

### `enki.path.join(...segments)`

Joins path segments using the OS separator.

```javascript
enki.path.join('/home/user', 'Documents', 'file.txt');
// "/home/user/Documents/file.txt"
```

---

### `enki.path.dirname(path)`

Returns the directory portion of a path.

```javascript
enki.path.dirname('/home/user/file.txt'); // "/home/user"
```

---

### `enki.path.basename(path)`

Returns the filename portion of a path.

```javascript
enki.path.basename('/home/user/file.txt'); // "file.txt"
```

---

### `enki.path.extname(path)`

Returns the file extension including the dot.

```javascript
enki.path.extname('/home/user/file.txt'); // ".txt"
```

---

### `enki.path.isAbsolute(path)`

Returns `true` if the path is absolute.

```javascript
enki.path.isAbsolute('/home/user'); // true
enki.path.isAbsolute('relative/path'); // false
```

---

## Error Handling

All async APIs throw on failure. Wrap calls in `try/catch`:

```javascript
try {
    const content = await enki.fs.readFile('/nonexistent/file.txt');
} catch (err) {
    console.error('Failed to read file:', err.message);
    await enki.dialog.message('Could not open file: ' + err.message, 'Error');
}
```
