#pragma once
/// @file audio_analyzer.hpp
/// @brief Digital Signal Processing (DSP) and Fast Fourier Transform (FFT) Analyzer.
///
/// Features:
///   - 100% C++20 pure Radix-2 Cooley-Tukey FFT implementation
///   - Hann windowing for spectral leakage suppression
///   - Perceptual equal-loudness logarithmic frequency binning
///   - Dynamic Adaptive Gain Control (AGC) for balanced visual activity
///   - Smooth gravity decay physics for Hi-Fi equalizer visualization
///   - Stereo separation and RMS volume metering
///   - Beat / Bass energy tracking for pulsing radial and ambient effects
///
/// @copyright ENKI Framework — MIT License

#include <vector>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <algorithm>

namespace enki::audio {

class AudioAnalyzer {
public:
    explicit AudioAnalyzer(size_t fft_size = 512);
    ~AudioAnalyzer() = default;

    /// Feed raw PCM float samples (interleaved stereo or mono)
    void processSamples(const float* samples, size_t count, uint8_t channels = 2);

    /// Compute logarithmic frequency bands normalized [0.0 .. 1.0]
    std::vector<float> getFrequencyBands(size_t num_bands, float sensitivity = 1.0f);

    /// Compute smoothly decaying frequency bands (physics falloff)
    std::vector<float> getDecayedBands(size_t num_bands, float decay_speed = 0.06f, float sensitivity = 1.0f);

    /// Get time-domain waveform amplitudes for voice / scrolling
    std::vector<float> getWaveformSamples(size_t target_count);

    /// Current root-mean-square volume [0.0 .. 1.0]
    [[nodiscard]] float getRmsVolume() const { return rms_volume_; }

    /// Current bass / beat energy [0.0 .. 1.0]
    [[nodiscard]] float getBassEnergy() const { return bass_energy_; }

    /// Reset internal buffers
    void reset();

private:
    void computeFFT();

    size_t              fft_size_;
    std::vector<float>  time_buffer_;
    std::vector<float>  window_;
    std::vector<float>  real_;
    std::vector<float>  imag_;
    std::vector<float>  magnitudes_;
    std::vector<float>  previous_bands_;
    float               rms_volume_{0.0f};
    float               bass_energy_{0.0f};
    float               rolling_max_{0.08f};
};

} // namespace enki::audio
