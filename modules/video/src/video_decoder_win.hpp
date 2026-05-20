#pragma once
/// @file video_decoder_win.hpp
/// @brief Windows-specific hardware video decoding (D3D11VA / DXVA2) & WASAPI audio playback engine.
/// @copyright ENKI Framework — MIT License

#if defined(_WIN32)

#include "video/video_types.hpp"

#include <cstdint>
#include <cstddef>
#include <memory>

struct AVBufferRef;
struct AVCodecContext;
struct AVFrame;
extern "C" {
#include <libavutil/pixfmt.h>
}

namespace enki::video {

/// Initialize Direct3D 11 / DXVA2 hardware accelerator device context for FFmpeg
bool initHardwareDeviceWin(AVBufferRef** out_hw_device_ctx);

/// Select hardware accelerated pixel format supported by Windows (D3D11 or DXVA2)
enum AVPixelFormat getHwFormatWin(AVCodecContext* ctx, const enum AVPixelFormat* pix_fmts);

/// Extract Direct3D 11 GPU surface texture and shared handle descriptor for Zero-Copy rendering
void extractD3D11FrameWin(AVFrame* frame, VideoFrame* out_video_frame);

/// High-precision low-latency WASAPI audio playback engine with A/V sync master clock
class WinAudioPlayer {
public:
    WinAudioPlayer();
    ~WinAudioPlayer();

    bool open(int sample_rate, int channels);
    void write(const int16_t* pcm_data, size_t num_frames);
    double getLatencySec() const;
    void flush();
    void close();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace enki::video

#endif // _WIN32
