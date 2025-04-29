#pragma once

#include "growing_mem_pool.h"

/**
 * This class implements a memory pool designed to allocate memory for the
 * wavefronts of the Theseus aligner. The pool is going to allocate a chunk per
 * wavefront in a list. The pool does not allow to deallocate memory, but it
 * allows to clear the memory pool, calling clear() so that all the allocated
 * chunks can be reused. If any of the chunks cannot be reused due to its size
 * after calling clear() several times, the chunk will be deallocated and a new
 * chunk will be created.
 */

namespace theseus {

class WavefrontMemPool : public GrowingMemPool {
public:
    /**
     * Construct a wavefront memory pool. Create a chunk without memory to
     * make it easier to allocate memory.
     *
     */
    WavefrontMemPool() noexcept {
        _first_chunk = new Chunk{0, 0, nullptr, nullptr};
        _curr_chunk = _first_chunk;
    };

    /**
     * Deallocate all the memory.
     *
     */
    ~WavefrontMemPool() noexcept {
        Chunk *chunk = _first_chunk;
        while (chunk != nullptr) {
            Chunk *next = chunk->next;

            ::operator delete(static_cast<void *>(chunk->data));
            delete chunk;

            chunk = next;
        }
    };

    /**
     * Clear the pool without deallocating the memory. The old memory will be
     * used for new allocations if possible.
     *
     */
    void clear() override { _curr_chunk = _first_chunk; }

    // Do not allow copy.
    WavefrontMemPool(const WavefrontMemPool &) = delete;
    WavefrontMemPool &operator=(const WavefrontMemPool &) = delete;

    /**
     * Allocate memory for @p nbytes bytes.
     * Use previously allocated chunks if possible.
     *
     * @param nbytes The number of bytes to allocate.
     * @return void* A pointer to the allocated memory.
     */
    void *allocate(std::size_t nbytes) override {
        constexpr double size_factor = 1.5; // Allocate 50% more.
        const std::size_t alloc_bytes = nbytes * size_factor;

        // Find a chunk that has enough space.
        // curr_chunk now points to the last used chunk.
        bool chunk_found = false;
        while (_curr_chunk->next != nullptr) {
            _curr_chunk = _curr_chunk->next;

            // This chunk should be deleted and reused.
            if (_curr_chunk->unused_count >= max_unused_count) {
                chunk_found = true;

                ::operator delete(static_cast<void *>(_curr_chunk->data));

                _curr_chunk->total_bytes = alloc_bytes;
                _curr_chunk->unused_count = 0;
                _curr_chunk->data = static_cast<std::byte *>(::operator new(alloc_bytes));

                break;
            }
            // Chunk found.
            else if (_curr_chunk->total_bytes >= nbytes) {
                chunk_found = true;

                break;
            }
            // This chunk can not be used.
            else {
                _curr_chunk->unused_count++;
            }
        }

        constexpr double too_big_factor = 3.0;

        // We need a new chunk.
        if (!chunk_found) {
            Chunk *new_chunk = new Chunk{alloc_bytes,
                                         0,
                                         static_cast<std::byte *>(::operator new(alloc_bytes)),
                                         nullptr};

            _curr_chunk->next = new_chunk;
            _curr_chunk = new_chunk;
        }
        // In case the chunk uses too much memory, increase the unused count.
        else if (_curr_chunk->total_bytes > nbytes * too_big_factor) {
            _curr_chunk->unused_count++;
        }

        return _curr_chunk->data;
    }

private:
    static constexpr int max_unused_count = 5;

    struct Chunk {
        std::size_t total_bytes;
        int unused_count;
        std::byte *data;

        Chunk *next;
    };

    Chunk *_first_chunk;
    Chunk *_curr_chunk;
};

} // namespace theseus