#pragma once

#include <assert.h>

#include <functional>
#include <iostream>
#include <stdexcept>

/**
 * Dynamic-resizable vector with segmented dynamic-allocated storage. The
 * segmented vector has an allocated capacity that can be different from the
 * size of the segmented vector. The segmented vector can be resized without
 * incurring in reallocation as long as the size is less than or equal to the
 * capacity. Additionally, the segmented vector can be reallocated to a new
 * capacity.
 *
 * The segmented vector is divided into segments of a given size at compile
 * time. Illustration of a segmented vector with capacity 3072 elements, an a
 * size of 2148 elements. The segment size is 1024 elements.
 *
 * |    s1     |    s2     |    s3     |
 *       |           |           |
 *       v           v           v
 * | 1024 el   | 1024 el   | 1024 el   |
 * | all used  | all used  | 100 used  |
 *
 * s* are pointers to the segments. el = elements.
 *
 * IMPORTANT: This container applies optimizations for power of 2 segment
 * sizes. Any segment size > 0 is allowed, but the performance may be degraded
 * for non-power of 2 segment sizes.
 *
 * By default, the vector must be manually reallocated to a new capacity if
 * required. However, the user can provide a reallocation policy that will be
 * used to reallocate the vector when the required size is greater than
 * the current capacity. The reallocation policy must be a callable object that
 * takes the current capacity (size_type) and the required size (size_type) as
 * arguments and returns the new capacity (size_type). It is assumed that the
 * new capacity provided by the policy is greater or equal to the required size.
 *
 * Note that when increasing the capacity, the elements are not reallocated,
 * only the vector that stores the pointers to the segments is reallocated.
 * Additionally since the elements are never reallocated (when increasing the
 * capacity), this is a stable container, i.e., none of the iterators or
 * references to the elements are invalidated, except the end() iterator that is
 * invalidated in some operations.
 *
 * In case the elements stored in the segmented vector are both standard layout
 * and trivial, the template parameter AvoidInitIfPossible can be set to true to
 * avoid calling the default constructor and destructor when resizing the
 * segmented vector. This improves the performance of the segmented vector but
 * the value of the elements is not guaranteed to be the same as the default
 * value of the type.
 *
 * @tparam T The type of the elements stored in the segmented vector.
 * @tparam N The size of the segments in the segmented vector. The segment
 * size must be greater than 0. IMPORTANT: This container applies optimizations
 * for power of 2 segment sizes.
 * @tparam AvoidInitIfPossible If true, the default constructor and
 * destructor are avoided when T is standard layout and trivial. This improves
 * the performance of the vector but the value of the elements is not guaranteed
 * to be the same as the default value of the type. If false, the default
 * constructor and destructor are always called when resizing the vector.
 * @tparam Allocator The type of the allocator used by the vector.
 */

namespace theseus {

template <typename T,
          std::ptrdiff_t N,
          bool AvoidInitIfPossible = false,
          typename Allocator = std::allocator<T>>
class SegmentedVector {
public:
    template <bool IsConst>
    class Iterator {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using iterator_concept = std::random_access_iterator_tag;
        using value_type = T;
        using size_type = std::ptrdiff_t;
        using difference_type = std::ptrdiff_t;
        using pointer = std::conditional_t<IsConst, const T*, T*>;
        using reference = std::conditional_t<IsConst, const T&, T&>;
        using segmented_vector_pointer =
            std::conditional_t<IsConst, const SegmentedVector*, SegmentedVector*>;

        /**
         * Default constructor.
         *
         */
        Iterator() noexcept = default;

        /**
         * Construct an iterator with a given pointer to the segmented vector
         * and the index of the element.
         *
         * @param svector A pointer to the segmented vector.
         * @param idx The index of the element.
         */
        Iterator(segmented_vector_pointer svector, size_type idx) noexcept
            : _svector(svector), _idx(idx) {}

        /**
         * Pre-increment operator.
         *
         * @return Reference to the current iterator.
         */
        Iterator &operator++() noexcept {
            ++_idx;
            return *this;
        }

        /**
         * Post-increment operator.
         *
         * @return Copy of the iterator before incrementing.
         */
        Iterator operator++(int) noexcept {
            Iterator temp = *this;
            ++(*this);
            return temp;
        }

        /**
         * Pre-decrement operator.
         *
         * @return Reference to the current iterator.
         */
        Iterator &operator--() noexcept {
            --_idx;

            return *this;
        }

        /**
         * Post-decrement operator.
         *
         * @return Copy of the iterator before decrementing.
         */
        Iterator operator--(int) noexcept {
            Iterator tmp = *this;
            --(*this);
            return tmp;
        }

        /**
         * Addition operator: it += n.
         *
         * We cannot optimize for power2 segment sizes here as n may be negative.
         *
         * @param n The number of elements to add.
         * @return A new iterator with the added value.
         */
        Iterator &operator+=(difference_type n) noexcept {
            _idx += n;
            return *this;
        }

        // Version based on pointer arithmetic.
        // Iterator &operator+=(difference_type n) noexcept {
        //     const difference_type offset = _element - *_segment + n;

        //     // Optimize for this case.
        //     if (offset >= 0 && offset < segment_size) {
        //         _element += n;
        //     }
        //     else {
        //         const auto segment_offset = (offset > 0) ?
        //                                     offset / segment_size :
        //                                     -((-offset - 1) / segment_size) - 1;

        //         _segment += segment_offset;
        //         _element = *_segment + (offset - segment_offset * segment_size);
        //     }

        //     return *this;
        // }

        /**
         * Subtraction operator: it -= n.
         *
         * We cannot optimize for power2 segment sizes here as n may be positive.
         *
         * @param n The number of elements to subtract.
         * @return A new iterator with the subtracted value.
         */
        Iterator &operator-=(difference_type n) noexcept {
            _idx -= n;
            return *this;
        }

        /**
         * Addition operator: it + n.
         *
         * This calls operator+=.
         *
         * @param n The number of elements to add.
         * @return A new iterator with the added value.
         */
        Iterator operator+(difference_type n) const noexcept {
            Iterator tmp = *this;
            tmp += n;
            return tmp;
        }

        /**
         * Subtraction operator: it - n.
         *
         * This calls operator-=.
         *
         * @param n The number of elements to subtract.
         * @return A new iterator with the subtracted value.
         */
        Iterator operator-(difference_type n) const noexcept {
            Iterator tmp = *this;
            tmp -= n;
            return tmp;
        }

        /**
         * Addition operator: n + it.
         *
         * This calls operator+.
         *
         * @param n The number of elements to add.
         * @param it The iterator to add to.
         * @return A new iterator with the added value.
         */
        friend Iterator operator+(difference_type n, const Iterator &it) noexcept {
            return it + n;
        }

        /**
         * Difference operator.
         *
         * @param x The first iterator.
         * @param y The second iterator.
         * @return The difference between the two iterators.
         */
        friend difference_type operator-(const Iterator &x,
                                         const Iterator &y) noexcept {
            return x._idx - y._idx;
        }

        /**
         * Dereference operator.
         *
         * @return Reference to the element pointed by the iterator.
         */
        reference operator*() const noexcept { return (*_svector)[_idx]; }

        /**
         * Member access operator.
         *
         * @return Pointer to the element pointed by the iterator.
         */
        pointer operator->() const noexcept { return &(*_svector)[_idx]; }

        /**
         * Subscript operator.
         *
         * This calls operator+.
         *
         * @param idx The index of the element to access with respect to the
         * iterator.
         * @return Reference to the element at the given index.
         */
        reference operator[] (difference_type idx) const noexcept {
            auto temp = *this;
            temp += idx;
            return *temp;
        }

        /**
         * Equality operator.
         *
         * @param x The first iterator.
         * @param y The second iterator.
         * @return True if the two iterators are equal, false otherwise.
         */
        friend bool operator==(const Iterator &x, const Iterator &y) noexcept {
            return x._idx == y._idx;
        }

        /**
         * Inequality operator.
         *
         * @param x The first iterator.
         * @param y The second iterator.
         * @return True if the two iterators are not equal, false otherwise.
         */
        friend bool operator!=(const Iterator &x, const Iterator &y) noexcept {
            return !(x == y);
        }

        /**
         * Less than operator.
         *
         * @param x The first iterator.
         * @param y The second iterator.
         * @return True if the first iterator is less than the second, false
         * otherwise.
         */
        friend bool operator<(const Iterator &x, const Iterator &y) noexcept {
            return x._idx < y._idx;
        }

        /**
         * Greater than operator.
         *
         * @param x The first iterator.
         * @param y The second iterator.
         * @return True if the first iterator is greater than the second, false
         * otherwise.
         */
        friend bool operator>(const Iterator &x, const Iterator &y) noexcept {
            return y < x;
        }

        /**
         * Less than or equal to operator.
         *
         * @param x The first iterator.
         * @param y The second iterator.
         * @return True if the first iterator is less than or equal to the
         * second, false otherwise.
         */
        friend bool operator<=(const Iterator &x, const Iterator &y) noexcept {
            return !(y < x);
        }

        /**
         * Greater than or equal to operator.
         *
         * @param x The first iterator.
         * @param y The second iterator.
         * @return True if the first iterator is greater than or equal to the
         * second, false otherwise.
         */
        friend bool operator>=(const Iterator &x, const Iterator &y) noexcept {
            return !(x < y);
        }
    private:
        // Pointer based iterator is slower and not stable.
        // T** _segment = nullptr;
        // T* _element = nullptr;
        segmented_vector_pointer _svector = nullptr;
        size_type _idx = 0;
    };
    static_assert(std::random_access_iterator<Iterator<false>>);

    static constexpr auto segment_size = N;
    static_assert(segment_size > 0, "Segment size must be greater than 0");

    using value_type = T;
    using allocator_type = Allocator;
    using size_type = std::ptrdiff_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using alloc_traits = std::allocator_traits<allocator_type>;
    using pointer = typename alloc_traits::pointer;
    using const_pointer = typename alloc_traits::const_pointer;
    using iterator = Iterator<false>;
    using const_iterator = Iterator<true>;
    using reverse_iterator = std::reverse_iterator<Iterator<false>>;
    using const_reverse_iterator = std::reverse_iterator<Iterator<true>>;

    using pointer_allocator = typename alloc_traits::template rebind_alloc<T*>;
    using pointer_alloc_traits = std::allocator_traits<pointer_allocator>;

    using realloc_policy = std::function<size_type(size_type, size_type)>;

    /**
     * Create an empty segmented vector. Both the size and capacity are 0 and
     * there is no reallocation policy. The default allocator is used.
     *
     */
    SegmentedVector() noexcept(noexcept(Allocator()))
        : SegmentedVector(Allocator()) {}

    /**
     * Create an empty segmented vector with a given allocator @p alloc. Both
     * the size and capacity are 0 and there is no reallocation policy.
     *
     * @param alloc The allocator to use.
     */
    explicit SegmentedVector(const Allocator &alloc) noexcept : _alloc(alloc) {}

    /**
     * Create a segmented vector with a given size @p size and optional
     * allocator @p alloc. The capacity is equal to the size and there is no
     * reallocation policy.
     *
     * @param size The size of the segmented vector.
     * @param alloc The allocator to use.
     */
    SegmentedVector(size_type size, const Allocator &alloc = Allocator())
        : _alloc(alloc) {

        if (size < 0) {
            throw std::length_error("SegmentedVector: size < 0");
        }

        realloc(size);
        _size = size;
        default_construct_elements(begin(), end());
    }

    /**
     * Create a segmented vector with a given size @p size and initialize all
     * elements to a given value @p value. The capacity is equal to the size and
     * there is no reallocation policy. The constructor takes an optional
     * allocator @p alloc.
     *
     * @param size The size of the segmented vector.
     * @param value The value to copy initialize all elements of the segmented
     * vector.
     * @param alloc The allocator to use (optional).
     */
    SegmentedVector(size_type size,
                    const T &value,
                    const Allocator &alloc = Allocator())
        : _alloc(alloc) {

        if (size < 0) {
            throw std::length_error("SegmentedVector: size < 0");
        }

        realloc(size);
        _size = size;
        copy_construct_elements(begin(), end(), value);
    }

    /**
     * Copy constructor. The reallocation policy is also copied, while the
     * behavior of the allocator is determined by the
     * propagate_on_container_copy_construction property of the allocator.
     *
     * @param other Source segmented vector to copy.
     */
    SegmentedVector(const SegmentedVector &other)
        : _realloc_policy(other._realloc_policy),
          _alloc(alloc_traits::select_on_container_copy_construction(other._alloc)) {

        realloc(other.capacity());
        _size = other.size();
        copy_construct_elements(begin(), end(), other.begin());
    }

    /**
     * Move constructor.
     *
     * @param other Source segmented vector to move.
     */
    SegmentedVector(SegmentedVector &&other) noexcept
        : _realloc_policy(std::move(other._realloc_policy)),
          _alloc(std::move(other._alloc)),
          _size(other._size),
          _capacity(other._capacity),
          _data(other._data) {

        other._size = 0;
        other._capacity = 0;
        other._data = nullptr;
    }

    /**
     * Copy assignment operator. The reallocation policy is also copied, while
     * the behavior of the allocator is determined by the
     * propagate_on_container_copy_assignment property of the allocator.
     *
     * @param other Source segmented vector to copy.
     * @return Reference to the current segmented vector.
     */
    SegmentedVector &operator=(const SegmentedVector &other) {
        if (this == &other) {
            return *this;
        }

        _realloc_policy = other._realloc_policy;

        destroy_elements(begin(), end());
        _size = 0;

        constexpr bool propagate_alloc =
            alloc_traits::propagate_on_container_copy_assignment::value;
        const bool reallocate = (propagate_alloc && _alloc != other._alloc);

        // Need to reallocate everything.
        if (reallocate) {
            realloc(0);
        }

        if constexpr (propagate_alloc) {
            _alloc = other._alloc;
        }

        if (reallocate || capacity() != other.capacity()) {
            realloc(other.capacity());
        }

        _size = other.size();

        copy_construct_elements(begin(), end(), other.begin());

        return *this;
    }

    /**
     * Move assignment operator. The reallocation policy is also moved while the
     * behavior of the allocator is determined by the
     * propagate_on_container_move_assignment property of the allocator.
     *
     * @param other Source segmented vector to move.
     * @return Reference to the current segmented vector.
     */
    SegmentedVector &operator=(SegmentedVector &&other) noexcept(noexcept_move_assign) {

        if (this == &other) {
            return *this;
        }

        destroy_elements(begin(), end());
        _size = 0;

        // We can safely move the data.
        if constexpr (noexcept_move_assign) {
            realloc(0);
            this->swap(other);
        }
        else {
            // We can safely move the data.
            if (_alloc != other._alloc) {
                realloc(0);
                this->swap(other);
            }
            // We need to copy the elements using the current allocator.
            else {
                _realloc_policy = other._realloc_policy;

                if (capacity() != other.capacity()) {
                    realloc(other.capacity());
                }

                _size = other.size();

                move_construct_elements(begin(), end(), other.begin());
            }
        }

        return *this;
    }

    /**
     * Destructor.
     *
     */
    ~SegmentedVector() {
        destroy_elements(begin(), end());
        _size = 0;
        realloc(0);
    }

    /**
     * Reallocate the segmented vector to a new capacity. The new capacity must
     * be greater than or equal to the current size of the segmented vector and
     * greater than or equal to 0. The new capacity will be greater than or
     * equal to the requested capacity.
     *
     * The iterators and pointer to the elements remain valid after calling this
     * function.
     *
     * @param new_capacity The new capacity of the segmented vector.
     */
    void realloc(size_type new_capacity) {
        const size_type old_size = size();
        const size_type min_nsegments = to_nsegments(old_size);

        const size_type old_nsegments = to_nsegments(capacity());
        const size_type new_nsegments = to_nsegments(new_capacity);

        if (new_nsegments == old_nsegments) {
            return;
        }
        else if (new_capacity < 0) {
            throw std::length_error("SegmentedVector: new_capacity < 0");
        }
        else if (new_nsegments < min_nsegments) {
            throw std::length_error("SegmentedVector: new_capacity < _size");
        }

        T** new_vector_of_ptrs = allocate_vector_of_ptr_to_segment(new_nsegments);

        if (new_nsegments < old_nsegments) {
            // Deallocate unreachable segments.
            for (size_type i = new_nsegments; i < old_nsegments; ++i) {
                deallocate_segment(&_data[i]);
            }

            // Copy the rest of the pointers.
            for (size_type i = 0; i < new_nsegments; ++i) {
                new_vector_of_ptrs[i] = _data[i];
            }
        }
        else {
            // Copy the old ptrs.
            for (size_type i = 0; i < old_nsegments; ++i) {
                new_vector_of_ptrs[i] = _data[i];
            }

            // Allocate the new segments.
            for (size_type i = old_nsegments; i < new_nsegments; ++i) {
                new_vector_of_ptrs[i] = allocate_segment();
            }
        }

        // Deallocate old vector_of_ptrs.
        deallocate_vector_of_ptr_to_segment(&_data, old_nsegments);

        // Assign the new vector_of_ptrs.
        _data = new_vector_of_ptrs;

        _capacity = new_nsegments * segment_size;
    }

    /**
     * Resize the segmented vector to a new size. The new size must be greater
     * than or equal to 0. If the segmented vector has a realloc_policy and the
     * new size is greater than the current capacity, the segmented vector will
     * be reallocated using the policy. Otherwise, a std::length_error will be
     * thrown.
     *
     * All iterators and references to the elements remain valid after calling
     * this function. The end() iterator is invalidated.
     *
     * @param new_size The new size of the segmented vector.
     */
    void resize(size_type new_size) {
        resize_prepare(new_size);
        resize_unsafe(new_size);
    }

    /**
     * Resize the segmented vector to a new size and copy initialize all new
     * elements to a given value. The new size must be greater than or equal to
     * 0. If the segmented vector has a realloc_policy and the new size is
     * greater than the current capacity, the segmented vector will be
     * reallocated using the policy. Otherwise, a std::length_error will be
     * thrown.
     *
     * All iterators and references to the elements remain valid after calling
     * this function. The end() iterator is invalidated.
     *
     * @param new_size The new size of the segmented vector.
     * @param value The value to copy initialize all new elements of the
     * segmented vector.
     */
    void resize(size_type new_size, const T &value) {
        resize_prepare(new_size);
        resize_unsafe(new_size, value);
    }

    /**
     * Resize the segmented vector without checking if the new size is valid or
     * if the new size is greater than the current capacity. This only works if
     * the capacity is greater than or equal to the new size.
     *
     * All iterators and references to the elements remain valid after calling
     * this function. The end() iterator is invalidated.
     *
     * @param new_size The new size of the segmented vector.
     */
    void resize_unsafe(size_type new_size) {
        const size_type old_size = size();

        if (new_size > old_size) {
            default_construct_elements(begin() + old_size, begin() + new_size);
        }
        else if (new_size < old_size) {
            destroy_elements(begin() + new_size, end());
        }

        _size = new_size;
    }

    /**
     * Resize the segmented vector without checking if the new size is valid or
     * if the new size is greater than the current capacity. Copy initialize all
     * new elements to a given value. This only works if the capacity is greater
     * than or equal to the new size.
     *
     * All iterators and references to the elements remain valid after calling
     * this function. The end() iterator is invalidated.
     *
     * @param new_size The new size of the segmented vector.
     * @param value The value to copy initialize all new elements of the
     * segmented vector.
     */
    void resize_unsafe(size_type new_size, const T &value) {
        const size_type old_size = size();

        if (new_size > old_size) {
            copy_construct_elements(begin() + old_size, begin() + new_size, value);
        }
        else if (new_size < old_size) {
            destroy_elements(begin() + new_size, end());
        }

        _size = new_size;
    }

    /**
     * Erases all elements from the container. After this call, size() returns
     * zero. The capacity of the segmented vector is not changed.
     *
     */
    void clear() {
        destroy_elements(begin(), end());
        _size = 0;
    }

    /**
     * Add a value to the end of the segmented vector. If the size is greater
     * than the capacity and the segmented vector has a realloc policy, the
     * segmented vector will be reallocated using the policy. Otherwise, a
     * std::length_error will be thrown.
     *
     * All iterators and references to the elements remain valid after calling
     * this function. The end() iterator is invalidated.
     *
     * @param value The value to add (copy) to the end of the segmented vector.
     */
    void push_back(const T &value) {
        add_back_prepare();
        push_back_unsafe(value);
    }

    /**
     * Move a value to the end of the segmented vector. If the size is greater
     * than the capacity and the segmented vector has a realloc policy, the
     * segmented vector will be reallocated using the policy. Otherwise, a
     * std::length_error will be thrown.
     *
     * All iterators and references to the elements remain valid after calling
     * this function. The end() iterator is invalidated.
     *
     * @param value The value to move to the end of the segmented vector.
     */
    void push_back(T &&value) {
        add_back_prepare();
        push_back_unsafe(std::move(value));
    }

    /**
     * Add a value to the end of the segmented vector without boundary checking.
     * This only works if the capacity is greater than or equal to the new size.
     *
     * All iterators and references to the elements remain valid after calling
     * this function. The end() iterator is invalidated.
     *
     * @param value The value to add to the end of the segmented vector.
     */
    void push_back_unsafe(const T &value) { emplace_back_unsafe(value); }

    /**
     * Move a value to the end of the segmented vector without boundary
     * checking. This only works if the capacity is greater than or equal to the
     * new size.
     *
     * All iterators and references to the elements remain valid after calling
     * this function. The end() iterator is invalidated.
     *
     * @param value The value to move to the end of the segmented vector.
     */
    void push_back_unsafe(T &&value) { emplace_back_unsafe(std::move(value)); }

    /**
     * Construct an element at the end of the segmented vector using the given
     * arguments. If the size is greater than the capacity and the segmented
     * vector has a realloc policy, the segmented vector will be reallocated
     * using the policy.
     *
     * All iterators and references to the elements remain valid after calling
     * this function. The end() iterator is invalidated.
     *
     * @tparam Args The types of the arguments to construct the element.
     * @param args The arguments to construct the element.
     */
    template <class... Args>
    void emplace_back(Args &&...args) {
        add_back_prepare();
        emplace_back_unsafe(std::forward<Args>(args)...);
    }

    /**
     * Construct an element at the end of the segmented vector using the given
     * arguments without boundary checking. This only works if the capacity is
     * greater than or equal to the new size.
     *
     * All iterators and references to the elements remain valid after calling
     * this function. The end() iterator is invalidated.
     *
     * @tparam Args The types of the arguments to construct the element.
     * @param args The arguments to construct the element.
     */
    template <class... Args>
    void emplace_back_unsafe(Args &&...args) {
        args_construct_element(&(*this)[_size], std::forward<Args>(args)...);
        ++_size;
    }

    /**
     * Remove the last element of the segmented vector. If the segmented vector
     * is empty, a std::out_of_range exception is thrown.
     *
     * All iterators and references to the elements remain valid after calling
     * this function. The end() iterator is invalidated.
     *
     */
    void pop_back() {
        if (_size == 0) {
            throw std::out_of_range(
                "SegmentedVector: pop_back on empty segmented vector");
        }
        pop_back_unsafe();
    }

    /**
     * Remove the last element of the segmented vector without boundary
     * checking. This only works if the segmented vector is not empty.
     *
     * All iterators and references to the elements remain valid after calling
     * this function. The end() iterator is invalidated.
     *
     */
    void pop_back_unsafe() noexcept(avoid_init) {
        --_size;
        destroy_elements(begin() + _size, begin() + _size + 1);
    }

    /**
     * Set the reallocation policy. The policy must be a callable object that
     * takes the current capacity (size_type) and the required size (size_type)
     * as arguments and returns the new capacity (size_type). It is assumed that
     * the new capacity provided by the policy is greater or equal to the
     * required size.
     *
     * A nullptr policy means that the segmented vector will not be reallocated
     * when the required size is greater than the current capacity.
     *
     * @param policy The reallocation policy to set.
     */
    void set_realloc_policy(realloc_policy policy) { _realloc_policy = policy; }

    /**
     * Get the reallocation policy.
     *
     * @return The reallocation policy.
     */
    [[nodiscard]] realloc_policy get_realloc_policy() const noexcept {
        return _realloc_policy;
    }

    /**
     * Get the allocator used by the segmented vector.
     *
     * @return The allocator used by the segmented vector.
     */
    [[nodiscard]] allocator_type get_allocator() const noexcept { return _alloc; }

    /**
     * Get the size of the segmented vector.
     *
     * @return The size of the segmented vector.
     */
    [[nodiscard]] size_type size() const noexcept { return _size; }

    /**
     * Get the capacity of the segmented vector.
     *
     * @return The capacity of the segmented vector.
     */
    [[nodiscard]] size_type capacity() const noexcept { return _capacity; }

    /**
     * Check if the segmented vector is empty, i.e., size() == 0.
     *
     * @return True if the segmented vector is empty, false otherwise.
     */
    [[nodiscard]] bool empty() const noexcept { return size() == 0; }

    /**
     * Get the number of segments that are required to store n elements.
     *
     * @param n A number of elements.
     * @return The number of segments required to store n elements.
     */
    [[nodiscard]] static constexpr size_type to_nsegments(size_type n) noexcept {
        return (n + segment_size - 1) / segment_size;
    }

    /**
     * Access operator with bounds checking. It is assumed that the index is
     * greater than or equal to 0 and less than the size of the segmented vector.
     *
     * @return A reference to the element at the given index.
     */
    [[nodiscard]] T &operator[](size_type idx) {
        assert(_capacity && "SegmentedVector is empty");
        assert(idx >= 0 && "SegmentedVector indexed access requires idx >= 0");

        auto [segment_idx, element_idx] = idx_to_internal_idxs(idx);
        return *(*(_data + segment_idx) + element_idx);
    }
    [[nodiscard]] const T &operator[](size_type idx) const {
        return const_cast<SegmentedVector *>(this)->operator[](idx);
    }

    /**
     * Access operator with bounds checking.
     *
     * @return A reference to the element at the given index.
     */
    [[nodiscard]] T &at(size_type idx) {
        if (idx >= size()) {
            throw std::out_of_range("SegmentedVector: index out of range");
        }
        return (*this)[idx];
    }
    [[nodiscard]] const T &at(size_type idx) const {
        return const_cast<SegmentedVector *>(this)->at(idx);
    }

    /**
     * Reference to the first element of the segmented vector. If the empty() is
     * true, the behavior is undefined.
     *
     * @return Reference to the first element of the segmented vector.
     */
    [[nodiscard]] T &front() {
        assert(_size > 0 && "SegmentedVector is empty");
        return (*this)[0];
    }
    [[nodiscard]] const T &front() const {
        return const_cast<SegmentedVector *>(this)->front();
    }

    /**
     * Reference to the last element of the segmented vector. If the empty() is
     * true, the behavior is undefined.
     *
     * @return Reference to the last element of the segmented vector.
     */
    [[nodiscard]] T &back() {
        assert(_size > 0 && "SegmentedVector is empty");
        return (*this)[_size - 1];
    }
    [[nodiscard]] const T &back() const {
        return const_cast<SegmentedVector *>(this)->back();
    }

    /**
     * Iterator to the beginning of the segmented segmented vector. If empty()
     * is true, the iterator is equal to the end() iterator.
     *
     * @return Iterator to the beginning of the segmented vector.
     */
    [[nodiscard]] iterator begin() noexcept { return iterator(this, 0); }
    [[nodiscard]] const_iterator begin() const noexcept {
        return const_iterator(this, 0);
    }
    [[nodiscard]] const_iterator cbegin() const noexcept { return begin(); }

    /**
     * Iterator past the last element of the segmented vector. If empty() is
     * true, the iterator is equal to the begin() iterator.
     *
     * @return Iterator past the last element of the segmented vector.
     */
    [[nodiscard]] iterator end() noexcept { return iterator(this, _size); }
    [[nodiscard]] const_iterator end() const noexcept {
        return const_iterator(this, _size);
    }
    [[nodiscard]] const_iterator cend() const noexcept { return end(); }

    /**
     * Reverse iterator to the beginning of the segmented vector. If empty() is
     * true, the iterator is equal to the rend() iterator.
     *
     * @return Reverse iterator to the beginning of the segmented vector.
     */
    [[nodiscard]] reverse_iterator rbegin() noexcept {
        return reverse_iterator(end());
    }
    [[nodiscard]] const_reverse_iterator rbegin() const noexcept {
        return reverse_iterator(cend());
    }
    [[nodiscard]] const_reverse_iterator crbegin() const noexcept {
        return reverse_iterator(cend());
    }

    /**
     * Reverse iterator before the first element of the segmented vector. If
     * empty() is true, the iterator is equal to the rbegin() iterator.
     *
     * @return Reverse iterator before the first element of the segmented
     * vector.
     */
    [[nodiscard]] reverse_iterator rend() noexcept {
        return reverse_iterator(begin());
    }
    [[nodiscard]] const_reverse_iterator rend() const noexcept {
        return reverse_iterator(cbegin());
    }
    [[nodiscard]] const_reverse_iterator crend() const noexcept {
        return reverse_iterator(cbegin());
    }

    /**
     * Swap the contents of the segmented vector with another segmented vector.
     * All iterators and references remain valid.
     *
     * @param other The segmented segmented vector to swap with.
     */
    void swap(SegmentedVector &other) noexcept(noexcept_swap) {
        std::swap(_realloc_policy, other._realloc_policy);

        if constexpr (alloc_traits::propagate_on_container_swap::value) {
            std::swap(_alloc, other._alloc);
        }

        std::swap(_data, other._data);
        std::swap(_size, other._size);
        std::swap(_capacity, other._capacity);
    }

    /**
     * Specialized swap function to swap two segmented vectors. This function
     * calls swap() on one of the segmented vectors.
     *
     * @param x The first segmented vector to swap.
     * @param y The second segmented vector to swap.
     */
    friend void swap(SegmentedVector &x,
                     SegmentedVector &y) noexcept(SegmentedVector::noexcept_swap) {
        x.swap(y);
    }

private:
    /**
     * Check if the segmented vector should avoid calling the default
     * constructor and destructor when resizing the segmented vector. I.e.,
     * check if the elements stored in the segmented vector are both standard
     * layout and trivial.
     */
    static constexpr bool avoid_init = AvoidInitIfPossible &&
                                       std::is_standard_layout_v<T> &&
                                       std::is_trivial_v<T>;

    static constexpr bool noexcept_move_assign =
        alloc_traits::propagate_on_container_move_assignment::value ||
        alloc_traits::is_always_equal::value;
    static constexpr bool noexcept_swap =
        alloc_traits::propagate_on_container_swap::value ||
        alloc_traits::is_always_equal::value;

    /**
     * Given an index, return the segment index and the element index within
     * the segment. This is done using bitwise operations if the segment size
     * is a power of 2, otherwise it is done using division and multiplication.
     *
     * @param idx The index to convert.
     * @return The <segment index, element index> pair for the given index.
     */
    std::pair<size_type, size_type> idx_to_internal_idxs(size_type idx) const {
        assert(idx >= 0 && "SegmentedVector indexed access requires idx >= 0");

        static constexpr bool power2_optimizations =
            (segment_size & (segment_size - 1)) == 0;

        if constexpr (power2_optimizations) {
            static constexpr size_type segment_idx_lshift = __builtin_ctz(segment_size);
            static constexpr size_type element_idx_mask = segment_size - 1;

            size_type segment_idx = idx >> segment_idx_lshift;
            size_type element_idx = idx & element_idx_mask;
            return {segment_idx, element_idx};
        }
        else {
            size_type segment_idx = idx / segment_size;
            // Equivalent to idx % segment_size
            size_type element_idx = idx - segment_idx * segment_size;
            return {segment_idx, element_idx};
        }
    }

    /**
     * Allocate a new segment and return a pointer to it.
     *
     * @return A pointer to the new segment.
     */
    T* allocate_segment() {
        T* segment = alloc_traits::allocate(_alloc, segment_size);
        if (!segment) {
            throw std::bad_alloc();
        }
        return segment;
    }

    /**
     * Deallocate a segment.
     *
     * @param segment The segment to deallocate.
     */
    void deallocate_segment(T** segment) {
        alloc_traits::deallocate(_alloc, *segment, segment_size);
        *segment = nullptr;
    }

    /**
     * Allocate a vector of pointers to segments. The size of the vector is
     * equal to the number of segments. If @p nsegments is 0, a nullptr is
     * returned.
     *
     * @param nsegments The number of segments.
     * @return A pointer to the vector of pointers to segments.
     */
    T** allocate_vector_of_ptr_to_segment(size_type nsegments) {
        if (nsegments == 0) {
            return nullptr;
        }

        pointer_allocator ptr_alloc(_alloc);
        T** ptrs = pointer_alloc_traits::allocate(ptr_alloc, nsegments);

        if (!ptrs) {
            throw std::bad_alloc();
        }

        return ptrs;
    }

    /**
     * Deallocate a vector of pointers to segments. Set the pointer to the
     * vector to nullptr.
     *
     * @param ptrs The vector of pointers to segments to deallocate.
     * @param nsegments The number of segments.
     */
    void deallocate_vector_of_ptr_to_segment(T*** ptrs, size_type nsegments) {
        pointer_allocator ptr_alloc(_alloc);
        pointer_alloc_traits::deallocate(ptr_alloc, *ptrs, to_nsegments(nsegments));
        *ptrs = nullptr;
    }

    /**
     * Construct an element at @p pdst using the given arguments @p args.
     *
     * @tparam Args The types of the arguments to construct the element.
     * @param pdst The pointer to the element.
     * @param args The arguments to construct the element.
     */
    template <class... Args>
    void args_construct_element(T *p, Args &&...args) {
        alloc_traits::construct(_alloc,
                                std::to_address(p),
                                std::forward<Args>(args)...);
    }

    /**
     * Default construct @p n elements starting at pointer @p pdst. In case
     * avoid_init is true, this function does nothing.
     *
     * @param pdst The pointer to the first element to initialize.
     * @param n The number of elements to initialize.
     */
    void default_construct_elements(iterator dst_start, iterator dst_end) {
        if constexpr (avoid_init) {
            return;
        }

        while (dst_start != dst_end) {
            alloc_traits::construct(_alloc, std::to_address(dst_start));
            ++dst_start;
        }
    }


    /**
     * Copy construct elements from @p src_start to @p dst_start until @p dst_end.
     *
     * @param dst_start An iterator to the first element to copy construct.
     * @param dst_end An iterator to one element past the last element to
     * copy construct.
     * @param src_start An iterator to the first element to copy construct
     * from.
     */
    void copy_construct_elements(iterator dst_start,
                                 iterator dst_end,
                                 const_iterator src_start) {

        while (dst_start != dst_end) {
            alloc_traits::construct(_alloc, std::to_address(dst_start),
                                    *src_start);

            ++dst_start;
            ++src_start;
        }
    }

    /**
     * Copy construct elements from @p value to @p dst_start until @p dst_end.
     *
     * @param dst_start Iterator to the first element to copy construct.
     * @param dst_end Iterator to one element past the last element to copy
     * construct.
     * @param value The value copy. 
     */
    void copy_construct_elements(iterator dst_start,
                                 iterator dst_end,
                                 const T& value) {

        while (dst_start != dst_end) {
            alloc_traits::construct(_alloc, std::to_address(dst_start), value);

            ++dst_start;
        }
    }

    /**
     * Move construct elements from @p src_start to @p dst_start until @p
     * dst_end.
     *
     * @param dst_start An iterator to the first element to move construct.
     * @param dst_end An iterator to one element past the last element to
     * move construct.
     * @param src_start An iterator to the first element to move construct
     * from.
     */
    void move_construct_elements(iterator dst_start,
                                 iterator dst_end,
                                 const_iterator src_start) {

        while (dst_start != dst_end) {
            alloc_traits::construct(_alloc, std::to_address(dst_start),
                                    std::move(*src_start));
            ++dst_start;
            ++src_start;
        }
    }

    /**
     * Destroy elements from @p start to @p end. In case avoid_init is true,
     * this function does nothing.
     *
     * @param start An iterator to the first element to destroy.
     * @param end An iterator to one element past the last element to destroy.
     */
    void destroy_elements(iterator start, iterator end) noexcept(avoid_init) {
        if constexpr (avoid_init) {
            return;
        }

        while (start != end) {
            alloc_traits::destroy(_alloc, std::to_address(start));
            ++start;
        }
    }

    /**
     * Check if the new size is valid. If the new size is less than 0, a
     * std::length_error is thrown. If the new size is greater than the current
     * capacity and the segmented vector has a realloc_policy, the segmented
     * vector is reallocated using the policy. Otherwise, a std::length_error is
     * thrown.
     *
     * @param new_size The new size of the segmented vector.
     */
    void resize_prepare(size_type new_size) {
        const size_type max_size = capacity();

        if (new_size <= max_size) {
            if (new_size < 0) [[unlikely]] {
                throw std::length_error("SegmentedVector: new_size < 0");
            }
            return;
        }
        else {
            if (_realloc_policy) {
                realloc(_realloc_policy(max_size, new_size));
            }
            else {
                throw std::length_error(
                    "SegmentedVector: capacity < new_size and no realloc "
                    "policy set");
            }
        }
    }

    /**
     * Prepare the segmented vector to be able to add a new element at the end.
     * If the new size is greater than the current capacity and the segmented
     * vector has a realloc_policy, the segmented vector is reallocated using
     * the policy. Otherwise, a std::length_error is thrown.
     *
     */
    void add_back_prepare() {
        // Optimize for this case.
        if (_size < _capacity) [[likely]] {
            return;
        }
        else {
            if (_realloc_policy) {
                realloc(_realloc_policy(_capacity, _capacity + 1));
            }
            else {
                throw std::length_error(
                    "SegmentedVector: capacity < new_size and no realloc "
                    "policy set");
            }
        }
    }

    realloc_policy _realloc_policy = nullptr;
    [[no_unique_address]] allocator_type _alloc;

    size_type _size = 0;
    size_type _capacity = 0;

    T** _data = nullptr;
};

}   // namespace theseus