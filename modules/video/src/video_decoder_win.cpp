/// @file video_decoder_win.cpp
/// @brief Windows Direct3D 11 / DXVA2 Hardware Zero-Copy Video Decoding and WASAPI Master Clock Implementation.
/// @copyright ENKI Framework — MIT License

#if defined(_WIN32)

#include "video_decoder_win.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <d3d11.h>
#include <dxgi1_2.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/hwcontext.h>
#include <libavutil/hwcontext_d3d11va.h>
#include <libavutil/hwcontext_dxva2.h>
#include <libavutil/pixfmt.h>
}

#include <iostream>
#include <algorithm>
#include <chrono>
#include <thread>

namespace enki::video {

// ════════════════════════════════════════════════════════════════
// Windows Hardware Accelerator Initializer (D3D11VA & DXVA2)
// ════════════════════════════════════════════════════════════════

bool initHardwareDeviceWin(AVBufferRef** out_hw_device_ctx) {
    if (!out_hw_device_ctx) return false;

    // 1. Primary: Modern Direct3D 11 Video Acceleration (D3D11VA)
    int err = av_hwdevice_ctx_create(out_hw_device_ctx, AV_HWDEVICE_TYPE_D3D11VA, nullptr, nullptr, 0);
    if (err >= 0 && *out_hw_device_ctx != nullptr) {
        return true;
    }

    // 2. Secondary fallback: DirectX Video Acceleration 2.0 (DXVA2)
    err = av_hwdevice_ctx_create(out_hw_device_ctx, AV_HWDEVICE_TYPE_DXVA2, nullptr, nullptr, 0);
    if (err >= 0 && *out_hw_device_ctx != nullptr) {
        return true;
    }

    return false;
}

enum AVPixelFormat getHwFormatWin(AVCodecContext* /*ctx*/, const enum AVPixelFormat* pix_fmts) {
    for (const enum AVPixelFormat* p = pix_fmts; *p != AV_PIX_FMT_NONE; ++p) {
        if (*p == AV_PIX_FMT_D3D11 || *p == AV_PIX_FMT_DXVA2_VLD) {
            return *p;
        }
    }
    return AV_PIX_FMT_NONE;
}

void extractD3D11FrameWin(AVFrame* frame, VideoFrame* out_video_frame) {
    if (!frame || !out_video_frame) return;

    if (frame->format == AV_PIX_FMT_D3D11) {
        out_video_frame->is_d3d11 = true;
        auto* tex = reinterpret_cast<ID3D11Texture2D*>(frame->data[0]);
        auto slice = reinterpret_cast<intptr_t>(frame->data[1]);

        out_video_frame->d3d11.texture_ptr = tex;
        out_video_frame->d3d11.subresource = slice;
        out_video_frame->d3d11.width = frame->width;
        out_video_frame->d3d11.height = frame->height;

        if (tex) {
            D3D11_TEXTURE2D_DESC desc{};
            tex->GetDesc(&desc);
            out_video_frame->d3d11.dxgi_format = static_cast<uint32_t>(desc.Format);

            // Query IDXGIResource to acquire cross-API Shared Handle for Zero-Copy rendering
            IDXGIResource* dxgi_res = nullptr;
            if (SUCCEEDED(tex->QueryInterface(__uuidof(IDXGIResource), reinterpret_cast<void**>(&dxgi_res)))) {
                HANDLE shared_h = nullptr;
                if (SUCCEEDED(dxgi_res->GetSharedHandle(&shared_h))) {
                    out_video_frame->d3d11.shared_handle = shared_h;
                }
                dxgi_res->Release();
            }
        }
    }
}

// ════════════════════════════════════════════════════════════════
// Windows WASAPI Audio Engine Implementation
// ════════════════════════════════════════════════════════════════

struct WinAudioPlayer::Impl {
    IMMDeviceEnumerator* enumerator{nullptr};
    IMMDevice*           device{nullptr};
    IAudioClient*        audio_client{nullptr};
    IAudioRenderClient*  render_client{nullptr};
    UINT32               buffer_frame_count{0};
    int                  sample_rate{48000};
    int                  channels{2};
    bool                 is_started{false};
    bool                 com_initialized{false};

    ~Impl() {
        close();
    }

    bool open(int rate, int ch) {
        close();
        sample_rate = rate;
        channels = ch;

        HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        com_initialized = SUCCEEDED(hr);

        hr = CoCreateInstance(
            __uuidof(MMDeviceEnumerator),
            nullptr,
            CLSCTX_ALL,
            __uuidof(IMMDeviceEnumerator),
            reinterpret_cast<void**>(&enumerator)
        );
        if (FAILED(hr) || !enumerator) return false;

        hr = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);
        if (FAILED(hr) || !device) return false;

        hr = device->Activate(
            __uuidof(IAudioClient),
            CLSCTX_ALL,
            nullptr,
            reinterpret_cast<void**>(&audio_client)
        );
        if (FAILED(hr) || !audio_client) return false;

        // Configure standard 16-bit stereo PCM
        WAVEFORMATEX wfx{};
        wfx.wFormatTag      = WAVE_FORMAT_PCM;
        wfx.nChannels       = static_cast<WORD>(channels);
        wfx.nSamplesPerSec  = static_cast<DWORD>(sample_rate);
        wfx.wBitsPerSample  = 16;
        wfx.nBlockAlign     = wfx.nChannels * (wfx.wBitsPerSample / 8);
        wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
        wfx.cbSize          = 0;

        // 150ms buffer target for smooth playback & low latency
        REFERENCE_TIME hns_buffer_duration = 1500000; // 150ms in 100ns units

        hr = audio_client->Initialize(
            AUDCLNT_SHAREMODE_SHARED,
            0,
            hns_buffer_duration,
            0,
            &wfx,
            nullptr
        );
        if (FAILED(hr)) return false;

        hr = audio_client->GetBufferSize(&buffer_frame_count);
        if (FAILED(hr)) return false;

        hr = audio_client->GetService(
            __uuidof(IAudioRenderClient),
            reinterpret_cast<void**>(&render_client)
        );
        if (FAILED(hr) || !render_client) return false;

        hr = audio_client->Start();
        if (SUCCEEDED(hr)) {
            is_started = true;
        }

        return true;
    }

    void write(const int16_t* pcm_data, size_t num_frames) {
        if (!render_client || !audio_client || !pcm_data || num_frames == 0) return;

        size_t frames_left = num_frames;
        const int16_t* src_ptr = pcm_data;

        while (frames_left > 0) {
            UINT32 padding = 0;
            if (FAILED(audio_client->GetCurrentPadding(&padding))) break;

            if (padding >= buffer_frame_count) {
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
                continue;
            }

            UINT32 available = buffer_frame_count - padding;
            UINT32 chunk = static_cast<UINT32>(std::min<size_t>(frames_left, available));
            if (chunk == 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
                continue;
            }

            BYTE* dst_buffer = nullptr;
            HRESULT hr = render_client->GetBuffer(chunk, &dst_buffer);
            if (SUCCEEDED(hr) && dst_buffer) {
                memcpy(dst_buffer, src_ptr, chunk * channels * sizeof(int16_t));
                render_client->ReleaseBuffer(chunk, 0);
                src_ptr += chunk * channels;
                frames_left -= chunk;
            } else {
                break;
            }
        }
    }

    double getLatencySec() const {
        if (!audio_client || sample_rate <= 0) return 0.0;
        UINT32 padding = 0;
        if (SUCCEEDED(audio_client->GetCurrentPadding(&padding))) {
            return static_cast<double>(padding) / static_cast<double>(sample_rate);
        }
        return 0.0;
    }

    void flush() {
        if (audio_client) {
            audio_client->Stop();
            audio_client->Reset();
            if (is_started) {
                audio_client->Start();
            }
        }
    }

    void close() {
        if (audio_client) {
            audio_client->Stop();
        }
        is_started = false;

        if (render_client) {
            render_client->Release();
            render_client = nullptr;
        }
        if (audio_client) {
            audio_client->Release();
            audio_client = nullptr;
        }
        if (device) {
            device->Release();
            device = nullptr;
        }
        if (enumerator) {
            enumerator->Release();
            enumerator = nullptr;
        }
        if (com_initialized) {
            CoUninitialize();
            com_initialized = false;
        }
    }
};

WinAudioPlayer::WinAudioPlayer() : impl_(std::make_unique<Impl>()) {}
WinAudioPlayer::~WinAudioPlayer() = default;

bool WinAudioPlayer::open(int sample_rate, int channels) {
    return impl_ ? impl_->open(sample_rate, channels) : false;
}

void WinAudioPlayer::write(const int16_t* pcm_data, size_t num_frames) {
    if (impl_) impl_->write(pcm_data, num_frames);
}

double WinAudioPlayer::getLatencySec() const {
    return impl_ ? impl_->getLatencySec() : 0.0;
}

void WinAudioPlayer::flush() {
    if (impl_) impl_->flush();
}

void WinAudioPlayer::close() {
    if (impl_) impl_->close();
}

} // namespace enki::video

#endif // _WIN32
