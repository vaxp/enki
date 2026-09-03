#pragma once
/// @file audio_controller.hpp
/// @brief High-level thread-safe controller connecting AudioCaptureEngine with AudioAnalyzer.
///
/// Serves as the primary bridge between background audio capture and ENKI's render loop.
///
/// @copyright ENKI Framework — MIT License

#include "audio_capture.hpp"
#include "audio_analyzer.hpp"
#include <memory>
#include <mutex>

namespace enki::audio {

class AudioController {
public:
    AudioController();
    ~AudioController();

    /// Start capturing system audio output (Spotify, YouTube, browser, etc.)
    bool startSystemAudio(size_t buffer_size = 512);

    /// Start capturing live microphone input (voice recording, speech)
    bool startMicrophone(size_t buffer_size = 512);

    /// Stop capture
    void stop();

    /// Get current frequency bands snapshot [0.0 .. 1.0]
    std::vector<float> getBands(size_t count, float decay = 0.06f, float sensitivity = 1.0f);

    /// Get current time-domain waveform snapshot
    std::vector<float> getWaveform(size_t count);

    /// Get instantaneous RMS volume level
    [[nodiscard]] float getVolume() const;

    /// Get instantaneous Bass / Beat energy level [0.0 .. 1.0]
    [[nodiscard]] float getBassEnergy() const;

    [[nodiscard]] bool isRunning() const;
    [[nodiscard]] AudioDeviceType currentMode() const;

    /// Programmatic feed (e.g. for custom audio pipelines or testing)
    void feedSamples(const float* samples, size_t count, uint8_t channels = 2);

private:
    AudioCaptureEngine engine_;
    AudioAnalyzer      analyzer_;
    AudioDeviceType    mode_{AudioDeviceType::SystemOutput};
    mutable std::mutex data_mutex_;
};

} // namespace enki::audio
