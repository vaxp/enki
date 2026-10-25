#include "enki/enki.hpp"
#include "enki/state.hpp"
#include <string>

using namespace enki;

// ════════════════════════════════════════════════════════════════
// 1. Reactive State Definition (C++20 Immutable Value Struct)
// ════════════════════════════════════════════════════════════════
struct CounterState {
    int count = 0;
    bool operator==(const CounterState& other) const = default;
};

// ════════════════════════════════════════════════════════════════
// 2. Business Logic Layer (Cubit)
// ════════════════════════════════════════════════════════════════
class CounterCubit : public Cubit<CounterState> {
public:
    CounterCubit() : Cubit(CounterState{0}) {}

    void increment() { emit(CounterState{state().count + 1}); }
    void decrement() { emit(CounterState{state().count - 1}); }
    void reset()     { emit(CounterState{0}); }
};

// ════════════════════════════════════════════════════════════════
// 3. Declarative UI Component
// ════════════════════════════════════════════════════════════════
class CounterPage : public StatelessWidget {
public:
    WidgetPtr build(BuildContext& context) override {
        // Retrieve the Cubit from context
        auto* cubit = BlocProvider<CounterCubit>::tryOf(context);

        return container({
            .color = 0xFF0B0F19, // (Obsidian Dark)
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .child = centerBox(
                column({
                    .justify_content = Justify::Center,
                    .align_items = Align::Center,
                    .gap = StyleValue::point(28.0f),
                    .children = {
                        // Title
                        container({
                            .color = 0xFF1E293B,
                            .border_radius = BorderRadius::circular(20.0f),
                            .border = Border(0xFF334155, 1.0f),
                            .padding = StyleInsets::symmetric(6.0f, 16.0f),
                            .child = text("ENKI CUBIT REACTIVE", {
                                .color = 0xFF38BDF8,
                                .font_size = 12.0f,
                                .font_weight = FontWeight::Bold,
                                .letter_spacing = 1.2f,
                            })
                        }),

                        text("Interactive Counter App", {
                            .color = 0xFFF8FAFC,
                            .font_size = 26.0f,
                            .font_weight = FontWeight::Bold,
                        }),

                        bloc_builder<CounterCubit>([](BuildContext&, const CounterState& state) -> WidgetPtr {
                            return container({
                                .color = 0xFF161F30,
                                .border_radius = BorderRadius::circular(24.0f),
                                .border = Border(0xFF2E3E59, 1.5f),
                                .box_shadow = {
                                    BoxShadow::standard(0x50000000, 16.0f, 6.0f),
                                    BoxShadow::glow(0x2038BDF8, 24.0f),
                                },
                                .padding = StyleInsets::symmetric(32.0f, 56.0f),
                                .child = column({
                                    .align_items = Align::Center,
                                    .gap = StyleValue::point(8.0f),
                                    .children = {
                                        text(std::to_string(state.count), {
                                            .color = 0xFF38BDF8, 
                                            .font_size = 76.0f,
                                            .font_weight = FontWeight::Bold,
                                        }),
                                        text("The Current Value", {
                                            .color = 0xFF94A3B8,
                                            .font_size = 14.0f,
                                            .font_weight = FontWeight::Medium,
                                        })
                                    }
                                })
                            });
                        }),

                        // Action Buttons Row
                        row({
                            .justify_content = Justify::Center,
                            .gap = StyleValue::point(16.0f),
                            .children = {
                                Button {
                                    .child = text("− Decrement", {
                                        .color = 0xFFF87171,
                                        .font_size = 15.0f,
                                        .font_weight = FontWeight::SemiBold,
                                    }),
                                    .on_pressed = [cubit]() { if (cubit) cubit->decrement(); },
                                    .normal_color = 0xFF1E293B,
                                    .hover_color = 0xFF334155,
                                    .pressed_color = 0xFF0F172A,
                                    .border_radius = 12.0f,
                                    .padding = EdgeInsets::symmetric(14.0f, 22.0f),
                                },

                                Button {
                                    .child = text("↺ Reset", {
                                        .color = 0xFFFBBF24,
                                        .font_size = 15.0f,
                                        .font_weight = FontWeight::SemiBold,
                                    }),
                                    .on_pressed = [cubit]() { if (cubit) cubit->reset(); },
                                    .normal_color = 0xFF1E293B,
                                    .hover_color = 0xFF334155,
                                    .pressed_color = 0xFF0F172A,
                                    .border_radius = 12.0f,
                                    .padding = EdgeInsets::symmetric(14.0f, 22.0f),
                                },

                                Button {
                                    .child = text("+ Increment", {
                                        .color = 0xFFFFFFFF,
                                        .font_size = 15.0f,
                                        .font_weight = FontWeight::Bold,
                                    }),
                                    .on_pressed = [cubit]() { if (cubit) cubit->increment(); },
                                    .normal_color = 0xFF0284C7,
                                    .hover_color = 0xFF0EA5E9,
                                    .pressed_color = 0xFF0369A1,
                                    .border_radius = 12.0f,
                                    .padding = EdgeInsets::symmetric(14.0f, 24.0f),
                                },
                            }
                        })
                    }
                })
            )
        });
    }

    std::string_view typeName() const override { return "CounterPage"; }
};

// ════════════════════════════════════════════════════════════════
// 4. Root Application with Dependency Injection
// ════════════════════════════════════════════════════════════════
class CounterApp : public StatelessWidget {
public:
    WidgetPtr build(BuildContext&) override {
        return bloc_provider<CounterCubit>(
            []() { return std::make_shared<CounterCubit>(); },
            std::make_shared<CounterPage>()
        );
    }

    std::string_view typeName() const override { return "CounterApp"; }
};

// ════════════════════════════════════════════════════════════════
// 5. Application Entry Point
// ════════════════════════════════════════════════════════════════
int main() {
    AppConfig config;
    config.title = "ENKI Cubit Counter Demo";
    config.width = 600;
    config.height = 700;
    config.resizable = true;
    config.vsync = true;
    config.clear_color = 0xFF0B0F19;

    return runApp(std::make_shared<CounterApp>(), config);
}
