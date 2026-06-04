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
else
  executable('enki_gallery',
    sources: gallery_sources,
    include_directories: [gallery_inc],
    dependencies: [enki_dep],
    install: true,
  )
endif
