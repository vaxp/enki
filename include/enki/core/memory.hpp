#pragma once
/// @file memory.hpp
/// @brief Memory management utilities: Arena allocator and Pool allocator.

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>
#include <memory>
#include <new>
#include <cassert>
#include <utility>
#include <type_traits>

namespace enki {

// ============================================================
// Arena Allocator
// ============================================================
/// A region-based bump allocator for frame-scoped allocations.
/// Objects allocated from the arena are NOT individually freed;
/// instead, the entire arena is reset at once (O(1)).
/// Ideal for widget tree rebuilds where many small allocations
/// are created and discarded each frame.
class Arena {
public:
    /// @param block_size  Size of each memory block (default 64 KB).
    explicit Arena(size_t block_size = 64 * 1024);
    ~Arena();

    // Non-copyable, movable
    Arena(const Arena&) = delete;
    Arena& operator=(const Arena&) = delete;
    Arena(Arena&& other) noexcept;
    Arena& operator=(Arena&& other) noexcept;

    /// Allocate raw memory with the given size and alignment.
    /// @return Pointer to allocated memory, never null (throws on OOM).
    void* allocate(size_t size, size_t alignment = alignof(std::max_align_t));

    /// Construct an object of type T in the arena.
    /// @note The destructor of T will NOT be called automatically.
    ///       Only use for trivially destructible types, or manually
    ///       call the destructor before reset().
    template<typename T, typename... Args>
    T* create(Args&&... args) {
        static_assert(alignof(T) <= alignof(std::max_align_t),
                      "Over-aligned types not supported");
        void* mem = allocate(sizeof(T), alignof(T));
        return new (mem) T(std::forward<Args>(args)...);
    }

    /// Allocate and zero-initialize a block of memory.
    void* allocateZeroed(size_t size, size_t alignment = alignof(std::max_align_t));

    /// Reset the arena, making all memory available for reuse.
    /// Does NOT free memory back to the OS — just resets the bump pointer.
    /// O(1) operation.
    void reset();

    /// @return Total bytes currently allocated (used).
    [[nodiscard]] size_t bytesUsed() const;

    /// @return Total bytes allocated from the OS (capacity).
    [[nodiscard]] size_t bytesAllocated() const;

    /// @return Number of blocks allocated.
    [[nodiscard]] size_t blockCount() const;

private:
    struct Block {
        std::unique_ptr<std::byte[]> data;
        size_t size;
        size_t used;
    };

    std::vector<Block> blocks_;
    size_t block_size_;
    size_t current_block_ = 0;

    void addBlock(size_t min_size);
};

// ============================================================
// Pool Allocator
// ============================================================
/// A slab/pool allocator for fixed-size objects.
/// Provides O(1) allocation and deallocation via a free list.
/// Ideal for Elements and RenderObjects that are frequently
/// created and destroyed.
template<typename T>
class Pool {
public:
    /// @param chunk_size  Number of objects per chunk (default 256).
    explicit Pool(size_t chunk_size = 256)
        : chunk_size_(chunk_size) {}

    ~Pool() {
        // All chunks are freed by unique_ptr
    }

    Pool(const Pool&) = delete;
    Pool& operator=(const Pool&) = delete;

    /// Acquire an object from the pool.
    /// @return Pointer to uninitialized memory of size sizeof(T).
    T* acquire() {
        if (free_list_) {
            // Reuse from free list
            FreeNode* node = free_list_;
            free_list_ = node->next;
            --free_count_;
            return reinterpret_cast<T*>(node);
        }

        // Allocate from current chunk
        if (chunks_.empty() || chunk_used_ >= chunk_size_) {
            addChunk();
        }

        T* ptr = &chunks_.back()[chunk_used_];
        ++chunk_used_;
        ++total_allocated_;
        return ptr;
    }

    /// Construct an object in the pool with the given arguments.
    template<typename... Args>
    T* create(Args&&... args) {
        T* ptr = acquire();
        return new (ptr) T(std::forward<Args>(args)...);
    }

    /// Release an object back to the pool.
    /// @note Does NOT call the destructor — call it manually if needed.
    void release(T* obj) {
        assert(obj != nullptr);
        auto* node = reinterpret_cast<FreeNode*>(obj);
        node->next = free_list_;
        free_list_ = node;
        ++free_count_;
    }

    /// Destroy and release an object back to the pool.
    void destroy(T* obj) {
        if (obj) {
            obj->~T();
            release(obj);
        }
    }

    /// @return Total number of objects allocated.
    [[nodiscard]] size_t totalAllocated() const { return total_allocated_; }

    /// @return Number of objects currently in the free list.
    [[nodiscard]] size_t freeCount() const { return free_count_; }

private:
    struct FreeNode { FreeNode* next; };

    static_assert(sizeof(T) >= sizeof(FreeNode),
                  "Pool object must be at least as large as a pointer");

    size_t chunk_size_;
    size_t chunk_used_ = 0;
    size_t total_allocated_ = 0;
    size_t free_count_ = 0;
    FreeNode* free_list_ = nullptr;

    // Each chunk is an array of T-sized slots (uninitialized)
    struct ChunkDeleter {
        void operator()(T* ptr) {
            ::operator delete(static_cast<void*>(ptr));
        }
    };
    std::vector<std::unique_ptr<T[], ChunkDeleter>> chunks_;

    void addChunk() {
        void* raw = ::operator new(sizeof(T) * chunk_size_);
        chunks_.emplace_back(static_cast<T*>(raw));
        chunk_used_ = 0;
    }
};

}  // namespace enki
