/// @file audio_capture.cpp
/// @brief Native PulseAudio / PipeWire ultra-low-latency audio capture implementation.
/// @copyright ENKI Framework — MIT License

#include "audio/audio_capture.hpp"
#include <pulse/simple.h>
#include <pulse/error.h>

#include <iostream>
#include <array>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <chrono>

namespace enki::audio {

AudioCaptureEngine::AudioCaptureEngine() = default;

AudioCaptureEngine::~AudioCaptureEngine() {
    stop();
}

std::string AudioCaptureEngine::resolveMonitorSourceName() {
    // 1. Check environment variable override
    if (const char* env_mon = std::getenv("PULSE_MONITOR")) {
        return env_mon;
    }

    // 2. Query default sink via pactl and append .monitor
    FILE* pipe = popen("pactl get-default-sink 2>/dev/null", "r");
    if (pipe) {
        char buffer[256];
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            std::string sink(buffer);
            // Trim whitespace/newline
            while (!sink.empty() && (sink.back() == '\n' || sink.back() == '\r' || sink.back() == ' ')) {
                sink.pop_back();
            }
            pclose(pipe);
            if (!sink.empty()) {
                return sink + ".monitor";
            }
        } else {
            pclose(pipe);
        }
    }

    // 3. Fallback to @DEFAULT_MONITOR@ (supported by PipeWire / PulseAudio 14+)
    return "@DEFAULT_MONITOR@";
}

bool AudioCaptureEngine::start(const AudioCaptureConfig& config, AudioChunkCallback callback) {
    stop();

    std::lock_guard<std::mutex> lock(mutex_);
    config_   = config;
    callback_ = std::move(callback);
    running_  = true;

    thread_ = std::make_unique<std::thread>(&AudioCaptureEngine::captureLoop, this);
    return true;
}

void AudioCaptureEngine::stop() {
    if (running_.exchange(false)) {
        if (thread_ && thread_->joinable()) {
            thread_->join();
        }
        thread_.reset();
    }
}

bool AudioCaptureEngine::isRunning() const {
    return running_.load();
}

void AudioCaptureEngine::captureLoop() {
    pa_sample_spec ss;
    ss.format   = PA_SAMPLE_S16LE;
    ss.rate     = config_.sample_rate;
    ss.channels = config_.channels;

    std::string device_name;
    const char* dev_ptr = nullptr;

    if (config_.device_type == AudioDeviceType::SystemOutput) {
        device_name = resolveMonitorSourceName();
        dev_ptr = device_name.c_str();
    } else {
        // NULL selects the default source (microphone)
        dev_ptr = nullptr;
    }

    const size_t frames_per_read = config_.buffer_size;
    const size_t samples_per_read = frames_per_read * config_.channels;
    const uint32_t frag_bytes = static_cast<uint32_t>(samples_per_read * sizeof(int16_t));

    // ════════════════════════════════════════════════════════════════════════
    // CRITICAL: Low Latency Buffering Configuration (Eliminates 2-4s delay)
    // ════════════════════════════════════════════════════════════════════════
    // By default, PulseAudio/PipeWire sets fragsize to 2000ms - 4000ms.
    // Setting fragsize explicitly to our chunk size (e.g. 2048 bytes = ~10.6ms at 48kHz)
    // and maxlength to 2x fragsize ensures the server buffers only 10-20ms of audio,
    // achieving true real-time, zero-lag synchronization with music and speech.
    pa_buffer_attr ba;
    ba.maxlength = frag_bytes * 2;
    ba.tlength   = static_cast<uint32_t>(-1);
    ba.prebuf    = static_cast<uint32_t>(-1);
    ba.minreq    = static_cast<uint32_t>(-1);
    ba.fragsize  = frag_bytes;

    int error = 0;
    pa_simple* s = pa_simple_new(
        nullptr,                                    // Server
        "ENKI Audio Visualizer",                    // Application name
        PA_STREAM_RECORD,                           // Stream direction
        dev_ptr,                                    // Device name
        (config_.device_type == AudioDeviceType::SystemOutput) ? "System Audio Monitor" : "Microphone Input",
        &ss,                                        // Sample spec
        nullptr,                                    // Channel map
        &ba,                                        // Low latency buffer attributes
        &error                                      // Error code
    );

    if (s) {
        // Purge any stale pre-buffered data so we begin instantly at current audio
        pa_simple_flush(s, &error);
    }

    std::vector<int16_t> pcm_buffer(samples_per_read, 0);
    std::vector<float>   float_buffer(samples_per_read, 0.0f);

    float synthetic_phase = 0.0f;

    while (running_.load()) {
        bool read_success = false;

        if (s) {
            int ret = pa_simple_read(s, pcm_buffer.data(), pcm_buffer.size() * sizeof(int16_t), &error);
            if (ret >= 0) {
                read_success = true;
                for (size_t i = 0; i < samples_per_read; ++i) {
                    float_buffer[i] = static_cast<float>(pcm_buffer[i]) / 32768.0f;
                }
            }
        }

        // Fallback generator if hardware read failed or not available
        if (!read_success) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            if (config_.fallback_synthetic) {
                for (size_t i = 0; i < frames_per_read; ++i) {
                    float t = synthetic_phase + (float(i) / float(frames_per_read)) * 0.1f;
                    float v = 0.0f;
                    if (config_.device_type == AudioDeviceType::SystemOutput) {
                        // Rhythmic beat simulation
                        float beat = std::pow(std::max(0.0f, std::sin(t * 4.0f)), 8.0f);
                        v = (std::sin(t * 80.0f) * 0.4f + std::sin(t * 220.0f) * 0.3f) * (0.3f + 0.7f * beat);
                    } else {
                        // Voice pitch simulation
                        v = std::sin(t * 120.0f) * 0.25f + std::sin(t * 260.0f) * 0.15f;
                    }
                    float_buffer[i * 2] = v;
                    if (config_.channels > 1) {
                        float_buffer[i * 2 + 1] = v;
                    }
                }
                synthetic_phase += 0.06f;
            }
        }

        // Emit samples to analyzer callback
        if (callback_ && running_.load()) {
            callback_(float_buffer.data(), float_buffer.size(), config_.channels);
        }
    }

    if (s) {
        pa_simple_free(s);
    }
}

} // namespace enki::audio
