#pragma once
/// @file gallery_cubit.hpp
/// @brief Reactive Cubit managing Gallery state, permissions, and async thumbnail worker.

#include "state/gallery_state.hpp"
#include "enki/state/cubit.hpp"
#include <string>
#include <memory>
#include <atomic>

namespace enki::gallery {

class GalleryCubit : public Cubit<GalleryState> {
public:
    GalleryCubit();
    ~GalleryCubit() override;

    /// Check runtime storage permission and trigger non-blocking scan if granted.
    void checkAndSyncStorage();

    /// Request runtime storage permission from system OS.
    void requestStoragePermission();

    /// Open device App Settings (used if permission was permanently denied).
    void openAppSettings();

    /// Change active folder filter.
    void setFolder(const std::string& folder);

    /// Open original full-resolution photo in Fullscreen Viewer.
    void openFullscreen(int index);

    /// Close Fullscreen Viewer and return to grid.
    void closeFullscreen();

    /// Navigate to next photo in Fullscreen Viewer.
    void nextPhoto();

    /// Navigate to previous photo in Fullscreen Viewer.
    void prevPhoto();

    /// Toggle favorite status of a photo.
    void toggleFavorite(size_t index);

private:
    static GalleryState createInitialState();

    /// Starts asynchronous background thread to scan metadata and generate thumbnails.
    void startAsyncScanAndThumbnailWorker();

    std::atomic<int> current_job_id_{0};
};

} // namespace enki::gallery
