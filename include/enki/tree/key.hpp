#pragma once
/// @file key.hpp
/// @brief Key system for widget identity and reconciliation.
///
/// Keys are used during tree reconciliation to determine whether an existing
/// Element can be reused for a new Widget. When two widgets have the same
/// runtime type and equal keys, the reconciler updates the existing element
/// rather than creating a new one. This is critical for preserving state
/// across rebuilds (e.g., scroll positions, text field content, animations).
///
/// Key types:
///   - None:   Default — identity is determined solely by widget type + position.
///   - Value:  Integer-based key for indexed collections (e.g., list items).
///   - String: String-based key for named/semantic identity.
///   - Unique: Auto-generated globally unique key (GlobalKey equivalent).
///
/// @copyright ENKI Framework — MIT License

#include <cstdint>
#include <string>
#include <string_view>
#include <variant>
#include <atomic>
#include <functional>

namespace enki {

/// Widget identity key for the tree reconciliation algorithm.
///
/// A Key uniquely identifies a widget within its sibling list, allowing the
/// framework to efficiently diff and reuse existing elements when the widget
/// tree is rebuilt.
///
/// @note Keys are immutable and value-comparable. Two keys are equal if and
///       only if they have the same type AND the same underlying value.
///
/// @code
///   // Create keys
///   auto k1 = Key::value(42);
///   auto k2 = Key::string("header");
///   auto k3 = Key::unique();      // GlobalKey — guaranteed unique
///   auto k4 = Key::none();        // No key — position-based identity
///
///   // Keys in widgets
///   auto widget = container({
///       .key = Key::value(item.id),
///       .child = text(item.title),
///   });
/// @endcode
class Key {
public:
    /// Key variant type tag.
    enum class Type : uint8_t {
        None,     ///< No key — identity by position only.
        Value,    ///< Integer-keyed identity.
        String,   ///< String-keyed identity.
        Unique,   ///< Globally unique identity (auto-generated).
    };

    // ── Factory Methods ────────────────────────────────────────

    /// Create a "no key" sentinel. Widgets with no key are matched
    /// by type and position in the child list.
    [[nodiscard]] static constexpr Key none() noexcept {
        return Key{};
    }

    /// Create an integer-valued key. Ideal for items in a list
    /// where each item has a stable numeric ID (database row ID, index, etc.).
    /// @param val The integer value that uniquely identifies this widget among siblings.
    [[nodiscard]] static constexpr Key value(int64_t val) noexcept {
        Key k;
        k.type_ = Type::Value;
        k.data_ = val;
        return k;
    }

    /// Create a string-valued key. Ideal for semantically named widgets
    /// such as page sections, form fields, etc.
    /// @param val The string that uniquely identifies this widget among siblings.
    [[nodiscard]] static Key string(std::string_view val) {
        Key k;
        k.type_ = Type::String;
        k.data_ = std::string(val);
        return k;
    }

    /// Create a globally unique key. The key is generated from a monotonically
    /// increasing counter, guaranteeing uniqueness across the entire widget tree.
    /// Analogous to  's `GlobalKey`.
    [[nodiscard]] static Key unique() {
        static std::atomic<uint64_t> counter{1};
        Key k;
        k.type_ = Type::Unique;
        k.data_ = counter.fetch_add(1, std::memory_order_relaxed);
        return k;
    }

    // ── Observers ──────────────────────────────────────────────

    /// @return The type of this key.
    [[nodiscard]] constexpr Type type() const noexcept { return type_; }

    /// @return true if this is a "no key" sentinel.
    [[nodiscard]] constexpr bool isNone() const noexcept {
        return type_ == Type::None;
    }

    /// @return true if this key carries an identity value (non-None).
    [[nodiscard]] constexpr bool hasValue() const noexcept {
        return type_ != Type::None;
    }

    /// @return The integer value (asserts if not a Value key).
    [[nodiscard]] int64_t intValue() const {
        return std::get<int64_t>(data_);
    }

    /// @return The string value (asserts if not a String key).
    [[nodiscard]] const std::string& stringValue() const {
        return std::get<std::string>(data_);
    }

    /// @return The unique ID (asserts if not a Unique key).
    [[nodiscard]] uint64_t uniqueId() const {
        return std::get<uint64_t>(data_);
    }

    // ── Comparison ─────────────────────────────────────────────

    /// Two keys are equal iff they have the same type and the same value.
    /// Two None keys are considered equal (vacuously — both lack identity).
    [[nodiscard]] bool operator==(const Key& other) const noexcept {
        if (type_ != other.type_) return false;
        if (type_ == Type::None) return true;
        return data_ == other.data_;
    }

    [[nodiscard]] bool operator!=(const Key& other) const noexcept {
        return !(*this == other);
    }

    // ── Hashing ────────────────────────────────────────────────

    /// Compute a hash suitable for use in unordered containers.
    [[nodiscard]] std::size_t hash() const noexcept {
        std::size_t seed = static_cast<std::size_t>(type_);
        switch (type_) {
            case Type::None:
                break;
            case Type::Value: {
                auto val = std::get<int64_t>(data_);
                seed ^= std::hash<int64_t>{}(val) + 0x9e3779b9
                         + (seed << 6) + (seed >> 2);
                break;
            }
            case Type::String: {
                const auto& s = std::get<std::string>(data_);
                seed ^= std::hash<std::string>{}(s) + 0x9e3779b9
                         + (seed << 6) + (seed >> 2);
                break;
            }
            case Type::Unique: {
                auto uid = std::get<uint64_t>(data_);
                seed ^= std::hash<uint64_t>{}(uid) + 0x9e3779b9
                         + (seed << 6) + (seed >> 2);
                break;
            }
        }
        return seed;
    }

    // ── Debug ──────────────────────────────────────────────────

    /// @return A human-readable representation of this key.
    [[nodiscard]] std::string toString() const;

private:
    Type type_ = Type::None;
    std::variant<std::monostate, int64_t, std::string, uint64_t> data_;
};

}  // namespace enki

// ── std::hash specialization ───────────────────────────────────
template<>
struct std::hash<enki::Key> {
    std::size_t operator()(const enki::Key& k) const noexcept {
        return k.hash();
    }
};
