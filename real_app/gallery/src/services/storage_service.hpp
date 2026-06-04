#pragma once
/// @file storage_service.hpp
/// @brief Non-blocking filesystem photo scanner & Skia thumbnail cache engine.

#include "models/photo_item.hpp"
#include <vector>
#include <string>
#include <cstdint>

namespace enki::gallery {

class StorageService {
public:
    /// Fast scan (pure directory iteration, zero decoding).
    /// Discovers photo paths and checks if a cached thumbnail already exists.
    static std::vector<PhotoItem> scanPhotosMetadata(size_t max_photos = 60);

    /// Get platform-specific writable thumbnail cache directory.
    static std::string getThumbnailCacheDir();

    /// Generate or fetch a cached lightweight (~10KB-20KB) Skia thumbnail.
    /// Safe to call on background worker threads.
    static std::string getOrCreateThumbnail(const std::string& original_path, uintmax_t file_size);
};

} // namespace enki::gallery
