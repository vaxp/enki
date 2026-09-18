#pragma once
/// @file font_manager.hpp
/// @brief Manages custom font loading for SkParagraph.
///
/// @copyright ENKI Framework — MIT License

#include <string>
#include <memory>

// Forward declarations
namespace skia { namespace textlayout { class FontCollection; } }
template <typename T> class sk_sp;

namespace enki {

class FontManager {
public:
    /// @brief Load a custom font from file.
    /// @param path Path to the TTF/OTF file.
    /// @param alias Name to use for the font family in TextStyle.
    /// @return true if successful.
    static bool loadFont(const std::string& path, const std::string& alias);

    /// @brief Load a custom font from a raw memory buffer (ideal for WebAssembly / embedded assets).
    /// @param data Pointer to font binary bytes.
    /// @param size Size in bytes.
    /// @param alias Name to use for the font family in TextStyle.
    /// @return true if successful.
    static bool loadFontFromMemory(const void* data, size_t size, const std::string& alias);

    /// @brief Get the global FontCollection used for SkParagraph.
    static sk_sp<skia::textlayout::FontCollection> getFontCollection();
};

} // namespace enki
