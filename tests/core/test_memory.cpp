/// @file test_memory.cpp
/// @brief Unit tests for Arena and Pool allocators.

#include "enki/core/memory.hpp"
#include <cassert>
#include <cstdio>
#include <vector>
#include <cstring>

void test_arena_basic_alloc() {
    enki::Arena arena(1024);

    // Allocate some memory
    int* a = arena.create<int>(42);
    assert(a != nullptr);
    assert(*a == 42);

    float* b = arena.create<float>(3.14f);
    assert(b != nullptr);
    assert(*b == 3.14f);

    printf("  [PASS] arena basic allocation\n");
}

void test_arena_multiple_allocs() {
    enki::Arena arena(256);

    // Allocate many objects — should span multiple blocks
    std::vector<int*> ptrs;
    for (int i = 0; i < 1000; ++i) {
        int* p = arena.create<int>(i);
        assert(p != nullptr);
        assert(*p == i);
        ptrs.push_back(p);
    }

    // Verify all values are still correct
    for (int i = 0; i < 1000; ++i) {
        assert(*ptrs[i] == i);
    }

    assert(arena.blockCount() > 1);
    printf("  [PASS] arena multiple allocations (%zu blocks)\n", arena.blockCount());
}

void test_arena_reset() {
    enki::Arena arena(1024);

    size_t before = arena.bytesUsed();
    assert(before == 0);

    arena.create<int>(1);
    arena.create<int>(2);
    arena.create<int>(3);

    assert(arena.bytesUsed() > 0);

    arena.reset();
    assert(arena.bytesUsed() == 0);

    // Can allocate again after reset
    int* p = arena.create<int>(99);
    assert(p != nullptr);
    assert(*p == 99);

    printf("  [PASS] arena reset\n");
}

void test_arena_zeroed() {
    enki::Arena arena(1024);

    auto* buf = static_cast<uint8_t*>(arena.allocateZeroed(64));
    for (int i = 0; i < 64; ++i) {
        assert(buf[i] == 0);
    }

    printf("  [PASS] arena zeroed allocation\n");
}

struct TestObj {
    int x, y, z;
    double w;
};

void test_pool_basic() {
    enki::Pool<TestObj> pool(16);

    TestObj* a = pool.create(TestObj{1, 2, 3, 4.0});
    assert(a != nullptr);
    assert(a->x == 1);
    assert(a->y == 2);
    assert(a->z == 3);
    assert(a->w == 4.0);

    assert(pool.totalAllocated() == 1);
    printf("  [PASS] pool basic allocation\n");
}

void test_pool_recycle() {
    enki::Pool<TestObj> pool(4);

    // Allocate 4 objects
    TestObj* a = pool.acquire();
    TestObj* b = pool.acquire();
    TestObj* c = pool.acquire();
    TestObj* d = pool.acquire();

    // Release 2
    pool.release(b);
    pool.release(c);

    assert(pool.freeCount() == 2);

    // Acquire again — should reuse freed slots
    TestObj* e = pool.acquire();
    TestObj* f = pool.acquire();

    // The recycled pointers should be one of the previously freed ones
    assert(e == c || e == b);  // free list is LIFO
    assert(f == c || f == b);
    assert(e != f);

    assert(pool.freeCount() == 0);
    printf("  [PASS] pool recycle\n");
}

void test_pool_grow() {
    enki::Pool<TestObj> pool(4);

    // Allocate more than chunk_size
    std::vector<TestObj*> ptrs;
    for (int i = 0; i < 100; ++i) {
        ptrs.push_back(pool.acquire());
    }

    assert(pool.totalAllocated() == 100);

    // All pointers should be unique
    for (size_t i = 0; i < ptrs.size(); ++i) {
        for (size_t j = i + 1; j < ptrs.size(); ++j) {
            assert(ptrs[i] != ptrs[j]);
        }
    }

    printf("  [PASS] pool grow\n");
}

int main() {
    printf("=== Memory Tests ===\n");

    test_arena_basic_alloc();
    test_arena_multiple_allocs();
    test_arena_reset();
    test_arena_zeroed();
    test_pool_basic();
    test_pool_recycle();
    test_pool_grow();

    printf("=== All memory tests passed! ===\n");
    return 0;
}
