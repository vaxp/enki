/// @file state.cpp
/// @brief State class implementation — widget state lifecycle management.
///
/// @copyright ENKI Framework — MIT License

#include "enki/state/state.hpp"
#include "enki/tree/element.hpp"
#include "enki/tree/widget.hpp"
#include "enki/tree/build_context.hpp"
#include <cassert>
#include <stdexcept>

namespace enki {

// ════════════════════════════════════════════════════════════════
// Lifecycle Methods
// ════════════════════════════════════════════════════════════════

void State::initState() {
    assert(element_ && "initState() called before element assignment");
    assert(!mounted_ && "initState() called on already-mounted State");
    assert(!disposed_ && "initState() called on disposed State");
    mounted_ = true;
}

void State::didUpdateWidget(const Widget& oldWidget) {
    assert(mounted_ && "didUpdateWidget() called on unmounted State");
    assert(!disposed_ && "didUpdateWidget() called on disposed State");
    // Subclasses override to react to widget configuration changes.
    // Base implementation is intentionally empty.
}

void State::didChangeDependencies() {
    assert(mounted_ && "didChangeDependencies() called on unmounted State");
    assert(!disposed_ && "didChangeDependencies() called on disposed State");
    // Subclasses override to react to inherited data changes.
    // Base implementation is intentionally empty.
}

void State::deactivate() {
    assert(mounted_ && "deactivate() called on unmounted State");
    assert(!disposed_ && "deactivate() called on disposed State");
    // Subclasses override to respond to temporary removal from the tree.
    // Base implementation is intentionally empty.
}

void State::dispose() {
    assert(mounted_ && "dispose() called on unmounted State");
    assert(!disposed_ && "dispose() called twice on same State");
    mounted_ = false;
    disposed_ = true;
    // Subclasses should release resources, cancel subscriptions, etc.
    // IMPORTANT: Always call State::dispose() LAST in overrides.
}

// ════════════════════════════════════════════════════════════════
// State Update
// ════════════════════════════════════════════════════════════════

void State::setState(std::function<void()> fn) {
    if (disposed_) {
        // Calling setState after dispose is a programming error.
        // In release builds, silently ignore. In debug builds, assert.
        assert(false && "setState() called after dispose(). "
                       "This typically happens when an async callback fires "
                       "after the widget has been removed from the tree.");
        return;
    }

    if (!mounted_) {
        // setState before mount — this can happen if createState() tries
        // to call setState in the constructor. The state will be built
        // during mount anyway, so we can safely ignore.
        return;
    }

    // Execute the state modification function
    if (fn) {
        fn();
    }

    // Mark the element as needing a rebuild
    if (element_) {
        element_->markNeedsBuild();
    }
}

// ════════════════════════════════════════════════════════════════
// Accessors
// ════════════════════════════════════════════════════════════════

BuildContext& State::context() {
    assert(element_ && "context() called before mount or after dispose");
    assert(mounted_ && "context() called on unmounted State");
    // BuildContext is backed by the element. We create a temporary
    // BuildContext wrapper on the stack.
    // Note: In production, this would be cached or the element would
    // directly serve as the BuildContext.
    static thread_local std::unique_ptr<BuildContext> cached_ctx;
    cached_ctx = std::make_unique<BuildContext>(element_);
    return *cached_ctx;
}

const BuildContext& State::context() const {
    assert(element_ && "context() called before mount or after dispose");
    assert(mounted_ && "context() called on unmounted State");
    static thread_local std::unique_ptr<BuildContext> cached_ctx;
    cached_ctx = std::make_unique<BuildContext>(element_);
    return *cached_ctx;
}

const StatefulWidget* State::widget() const {
    if (!element_) return nullptr;
    return dynamic_cast<const StatefulWidget*>(element_->widget().get());
}

// ════════════════════════════════════════════════════════════════
// Internal — called by StatefulElement
// ════════════════════════════════════════════════════════════════

void State::setElement(StatefulElement* element) {
    assert(!element_ || element == nullptr);  // Can only set once, or clear
    element_ = element;
}

}  // namespace enki
