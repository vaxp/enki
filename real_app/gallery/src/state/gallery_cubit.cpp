#include "state/gallery_cubit.hpp"
#include "services/storage_service.hpp"
#include <iostream>
#include <thread>

namespace enki::gallery {

GalleryCubit::GalleryCubit() : Cubit(createInitialState()) {
    checkAndSyncStorage();
}

GalleryCubit::~GalleryCubit() {
    // Invalidate background job so worker thread terminates early
    current_job_id_++;
}

GalleryState GalleryCubit::createInitialState() {
    GalleryState s;
    s.storage_status = PermissionStatus::Unknown;
    s.selected_folder = "All";
    s.active_fullscreen_index = std::nullopt;
    s.status_message = "Checking runtime storage permission...";
    return s;
}

void GalleryCubit::checkAndSyncStorage() {
    PermissionStatus status = Permissions::check(Permission::Storage);
    auto s = state();
    s.storage_status = status;

    if (status != PermissionStatus::Granted) {
        s.photos.clear();
        if (status == PermissionStatus::PermanentlyDenied) {
            s.status_message = "Storage permission was permanently denied by the system. Please grant it in App Settings.";
        } else {
            s.status_message = "Storage permission is required to view device photos. Please grant access below.";
        }
        s.scan_version++;
        emit(s);
        return;
    }

    s.status_message = "Scanning storage photos in background...";
    s.scan_version++;
    emit(s);

    startAsyncScanAndThumbnailWorker();
}

void GalleryCubit::startAsyncScanAndThumbnailWorker() {
    int job = ++current_job_id_;

    std::thread([this, job]() {
        try {
            std::cout << "[GalleryWorker] Starting async photo metadata scan (job " << job << ")...\n";
            auto scanned = StorageService::scanPhotosMetadata(60);

            if (job != current_job_id_.load()) {
                std::cout << "[GalleryWorker] Job " << job << " cancelled.\n";
                return;
            }

            // Emit discovered photos immediately to display cards in UI
            {
                auto s = state();
                s.photos = scanned;
                s.status_message = "Found " + std::to_string(scanned.size()) + " photos. Loading thumbnails...";
                s.scan_version++;
                emit(s);
            }

            std::cout << "[GalleryWorker] Metadata scan done. Generating thumbnails in background...\n";

            // Generate thumbnails asynchronously
            for (size_t i = 0; i < scanned.size(); ++i) {
                if (job != current_job_id_.load()) return;

                if (!scanned[i].thumb_path.empty()) continue;

                std::string thumb = StorageService::getOrCreateThumbnail(scanned[i].path, 1024 * 1024);
                if (!thumb.empty() && job == current_job_id_.load()) {
                    auto s = state();
                    if (i < s.photos.size() && s.photos[i].path == scanned[i].path) {
                        s.photos[i].thumb_path = thumb;
                        emit(s);
                    }
                }
            }

            if (job == current_job_id_.load()) {
                auto s = state();
                s.status_message = "Storage synced: " + std::to_string(s.photos.size()) + " photos ready.";
                emit(s);
                std::cout << "[GalleryWorker] All thumbnails generated.\n";
            }
        } catch (const std::exception& e) {
            std::cerr << "[GalleryWorker] Background worker caught exception: " << e.what() << "\n";
        } catch (...) {
            std::cerr << "[GalleryWorker] Background worker caught unknown error.\n";
        }
    }).detach();
}

void GalleryCubit::requestStoragePermission() {
    std::cout << "[GalleryCubit] Calling Permissions::request(Permission::Storage)...\n";
    Permissions::request(Permission::Storage, [this](PermissionStatus status) {
        std::cout << "[GalleryCubit] Permission response: "
                  << Permissions::toString(status) << "\n";
        checkAndSyncStorage();
    });
}

void GalleryCubit::openAppSettings() {
    std::cout << "[GalleryCubit] Calling Permissions::openAppSettings()...\n";
    Permissions::openAppSettings();
}

void GalleryCubit::setFolder(const std::string& folder) {
    auto s = state();
    s.selected_folder = folder;
    emit(s);
}

void GalleryCubit::openFullscreen(int index) {
    std::cout << "[GalleryCubit] >>> openFullscreen triggered for photo index: " << index << " <<<\n";
    auto s = state();
    if (index >= 0 && index < static_cast<int>(s.photos.size())) {
        s.active_fullscreen_index = index;
        emit(s);
    }
}

void GalleryCubit::closeFullscreen() {
    std::cout << "[GalleryCubit] >>> closeFullscreen triggered <<<\n";
    auto s = state();
    s.active_fullscreen_index = std::nullopt;
    emit(s);
}

void GalleryCubit::nextPhoto() {
    auto s = state();
    if (!s.active_fullscreen_index.has_value() || s.photos.empty()) return;
    int cur = *s.active_fullscreen_index;
    s.active_fullscreen_index = (cur + 1) % static_cast<int>(s.photos.size());
    std::cout << "[GalleryCubit] nextPhoto: " << *s.active_fullscreen_index << "\n";
    emit(s);
}

void GalleryCubit::prevPhoto() {
    auto s = state();
    if (!s.active_fullscreen_index.has_value() || s.photos.empty()) return;
    int cur = *s.active_fullscreen_index;
    s.active_fullscreen_index = (cur - 1 + static_cast<int>(s.photos.size())) % static_cast<int>(s.photos.size());
    std::cout << "[GalleryCubit] prevPhoto: " << *s.active_fullscreen_index << "\n";
    emit(s);
}

void GalleryCubit::toggleFavorite(size_t index) {
    auto s = state();
    if (index < s.photos.size()) {
        s.photos[index].is_favorite = !s.photos[index].is_favorite;
        std::cout << "[GalleryCubit] toggleFavorite: " << index << " -> " << s.photos[index].is_favorite << "\n";
        emit(s);
    }
}

} // namespace enki::gallery
