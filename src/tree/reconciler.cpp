/// @file reconciler.cpp
/// @brief Reconciler / Global build and diffing utilities.
///
/// Note: Most of the reconciliation logic for single children is in
/// Element::updateChild, and for multiple children is in
/// MultiChildRenderObjectElement::updateChildren.
///
/// This file provides global utilities for initiating the build process
/// and tree diffing helpers if needed.

#include "enki/tree/element.hpp"
#include "enki/tree/widget.hpp"
#include "enki/tree/build_context.hpp"

namespace enki {

// Global reconciler utilities can be added here.
// Currently, the core O(n) reconciliation algorithm is implemented directly
// inside MultiChildRenderObjectElement::updateChildren (in element.cpp)
// to have direct access to its child list.

// We keep this file for future expansion of diffing utilities,
// global tree validators, or advanced reconciliation strategies.

}  // namespace enki
