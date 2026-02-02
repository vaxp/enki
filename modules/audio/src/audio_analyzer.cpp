/// @file audio_analyzer.cpp
/// @brief Pure C++20 Fast Fourier Transform (FFT) and DSP Audio Analyzer.
/// @copyright ENKI Framework — MIT License

#include "audio/audio_analyzer.hpp"
#include <cmath>
#include <numeric>

namespace enki::audio {

namespace {

constexpr float kPi = 3.14159265358979323846f;

/// Fast bit-reversal for Radix-2 FFT
size_t reverseBits(size_t val, size_t bits) {
    size_t result = 0;
    for (size_t i = 0; i < bits; ++i) {
        result = (result << 1) | (val & 1);
        val >>= 1;
    }
    return result;
}

} // namespace

AudioAnalyzer::AudioAnalyzer(size_t fft_size)
    : fft_size_(fft_size),
      time_buffer_(fft_size, 0.0f),
      window_(fft_size, 0.0f),
      real_(fft_size, 0.0f),
      imag_(fft_size, 0.0f),
      magnitudes_(fft_size / 2, 0.0f) {
    // Precompute Hann window to suppress spectral leakage
    for (size_t i = 0; i < fft_size_; ++i) {
        window_[i] = 0.5f * (1.0f - std::cos((2.0f * kPi * float(i)) / float(fft_size_ - 1)));
    }
}

void AudioAnalyzer::reset() {
    std::fill(time_buffer_.begin(), time_buffer_.end(), 0.0f);
    std::fill(real_.begin(), real_.end(), 0.0f);
    std::fill(imag_.begin(), imag_.end(), 0.0f);
    std::fill(magnitudes_.begin(), magnitudes_.end(), 0.0f);
    previous_bands_.clear();
    rms_volume_ = 0.0f;
    bass_energy_ = 0.0f;
    rolling_max_ = 0.08f;
}

void AudioAnalyzer::processSamples(const float* samples, size_t count, uint8_t channels) {
    if (!samples || count == 0) return;

    // 1. Calculate RMS Volume & Convert to Mono time buffer
    float sum_sq = 0.0f;
    size_t frames = count / channels;

    if (frames >= fft_size_) {
        // Take latest fft_size_ frames
        size_t start_frame = frames - fft_size_;
        for (size_t i = 0; i < fft_size_; ++i) {
            float mono = 0.0f;
            for (uint8_t c = 0; c < channels; ++c) {
                mono += samples[(start_frame + i) * channels + c];
            }
            mono /= float(channels);
            time_buffer_[i] = mono;
            sum_sq += mono * mono;
        }
    } else {
        // Shift old samples left and append new ones
        size_t keep = fft_size_ - frames;
        std::copy(time_buffer_.begin() + frames, time_buffer_.end(), time_buffer_.begin());
        for (size_t i = 0; i < frames; ++i) {
            float mono = 0.0f;
            for (uint8_t c = 0; c < channels; ++c) {
                mono += samples[i * channels + c];
            }
            mono /= float(channels);
            time_buffer_[keep + i] = mono;
            sum_sq += mono * mono;
        }
    }

    rms_volume_ = std::min(1.0f, std::sqrt(sum_sq / float(fft_size_)) * 2.5f);

    // 2. Perform Radix-2 Cooley-Tukey FFT
    computeFFT();

    // 3. Compute Bass Energy (bins 1 to 6, ~20Hz to 180Hz)
    float bass_sum = 0.0f;
    for (size_t i = 1; i <= 6 && i < magnitudes_.size(); ++i) {
        bass_sum += magnitudes_[i];
    }
    bass_energy_ = std::clamp(bass_sum * 12.0f, 0.0f, 1.0f);
}

void AudioAnalyzer::computeFFT() {
    const size_t n = fft_size_;
    const size_t bits = static_cast<size_t>(std::log2(n));

    // Apply Hann window and bit-reversal permutation
    for (size_t i = 0; i < n; ++i) {
        size_t rev = reverseBits(i, bits);
        real_[rev] = time_buffer_[i] * window_[i];
        imag_[rev] = 0.0f;
    }

    // Cooley-Tukey iterative Radix-2
    for (size_t s = 1; s <= bits; ++s) {
        size_t m = 1 << s;
        float theta = -2.0f * kPi / float(m);
        float w_step_r = std::cos(theta);
        float w_step_i = std::sin(theta);

        for (size_t k = 0; k < n; k += m) {
            float wr = 1.0f;
            float wi = 0.0f;
            for (size_t j = 0; j < m / 2; ++j) {
                size_t u_idx = k + j;
                size_t v_idx = k + j + m / 2;

                float tr = wr * real_[v_idx] - wi * imag_[v_idx];
                float ti = wr * imag_[v_idx] + wi * real_[v_idx];

                real_[v_idx] = real_[u_idx] - tr;
                imag_[v_idx] = imag_[u_idx] - ti;
                real_[u_idx] = real_[u_idx] + tr;
                imag_[u_idx] = imag_[u_idx] + ti;

                float next_wr = wr * w_step_r - wi * w_step_i;
                wi = wr * w_step_i + wi * w_step_r;
                wr = next_wr;
            }
        }
    }

    // Compute magnitude spectrum (first N/2 bins)
    const size_t half = n / 2;
    for (size_t i = 0; i < half; ++i) {
        float r = real_[i];
        float im = imag_[i];
        magnitudes_[i] = std::sqrt(r * r + im * im) / float(half);
    }
}

std::vector<float> AudioAnalyzer::getFrequencyBands(size_t num_bands, float sensitivity) {
    if (num_bands == 0) return {};
    std::vector<float> bands(num_bands, 0.0f);

    const size_t half = fft_size_ / 2;

    float current_max = 0.001f;

    // Group bins logarithmically with perceptual frequency compensation
    for (size_t b = 0; b < num_bands; ++b) {
        float norm_start = std::pow(float(b) / float(num_bands), 2.2f);
        float norm_end   = std::pow(float(b + 1) / float(num_bands), 2.2f);

        size_t start_bin = std::clamp(static_cast<size_t>(norm_start * float(half)), size_t(1), half - 1);
        size_t end_bin   = std::clamp(static_cast<size_t>(norm_end * float(half)), start_bin + 1, half);

        float sum = 0.0f;
        for (size_t i = start_bin; i < end_bin; ++i) {
            sum += magnitudes_[i];
        }

        float avg = sum / float(end_bin - start_bin);

        // Perceptual equal-loudness weighting (slight treble lift for visual richness)
        float freq_ratio = float(b) / float(num_bands);
        float treble_boost = 1.0f + std::pow(freq_ratio, 1.4f) * 6.5f;

        float raw_val = avg * treble_boost;
        if (raw_val > current_max) {
            current_max = raw_val;
        }

        bands[b] = raw_val;
    }

    // Adaptive Gain Control (slowly decay rolling max for natural responsiveness)
    rolling_max_ = std::max(current_max, rolling_max_ * 0.995f);
    rolling_max_ = std::max(0.035f, rolling_max_);

    for (size_t b = 0; b < num_bands; ++b) {
        float norm_val = (bands[b] / rolling_max_) * sensitivity;
        bands[b] = std::clamp(norm_val, 0.0f, 1.0f);
    }

    return bands;
}

std::vector<float> AudioAnalyzer::getDecayedBands(size_t num_bands, float decay_speed, float sensitivity) {
    std::vector<float> target = getFrequencyBands(num_bands, sensitivity);

    if (previous_bands_.size() != num_bands) {
        previous_bands_ = target;
        return target;
    }

    for (size_t i = 0; i < num_bands; ++i) {
        if (target[i] >= previous_bands_[i]) {
            // Instant attack
            previous_bands_[i] = target[i];
        } else {
            // Smooth gravity falloff
            previous_bands_[i] = std::max(0.0f, previous_bands_[i] - decay_speed);
        }
    }

    return previous_bands_;
}

std::vector<float> AudioAnalyzer::getWaveformSamples(size_t target_count) {
    if (target_count == 0) return {};
    std::vector<float> result(target_count, 0.0f);

    const size_t n = time_buffer_.size();
    for (size_t i = 0; i < target_count; ++i) {
        size_t idx = (i * n) / target_count;
        result[i] = std::clamp(time_buffer_[idx], -1.0f, 1.0f);
    }

    return result;
}

} // namespace enki::audio
