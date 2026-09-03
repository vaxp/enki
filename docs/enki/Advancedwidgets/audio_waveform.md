# AudioWaveform

> A high-performance, real-time audio spectrum analyzer and microphone visualizer for ENKI, featuring dual modes (Wave Music for system audio output monitor and Wave Mic for live microphone input), powered by a dedicated `audio/` subsystem with native C++20 FFT DSP and GPU Skia rendering.

- **Header File**: `#include "enki/widgets/audio_waveform.hpp"`
- **C++ Class**: `enki::AudioWaveformWidget` (inherits from `enki::SingleChildRenderObjectWidget`)
- **Declarative Struct**: `enki::AudioWaveformProps`, `enki::AudioWaveform` (converts implicitly to `WidgetPtr`)
- **Render Object**: `enki::RenderAudioWaveform` (inherits from `enki::RenderBox`)
- **Audio Subsystem**: `audio/` (`<audio/audio_controller.hpp>`, `<audio/audio_analyzer.hpp>`, `<audio/audio_capture.hpp>`)
- **Factory Helper**: `enki::audioWaveform()`

---

## Architectural Overview

The audio engine and spectrum analyzer are cleanly isolated in the root `audio/` subsystem, keeping the core ENKI UI framework lightweight, modular, and 100% independent of any desktop environment.

```
┌─────────────────────────────────────────────────────────────┐
│                    audio/ Subsystem                         │
│  ┌─────────────────────────┐   ┌─────────────────────────┐  │
│  │   AudioCaptureEngine    │   │      AudioAnalyzer      │  │
│  │ (libpulse-simple / PW)  │──►│ (Pure C++20 Radix-2 FFT)│  │
│  │ - Sink Monitor (Music)  │   │ - Hann Windowing        │  │
│  │ - Source In (Mic)       │   │ - Gravity Peak Decay    │  │
│  └─────────────────────────┘   └─────────────────────────┘  │
└──────────────────────────────┬──────────────────────────────┘
                               │ Thread-safe snapshot
                               ▼
┌─────────────────────────────────────────────────────────────┐
│                  AudioWaveform Widget                       │
│  • SingleChildRenderObjectWidget with Ticker (60+ FPS)       │
│  • Skia GPU Vector Drawing (Bars, Splines, Waves)            │
│  • Zero tree rebuilds during playback                        │
└─────────────────────────────────────────────────────────────┘
```

### 1. Wave Music (System Audio Output Visualizer)
- **Mode**: `AudioWaveformType::Music`
- **Signal Source**: Live desktop output monitor (`default_sink.monitor` via PulseAudio / PipeWire).
- **Behavior**: Reacts in real-time to any audio playing on the system (Spotify, YouTube in a browser, media players, system alerts, games).
- **DSP Analysis**: Real-time Fast Fourier Transform (FFT) dividing audio frequencies into logarithmic musical bands (Sub-Bass, Bass, Mid, High-Mid, Treble) with physics-based gravity peak decay.

### 2. Wave Mic (Live Microphone Waveform)
- **Mode**: `AudioWaveformType::Mic`
- **Signal Source**: System default microphone input (`default_source`).
- **Behavior**: Reacts to human speech and ambient sounds in real-time.
- **Visuals**: Symmetrical dancing voice bars centered around the baseline, continuous scrolling time-domain speech tapes, and high-frequency oscilloscopes.

---

## C++ Declarative Definition

```cpp
namespace enki {

enum class AudioWaveformType {
    Music,   ///< System Output Monitor (Spotify, YouTube, Desktop Sound)
    Mic,     ///< Microphone Input (Voice activity, speech recording)
};

enum class WaveformStyle {
    SpectrumBars,     ///< Vertical frequency equalizer bars with peak caps
    FluidWave,        ///< Smooth cubic bezier wave with glowing gradient fill
    StereoBands,      ///< Split Left & Right stereo spectrum bars
    VoiceBars,        ///< Symmetrical dancing voice bars around center line
    ScrollingLive,    ///< Time-domain tape scrolling from right to left
    LiveOscilloscope, ///< Real-time oscillating waveform
};

struct AudioWaveformProps {
    AudioWaveformType                       type            = AudioWaveformType::Music;
    WaveformStyle                           style           = WaveformStyle::SpectrumBars;

    std::shared_ptr<audio::AudioController> controller      = nullptr;
    std::vector<float>                      amplitudes      = {};

    size_t                                  bands           = 32;
    float                                   bar_width       = 4.0f;
    float                                   bar_gap         = 2.0f;
    BorderRadius                            bar_radius      = BorderRadius::circular(2.0f);
    Color                                   primary_color   = 0xFF38BDF8;
    std::optional<Color>                    secondary_color = std::nullopt;
    bool                                    show_peaks      = true;
    float                                   min_bar_height  = 3.0f;

    float                                   sensitivity     = 1.0f;
    float                                   decay_rate      = 0.06f;
    bool                                    auto_start      = true;

    std::optional<StyleValue>               width           = std::nullopt;
    std::optional<StyleValue>               height          = std::nullopt;
    Clip                                    clip_behavior   = Clip::AntiAlias;
    BorderRadius                            clip_radius     = BorderRadius::circular(12.0f);
    Key                                     key             = Key::none();

    operator WidgetPtr() const;
};

inline WidgetPtr audioWaveform(const AudioWaveformProps& props = {});

} // namespace enki
```

---

## Usage Examples

### 1. Real-Time Spotify / YouTube Equalizer (Wave Music)

```cpp
#include "enki/widgets/audio_waveform.hpp"

using namespace enki;

WidgetPtr buildMusicEqualizer() {
    return audioWaveform({
        .type = AudioWaveformType::Music,
        .style = WaveformStyle::SpectrumBars,
        .bands = 48,
        .bar_width = 6.0f,
        .bar_gap = 2.5f,
        .primary_color = 0xFF06B6D4,     // Cyan base
        .secondary_color = 0xFFA855F7,   // Electric purple peak gradient
        .show_peaks = true,
        .width = 480.0f,
        .height = 160.0f,
    });
}
```

### 2. Siri / Assistant Voice Activity Bars (Wave Mic)

```cpp
WidgetPtr buildVoiceVisualizer() {
    return audioWaveform({
        .type = AudioWaveformType::Mic,
        .style = WaveformStyle::VoiceBars,
        .bands = 28,
        .bar_width = 6.0f,
        .bar_gap = 3.0f,
        .primary_color = 0xFF10B981,     // Emerald voice glow
        .sensitivity = 1.6f,
        .width = 360.0f,
        .height = 120.0f,
    });
}
```

### 3. Continuous Fluid Rhythm Wave (Music Spline)

```cpp
WidgetPtr buildFluidRhythm() {
    return audioWaveform({
        .type = AudioWaveformType::Music,
        .style = WaveformStyle::FluidWave,
        .bands = 32,
        .primary_color = 0xFFA855F7,
        .secondary_color = 0xFFEC4899,
        .width = StyleValue::percent(100.0f),
        .height = 140.0f,
    });
}
```
