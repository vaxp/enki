/// @file audio_controller.cpp
/// @brief High-level thread-safe controller connecting capture engine and FFT analyzer.
/// @copyright ENKI Framework — MIT License

#include "audio/audio_controller.hpp"

namespace enki::audio {

AudioController::AudioController() : analyzer_(512) {}

AudioController::~AudioController() {
    stop();
}

bool AudioController::startSystemAudio(size_t buffer_size) {
    stop();
    {
        std::lock_guard<std::mutex> lock(data_mutex_);
        mode_ = AudioDeviceType::SystemOutput;
        analyzer_.reset();
    }

    AudioCaptureConfig cfg;
    cfg.device_type = AudioDeviceType::SystemOutput;
    cfg.sample_rate = 48000;
    cfg.channels    = 2;
    cfg.buffer_size = buffer_size;
    cfg.fallback_synthetic = true;

    return engine_.start(cfg, [this](const float* samples, size_t count, uint8_t channels) {
        std::lock_guard<std::mutex> lock(data_mutex_);
        analyzer_.processSamples(samples, count, channels);
    });
}

bool AudioController::startMicrophone(size_t buffer_size) {
    stop();
    {
        std::lock_guard<std::mutex> lock(data_mutex_);
        mode_ = AudioDeviceType::Microphone;
        analyzer_.reset();
    }

    AudioCaptureConfig cfg;
    cfg.device_type = AudioDeviceType::Microphone;
    cfg.sample_rate = 48000;
    cfg.channels    = 2;
    cfg.buffer_size = buffer_size;
    cfg.fallback_synthetic = true;

    return engine_.start(cfg, [this](const float* samples, size_t count, uint8_t channels) {
        std::lock_guard<std::mutex> lock(data_mutex_);
        analyzer_.processSamples(samples, count, channels);
    });
}

void AudioController::stop() {
    engine_.stop();
    std::lock_guard<std::mutex> lock(data_mutex_);
    analyzer_.reset();
}

std::vector<float> AudioController::getBands(size_t count, float decay, float sensitivity) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    return analyzer_.getDecayedBands(count, decay, sensitivity);
}

std::vector<float> AudioController::getWaveform(size_t count) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    return analyzer_.getWaveformSamples(count);
}

float AudioController::getVolume() const {
    std::lock_guard<std::mutex> lock(data_mutex_);
    return analyzer_.getRmsVolume();
}

float AudioController::getBassEnergy() const {
    std::lock_guard<std::mutex> lock(data_mutex_);
    return analyzer_.getBassEnergy();
}

bool AudioController::isRunning() const {
    return engine_.isRunning();
}

AudioDeviceType AudioController::currentMode() const {
    return mode_;
}

void AudioController::feedSamples(const float* samples, size_t count, uint8_t channels) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    analyzer_.processSamples(samples, count, channels);
}

} // namespace enki::audio
