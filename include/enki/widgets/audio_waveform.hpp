#pragma once
/// @file audio_waveform.hpp
/// @brief Declarative AudioWaveform Widget (Music Spectrum Visualizer & Live Mic Waveform).
///
/// Features:
///   - Wave Music: Real-time System Audio Spectrum Analyzer (Spotify, YouTube, Desktop sound)
///   - Wave Mic: Real-time Microphone Waveform & Voice Activity Visualizer
///   - Distinctive Luxurious Styles:
///       • SegmentedLed: Studio Rack LED ladder with translucent glass floor reflection
///       • RadialArc: 360° circular pulsing sound reactor with bass beat core
///       • LayeredAurora: Triple-layer translucent neon spline waves with ambient glow
///       • SymmetricWings: Center-outward mirrored cyber wings
///       • VoiceBars: Centered symmetrical speech activity pills
///       • ScrollingLive: Right-to-left scrolling audio history tape
///       • LiveOscilloscope: Real-time vibration oscilloscope
///   - High-performance Skia GPU rendering with zero-rebuild 60FPS ticker
///   - Automatic Anu Flexbox layout sizing
///   - 100% Declarative C++20 Designated Initializers
///
/// @copyright ENKI Framework — MIT License

#include "enki/core/types.hpp"
#include "enki/tree/widget.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/rendering/canvas.hpp"
#include "audio/audio_controller.hpp"

#include <vector>
#include <memory>
#include <optional>
#include <string_view>

namespace enki {

/// @brief Primary operation mode
enum class AudioWaveformType {
    Music,   ///< System Output Monitor (Spotify, YouTube, media player, desktop audio)
    Mic,     ///< Microphone Input (Voice activity, speech recording, mic monitor)
};

/// @brief Visual rendering style
enum class WaveformStyle {
    SegmentedLed,     ///< High-end studio rack segmented LED ladder with floor reflection
    RadialArc,        ///< 360-degree circular pulsing reactor with bass beat core
    LayeredAurora,    ///< Triple-layer translucent neon spline waves with ambient glow
    SymmetricWings,   ///< Center-outward mirrored cyber wings (butterfly spectrum)
    SpectrumBars,     ///< Classic vertical frequency equalizer bars with peak caps
    FluidWave,        ///< Continuous smooth cubic bezier wave with gradient fill
    StereoBands,      ///< Split Left & Right stereo spectrum bars
    VoiceBars,        ///< Symmetrical dancing voice bars around center line (Mic default)
    ScrollingLive,    ///< Time-domain tape scrolling from right to left as sound arrives
    LiveOscilloscope, ///< Real-time oscilloscope vibration wave
};

// ════════════════════════════════════════════════════════════════
// AudioWaveformProps & Declarative Factory
// ════════════════════════════════════════════════════════════════

struct AudioWaveformProps {
    AudioWaveformType                       type            = AudioWaveformType::Music;
    WaveformStyle                           style           = WaveformStyle::SegmentedLed;

    // ── Data & Controller ───────────────────────────────────────
    std::shared_ptr<audio::AudioController> controller      = nullptr; ///< Optional external controller
    std::vector<float>                      amplitudes      = {};      ///< Static or programmatic data

    // ── Visual Styling & Geometry ───────────────────────────────
    size_t                                  bands           = 32;      ///< Number of visual bars (16, 24, 32, 48, 64)
    float                                   bar_width       = 5.0f;
    float                                   bar_gap         = 3.0f;
    BorderRadius                            bar_radius      = BorderRadius::circular(2.5f);
    Color                                   primary_color   = 0xFF00E5FF; ///< Base / Cyan
    std::optional<Color>                    secondary_color = 0xFFA855F7; ///< Mid / Purple
    std::optional<Color>                    accent_color    = 0xFFF43F5E; ///< High / Coral Neon
    bool                                    show_peaks      = true;    ///< Floating peak caps on bars
    bool                                    show_reflection = true;    ///< Mirrored glass floor reflection
    float                                   min_bar_height  = 4.0f;    ///< Resting idle height in pixels

    // ── DSP & Audio Tuning ──────────────────────────────────────
    float                                   sensitivity     = 1.0f;    ///< Gain boost
    float                                   decay_rate      = 0.05f;   ///< Gravity falloff speed
    bool                                    auto_start      = true;    ///< Auto-start audio stream on mount

    // ── Layout Dimensions & Constraints ─────────────────────────
    std::optional<StyleValue>               width           = std::nullopt;
    std::optional<StyleValue>               height          = std::nullopt;
    std::optional<StyleValue>               min_width       = std::nullopt;
    std::optional<StyleValue>               min_height      = std::nullopt;
    std::optional<StyleValue>               max_width       = std::nullopt;
    std::optional<StyleValue>               max_height      = std::nullopt;

    Clip                                    clip_behavior   = Clip::AntiAlias;
    BorderRadius                            clip_radius     = BorderRadius::circular(16.0f);
    Key                                     key             = Key::none();

    operator WidgetPtr() const;
};

struct AudioWaveform : public AudioWaveformProps {
    using AudioWaveformProps::AudioWaveformProps;
};

inline WidgetPtr audioWaveform(const AudioWaveformProps& props = {}) {
    return static_cast<WidgetPtr>(props);
}

// ════════════════════════════════════════════════════════════════
// AudioWaveformWidget (SingleChildRenderObjectWidget)
// ════════════════════════════════════════════════════════════════

class AudioWaveformWidget : public SingleChildRenderObjectWidget {
public:
    AudioWaveformProps props;

    explicit AudioWaveformWidget(AudioWaveformProps p)
        : SingleChildRenderObjectWidget(p.key, nullptr), props(std::move(p)) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext& ctx) override;
    void updateRenderObject(BuildContext& ctx, RenderObject& renderObject) override;
    [[nodiscard]] std::string_view typeName() const override { return "AudioWaveform"; }
};

} // namespace enki
