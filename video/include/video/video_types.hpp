#pragma once
/// @file video_types.hpp
/// @brief Core types, states, and frame descriptors for the ENKI Video Subsystem.
/// @copyright ENKI Framework — MIT License

#include <string>
#include <cstdint>
#include <cstddef>
#include <vector>
#include <memory>

namespace enki::video {

enum class PlaybackState {
    Unloaded,   ///< No media loaded
    Loading,    ///< Opening media source
    Playing,    ///< Active playback
    Paused,     ///< Paused
    Completed,  ///< Reached end of stream
    Error,      ///< Unrecoverable playback error
};

struct VideoMetadata {
    int         width             = 0;
    int         height            = 0;
    double      duration_sec      = 0.0;
    double      fps               = 0.0;
    std::string video_codec       = "";
    std::string audio_codec       = "";
    int         audio_channels    = 0;
    int         audio_sample_rate = 0;
    bool        has_video         = false;
    bool        has_audio         = false;
    bool        hw_accelerated    = false;
};

/// Hardware DRM DMA-BUF descriptor for Zero-Copy GPU rendering
struct DmaBufPlane {
    int      fd       = -1;
    uint32_t stride   = 0;
    uint32_t offset   = 0;
};

struct DmaBufFrame {
    int                      width      = 0;
    int                      height     = 0;
    uint32_t                 drm_format = 0;
    uint64_t                 modifier   = 0;
    std::vector<DmaBufPlane> planes;
};

/// Decoded video frame ready for Skia rendering
struct VideoFrame {
    double               pts_sec   = 0.0;
    int                  width     = 0;
    int                  height    = 0;
    bool                 is_dma_buf = false;
    DmaBufFrame          dma_buf;
    std::vector<uint8_t> rgba_data; ///< Pre-converted RGB32 buffer (fast GPU upload)
};

} // namespace enki::video
