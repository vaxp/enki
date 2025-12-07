/// @file video_controller.cpp
/// @brief Implementation of VideoController.
/// @copyright ENKI Framework — MIT License

#include "video/video_controller.hpp"

namespace enki::video {

VideoController::VideoController()
    : decoder_(std::make_unique<VideoDecoder>()) {}

VideoController::VideoController(const std::string& source)
    : decoder_(std::make_unique<VideoDecoder>()) {
    open(source);
}

VideoController::~VideoController() {
    close();
}

bool VideoController::open(const std::string& source) {
    if (!decoder_) {
        decoder_ = std::make_unique<VideoDecoder>();
    }
    decoder_->setVolume(is_muted_ ? 0.0f : volume_);
    decoder_->setPlaybackSpeed(speed_);
    decoder_->setLooping(looping_);
    return decoder_->open(source);
}

void VideoController::close() {
    if (decoder_) {
        decoder_->close();
    }
}

void VideoController::play() {
    if (decoder_) {
        decoder_->play();
    }
}

void VideoController::pause() {
    if (decoder_) {
        decoder_->pause();
    }
}

void VideoController::togglePlay() {
    if (!decoder_) return;
    if (decoder_->getState() == PlaybackState::Playing) {
        pause();
    } else {
        play();
    }
}

void VideoController::stop() {
    if (decoder_) {
        decoder_->pause();
        decoder_->seek(0.0);
    }
}

void VideoController::seek(double seconds) {
    if (decoder_) {
        decoder_->seek(seconds);
    }
}

void VideoController::setVolume(float volume) {
    volume_ = std::clamp(volume, 0.0f, 1.0f);
    if (volume_ > 0.0f) {
        is_muted_ = false;
    }
    if (decoder_) {
        decoder_->setVolume(is_muted_ ? 0.0f : volume_);
    }
}

void VideoController::setMuted(bool muted) {
    if (is_muted_ == muted) return;
    is_muted_ = muted;
    if (is_muted_) {
        previous_volume_ = volume_;
        if (decoder_) decoder_->setVolume(0.0f);
    } else {
        if (decoder_) decoder_->setVolume(previous_volume_);
    }
}

void VideoController::setPlaybackSpeed(float speed) {
    speed_ = std::clamp(speed, 0.25f, 3.0f);
    if (decoder_) {
        decoder_->setPlaybackSpeed(speed_);
    }
}

void VideoController::setLooping(bool looping) {
    looping_ = looping;
    if (decoder_) {
        decoder_->setLooping(looping_);
    }
}

PlaybackState VideoController::getState() const {
    return decoder_ ? decoder_->getState() : PlaybackState::Unloaded;
}

bool VideoController::isPlaying() const {
    return getState() == PlaybackState::Playing;
}

double VideoController::getCurrentPosition() const {
    return decoder_ ? decoder_->getCurrentPosition() : 0.0;
}

double VideoController::getDuration() const {
    return decoder_ ? decoder_->getDuration() : 0.0;
}

VideoMetadata VideoController::getMetadata() const {
    return decoder_ ? decoder_->getMetadata() : VideoMetadata{};
}

std::shared_ptr<VideoFrame> VideoController::getNextRenderFrame() {
    return decoder_ ? decoder_->getNextRenderFrame() : nullptr;
}

} // namespace enki::video
