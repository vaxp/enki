# ============================================================
# ENKI Cubit Counter Showcase —    Hello World Architecture
# ============================================================

counter_sources = files(
  'src/main.cpp',
)

if is_windows
  configure_file(
    input: meson.project_source_root() / 'core/Skia-Windows/out/Release-x64/icudtl.dat',
    output: 'icudtl.dat',
    copy: true
  )
endif

enki_counter_exe = executable('enki_counter',
  sources: counter_sources,
  dependencies: [enki_dep],
  install: true,
)

