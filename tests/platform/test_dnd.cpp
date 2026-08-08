/// @file test_dnd.cpp
/// @brief Unit tests for DragAction, DataOffer abstractions, and DnD lifecycle events.

#include "enki/platform/dnd.hpp"
#include <cassert>
#include <iostream>
#include <vector>
#include <string>

void test_drag_action_flags() {
    using namespace enki;

    DragAction action = DragAction::Copy | DragAction::Move;
    assert(hasDragAction(action, DragAction::Copy));
    assert(hasDragAction(action, DragAction::Move));
    assert(!hasDragAction(action, DragAction::Link));
    assert(!hasDragAction(action, DragAction::Ask));

    DragAction single = DragAction::Link;
    assert(hasDragAction(single, DragAction::Link));
    assert(!hasDragAction(single, DragAction::Copy));

    std::cout << "[PASS] test_drag_action_flags\n";
}

void test_memory_data_offer() {
    using namespace enki;

    ClipboardData data;
    data.setText("Dragged Payload Text");
    std::vector<std::string> uris = { "file:///path/to/dragged_file.txt" };
    data.setUris(uris);

    MemoryDataOffer offer(data);

    assert(offer.hasFormat(mime::TextPlainUtf8));
    assert(offer.hasFormat(mime::TextUriList));
    assert(!offer.hasFormat(mime::ImagePng));

    assert(offer.readText() == "Dragged Payload Text");

    auto read_uris = offer.readUris();
    assert(read_uris.size() == 1);
    assert(read_uris[0] == "file:///path/to/dragged_file.txt");

    std::cout << "[PASS] test_memory_data_offer\n";
}

void test_dnd_events_lifecycle() {
    using namespace enki;

    // Test DragEnterEvent
    DragEnterEvent enter_ev;
    enter_ev.position = Point{100.0f, 150.0f};
    enter_ev.mime_types = { std::string(mime::TextPlainUtf8), std::string(mime::TextUriList) };
    enter_ev.suggested_action = DragAction::Copy;

    assert(!enter_ev.isAccepted());
    enter_ev.accept(DragAction::Copy);
    assert(enter_ev.isAccepted());
    assert(enter_ev.accepted_action == DragAction::Copy);

    enter_ev.reject();
    assert(!enter_ev.isAccepted());

    // Test DragMotionEvent
    DragMotionEvent motion_ev;
    motion_ev.position = Point{110.0f, 155.0f};
    motion_ev.accept(DragAction::Move);
    assert(motion_ev.isAccepted());
    assert(motion_ev.accepted_action == DragAction::Move);

    // Test DropEvent
    ClipboardData drop_data;
    drop_data.setText("Dropped Data");
    DropEvent drop_ev;
    drop_ev.position = Point{110.0f, 155.0f};
    drop_ev.data = std::make_shared<MemoryDataOffer>(drop_data);
    drop_ev.action = DragAction::Move;
    drop_ev.handled = true;

    assert(drop_ev.handled);
    assert(drop_ev.data->readText() == "Dropped Data");

    std::cout << "[PASS] test_dnd_events_lifecycle\n";
}

int main() {
    test_drag_action_flags();
    test_memory_data_offer();
    test_dnd_events_lifecycle();
    std::cout << "All Drag-and-Drop unit tests passed successfully!\n";
    return 0;
}
