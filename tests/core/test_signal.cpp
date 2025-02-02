/// @file test_signal.cpp
/// @brief Unit tests for Signal/Slot system.

#include "enki/core/signal.hpp"
#include <cassert>
#include <cstdio>
#include <string>
#include <atomic>
#include <thread>
#include <vector>

using namespace enki;

void test_signal_basic_emit() {
    Signal<int> sig;
    int received = 0;

    sig.connect([&received](int val) { received = val; });
    sig.emit(42);

    assert(received == 42);
    printf("  [PASS] signal basic emit\n");
}

void test_signal_multiple_slots() {
    Signal<int, int> sig;
    int sum = 0;
    int product = 0;

    sig.connect([&sum](int a, int b) { sum = a + b; });
    sig.connect([&product](int a, int b) { product = a * b; });

    sig.emit(3, 4);

    assert(sum == 7);
    assert(product == 12);
    printf("  [PASS] signal multiple slots\n");
}

void test_signal_disconnect() {
    Signal<> sig;
    int count = 0;

    auto id = sig.connect([&count]() { ++count; });
    sig.emit();
    assert(count == 1);

    sig.disconnect(id);
    sig.emit();
    assert(count == 1);  // should not have been called again

    printf("  [PASS] signal disconnect\n");
}

void test_signal_disconnect_all() {
    Signal<> sig;
    int count = 0;

    sig.connect([&count]() { ++count; });
    sig.connect([&count]() { ++count; });
    sig.connect([&count]() { ++count; });

    assert(sig.connectionCount() == 3);

    sig.disconnectAll();
    assert(sig.connectionCount() == 0);

    sig.emit();
    assert(count == 0);

    printf("  [PASS] signal disconnect all\n");
}

void test_signal_no_args() {
    Signal<> sig;
    bool fired = false;

    sig.connect([&fired]() { fired = true; });
    sig.emit();

    assert(fired);
    printf("  [PASS] signal no args\n");
}

void test_signal_string_args() {
    Signal<std::string, int> sig;
    std::string last_name;
    int last_age = 0;

    sig.connect([&](std::string name, int age) {
        last_name = name;
        last_age = age;
    });

    sig.emit("ENKI", 1);
    assert(last_name == "ENKI");
    assert(last_age == 1);

    printf("  [PASS] signal string args\n");
}

void test_scoped_connection() {
    Signal<int> sig;
    int received = 0;

    {
        auto conn = scopedConnect(sig, [&received](int v) { received = v; });
        assert(conn.connected());
        sig.emit(10);
        assert(received == 10);
    }
    // Connection should be automatically disconnected

    sig.emit(20);
    assert(received == 10);  // should NOT have been called

    printf("  [PASS] scoped connection\n");
}

void test_scoped_connection_move() {
    Signal<int> sig;
    int received = 0;

    ScopedConnection<int> conn2;
    {
        auto conn1 = scopedConnect(sig, [&received](int v) { received = v; });
        conn2 = std::move(conn1);
        assert(!conn1.connected());
        assert(conn2.connected());
    }

    sig.emit(99);
    assert(received == 99);

    conn2.disconnect();
    sig.emit(100);
    assert(received == 99);

    printf("  [PASS] scoped connection move\n");
}

struct TestReceiver {
    int value = 0;
    void onValueChanged(int v) { value = v; }
};

void test_signal_member_function() {
    Signal<int> sig;
    TestReceiver receiver;

    sig.connect(&receiver, &TestReceiver::onValueChanged);
    sig.emit(42);

    assert(receiver.value == 42);
    printf("  [PASS] signal member function\n");
}

void test_signal_empty() {
    Signal<int> sig;
    assert(sig.empty());
    assert(sig.connectionCount() == 0);

    auto id = sig.connect([](int) {});
    assert(!sig.empty());
    assert(sig.connectionCount() == 1);

    sig.disconnect(id);
    assert(sig.empty());

    printf("  [PASS] signal empty check\n");
}

void test_signal_thread_safety() {
    Signal<int> sig;
    std::atomic<int> total{0};

    // Connect multiple slots
    for (int i = 0; i < 10; ++i) {
        sig.connect([&total](int v) { total += v; });
    }

    // Emit from multiple threads
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back([&sig]() {
            for (int j = 0; j < 100; ++j) {
                sig.emit(1);
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    // Each emit calls 10 slots, adding 1 each = 10 per emit
    // 4 threads × 100 emits × 10 slots × 1 = 4000
    assert(total == 4000);
    printf("  [PASS] signal thread safety\n");
}

int main() {
    printf("=== Signal Tests ===\n");

    test_signal_basic_emit();
    test_signal_multiple_slots();
    test_signal_disconnect();
    test_signal_disconnect_all();
    test_signal_no_args();
    test_signal_string_args();
    test_scoped_connection();
    test_scoped_connection_move();
    test_signal_member_function();
    test_signal_empty();
    test_signal_thread_safety();

    printf("=== All Signal tests passed! ===\n");
    return 0;
}
