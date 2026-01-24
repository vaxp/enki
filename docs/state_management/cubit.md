# Cubit State Management

> A complete engineering guide to reactive state management in ENKI using `Cubit<S>`, `BlocProvider<B>`, and `BlocBuilder<B>`. Covers granular subtree rebuilding, dependency injection via `BuildContext`, and real-world C++20 implementation code from `real_app/counter`.

- **Header Files**:
  - `#include "enki/state/cubit.hpp"` — `Cubit<S>`, `Change<S>`, `BlocObserver`.
  - `#include "enki/state/bloc_provider.hpp"` — `BlocProvider<B>`, `bloc_provider<B>()`.
  - `#include "enki/state/bloc_builder.hpp"` — `BlocBuilderWidget<B>`, `bloc_builder<B>()`.
  - `#include "enki/state/bloc_listener.hpp"` — `BlocListenerWidget<B>`, `bloc_listener<B>()`.
- **Primary Classes**:
  - `enki::Cubit<S>` — Reactive state container emitting immutable state changes.
  - `enki::BlocProvider<B>` — Dependency injection mechanism via `ProxyWidget` / `BuildContext`.
  - `enki::BlocBuilderWidget<B>` — Granular reactive widget subscribing to state changes.
  - `enki::BlocListenerWidget<B>` — Side-effect executor (dialogs, navigation, audio).
  - `enki::BlocObserver` — Global application-wide state transition and error observer.
- **Primary Factory Helpers**:
  - `enki::bloc_provider<B>(createFn, child)`
  - `enki::bloc_builder<B>(builderFn, buildWhenFn = nullptr)`
  - `enki::bloc_listener<B>(listenerFn, child, listenWhenFn = nullptr)`

---

## 1. Overview & Architectural Goals

In desktop UI frameworks built on C++, UI rebuild efficiency directly dictates CPU and GPU rasterization load. When state changes, rebuilding whole widget trees leads to unnecessary layout passes and wasted GPU cycles.

The **ENKI Cubit system** introduces a reactive, stream-based state management architecture inspired by BLoC, specifically optimized for C++20:

1. **Separation of Concerns**: Business logic and data manipulation live exclusively inside `Cubit<S>`. The UI layer remains strictly declarative and stateless.
2. **Granular Reactivity**: State updates rebuild **only** the `BlocBuilder` subtree. The enclosing page remains a `StatelessWidget` and is never rebuilt after initial mounting.
3. **Deterministic Memory Management**: Subscriptions are lifecycle-bound to the Element tree. When a widget unmounts, its stream subscription is cancelled automatically with zero leaks.
4. **Testability & Observability**: Cubits can be instantiated and tested in headless environments without initializing display servers, OpenGL, or Skia.

```
┌────────────────────────────────────────────────────────────────────────┐
│                          Widget Tree Hierarchy                         │
│                                                                        │
│  [CounterApp: StatelessWidget]                                         │
│         │                                                              │
│  [BlocProvider<CounterCubit>] (Injects Cubit into BuildContext)        │
│         │                                                              │
│  [CounterPage: StatelessWidget] (BUILT ONCE ON LAUNCH)                 │
│         │                                                              │
│         ├── [AppBar] (Static — never rebuilt)                          │
│         ├── [TelemetryCard] (Static — never rebuilt)                   │
│         ├── [ActionButtons Row] (Static — never rebuilt)               │
│         │                                                              │
│         └── [BlocBuilder<CounterCubit>]                                │
│                    │                                                   │
│                    ▼                                                   │
│            [Text(state.value)]                                         │
│         ⚡ ONLY THIS SUBTREE IS REBUILT ON STATE CHANGE! ⚡            │
└────────────────────────────────────────────────────────────────────────┘
```

---

## 2. Core Components

### A. Immutable State (`struct S`)

States in ENKI are immutable value structs representing data snapshots at a given point in time. States should implement C++20 default equality comparison (`operator== = default`) so the engine can detect actual state changes:

```cpp
struct CounterState {
    int value = 0;
    bool operator==(const CounterState& other) const = default;
};
```

### B. `Cubit<S>`

`Cubit<S>` inherits from `Stream<S>` and manages a single type of state `S`. It exposes public methods that encapsulate state transitions and invokes the protected method `emit(newState)`:

```cpp
class CounterCubit : public enki::Cubit<CounterState> {
public:
    CounterCubit() : Cubit(CounterState{0}) {}

    void increment() { emit(CounterState{state().value + 1}); }
    void decrement() { emit(CounterState{state().value - 1}); }
    void reset()     { emit(CounterState{0}); }

protected:
    void onChange(const enki::Change<CounterState>& change) override {
        // Optional hook: called on every state transition
    }
};
```

- `state()` returns `const S&` of the current state.
- `emit(newState)` checks if `newState == currentState`. If distinct, it updates `state_` and notifies all active stream subscribers.

### C. `BlocProvider<B>`

`BlocProvider<B>` inherits from `ProxyWidget`. It injects a `Cubit` or `BLoC` instance into the widget tree so descendant widgets can access it without manual prop drilling.

- **Lazy Construction**: In factory mode (`CreateFn`), the Cubit is instantiated only when first queried by a descendant.
- **Context Lookup**:
  - `BlocProvider<B>::of(ctx)`: Returns `B&`. Asserts if not found in the ancestor tree.
  - `BlocProvider<B>::tryOf(ctx)`: Returns `B*`. Safely returns `nullptr` if not found.

```cpp
auto root = enki::bloc_provider<CounterCubit>(
    []() { return std::make_shared<CounterCubit>(); },
    std::make_shared<CounterPage>()
);
```

### D. `BlocBuilder<B>`

`BlocBuilderWidget<B>` is an internal `StatefulWidget` that binds a widget subtree to a Cubit's state stream:

1. **Mount (`initState`)**: Locates `BlocProvider<B>` from `BuildContext` and subscribes via `bloc->listen(...)`.
2. **State Change**: When `emit()` fires, `BlocBuilderState` calls `setState()`, triggering `builder(ctx, state)` **only for that subtree**.
3. **Unmount (`dispose`)**: Automatically cancels the subscription via `SubscriptionId`.

```cpp
auto counter_widget = enki::bloc_builder<CounterCubit>(
    [](BuildContext& ctx, const CounterState& state) {
        return text(std::to_string(state.value), {
            .color = 0xFF38BDF8,
            .font_size = 56.0f,
            .font_weight = FontWeight::Bold
        });
    }
);
```

### E. `BlocListener<B>`

`BlocListenerWidget<B>` listens to state changes to execute **side effects** rather than rebuilding widgets:
- Showing popups, dialogs, or snackbars.
- Routing and navigation (`Navigator::push`).
- Triggering audio effects or background tasks.

```cpp
auto listener_widget = enki::bloc_listener<CounterCubit>(
    [](BuildContext& ctx, const CounterState& state) {
        if (state.value == 100) {
            std::cout << "Target milestone reached: 100!\n";
        }
    },
    child_widget
);
```

---

## 3. Architecture Comparison: `setState()` vs `BlocBuilder`

| Architectural Criterion | `setState()` on Page (`StatefulWidget`) | `BlocBuilder` + `Cubit` (`StatelessWidget`) |
| :--- | :--- | :--- |
| **Page Widget Type** | Must be `StatefulWidget` with dedicated `State` | Pure `StatelessWidget` |
| **Rebuild Scope** | **Entire Page**: AppBar, rows, cards, buttons | **Granular**: Only the specific `BlocBuilder` subtree |
| **Element Tree Passes** | Full tree reconciliation from page root down | Targeted dirty marking of single `BlocBuilderElement` |
| **Logic Coupling** | Business logic mixed with UI rendering code | Clean separation: logic in `Cubit`, layout in `Widget` |
| **Rebuild Count on Click** | **1 Whole Page Rebuild** per user action | **0 Page Rebuilds**, **1 Subtree Rebuild** |
| **Scalability** | Becomes slow as UI complexity increases | Remains instantaneous regardless of page size |

---

## 4. Real-World Counter Implementation (`real_app/counter`)

Below is the complete, tested, and verified code from `real_app/counter/src/main.cpp`.

### Step 1: Define Immutable State and Cubit

```cpp
#include "enki/state/cubit.hpp"
#include <iostream>

using namespace enki;

// 1. Immutable State Struct
struct CounterState {
    int value = 0;
    bool operator==(const CounterState& other) const = default;
};

// 2. Business Logic Component (Cubit)
class CounterCubit : public Cubit<CounterState> {
public:
    CounterCubit() : Cubit(CounterState{0}) {}

    void increment() { emit(CounterState{state().value + 1}); }
    void decrement() { emit(CounterState{state().value - 1}); }
    void reset()     { emit(CounterState{0}); }
};
```

### Step 2: Inject Cubit via `BlocProvider` in App Root

```cpp
#include "enki/state/bloc_provider.hpp"
#include "enki/widgets/window_frame.hpp"

class CounterApp : public StatelessWidget {
public:
    WidgetPtr build(BuildContext&) override {
        // Inject CounterCubit into the widget tree
        auto page = bloc_provider<CounterCubit>(
            []() { return std::make_shared<CounterCubit>(); },
            std::make_shared<CounterPage>()
        );

        return windowFrame(WindowFrameProps{
            .content = page,
            .title = "ENKI Cubit Showcase",
            .background_color = 0xFF0B0F19,
        });
    }
    std::string_view typeName() const override { return "CounterApp"; }
};
```

### Step 3: Build the Page as a Pure `StatelessWidget`

The page is a `StatelessWidget`. It builds the layout once and retrieves the cubit via `BlocProvider<CounterCubit>::tryOf(ctx)`:

```cpp
class CounterPage : public StatelessWidget {
public:
    std::string_view typeName() const override { return "CounterPage"; }

    WidgetPtr build(BuildContext& ctx) override {
        // Retrieve ancestor Cubit pointer safely
        auto* cubit = BlocProvider<CounterCubit>::tryOf(ctx);

        // Header bar (static)
        auto app_bar = container({
            .color = 0xFF1E293B,
            .width = 100_pct,
            .padding = StyleInsets::symmetric(14.0f, 20.0f),
            .child = text("⚡ Cubit Counter (Stateless + Reactive)", {
                .color = 0xFF38BDF8,
                .font_size = 17.0f,
                .font_weight = FontWeight::Bold
            })
        });

        // Granular Reactive Counter Display
        auto counter_display = bloc_builder<CounterCubit>(
            [](BuildContext& b_ctx, const CounterState& state) {
                // ONLY this lambda executes when cubit emits a new state!
                return container({
                    .color = 0x2538BDF8,
                    .border_radius = BorderRadius::circular(20.0f),
                    .border = Border(0x8038BDF8, 2.0f),
                    .padding = StyleInsets::symmetric(14.0f, 48.0f),
                    .child = text(std::to_string(state.value), {
                        .color = 0xFF38BDF8,
                        .font_size = 56.0f,
                        .font_weight = FontWeight::Bold
                    })
                });
            }
        );

        // Action Buttons dispatching Cubit methods
        auto action_row = row({
            .justify_content = Justify::Center,
            .gap = StyleValue::point(16.0f),
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

        return column({
            .width = 100_pct,
            .height = 100_pct,
            .children = {
                app_bar,
                container({
                    .width = 100_pct,
                    .flex = 1.0f,
                    .child = column({
                        .justify_content = Justify::Center,
                        .align_items = Align::Center,
                        .gap = StyleValue::point(16.0f),
                        .children = {
                            text("Pushed the button this many times:", {.color = 0xFF94A3B8, .font_size = 16.0f}),
                            counter_display,
                            action_row
                        }
                    })
                })
            }
        });
    }
};
```

### Step 4: Compilable Standalone Example

```cpp
#include "enki/app/app.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/state/cubit.hpp"
#include "enki/state/bloc_provider.hpp"
#include "enki/state/bloc_builder.hpp"

using namespace enki;

struct CounterState {
    int value = 0;
    bool operator==(const CounterState&) const = default;
};

class CounterCubit : public Cubit<CounterState> {
public:
    CounterCubit() : Cubit(CounterState{0}) {}
    void increment() { emit(CounterState{state().value + 1}); }
    void decrement() { emit(CounterState{state().value - 1}); }
    void reset()     { emit(CounterState{0}); }
};

class SimpleCounterPage : public StatelessWidget {
public:
    std::string_view typeName() const override { return "SimpleCounterPage"; }

    WidgetPtr build(BuildContext& ctx) override {
        auto* cubit = BlocProvider<CounterCubit>::tryOf(ctx);

        return container({
            .color = 0xFF0F172A,
            .width = 100_pct,
            .height = 100_pct,
            .child = column({
                .justify_content = Justify::Center,
                .align_items = Align::Center,
                .gap = StyleValue::point(20.0f),
                .children = {
                    text("Current Counter Value:", {.color = 0xFF94A3B8, .font_size = 18.0f}),
                    bloc_builder<CounterCubit>([](BuildContext&, const CounterState& s) {
                        return text(std::to_string(s.value), {
                            .color = 0xFF38BDF8,
                            .font_size = 64.0f,
                            .font_weight = FontWeight::Bold
                        });
                    }),
                    row({
                        .gap = StyleValue::point(16.0f),
                        .children = {
                            gestureDetector({
                                .child = container({
                                    .color = 0xFF2563EB,
                                    .padding = StyleInsets::symmetric(12.0f, 28.0f),
                                    .border_radius = BorderRadius::circular(8.0f),
                                    .child = text("+ Increment", {.color = 0xFFFFFFFF, .font_size = 16.0f})
                                }),
                                .on_tap = [cubit]() { if (cubit) cubit->increment(); }
                            })
                        }
                    })
                }
            })
        });
    }
};

int main() {
    auto app = bloc_provider<CounterCubit>(
        []() { return std::make_shared<CounterCubit>(); },
        std::make_shared<SimpleCounterPage>()
    );

    return runApp(app, AppConfig{
        .title = "ENKI Cubit Example",
        .width = 450,
        .height = 550,
    });
}
```

---

## 5. Advanced Features & Lifecycle

### A. Selective Rebuilding with `buildWhen`

To prevent unnecessary UI updates when only certain fields of a complex state change, pass an optional `buildWhen` predicate:

```cpp
auto even_only_counter = bloc_builder<CounterCubit>(
    // Builder function
    [](BuildContext& ctx, const CounterState& state) {
        return text("Even count: " + std::to_string(state.value));
    },
    // buildWhen predicate: rebuilds ONLY when the new value is even
    [](const CounterState& previous, const CounterState& current) {
        return (current.value % 2 == 0) && (previous.value != current.value);
    }
);
```

### B. Element Lifecycle & Safe Disconnection

1. In `BlocBuilderState<B>::initState()`, the widget queries `BlocProvider<B>::tryOf(context())`.
2. It calls `bloc->listen(...)`, receiving a unique `SubscriptionId`.
3. In `BlocBuilderState<B>::dispose()`, it cancels the subscription:
   ```cpp
   if (bloc_ && subscription_id_ != InvalidSubscriptionId) {
       bloc_->cancel(subscription_id_);
       subscription_id_ = InvalidSubscriptionId;
   }
   ```
4. This ensures that when a window or popup closes, no dangling callbacks or leaks can occur.

### C. Application-Wide Monitoring via `BlocObserver`

Register a custom `BlocObserver` in `int main()` to centrally monitor all Cubit state changes across the entire app for telemetry, crash diagnostics, and analytics:

```cpp
class AppBlocObserver : public enki::BlocObserver {
public:
    void onCreate(std::string_view name) override {
        std::cout << "[OBSERVER] Created: " << name << "\n";
    }

    void onChange(std::string_view name,
                  const std::string& current,
                  const std::string& next) override {
        std::cout << "[OBSERVER] " << name << " transition: " << current << " -> " << next << "\n";
    }

    void onError(std::string_view name, const std::string& error) override {
        std::cerr << "[OBSERVER ERROR] " << name << ": " << error << "\n";
    }
};

int main() {
    enki::BlocObserver::setGlobal(std::make_shared<AppBlocObserver>());
    // ... runApp(...)
}
```

---

## 6. Cubit vs BLoC: When to Use Which?

| Feature | `Cubit<S>` | `Bloc<E, S>` |
| :--- | :--- | :--- |
| **Trigger Mechanism** | Direct function calls (`cubit.increment()`) | Event objects (`bloc.add(std::make_unique<IncrementEvent>())`) |
| **Boilerplate** | Very minimal (ideal for 90% of UI cases) | Higher (requires event structs and handlers) |
| **Traceability** | Direct call stack | Event log + Transition records (`oldState + event -> newState`) |
| **Best Used For** | Counters, toggles, form validation, theme switching, tab navigation | Multi-step wizards, authentication flows, real-time networking, undo/redo buffers |

---

## 7. Best Practices & Rules

1. **Pages Should Be `StatelessWidget`**:
   Never make a top-level page `StatefulWidget` just to manage shared business data. Keep the page stateless and inject a `Cubit` with `BlocProvider`.
2. **Minimize `BlocBuilder` Scope**:
   Place `BlocBuilder` as close to the leaf nodes of the widget tree as possible. Do not wrap an entire screen inside a single `BlocBuilder`.
3. **Keep `build()` Functions Pure**:
   Never call `cubit->increment()` or any state-modifying function directly inside `build()`. Actions must be dispatched in response to user events (e.g. `on_tap`, `on_hover`).
4. **Prefer Immutable State**:
   Always treat state structs as immutable snapshots. Emit completely new structs rather than mutating existing state members in place.
5. **Use `buildWhen` on High-Frequency Streams**:
   When listening to rapid streams (such as audio spectrums, sensor data, or mouse coordinates), use `buildWhen` to throttle or filter rebuilds.
