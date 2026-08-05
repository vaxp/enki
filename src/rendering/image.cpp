/// @file image.cpp
/// @brief Image implementation using Skia.

#include "enki/rendering/image.hpp"
#include <include/core/SkImage.h>
#include <include/core/SkData.h>
#include <include/core/SkStream.h>

namespace enki {

struct Image::Impl {
    sk_sp<SkImage> image;
};

Image::Image() : impl_(std::make_unique<Impl>()) {}
Image::~Image() = default;

Result<std::shared_ptr<Image>> Image::loadFromFile(std::string_view path) {
    auto data = SkData::MakeFromFileName(std::string(path).c_str());
    if (!data) {
        return Result<std::shared_ptr<Image>>::err(
            ErrorCode::IOError, "Failed to read image file: " + std::string(path));
    }

    auto sk_img = SkImage::MakeFromEncoded(data);
    if (!sk_img) {
        return Result<std::shared_ptr<Image>>::err(
            ErrorCode::RenderingError, "Failed to decode image: " + std::string(path));
    }

    auto img = std::shared_ptr<Image>(new Image());
    img->impl_->image = std::move(sk_img);
    return Result<std::shared_ptr<Image>>::ok(img);
}

Result<std::shared_ptr<Image>> Image::loadFromMemory(const std::vector<uint8_t>& data) {
    auto sk_data = SkData::MakeWithCopy(data.data(), data.size());
    auto sk_img = SkImage::MakeFromEncoded(sk_data);
    
    if (!sk_img) {
        return Result<std::shared_ptr<Image>>::err(
            ErrorCode::RenderingError, "Failed to decode image from memory");
    }

    auto img = std::shared_ptr<Image>(new Image());
    img->impl_->image = std::move(sk_img);
    return Result<std::shared_ptr<Image>>::ok(img);
}

int Image::getWidth() const {
    return impl_->image ? impl_->image->width() : 0;
}

int Image::getHeight() const {
    return impl_->image ? impl_->image->height() : 0;
}

Size Image::getSize() const {
    return {static_cast<float>(getWidth()), static_cast<float>(getHeight())};
}

void* Image::getNativeHandle() const {
    return impl_->image.get();
}

}  // namespace enki
