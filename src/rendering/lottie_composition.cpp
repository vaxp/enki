/// @file lottie_composition.cpp
/// @brief Implementation of LottieComposition and LottieCache using Skia Skottie.
/// @copyright ENKI Framework — MIT License

#include "enki/rendering/lottie_composition.hpp"
#include "enki/rendering/canvas.hpp"

#include <modules/skottie/include/Skottie.h>
#include <modules/skottie/include/SkottieProperty.h>
#include <modules/skresources/include/SkResources.h>
#include <include/core/SkCanvas.h>
#include <include/core/SkData.h>
#include <include/core/SkStream.h>
#include <include/core/SkFontMgr.h>
#include <include/ports/SkFontMgr_fontconfig.h>

#include <mutex>
#include <unordered_map>
#include <algorithm>
#include <filesystem>
#include <iostream>

namespace enki {

namespace {

sk_sp<SkFontMgr> getLottieFontMgr() {
    static sk_sp<SkFontMgr> mgr = []() {
        auto m = SkFontMgr_New_FontConfig(nullptr);
        if (!m) m = SkFontMgr::RefDefault();
        return m;
    }();
    return mgr;
}

class EnkiMarkerObserver : public skottie::MarkerObserver {
public:
    std::vector<LottieMarker> markers;

    void onMarker(const char name[], float t0, float t1) override {
        if (name) {
            markers.push_back({std::string(name), t0, t1});
        }
    }
};

class EnkiPropertyObserver : public skottie::PropertyObserver {
public:
    std::unordered_map<std::string, std::vector<std::unique_ptr<skottie::ColorPropertyHandle>>> color_handles;
    std::unordered_map<std::string, std::vector<std::unique_ptr<skottie::OpacityPropertyHandle>>> opacity_handles;

    void onColorProperty(const char node_name[],
                         const LazyHandle<skottie::ColorPropertyHandle>& lazy) override {
        if (node_name && lazy) {
            if (auto handle = lazy()) {
                color_handles[std::string(node_name)].push_back(std::move(handle));
            }
        }
    }

    void onOpacityProperty(const char node_name[],
                           const LazyHandle<skottie::OpacityPropertyHandle>& lazy) override {
        if (node_name && lazy) {
            if (auto handle = lazy()) {
                opacity_handles[std::string(node_name)].push_back(std::move(handle));
            }
        }
    }
};

} // namespace

// ════════════════════════════════════════════════════════════════
// LottieComposition::Impl
// ════════════════════════════════════════════════════════════════

struct LottieComposition::Impl {
    sk_sp<skottie::Animation>      animation;
    sk_sp<EnkiMarkerObserver>      marker_observer;
    sk_sp<EnkiPropertyObserver>    property_observer;
    std::vector<LottieMarker>      markers;
    std::string                    source_path;
    double                         current_progress = 0.0;
};

LottieComposition::LottieComposition() : impl_(std::make_unique<Impl>()) {}
LottieComposition::~LottieComposition() = default;

Result<std::shared_ptr<LottieComposition>> LottieComposition::loadFromFile(std::string_view path) {
    std::string path_str(path);
    auto data = SkData::MakeFromFileName(path_str.c_str());

    if (!data) {
        // Fallback: try ../path (e.g. running from build/)
        std::string parent_rel = "../" + path_str;
        data = SkData::MakeFromFileName(parent_rel.c_str());
        if (data) path_str = parent_rel;
    }

    if (!data) {
        // Fallback: try workspace relative
        std::string ws_rel = "/home/x/Work/enki/" + path_str;
        data = SkData::MakeFromFileName(ws_rel.c_str());
        if (data) path_str = ws_rel;
    }

    if (!data) {
        return Result<std::shared_ptr<LottieComposition>>::err(
            ErrorCode::IOError, "Failed to read Lottie file: " + std::string(path));
    }

    std::filesystem::path fs_path(path_str);
    std::string base_dir = fs_path.has_parent_path() ? fs_path.parent_path().string() : ".";

    auto marker_obs = sk_make_sp<EnkiMarkerObserver>();
    auto prop_obs = sk_make_sp<EnkiPropertyObserver>();
    auto font_mgr = getLottieFontMgr();
    auto resource_provider = skresources::FileResourceProvider::Make(SkString(base_dir.c_str()));

    skottie::Animation::Builder builder;
    builder.setFontManager(font_mgr);
    builder.setResourceProvider(resource_provider);
    builder.setMarkerObserver(marker_obs);
    builder.setPropertyObserver(prop_obs);

    SkMemoryStream stream(data);
    auto anim = builder.make(&stream);
    if (!anim) {
        return Result<std::shared_ptr<LottieComposition>>::err(
            ErrorCode::RenderingError, "Failed to parse Lottie animation JSON: " + std::string(path));
    }

    auto comp = std::shared_ptr<LottieComposition>(new LottieComposition());
    comp->impl_->animation = std::move(anim);
    comp->impl_->marker_observer = marker_obs;
    comp->impl_->property_observer = prop_obs;
    comp->impl_->markers = marker_obs->markers;
    comp->impl_->source_path = path_str;

    // Set initial seek to 0 so render() is well-defined
    comp->seek(0.0f);

    return Result<std::shared_ptr<LottieComposition>>::ok(comp);
}

Result<std::shared_ptr<LottieComposition>> LottieComposition::loadFromJson(std::string_view json_content) {
    if (json_content.empty()) {
        return Result<std::shared_ptr<LottieComposition>>::err(
            ErrorCode::InvalidArgument, "JSON content is empty");
    }

    return loadFromMemory(json_content.data(), json_content.size());
}

Result<std::shared_ptr<LottieComposition>> LottieComposition::loadFromMemory(const std::vector<uint8_t>& data) {
    if (data.empty()) {
        return Result<std::shared_ptr<LottieComposition>>::err(
            ErrorCode::InvalidArgument, "Memory data is empty");
    }

    return loadFromMemory(data.data(), data.size());
}

Result<std::shared_ptr<LottieComposition>> LottieComposition::loadFromMemory(const void* data, size_t size) {
    if (!data || size == 0) {
        return Result<std::shared_ptr<LottieComposition>>::err(
            ErrorCode::InvalidArgument, "Invalid data pointer or size");
    }

    auto marker_obs = sk_make_sp<EnkiMarkerObserver>();
    auto prop_obs = sk_make_sp<EnkiPropertyObserver>();
    auto font_mgr = getLottieFontMgr();

    skottie::Animation::Builder builder;
    builder.setFontManager(font_mgr);
    builder.setMarkerObserver(marker_obs);
    builder.setPropertyObserver(prop_obs);

    SkMemoryStream stream(data, size);
    auto anim = builder.make(&stream);
    if (!anim) {
        return Result<std::shared_ptr<LottieComposition>>::err(
            ErrorCode::RenderingError, "Failed to parse Lottie animation from memory");
    }

    auto comp = std::shared_ptr<LottieComposition>(new LottieComposition());
    comp->impl_->animation = std::move(anim);
    comp->impl_->marker_observer = marker_obs;
    comp->impl_->property_observer = prop_obs;
    comp->impl_->markers = marker_obs->markers;

    comp->seek(0.0f);

    return Result<std::shared_ptr<LottieComposition>>::ok(comp);
}

double LottieComposition::duration() const {
    return impl_->animation ? impl_->animation->duration() : 0.0;
}

double LottieComposition::durationMs() const {
    return duration() * 1000.0;
}

double LottieComposition::fps() const {
    return impl_->animation ? impl_->animation->fps() : 60.0;
}

double LottieComposition::inPoint() const {
    return impl_->animation ? impl_->animation->inPoint() : 0.0;
}

double LottieComposition::outPoint() const {
    return impl_->animation ? impl_->animation->outPoint() : 0.0;
}

double LottieComposition::frameCount() const {
    return duration() * fps();
}

Size LottieComposition::getSize() const {
    if (!impl_->animation) return {0.0f, 0.0f};
    const auto& s = impl_->animation->size();
    return {static_cast<float>(s.width()), static_cast<float>(s.height())};
}

float LottieComposition::getWidth() const {
    return getSize().width;
}

float LottieComposition::getHeight() const {
    return getSize().height;
}

const std::vector<LottieMarker>& LottieComposition::getMarkers() const {
    return impl_->markers;
}

std::optional<LottieMarker> LottieComposition::getMarker(std::string_view name) const {
    for (const auto& m : impl_->markers) {
        if (m.name == name) return m;
    }
    return std::nullopt;
}

void LottieComposition::seek(float progress) {
    if (!impl_->animation) return;
    float clamped = std::clamp(progress, 0.0f, 1.0f);
    impl_->current_progress = clamped;
    impl_->animation->seek(clamped);
}

void LottieComposition::seekFrame(double frame_index) {
    if (!impl_->animation) return;
    impl_->animation->seekFrame(frame_index);
}

void LottieComposition::seekTime(double seconds) {
    if (!impl_->animation) return;
    impl_->animation->seekFrameTime(seconds);
}

void LottieComposition::setColor(std::string_view node_name, Color color) {
    if (!impl_->property_observer) return;
    std::string key(node_name);
    auto it = impl_->property_observer->color_handles.find(key);
    if (it != impl_->property_observer->color_handles.end()) {
        SkColor sk_color = static_cast<SkColor>(color);
        for (auto& handle : it->second) {
            if (handle) handle->set(sk_color);
        }
    }
}

void LottieComposition::setOpacity(std::string_view node_name, float opacity) {
    if (!impl_->property_observer) return;
    std::string key(node_name);
    auto it = impl_->property_observer->opacity_handles.find(key);
    if (it != impl_->property_observer->opacity_handles.end()) {
        float clamped_op = std::clamp(opacity, 0.0f, 1.0f) * 100.0f; // Skottie opacity is 0..100
        for (auto& handle : it->second) {
            if (handle) handle->set(clamped_op);
        }
    }
}

void LottieComposition::render(Canvas& canvas, const Rect& dst, uint32_t render_flags) const {
    if (!impl_->animation) return;
    SkCanvas* sk_canvas = static_cast<SkCanvas*>(canvas.getNativeHandle());
    if (!sk_canvas) return;

    SkRect sk_dst = SkRect::MakeXYWH(dst.x, dst.y, dst.width, dst.height);
    impl_->animation->render(sk_canvas, &sk_dst, static_cast<skottie::Animation::RenderFlags>(render_flags));
}

void* LottieComposition::getNativeAnimation() const {
    return impl_->animation.get();
}

// ════════════════════════════════════════════════════════════════
// LottieCache Implementation
// ════════════════════════════════════════════════════════════════

namespace {
    std::unordered_map<std::string, std::shared_ptr<LottieComposition>> s_lottie_cache;
    std::mutex s_lottie_cache_mutex;
}

std::shared_ptr<LottieComposition> LottieCache::getOrLoad(std::string_view path) {
    std::string key(path);
    {
        std::lock_guard<std::mutex> lock(s_lottie_cache_mutex);
        auto it = s_lottie_cache.find(key);
        if (it != s_lottie_cache.end()) {
            return it->second;
        }
    }

    auto res = LottieComposition::loadFromFile(path);
    if (!res.isOk()) {
        return nullptr;
    }

    auto comp = res.value();
    {
        std::lock_guard<std::mutex> lock(s_lottie_cache_mutex);
        s_lottie_cache[key] = comp;
    }
    return comp;
}

void LottieCache::put(std::string_view key, std::shared_ptr<LottieComposition> comp) {
    std::lock_guard<std::mutex> lock(s_lottie_cache_mutex);
    s_lottie_cache[std::string(key)] = std::move(comp);
}

std::shared_ptr<LottieComposition> LottieCache::get(std::string_view key) {
    std::lock_guard<std::mutex> lock(s_lottie_cache_mutex);
    auto it = s_lottie_cache.find(std::string(key));
    if (it != s_lottie_cache.end()) {
        return it->second;
    }
    return nullptr;
}

void LottieCache::clear() {
    std::lock_guard<std::mutex> lock(s_lottie_cache_mutex);
    s_lottie_cache.clear();
}

size_t LottieCache::count() {
    std::lock_guard<std::mutex> lock(s_lottie_cache_mutex);
    return s_lottie_cache.size();
}

} // namespace enki
