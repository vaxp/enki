#pragma once
/// @file enki.hpp
/// @brief ENKI Master Widgets & UI Framework Header.
///
/// Comprehensive single-include header providing all declarative widgets,
/// layout models, styling, gesture recognizers, rendering abstractions,
/// and application lifecycle runner in the ENKI Linux Desktop Framework.
///
/// Usage:
/// @code
///   #include "enki/enki.hpp"
///   // or #include "enki.hpp"
/// @endcode
///
/// @copyright ENKI Framework — MIT License

// ════════════════════════════════════════════════════════════════
// 1. Core & Foundation
// ════════════════════════════════════════════════════════════════
#include "enki/core/types.hpp"
#include "enki/core/memory.hpp"
#include "enki/core/result.hpp"
#include "enki/core/signal.hpp"
#include "enki/core/string_utils.hpp"

// ════════════════════════════════════════════════════════════════
// 2. Rendering & Graphics Abstractions
// ════════════════════════════════════════════════════════════════
#include "enki/rendering/color.hpp"
#include "enki/rendering/paint.hpp"
#include "enki/rendering/canvas.hpp"
#include "enki/rendering/path.hpp"
#include "enki/rendering/image.hpp"
#include "enki/rendering/font_manager.hpp"
#include "enki/rendering/svg.hpp"
#include "enki/rendering/lottie_composition.hpp"

// ════════════════════════════════════════════════════════════════
// 3. Widget Tree & Element Lifecycle
// ════════════════════════════════════════════════════════════════
#include "enki/tree/key.hpp"
#include "enki/tree/widget.hpp"
#include "enki/tree/element.hpp"
#include "enki/tree/build_context.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/state/state.hpp"

// ════════════════════════════════════════════════════════════════
// 4. Gestures & Pointers
// ════════════════════════════════════════════════════════════════
#include "enki/gestures/gesture_types.hpp"
#include "enki/gestures/recognizer.hpp"

// ════════════════════════════════════════════════════════════════
// 5. Application & Shell Lifecycle
// ════════════════════════════════════════════════════════════════
#include "enki/app/app.hpp"
#include "enki/app/theme.hpp"
#include "enki/platform/platform.hpp"
#include "enki/platform/window.hpp"

// ════════════════════════════════════════════════════════════════
// 6. All Declarative Widgets
// ════════════════════════════════════════════════════════════════

// ── Layout, Containers & Structure ──────────────────────────────
#include "enki/widgets/container.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/stack.hpp"
#include "enki/widgets/flow.hpp"
#include "enki/widgets/clip.hpp"
#include "enki/widgets/divider.hpp"
#include "enki/widgets/limited_box.hpp"
#include "enki/widgets/overflow_box.hpp"
#include "enki/widgets/intrinsic_width.hpp"
#include "enki/widgets/intrinsic_height.hpp"
#include "enki/widgets/custom_multi_child_layout.hpp"
#include "enki/widgets/split_view.hpp"
#include "enki/widgets/resizable_panel.hpp"
#include "enki/widgets/floating_panel.hpp"

// ── Interaction, Gestures & Drag-Drop ───────────────────────────
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/draggable.hpp"
#include "enki/widgets/dismissible.hpp"
#include "enki/widgets/file_drop_zone.hpp"
#include "enki/widgets/focus.hpp"
#include "enki/widgets/reorderable_list.hpp"

// ── Typography & Text ───────────────────────────────────────────
#include "enki/widgets/text.hpp"
#include "enki/widgets/typography.hpp"
#include "enki/widgets/selectable_text.hpp"
#include "enki/widgets/code_block.hpp"
#include "enki/widgets/marquee.hpp"

// ── Buttons & Action Controls ───────────────────────────────────
#include "enki/widgets/button.hpp"
#include "enki/widgets/icon_button.hpp"
#include "enki/widgets/floating_action_button.hpp"
#include "enki/widgets/toggle_button.hpp"
#include "enki/widgets/segmented_control.hpp"
#include "enki/widgets/chip.hpp"
#include "enki/widgets/badge.hpp"

// ── Form Controls & Inputs ──────────────────────────────────────
#include "enki/widgets/form.hpp"
#include "enki/widgets/text_field.hpp"
#include "enki/widgets/text_area.hpp"
#include "enki/widgets/number_field.hpp"
#include "enki/widgets/password_field.hpp"
#include "enki/widgets/otp_field.hpp"
#include "enki/widgets/pin_field.hpp"
#include "enki/widgets/search_field.hpp"
#include "enki/widgets/tag_input.hpp"
#include "enki/widgets/checkbox.hpp"
#include "enki/widgets/radio.hpp"
#include "enki/widgets/switch.hpp"
#include "enki/widgets/slider.hpp"
#include "enki/widgets/range_slider.hpp"
#include "enki/widgets/rating_bar.hpp"
#include "enki/widgets/knob.hpp"
#include "enki/widgets/combo_box.hpp"
#include "enki/widgets/color_picker.hpp"
#include "enki/widgets/date_picker.hpp"
#include "enki/widgets/time_picker.hpp"
#include "enki/widgets/calendar.hpp"

// ── Menus, Overlays, Dialogs & Popups ───────────────────────────
#include "enki/widgets/menu.hpp"
#include "enki/widgets/dropdown_menu.hpp"
#include "enki/widgets/context_menu.hpp"
#include "enki/widgets/popover.hpp"
#include "enki/widgets/popup.hpp"
#include "enki/widgets/dialog.hpp"
#include "enki/widgets/bottom_sheet.hpp"
#include "enki/widgets/drawer.hpp"
#include "enki/widgets/sidebar.hpp"
#include "enki/widgets/navigation_bar.hpp"
#include "enki/widgets/navigation_rail.hpp"
#include "enki/widgets/navigator.hpp"
#include "enki/widgets/tab_bar.hpp"
#include "enki/widgets/breadcrumb.hpp"
#include "enki/widgets/accordion.hpp"
#include "enki/widgets/expansion_panel.hpp"
#include "enki/widgets/command_palette.hpp"
#include "enki/widgets/tooltip.hpp"
#include "enki/widgets/snackbar.hpp"
#include "enki/widgets/notification.hpp"
#include "enki/widgets/file_picker.hpp"

// ── Lists, Tables & Scroll Views ────────────────────────────────
#include "enki/widgets/list_view.hpp"
#include "enki/widgets/list_tile.hpp"
#include "enki/widgets/grid_view.hpp"
#include "enki/widgets/grid_tile.hpp"
#include "enki/widgets/table.hpp"
#include "enki/widgets/data_table.hpp"
#include "enki/widgets/data_grid.hpp"
#include "enki/widgets/tree_view.hpp"
#include "enki/widgets/scroll_view.hpp"
#include "enki/widgets/sliver.hpp"

// ── Visual Assets, Media & Rich Effects ─────────────────────────
#include "enki/widgets/image.hpp"
#include "enki/widgets/icon.hpp"
#include "enki/widgets/icons_material.hpp"
#include "enki/widgets/avatar.hpp"
#include "enki/widgets/card.hpp"
// Note: video_player.hpp and audio_waveform.hpp are excluded from the master header as they require dedicated media engines/runtimes.
#include "enki/widgets/skia_canvas.hpp"
#include "enki/widgets/lottie.hpp"
#include "enki/widgets/svg_morph.hpp"
#include "enki/widgets/paint_effects.hpp"
#include "enki/widgets/particle_emitter.hpp"
#include "enki/widgets/motion.hpp"
#include "enki/widgets/hero.hpp"
#include "enki/widgets/carousel.hpp"
#include "enki/widgets/timeline.hpp"
#include "enki/widgets/spotlight.hpp"
#include "enki/widgets/progress_bar.hpp"
#include "enki/widgets/progress_ring.hpp"
#include "enki/widgets/spinner.hpp"
#include "enki/widgets/feedback_status.hpp"
#include "enki/widgets/loading_overlay.hpp"
#include "enki/widgets/placeholder.hpp"
#include "enki/widgets/utility.hpp"

// ── Desktop Window Frame & CSD ──────────────────────────────────
#include "enki/widgets/window_frame.hpp"
#include "enki/widgets/titlebar.hpp"

// ── Mobile / System UI ──────────────────────────────────────────
#include "enki/widgets/safe_area.hpp"
#include "enki/widgets/aspect_ratio.hpp"
