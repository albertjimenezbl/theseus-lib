#pragma once

/**
 * Memory pool interface.
 *
 * It is not guaranteed that a MemPool works in multi-threaded applications.
 */

namespace theseus {

class MemPool {
public:
    using propagate_on_container_move_assignment = std::true_type;

    /**
     * Allocate @p nbytes of memory.
     *
     * @param nbytes The number of bytes to allocate.
     * @return A pointer to the allocated memory.
     */
    virtual void *allocate(std::size_t nbytes) = 0;

    /**
     * Deallocate @p nbytes of memory from @p ptr.
     *
     * @param ptr The pointer to the memory to deallocate.
     * @param nbytes The number of bytes to deallocate.
     */
    virtual void deallocate(void *ptr, std::size_t nbytes) = 0;

    /**
     * Reallocate @p ptr to @p new_size bytes.
     *
     * @param ptr The pointer to the memory to reallocate.
     * @param new_size The new size of the memory.
     * @return A pointer to the reallocated memory.
     */
    // virtual void *reallocate(void *ptr, std::size_t new_size) = 0;

    /**
     * Clear the memory pool so that all the allocated data can be reused.
     *
     */
    virtual void clear() = 0;

    /**
     * True if other can deallocate the memory allocated by this memory pool.
     *
     * We allow allocate() and clear() to deallocate memory, so for safety
     * we only allow this if the two memory pools are the same.
     *
     * @param other Another memory pool.
     * @return bool True if the other memory pool can deallocate the memory
     * allocated by this memory pool.
     */
    virtual bool operator==(const MemPool &other) = 0;

    /**
     * The inverse to == operator.
     *
     * @param other Another memory pool.
     * @return bool True if the other memory pool cannot deallocate the memory
     * allocated by this memory pool.
     */
    virtual bool operator!=(const MemPool &other) = 0;

    /**
     * Virtual class destructor.
     *
     */
    virtual ~MemPool() = default;
};

} // namespace theseus