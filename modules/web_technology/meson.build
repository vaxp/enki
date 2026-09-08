# ═════════════════════════════════════════════════════════════════════════════
# web_technology/meson.build
#
# Builds the CEF Web Host subsystem as an isolated static library.
#
# Activated by:
#   meson setup build -Denable_webview=true [-Dcef_root=/path/to/cef]
# ═════════════════════════════════════════════════════════════════════════════

fs = import('fs')
cef_root = get_option('cef_root')

local_cef_rel = 'cef_binary/cef_binary_144.0.34+g8fc21c8+chromium-144.0.7559.261_linux64_minimal'
local_cef_abs = meson.current_source_dir() / local_cef_rel

# Auto-detect local prebuilt folder if cef_root is not explicitly provided
if cef_root == ''
  if fs.is_dir(local_cef_abs)
    cef_root = local_cef_abs
    cef_inc = include_directories(local_cef_rel, 'include')
  endif
endif

if cef_root == ''
  # Try to find CEF via pkg-config (some distros package it).
  cef_dep_pkgconfig = dependency('cef', required : false)
  if cef_dep_pkgconfig.found()
    cef_lib         = cef_dep_pkgconfig
    cef_dll_wrapper = dependency('', required : false)
    cef_root_found  = true
    cef_inc         = include_directories('include')
  else
    warning('web_technology: CEF not found. Web Host will not be available.')
    warning('  Set -Dcef_root=/path/to/cef-prebuilt to enable Web Host.')
    web_technology_dep = declare_dependency()
    subdir_done()
  endif
else
  cef_root_found = true

  cef_lib = cpp.find_library('cef',
    dirs    : [cef_root / 'Release'],
    required: true)

  # cef_dll_wrapper is built from source alongside CEF.
  cef_dll_wrapper = cpp.find_library('cef_dll_wrapper',
    dirs    : [
      cef_root / 'build' / 'libcef_dll_wrapper',
      cef_root / 'libcef_dll_wrapper',
    ],
    required: false)

  if not is_variable('cef_inc')
    cef_inc = include_directories(cef_root, 'include')
  endif
endif

# ── CEF helper subprocess ─────────────────────────────────────────────────────

subdir('subprocess')

# ── web_technology library ────────────────────────────────────────────────────

web_sources = files(
  # CEF subsystem core
  'cef/CefBridge.cpp',
  'cef/EnkiCefApp.cpp',
  'cef/EnkiCefClient.cpp',
  'cef/EnkiRenderHandler.cpp',
  'cef/EnkiLifeSpanHandler.cpp',
  'cef/InputForwarder.cpp',
  'cef/JSBridge.cpp',

  # Web Host & Native APIs
  'host/AppConfig.cpp',
  'host/NativeAPIs.cpp',
  'host/EnkiWebHost.cpp',
)

web_deps = [cef_lib, thread_dep]
if cef_dll_wrapper.found()
  web_deps += cef_dll_wrapper
endif

web_technology_lib = static_library('enki_web',
  web_sources,

  include_directories : cef_inc,
  dependencies        : web_deps,

  # CEF headers emit many warnings — suppress them
  cpp_args : [
    '-DENKI_HAS_WEBVIEW=1',
    '-w',
  ],

  override_options : ['warning_level=0'],
)

# ── Exported dependency ───────────────────────────────────────────────────────

web_technology_dep = declare_dependency(
  link_with           : web_technology_lib,
  include_directories : include_directories('include'),
  compile_args        : ['-DENKI_HAS_WEBVIEW=1'],
)

message('web_technology: CEF Web Host subsystem enabled ✓ (' + cef_root + ')')
