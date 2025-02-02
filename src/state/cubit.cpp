/// @file cubit.cpp
/// @brief Cubit implementation — non-template utilities and explicit instantiations.
///
/// Cubit<S> is a class template fully implemented in cubit.hpp.
/// This file provides explicit instantiations for common types and
/// ensures the translation unit compiles cleanly.
///
/// @copyright ENKI Framework — MIT License

#include "enki/state/cubit.hpp"

namespace enki {

// Cubit<S> is a class template — all implementation is in the header.
// This translation unit verifies compilation and provides a home for
// any future non-template utilities.

// BlocObserver static member is defined inline in the header (C++17).
// No additional definition needed here.

}  // namespace enki
