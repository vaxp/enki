#pragma once
/// @file state.hpp
/// @brief ENKI Master State Management Header.
///
/// Includes all reactive state management, lifecycle, and dependency injection
/// systems provided by the ENKI framework:
///   - StatefulWidget State & Lifecycle (setState, initState, dispose)
///   - Reactive Streams & Subscriptions (Stream<T>)
///   - Cubit State Management (Cubit<S>, Change<S>)
///   - BLoC Event-Driven Architecture (Bloc<E, S>, BlocEvent, Transition)
///   - Dependency Injection (BlocProvider<B>, MultiBlocProvider)
///   - Reactive Granular Rebuilders (BlocBuilder<B>, bloc_builder)
///   - Side-Effect Listeners (BlocListener<B>, bloc_listener)
///   - Global Telemetry & Observers (BlocObserver)
///
/// Usage:
/// @code
///   #include "enki/state.hpp"
///   // or #include "state.hpp"
/// @endcode
///
/// @copyright ENKI Framework — MIT License

// ── Widget Lifecycle & Base State ───────────────────────────────
#include "enki/state/state.hpp"

// ── Reactive Streams ────────────────────────────────────────────
#include "enki/state/stream.hpp"

// ── Cubit & BLoC Core ───────────────────────────────────────────
#include "enki/state/cubit.hpp"
#include "enki/state/bloc.hpp"

// ── Dependency Injection & Widget Integration ───────────────────
#include "enki/state/bloc_provider.hpp"
#include "enki/state/bloc_builder.hpp"
#include "enki/state/bloc_listener.hpp"
