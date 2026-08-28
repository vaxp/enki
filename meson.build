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

add_project_arguments(
  '-Wno-unused-parameter',
  '-Wno-missing-field-initializers',
  language : 'cpp',
)

# ============================================================
# Native Linux Dependencies (Zero SDL)
# ============================================================
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

# ── All project-wide compile definitions — MUST come before any build target ──
# Meson prohibits add_project_arguments after the first build target is declared.
enki_compile_args = ['-DENKI_HAS_ANU=1']
if wayland_client_dep.found()
  enki_compile_args += '-DENKI_HAS_WAYLAND=1'
endif
if xrandr_dep.found()
  enki_compile_args += '-DENKI_HAS_XRANDR=1'
endif

add_project_arguments(enki_compile_args, language : 'cpp')

# ── Protocol code generation ──────────────────────────────────────────────────
# wayland-scanner produces plain C files (.c + .h), not C++.
# We compile them into a tiny static C lib, then expose headers to C++ code.

if wayland_scanner.found() and wayland_client_dep.found()
  proto_c_sources = []
  proto_h_sources = []

  foreach proto : ['wlr-layer-shell-unstable-v1', 'xdg-shell', 'wlr-foreign-toplevel-management-unstable-v1', 'xdg-output-unstable-v1']
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


# PulseAudio for live audio spectrum analysis
pulse_dep        = dependency('libpulse',        required : false)
pulse_simple_dep = dependency('libpulse-simple', required : false)

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

# Skia (prebuilt in core/skia)
skia_inc = include_directories('core/skia')
skia_dep = cpp.find_library('skia', dirs : [meson.current_source_dir() + '/core/skia/out/Release-x64'], required : false)
skparagraph_dep = cpp.find_library('skparagraph', dirs : [meson.current_source_dir() + '/core/skia/out/Release-x64'], required : false)
skshaper_dep = cpp.find_library('skshaper', dirs : [meson.current_source_dir() + '/core/skia/out/Release-x64'], required : false)
skunicode_dep = cpp.find_library('skunicode', dirs : [meson.current_source_dir() + '/core/skia/out/Release-x64'], required : false)

# Optional system Skia
if not skia_dep.found()
  skia_dep = dependency('skia', required : false)
  if not skia_dep.found()
    skia_dep = cpp.find_library('skia', required : false)
  endif
endif

# System libraries required by Skia
fontconfig_dep = dependency('fontconfig', required : false)
zlib_dep = dependency('zlib', required : false)
if not zlib_dep.found()
  zlib_dep = cpp.find_library('z', required : false)
endif

# ============================================================
# Collect all dependencies
# ============================================================
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

if skia_dep.found()
  # Wrap Skia with compile-time macros so downstream headers see SK_GL
  skia_with_flags = declare_dependency(
    dependencies  : [skia_dep],
    compile_args  : ['-DENKI_HAS_SKIA=1', '-DSK_GL'],
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
  if fontconfig_dep.found()
    enki_deps += fontconfig_dep
  endif
  if zlib_dep.found()
    enki_deps += zlib_dep
  endif
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

# ============================================================
# Feature detection macros — already emitted above
# ============================================================
# (add_project_arguments was called after the protocol static_library)

# ============================================================
# Include directories
# ============================================================
enki_inc          = include_directories('include', 'core/skia', 'core')
enki_platform_inc = [enki_inc]

# ============================================================
# Source files
# ============================================================

# Platform backend sources — always built
platform_sources = files(
  'src/platform/platform_native.cpp',
  'src/platform/window_native.cpp',
  'src/platform/input.cpp',
  # X11 backend
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
  'src/widgets/motion.cpp',
  'src/widgets/feedback_status.cpp',

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

  # Animation
  'src/animation/animation_controller.cpp',
  'src/animation/tween.cpp',
  'src/animation/curves.cpp',

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
# Library target
# ============================================================
enki_lib = library('enki',
  enki_sources,
  include_directories : enki_platform_inc,
  dependencies : enki_deps,
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
if get_option('build_tests')
  subdir('tests')
endif

# ============================================================
# Examples
# ============================================================
if get_option('build_examples')
 subdir('examples')
 subdir('widgets_demo')
endif



