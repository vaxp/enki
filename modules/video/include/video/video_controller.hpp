#pragma once
/// @file video_controller.hpp
/// @brief Thread-safe Playback Controller for the ENKI Video Subsystem.
/// @copyright ENKI Framework — MIT License

#include "video/video_decoder.hpp"
#include <memory>
#include <string>
#include <functional>

namespace enki::video {

class VideoController {
public:
    VideoController();
    explicit VideoController(const std::string& source);
    ~VideoController();

    /// Load media source
    bool open(const std::string& source);
    void close();

    /// Playback transport controls
    void play();
    void pause();
    void togglePlay();
    void stop();
    void seek(double seconds);

    /// Volume & Audio Controls
    void setVolume(float volume);
    [[nodiscard]] float getVolume() const { return volume_; }
    void setMuted(bool muted);
    [[nodiscard]] bool isMuted() const { return is_muted_; }

    /// Playback Speed & Looping
    void setPlaybackSpeed(float speed);
    [[nodiscard]] float getPlaybackSpeed() const { return speed_; }
    void setLooping(bool looping);
    [[nodiscard]] bool isLooping() const { return looping_; }

    /// State & Timestamps
    [[nodiscard]] PlaybackState getState() const;
    [[nodiscard]] bool isPlaying() const;
    [[nodiscard]] double getCurrentPosition() const;
    [[nodiscard]] double getDuration() const;
    [[nodiscard]] VideoMetadata getMetadata() const;

    /// Retrieve the synchronized frame for the current frame presentation
    std::shared_ptr<VideoFrame> getNextRenderFrame();

private:
    std::unique_ptr<VideoDecoder> decoder_;
    float  volume_{1.0f};
    float  previous_volume_{1.0f};
    bool   is_muted_{false};
    float  speed_{1.0f};
    bool   looping_{false};
};

} // namespace enki::video
