project('enki', ['c', 'cpp'],
  version : '0.1.0',
  license : 'MIT',
  default_options : [
    'c_std=c11',
    'cpp_std=c++20',
    'warning_level=3',
    'werror=false',
    'optimization=2',
    'default_library=static',
  ],
  meson_version : '>= 1.1.0',
)

# ============================================================
# Compiler Configuration
# ============================================================
cpp = meson.get_compiler('cpp')

if cpp.get_id() == 'msvc'
  add_project_arguments(
    '/wd4100',
    '/wd4201',
    '/D_USE_MATH_DEFINES',
    '/DNOMINMAX',
    '/permissive-',
    '/Zc:__cplusplus',
    language : 'cpp',
  )
else
  add_project_arguments(
    '-Wno-unused-parameter',
    '-Wno-missing-field-initializers',
    '-D_USE_MATH_DEFINES',
    language : 'cpp',
  )
endif

is_windows = host_machine.system() == 'windows'
is_android = host_machine.system() == 'android'

# ============================================================
# Platform Dependencies
# ============================================================
if is_android
  # Android NDK system libraries (resolved by the cross-file toolchain)
  android_dep = cpp.find_library('android', required : true)
  log_dep     = cpp.find_library('log',     required : true)
  egl_dep     = cpp.find_library('EGL',     required : true)
  gles3_dep   = cpp.find_library('GLESv3',  required : true)
  thread_dep  = dependency('threads')

  android_deps = [android_dep, log_dep, egl_dep, gles3_dep, thread_dep]

  # Android does not use PulseAudio, Wayland, X11, FFmpeg, or VA-API
  pulse_dep        = dependency('libpulse',        required : false)
  pulse_simple_dep = dependency('libpulse-simple', required : false)
  avcodec_dep      = dependency('libavcodec',      required : false)
  avformat_dep     = dependency('libavformat',     required : false)
  avutil_dep       = dependency('libavutil',       required : false)
  swscale_dep      = dependency('libswscale',      required : false)
  swresample_dep   = dependency('libswresample',   required : false)
  va_dep           = dependency('libva',           required : false)
  va_drm_dep       = dependency('libva-drm',       required : false)
  protocol_dep     = declare_dependency()

  # Android: Wayland/X11/FFmpeg deps are absent — define as not-found placeholders
  wayland_client_dep = dependency('', required : false)
  wayland_egl_dep    = dependency('', required : false)
  wayland_scanner    = find_program('wayland-scanner', required : false)
  x11_dep            = dependency('', required : false)
  xrandr_dep         = dependency('', required : false)
  egl_dep            = dependency('', required : false)
  gl_dep             = dependency('', required : false)
  dl_dep             = cpp.find_library('dl', required : false)
  m_dep              = cpp.find_library('m',  required : false)

elif is_windows
  thread_dep = dependency('threads')
  opengl_dep = cpp.find_library('opengl32', required : true)
  gdi_dep    = cpp.find_library('gdi32', required : true)
  user32_dep = cpp.find_library('user32', required : true)
  dwmapi_dep = cpp.find_library('dwmapi', required : true)
  uxtheme_dep = cpp.find_library('uxtheme', required : true)
  ole32_dep  = cpp.find_library('ole32', required : true)
  shell32_dep = cpp.find_library('shell32', required : true)
  shcore_dep = cpp.find_library('shcore', required : false)

  windows_deps = [thread_dep, opengl_dep, gdi_dep, user32_dep, dwmapi_dep, uxtheme_dep, ole32_dep, shell32_dep]
  if shcore_dep.found()
    windows_deps += shcore_dep
  endif

  pulse_dep        = dependency('libpulse',        required : false)
  pulse_simple_dep = dependency('libpulse-simple', required : false)
  ffmpeg_win_dir = meson.current_source_dir() + '/core/ffmpeg-windows/lib'
  ffmpeg_inc_dir = include_directories('core/ffmpeg-windows/include')
  avcodec_lib    = cpp.find_library('avcodec',    dirs : [ffmpeg_win_dir], required : false)
  avformat_lib   = cpp.find_library('avformat',   dirs : [ffmpeg_win_dir], required : false)
  avutil_lib     = cpp.find_library('avutil',     dirs : [ffmpeg_win_dir], required : false)
  swscale_lib    = cpp.find_library('swscale',    dirs : [ffmpeg_win_dir], required : false)
  swresample_lib = cpp.find_library('swresample', dirs : [ffmpeg_win_dir], required : false)

  if avcodec_lib.found()
    avcodec_dep    = declare_dependency(dependencies : avcodec_lib, include_directories : ffmpeg_inc_dir)
    avformat_dep   = declare_dependency(dependencies : avformat_lib, include_directories : ffmpeg_inc_dir)
    avutil_dep     = declare_dependency(dependencies : avutil_lib, include_directories : ffmpeg_inc_dir)
    swscale_dep    = declare_dependency(dependencies : swscale_lib, include_directories : ffmpeg_inc_dir)
    swresample_dep = declare_dependency(dependencies : swresample_lib, include_directories : ffmpeg_inc_dir)
  else
    avcodec_dep    = dependency('', required : false)
    avformat_dep   = dependency('', required : false)
    avutil_dep     = dependency('', required : false)
    swscale_dep    = dependency('', required : false)
    swresample_dep = dependency('', required : false)
  endif
  va_dep           = dependency('libva',           required : false)
  va_drm_dep       = dependency('libva-drm',       required : false)
  protocol_dep     = declare_dependency()
else
  # Native Linux Dependencies (Zero SDL)
  x11_dep    = dependency('x11',     required : true)
  xrandr_dep = dependency('xrandr',  required : false)
  egl_dep    = dependency('egl',     required : true)
  gl_dep     = dependency('gl',      required : true)
  thread_dep = dependency('threads')
  dl_dep = cpp.find_library('dl', required : false)
  m_dep = cpp.find_library('m',  required : false)

  # Wayland client + EGL window support + keyboard handling
  wayland_client_dep  = dependency('wayland-client',  required : false)
  wayland_egl_dep     = dependency('wayland-egl',     required : false)
  wayland_cursor_dep  = dependency('wayland-cursor',  required : false)
  xkbcommon_dep       = dependency('xkbcommon',       required : false)

  # wayland-scanner: generates C protocol glue from XML specs
  wayland_scanner = find_program('wayland-scanner', required : false)

  # PulseAudio for live audio spectrum analysis
  pulse_dep        = dependency('libpulse',        required : false)
  pulse_simple_dep = dependency('libpulse-simple', required : false)

  # FFmpeg & VA-API for hardware-accelerated video decoding 
  avcodec_dep    = dependency('libavcodec',    required : true)
  avformat_dep   = dependency('libavformat',   required : true)
  avutil_dep     = dependency('libavutil',     required : true)
  swscale_dep    = dependency('libswscale',    required : true)
  swresample_dep = dependency('libswresample', required : true)
  va_dep         = dependency('libva',         required : false)
  va_drm_dep     = dependency('libva-drm',     required : false)
endif

# ── All project-wide compile definitions — MUST come before any build target ──
enki_compile_args = ['-DENKI_HAS_ANU=1']
if is_android
  enki_compile_args += '-DENKI_HAS_ANDROID=1'
  enki_compile_args += '-DENKI_PLATFORM_ANDROID=1'
elif not is_windows
  if wayland_client_dep.found()
    enki_compile_args += '-DENKI_HAS_WAYLAND=1'
  endif
  if xrandr_dep.found()
    enki_compile_args += '-DENKI_HAS_XRANDR=1'
  endif
endif

add_project_arguments(enki_compile_args, language : 'cpp')

# ── Protocol code generation ──────────────────────────────────────────────────
if not is_windows and not is_android and wayland_scanner.found() and wayland_client_dep.found()
  proto_c_sources = []
  proto_h_sources = []

  foreach proto : ['wlr-layer-shell-unstable-v1', 'xdg-shell', 'wlr-foreign-toplevel-management-unstable-v1', 'xdg-output-unstable-v1', 'xdg-decoration-unstable-v1']
    xml = 'protocols' / proto + '.xml'

    hdr = custom_target(proto + '-client-header',
      input  : files(xml),
      output : proto + '-client-protocol.h',
      command: [wayland_scanner, 'client-header', '@INPUT@', '@OUTPUT@'],
    )
    src = custom_target(proto + '-client-code',
      input  : files(xml),
      output : proto + '-protocol.c',
      command: [wayland_scanner, 'private-code', '@INPUT@', '@OUTPUT@'],
    )

    proto_h_sources += hdr
    proto_c_sources += src
  endforeach

  wayland_proto_lib = static_library('wayland_protocols',
    proto_c_sources,
    dependencies : [wayland_client_dep],
    override_options : ['warning_level=0'],
  )

  protocol_dep = declare_dependency(
    link_with : wayland_proto_lib,
    sources   : proto_h_sources,
  )
else
  protocol_dep = declare_dependency()
endif

# Anu layout engine (compiled directly)
anu_inc = include_directories('core')
anu_sources = files(
  'core/layout_engine/node/LayoutResults.cpp',
  'core/layout_engine/node/Node.cpp',
  'core/layout_engine/AnuValue.cpp',
  'core/layout_engine/AnuNode.cpp',
  'core/layout_engine/algorithm/PixelGrid.cpp',
  'core/layout_engine/algorithm/FlexLine.cpp',
  'core/layout_engine/algorithm/CalculateLayout.cpp',
  'core/layout_engine/algorithm/Cache.cpp',
  'core/layout_engine/algorithm/AbsoluteLayout.cpp',
  'core/layout_engine/algorithm/Baseline.cpp',
  'core/layout_engine/debug/Log.cpp',
  'core/layout_engine/debug/AssertFatal.cpp',
  'core/layout_engine/AnuNodeLayout.cpp',
  'core/layout_engine/AnuNodeStyle.cpp',
  'core/layout_engine/event/event.cpp',
  'core/layout_engine/AnuConfig.cpp',
  'core/layout_engine/AnuPixelGrid.cpp',
  'core/layout_engine/AnuEnums.cpp',
  'core/layout_engine/config/Config.cpp'
)

# Skia (system or prebuilt in core/skia)
skia_inc = include_directories(is_windows ? 'core/Skia-Windows' : 'core/skia')

if is_android
  # Skia for Android arm64 — built separately via NDK:
  #   cd core/skia
  #   bin/gn gen out/Release-android-arm64 --args='target_os="android" target_cpu="arm64" ndk="/path/to/ndk"'
  #   ninja -C out/Release-android-arm64
  skia_android_dir = meson.current_source_dir() + '/core/skia/out/Release-android-arm64'
  skia_dep        = cpp.find_library('skia',        dirs : [skia_android_dir], required : false)
  skparagraph_dep = cpp.find_library('skparagraph', dirs : [skia_android_dir], required : false)
  skshaper_dep    = cpp.find_library('skshaper',    dirs : [skia_android_dir], required : false)
  skunicode_dep   = cpp.find_library('skunicode',   dirs : [skia_android_dir], required : false)
  skottie_dep     = cpp.find_library('skottie',     dirs : [skia_android_dir], required : false)
  sksg_dep        = cpp.find_library('sksg',        dirs : [skia_android_dir], required : false)
  skresources_dep = cpp.find_library('skresources', dirs : [skia_android_dir], required : false)
elif is_windows
  skia_win_dir = meson.current_source_dir() + '/core/Skia-Windows/out/Release-x64'
  skia_dep = cpp.find_library('skia', dirs : [skia_win_dir], required : true)
  skparagraph_dep = cpp.find_library('skparagraph', dirs : [skia_win_dir], required : false)
  skshaper_dep    = cpp.find_library('skshaper',    dirs : [skia_win_dir], required : false)
  skunicode_dep   = cpp.find_library('skunicode',   dirs : [skia_win_dir], required : false)
  skottie_dep     = cpp.find_library('skottie',     dirs : [skia_win_dir], required : false)
  sksg_dep        = cpp.find_library('sksg',        dirs : [skia_win_dir], required : false)
  skresources_dep = cpp.find_library('skresources', dirs : [skia_win_dir], required : false)

  # ── ICU data file required by skunicode / SkParagraph ──────────────────────
  # Copy icudtl.dat to the build root so that every EXE in any subdirectory can
  # locate it at startup (Skia's SkIcuLoader searches CWD and EXE directory).
  icu_dat_src = meson.current_source_dir() + '/core/Skia-Windows/out/Release-x64/icudtl.dat'
  run_command(
    'cmd', '/c',
    'copy /Y "' + icu_dat_src + '" "' + meson.current_build_dir() + '\\icudtl.dat"',
    check : false,
  )

  # ── FFmpeg DLLs required by enki_video / video_player_demo ────────────────
  ffmpeg_dll_src = meson.current_source_dir() + '\\core\\ffmpeg-windows\\bin\\*.dll'
  run_command(
    'cmd', '/c',
    'copy /Y "' + ffmpeg_dll_src + '" "' + meson.current_build_dir() + '\\"',
    check : false,
  )
else
  skia_dep = cpp.find_library('skia', dirs : [meson.current_source_dir() + '/core/skia/out/Release-x64'], required : false)
  if not skia_dep.found()
    skia_dep = cpp.find_library('skia', required : false)
  endif
  skparagraph_dep = cpp.find_library('skparagraph', dirs : [meson.current_source_dir() + '/core/skia/out/Release-x64'], required : false)
  skshaper_dep    = cpp.find_library('skshaper',    dirs : [meson.current_source_dir() + '/core/skia/out/Release-x64'], required : false)
  skunicode_dep   = cpp.find_library('skunicode',   dirs : [meson.current_source_dir() + '/core/skia/out/Release-x64'], required : false)
  skottie_dep     = cpp.find_library('skottie',     dirs : [meson.current_source_dir() + '/core/skia/out/Release-x64'], required : false)
  sksg_dep        = cpp.find_library('sksg',        dirs : [meson.current_source_dir() + '/core/skia/out/Release-x64'], required : false)
  skresources_dep = cpp.find_library('skresources', dirs : [meson.current_source_dir() + '/core/skia/out/Release-x64'], required : false)
endif

# System libraries required by Skia
# Android: fontconfig not available; zlib is part of the NDK sysroot
if is_windows or is_android
  fontconfig_dep = dependency('', required : false)
  zlib_dep = dependency('', required : false)
else
  fontconfig_dep = dependency('fontconfig', required : false)
  zlib_dep = dependency('zlib', required : false)
  if not zlib_dep.found()
    zlib_dep = cpp.find_library('z', required : false)
  endif
endif

# ============================================================
# Collect all dependencies
# ============================================================
if is_android
  enki_deps = android_deps
elif is_windows
  enki_deps = windows_deps
else
  enki_deps = [thread_dep, x11_dep, egl_dep, gl_dep]

  if xrandr_dep.found()
    enki_deps += [xrandr_dep]
  endif
  if wayland_client_dep.found()
    enki_deps += [wayland_client_dep]
  endif
  if wayland_egl_dep.found()
    enki_deps += [wayland_egl_dep]
  endif
  if wayland_cursor_dep.found()
    enki_deps += [wayland_cursor_dep]
  endif
  if xkbcommon_dep.found()
    enki_deps += [xkbcommon_dep]
  endif
  if dl_dep.found()
    enki_deps += dl_dep
  endif
  if m_dep.found()
    enki_deps += m_dep
  endif
  if pulse_dep.found()
    enki_deps += pulse_dep
  endif
  if pulse_simple_dep.found()
    enki_deps += pulse_simple_dep
  endif

  # Add generated Wayland protocol headers as a dependency
  enki_deps += protocol_dep
endif

if skia_dep.found()
  # Wrap Skia with compile-time macros so downstream headers see SK_GL
  skia_with_flags = declare_dependency(
    dependencies  : [skia_dep],
    compile_args  : ['-DENKI_HAS_SKIA=1', '-DSK_GL', '-DSK_GANESH'],
  )
  enki_deps += [skia_with_flags]
  if skparagraph_dep.found()
    enki_deps += skparagraph_dep
  endif
  if skshaper_dep.found()
    enki_deps += skshaper_dep
  endif
  if skunicode_dep.found()
    enki_deps += skunicode_dep
  endif
  if skottie_dep.found()
    enki_deps += skottie_dep
  endif
  if sksg_dep.found()
    enki_deps += sksg_dep
  endif
  if skresources_dep.found()
    enki_deps += skresources_dep
  endif
  if fontconfig_dep.found()
    enki_deps += fontconfig_dep
  endif
  if zlib_dep.found()
    enki_deps += zlib_dep
  endif
  if is_windows
    foreach extra_lib : ['icu', 'harfbuzz', 'freetype2', 'libpng', 'libjpeg', 'libwebp', 'zlib', 'expat']
      l = cpp.find_library(extra_lib, dirs : [skia_win_dir], required : false)
      if l.found()
        enki_deps += l
      endif
    endforeach
  endif
else
  # Skia headers are included in enki_inc; compile flags are required
  skia_compile_flags = declare_dependency(
    compile_args : ['-DENKI_HAS_SKIA=1', '-DSK_GL', '-DSK_GANESH'],
  )
  enki_deps += [skia_compile_flags]
endif

# ============================================================
# Include directories
# ============================================================
enki_inc          = include_directories('include', is_windows ? 'core/Skia-Windows' : 'core/skia', 'core')
enki_platform_inc = [enki_inc]

# ============================================================
# Source files
# ============================================================

# Platform backend sources
platform_sources = files(
  'src/platform/platform_native.cpp',
  'src/platform/window_native.cpp',
  'src/platform/input.cpp',
)

if is_android
  platform_sources += files(
    'src/platform/android/android_platform.cpp',
    'src/platform/android/android_surface.cpp',
    'src/platform/android/android_app_glue.cpp',
  )
elif is_windows
  platform_sources += files(
    'src/platform/windows/win32_platform.cpp',
    'src/platform/windows/win32_window.cpp',
  )
else
  # X11 backend (always present on Linux)
  platform_sources += files(
    'src/platform/x11/x11_platform.cpp',
    'src/platform/x11/x11_window.cpp',
  )

  # Wayland backend — only built when wayland-client + wayland-scanner are available
  if wayland_scanner.found() and wayland_client_dep.found()
    platform_sources += files(
      'src/platform/wayland/wayland_platform.cpp',
      'src/platform/wayland/wayland_surface.cpp',
      'src/platform/wayland/wayland_window.cpp',
    )
  endif
endif

enki_sources = files(
  # Core
  'src/core/memory.cpp',
  'src/core/string_utils.cpp',

  # App
  'src/app/app.cpp',
  'src/app/theme.cpp',

  # Rendering
  'src/rendering/canvas.cpp',
  'src/rendering/paint.cpp',
  'src/rendering/color.cpp',
  'src/rendering/path.cpp',
  'src/rendering/image.cpp',
  'src/rendering/svg.cpp',
  'src/rendering/font_manager.cpp',

  # Widget Tree
  'src/tree/widget.cpp',
  'src/tree/element.cpp',
  'src/tree/render_object.cpp',
  'src/tree/reconciler.cpp',
  'src/tree/build_context.cpp',
  'src/tree/key.cpp',

  # State Management
  'src/state/state.cpp',
  'src/state/cubit.cpp',
  'src/state/bloc.cpp',
  'src/state/stream.cpp',
  'src/state/bloc_provider.cpp',
  'src/state/bloc_builder.cpp',
  'src/state/bloc_listener.cpp',

  # Gestures
  'src/gestures/recognizer.cpp',

  # Widgets
  'src/widgets/flexbox.cpp',
  'src/widgets/container.cpp',
  'src/widgets/button.cpp',
  'src/widgets/avatar.cpp',
  'src/widgets/text.cpp',
  'src/widgets/text_field.cpp',
  'src/widgets/text_area.cpp',
  'src/widgets/password_field.cpp',
  'src/widgets/number_field.cpp',
  'src/widgets/search_field.cpp',
  'src/widgets/combo_box.cpp',
  'src/widgets/date_picker.cpp',
  'src/widgets/time_picker.cpp',
  'src/widgets/color_picker.cpp',
  'src/widgets/placeholder.cpp',
  'src/widgets/chip.cpp',
  'src/widgets/form.cpp',
  'src/widgets/reorderable_list.cpp',
  'src/widgets/gesture_detector.cpp',
  'src/widgets/draggable.cpp',
  'src/widgets/dismissible.cpp',
  'src/widgets/focus.cpp',
  'src/widgets/stack.cpp',
  'src/widgets/image.cpp',
  'src/widgets/lottie.cpp',
  'src/widgets/scroll_view.cpp',
  'src/widgets/icon.cpp',
  'src/widgets/divider.cpp',
  'src/widgets/checkbox.cpp',
  'src/widgets/switch.cpp',
  'src/widgets/radio.cpp',
  'src/widgets/card.cpp',
  'src/widgets/badge.cpp',
  'src/widgets/tooltip.cpp',
  'src/widgets/context_menu.cpp',
  'src/widgets/popover.cpp',
  'src/widgets/file_picker.cpp',
  'src/widgets/bottom_sheet.cpp',
  'src/widgets/dropdown_menu.cpp',
  'src/widgets/dialog.cpp',
  'src/widgets/snackbar.cpp',
  'src/widgets/notification.cpp',
  'src/widgets/loading_overlay.cpp',
  'src/widgets/popup.cpp',
  'src/widgets/menu.cpp',
  'src/widgets/clip.cpp',
  'src/widgets/paint_effects.cpp',
  'src/widgets/skia_canvas.cpp',
  'src/widgets/motion.cpp',
  'src/widgets/feedback_status.cpp',
  'src/widgets/utility.cpp',
  'src/widgets/marquee.cpp',
  'src/widgets/code_block.cpp',
  'src/widgets/titlebar.cpp',
  'src/widgets/window_frame.cpp',
  'src/widgets/spotlight.cpp',
  'src/widgets/floating_panel.cpp',
  'src/widgets/command_palette.cpp',

  # Layout — Extended (Roadmap v0.2.0 Section 11)
  'src/widgets/intrinsic_width.cpp',
  'src/widgets/intrinsic_height.cpp',
  'src/widgets/overflow_box.cpp',
  'src/widgets/limited_box.cpp',
  'src/widgets/custom_multi_child_layout.cpp',
  'src/widgets/flow.cpp',

  # Rich Input Controls (Roadmap v0.2.0 Section 15)
  'src/widgets/toggle_button.cpp',
  'src/widgets/segmented_control.cpp',
  'src/widgets/rating_bar.cpp',
  'src/widgets/knob.cpp',
  'src/widgets/otp_field.cpp',
  'src/widgets/pin_field.cpp',
  'src/widgets/tag_input.cpp',
  'src/widgets/file_drop_zone.cpp',

  'src/widgets/icon_button.cpp',
  'src/widgets/floating_action_button.cpp',
  'src/widgets/slider.cpp',
  'src/widgets/range_slider.cpp',
  'src/widgets/progress_bar.cpp',
  'src/widgets/progress_ring.cpp',
  'src/widgets/spinner.cpp',

  # Scrolling / Lists Widgets
  'src/widgets/list_tile.cpp',
  'src/widgets/list_view.cpp',
  'src/widgets/grid_tile.cpp',
  'src/widgets/grid_view.cpp',
  'src/widgets/sliver.cpp',
  'src/widgets/tree_view.cpp',
  'src/widgets/table.cpp',
  'src/widgets/data_table.cpp',
  'src/widgets/data_grid.cpp',
  'src/widgets/timeline.cpp',
  'src/widgets/accordion.cpp',
  'src/widgets/split_view.cpp',
  'src/widgets/carousel.cpp',
  'src/widgets/calendar.cpp',
  'src/widgets/expansion_panel.cpp',
  'src/widgets/resizable_panel.cpp',

  # Navigation Widgets
  'src/widgets/tab_bar.cpp',
  'src/widgets/navigation_bar.cpp',
  'src/widgets/navigation_rail.cpp',
  'src/widgets/breadcrumb.cpp',
  'src/widgets/sidebar.cpp',
  'src/widgets/drawer.cpp',
  'src/widgets/navigator.cpp',
  'src/widgets/hero.cpp',

  # Animation
  'src/animation/animation_controller.cpp',
  'src/animation/lottie_controller.cpp',
  'src/animation/tween.cpp',
  'src/animation/curves.cpp',
  'src/animation/spring_simulation.cpp',
  'src/animation/spring_controller.cpp',
  'src/animation/timeline.cpp',
  'src/animation/path_morph.cpp',
  'src/animation/particle_system.cpp',

  # Desktop Shell Subsystem
  'src/shell/surface_host.cpp',
  'src/shell/shell_app.cpp',
  'src/shell/native_popup.cpp',
) + platform_sources

anu_lib = static_library('anucore',
  anu_sources,
  include_directories: anu_inc,
  override_options: ['werror=false', 'warning_level=0']
)

anu_dep = declare_dependency(
  include_directories: anu_inc,
  link_with: anu_lib
)

enki_deps += anu_dep

# ============================================================
# WebView / CEF Subsystem (optional)
# ============================================================
if get_option('enable_webview')
  subdir('modules/web_technology')
  enki_deps += web_technology_dep
endif

# ============================================================
# Audio Subsystem (Capture & FFT DSP Analyzer)
# ============================================================
if pulse_dep.found() or host_machine.system() == 'windows'
  subdir('modules/audio')
  enki_deps += audio_dep
  enki_sources += files('src/widgets/audio_waveform.cpp')
endif

# ============================================================
# Video Subsystem (FFmpeg Zero-Copy & PulseAudio Master Clock)
# ============================================================
if avcodec_dep.found()
  subdir('modules/video')
  enki_deps += video_dep
  enki_sources += files('src/widgets/video_player.cpp')
endif

# ============================================================
# Lottie Rendering Subsystem (-fno-rtti to match Skia Skottie)
# ============================================================
lottie_cpp_args = cpp.get_id() == 'msvc' ? ['/GR-'] : ['-fno-rtti']
if skottie_dep.found()
  lottie_cpp_args += '-DENKI_HAS_SKOTTIE=1'
endif

lottie_rendering_lib = static_library('enki_lottie_rendering',
  'src/rendering/lottie_composition.cpp',
  include_directories : enki_platform_inc,
  dependencies : enki_deps,
  cpp_args : lottie_cpp_args,
  pic : true,
)

# ============================================================
# Library target
# ============================================================
enki_lib = library('enki',
  enki_sources,
  include_directories : enki_platform_inc,
  dependencies : enki_deps,
  link_whole : [lottie_rendering_lib],
  install : true,
)

enki_dep = declare_dependency(
  include_directories : enki_platform_inc,
  link_with : enki_lib,
  dependencies : enki_deps,
)

# ============================================================
# Install headers
# ============================================================
install_subdir('include/enki', install_dir : get_option('includedir'))

# ============================================================
# Tests
# ============================================================
if get_option('build_tests') and not is_android
#  subdir('tests')
endif

# ============================================================
# Examples
# ============================================================
if get_option('build_examples') and not is_android
  subdir('examples')
endif

# ============================================================
# Widgets Demo
# ============================================================
if get_option('build_widgets_demo') and not is_android
  subdir('widgets_demo')
endif

# ============================================================
# Real Applications
# ============================================================
subdir('real_app')
