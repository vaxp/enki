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

if is_android
  shared_library('enki_calculator',
    sources: calculator_sources,
    include_directories: include_directories('include'),
    dependencies: [enki_dep],
    name_prefix: 'lib',
    link_args: ['-u', 'ANativeActivity_onCreate'],
    install: true,
  )
else
  executable('enki_calculator',
    sources: calculator_sources,
    include_directories: include_directories('include'),
    dependencies: [enki_dep],
    install: true,
  )
endif
