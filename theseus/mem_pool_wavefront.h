#pragma once

#include <assert.h>

#include <limits>
#include <list>

#include "mem_pool.h"

/**
 * This class implements a memory pool designed to allocate memory for the
 * wavefronts of the Theseus aligner. The pool is going to allocate a chunk per
 * wavefront in a list. The pool implements clear() so that all the allocated
 * chunks can be reused.
 */

namespace theseus {

class MemPoolWavefront : public MemPool {
public:
    /**
     * Construct a wavefront memory pool. Create a chunk without memory so that
     * the allocate() code is simpler.
     *
     */
    MemPoolWavefront() noexcept : _nchunks_acc(0), _nclears(0) {
        _chunks.push_back(Chunk{0, 0, nullptr});
        _curr_chunk = _chunks.begin();
    };

    /**
     * Deallocate all the memory.
     *
     */
    ~MemPoolWavefront() noexcept {
        for (auto &chunk : _chunks) {
            ::operator delete(static_cast<void *>(chunk.data));
        }
    };

    /**
     * Clear the pool without deallocating the memory. The old memory will be
     * used for new allocations if possible. In case we are storing too many
     * wavefronts given the average size of the list, we will remove the tail
     * chunks.
     *
     */
    void clear() override {
        if (_nclears == nclears_to_reset) {
            _nchunks_acc = 0;
        }

        ++_nclears;
        _nchunks_acc += _chunks.size();

        std::ptrdiff_t nchunks_stored = nchunks_over_avg_factor * _nchunks_acc /
                                        _nclears;

        if (nchunks_stored < 1) {
            nchunks_stored = 1;
        }

        // Remove tail chunks until we have nchunks_stored chunks.
        while (std::ssize(_chunks) > nchunks_stored) {
            ::operator delete(static_cast<void *>(_chunks.back().data));
            _chunks.pop_back();
        }

        _curr_chunk = _chunks.begin();
    }

    // Do not allow copy.
    MemPoolWavefront(const MemPoolWavefront &) = delete;
    MemPoolWavefront &operator=(const MemPoolWavefront &) = delete;

    /**
     * Allocate memory for @p nbytes bytes.
     * Use previously allocated chunks if possible.
     *
     * @param nbytes The number of bytes to allocate.
     * @return void* A pointer to the allocated memory.
     */
    void *allocate(std::size_t nbytes) override {
        assert(nbytes <= std::numeric_limits<std::size_t>::max() / size_factor);
        const std::size_t alloc_bytes = nbytes * size_factor;

        assert(nbytes <= std::numeric_limits<std::size_t>::max() / too_big_factor);
        const std::size_t too_many_bytes = nbytes * too_big_factor;

        _curr_chunk = ++_curr_chunk;

        // We need to allocate a new chunk.
        if (_curr_chunk == _chunks.end()) {
            Chunk new_chunk = Chunk{alloc_bytes,
                                    0,
                                    static_cast<std::byte *>(::operator new(alloc_bytes))};

            _chunks.push_back(std::move(new_chunk));
            _curr_chunk = --_chunks.end();
        }
        // We need to reallocate the data of the current chunk.
        else if (_curr_chunk->total_bytes < nbytes ||
                 (_curr_chunk->total_bytes > nbytes * too_big_factor &&
                  _curr_chunk->too_big_count > max_too_big_count)) {

            ::operator delete(static_cast<void *>(_curr_chunk->data));

            _curr_chunk->total_bytes = alloc_bytes;
            _curr_chunk->too_big_count = 0;
            _curr_chunk->data = static_cast<std::byte *>(::operator new(alloc_bytes));
        }
        // Increase the too_big_count.
        else if (_curr_chunk->total_bytes > too_many_bytes) {
            _curr_chunk->too_big_count++;
        }
        // Decrease the too_big_count.
        else if (_curr_chunk->total_bytes <= too_many_bytes &&
                 _curr_chunk->too_big_count > 0) {
            _curr_chunk->too_big_count--;
        }

        return _curr_chunk->data;
    }

    /**
     * This memory pool does not deallocate memory.
     */
    void deallocate([[maybe_unused]] void *ptr, [[maybe_unused]] std::size_t nbytes) override {
        // Do nothing.
    }

    /**
     * Memory allocated by this pool can be deallocated by other pool.
     *
     * @param other Another memory pool.
     */
    bool operator==(const MemPool &other) override { return true; }
    bool operator!=(const MemPool &other) override { return false; }

private:
    static constexpr int max_too_big_count = 10;
    static constexpr double too_big_factor = 3.0;

    static constexpr double size_factor = 1.0;

    // Reset _nclears every nclears_to_reset calls to clear().
    static constexpr std::ptrdiff_t nclears_to_reset = 1000;
    // Store up to nchunks_over_avg_factor times the average size of the chunks
    static constexpr double nchunks_over_avg_factor = 1.5;

    struct Chunk {
        std::size_t total_bytes;
        int too_big_count;
        std::byte *data;
    };

    std::list<Chunk> _chunks;
    std::list<Chunk>::iterator _curr_chunk;

    std::ptrdiff_t _nchunks_acc;
    std::ptrdiff_t _nclears;
};

}   // namespace theseus