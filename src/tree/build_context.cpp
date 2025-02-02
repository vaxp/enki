/// @file build_context.cpp
/// @brief BuildContext implementation.

#include "enki/tree/build_context.hpp"
#include "enki/tree/element.hpp"
#include "enki/tree/widget.hpp"
#include "enki/tree/render_object.hpp"
// #include "enki/app/theme.hpp" // For media queries, if handled there

// Stub implementations of dependencies for Phase 4 to compile without Phase 5/8
namespace enki {

class Theme {
public:
    static Theme dark() { return Theme{}; }
};

BuildContext::BuildContext(Element* element) : element_(element) {
    assert(element_ && "BuildContext must be created with a valid Element");
}

template<typename T>
T* BuildContext::findAncestorWidgetOfType() const {
    return element_->findAncestorWidgetOfType<T>();
}

template<typename T>
T* BuildContext::findAncestorElementOfType() const {
    return element_->findAncestorElementOfType<T>();
}

RenderObject* BuildContext::findAncestorRenderObject() const {
    // Walk up the element tree to find the first RenderObjectElement
    Element* ancestor = element_->parent();
    while (ancestor) {
        if (auto* ro_element = dynamic_cast<RenderObjectElement*>(ancestor)) {
            return ro_element->renderObject();
        }
        ancestor = ancestor->parent();
    }
    return nullptr;
}

Size BuildContext::mediaSize() const {
    // For now, we return a default size or fetch it from the root.
    // In a full app, this would query a MediaQuery InheritedWidget
    // or the Platform/Window abstraction.
    // Stub implementation for Phase 4:
    return {800, 600};
}

Widget* BuildContext::widget() const {
    return element_->widget().get();
}

size_t BuildContext::depth() const {
    return element_->depth();
}

void BuildContext::markNeedsBuild() {
    element_->markNeedsBuild();
}

}  // namespace enki
