# ============================================================
# ENKI Gallery — Real Application Build
# ============================================================

gallery_sources = files(
  'src/main.cpp',
  'src/services/storage_service.cpp',
  'src/state/gallery_cubit.cpp',
)

gallery_inc = include_directories('src')

if is_windows
  configure_file(
    input: meson.project_source_root() / 'core/Skia-Windows/out/Release-x64/icudtl.dat',
    output: 'icudtl.dat',
    copy: true
  )
endif

if is_android
  shared_library('enki_gallery',
    sources: gallery_sources,
    include_directories: [gallery_inc],
    dependencies: [enki_dep],
    name_prefix: 'lib',
    link_args: ['-u', 'ANativeActivity_onCreate'],
    install: true,
  )
elif is_wasm
  executable('enki_gallery',
    sources: gallery_sources,
    include_directories: [gallery_inc],
    dependencies: [enki_dep],
    link_args: [
      '-s', 'WASM=1',
      '-s', 'USE_WEBGL2=1',
      '-s', 'FULL_ES3=1',
      '-s', 'ALLOW_MEMORY_GROWTH=1',
      '-s', 'INITIAL_MEMORY=67108864',
      '-s', 'STACK_SIZE=8388608',
      '-s', 'NO_EXIT_RUNTIME=1',
      '-s', 'USE_HARFBUZZ=1',
      '-s', 'EMULATE_FUNCTION_POINTER_CASTS=1',
      '-s', 'ASSERTIONS=2',
      '-g2',
      '--embed-file', meson.project_source_root() / 'assets/fonts@/assets/fonts',
    ],
    name_suffix: 'js',
    install: true,
  )
else
  executable('enki_gallery',
    sources: gallery_sources,
    include_directories: [gallery_inc],
    dependencies: [enki_dep],
    install: true,
  )
endif
