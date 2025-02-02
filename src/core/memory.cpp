/// @file memory.cpp
/// @brief Implementation of Arena allocator.

#include "enki/core/memory.hpp"
#include <algorithm>
#include <stdexcept>

namespace enki {

// ============================================================
// Arena Implementation
// ============================================================

Arena::Arena(size_t block_size)
    : block_size_(block_size) {
    // Pre-allocate the first block
    addBlock(block_size_);
}

Arena::~Arena() = default;

Arena::Arena(Arena&& other) noexcept
    : blocks_(std::move(other.blocks_)),
      block_size_(other.block_size_),
      current_block_(other.current_block_) {
    other.current_block_ = 0;
}

Arena& Arena::operator=(Arena&& other) noexcept {
    if (this != &other) {
        blocks_ = std::move(other.blocks_);
        block_size_ = other.block_size_;
        current_block_ = other.current_block_;
        other.current_block_ = 0;
    }
    return *this;
}

void* Arena::allocate(size_t size, size_t alignment) {
    if (size == 0) return nullptr;

    // Try current block first
    if (current_block_ < blocks_.size()) {
        auto& block = blocks_[current_block_];
        // Align the current position
        size_t offset = block.used;
        size_t aligned = (offset + alignment - 1) & ~(alignment - 1);
        if (aligned + size <= block.size) {
            block.used = aligned + size;
            return block.data.get() + aligned;
        }
    }

    // Try subsequent existing blocks (from a previous reset)
    for (size_t i = current_block_ + 1; i < blocks_.size(); ++i) {
        auto& block = blocks_[i];
        block.used = 0;
        size_t aligned = (alignment - 1) & ~(alignment - 1);  // align from 0
        if (aligned + size <= block.size) {
            block.used = aligned + size;
            current_block_ = i;
            return block.data.get() + aligned;
        }
    }

    // Need a new block
    size_t needed = std::max(block_size_, size + alignment);
    addBlock(needed);
    current_block_ = blocks_.size() - 1;

    auto& block = blocks_.back();
    size_t aligned = 0;  // fresh block, already aligned
    block.used = aligned + size;
    return block.data.get() + aligned;
}

void* Arena::allocateZeroed(size_t size, size_t alignment) {
    void* ptr = allocate(size, alignment);
    if (ptr) {
        std::memset(ptr, 0, size);
    }
    return ptr;
}

void Arena::reset() {
    for (auto& block : blocks_) {
        block.used = 0;
    }
    current_block_ = 0;
}

size_t Arena::bytesUsed() const {
    size_t total = 0;
    for (const auto& block : blocks_) {
        total += block.used;
    }
    return total;
}

size_t Arena::bytesAllocated() const {
    size_t total = 0;
    for (const auto& block : blocks_) {
        total += block.size;
    }
    return total;
}

size_t Arena::blockCount() const {
    return blocks_.size();
}

void Arena::addBlock(size_t min_size) {
    Block block;
    block.size = min_size;
    block.used = 0;
    block.data = std::make_unique<std::byte[]>(min_size);
    blocks_.push_back(std::move(block));
}

}  // namespace enki
