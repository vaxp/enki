#pragma once
/// @file image.hpp
/// @brief Image abstraction for loading and drawing images.

#include "enki/core/types.hpp"
#include "enki/core/result.hpp"
#include <memory>
#include <string_view>
#include <vector>

namespace enki {

class Canvas;

/// An immutable 2D pixel image loaded into CPU or GPU memory.
class Image {
public:
    /// Load an image from a file path.
    static Result<std::shared_ptr<Image>> loadFromFile(std::string_view path);

    /// Load an image from memory (encoded bytes like PNG/JPEG).
    static Result<std::shared_ptr<Image>> loadFromMemory(const std::vector<uint8_t>& data);

    ~Image();

    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;

    [[nodiscard]] int getWidth() const;
    [[nodiscard]] int getHeight() const;
    [[nodiscard]] Size getSize() const;

    /// Internal: get native SkImage handle
    void* getNativeHandle() const;

private:
    Image();
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace enki
