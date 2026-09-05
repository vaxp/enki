# Enki Animation & Motion Suite

> Declarative C++20 implicit and explicit animation widgets, analytical physics springs, multi-track timelines, SVG vector morphing, GPU particles, and shared-element transitions.

The **Animation & Motion** subsystem provides fluid, 60+ FPS hardware-accelerated visual transitions for user interfaces in the Enki GUI Framework. The suite is divided into three intuitive architectural tiers:

1. **Implicit Animations** (Fire-and-forget; automatically interpolate changing properties on `setState()`).
2. **Explicit Transitions** (Driven by an `AnimationController` for scrubbing, looping, and bidirectional choreography).
3. **Next-Gen Motion Engines** (Analytical physics springs, multi-track timeline sequencers, continuous SVG path morphing, GPU particle simulations, and shared-element Hero transitions).

---

## Architectural Paradigms

### 1. Implicit Animations (Fire-and-Forget)
Implicit widgets manage their own internal tickers. When a property value (dimensions, colors, scale, rotation, opacity) changes during a reactive `setState()` rebuild, the widget automatically interpolates smoothly from its current value to the target over a given duration:
- `AnimatedOpacity`
- `AnimatedContainer`
- `AnimatedScale`
- `AnimatedRotation`
- `AnimatedSlide`
- `AnimatedSwitcher`

### 2. Explicit Transitions (Controller-Driven)
Explicit transitions accept an external `std::shared_ptr<AnimationController>` and target bounds, allowing manual scrubbing, loop control, and gesture interaction:
- `SlideTransition`

### 3. Next-Gen Motion Systems
Advanced physics, vector, particle, and orchestration subsystems added in Roadmap v0.2.0:
- [**Physics-Based Spring Engine**](./spring_physics.md) — Analytical $O(1)$ damped harmonic oscillator preserving velocity on retargeting with zero idle CPU.
- [**Timeline & Staggered Sequencer**](./timeline_and_stagger.md) — Multi-track keyframe sequencer and cascading list/grid stagger calculation.
- [**SVG Vector Path Morphing**](./svg_morph.md) — Topology normalization and arc-length perimeter sampling for morphing arbitrary vector shapes.
- [**2D GPU Particle Physics System**](./particle_system.md) — Zero-allocation particle pool with gravity, drag, tumbling rotation, and preset effects (Confetti, Sparks, Snow).
- [**Hero & Shared Element Transitions**](./hero_transitions.md) — Seamless element morphing between routes/views via global coordinate tracking.

---

## Supported Physical Curves (`enki/animation/curves.hpp`)

All motion widgets accept a pointer to a `Curve` or physics adapter:
- `Curves::linear` — Constant velocity.
- `Curves::easeIn`, `Curves::easeOut`, `Curves::easeInOut` — Standard cubic bezier easing.
- `Curves::fastOutSlowIn` — Natural material design acceleration and deceleration.
- `Curves::elasticOut` — Elastic spring-loaded overshoot.
- `Curves::bounceOut` — Gravitational bounce settling.
- `SpringCurve` — Direct analytical spring physics adapter.
- `IntervalCurve` — Normalized sub-interval window $[begin, end] \subseteq [0, 1]$.

---

## Complete Catalog (Animation & Motion)

| # | System / Widget | Primary Type / Helper | Header | Description |
|---|---|---|---|---|
| 1 | [**Spring Physics Engine**](./spring_physics.md) | `SpringController`, `SpringValue<T>`, `SpringCurve` | `<enki/animation/spring_controller.hpp>` | Analytical damped harmonic oscillator preserving velocity upon mid-flight interruption. |
| 2 | [**Timeline & Stagger**](./timeline_and_stagger.md) | `AnimationTimeline`, `StaggerHelper`, `KeyframeSequence<T>` | `<enki/animation/timeline.hpp>` | Multi-track timeline orchestration, keyframe sequencing, and cascading entry delays. |
| 3 | [**SVG Vector Morph**](./svg_morph.md) | `struct SvgMorph`, `svgMorph()` | `<enki/widgets/svg_morph.hpp>` | Continuous vector path morphing and topology normalization via Skia perimeter sampling. |
| 4 | [**GPU Particle System**](./particle_system.md) | `struct ParticleEmitter`, `particleEmitter()` | `<enki/widgets/particle_emitter.hpp>` | Zero-allocation 2D physics particle simulation (Confetti, Sparks, Dust, Snow). |
| 5 | [**Hero Transitions**](./hero_transitions.md) | `struct Hero`, `hero()` | `<enki/widgets/hero.hpp>` | Shared-element flight morphing between screens using global coordinate tracking. |
| 6 | [**AnimatedContainer**](./animated_container.md) | `struct AnimatedContainer`, `animatedContainer()` | `<enki/widgets/motion.hpp>` | Automatically morphs size, colors, borders, radius, and insets. |
| 7 | [**AnimatedOpacity**](./animated_opacity.md) | `struct AnimatedOpacity`, `animatedOpacity()` | `<enki/widgets/motion.hpp>` | Smoothly animates opacity fading via GPU saveLayer alpha blending. |
| 8 | [**AnimatedScale**](./animated_scale.md) | `struct AnimatedScale`, `animatedScale()` | `<enki/widgets/motion.hpp>` | Smooth scale transformations centered around an `Alignment` pivot. |
| 9 | [**AnimatedRotation**](./animated_rotation.md) | `struct AnimatedRotation`, `animatedRotation()` | `<enki/widgets/motion.hpp>` | Smooth rotation animation driven by turn count (`1.0f = 360°`). |
| 10 | [**AnimatedSlide**](./animated_slide.md) | `struct AnimatedSlide`, `animatedSlide()` | `<enki/widgets/motion.hpp>` | Translates child position relative to its layout bounding box. |
| 11 | [**AnimatedSwitcher**](./animated_switcher.md) | `struct AnimatedSwitcher`, `animatedSwitcher()` | `<enki/widgets/motion.hpp>` | Cross-fades and transitions between changing child widgets. |
| 12 | [**SlideTransition**](./slide_transition.md) | `struct SlideTransition`, `slideTransition()` | `<enki/widgets/motion.hpp>` | Low-level positional slide driven by an explicit `AnimationController`. |

---

## Live Showcase Demo

All 5 new animation systems can be evaluated interactively in the dedicated showcase application:

```bash
# Build and run the Next-Gen Animation Suite Demo
./build/widgets_demo/animation_suite_demo/animation_suite_demo
```
