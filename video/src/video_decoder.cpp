/// @file video_decoder.cpp
/// @brief Multi-threaded FFmpeg Demuxer, Hardware Zero-Copy & PulseAudio Master Clock Implementation.
/// @copyright ENKI Framework — MIT License

#include "video/video_decoder.hpp"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavutil/hwcontext.h>
#include <libavutil/hwcontext_drm.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

#include <pulse/simple.h>
#include <pulse/error.h>

#include <iostream>
#include <algorithm>
#include <cmath>

namespace enki::video {

namespace {

enum AVPixelFormat getHwFormat(AVCodecContext* ctx, const enum AVPixelFormat* pix_fmts) {
    for (const enum AVPixelFormat* p = pix_fmts; *p != AV_PIX_FMT_NONE; ++p) {
        if (*p == AV_PIX_FMT_VAAPI) {
            return *p;
        }
    }
    return AV_PIX_FMT_NONE;
}

} // namespace

VideoDecoder::VideoDecoder() {
    av_log_set_level(AV_LOG_QUIET);
}

VideoDecoder::~VideoDecoder() {
    close();
}

bool VideoDecoder::initHardwareDevice() {
    // Attempt VA-API hardware acceleration via Linux DRI render device
    int err = av_hwdevice_ctx_create(&hw_device_ctx_, AV_HWDEVICE_TYPE_VAAPI, "/dev/dri/renderD128", nullptr, 0);
    if (err < 0) {
        // Fallback without device path
        err = av_hwdevice_ctx_create(&hw_device_ctx_, AV_HWDEVICE_TYPE_VAAPI, nullptr, nullptr, 0);
    }
    return (err >= 0 && hw_device_ctx_ != nullptr);
}

bool VideoDecoder::open(const std::string& path_or_url) {
    close();

    source_ = path_or_url;
    state_  = PlaybackState::Loading;

    // 1. Open input format
    if (avformat_open_input(&format_ctx_, path_or_url.c_str(), nullptr, nullptr) < 0) {
        std::cerr << "[ENKI Video] Failed to open media: " << path_or_url << "\n";
        state_ = PlaybackState::Error;
        return false;
    }

    if (avformat_find_stream_info(format_ctx_, nullptr) < 0) {
        std::cerr << "[ENKI Video] Failed to find stream info.\n";
        close();
        state_ = PlaybackState::Error;
        return false;
    }

    // 2. Discover video and audio streams
    video_stream_idx_ = av_find_best_stream(format_ctx_, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    audio_stream_idx_ = av_find_best_stream(format_ctx_, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);

    bool hw_ready = initHardwareDevice();

    // 3. Initialize Video Codec
    if (video_stream_idx_ >= 0) {
        AVStream* stream = format_ctx_->streams[video_stream_idx_];
        video_time_base_ = av_q2d(stream->time_base);

        const AVCodec* dec = avcodec_find_decoder(stream->codecpar->codec_id);
        if (dec) {
            video_codec_ctx_ = avcodec_alloc_context3(dec);
            avcodec_parameters_to_context(video_codec_ctx_, stream->codecpar);

            if (hw_ready) {
                video_codec_ctx_->hw_device_ctx = av_buffer_ref(hw_device_ctx_);
                video_codec_ctx_->get_format = getHwFormat;
            }

            if (avcodec_open2(video_codec_ctx_, dec, nullptr) >= 0) {
                std::lock_guard<std::mutex> lock(metadata_mutex_);
                metadata_.has_video = true;
                metadata_.width = stream->codecpar->width;
                metadata_.height = stream->codecpar->height;
                metadata_.video_codec = dec->name ? dec->name : "unknown";
                metadata_.hw_accelerated = hw_ready;
                if (stream->r_frame_rate.den > 0) {
                    metadata_.fps = av_q2d(stream->r_frame_rate);
                } else if (stream->avg_frame_rate.den > 0) {
                    metadata_.fps = av_q2d(stream->avg_frame_rate);
                } else {
                    metadata_.fps = 30.0;
                }
            }
        }
    }

    // 4. Initialize Audio Codec & PulseAudio
    if (audio_stream_idx_ >= 0) {
        AVStream* stream = format_ctx_->streams[audio_stream_idx_];
        audio_time_base_ = av_q2d(stream->time_base);

        const AVCodec* dec = avcodec_find_decoder(stream->codecpar->codec_id);
        if (dec) {
            audio_codec_ctx_ = avcodec_alloc_context3(dec);
            avcodec_parameters_to_context(audio_codec_ctx_, stream->codecpar);

            if (avcodec_open2(audio_codec_ctx_, dec, nullptr) >= 0) {
                std::lock_guard<std::mutex> lock(metadata_mutex_);
                metadata_.has_audio = true;
                metadata_.audio_codec = dec->name ? dec->name : "unknown";
                metadata_.audio_channels = audio_codec_ctx_->ch_layout.nb_channels;
                metadata_.audio_sample_rate = audio_codec_ctx_->sample_rate;

                // Setup PulseAudio playback (48kHz Stereo S16LE)
                pa_sample_spec ss;
                ss.format   = PA_SAMPLE_S16LE;
                ss.rate     = 48000;
                ss.channels = 2;

                pa_buffer_attr ba;
                ba.maxlength = 48000 * 2 * sizeof(int16_t); // ~1 sec buffer max
                ba.tlength   = 4800 * 2 * sizeof(int16_t);  // ~100ms latency target
                ba.prebuf    = static_cast<uint32_t>(-1);
                ba.minreq    = static_cast<uint32_t>(-1);
                ba.fragsize  = static_cast<uint32_t>(-1);

                int error = 0;
                pa_playback_ = pa_simple_new(
                    nullptr,
                    "ENKI Video Player",
                    PA_STREAM_PLAYBACK,
                    nullptr,
                    "Video Soundtrack",
                    &ss,
                    nullptr,
                    &ba,
                    &error
                );

                // Setup SwrContext resampler to 48000Hz stereo s16
                AVChannelLayout out_ch_layout;
                av_channel_layout_default(&out_ch_layout, 2);

                swr_alloc_set_opts2(
                    &swr_ctx_,
                    &out_ch_layout,
                    AV_SAMPLE_FMT_S16,
                    48000,
                    &audio_codec_ctx_->ch_layout,
                    audio_codec_ctx_->sample_fmt,
                    audio_codec_ctx_->sample_rate,
                    0,
                    nullptr
                );
                if (swr_ctx_) {
                    swr_init(swr_ctx_);
                }
            }
        }
    }

    if (format_ctx_->duration != AV_NOPTS_VALUE) {
        std::lock_guard<std::mutex> lock(metadata_mutex_);
        metadata_.duration_sec = double(format_ctx_->duration) / double(AV_TIME_BASE);
    }

    // Start background threads
    running_ = true;
    demux_thread_ = std::make_unique<std::thread>(&VideoDecoder::demuxerLoop, this);
    if (metadata_.has_video) {
        video_thread_ = std::make_unique<std::thread>(&VideoDecoder::videoDecodeLoop, this);
    }
    if (metadata_.has_audio) {
        audio_thread_ = std::make_unique<std::thread>(&VideoDecoder::audioDecodeLoop, this);
    }

    state_ = PlaybackState::Paused;
    play_start_time_ = std::chrono::steady_clock::now();
    monotonic_clock_offset_ = 0.0;

    return true;
}

void VideoDecoder::close() {
    running_ = false;
    state_ = PlaybackState::Unloaded;

    packet_cv_.notify_all();
    frame_cv_.notify_all();

    if (demux_thread_ && demux_thread_->joinable()) demux_thread_->join();
    if (video_thread_ && video_thread_->joinable()) video_thread_->join();
    if (audio_thread_ && audio_thread_->joinable()) audio_thread_->join();

    demux_thread_.reset();
    video_thread_.reset();
    audio_thread_.reset();

    flushQueues();

    if (pa_playback_) {
        pa_simple_free(pa_playback_);
        pa_playback_ = nullptr;
    }

    if (swr_ctx_) {
        swr_free(&swr_ctx_);
        swr_ctx_ = nullptr;
    }
    if (sws_ctx_) {
        sws_freeContext(sws_ctx_);
        sws_ctx_ = nullptr;
    }

    if (video_codec_ctx_) {
        avcodec_free_context(&video_codec_ctx_);
        video_codec_ctx_ = nullptr;
    }
    if (audio_codec_ctx_) {
        avcodec_free_context(&audio_codec_ctx_);
        audio_codec_ctx_ = nullptr;
    }
    if (hw_device_ctx_) {
        av_buffer_unref(&hw_device_ctx_);
        hw_device_ctx_ = nullptr;
    }
    if (format_ctx_) {
        avformat_close_input(&format_ctx_);
        format_ctx_ = nullptr;
    }

    {
        std::lock_guard<std::mutex> lock(metadata_mutex_);
        metadata_ = VideoMetadata{};
    }
    last_displayed_frame_.reset();
}

void VideoDecoder::flushQueues() {
    {
        std::lock_guard<std::mutex> lock(packet_mutex_);
        for (auto* pkt : video_packets_) av_packet_free(&pkt);
        video_packets_.clear();
        for (auto* pkt : audio_packets_) av_packet_free(&pkt);
        audio_packets_.clear();
    }
    {
        std::lock_guard<std::mutex> lock(frame_mutex_);
        frame_queue_.clear();
    }
}

void VideoDecoder::play() {
    if (state_ == PlaybackState::Paused || state_ == PlaybackState::Completed) {
        if (state_ == PlaybackState::Completed) {
            seek(0.0);
        }
        play_start_time_ = std::chrono::steady_clock::now();
        state_ = PlaybackState::Playing;
    }
}

void VideoDecoder::pause() {
    if (state_ == PlaybackState::Playing) {
        monotonic_clock_offset_ = getMasterClock();
        state_ = PlaybackState::Paused;
    }
}

void VideoDecoder::seek(double target_seconds) {
    if (!format_ctx_) return;

    requested_seek_pos_ = std::clamp(target_seconds, 0.0, std::max(0.0, metadata_.duration_sec));
    seek_requested_ = true;
    is_seeking_ = true;
    packet_cv_.notify_all();
    frame_cv_.notify_all();
}

void VideoDecoder::setVolume(float volume) {
    volume_ = std::clamp(volume, 0.0f, 1.0f);
}

void VideoDecoder::setPlaybackSpeed(float speed) {
    playback_speed_ = std::clamp(speed, 0.25f, 3.0f);
}

void VideoDecoder::setLooping(bool loop) {
    looping_ = loop;
}

VideoMetadata VideoDecoder::getMetadata() const {
    std::lock_guard<std::mutex> lock(metadata_mutex_);
    return metadata_;
}

double VideoDecoder::getCurrentPosition() const {
    return getMasterClock();
}

double VideoDecoder::getDuration() const {
    std::lock_guard<std::mutex> lock(metadata_mutex_);
    return metadata_.duration_sec;
}

double VideoDecoder::getMasterClock() const {
    if (metadata_.has_audio && pa_playback_ && audio_clock_.load() > 0.0) {
        int error = 0;
        pa_usec_t latency = pa_simple_get_latency(pa_playback_, &error);
        double lat_sec = (error == 0 && latency > 0) ? (double(latency) / 1000000.0) : 0.0;
        double speed = playback_speed_.load();
        double current = audio_clock_.load() - (lat_sec * speed);
        return std::max(0.0, current);
    }
    // Fallback: steady monotonic clock
    if (state_ == PlaybackState::Playing) {
        auto now = std::chrono::steady_clock::now();
        double elapsed = std::chrono::duration<double>(now - play_start_time_).count();
        return monotonic_clock_offset_ + elapsed * playback_speed_.load();
    }
    return monotonic_clock_offset_;
}

// ════════════════════════════════════════════════════════════════
// Demuxer Thread Loop
// ════════════════════════════════════════════════════════════════
void VideoDecoder::demuxerLoop() {
    AVPacket* packet = av_packet_alloc();

    while (running_) {
        if (seek_requested_.exchange(false)) {
            double target = requested_seek_pos_.load();
            int64_t seek_ts = static_cast<int64_t>(target * AV_TIME_BASE);
            av_seek_frame(format_ctx_, -1, seek_ts, AVSEEK_FLAG_BACKWARD);

            {
                std::lock_guard<std::mutex> lock(video_codec_mutex_);
                if (video_codec_ctx_) avcodec_flush_buffers(video_codec_ctx_);
            }
            {
                std::lock_guard<std::mutex> lock(audio_codec_mutex_);
                if (audio_codec_ctx_) avcodec_flush_buffers(audio_codec_ctx_);
            }

            if (pa_playback_) {
                int error = 0;
                pa_simple_flush(pa_playback_, &error);
            }

            flushQueues();

            audio_clock_ = target;
            monotonic_clock_offset_ = target;
            play_start_time_ = std::chrono::steady_clock::now();
            is_seeking_ = false;
        }

        if (is_seeking_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            continue;
        }

        // Throttle demuxer if queues are saturated
        {
            std::unique_lock<std::mutex> lock(packet_mutex_);
            if (video_packets_.size() >= kMaxPacketQueue || audio_packets_.size() >= kMaxPacketQueue) {
                packet_cv_.wait_for(lock, std::chrono::milliseconds(10));
                continue;
            }
        }

        int ret = av_read_frame(format_ctx_, packet);
        if (ret < 0) {
            // End of stream reached
            if (looping_) {
                seek(0.0);
                continue;
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
                continue;
            }
        }

        if (packet->stream_index == video_stream_idx_) {
            AVPacket* clone = av_packet_clone(packet);
            std::lock_guard<std::mutex> lock(packet_mutex_);
            video_packets_.push_back(clone);
            packet_cv_.notify_one();
        } else if (packet->stream_index == audio_stream_idx_) {
            AVPacket* clone = av_packet_clone(packet);
            std::lock_guard<std::mutex> lock(packet_mutex_);
            audio_packets_.push_back(clone);
            packet_cv_.notify_one();
        }

        av_packet_unref(packet);
    }

    av_packet_free(&packet);
}

// ════════════════════════════════════════════════════════════════
// Video Decoder Thread Loop (Zero-Copy Frame Extraction)
// ════════════════════════════════════════════════════════════════
void VideoDecoder::videoDecodeLoop() {
    AVFrame* frame = av_frame_alloc();
    AVFrame* sw_frame = av_frame_alloc();

    while (running_) {
        if (is_seeking_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            continue;
        }

        // Limit decoded frame queue
        {
            std::unique_lock<std::mutex> lock(frame_mutex_);
            if (frame_queue_.size() >= kMaxFrameQueue) {
                frame_cv_.wait_for(lock, std::chrono::milliseconds(10));
                continue;
            }
        }

        AVPacket* packet = nullptr;
        {
            std::unique_lock<std::mutex> lock(packet_mutex_);
            if (video_packets_.empty()) {
                packet_cv_.wait_for(lock, std::chrono::milliseconds(10));
                continue;
            }
            packet = video_packets_.front();
            video_packets_.pop_front();
        }

        {
            std::lock_guard<std::mutex> codec_lock(video_codec_mutex_);
            if (!video_codec_ctx_ || avcodec_send_packet(video_codec_ctx_, packet) != 0) {
                av_packet_free(&packet);
                continue;
            }

            while (avcodec_receive_frame(video_codec_ctx_, frame) == 0) {
                auto video_frame = std::make_shared<VideoFrame>();

                // Calculate presentation timestamp
                double pts = 0.0;
                if (frame->best_effort_timestamp != AV_NOPTS_VALUE) {
                    pts = double(frame->best_effort_timestamp) * video_time_base_;
                } else if (frame->pts != AV_NOPTS_VALUE) {
                    pts = double(frame->pts) * video_time_base_;
                }
                video_frame->pts_sec = pts;
                video_frame->width = frame->width;
                video_frame->height = frame->height;

                AVFrame* render_src = frame;

                // Handle Hardware VA-API frame
                if (frame->format == AV_PIX_FMT_VAAPI) {
                    if (av_hwframe_transfer_data(sw_frame, frame, 0) >= 0) {
                        render_src = sw_frame;
                    }
                }

                // Always convert render_src to RGBA32 for Skia GPU presentation
                sws_ctx_ = sws_getCachedContext(
                    sws_ctx_,
                    render_src->width, render_src->height, static_cast<AVPixelFormat>(render_src->format),
                    render_src->width, render_src->height, AV_PIX_FMT_RGBA,
                    SWS_FAST_BILINEAR, nullptr, nullptr, nullptr
                );

                if (sws_ctx_) {
                    video_frame->rgba_data.resize(render_src->width * render_src->height * 4);
                    uint8_t* dst_slices[4] = { video_frame->rgba_data.data(), nullptr, nullptr, nullptr };
                    int dst_strides[4] = { render_src->width * 4, 0, 0, 0 };

                    sws_scale(
                        sws_ctx_,
                        render_src->data, render_src->linesize,
                        0, render_src->height,
                        dst_slices, dst_strides
                    );
                }

                {
                    std::lock_guard<std::mutex> lock(frame_mutex_);
                    frame_queue_.push_back(video_frame);
                    frame_cv_.notify_one();
                }

                av_frame_unref(sw_frame);
            }
        }

        av_packet_free(&packet);
    }

    av_frame_free(&sw_frame);
    av_frame_free(&frame);
}

// ════════════════════════════════════════════════════════════════
// Audio Decoder Thread Loop (PulseAudio Sound Stream)
// ════════════════════════════════════════════════════════════════
void VideoDecoder::audioDecodeLoop() {
    AVFrame* frame = av_frame_alloc();
    std::vector<int16_t> pcm_out;

    while (running_) {
        if (is_seeking_ || state_ != PlaybackState::Playing) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        AVPacket* packet = nullptr;
        {
            std::unique_lock<std::mutex> lock(packet_mutex_);
            if (audio_packets_.empty()) {
                packet_cv_.wait_for(lock, std::chrono::milliseconds(10));
                continue;
            }
            packet = audio_packets_.front();
            audio_packets_.pop_front();
        }

        {
            std::lock_guard<std::mutex> codec_lock(audio_codec_mutex_);
            if (!audio_codec_ctx_ || avcodec_send_packet(audio_codec_ctx_, packet) != 0) {
                av_packet_free(&packet);
                continue;
            }

            while (avcodec_receive_frame(audio_codec_ctx_, frame) == 0) {
                if (swr_ctx_ && pa_playback_) {
                    int out_samples = swr_get_out_samples(swr_ctx_, frame->nb_samples);
                    if (out_samples <= 0) continue;

                    pcm_out.resize(out_samples * 2);

                    uint8_t* out_data[1] = { reinterpret_cast<uint8_t*>(pcm_out.data()) };
                    int converted = swr_convert(
                        swr_ctx_,
                        out_data,
                        out_samples,
                        const_cast<const uint8_t**>(frame->data),
                        frame->nb_samples
                    );

                    if (converted > 0) {
                        float speed = playback_speed_.load();
                        int play_samples = converted;

                        // Linear interpolation PCM resampler for playback speed (0.25x - 3.0x)
                        std::vector<int16_t> speed_pcm;
                        if (std::abs(speed - 1.0f) > 0.02f && speed > 0.1f) {
                            int out_frames = std::max(1, static_cast<int>(float(converted) / speed));
                            speed_pcm.resize(out_frames * 2);

                            for (int i = 0; i < out_frames; ++i) {
                                float src_f = float(i) * speed;
                                int idx0 = static_cast<int>(src_f);
                                int idx1 = std::min(idx0 + 1, converted - 1);
                                float frac = src_f - float(idx0);

                                float l0 = float(pcm_out[idx0 * 2]);
                                float l1 = float(pcm_out[idx1 * 2]);
                                speed_pcm[i * 2] = static_cast<int16_t>(l0 + frac * (l1 - l0));

                                float r0 = float(pcm_out[idx0 * 2 + 1]);
                                float r1 = float(pcm_out[idx1 * 2 + 1]);
                                speed_pcm[i * 2 + 1] = static_cast<int16_t>(r0 + frac * (r1 - r0));
                            }
                            play_samples = out_frames;
                        }

                        int16_t* write_ptr = speed_pcm.empty() ? pcm_out.data() : speed_pcm.data();

                        // Apply volume attenuation
                        float vol = volume_.load();
                        if (vol < 0.99f) {
                            for (int i = 0; i < play_samples * 2; ++i) {
                                write_ptr[i] = static_cast<int16_t>(float(write_ptr[i]) * vol);
                            }
                        }

                        int error = 0;
                        pa_simple_write(
                            pa_playback_,
                            write_ptr,
                            play_samples * 2 * sizeof(int16_t),
                            &error
                        );

                        // Update Master Audio Clock
                        double pts = 0.0;
                        if (frame->best_effort_timestamp != AV_NOPTS_VALUE) {
                            pts = double(frame->best_effort_timestamp) * audio_time_base_;
                        } else if (frame->pts != AV_NOPTS_VALUE) {
                            pts = double(frame->pts) * audio_time_base_;
                        }
                        audio_clock_ = pts;
                    }
                }
            }
        }

        av_packet_free(&packet);
    }

    av_frame_free(&frame);
}

std::shared_ptr<VideoFrame> VideoDecoder::getNextRenderFrame() {
    std::lock_guard<std::mutex> lock(frame_mutex_);
    if (frame_queue_.empty()) {
        return last_displayed_frame_;
    }

    if (state_ != PlaybackState::Playing) {
        return last_displayed_frame_ ? last_displayed_frame_ : frame_queue_.front();
    }

    double current_clock = getMasterClock();

    // Check if the next queued frame is ready to display
    while (!frame_queue_.empty()) {
        auto frame = frame_queue_.front();
        if (frame->pts_sec <= current_clock) {
            last_displayed_frame_ = frame;
            frame_queue_.pop_front();
            frame_cv_.notify_one();

            // Frame skipping check: only drop if severely behind (>150ms)
            if (!frame_queue_.empty() && frame_queue_.front()->pts_sec < current_clock - 0.150) {
                frame_queue_.pop_front();
                frame_cv_.notify_one();
            }
        } else {
            break;
        }
    }

    return last_displayed_frame_ ? last_displayed_frame_ : frame_queue_.front();
}

} // namespace enki::video
