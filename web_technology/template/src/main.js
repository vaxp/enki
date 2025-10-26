/**
 * Enki Workstation Pro — Main Application Logic
 * Powered by Enki Web Host Runtime & window.enki native bridge.
 */

// ── State ───────────────────────────────────────────────────
let currentFilePath = null;
let isEditorDirty = false;
let chartAnimationId = null;

// ── Permissions State (Live Simulator) ──────────────────────
const runtimePermissions = {
    'fs.read': true,
    'fs.write': true,
    'dialog': true,
    'clipboard': true,
    'notifications': true,
    'system.info': true,
    'shell.open_external': true
};

function checkPermission(perm) {
    if (!runtimePermissions[perm]) {
        const errMsg = `[SECURITY EXCEPTION] Access Denied: Permission "${perm}" is not granted in enki.json manifest.`;
        logConsole('SECURITY REJECT', errMsg);
        showToast('Permission Denied', `Access to "${perm}" was blocked by Enki Security Manager.`, 'error');
        throw new Error(errMsg);
    }
    return true;
}

function togglePerm(perm, isEnabled) {
    runtimePermissions[perm] = isEnabled;
    const activeCount = Object.values(runtimePermissions).filter(Boolean).length;
    const badge = document.getElementById('active-perms-count');
    if (badge) {
        badge.textContent = `${activeCount} Permissions Granted`;
        badge.className = `badge ${activeCount === 7 ? 'emerald' : activeCount > 3 ? 'amber' : 'rose'}`;
    }
    showToast(
        isEnabled ? 'Permission Granted' : 'Permission Revoked',
        `Runtime token "${perm}" is now ${isEnabled ? 'ENABLED' : 'BLOCKED'}.`,
        isEnabled ? 'success' : 'error'
    );
    logConsole('Security Policy Update', { permission: perm, status: isEnabled ? 'GRANTED' : 'REVOKED' });
}

// ── Ensure window.enki Native Bridge ────────────────────────
window.enki = window.enki || {};

// 1. Filesystem API
window.enki.fs = window.enki.fs || {
    readFile: async function(path) {
        checkPermission('fs.read');
        if (window.__enki_fs_readFile) {
            const res = await window.enki.__call('__enki_fs_readFile', { path });
            if (res && res.content !== undefined) return res.content;
        }
        return `# Sample content for ${path}\nOpened at ${new Date().toLocaleTimeString()}\nRead operation executed via enki.fs.readFile().`;
    },
    writeFile: async function(path, data) {
        checkPermission('fs.write');
        if (window.__enki_fs_writeFile) {
            await window.enki.__call('__enki_fs_writeFile', { path, data });
        }
        return true;
    },
    exists: async function(path) {
        checkPermission('fs.read');
        return true;
    },
    listDir: async function(path) {
        checkPermission('fs.read');
        return [
            { name: "enki.json", is_dir: false, size: 617 },
            { name: "src", is_dir: true, size: 4096 },
            { name: "styles", is_dir: true, size: 4096 },
            { name: "assets", is_dir: true, size: 4096 },
            { name: "README.md", is_dir: false, size: 1820 }
        ];
    },
    mkdir: async function(path) {
        checkPermission('fs.write');
        return true;
    },
    remove: async function(path) {
        checkPermission('fs.write');
        return true;
    }
};

// 2. Dialog API
window.enki.dialog = window.enki.dialog || {
    openFile: async function(opts = {}) {
        checkPermission('dialog');
        return new Promise((resolve) => {
            const input = document.createElement('input');
            input.type = 'file';
            input.onchange = (e) => {
                const file = e.target.files[0];
                if (!file) return resolve(null);
                const reader = new FileReader();
                reader.onload = () => {
                    resolve({
                        name: file.name,
                        path: file.name,
                        content: reader.result
                    });
                };
                reader.readAsText(file);
            };
            input.click();
        });
    },
    saveFile: async function(opts = {}) {
        checkPermission('dialog');
        const defaultName = opts.default_name || 'document.txt';
        const name = prompt('Enter filename to save to local system:', defaultName);
        return name ? `/home/user/${name}` : null;
    },
    confirm: async function(message, title = 'Confirm') {
        checkPermission('dialog');
        return window.confirm(`${title}\n\n${message}`);
    },
    message: async function(message, title = 'Message') {
        checkPermission('dialog');
        window.alert(`${title}\n\n${message}`);
    }
};

// 3. Notification API
window.enki.notification = window.enki.notification || {
    show: function(opts = {}) {
        checkPermission('notifications');
        const title = opts.title || 'Enki Notification';
        const body = opts.body || '';

        // Try Web Notification API if permitted
        if ('Notification' in window && Notification.permission === 'granted') {
            new Notification(title, { body });
        } else if ('Notification' in window && Notification.permission !== 'denied') {
            Notification.requestPermission().then(p => {
                if (p === 'granted') new Notification(title, { body });
            });
        }

        // Always show on-screen Toast as well
        showToast(`🔔 ${title}`, body, 'info');
    }
};

// 4. Clipboard API
window.enki.clipboard = window.enki.clipboard || {
    write: async function(text) {
        checkPermission('clipboard');
        try {
            await navigator.clipboard.writeText(text);
        } catch (e) {
            // fallback
        }
        return true;
    },
    read: async function() {
        checkPermission('clipboard');
        try {
            return await navigator.clipboard.readText();
        } catch (e) {
            return "Enki Web Host is ultra fast!";
        }
    }
};

// 5. System API
window.enki.system = window.enki.system || {
    platform: function() {
        checkPermission('system.info');
        return 'linux';
    },
    arch: function() {
        checkPermission('system.info');
        return 'x86_64';
    },
    hostname: function() {
        checkPermission('system.info');
        return 'enki-workstation';
    },
    memory: function() {
        checkPermission('system.info');
        return 16 * 1024 * 1024 * 1024; // 16 GB
    },
    cpuCount: function() {
        checkPermission('system.info');
        return navigator.hardwareConcurrency || 8;
    }
};

// 6. Shell API
window.enki.shell = window.enki.shell || {
    openExternal: function(url) {
        checkPermission('shell.open_external');
        window.open(url, '_blank');
        return true;
    }
};

// 7. Window API (Always allowed - no permission token required)
window.enki.window = window.enki.window || {
    setTitle: function(t) {
        document.title = t;
        const disp = document.getElementById('window-title-display');
        if (disp) disp.textContent = t;
    },
    setSize: function(w, h) {
        showToast('Window Resize', `Target geometry requested: ${w}x${h}px`, 'info');
    },
    minimize: function() {
        showToast('Window', 'Minimize requested', 'info');
    },
    maximize: function() {
        showToast('Window', 'Maximize / Restore toggled', 'info');
    },
    close: function() {
        window.close();
    }
};

// 8. Path API (Utility - no permission required)
window.enki.path = window.enki.path || {
    join: function(...args) {
        return args.join('/').replace(/\/+/g, '/');
    },
    dirname: function(p) {
        const idx = p.lastIndexOf('/');
        return idx === -1 ? '.' : p.substr(0, idx) || '/';
    },
    basename: function(p, ext) {
        let base = p.substr(p.lastIndexOf('/') + 1);
        if (ext && base.endsWith(ext)) base = base.substr(0, base.length - ext.length);
        return base;
    },
    extname: function(p) {
        const idx = p.lastIndexOf('.');
        return idx === -1 ? '' : p.substr(idx);
    },
    isAbsolute: function(p) {
        return p.startsWith('/');
    }
};

// ── Initialization ──────────────────────────────────────────
document.addEventListener('DOMContentLoaded', () => {
    initClock();
    initNavigation();
    initLiveChart();
    initEditor();
    initGlobalSearch();

    // Query system hardware specs
    setTimeout(() => {
        refreshSystemMetrics();
        showToast('Enki Engine Ready', 'Native C++ bridge connected to Chromium 144.', 'success');
    }, 150);
});

// ── Clock ───────────────────────────────────────────────────
function initClock() {
    const clockEl = document.getElementById('live-clock');
    function update() {
        const now = new Date();
        clockEl.textContent = now.toTimeString().split(' ')[0];
    }
    update();
    setInterval(update, 1000);
}

// ── Navigation ──────────────────────────────────────────────
function initNavigation() {
    const navItems = document.querySelectorAll('.nav-item');
    navItems.forEach(item => {
        item.addEventListener('click', () => {
            const tabId = item.getAttribute('data-tab');
            switchTab(tabId);
        });
    });
}

function switchTab(tabId) {
    document.querySelectorAll('.nav-item').forEach(item => {
        item.classList.toggle('active', item.getAttribute('data-tab') === tabId);
    });

    document.querySelectorAll('.tab-view').forEach(view => {
        view.classList.toggle('active', view.id === `tab-${tabId}`);
    });

    const titleMap = {
        'dashboard': 'Workstation Dashboard',
        'sysmonitor': 'System & Hardware Diagnostics',
        'filestudio': 'File Studio & Native Editor',
        'apiplayground': 'Native C++ APIs & Permissions Playground',
        'uishowcase': 'UI & Component Showcase',
        'webportal': 'VAXP Web Portal & Online Studio'
    };
    const subtitle = titleMap[tabId] || 'Workstation Pro';
    document.getElementById('window-title-display').textContent = subtitle;
}

// ── Live Permissions Action Tester ──────────────────────────
async function testPermAction(perm) {
    try {
        switch (perm) {
            case 'fs.read': {
                const entries = await enki.fs.listDir('.');
                logConsole('enki.fs.listDir (Permission: fs.read)', entries);
                showToast('fs.read Passed', `Successfully read directory (${entries.length} items found).`, 'success');
                break;
            }
            case 'fs.write': {
                await enki.fs.writeFile('/tmp/enki_test.txt', 'Enki permission test content');
                logConsole('enki.fs.writeFile (Permission: fs.write)', { path: '/tmp/enki_test.txt', status: 'written' });
                showToast('fs.write Passed', 'Successfully wrote test file to /tmp/enki_test.txt', 'success');
                break;
            }
            case 'dialog': {
                const confirmed = await enki.dialog.confirm('Permission test: Dialog subsystem is responding properly. Click OK.', 'Enki Security Verification');
                logConsole('enki.dialog.confirm (Permission: dialog)', { confirmed });
                showToast('dialog Passed', `Dialog action completed (Result: ${confirmed})`, 'success');
                break;
            }
            case 'clipboard': {
                const sample = `Enki Token Verification: ${Math.random().toString(36).substring(7)}`;
                await enki.clipboard.write(sample);
                const readBack = await enki.clipboard.read();
                logConsole('enki.clipboard (Permission: clipboard)', { written: sample, readBack });
                showToast('clipboard Passed', `Wrote & read from clipboard: "${readBack}"`, 'success');
                break;
            }
            case 'notifications': {
                enki.notification.show({
                    title: 'Security Clearance: GRANTED',
                    body: 'Notification subsystem verified with active permission token.'
                });
                logConsole('enki.notification (Permission: notifications)', 'Triggered successfully');
                break;
            }
            case 'system.info': {
                const platform = enki.system.platform();
                const cpus = enki.system.cpuCount();
                const mem = (enki.system.memory() / (1024*1024*1024)).toFixed(1);
                logConsole('enki.system.info (Permission: system.info)', { platform, cpus, ramGB: mem });
                showToast('system.info Passed', `Query returned: ${platform.toUpperCase()}, ${cpus} CPUs, ${mem}GB RAM`, 'success');
                break;
            }
            case 'shell.open_external': {
                enki.shell.openExternal('https://vaxp.org');
                logConsole('enki.shell.openExternal (Permission: shell.open_external)', 'Opened https://vaxp.org');
                showToast('shell.open_external Passed', 'Dispatched external URI handler to https://vaxp.org', 'success');
                break;
            }
        }
    } catch (err) {
        console.warn('Action blocked:', err.message);
    }
}

// ── System Metrics Query ────────────────────────────────────
function refreshSystemMetrics() {
    try {
        const platform = enki.system.platform();
        const arch     = enki.system.arch();
        const hostname = enki.system.hostname();
        const memBytes = enki.system.memory();
        const cpus     = enki.system.cpuCount();

        const memGB = (memBytes / (1024 * 1024 * 1024)).toFixed(1);

        document.getElementById('kpi-os').textContent   = platform ? platform.toUpperCase() : 'LINUX';
        document.getElementById('kpi-ram').textContent  = `${memGB} GB`;
        document.getElementById('kpi-cpus').textContent = `${cpus} Cores`;

        document.getElementById('dash-hostname').textContent = hostname || 'localhost';
        document.getElementById('dash-arch').textContent     = arch || 'x86_64';

        document.getElementById('side-platform').textContent = platform || 'Linux';
        document.getElementById('side-arch').textContent     = arch || 'x86_64';

        document.getElementById('mon-ram-total').textContent  = `${memGB} GB`;
        document.getElementById('mon-arch').textContent       = arch || 'x86_64';
        document.getElementById('cpu-cores-count').textContent= `${cpus}`;

        document.getElementById('tbl-os').textContent   = platform;
        document.getElementById('tbl-arch').textContent = arch;
        document.getElementById('tbl-host').textContent = hostname;
        document.getElementById('tbl-ram').textContent  = `${memGB} GB`;
        document.getElementById('tbl-cpus').textContent = `${cpus} Cores`;

        logConsole('enki.system.* Query', {
            platform, arch, hostname, totalMemoryBytes: memBytes, cpuCores: cpus
        });

    } catch (err) {
        console.error('Failed to query system metrics:', err);
    }
}

// ── Live Chart Oscilloscope ─────────────────────────────────
function initLiveChart() {
    const canvas = document.getElementById('live-chart');
    if (!canvas) return;
    const ctx = canvas.getContext('2d');

    function resize() {
        canvas.width = canvas.parentElement.clientWidth;
        canvas.height = canvas.parentElement.clientHeight;
    }
    resize();
    window.addEventListener('resize', resize);

    let step = 0;
    const pointsCount = 80;
    const history1 = new Array(pointsCount).fill(50);
    const history2 = new Array(pointsCount).fill(40);
    const history3 = new Array(pointsCount).fill(60);

    function draw() {
        step += 0.05;

        const val1 = 50 + Math.sin(step) * 20 + Math.cos(step * 2.3) * 10 + (Math.random() - 0.5) * 8;
        const val2 = 45 + Math.cos(step * 0.8) * 25 + Math.sin(step * 1.7) * 8;
        const val3 = 60 + Math.sin(step * 1.5) * 15 + Math.cos(step * 0.4) * 10;

        history1.shift(); history1.push(val1);
        history2.shift(); history2.push(val2);
        history3.shift(); history3.push(val3);

        ctx.clearRect(0, 0, canvas.width, canvas.height);

        ctx.strokeStyle = 'rgba(255, 255, 255, 0.04)';
        ctx.lineWidth = 1;
        const gridX = canvas.width / 10;
        const gridY = canvas.height / 4;
        for (let x = 0; x < canvas.width; x += gridX) {
            ctx.beginPath(); ctx.moveTo(x, 0); ctx.lineTo(x, canvas.height); ctx.stroke();
        }
        for (let y = 0; y < canvas.height; y += gridY) {
            ctx.beginPath(); ctx.moveTo(0, y); ctx.lineTo(canvas.width, y); ctx.stroke();
        }

        function drawSeries(data, color, fillColor) {
            ctx.beginPath();
            const stepX = canvas.width / (pointsCount - 1);
            for (let i = 0; i < pointsCount; i++) {
                const x = i * stepX;
                const y = canvas.height - (data[i] / 100) * canvas.height;
                if (i === 0) ctx.moveTo(x, y);
                else ctx.lineTo(x, y);
            }
            ctx.strokeStyle = color;
            ctx.lineWidth = 2;
            ctx.stroke();

            ctx.lineTo(canvas.width, canvas.height);
            ctx.lineTo(0, canvas.height);
            ctx.closePath();
            ctx.fillStyle = fillColor;
            ctx.fill();
        }

        drawSeries(history2, '#06b6d4', 'rgba(6, 182, 212, 0.06)');
        drawSeries(history1, '#6366f1', 'rgba(99, 102, 241, 0.12)');
        drawSeries(history3, '#10b981', 'rgba(16, 185, 129, 0.05)');

        chartAnimationId = requestAnimationFrame(draw);
    }

    draw();
}

// ── File Studio ─────────────────────────────────────────────
function initEditor() {
    const editor = document.getElementById('code-editor');
    if (!editor) return;

    editor.addEventListener('input', () => {
        isEditorDirty = true;
        document.getElementById('dirty-dot').classList.add('visible');
        updateEditorMeta();
    });

    loadSampleText('manifest');
}

function updateEditorMeta() {
    const editor = document.getElementById('code-editor');
    const text = editor.value;
    document.getElementById('editor-char-count').textContent = text.length;
    document.getElementById('editor-line-count').textContent = text.split('\n').length;
}

function loadSampleText(type) {
    const editor = document.getElementById('code-editor');
    if (type === 'manifest') {
        editor.value = JSON.stringify({
            name: "Enki Workstation Pro",
            version: "2.0.0",
            entry: "src/index.html",
            window: { width: 1360, height: 860 },
            permissions: ["fs.read", "fs.write", "dialog", "clipboard", "notifications", "system.info"]
        }, null, 4);
        document.getElementById('editor-tab-name').textContent = 'enki.json';
    } else if (type === 'hello') {
        editor.value = `# Welcome to Enki Workstation Studio\n\nThis is a native file edited inside the Web Host.\nYou can save it directly to your Linux filesystem via enki.fs.writeFile()!`;
        document.getElementById('editor-tab-name').textContent = 'hello_world.txt';
    } else if (type === 'script') {
        editor.value = `// Asynchronous IPC call to Enki C++ backend\nasync function queryPlatform() {\n    const p = enki.system.platform();\n    console.log("Current OS:", p);\n}\nqueryPlatform();`;
        document.getElementById('editor-tab-name').textContent = 'app_logic.js';
    }
    currentFilePath = null;
    document.getElementById('current-file-path').textContent = 'Sample File (Unsaved)';
    document.getElementById('dirty-dot').classList.remove('visible');
    updateEditorMeta();
}

async function editorOpenFile() {
    try {
        const file = await enki.dialog.openFile({
            title: 'Open Document',
            filters: [
                { name: 'All Supported', extensions: ['txt', 'json', 'js', 'html', 'css', 'md', 'cpp', 'hpp'] },
                { name: 'All Files', extensions: ['*'] }
            ]
        });

        if (!file) return;

        const content = file.content !== undefined ? file.content : await enki.fs.readFile(file.path, 'utf8');
        const editor = document.getElementById('code-editor');
        editor.value = content;
        currentFilePath = file.path;

        document.getElementById('editor-tab-name').textContent = file.name;
        document.getElementById('current-file-path').textContent = file.path;
        document.getElementById('dirty-dot').classList.remove('visible');
        updateEditorMeta();

        showToast('File Opened', file.name, 'success');
        logConsole('enki.fs.readFile', { path: file.path, bytes: content.length });

    } catch (err) {
        showToast('Error Opening File', err.message || err, 'error');
        console.error(err);
    }
}

async function editorSaveFile() {
    try {
        const editor = document.getElementById('code-editor');
        const savePath = await enki.dialog.saveFile({
            title: 'Save As',
            default_name: document.getElementById('editor-tab-name').textContent || 'untitled.txt'
        });

        if (!savePath) return;

        await enki.fs.writeFile(savePath, editor.value);
        currentFilePath = savePath;
        const filename = enki.path.basename(savePath);

        document.getElementById('editor-tab-name').textContent = filename;
        document.getElementById('current-file-path').textContent = savePath;
        document.getElementById('dirty-dot').classList.remove('visible');

        showToast('File Saved', filename, 'success');
        logConsole('enki.fs.writeFile', { path: savePath, length: editor.value.length });

    } catch (err) {
        showToast('Save Failed', err.message || err, 'error');
        console.error(err);
    }
}

async function editorQuickSave() {
    if (!currentFilePath) {
        return editorSaveFile();
    }
    try {
        const editor = document.getElementById('code-editor');
        await enki.fs.writeFile(currentFilePath, editor.value);
        document.getElementById('dirty-dot').classList.remove('visible');
        showToast('Quick Saved', enki.path.basename(currentFilePath), 'success');
        logConsole('enki.fs.writeFile (Quick)', { path: currentFilePath });
    } catch (err) {
        showToast('Save Failed', err.message || err, 'error');
    }
}

function clearEditor() {
    document.getElementById('code-editor').value = '';
    updateEditorMeta();
}

async function copyEditorContent() {
    try {
        const text = document.getElementById('code-editor').value;
        await enki.clipboard.write(text);
        showToast('Copied', `${text.length} characters copied to clipboard.`, 'success');
    } catch (err) {
        showToast('Copy Failed', err.message, 'error');
    }
}

async function listCurrentDir() {
    try {
        const entries = await enki.fs.listDir('.');
        logConsole('enki.fs.listDir(".") Result:', entries);
        showToast('Scanned Directory', `Found ${entries.length} items. Check console.`, 'success');
    } catch (err) {
        showToast('Scan Failed', err.message || err, 'error');
    }
}

// ── API Playground ──────────────────────────────────────────
async function testDialogOpen() {
    try {
        const res = await enki.dialog.openFile();
        logConsole('enki.dialog.openFile', res);
        if (res) showToast('Dialog Success', `Selected: ${res.name}`, 'success');
    } catch (e) {
        showToast('Dialog Blocked', e.message, 'error');
    }
}
async function testDialogSave() {
    try {
        const res = await enki.dialog.saveFile();
        logConsole('enki.dialog.saveFile', res);
        if (res) showToast('Save Target', res, 'success');
    } catch (e) {
        showToast('Dialog Blocked', e.message, 'error');
    }
}
async function testDialogConfirm() {
    try {
        const confirmed = await enki.dialog.confirm('Do you want to proceed with this native action?', 'Enki Confirmation');
        logConsole('enki.dialog.confirm', { confirmed });
        showToast('Confirm Result', confirmed ? 'User clicked Yes' : 'User cancelled', confirmed ? 'success' : 'error');
    } catch (e) {
        showToast('Dialog Blocked', e.message, 'error');
    }
}
async function testDialogMessage() {
    try {
        await enki.dialog.message('This is a native dialog message powered by Enki C++ backend!', 'Enki Message');
        logConsole('enki.dialog.message', 'Shown successfully');
    } catch (e) {
        showToast('Dialog Blocked', e.message, 'error');
    }
}

function sendCustomNotification() {
    try {
        const title = document.getElementById('notif-title').value;
        const body  = document.getElementById('notif-body').value;
        enki.notification.show({ title, body });
        logConsole('enki.notification.show', { title, body });
    } catch (e) {
        showToast('Notification Blocked', e.message, 'error');
    }
}
function demoNotify() {
    try {
        enki.notification.show({
            title: 'Enki Workstation Alert',
            body: 'Real-time background service pinged successfully!'
        });
    } catch (e) {
        showToast('Notification Blocked', e.message, 'error');
    }
}

async function testClipboardWrite() {
    try {
        const text = document.getElementById('clip-input').value;
        await enki.clipboard.write(text);
        showToast('Copied to Clipboard', text, 'success');
        logConsole('enki.clipboard.write', { text });
    } catch (e) {
        showToast('Clipboard Blocked', e.message, 'error');
    }
}
async function testClipboardRead() {
    try {
        const text = await enki.clipboard.read();
        document.getElementById('clip-input').value = text;
        showToast('Read from Clipboard', text, 'success');
        logConsole('enki.clipboard.read', { text });
    } catch (e) {
        showToast('Clipboard Blocked', e.message, 'error');
    }
}

function testChangeTitle() {
    const newTitle = document.getElementById('win-title-input').value;
    enki.window.setTitle(newTitle);
    showToast('Window Title Updated', newTitle, 'success');
    logConsole('enki.window.setTitle', { newTitle });
}

function testShellOpen() {
    try {
        const url = document.getElementById('shell-url-input').value;
        enki.shell.openExternal(url);
        showToast('Opening Link', url, 'success');
        logConsole('enki.shell.openExternal', { url });
    } catch (e) {
        showToast('Shell Blocked', e.message, 'error');
    }
}

function testPathUtilities() {
    const sample = '/home/developer/projects/enki/src/main.cpp';
    const result = {
        samplePath: sample,
        dirname:    enki.path.dirname(sample),
        basename:   enki.path.basename(sample),
        extname:    enki.path.extname(sample),
        isAbsolute: enki.path.isAbsolute(sample),
        joined:     enki.path.join('/var/log', 'enki', 'app.log')
    };

    document.getElementById('path-demo-box').textContent = JSON.stringify(result, null, 2);
    logConsole('enki.path Diagnostics', result);
}

// ── Web Portal Controls ─────────────────────────────────────
function navigateWebPortal() {
    let url = document.getElementById('portal-url-input').value.trim();
    if (!url.startsWith('http://') && !url.startsWith('https://')) {
        url = 'https://' + url;
        document.getElementById('portal-url-input').value = url;
    }
    const iframe = document.getElementById('vaxp-iframe');
    iframe.src = url;
    showToast('Navigating...', url, 'info');
    logConsole('WebPortal.navigate', { url });
}

function reloadWebPortal() {
    const iframe = document.getElementById('vaxp-iframe');
    const current = iframe.src;
    iframe.src = current;
    showToast('Reloading...', 'Page refreshed', 'info');
}

function portalGoBack() {
    try {
        const iframe = document.getElementById('vaxp-iframe');
        iframe.contentWindow.history.back();
    } catch (e) {
        showToast('History Navigation', 'Cannot access iframe history directly due to cross-origin.', 'info');
    }
}

function portalGoForward() {
    try {
        const iframe = document.getElementById('vaxp-iframe');
        iframe.contentWindow.history.forward();
    } catch (e) {
        showToast('History Navigation', 'Cannot access iframe history directly due to cross-origin.', 'info');
    }
}

// ── Global Search ───────────────────────────────────────────
function initGlobalSearch() {
    const search = document.getElementById('global-search');
    search.addEventListener('keydown', (e) => {
        if (e.key === 'Enter') {
            const q = search.value.toLowerCase();
            if (q.includes('vaxp') || q.includes('web') || q.includes('portal') || q.includes('site')) switchTab('webportal');
            else if (q.includes('file') || q.includes('editor')) switchTab('filestudio');
            else if (q.includes('sys') || q.includes('mon') || q.includes('ram') || q.includes('cpu')) switchTab('sysmonitor');
            else if (q.includes('api') || q.includes('clip') || q.includes('dialog') || q.includes('perm')) switchTab('apiplayground');
            else if (q.includes('ui') || q.includes('card')) switchTab('uishowcase');
            else switchTab('dashboard');
        }
    });
}

// ── Logging & Toast Helpers ─────────────────────────────────
function logConsole(action, data) {
    const consoleEl = document.getElementById('api-console-output');
    if (!consoleEl) return;
    const timestamp = new Date().toLocaleTimeString();
    const entry = `[${timestamp}] ${action} → ${typeof data === 'object' ? JSON.stringify(data, null, 2) : data}\n`;
    consoleEl.textContent = entry + consoleEl.textContent;
}
function clearConsole() {
    const consoleEl = document.getElementById('api-console-output');
    if (consoleEl) consoleEl.textContent = '// Console cleared.';
}

function showToast(title, body, type = 'info') {
    const container = document.getElementById('toast-container');
    if (!container) return;
    const toast = document.createElement('div');
    toast.className = `toast ${type}`;
    toast.innerHTML = `
        <div>
            <div class="toast-title">${title}</div>
            <div class="toast-body">${body}</div>
        </div>
    `;
    container.appendChild(toast);

    setTimeout(() => {
        toast.style.transition = 'opacity 0.3s ease, transform 0.3s ease';
        toast.style.opacity = '0';
        toast.style.transform = 'translateX(40px)';
        setTimeout(() => toast.remove(), 300);
    }, 3500);
}
