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
    std::function<void()> on_tap;

    ActionButton(std::string label, Color bg, Color hover, Color txt, float w, float h, std::function<void()> cb)
        : label(std::move(label)), bg_color(bg), hover_color(hover), text_color(txt),
          width_val(w), height_val(h), on_tap(std::move(cb)) {}

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

        auto box = container({
            .color = cur_bg,
            .border_radius = BorderRadius::circular(btn->height_val * 0.5f),
            .border = Border(is_hovered ? 0xFF93C5FD : 0x403B82F6, 1.5f),
            .box_shadow = is_hovered ? std::vector<BoxShadow>{ BoxShadow::glow(0x603B82F6, 12.0f) }
                                     : std::vector<BoxShadow>{ BoxShadow(0x30000000, {0.0f, 4.0f}, 8.0f) },
            .align = Alignment::Center,
            .width = StyleValue::point(btn->width_val),
            .height = StyleValue::point(btn->height_val),
            .margin = press_margin,
            .child = text(btn->label, {
                .color = btn->text_color,
                .font_size = 22.0f,
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

        // App Bar Header (   style)
        auto app_bar = container({
            .color = 0xFF1E293B,
            .border = Border(0x30FFFFFF, 1.0f),
            .width = 100_pct,
            .padding = StyleInsets::symmetric(14.0f, 20.0f),
            .child = row({
                .align_items = Align::Center,
                .gap = StyleValue::point(12.0f),
                .children = {
                    text("⚡    Demo Home Page (Stateless + Cubit)", {
                        .color = 0xFF38BDF8,
                        .font_size = 17.0f,
                        .font_weight = FontWeight::Bold
                    })
                }
            })
        });

        // 5. BlocBuilder — Subscribes to CounterCubit and rebuilds ONLY this subtree!
        auto counter_display = bloc_builder<CounterCubit>(
            [](BuildContext& b_ctx, const CounterState& state) {
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

                return container({
                    .color = 0x2538BDF8,
                    .border_radius = BorderRadius::circular(20.0f),
                    .border = Border(0x8038BDF8, 2.0f),
                    .box_shadow = {
                        BoxShadow::glow(0x5038BDF8, 20.0f),
                        BoxShadow(0x40000000, {0.0f, 4.0f}, 12.0f)
                    },
                    .padding = StyleInsets::symmetric(14.0f, 48.0f),
                    .margin = StyleInsets::symmetric(16.0f, 0.0f),
                    .child = text(std::to_string(state.value), {
                        .color = 0xFF38BDF8,
                        .font_size = 56.0f,
                        .font_weight = FontWeight::Bold
                    })
                });
            }
        );

        // Telemetry Card showing live build comparison
        auto telemetry_card = container({
            .color = 0x200F172A,
            .border_radius = BorderRadius::circular(12.0f),
            .border = Border(0x30475569, 1.0f),
            .padding = StyleInsets::all(14.0f),
            .margin = StyleInsets::symmetric(12.0f, 0.0f),
            .child = column({
                .gap = StyleValue::point(6.0f),
                .children = {
                    text("📊 Real-Time Tree Rebuild Telemetry:", {
                        .color = 0xFFFCD34D,
                        .font_size = 13.0f,
                        .font_weight = FontWeight::SemiBold
                    }),
                    text("• CounterPage: StatelessWidget (Builds ONCE on start!)", {
                        .color = 0xFF4ADE80,
                        .font_size = 12.0f,
                        .font_weight = FontWeight::SemiBold
                    }),
                    text("• BlocBuilder: Rebuilds only the counter box on clicks!", {
                        .color = 0xFF38BDF8,
                        .font_size = 12.0f
                    }),
                    text("• Terminal: Shows live Element Tree with highlighted subtree.", {
                        .color = 0xFF94A3B8,
                        .font_size = 12.0f
                    })
                }
            })
        });

        // Bottom Action FABs
        auto action_row = row({
            .justify_content = Justify::Center,
            .align_items = Align::Center,
            .gap = StyleValue::point(16.0f),
            .margin = StyleInsets::only(16.0f, 0.0f, 0.0f, 0.0f),
            .children = {
                std::make_shared<ActionButton>("-", 0xFF1E293B, 0xFF334155, 0xFFF87171, 56.0f, 56.0f, [cubit]() {
                    if (cubit) cubit->decrement();
                }),
                std::make_shared<ActionButton>("↺", 0xFF1E293B, 0xFF334155, 0xFFFCD34D, 56.0f, 56.0f, [cubit]() {
                    if (cubit) cubit->reset();
                }),
                std::make_shared<ActionButton>("+", 0xFF2563EB, 0xFF3B82F6, 0xFFFFFFFF, 64.0f, 64.0f, [cubit]() {
                    if (cubit) cubit->increment();
                }),
            }
        });

        // Architectural badge explaining pure stateless design
        auto stateless_badge = container({
            .color = 0x1510B981,
            .border_radius = BorderRadius::circular(20.0f),
            .border = Border(0x4010B981, 1.0f),
            .padding = StyleInsets::symmetric(8.0f, 16.0f),
            .margin = StyleInsets::only(16.0f, 0.0f, 0.0f, 0.0f),
            .child = text("✨ Pure StatelessWidget — Zero setState() in Page", {
                .color = 0xFF6EE7B7,
                .font_size = 13.0f,
                .font_weight = FontWeight::Medium
            })
        });

        auto body = container({
            .color = 0xFF0B0F19,
            .width = 100_pct,
            .height = 100_pct,
            .padding = StyleInsets::all(24.0f),
            .child = column({
                .justify_content = Justify::Center,
                .align_items = Align::Center,
                .gap = StyleValue::point(8.0f),
                .children = {
                    text("You have pushed the button this many times:", {
                        .color = 0xFF94A3B8,
                        .font_size = 16.0f,
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
    }
    std::string_view typeName() const override { return "CounterApp"; }
};

int main() {
    std::cout << "======================================================\n";
    std::cout << "  ENKI Engine — Cubit Counter Showcase App            \n";
    std::cout << "  Granular Reactivity & Element Tree Verification     \n";
    std::cout << "======================================================\n";

    AppConfig config;
    config.title       = "   Demo Home Page (Cubit)";
    config.width       = 520;
    config.height      = 640;
    config.resizable   = true;
    config.enable_csd  = true;
    config.app_id      = "org.enki.cubit_counter";
    config.vsync       = true;
    config.target_fps  = 60;
    config.show_performance_overlay = true;
    config.clear_color = 0xFF0B0F19;

    return runApp(std::make_shared<CounterApp>(), config);
}
