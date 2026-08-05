/// @file widget.cpp
/// @brief Widget base class implementations.

#include "enki/tree/widget.hpp"
#include "enki/tree/element.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/tree/build_context.hpp"
#include <sstream>
#include <typeinfo>

// Need State stub for unique_ptr destruction in StatefulWidget
namespace enki {
#ifndef ENKI_STATE_DEFINED
class State {
public:
    virtual ~State() = default;
};
#endif
}

namespace enki {

// ════════════════════════════════════════════════════════════════
// Widget
// ════════════════════════════════════════════════════════════════

bool Widget::canUpdate(const Widget& oldWidget) const {
    // Two widgets can update the same element iff they have the
    // same runtime type AND the same key.
    return typeName() == oldWidget.typeName() && key == oldWidget.key;
}

std::string Widget::debugDescription() const {
    std::ostringstream oss;
    oss << typeName();
    if (key.hasValue()) {
        oss << " " << key.toString();
    }
    return oss.str();
}

// ════════════════════════════════════════════════════════════════
// StatelessWidget
// ════════════════════════════════════════════════════════════════

std::unique_ptr<Element> StatelessWidget::createElement() {
    return std::make_unique<StatelessElement>(shared_from_this());
}

// ════════════════════════════════════════════════════════════════
// StatefulWidget
// ════════════════════════════════════════════════════════════════

std::unique_ptr<Element> StatefulWidget::createElement() {
    return std::make_unique<StatefulElement>(shared_from_this());
}

// ════════════════════════════════════════════════════════════════
// RenderObjectWidget
// ════════════════════════════════════════════════════════════════

std::unique_ptr<Element> RenderObjectWidget::createElement() {
    // RenderObjectWidget by itself creates a basic RenderObjectElement.
    // But we don't have a concrete one here—subclasses
    // (SingleChild/MultiChild) provide their own elements.
    // This base implementation shouldn't normally be called directly.
    assert(false && "RenderObjectWidget::createElement() should not be called directly. "
                    "Use SingleChildRenderObjectWidget or MultiChildRenderObjectWidget.");
    return nullptr;
}

// ════════════════════════════════════════════════════════════════
// SingleChildRenderObjectWidget
// ════════════════════════════════════════════════════════════════

std::unique_ptr<Element> SingleChildRenderObjectWidget::createElement() {
    return std::make_unique<SingleChildRenderObjectElement>(shared_from_this());
}

// ════════════════════════════════════════════════════════════════
// MultiChildRenderObjectWidget
// ════════════════════════════════════════════════════════════════

std::unique_ptr<Element> MultiChildRenderObjectWidget::createElement() {
    return std::make_unique<MultiChildRenderObjectElement>(shared_from_this());
}

// ════════════════════════════════════════════════════════════════
// ProxyWidget
// ════════════════════════════════════════════════════════════════

std::unique_ptr<Element> ProxyWidget::createElement() {
    return std::make_unique<ProxyElement>(shared_from_this());
}

}  // namespace enki
