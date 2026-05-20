# ─────────────────────────────────────────────────────────────────────────────
# web_technology/subprocess/meson.build
# Builds the CEF helper subprocess binary (enki_cef_subprocess).
# ─────────────────────────────────────────────────────────────────────────────

executable('enki_cef_subprocess',
  'cef_subprocess_main.cpp',
  '../cef/EnkiCefApp.cpp',

  include_directories : [
    cef_inc,
    include_directories('../include'),
  ],

  dependencies : [
    cef_lib,
    cef_dll_wrapper,
    thread_dep,
  ] + (host_machine.system() == 'windows' ? [
    cpp.find_library('user32', required: true),
    cpp.find_library('shell32', required: true),
    cpp.find_library('ole32', required: true),
    cpp.find_library('advapi32', required: true),
  ] : []),

  # Disable warnings from CEF headers.
  cpp_args : ['-w'],

  install : true,
  install_dir : get_option('bindir'),
)
