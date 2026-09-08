#pragma once
/// @file audio_capture.hpp
/// @brief PulseAudio / PipeWire background stream recorder for System Output & Microphone.
///
/// Uses native libpulse-simple for lightweight, ultra-low-latency real-time audio capture.
///
/// @copyright ENKI Framework — MIT License

#include <vector>
#include <string>
#include <functional>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>

namespace enki::audio {

enum class AudioDeviceType {
    SystemOutput, ///< Monitor of system sink (Spotify, YouTube, browser audio, etc.)
    Microphone,   ///< Default audio input source (voice / microphone)
};

struct AudioCaptureConfig {
    AudioDeviceType device_type        = AudioDeviceType::SystemOutput;
    uint32_t        sample_rate        = 48000; ///< Matches native hardware/PipeWire clock
    uint8_t         channels           = 2;     ///< Stereo
    size_t          buffer_size        = 512;   ///< Samples per capture slice (~10.6ms at 48kHz)
    bool            fallback_synthetic = true;  ///< Procedural tone/beat if hardware is idle/muted
};

using AudioChunkCallback = std::function<void(const float* samples, size_t count, uint8_t channels)>;

class AudioCaptureEngine {
public:
    AudioCaptureEngine();
    ~AudioCaptureEngine();

    bool start(const AudioCaptureConfig& config, AudioChunkCallback callback);
    void stop();
    [[nodiscard]] bool isRunning() const;

    [[nodiscard]] AudioDeviceType deviceType() const { return config_.device_type; }

private:
    void captureLoop();
    static std::string resolveMonitorSourceName();

    AudioCaptureConfig config_;
    AudioChunkCallback callback_;
    std::atomic<bool>  running_{false};
    std::unique_ptr<std::thread> thread_;
    std::mutex         mutex_;
};

} // namespace enki::audio
