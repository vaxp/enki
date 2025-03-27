/// @file test_output.cpp
/// @brief Unit tests for the Output/Monitor subsystem abstractions and OutputMode.

#include "enki/platform/output.hpp"
#include "enki/core/signal.hpp"
#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

// ── In-memory mock Output ────────────────────────────────────────────────────

namespace enki {

class MockOutput : public Output {
public:
    uint32_t        id_          = 0;
    std::string     name_;
    std::string     make_;
    std::string     model_;
    std::string     description_;
    Rect            geometry_{0, 0, 0, 0};
    Rect            logical_geometry_{0, 0, 0, 0};
    int32_t         phys_w_mm_   = 0;
    int32_t         phys_h_mm_   = 0;
    int32_t         scale_       = 1;
    double          frac_scale_  = 1.0;
    OutputTransform transform_   = OutputTransform::Normal;
    OutputSubpixel  subpixel_    = OutputSubpixel::Unknown;
    std::vector<OutputMode> modes_;
    OutputMode      current_mode_;
    bool            is_primary_  = false;

    MockOutput(uint32_t id, std::string name, int32_t w, int32_t h, int32_t refresh_mHz)
        : id_(id), name_(std::move(name))
    {
        geometry_         = {0.0f, 0.0f, static_cast<float>(w), static_cast<float>(h)};
        logical_geometry_ = geometry_;
        description_      = name_ + " Mock Display";
        current_mode_     = {w, h, refresh_mHz, true, true};
        modes_.push_back(current_mode_);
    }

    [[nodiscard]] uint32_t id()                                const noexcept override { return id_; }
    [[nodiscard]] const std::string& name()                   const noexcept override { return name_; }
    [[nodiscard]] const std::string& make()                   const noexcept override { return make_; }
    [[nodiscard]] const std::string& model()                  const noexcept override { return model_; }
    [[nodiscard]] const std::string& description()            const noexcept override { return description_; }
    [[nodiscard]] Rect geometry()                             const noexcept override { return geometry_; }
    [[nodiscard]] Rect logicalGeometry()                      const noexcept override { return logical_geometry_; }
    [[nodiscard]] int32_t physicalWidthMm()                   const noexcept override { return phys_w_mm_; }
    [[nodiscard]] int32_t physicalHeightMm()                  const noexcept override { return phys_h_mm_; }
    [[nodiscard]] int32_t scaleFactor()                       const noexcept override { return scale_; }
    [[nodiscard]] double fractionalScale()                    const noexcept override { return frac_scale_; }
    [[nodiscard]] OutputTransform transform()                 const noexcept override { return transform_; }
    [[nodiscard]] OutputSubpixel subpixel()                   const noexcept override { return subpixel_; }
    [[nodiscard]] const std::vector<OutputMode>& modes()      const noexcept override { return modes_; }
    [[nodiscard]] const OutputMode& currentMode()             const noexcept override { return current_mode_; }
    [[nodiscard]] bool isPrimary()                            const noexcept override { return is_primary_; }
    [[nodiscard]] void* nativeHandle()                        const noexcept override { return nullptr; }

    void addMode(int32_t w, int32_t h, int32_t refresh_mHz, bool preferred = false) {
        modes_.push_back({w, h, refresh_mHz, false, preferred});
    }

    void emitGeometryChanged() { on_geometry_changed_.emit(); }
    void emitModeChanged()     { on_mode_changed_.emit(); }
    void emitScaleChanged()    { on_scale_changed_.emit(); }
    void emitRemoved()         { on_removed_.emit(); }
};

} // namespace enki

// ── Tests ────────────────────────────────────────────────────────────────────

void test_output_mode_refresh() {
    using namespace enki;
    OutputMode mode60{1920, 1080, 60000, true, true};
    assert(mode60.width == 1920);
    assert(mode60.height == 1080);
    assert(mode60.refresh_rate_mHz == 60000);
    assert(mode60.is_current);
    assert(mode60.is_preferred);

    // refreshRateHz() conversion
    double hz = mode60.refreshRateHz();
    assert(hz >= 59.9 && hz <= 60.1);

    OutputMode mode144{2560, 1440, 144000, false, false};
    assert(mode144.refreshRateHz() >= 143.9 && mode144.refreshRateHz() <= 144.1);

    OutputMode mode165{2560, 1440, 165000, false, false};
    assert(mode165.refreshRateHz() >= 164.9 && mode165.refreshRateHz() <= 165.1);

    std::cout << "[PASS] test_output_mode_refresh\n";
}

void test_output_basic_properties() {
    using namespace enki;

    auto out = std::make_shared<MockOutput>(1, "eDP-1", 1920, 1080, 60000);
    out->make_  = "AU Optronics";
    out->model_ = "B156HTN03.2";
    out->phys_w_mm_ = 344;
    out->phys_h_mm_ = 194;
    out->is_primary_ = true;

    assert(out->id() == 1);
    assert(out->name() == "eDP-1");
    assert(out->make() == "AU Optronics");
    assert(out->model() == "B156HTN03.2");
    assert(out->isPrimary());
    assert(out->physicalWidthMm() == 344);
    assert(out->physicalHeightMm() == 194);

    auto geom = out->geometry();
    assert(geom.width == 1920.0f);
    assert(geom.height == 1080.0f);
    assert(geom.x == 0.0f && geom.y == 0.0f);

    assert(out->scaleFactor() == 1);
    assert(out->fractionalScale() == 1.0);
    assert(out->transform() == OutputTransform::Normal);
    assert(out->subpixel() == OutputSubpixel::Unknown);

    std::cout << "[PASS] test_output_basic_properties\n";
}

void test_output_modes() {
    using namespace enki;

    auto out = std::make_shared<MockOutput>(2, "HDMI-A-1", 1920, 1080, 60000);
    out->addMode(1280, 720, 60000, false);
    out->addMode(1920, 1080, 144000, false);
    out->addMode(1920, 1080, 75000, false);

    // Total modes: 1 (from constructor) + 3 added = 4
    assert(out->modes().size() == 4);

    // Current mode should be the one set in constructor
    const auto& cm = out->currentMode();
    assert(cm.width == 1920);
    assert(cm.height == 1080);
    assert(cm.refresh_rate_mHz == 60000);
    assert(cm.is_current);
    assert(cm.is_preferred);

    // Verify non-current modes
    bool found_144 = false;
    for (const auto& m : out->modes()) {
        if (m.refresh_rate_mHz == 144000) {
            found_144 = true;
            assert(!m.is_current);
        }
    }
    assert(found_144);

    std::cout << "[PASS] test_output_modes\n";
}

void test_output_scale_factor() {
    using namespace enki;

    auto out = std::make_shared<MockOutput>(3, "DP-1", 3840, 2160, 60000);
    out->scale_      = 2;
    out->frac_scale_ = 2.0;
    out->logical_geometry_ = {0.0f, 0.0f, 1920.0f, 1080.0f};

    assert(out->scaleFactor() == 2);
    assert(out->fractionalScale() == 2.0);

    auto phys = out->geometry();
    assert(phys.width == 3840.0f && phys.height == 2160.0f);

    auto logical = out->logicalGeometry();
    assert(logical.width == 1920.0f && logical.height == 1080.0f);

    // Test fractional 1.5× scale
    auto out2 = std::make_shared<MockOutput>(4, "DP-2", 2560, 1600, 60000);
    out2->scale_      = 2;
    out2->frac_scale_ = 1.5;
    out2->logical_geometry_ = {0.0f, 0.0f, 1706.0f, 1066.0f};
    assert(out2->fractionalScale() == 1.5);

    std::cout << "[PASS] test_output_scale_factor\n";
}

void test_output_transform() {
    using namespace enki;

    auto out = std::make_shared<MockOutput>(5, "HDMI-A-2", 1920, 1080, 60000);

    out->transform_ = OutputTransform::Rotated90;
    assert(out->transform() == OutputTransform::Rotated90);

    out->transform_ = OutputTransform::Rotated270;
    assert(out->transform() == OutputTransform::Rotated270);

    out->transform_ = OutputTransform::Flipped;
    assert(out->transform() == OutputTransform::Flipped);

    out->transform_ = OutputTransform::Normal;
    assert(out->transform() == OutputTransform::Normal);

    std::cout << "[PASS] test_output_transform\n";
}

void test_output_multi_monitor_layout() {
    using namespace enki;

    // Simulate: primary at 0,0 (1920×1080) + secondary at 1920,0 (2560×1440)
    auto primary = std::make_shared<MockOutput>(1, "eDP-1", 1920, 1080, 60000);
    primary->is_primary_ = true;
    primary->geometry_ = {0.0f, 0.0f, 1920.0f, 1080.0f};

    auto secondary = std::make_shared<MockOutput>(2, "DP-1", 2560, 1440, 144000);
    secondary->is_primary_ = false;
    secondary->geometry_ = {1920.0f, 0.0f, 2560.0f, 1440.0f};

    std::vector<std::shared_ptr<Output>> outputs = {primary, secondary};

    // Primary check
    std::shared_ptr<Output> found_primary = nullptr;
    for (const auto& o : outputs) {
        if (o->isPrimary()) { found_primary = o; break; }
    }
    assert(found_primary != nullptr);
    assert(found_primary->name() == "eDP-1");

    // Total area
    float total_width = primary->geometry().width + secondary->geometry().width;
    assert(total_width == 1920.0f + 2560.0f);

    // Find by name
    std::shared_ptr<Output> found_dp1 = nullptr;
    for (const auto& o : outputs) {
        if (o->name() == "DP-1") { found_dp1 = o; break; }
    }
    assert(found_dp1 != nullptr);
    assert(found_dp1->currentMode().refresh_rate_mHz == 144000);
    assert(found_dp1->geometry().x == 1920.0f);

    std::cout << "[PASS] test_output_multi_monitor_layout\n";
}

void test_output_signals() {
    using namespace enki;

    auto out = std::make_shared<MockOutput>(6, "VGA-1", 1280, 1024, 75000);

    int geometry_count = 0;
    int mode_count     = 0;
    int scale_count    = 0;
    int removed_count  = 0;

    out->onGeometryChanged().connect([&] { ++geometry_count; });
    out->onModeChanged().connect([&]     { ++mode_count; });
    out->onScaleChanged().connect([&]    { ++scale_count; });
    out->onRemoved().connect([&]         { ++removed_count; });

    out->emitGeometryChanged();
    assert(geometry_count == 1);
    assert(mode_count == 0);

    out->emitModeChanged();
    assert(mode_count == 1);

    out->emitScaleChanged();
    assert(scale_count == 1);

    out->emitRemoved();
    assert(removed_count == 1);

    // Multiple fires
    out->emitGeometryChanged();
    out->emitGeometryChanged();
    assert(geometry_count == 3);

    std::cout << "[PASS] test_output_signals\n";
}

void test_output_hotplug_simulation() {
    using namespace enki;

    // Simulate: output is added then removed
    Signal<std::shared_ptr<Output>> on_added;
    Signal<std::shared_ptr<Output>> on_removed;

    std::vector<std::shared_ptr<Output>> active_outputs;
    int added_count   = 0;
    int removed_count = 0;

    on_added.connect([&](std::shared_ptr<Output> o) {
        active_outputs.push_back(o);
        ++added_count;
    });

    on_removed.connect([&](std::shared_ptr<Output> o) {
        active_outputs.erase(
            std::remove_if(active_outputs.begin(), active_outputs.end(),
                           [&](const auto& p) { return p->id() == o->id(); }),
            active_outputs.end());
        ++removed_count;
    });

    // Plug in two monitors
    auto mon1 = std::make_shared<MockOutput>(10, "HDMI-A-1", 1920, 1080, 60000);
    auto mon2 = std::make_shared<MockOutput>(11, "DP-1",     2560, 1440, 144000);
    on_added.emit(mon1);
    on_added.emit(mon2);

    assert(added_count == 2);
    assert(active_outputs.size() == 2);

    // Unplug first monitor
    on_removed.emit(mon1);
    assert(removed_count == 1);
    assert(active_outputs.size() == 1);
    assert(active_outputs.front()->name() == "DP-1");

    // Plug it back in
    on_added.emit(mon1);
    assert(added_count == 3);
    assert(active_outputs.size() == 2);

    std::cout << "[PASS] test_output_hotplug_simulation\n";
}

int main() {
    std::cout << "=== ENKI Output/Monitor Unit Tests ===\n\n";

    test_output_mode_refresh();
    test_output_basic_properties();
    test_output_modes();
    test_output_scale_factor();
    test_output_transform();
    test_output_multi_monitor_layout();
    test_output_signals();
    test_output_hotplug_simulation();

    std::cout << "\nAll Output unit tests passed successfully! ✓\n";
    return 0;
}
