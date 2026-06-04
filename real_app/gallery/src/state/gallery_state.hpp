#pragma once
/// @file gallery_state.hpp
/// @brief Reactive state definition for ENKI Gallery application.

#include "models/photo_item.hpp"
#include "enki/platform/permissions.hpp"
#include <vector>
#include <string>
#include <optional>

namespace enki::gallery {

struct GalleryState {
    PermissionStatus        storage_status = PermissionStatus::Unknown;
    std::string             selected_folder = "All";
    std::optional<int>      active_fullscreen_index = std::nullopt;
    std::vector<PhotoItem>  photos;
    std::string             status_message = "";
    int                     scan_version = 0;

    bool operator==(const GalleryState& other) const = default;
};

} // namespace enki::gallery
