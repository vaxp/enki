#pragma once
/// @file video_decoder.hpp
/// @brief Multi-threaded FFmpeg Demuxer & Decoder with Hardware Zero-Copy & PulseAudio Master Clock.
/// @copyright ENKI Framework — MIT License

#include "video/video_types.hpp"

#include <string>
#include <vector>
#include <deque>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>

struct AVFormatContext;
struct AVCodecContext;
struct AVBufferRef;
struct SwsContext;
struct SwrContext;
struct AVPacket;
struct AVFrame;
struct pa_simple;

namespace enki::video {

class VideoDecoder {
public:
    VideoDecoder();
    ~VideoDecoder();

    /// Open local file or network stream
    bool open(const std::string& path_or_url);
    void close();

    /// Playback control
    void play();
    void pause();
    void seek(double target_seconds);
    void setVolume(float volume);
    void setPlaybackSpeed(float speed);
    void setLooping(bool loop);

    [[nodiscard]] PlaybackState getState() const { return state_.load(); }
    [[nodiscard]] VideoMetadata getMetadata() const;
    [[nodiscard]] double        getCurrentPosition() const;
    [[nodiscard]] double        getDuration() const;

    /// Retrieve the current synchronized video frame
    std::shared_ptr<VideoFrame> getNextRenderFrame();

private:
    // Worker threads
    void demuxerLoop();
    void videoDecodeLoop();
    void audioDecodeLoop();

    // Hardware acceleration initialization
    bool initHardwareDevice();

    // Master audio clock helper
    void updateAudioClock(double pts, size_t bytes_played);
    double getMasterClock() const;

    // Internal clean-up
    void flushQueues();

    std::string source_;
    std::atomic<PlaybackState> state_{PlaybackState::Unloaded};
    std::atomic<bool>          running_{false};
    std::atomic<bool>          is_seeking_{false};
    std::atomic<double>        seek_target_{0.0};
    std::atomic<bool>          seek_requested_{false};
    std::atomic<double>        requested_seek_pos_{0.0};
    std::atomic<float>         volume_{1.0f};
    std::atomic<float>         playback_speed_{1.0f};
    std::atomic<bool>          looping_{false};

    // Metadata
    VideoMetadata metadata_;
    mutable std::mutex metadata_mutex_;

    // FFmpeg contexts
    AVFormatContext* format_ctx_{nullptr};
    AVCodecContext*  video_codec_ctx_{nullptr};
    AVCodecContext*  audio_codec_ctx_{nullptr};
    std::mutex       video_codec_mutex_;
    std::mutex       audio_codec_mutex_;
    AVBufferRef*     hw_device_ctx_{nullptr};
    SwsContext*      sws_ctx_{nullptr};
    SwrContext*      swr_ctx_{nullptr};
    int              video_stream_idx_{-1};
    int              audio_stream_idx_{-1};
    double           video_time_base_{0.0};
    double           audio_time_base_{0.0};

    // PulseAudio playback
    pa_simple*       pa_playback_{nullptr};

    // Synchronization Clocks
    std::atomic<double> audio_clock_{0.0};
    std::chrono::steady_clock::time_point play_start_time_;
    double monotonic_clock_offset_{0.0};

    // Threads
    std::unique_ptr<std::thread> demux_thread_;
    std::unique_ptr<std::thread> video_thread_;
    std::unique_ptr<std::thread> audio_thread_;

    // Packet Queues
    std::deque<AVPacket*>   video_packets_;
    std::deque<AVPacket*>   audio_packets_;
    mutable std::mutex      packet_mutex_;
    std::condition_variable packet_cv_;

    // Decoded Frame Ring Buffer
    std::deque<std::shared_ptr<VideoFrame>> frame_queue_;
    std::shared_ptr<VideoFrame>             last_displayed_frame_;
    mutable std::mutex                      frame_mutex_;
    std::condition_variable                 frame_cv_;

    static constexpr size_t kMaxPacketQueue = 90;
    static constexpr size_t kMaxFrameQueue  = 16;
};

} // namespace enki::video
