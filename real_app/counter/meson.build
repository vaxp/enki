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

if is_android
  enki_counter_lib = shared_library('enki_counter',
    sources: counter_sources,
    dependencies: [enki_dep],
    name_prefix: 'lib',
    link_args: ['-u', 'ANativeActivity_onCreate'],
    install: true,
  )
else
  enki_counter_exe = executable('enki_counter',
    sources: counter_sources,
    dependencies: [enki_dep],
    install: true,
  )
endif

