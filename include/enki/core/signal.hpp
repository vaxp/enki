#pragma once
/// @file signal.hpp
/// @brief Type-safe Signal/Slot system for decoupled event handling.
/// modern C++20 (no MOC required).

#include <functional>
#include <unordered_map>
#include <cstdint>
#include <mutex>
#include <vector>
#include <algorithm>

namespace enki {

/// A unique identifier for a slot connection.
using SlotId = uint64_t;

/// Invalid slot id sentinel.
inline constexpr SlotId InvalidSlotId = 0;

// ============================================================
// Signal<Args...>
// ============================================================
/// A signal that can be connected to multiple slots (callbacks).
/// When emitted, all connected slots are called with the provided arguments.
///
/// Thread safety: connections and emissions are mutex-protected.
///
/// Usage:
///   Signal<int, int> onResize;
///   auto id = onResize.connect([](int w, int h) { ... });
///   onResize.emit(800, 600);
///   onResize.disconnect(id);
template<typename... Args>
class Signal {
public:
    using SlotFn = std::function<void(Args...)>;

    Signal() = default;
    ~Signal() = default;

    // Non-copyable (connections are identity-bound)
    Signal(const Signal&) = delete;
    Signal& operator=(const Signal&) = delete;

    // Movable
    Signal(Signal&& other) noexcept {
        std::lock_guard lock(other.mutex_);
        slots_ = std::move(other.slots_);
        next_id_ = other.next_id_;
    }

    Signal& operator=(Signal&& other) noexcept {
        if (this != &other) {
            std::lock_guard lock1(mutex_);
            std::lock_guard lock2(other.mutex_);
            slots_ = std::move(other.slots_);
            next_id_ = other.next_id_;
        }
        return *this;
    }

    /// Connect a slot to this signal.
    /// @return A SlotId that can be used to disconnect the slot.
    SlotId connect(SlotFn slot) {
        std::lock_guard lock(mutex_);
        SlotId id = ++next_id_;
        slots_[id] = std::move(slot);
        return id;
    }

    /// Connect a member function as a slot.
    /// @param instance Pointer to the object instance.
    /// @param member_fn Pointer to the member function.
    /// @return A SlotId that can be used to disconnect.
    template<typename T>
    SlotId connect(T* instance, void (T::*member_fn)(Args...)) {
        return connect([instance, member_fn](Args... args) {
            (instance->*member_fn)(std::forward<Args>(args)...);
        });
    }

    /// Disconnect a specific slot by ID.
    void disconnect(SlotId id) {
        std::lock_guard lock(mutex_);
        slots_.erase(id);
    }

    /// Disconnect all connected slots.
    void disconnectAll() {
        std::lock_guard lock(mutex_);
        slots_.clear();
    }

    /// Emit the signal, calling all connected slots.
    void emit(Args... args) {
        // Copy the slots map to avoid issues if a slot modifies connections
        std::unordered_map<SlotId, SlotFn> slots_copy;
        {
            std::lock_guard lock(mutex_);
            slots_copy = slots_;
        }
        for (auto& [id, slot] : slots_copy) {
            slot(std::forward<Args>(args)...);
        }
    }

    /// @return Number of connected slots.
    [[nodiscard]] size_t connectionCount() const {
        std::lock_guard lock(mutex_);
        return slots_.size();
    }

    /// @return true if there are no connected slots.
    [[nodiscard]] bool empty() const {
        std::lock_guard lock(mutex_);
        return slots_.empty();
    }

private:
    mutable std::mutex mutex_;
    std::unordered_map<SlotId, SlotFn> slots_;
    SlotId next_id_ = 0;
};

// ============================================================
// ScopedConnection
// ============================================================
/// RAII wrapper for a signal connection.
/// Automatically disconnects when destroyed.
template<typename... Args>
class ScopedConnection {
public:
    ScopedConnection() = default;

    ScopedConnection(Signal<Args...>& signal, SlotId id)
        : signal_(&signal), id_(id) {}

    ~ScopedConnection() {
        disconnect();
    }

    // Non-copyable
    ScopedConnection(const ScopedConnection&) = delete;
    ScopedConnection& operator=(const ScopedConnection&) = delete;

    // Movable
    ScopedConnection(ScopedConnection&& other) noexcept
        : signal_(other.signal_), id_(other.id_) {
        other.signal_ = nullptr;
        other.id_ = InvalidSlotId;
    }

    ScopedConnection& operator=(ScopedConnection&& other) noexcept {
        if (this != &other) {
            disconnect();
            signal_ = other.signal_;
            id_ = other.id_;
            other.signal_ = nullptr;
            other.id_ = InvalidSlotId;
        }
        return *this;
    }

    void disconnect() {
        if (signal_ && id_ != InvalidSlotId) {
            signal_->disconnect(id_);
            signal_ = nullptr;
            id_ = InvalidSlotId;
        }
    }

    [[nodiscard]] SlotId id() const { return id_; }
    [[nodiscard]] bool connected() const { return signal_ != nullptr && id_ != InvalidSlotId; }

private:
    Signal<Args...>* signal_ = nullptr;
    SlotId id_ = InvalidSlotId;
};

/// Helper to create a ScopedConnection.
template<typename... Args>
ScopedConnection<Args...> scopedConnect(Signal<Args...>& signal,
                                         typename Signal<Args...>::SlotFn slot) {
    SlotId id = signal.connect(std::move(slot));
    return ScopedConnection<Args...>(signal, id);
}

}  // namespace enki
