#include "enki/widgets/toggle_button.hpp"
#include "enki/widgets/segmented_control.hpp"
#include "enki/widgets/rating_bar.hpp"
#include "enki/widgets/knob.hpp"
#include "enki/widgets/otp_field.hpp"
#include "enki/widgets/pin_field.hpp"
#include "enki/widgets/tag_input.hpp"
#include "enki/widgets/file_drop_zone.hpp"
#include "enki/tree/build_context.hpp"
#include "enki/tree/element.hpp"
#include <iostream>
#include <cassert>

using namespace enki;

void test_toggle_button() {
    bool toggled = false;
    auto tb = toggleButton({
        .is_toggled = false,
        .on_toggle = [&toggled](bool val) { toggled = val; },
        .label = "Dark Mode",
    });
    auto el = tb->createElement();
    assert(el != nullptr);
    std::cout << "  [PASS] ToggleButton element created\n";
}

void test_segmented_control() {
    int selected = 0;
    auto sc = segmentedControl({
        .items = {
            SegmentItem("Day"),
            SegmentItem("Week"),
            SegmentItem("Month"),
        },
        .selected_index = 1,
        .on_change = [&selected](int idx) { selected = idx; },
    });
    auto el = sc->createElement();
    assert(el != nullptr);
    std::cout << "  [PASS] SegmentedControl element created\n";
}

void test_rating_bar() {
    float rating_val = 3.5f;
    auto rb = ratingBar({
        .rating = rating_val,
        .max_rating = 5,
        .allow_half = true,
        .on_rating_changed = [&rating_val](float r) { rating_val = r; },
    });
    auto el = rb->createElement();
    assert(el != nullptr);
    std::cout << "  [PASS] RatingBar element created\n";
}

void test_knob() {
    float knob_val = 50.0f;
    auto kn = knob({
        .value = knob_val,
        .min_value = 0.0f,
        .max_value = 100.0f,
        .label = "VOL",
        .on_value_changed = [&knob_val](float v) { knob_val = v; },
    });
    auto el = kn->createElement();
    assert(el != nullptr);
    std::cout << "  [PASS] Knob element created\n";
}

void test_otp_field() {
    std::string otp = "";
    auto of = otpField({
        .length = 6,
        .on_changed = [&otp](const std::string& code) { otp = code; },
    });
    auto el = of->createElement();
    assert(el != nullptr);
    std::cout << "  [PASS] OTPField element created\n";
}

void test_pin_field() {
    std::string pin = "";
    auto pf = pinField({
        .length = 4,
        .on_completed = [&pin](const std::string& p) { pin = p; },
    });
    auto el = pf->createElement();
    assert(el != nullptr);
    std::cout << "  [PASS] PinField element created\n";
}

void test_tag_input() {
    std::vector<std::string> tags = {"C++", "Skia", "VAXP-OS"};
    auto ti = tagInput({
        .tags = tags,
        .on_tags_changed = [&tags](const std::vector<std::string>& t) { tags = t; },
    });
    auto el = ti->createElement();
    assert(el != nullptr);
    std::cout << "  [PASS] TagInput element created\n";
}

void test_file_drop_zone() {
    std::vector<std::string> dropped;
    auto fdz = fileDropZone({
        .allowed_extensions = {".mp4", ".png"},
        .on_files_dropped = [&dropped](const std::vector<std::string>& files) { dropped = files; },
    });
    auto el = fdz->createElement();
    assert(el != nullptr);
    std::cout << "  [PASS] FileDropZone element created\n";
}

int main() {
    std::cout << "========================================\n";
    std::cout << "  ENKI Rich Input Controls Unit Tests   \n";
    std::cout << "  Roadmap v0.2.0 | Section 15           \n";
    std::cout << "========================================\n";

    test_toggle_button();
    test_segmented_control();
    test_rating_bar();
    test_knob();
    test_otp_field();
    test_pin_field();
    test_tag_input();
    test_file_drop_zone();

    std::cout << "All Section 15 Rich Input Controls tests passed successfully!\n";
    return 0;
}
