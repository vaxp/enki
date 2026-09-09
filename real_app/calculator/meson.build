# ============================================================
# ENKI Quantum Calculator — Real Application Build
# ============================================================

calculator_sources = files(
  'src/calculator_engine.cpp',
  'src/main.cpp',
)

if is_windows
  configure_file(
    input: meson.project_source_root() / 'core/Skia-Windows/out/Release-x64/icudtl.dat',
    output: 'icudtl.dat',
    copy: true
  )
endif

enki_calculator_exe = executable('enki_calculator',
  sources: calculator_sources,
  include_directories: include_directories('include'),
  dependencies: [enki_dep],
  install: true,
)

