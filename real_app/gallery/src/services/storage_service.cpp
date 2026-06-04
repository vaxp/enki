#include "services/storage_service.hpp"
#include <include/core/SkEncodedImageFormat.h>
#include <include/core/SkImage.h>
#include <include/core/SkSurface.h>
#include <include/core/SkCanvas.h>
#include <include/core/SkData.h>
#include <include/core/SkPaint.h>
#include <include/core/SkSamplingOptions.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cstdlib>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace fs = std::filesystem;

namespace enki::gallery {

static std::string pathToUtf8(const fs::path& p) {
#if defined(_WIN32)
    const std::wstring& wstr = p.native();
    if (wstr.empty()) return "";
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), static_cast<int>(wstr.size()), NULL, 0, NULL, NULL);
    if (size_needed <= 0) return "";
    std::string str(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.data(), static_cast<int>(wstr.size()), &str[0], size_needed, NULL, NULL);
    return str;
#else
    return p.string();
#endif
}

static fs::path stringToPath(const std::string& s) {
#if defined(_WIN32)
    if (s.empty()) return fs::path();
    int wlen = MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), NULL, 0);
    if (wlen <= 0) return fs::path(s);
    std::wstring wstr(wlen, 0);
    MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), &wstr[0], wlen);
    return fs::path(wstr);
#else
    return fs::path(s);
#endif
}

std::string StorageService::getThumbnailCacheDir() {
#if defined(__ANDROID__)
    std::string dir = "/sdcard/Android/data/org.enki.gallery/cache/thumbnails";
#elif defined(_WIN32)
    const char* user_profile = std::getenv("USERPROFILE");
    std::string dir = user_profile ? (std::string(user_profile) + "\\Pictures\\.thumbnails") : ".thumbnails";
#else
    const char* home = std::getenv("HOME");
    std::string dir = home ? (std::string(home) + "/.cache/enki_gallery/thumbnails") : ".thumbnails";
#endif
    std::error_code ec;
    fs::create_directories(stringToPath(dir), ec);
    return dir;
}

std::string StorageService::getOrCreateThumbnail(const std::string& original_path, uintmax_t file_size) {
    try {
        // 1. Compute unique cache filename from path hash
        size_t path_hash = std::hash<std::string>{}(original_path);
        std::string cache_dir = getThumbnailCacheDir();
        std::string thumb_filename = "thumb_" + std::to_string(path_hash) + ".jpg";
        fs::path thumb_path = stringToPath(cache_dir) / thumb_filename;

        std::error_code ec;
        // 2. Fast hit: If cached thumbnail already exists and is non-empty, return immediately!
        if (fs::exists(thumb_path, ec) && fs::file_size(thumb_path, ec) > 0) {
            return pathToUtf8(thumb_path);
        }

        // 3. Skia downsampling engine (Target max 280px dimension)
        auto data = SkData::MakeFromFileName(original_path.c_str());
        if (!data) return original_path;

        auto orig_img = SkImage::MakeFromEncoded(data);
        if (!orig_img) return original_path;

        int orig_w = orig_img->width();
        int orig_h = orig_img->height();
        if (orig_w <= 0 || orig_h <= 0) return original_path;

        float scale = std::min(1.0f, 280.0f / static_cast<float>(std::max(orig_w, orig_h)));
        int dst_w = std::max(1, static_cast<int>(orig_w * scale));
        int dst_h = std::max(1, static_cast<int>(orig_h * scale));

        auto surface = SkSurface::MakeRasterN32Premul(dst_w, dst_h);
        if (!surface) return original_path;

        SkCanvas* canvas = surface->getCanvas();
        SkPaint paint;
        SkSamplingOptions sampling(SkFilterMode::kLinear);
        SkRect dst_rect = SkRect::MakeWH(static_cast<float>(dst_w), static_cast<float>(dst_h));
        canvas->drawImageRect(orig_img, dst_rect, sampling, &paint);

        auto snap = surface->makeImageSnapshot();
        if (!snap) return original_path;

        auto encoded = snap->encodeToData(SkEncodedImageFormat::kJPEG, 75);
        if (!encoded) return original_path;

        std::ofstream ofs(thumb_path, std::ios::binary);
        if (!ofs.is_open()) return original_path;
        ofs.write(reinterpret_cast<const char*>(encoded->data()), static_cast<std::streamsize>(encoded->size()));
        ofs.close();

        return pathToUtf8(thumb_path);
    } catch (...) {
        return original_path;
    }
}

std::vector<PhotoItem> StorageService::scanPhotosMetadata(size_t max_photos) {
    std::vector<PhotoItem> found_photos;
    std::vector<std::string> search_dirs;

#if defined(__ANDROID__)
    search_dirs.push_back("/sdcard/DCIM");
    search_dirs.push_back("/sdcard/DCIM/Camera");
    search_dirs.push_back("/sdcard/DCIM/Screenshots");
    search_dirs.push_back("/sdcard/Pictures");
    search_dirs.push_back("/sdcard/Pictures/Screenshots");
    search_dirs.push_back("/sdcard/Download");
    search_dirs.push_back("/sdcard");
    search_dirs.push_back("/storage/emulated/0/DCIM");
    search_dirs.push_back("/storage/emulated/0/DCIM/Camera");
    search_dirs.push_back("/storage/emulated/0/Pictures");
    search_dirs.push_back("/storage/emulated/0/Download");

#elif defined(_WIN32)
    const char* user_profile = std::getenv("USERPROFILE");
    if (user_profile) {
        std::string pic = std::string(user_profile) + "\\Pictures";
        search_dirs.push_back(pic);
        search_dirs.push_back(pic + "\\Screenshots");
        search_dirs.push_back(pic + "\\Saved Pictures");
        search_dirs.push_back(pic + "\\Camera Roll");
        search_dirs.push_back(pic + "\\imges");
        search_dirs.push_back(std::string(user_profile) + "\\Downloads");
    }
#else
    const char* home = std::getenv("HOME");
    if (home) {
        search_dirs.push_back(std::string(home) + "/Pictures");
        search_dirs.push_back(std::string(home) + "/Downloads");
    }
#endif

    std::string cache_dir = getThumbnailCacheDir();
    std::vector<std::string> scanned_paths;
    int photo_id = 1;

    for (const auto& dir_str : search_dirs) {
        std::error_code ec;
        fs::path p = stringToPath(dir_str);
        if (!fs::exists(p, ec) || !fs::is_directory(p, ec)) {
            continue;
        }

        fs::directory_iterator it(p, fs::directory_options::skip_permission_denied, ec);
        fs::directory_iterator end;

        while (!ec && it != end) {
            try {
                const auto& entry = *it;
                std::error_code entry_ec;
                if (entry.is_regular_file(entry_ec)) {
                    std::string ext = pathToUtf8(entry.path().extension());
                    if (isImageExtension(ext)) {
                        std::string full_path = pathToUtf8(entry.path());
                        if (std::find(scanned_paths.begin(), scanned_paths.end(), full_path) == scanned_paths.end()) {
                            scanned_paths.push_back(full_path);

                            PhotoItem item;
                            item.id = "img_" + std::to_string(photo_id++);
                            item.title = pathToUtf8(entry.path().stem());
                            item.path = full_path;
                            item.folder_name = pathToUtf8(entry.path().parent_path().filename());
                            if (item.folder_name == ".thumbnails" || item.folder_name == "thumbnails") {
                                item.folder_name = "Thumbnails";
                            } else if (item.folder_name.empty() || item.folder_name == "0" || item.folder_name == "sdcard") {
                                item.folder_name = "Storage";
                            }

                            uintmax_t sz = entry.file_size(entry_ec);
                            item.file_size = entry_ec ? "Unknown" : formatFileSize(sz);
                            item.is_favorite = false;

                            // Check if cached thumbnail exists
                            size_t path_hash = std::hash<std::string>{}(full_path);
                            std::string thumb_filename = "thumb_" + std::to_string(path_hash) + ".jpg";
                            fs::path tp = stringToPath(cache_dir) / thumb_filename;
                            std::error_code tp_ec;
                            if (fs::exists(tp, tp_ec) && fs::file_size(tp, tp_ec) > 0) {
                                item.thumb_path = pathToUtf8(tp);
                            } else {
                                item.thumb_path = "";
                            }

                            found_photos.push_back(std::move(item));
                            if (found_photos.size() >= max_photos) break;
                        }
                    }
                }
            } catch (const std::exception& e) {
                // Safely skip any problematic file
            } catch (...) {
                // Safely skip unknown error
            }

            it.increment(ec);
        }

        if (found_photos.size() >= max_photos) break;
    }

    std::cout << "[StorageService] Fast metadata scan completed: "
              << found_photos.size() << " photos discovered.\n";

    return found_photos;
}

} // namespace enki::gallery

