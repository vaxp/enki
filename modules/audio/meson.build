# ═════════════════════════════════════════════════════════════════════════════
# audio/meson.build
#
# Builds the standalone native Audio Subsystem (Capture & FFT DSP Analyzer)
# as an isolated static library.
#
# ═════════════════════════════════════════════════════════════════════════════

audio_inc = include_directories('include')

audio_sources = files(
  'src/audio_capture.cpp',
  'src/audio_analyzer.cpp',
  'src/audio_controller.cpp',
)

audio_deps = []
if host_machine.system() == 'windows'
  audio_deps += cpp.find_library('ole32', required : true)
else
  if pulse_dep.found()
    audio_deps += pulse_dep
  endif
  if pulse_simple_dep.found()
    audio_deps += pulse_simple_dep
  endif
endif

audio_cpp_args = []
if cpp.get_id() != 'msvc'
  audio_cpp_args += ['-std=c++20']
endif

audio_lib = static_library('enki_audio',
  audio_sources,
  include_directories : [audio_inc],
  dependencies : audio_deps,
  cpp_args : audio_cpp_args,
  pic : true,
)

audio_dep = declare_dependency(
  include_directories : audio_inc,
  link_with : audio_lib,
  dependencies : audio_deps,
)
