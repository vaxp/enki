#pragma once
/// @file photo_item.hpp
/// @brief PhotoItem data model and file formatting utilities.

#include <string>
#include <cstdint>
#include <cctype>
#include <cstdio>

namespace enki::gallery {

struct PhotoItem {
    std::string id;
    std::string title;
    std::string path;          // Original full-resolution image (used for Fullscreen Viewer)
    std::string thumb_path;    // Lightweight thumbnail (used for PhotoGrid)
    std::string folder_name;
    std::string file_size;
    bool        is_favorite = false;

    bool operator==(const PhotoItem& other) const = default;
};

inline std::string formatFileSize(uintmax_t bytes) {
    if (bytes >= 1024 * 1024) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%.1f MB", static_cast<double>(bytes) / (1024.0 * 1024.0));
        return buf;
    } else if (bytes >= 1024) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%.0f KB", static_cast<double>(bytes) / 1024.0);
        return buf;
    }
    return std::to_string(bytes) + " B";
}

inline bool isImageExtension(const std::string& ext) {
    std::string lower = ext;
    for (char& c : lower) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return lower == ".png" || lower == ".jpg" || lower == ".jpeg" ||
           lower == ".webp" || lower == ".bmp";
}

} // namespace enki::gallery
