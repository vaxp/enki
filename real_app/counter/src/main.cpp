/// @file main.cpp
/// @brief ENKI Cubit Counter Showcase —    Hello World with Real-Time Tree Rebuild Verification.
///
/// Demonstrates:
///   1. Cubit<CounterState> reactive state management.
///   2. BlocProvider<CounterCubit> dependency injection.
///   3. BlocBuilder<CounterCubit> granular reactivity (rebuilding ONLY the counter).
///   4. Real-time Element Tree printing to stdout proving whole page is NOT rebuilt.

#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/window_frame.hpp"
#include "enki/widgets/titlebar.hpp"
#include "enki/state/cubit.hpp"
#include "enki/state/bloc_provider.hpp"
#include "enki/state/bloc_builder.hpp"
#include "enki/tree/element.hpp"
#include "enki/tree/build_context.hpp"

#include <iostream>
#include <memory>
#include <string>

using namespace enki;

// ════════════════════════════════════════════════════════════════
// 1. State & Cubit
// ════════════════════════════════════════════════════════════════

struct CounterState {
    int value = 0;
    bool operator==(const CounterState& other) const = default;
};

class CounterCubit : public Cubit<CounterState> {
public:
    CounterCubit() : Cubit(CounterState{0}) {
        std::cout << "[CounterCubit] Constructed!\n";
    }

    void increment() {
        std::cout << "[CounterCubit] increment() called!\n";
        emit(CounterState{state().value + 1});
    }

    void decrement() {
        std::cout << "[CounterCubit] decrement() called!\n";
        emit(CounterState{state().value - 1});
    }

    void reset() {
        std::cout << "[CounterCubit] reset() called!\n";
        emit(CounterState{0});
    }
};

// ════════════════════════════════════════════════════════════════
// 2. Tree Rebuild Diagnostic Tracker
// ════════════════════════════════════════════════════════════════

static int s_page_rebuild_count = 0;
static int s_builder_rebuild_count = 0;

void printElementTree(Element* node, int indent = 0, Element* target = nullptr) {
    if (!node) return;
    std::string prefix = "";
    for (int i = 0; i < indent; ++i) {
        prefix += (i == indent - 1) ? "├── " : "│   ";
    }

    std::string name = node->widget() ? std::string(node->widget()->typeName()) : "Element";
    std::string highlight = "";
    if (node == target) {
        highlight = "  <=== [REBUILT! ONLY THIS SUBTREE (GRANULAR)]";
    }

    std::cout << prefix << name << highlight << "\n";

    node->visitChildren([&](Element& child) {
        printElementTree(&child, indent + 1, target);
    });
}

// ════════════════════════════════════════════════════════════════
// 3. Action Buttons with Hover & Click
// ════════════════════════════════════════════════════════════════

class ActionButton : public StatefulWidget {
public:
    std::string label;
    Color bg_color;
    Color hover_color;
    Color text_color;
    float width_val;
    float height_val;
    float font_size_val;
    std::function<void()> on_tap;

    ActionButton(std::string label, Color bg, Color hover, Color txt, float w, float h, float font_sz, std::function<void()> cb)
        : label(std::move(label)), bg_color(bg), hover_color(hover), text_color(txt),
          width_val(w), height_val(h), font_size_val(font_sz), on_tap(std::move(cb)) {}

    ActionButton(std::string label, Color bg, Color hover, Color txt, float w, float h, std::function<void()> cb)
        : ActionButton(std::move(label), bg, hover, txt, w, h, 22.0f, std::move(cb)) {}

    std::unique_ptr<State> createState() override;
    std::string_view typeName() const override { return "ActionButton"; }
};

class ActionButtonState : public State {
public:
    bool is_hovered = false;
    bool is_pressed = false;

    WidgetPtr build(BuildContext&) override {
        auto* btn = static_cast<const ActionButton*>(widget());

        Color cur_bg = is_pressed ? 0xFF1E3A8A : (is_hovered ? btn->hover_color : btn->bg_color);
        StyleInsets press_margin = is_pressed ? StyleInsets::only(1.0f, 0.0f, 0.0f, 0.0f) : StyleInsets{};

        float btn_border_w = btn->height_val > 90.0f ? 3.0f : 1.5f;
        float btn_shadow_blur = btn->height_val > 90.0f ? 16.0f : 8.0f;
        float btn_shadow_off_y = btn->height_val > 90.0f ? 6.0f : 4.0f;

        auto box = container({
            .color = cur_bg,
            .border_radius = BorderRadius::circular(btn->height_val * 0.5f),
            .border = Border(is_hovered ? 0xFF93C5FD : 0x403B82F6, btn_border_w),
            .box_shadow = is_hovered ? std::vector<BoxShadow>{ BoxShadow::glow(0x603B82F6, btn_shadow_blur * 1.5f) }
                                     : std::vector<BoxShadow>{ BoxShadow(0x30000000, {0.0f, btn_shadow_off_y}, btn_shadow_blur) },
            .align = Alignment::Center,
            .width = StyleValue::point(btn->width_val),
            .height = StyleValue::point(btn->height_val),
            .margin = press_margin,
            .child = text(btn->label, {
                .color = btn->text_color,
                .font_size = btn->font_size_val,
                .font_weight = FontWeight::Bold
            })
        });

        GestureDetectorProps gprops;
        gprops.child = box;
        gprops.cursor_type = SystemCursor::Pointer;
        gprops.on_hover_enter = [this](const PointerEvent&) { setState([this]{ is_hovered = true; }); };
        gprops.on_hover_exit  = [this](const PointerEvent&) { setState([this]{ is_hovered = false; is_pressed = false; }); };
        gprops.on_tap_down    = [this](const TapDownDetails&) { setState([this]{ is_pressed = true; }); };
        gprops.on_tap_up      = [this](const TapUpDetails&) { setState([this]{ is_pressed = false; }); };
        gprops.on_tap_cancel  = [this]() { setState([this]{ is_pressed = false; }); };
        gprops.on_tap         = [btn]() { if (btn->on_tap) btn->on_tap(); };

        return gestureDetector(std::move(gprops));
    }
};

inline std::unique_ptr<State> ActionButton::createState() {
    return std::make_unique<ActionButtonState>();
}

// ════════════════════════════════════════════════════════════════
// 3.5. Responsive Layout Dimensions (Desktop & Mobile Profiles)
// ════════════════════════════════════════════════════════════════

struct LayoutDimensions {
    // Safe Area & Status Bar
    float status_bar_top;

    // Header / App Bar
    float app_bar_pad_v;
    float app_bar_pad_h;
    float app_bar_title_size;

    // Headings and Main Counter
    float prompt_font_size;
    float counter_font_size;
    float counter_pad_v;
    float counter_pad_h;
    float counter_radius;
    float counter_margin_v;

    // Telemetry Card
    float card_pad;
    float card_margin_v;
    float card_title_size;
    float card_body_size;
    float card_gap;
    float card_max_width;

    // Action Buttons
    float btn_normal_size;
    float btn_accent_size;
    float btn_font_size;
    float btn_gap;
    float action_row_margin_top;

    // Status Badge
    float badge_pad_v;
    float badge_pad_h;
    float badge_font_size;
    float badge_margin_top;

    // Spacing
    float page_pad;
    float column_gap;
};

// ── Baseline Dimensions (Original Desktop Baseline 520x640) ──────
[[maybe_unused]] constexpr LayoutDimensions kOriginalDimensions {
    .status_bar_top        = 0.0f,
    .app_bar_pad_v         = 14.0f,
    .app_bar_pad_h         = 20.0f,
    .app_bar_title_size    = 17.0f,

    .prompt_font_size      = 16.0f,
    .counter_font_size     = 56.0f,
    .counter_pad_v         = 14.0f,
    .counter_pad_h         = 48.0f,
    .counter_radius        = 20.0f,
    .counter_margin_v      = 16.0f,

    .card_pad              = 14.0f,
    .card_margin_v         = 12.0f,
    .card_title_size       = 13.0f,
    .card_body_size        = 12.0f,
    .card_gap              = 6.0f,
    .card_max_width        = 440.0f,

    .btn_normal_size       = 56.0f,
    .btn_accent_size       = 64.0f,
    .btn_font_size         = 22.0f,
    .btn_gap               = 16.0f,
    .action_row_margin_top = 16.0f,

    .badge_pad_v           = 8.0f,
    .badge_pad_h           = 16.0f,
    .badge_font_size       = 13.0f,
    .badge_margin_top      = 16.0f,

    .page_pad              = 24.0f,
    .column_gap            = 8.0f,
};

// ── Mobile Dimensions (Phones with High-DPI Touchscreens: 1080p - 1440p) ──
[[maybe_unused]] constexpr LayoutDimensions kMobileDimensions {
    .status_bar_top        = 110.0f,    // Clear Android system status bar, notch & camera cutout
    .app_bar_pad_v         = 36.0f,
    .app_bar_pad_h         = 44.0f,
    .app_bar_title_size    = 52.0f,

    .prompt_font_size      = 48.0f,     // ~17.5dp: crystal clear mobile body text
    .counter_font_size     = 220.0f,    // ~80dp: bold, striking hero counter display
    .counter_pad_v         = 52.0f,
    .counter_pad_h         = 140.0f,
    .counter_radius        = 56.0f,
    .counter_margin_v      = 44.0f,

    .card_pad              = 48.0f,
    .card_margin_v         = 40.0f,
    .card_title_size       = 44.0f,     // ~16dp: clear section title
    .card_body_size        = 38.0f,     // ~14dp: comfortable, effortless reading
    .card_gap              = 22.0f,
    .card_max_width        = 1180.0f,

    .btn_normal_size       = 185.0f,    // ~68dp: generous, effortless finger touch targets
    .btn_accent_size       = 210.0f,    // ~76dp: prominent action button
    .btn_font_size         = 88.0f,     // ~32dp: large, crystal clear icon symbols
    .btn_gap               = 48.0f,
    .action_row_margin_top = 52.0f,

    .badge_pad_v           = 26.0f,
    .badge_pad_h           = 52.0f,
    .badge_font_size       = 38.0f,     // ~14dp: clear badge text
    .badge_margin_top      = 44.0f,

    .page_pad              = 40.0f,
    .column_gap            = 32.0f,
};

inline const LayoutDimensions& getLayoutDimensions() {
#ifdef __ANDROID__
    return kMobileDimensions;
#else
    return kOriginalDimensions;
#endif
}

// ════════════════════════════════════════════════════════════════
// 4. Counter Page (StatelessWidget — Pure Architecture)
// ════════════════════════════════════════════════════════════════

class CounterPage : public StatelessWidget {
public:
    std::string_view typeName() const override { return "CounterPage"; }

    WidgetPtr build(BuildContext& ctx) override {
        s_page_rebuild_count++;

        std::cout << "\n======================================================================\n";
        std::cout << " >>> [PAGE BUILD TRIGGERED] CounterPage::build() (StatelessWidget) ran! <<<\n";
        std::cout << " Total Entire Page Builds so far: " << s_page_rebuild_count << " (Only on startup!)\n";
        std::cout << "======================================================================\n";

        // Access Cubit via BlocProvider from ancestor context
        auto* cubit = BlocProvider<CounterCubit>::tryOf(ctx);
        const auto& dim = getLayoutDimensions();

#ifdef __ANDROID__
        const std::string reset_label = "0";
        const std::string app_bar_icon = "[*] ";
        const std::string telemetry_icon = "[i] ";
        const std::string badge_icon = "[OK] ";
#else
        const std::string reset_label = "↺";
        const std::string app_bar_icon = "⚡ ";
        const std::string telemetry_icon = "📊 ";
        const std::string badge_icon = "✨ ";
#endif

        // App Bar Header
        auto app_bar = container({
            .color = 0xFF1E293B,
            .border = Border(0x30FFFFFF, 1.0f),
            .width = 100_pct,
            .padding = StyleInsets::only(
                dim.app_bar_pad_v + dim.status_bar_top,
                dim.app_bar_pad_h,
                dim.app_bar_pad_v,
                dim.app_bar_pad_h
            ),
            .child = row({
                .align_items = Align::Center,
                .gap = StyleValue::point(12.0f),
                .children = {
                    text(app_bar_icon + "Demo Home Page (Stateless + Cubit)", {
                        .color = 0xFF38BDF8,
                        .font_size = dim.app_bar_title_size,
                        .font_weight = FontWeight::Bold
                    })
                }
            })
        });

        // 5. BlocBuilder — Subscribes to CounterCubit and rebuilds ONLY this subtree!
        auto counter_display = bloc_builder<CounterCubit>(
            [&dim](BuildContext& b_ctx, const CounterState& state) {
                s_builder_rebuild_count++;

                Element* target = b_ctx.element();
                Element* root = target;
                while (root && root->parent()) {
                    root = root->parent();
                }

                std::cout << "\n======================================================================\n";
                std::cout << " [EVENT] CounterCubit State Updated -> State Value = " << state.value << "\n";
                std::cout << " [BUILD TRACKER] >> ONLY BlocBuilder<CounterCubit> is Rebuilding!\n";
                std::cout << "----------------------------------------------------------------------\n";
                std::cout << " [TREE DUMP & REBUILD ANALYSIS]:\n";
                if (root) {
                    printElementTree(root, 0, target);
                }
                std::cout << "----------------------------------------------------------------------\n";
                std::cout << " [VERIFICATION STATS]:\n";
                std::cout << "  • Whole Page Builds : " << s_page_rebuild_count << " (StatelessWidget — Initial Mount Only!)\n";
                std::cout << "  • BlocBuilder Builds: " << s_builder_rebuild_count << " (Granular updates!)\n";
                std::cout << "  • Conclusion        : CounterPage is StatelessWidget! Zero page rebuilds.\n";
                std::cout << "======================================================================\n\n";

                float border_w = dim.counter_font_size > 80.0f ? 4.0f : 2.0f;
                float glow_r = dim.counter_font_size > 80.0f ? 36.0f : 20.0f;
                float shadow_blur = dim.counter_font_size > 80.0f ? 24.0f : 12.0f;
                float shadow_y = dim.counter_font_size > 80.0f ? 8.0f : 4.0f;

                return container({
                    .color = 0x2538BDF8,
                    .border_radius = BorderRadius::circular(dim.counter_radius),
                    .border = Border(0x8038BDF8, border_w),
                    .box_shadow = {
                        BoxShadow::glow(0x5038BDF8, glow_r),
                        BoxShadow(0x40000000, {0.0f, shadow_y}, shadow_blur)
                    },
                    .padding = StyleInsets::symmetric(dim.counter_pad_v, dim.counter_pad_h),
                    .margin = StyleInsets::symmetric(dim.counter_margin_v, 0.0f),
                    .child = text(std::to_string(state.value), {
                        .color = 0xFF38BDF8,
                        .font_size = dim.counter_font_size,
                        .font_weight = FontWeight::Bold
                    })
                });
            }
        );

        // Telemetry Card showing live build comparison
        float card_radius = dim.counter_font_size > 80.0f ? 24.0f : 12.0f;
        float card_border_w = dim.counter_font_size > 80.0f ? 2.0f : 1.0f;

        auto telemetry_card = container({
            .color = 0x200F172A,
            .border_radius = BorderRadius::circular(card_radius),
            .border = Border(0x30475569, card_border_w),
            .width = 100_pct,
            .max_width = StyleValue::point(dim.card_max_width),
            .padding = StyleInsets::all(dim.card_pad),
            .margin = StyleInsets::symmetric(dim.card_margin_v, 0.0f),
            .child = column({
                .gap = StyleValue::point(dim.card_gap),
                .children = {
                    text(telemetry_icon + "Real-Time Tree Rebuild Telemetry:", {
                        .color = 0xFFFCD34D,
                        .font_size = dim.card_title_size,
                        .font_weight = FontWeight::SemiBold
                    }),
                    text("• CounterPage: StatelessWidget (Builds ONCE on start!)", {
                        .color = 0xFF4ADE80,
                        .font_size = dim.card_body_size,
                        .font_weight = FontWeight::SemiBold
                    }),
                    text("• BlocBuilder: Rebuilds only the counter box on clicks!", {
                        .color = 0xFF38BDF8,
                        .font_size = dim.card_body_size
                    }),
                    text("• Terminal: Shows live Element Tree with highlighted subtree.", {
                        .color = 0xFF94A3B8,
                        .font_size = dim.card_body_size
                    })
                }
            })
        });

        // Bottom Action FABs
        auto action_row = row({
            .justify_content = Justify::Center,
            .align_items = Align::Center,
            .gap = StyleValue::point(dim.btn_gap),
            .margin = StyleInsets::only(dim.action_row_margin_top, 0.0f, 0.0f, 0.0f),
            .children = {
                std::make_shared<ActionButton>("-", 0xFF1E293B, 0xFF334155, 0xFFF87171,
                    dim.btn_normal_size, dim.btn_normal_size, dim.btn_font_size, [cubit]() {
                    if (cubit) cubit->decrement();
                }),
                std::make_shared<ActionButton>(reset_label, 0xFF1E293B, 0xFF334155, 0xFFFCD34D,
                    dim.btn_normal_size, dim.btn_normal_size, dim.btn_font_size, [cubit]() {
                    if (cubit) cubit->reset();
                }),
                std::make_shared<ActionButton>("+", 0xFF2563EB, 0xFF3B82F6, 0xFFFFFFFF,
                    dim.btn_accent_size, dim.btn_accent_size, dim.btn_font_size, [cubit]() {
                    if (cubit) cubit->increment();
                }),
            }
        });

        // Architectural badge explaining pure stateless design
        float badge_radius = dim.counter_font_size > 80.0f ? 36.0f : 20.0f;
        float badge_border_w = dim.counter_font_size > 80.0f ? 2.0f : 1.0f;

        auto stateless_badge = container({
            .color = 0x1510B981,
            .border_radius = BorderRadius::circular(badge_radius),
            .border = Border(0x4010B981, badge_border_w),
            .padding = StyleInsets::symmetric(dim.badge_pad_v, dim.badge_pad_h),
            .margin = StyleInsets::only(dim.badge_margin_top, 0.0f, 0.0f, 0.0f),
            .child = text(badge_icon + "Pure StatelessWidget — Zero setState() in Page", {
                .color = 0xFF6EE7B7,
                .font_size = dim.badge_font_size,
                .font_weight = FontWeight::Medium
            })
        });

        auto body = container({
            .color = 0xFF0B0F19,
            .align = Alignment::Center,
            .width = 100_pct,
            .height = 100_pct,
            .padding = StyleInsets::all(dim.page_pad),
            .child = column({
                .justify_content = Justify::Center,
                .align_items = Align::Center,
                .gap = StyleValue::point(dim.column_gap),
                .children = {
                    text("You have pushed the button this many times:", {
                        .color = 0xFF94A3B8,
                        .font_size = dim.prompt_font_size,
                        .font_weight = FontWeight::Medium
                    }),
                    counter_display,
                    telemetry_card,
                    action_row,
                    stateless_badge
                }
            })
        });

        auto full_layout = column({
            .width = 100_pct,
            .height = 100_pct,
            .children = {
                app_bar,
                container({
                    .align = Alignment::Center,
                    .width = 100_pct,
                    .flex = 1.0f,
                    .child = body
                })
            }
        });

        return full_layout;
    }
};

// ════════════════════════════════════════════════════════════════
// 5. App Root with BlocProvider & WindowFrame
// ════════════════════════════════════════════════════════════════

class CounterApp : public StatelessWidget {
public:
    WidgetPtr build(BuildContext&) override {
        // Inject CounterCubit into the widget tree via BlocProvider
        auto page = bloc_provider<CounterCubit>(
            []() {
                return std::make_shared<CounterCubit>();
            },
            std::make_shared<CounterPage>()
        );

#ifdef __ANDROID__
        return page;
#else
        return windowFrame(WindowFrameProps{
            .content = page,
            .title = "   Demo Home Page (Cubit)",
            .border_radius = 12.0f,
            .border_color = 0x4038BDF8,
            .border_width = 1.5f,
            .background_color = 0xFF0B0F19,
            .titlebar_background_color = 0xFF1E293B,
            .titlebar_inactive_background_color = 0xFF1E293B,
            .titlebar_style = TitleBarStyle::VAXPOS,
        });
#endif
    }
    std::string_view typeName() const override { return "CounterApp"; }
};


// ─────────────────────────────────────────────────────────────────────────────
// Application entry point
// On Android : enki/app/main.hpp renames this to enki_user_main(), which the
//              framework's enki_android_main() calls after NativeWindow setup.
// On Desktop : the OS calls int main() directly — no bridging needed.
// ─────────────────────────────────────────────────────────────────────────────
int main() {
    AppConfig config;
    config.title       = "ENKI Counter";
    config.app_id      = "org.enki.cubit_counter";
    config.width       = 520;
    config.height      = 640;
    config.resizable   = false;
    config.enable_csd  = false;
    config.vsync       = true;
    config.show_performance_overlay = true;
    config.clear_color = 0xFF0B0F19;

    return runApp(std::make_shared<CounterApp>(), config);
}
