# ═════════════════════════════════════════════════════════════════════════════
# video/meson.build
#
# Builds the standalone native Video Subsystem (FFmpeg Zero-Copy & PulseAudio Master Clock)
# as an isolated static library.
#
# 
# ═════════════════════════════════════════════════════════════════════════════

video_inc = include_directories('include')

video_sources = files(
  'src/video_decoder.cpp',
  'src/video_controller.cpp',
)

video_deps = [
  avcodec_dep,
  avformat_dep,
  avutil_dep,
  swscale_dep,
  swresample_dep,
  thread_dep,
]

if pulse_simple_dep.found()
  video_deps += pulse_simple_dep
endif
if va_dep.found()
  video_deps += va_dep
endif
if va_drm_dep.found()
  video_deps += va_drm_dep
endif

if host_machine.system() == 'windows'
  video_sources += files('src/video_decoder_win.cpp')
  d3d11_dep = cpp.find_library('d3d11', required : true)
  dxgi_dep  = cpp.find_library('dxgi', required : true)
  video_deps += [d3d11_dep, dxgi_dep, ole32_dep]
endif

libenki_video = static_library('enki_video',
  video_sources,
  include_directories : video_inc,
  dependencies : video_deps,
  install : false,
)

video_dep = declare_dependency(
  link_with : libenki_video,
  include_directories : video_inc,
  dependencies : video_deps,
)
