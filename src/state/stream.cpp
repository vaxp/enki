/// @file stream.cpp
/// @brief Stream implementation — non-template utilities.
///
/// The Stream<T> class is fully implemented in the header (stream.hpp)
/// because it is a class template. This file provides any non-template
/// utilities and ensures the translation unit is compiled.
///
/// @copyright ENKI Framework — MIT License

#include "enki/state/stream.hpp"

namespace enki {

// Stream<T> is a class template — all implementation is in the header.
// This translation unit exists to:
//   1. Verify the header compiles cleanly.
//   2. Provide a place for future non-template utilities.
//   3. Satisfy the build system's source file expectations.

}  // namespace enki
