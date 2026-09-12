/// @file audio_capture.cpp
/// @brief Native PulseAudio / PipeWire ultra-low-latency audio capture implementation.
/// @copyright ENKI Framework — MIT License

#include "audio/audio_capture.hpp"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <ksmedia.h>
#else
#include <pulse/simple.h>
#include <pulse/error.h>
#endif

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

#if !defined(_WIN32)
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
#endif

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

#if !defined(_WIN32)
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
#else
// ════════════════════════════════════════════════════════════════════════
// Windows Native WASAPI Loopback & Microphone Audio Capture
// ════════════════════════════════════════════════════════════════════════
void AudioCaptureEngine::captureLoop() {
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    bool com_initialized = SUCCEEDED(hr);

    IMMDeviceEnumerator* enumerator = nullptr;
    hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator),
        nullptr,
        CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator),
        reinterpret_cast<void**>(&enumerator)
    );

    IMMDevice* device = nullptr;
    if (SUCCEEDED(hr) && enumerator) {
        EDataFlow data_flow = (config_.device_type == AudioDeviceType::SystemOutput) ? eRender : eCapture;
        hr = enumerator->GetDefaultAudioEndpoint(data_flow, eConsole, &device);
    }

    IAudioClient* audio_client = nullptr;
    if (SUCCEEDED(hr) && device) {
        hr = device->Activate(
            __uuidof(IAudioClient),
            CLSCTX_ALL,
            nullptr,
            reinterpret_cast<void**>(&audio_client)
        );
    }

    WAVEFORMATEX* mix_format = nullptr;
    if (SUCCEEDED(hr) && audio_client) {
        hr = audio_client->GetMixFormat(&mix_format);
    }

    IAudioCaptureClient* capture_client = nullptr;
    bool client_started = false;

    if (SUCCEEDED(hr) && audio_client && mix_format) {
        REFERENCE_TIME hns_buffer_duration = 1000000; // 100ms in 100ns units
        DWORD stream_flags = 0;
        if (config_.device_type == AudioDeviceType::SystemOutput) {
            stream_flags |= AUDCLNT_STREAMFLAGS_LOOPBACK;
        }

        hr = audio_client->Initialize(
            AUDCLNT_SHAREMODE_SHARED,
            stream_flags,
            hns_buffer_duration,
            0,
            mix_format,
            nullptr
        );

        if (SUCCEEDED(hr)) {
            hr = audio_client->GetService(
                __uuidof(IAudioCaptureClient),
                reinterpret_cast<void**>(&capture_client)
            );
        }

        if (SUCCEEDED(hr) && capture_client) {
            hr = audio_client->Start();
            if (SUCCEEDED(hr)) {
                client_started = true;
            }
        }
    }

    const size_t target_channels = config_.channels > 0 ? config_.channels : 2;
    const size_t frames_per_chunk = config_.buffer_size > 0 ? config_.buffer_size : 512;
    const size_t chunk_sample_count = frames_per_chunk * target_channels;

    std::vector<float> float_buffer;
    float_buffer.reserve(chunk_sample_count * 4);

    float synthetic_phase = 0.0f;

    bool is_float = false;
    WORD bits_per_sample = 16;
    WORD src_channels = 2;

    if (mix_format) {
        src_channels = mix_format->nChannels;
        bits_per_sample = mix_format->wBitsPerSample;
        if (mix_format->wFormatTag == WAVE_FORMAT_IEEE_FLOAT) {
            is_float = true;
        } else if (mix_format->wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
            auto* wfex = reinterpret_cast<const WAVEFORMATEXTENSIBLE*>(mix_format);
            if (wfex->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT) {
                is_float = true;
            }
        }
    }

    while (running_.load()) {
        bool read_success = false;

        if (client_started && capture_client) {
            UINT32 packet_length = 0;
            hr = capture_client->GetNextPacketSize(&packet_length);

            while (SUCCEEDED(hr) && packet_length > 0 && running_.load()) {
                BYTE* pData = nullptr;
                UINT32 num_frames = 0;
                DWORD flags = 0;

                hr = capture_client->GetBuffer(&pData, &num_frames, &flags, nullptr, nullptr);
                if (SUCCEEDED(hr) && num_frames > 0) {
                    if (flags & AUDCLNT_BUFFERFLAGS_SILENT) {
                        for (size_t f = 0; f < num_frames; ++f) {
                            for (size_t c = 0; c < target_channels; ++c) {
                                float_buffer.push_back(0.0f);
                            }
                        }
                    } else if (pData) {
                        read_success = true;
                        if (is_float) {
                            const float* src = reinterpret_cast<const float*>(pData);
                            for (size_t f = 0; f < num_frames; ++f) {
                                for (size_t c = 0; c < target_channels; ++c) {
                                    size_t ch_idx = (c < src_channels) ? c : 0;
                                    float_buffer.push_back(src[f * src_channels + ch_idx]);
                                }
                            }
                        } else if (bits_per_sample == 16) {
                            const int16_t* src = reinterpret_cast<const int16_t*>(pData);
                            for (size_t f = 0; f < num_frames; ++f) {
                                for (size_t c = 0; c < target_channels; ++c) {
                                    size_t ch_idx = (c < src_channels) ? c : 0;
                                    float val = static_cast<float>(src[f * src_channels + ch_idx]) / 32768.0f;
                                    float_buffer.push_back(val);
                                }
                            }
                        } else if (bits_per_sample == 32) {
                            const int32_t* src = reinterpret_cast<const int32_t*>(pData);
                            for (size_t f = 0; f < num_frames; ++f) {
                                for (size_t c = 0; c < target_channels; ++c) {
                                    size_t ch_idx = (c < src_channels) ? c : 0;
                                    float val = static_cast<float>(src[f * src_channels + ch_idx]) / 2147483648.0f;
                                    float_buffer.push_back(val);
                                }
                            }
                        } else if (bits_per_sample == 24) {
                            const uint8_t* b = pData;
                            for (size_t f = 0; f < num_frames; ++f) {
                                for (size_t c = 0; c < target_channels; ++c) {
                                    size_t ch_idx = (c < src_channels) ? c : 0;
                                    size_t idx = (f * src_channels + ch_idx) * 3;
                                    int32_t sample24 = (b[idx + 0] << 8) | (b[idx + 1] << 16) | (b[idx + 2] << 24);
                                    float val = static_cast<float>(sample24) / 2147483648.0f;
                                    float_buffer.push_back(val);
                                }
                            }
                        }
                    }

                    capture_client->ReleaseBuffer(num_frames);

                    // Flush complete chunks to callback
                    while (float_buffer.size() >= chunk_sample_count) {
                        if (callback_ && running_.load()) {
                            callback_(float_buffer.data(), chunk_sample_count, static_cast<uint8_t>(target_channels));
                        }
                        float_buffer.erase(float_buffer.begin(), float_buffer.begin() + chunk_sample_count);
                    }
                }

                hr = capture_client->GetNextPacketSize(&packet_length);
            }
        }

        // Fallback procedural wave generator if hardware audio is idle/muted or unavailable
        if (!read_success) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            if (config_.fallback_synthetic) {
                std::vector<float> synth_buffer(chunk_sample_count, 0.0f);
                for (size_t i = 0; i < frames_per_chunk; ++i) {
                    float t = synthetic_phase + (float(i) / float(frames_per_chunk)) * 0.1f;
                    float v = 0.0f;
                    if (config_.device_type == AudioDeviceType::SystemOutput) {
                        // Rhythmic beat simulation
                        float beat = std::pow(std::max(0.0f, std::sin(t * 4.0f)), 8.0f);
                        v = (std::sin(t * 80.0f) * 0.4f + std::sin(t * 220.0f) * 0.3f) * (0.3f + 0.7f * beat);
                    } else {
                        // Voice pitch simulation
                        v = std::sin(t * 120.0f) * 0.25f + std::sin(t * 260.0f) * 0.15f;
                    }
                    synth_buffer[i * target_channels] = v;
                    if (target_channels > 1) {
                        synth_buffer[i * target_channels + 1] = v;
                    }
                }
                synthetic_phase += 0.06f;

                if (callback_ && running_.load()) {
                    callback_(synth_buffer.data(), synth_buffer.size(), static_cast<uint8_t>(target_channels));
                }
            }
        }
    }

    if (client_started && audio_client) {
        audio_client->Stop();
    }
    if (capture_client) {
        capture_client->Release();
    }
    if (mix_format) {
        CoTaskMemFree(mix_format);
    }
    if (audio_client) {
        audio_client->Release();
    }
    if (device) {
        device->Release();
    }
    if (enumerator) {
        enumerator->Release();
    }
    if (com_initialized) {
        CoUninitialize();
    }
}
#endif

} // namespace enki::audio
