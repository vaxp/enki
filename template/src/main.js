/**
 * Enki Workstation Pro — Main Application Logic
 * Powered by Enki Web Host Runtime & window.enki native bridge.
 */

// ── State ───────────────────────────────────────────────────
let currentFilePath = null;
let isEditorDirty = false;
let chartAnimationId = null;

// ── Initialization ──────────────────────────────────────────
document.addEventListener('DOMContentLoaded', () => {
    initClock();
    initNavigation();
    initLiveChart();
    initEditor();
    initGlobalSearch();

    // Query system hardware specs once bridge is available
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
    // Update nav buttons
    document.querySelectorAll('.nav-item').forEach(item => {
        item.classList.toggle('active', item.getAttribute('data-tab') === tabId);
    });

    // Update tab sections
    document.querySelectorAll('.tab-view').forEach(view => {
        view.classList.toggle('active', view.id === `tab-${tabId}`);
    });

    // Update window title bar display
    const titleMap = {
        'dashboard': 'Workstation Dashboard',
        'sysmonitor': 'System & Hardware Diagnostics',
        'filestudio': 'File Studio & Native Editor',
        'apiplayground': 'Native C++ APIs Playground',
        'uishowcase': 'UI & Component Showcase',
        'webbrowser': 'Web Browser — Direct GPU Accelerated'
    };
    const subtitle = titleMap[tabId] || 'Workstation Pro';
    document.getElementById('window-title-display').textContent = subtitle;
}

// ── System Metrics Query ────────────────────────────────────
function refreshSystemMetrics() {
    try {
        if (!window.enki || !window.enki.system) {
            console.warn('[Enki] window.enki.system is not ready yet.');
            return;
        }

        const platform = enki.system.platform();
        const arch     = enki.system.arch();
        const hostname = enki.system.hostname();
        const memBytes = enki.system.memory();
        const cpus     = enki.system.cpuCount();

        const memGB = (memBytes / (1024 * 1024 * 1024)).toFixed(1);

        // Update Dashboard KPIs
        document.getElementById('kpi-os').textContent   = platform ? platform.toUpperCase() : 'LINUX';
        document.getElementById('kpi-ram').textContent  = `${memGB} GB`;
        document.getElementById('kpi-cpus').textContent = `${cpus} Cores`;

        document.getElementById('dash-hostname').textContent = hostname || 'localhost';
        document.getElementById('dash-arch').textContent     = arch || 'x86_64';

        // Update Sidebar
        document.getElementById('side-platform').textContent = platform || 'Linux';
        document.getElementById('side-arch').textContent     = arch || 'x86_64';

        // Update System Monitor View
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

        // Simulate waveforms
        const val1 = 50 + Math.sin(step) * 20 + Math.cos(step * 2.3) * 10 + (Math.random() - 0.5) * 8;
        const val2 = 45 + Math.cos(step * 0.8) * 25 + Math.sin(step * 1.7) * 8;
        const val3 = 60 + Math.sin(step * 1.5) * 15 + Math.cos(step * 0.4) * 10;

        history1.shift(); history1.push(val1);
        history2.shift(); history2.push(val2);
        history3.shift(); history3.push(val3);

        ctx.clearRect(0, 0, canvas.width, canvas.height);

        // Draw grid
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

        // Helper to draw smooth series
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

            // Fill area
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
            permissions: ["fs.read", "fs.write", "dialog", "notifications", "system.info"]
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

        const content = await enki.fs.readFile(file.path, 'utf8');
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
    const text = document.getElementById('code-editor').value;
    await enki.clipboard.write(text);
    showToast('Copied', `${text.length} characters copied to clipboard.`, 'success');
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
    const res = await enki.dialog.openFile();
    logConsole('enki.dialog.openFile', res);
}
async function testDialogSave() {
    const res = await enki.dialog.saveFile();
    logConsole('enki.dialog.saveFile', res);
}
async function testDialogConfirm() {
    const confirmed = await enki.dialog.confirm('Do you want to proceed with this native action?', 'Enki Confirmation');
    logConsole('enki.dialog.confirm', { confirmed });
    showToast('Confirm Result', confirmed ? 'User clicked Yes' : 'User cancelled', confirmed ? 'success' : 'error');
}
async function testDialogMessage() {
    await enki.dialog.message('This is a native dialog message powered by Enki C++ backend!', 'Enki Message');
    logConsole('enki.dialog.message', 'Shown successfully');
}

function sendCustomNotification() {
    const title = document.getElementById('notif-title').value;
    const body  = document.getElementById('notif-body').value;
    enki.notification.show({ title, body });
    showToast('Notification Dispatched', title, 'success');
    logConsole('enki.notification.show', { title, body });
}
function demoNotify() {
    enki.notification.show({
        title: 'Enki Workstation Alert',
        body: 'Real-time background service pinged successfully!'
    });
    showToast('Notification Sent', 'Desktop alert dispatched', 'success');
}

async function testClipboardWrite() {
    const text = document.getElementById('clip-input').value;
    await enki.clipboard.write(text);
    showToast('Copied to Clipboard', text, 'success');
    logConsole('enki.clipboard.write', { text });
}
async function testClipboardRead() {
    const text = await enki.clipboard.read();
    document.getElementById('clip-input').value = text;
    showToast('Read from Clipboard', text, 'success');
    logConsole('enki.clipboard.read', { text });
}

function testChangeTitle() {
    const newTitle = document.getElementById('win-title-input').value;
    enki.window.setTitle(newTitle);
    showToast('Window Title Updated', newTitle, 'success');
    logConsole('enki.window.setTitle', { newTitle });
}

function testShellOpen() {
    const url = document.getElementById('shell-url-input').value;
    enki.shell.openExternal(url);
    showToast('Opening Link', url, 'success');
    logConsole('enki.shell.openExternal', { url });
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

// ── Global Search ───────────────────────────────────────────
function initGlobalSearch() {
    const search = document.getElementById('global-search');
    search.addEventListener('keydown', (e) => {
        if (e.key === 'Enter') {
            const q = search.value.toLowerCase();
            if (q.includes('file') || q.includes('editor')) switchTab('filestudio');
            else if (q.includes('sys') || q.includes('mon') || q.includes('ram') || q.includes('cpu')) switchTab('sysmonitor');
            else if (q.includes('api') || q.includes('clip') || q.includes('dialog')) switchTab('apiplayground');
            else if (q.includes('ui') || q.includes('card')) switchTab('uishowcase');
            else switchTab('dashboard');
        }
    });
}

// ── Logging & Toast Helpers ─────────────────────────────────
function logConsole(action, data) {
    const consoleEl = document.getElementById('api-console-output');
    const timestamp = new Date().toLocaleTimeString();
    const entry = `[${timestamp}] ${action} → ${typeof data === 'object' ? JSON.stringify(data, null, 2) : data}\n`;
    consoleEl.textContent = entry + consoleEl.textContent;
}
function clearConsole() {
    document.getElementById('api-console-output').textContent = '// Console cleared.';
}

function showToast(title, body, type = 'info') {
    const container = document.getElementById('toast-container');
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

// ── Web Browser Tab Controls ────────────────────────────────
function navigateBrowser() {
    let url = document.getElementById('browser-url-input').value.trim();
    if (!url) return;
    if (!url.startsWith('http://') && !url.startsWith('https://') && !url.startsWith('file://')) {
        url = 'https://' + url;
    }
    document.getElementById('browser-url-input').value = url;
    document.getElementById('web-browser-frame').src = url;
    showToast('Navigating...', url, 'info');
}

function navigateToUrl(url) {
    document.getElementById('browser-url-input').value = url;
    document.getElementById('web-browser-frame').src = url;
    showToast('Opening Website', url, 'success');
}

function browserGoBack() {
    try {
        const frame = document.getElementById('web-browser-frame');
        frame.contentWindow.history.back();
    } catch (e) {
        showToast('Navigation', 'Back action triggered', 'info');
    }
}

function browserGoForward() {
    try {
        const frame = document.getElementById('web-browser-frame');
        frame.contentWindow.history.forward();
    } catch (e) {
        showToast('Navigation', 'Forward action triggered', 'info');
    }
}

function browserReload() {
    const frame = document.getElementById('web-browser-frame');
    frame.src = frame.src;
    showToast('Reloaded', 'Page refreshed', 'info');
}
