/// @file test_timeline.cpp
/// @brief Tests for AnimationTimeline, KeyframeSequence, IntervalCurve, and StaggerHelper.

#include "enki/animation/timeline.hpp"
#include "enki/animation/stagger.hpp"
#include <cassert>
#include <cstdio>
#include <cmath>

using namespace enki;
using namespace std::chrono_literals;

void test_timeline_tracks() {
    AnimationTimeline timeline;

    float val1 = 0.0f;
    float val2 = 0.0f;

    // Track 1: 0ms to 200ms
    timeline.add(0ms, 200ms, [&val1](float p) { val1 = p; });

    // Track 2: 100ms to 300ms
    timeline.add(100ms, 200ms, [&val2](float p) { val2 = p; });

    assert(timeline.totalDuration() == 300ms);

    // At progress 0.0 (0ms)
    timeline.seek(0.0f);
    assert(val1 == 0.0f);
    assert(val2 == 0.0f);

    // At progress 100ms / 300ms = 0.3333
    timeline.seekMs(100);
    assert(std::abs(val1 - 0.5f) < 1e-2f); // 100ms in a 200ms track = 0.5
    assert(val2 == 0.0f);                  // Track 2 just started

    // At progress 200ms / 300ms = 0.6666
    timeline.seekMs(200);
    assert(val1 == 1.0f);                  // Track 1 finished
    assert(std::abs(val2 - 0.5f) < 1e-2f); // Track 2 halfway

    // At 300ms (end)
    timeline.seekMs(300);
    assert(val1 == 1.0f);
    assert(val2 == 1.0f);

    printf("  [PASS] timeline multi-track parallel & staggered evaluation\n");
}

void test_keyframe_sequence() {
    KeyframeSequence<float> seq;
    seq.addKeyframe(0.0f, 0.0f);
    seq.addKeyframe(0.2f, 10.0f);
    seq.addKeyframe(0.8f, 50.0f);
    seq.addKeyframe(1.0f, 100.0f);

    assert(std::abs(seq.evaluate(0.0f) - 0.0f) < 1e-4f);
    assert(std::abs(seq.evaluate(0.1f) - 5.0f) < 1e-3f);
    assert(std::abs(seq.evaluate(0.2f) - 10.0f) < 1e-4f);
    assert(std::abs(seq.evaluate(0.5f) - 30.0f) < 1e-3f); // halfway between 0.2 and 0.8
    assert(std::abs(seq.evaluate(1.0f) - 100.0f) < 1e-4f);

    printf("  [PASS] keyframe sequence multi-stop interpolation\n");
}

void test_interval_curve() {
    IntervalCurve interval(0.25f, 0.75f, &Curves::linear);

    assert(interval.evaluate(0.0) == 0.0);
    assert(interval.evaluate(0.1) == 0.0);
    assert(interval.evaluate(0.25) == 0.0);
    assert(std::abs(interval.evaluate(0.5) - 0.5) < 1e-3);
    assert(interval.evaluate(0.75) == 1.0);
    assert(interval.evaluate(0.9) == 1.0);
    assert(interval.evaluate(1.0) == 1.0);

    printf("  [PASS] interval curve sub-window clipping & scaling\n");
}

void test_stagger_helper() {
    StaggerConfig cfg{
        .item_duration = 200ms,
        .delay_between_items = 50ms,
        .curve = &Curves::linear
    };

    // 5 items: total = 4 * 50 + 200 = 400ms
    auto total_dur = StaggerHelper::totalDuration(5, cfg);
    assert(total_dur == 400ms);

    // Item 0: [0ms, 200ms] / 400ms = [0.0, 0.5]
    auto [b0, e0] = StaggerHelper::itemInterval(0, 5, cfg);
    assert(std::abs(b0 - 0.0f) < 1e-4f);
    assert(std::abs(e0 - 0.5f) < 1e-4f);

    // Item 4: [200ms, 400ms] / 400ms = [0.5, 1.0]
    auto [b4, e4] = StaggerHelper::itemInterval(4, 5, cfg);
    assert(std::abs(b4 - 0.5f) < 1e-4f);
    assert(std::abs(e4 - 1.0f) < 1e-4f);

    // At master progress 0.25 (100ms): item 0 is at 0.5, item 4 is at 0.0
    float p0 = StaggerHelper::itemProgress(0, 5, 0.25f, cfg);
    float p4 = StaggerHelper::itemProgress(4, 5, 0.25f, cfg);
    assert(std::abs(p0 - 0.5f) < 1e-3f);
    assert(p4 == 0.0f);

    printf("  [PASS] stagger helper cascading calculations\n");
}

int main() {
    printf("Running Timeline & Stagger tests...\n");
    test_timeline_tracks();
    test_keyframe_sequence();
    test_interval_curve();
    test_stagger_helper();
    printf("All Timeline & Stagger tests passed successfully!\n");
    return 0;
}
