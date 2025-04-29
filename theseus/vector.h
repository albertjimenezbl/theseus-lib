#pragma once

#include <functional>
#include <iostream>
#include <stdexcept>

/**
 * Dynamic-resizable vector with contiguous dynamic-allocated storage.
 * The vector has an allocated capacity that can be different from the
 * size of the vector. The vector can be resized without incurring in
 * reallocation as long as the size is less than or equal to the capacity.
 * Additionally, the vector can be reallocated to a new capacity.
 *
 * By default, the vector must be manually reallocated to a new capacity if
 * required. However, the user can provide a reallocation policy that will be
 * used to reallocate the vector when the required size is greater than
 * the current capacity. The reallocation policy must be a callable object that
 * takes the current capacity (std::ptrdiff_t) and the required size
 * (std::ptrdiff_t) as arguments and returns the new capacity (std::ptrdiff_t).
 * It is assumed that the new capacity provided by the policy is greater or
 * equal to the required size.
 *
 * If the elements stored in the vector are both standard layout and trivial,
 * calling to the default constructor and destructor is avoided when resizing
 * the vector. This improves the performance of the vector but the value of the
 * elements is not guaranteed to be the same as the default value of the type.
 *
 * @tparam T The type of the elements stored in the vector.
 * @tparam ReallocPolicy The policy used to reallocate the vector. The
 * default policy is nullptr, which means that the vector will not be
 * reallocated. If a policy is provided, it must be a callable object that
 * takes the current capacity (std::ptrdiff_t) and the required size
 * (std::ptrdiff_t) as arguments and returns the new capacity
 * (std::ptrdiff_t). It is assumed that the new capacity provided
 * is greater or equal to the required size.
 * @tparam Allocator The type of the allocator used by the vector.
 */

namespace theseus {

template <typename T, auto ReallocPolicy = nullptr, typename Allocator = std::allocator<T>>
requires(ReallocPolicy == nullptr ||
requires(std::ptrdiff_t c, std::ptrdiff_t s) {
    { ReallocPolicy(c, s) } -> std::same_as<std::ptrdiff_t>;
})
class Vector {
public:
    using allocator_type = Allocator;
    using alloc_traits = std::allocator_traits<allocator_type>;
    using value_type = T;
    using size_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = typename alloc_traits::pointer;
    using const_pointer = typename alloc_traits::const_pointer;

    /**
     * Iterator for the Vector.
     *
     */
    class Iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = const T*;
        using reference = const T&;

        /**
         * Construct an iterator with a given pointer.
         *
         * @param ptr The pointer to the element.
         */
        Iterator(T *ptr) : _ptr(ptr) {}

        /**
         * Pre-increment operator.
         *
         * @return Reference to the current iterator.
         */
        Iterator &operator++() {
            _ptr++;
            return *this;
        }

        /**
         * Post-increment operator.
         *
         * @return Copy of the iterator before incrementing.
         */
        Iterator operator++(int) {
            Iterator temp = *this;
            ++(*this);
            return temp;
        }

        /**
         * Dereference operator.
         *
         * @return Reference to the element pointed by the iterator.
         */
        T &operator*() const { return *_ptr; }

        /**
         * Member access operator.
         *
         * @return Pointer to the element pointed by the iterator.
         */
        T *operator->() { return _ptr; }

        /**
         * Equality operator.
         *
         * @param other The other iterator to compare.
         * @return True if the iterators are equal, false otherwise.
         */
        bool operator==(const Iterator &other) const {
            return _ptr == other._ptr;
        }

        /**
         * Inequality operator.
         *
         * @param other The other iterator to compare.
         * @return True if the iterators are different, false otherwise.
         */
        bool operator!=(const Iterator &other) const {
            return _ptr != other._ptr;
        }

    private:
        T *_ptr;
    };

    /**
     * Create an empty vector. Both the size and capacity are 0.
     *
     */
    Vector()
        : _alloc(Allocator()), _size(0), _capacity(0), _data(nullptr) {
    }

    /**
     * Create an empty vector with a given allocator. Both the size and capacity
     * are 0.
     * 
     * @param alloc The allocator to use.
     */
    explicit Vector(const Allocator& alloc)
        : _alloc(alloc), _size(0), _capacity(0), _data(nullptr) {}

    /**
     * Create an vector with a given size. The capacity is equal to the size.
     *
     * @param size The size of the vector.
     * @param alloc The allocator to use.
     */
    explicit Vector(size_type size, const Allocator& alloc = Allocator())
        : _alloc(alloc), _size(size), _capacity(size), _data(nullptr) {
        if (size < 0) {
            throw std::length_error("Vector: size < 0");
        }

        allocate();

        if constexpr (!avoid_init()) {
            construct_elements(T());
        }
    }

    /**
     * Create an vector with a given size and initialize all elements to a given
     * value. The capacity is equal to the size.
     *
     * @param size The size of the vector.
     * @param value The value to initialize all elements of the vector.
     * @param alloc The allocator to use.
     */
    Vector(size_type size, const T &value, const Allocator& alloc = Allocator())
        : _alloc(alloc), _size(size), _capacity(size), _data(nullptr) {

        if (size < 0) {
            throw std::length_error("Vector: size < 0");
        }

        allocate();
        construct_elements(value);
    }

    /**
     * Copy constructor.
     *
     * @param other Source vector to copy.
     */
    Vector(const Vector<T> &other)
        : _alloc(alloc_traits::select_on_container_copy_construction(other._alloc)),
          _size(other._size), _capacity(other._capacity), _data(nullptr) {

        allocate();
        copy_elements(other._data);
    }

    /**
     * Move constructor.
     *
     * @param other Source vector to move.
     */
    Vector(Vector<T> &&other) noexcept
        : _alloc(std::move(other._alloc)),
          _size(other._size), _capacity(other._capacity), _data(other._data) {

        other._size = 0;
        other._capacity = 0;
        other._data = nullptr;
    }

    /**
     * Copy assignment operator.
     *
     * @param other Source vector to copy.
     * @return Reference to the current vector.
     */
    Vector &operator=(const Vector &other) {
        if (this == &other) {
            return *this;
        }

        if constexpr (!avoid_init()) {
            destroy_elements();
        }

        constexpr bool propagate_alloc = alloc_traits::propagate_on_container_copy_assignment::value;
        const bool reallocate = (propagate_alloc && _alloc != other._alloc) ||
                                _capacity != other._capacity;

        if (reallocate) {
            deallocate();
        }

        if (propagate_alloc) {
            _alloc = other._alloc;
        }

        if (reallocate) {
            _capacity = other._capacity;
            allocate();
        }

        _size = other._size;
        copy_elements(other._data);

        return *this;
    }

    /**
     * Move assignment operator.
     *
     * @param other Source vector to move.
     * @return Reference to the current vector.
     */
    Vector &operator=(Vector &&other) noexcept {
        if (this == &other) {
            return *this;
        }

        if constexpr (!avoid_init()) {
            destroy_elements();
        }

        // We can safely move the data.
        if (alloc_traits::propagate_on_container_move_assignment::value ||
            _alloc == other._alloc) {

            deallocate();

            _alloc = std::move(other._alloc);

            _size = other._size;
            _capacity = other._capacity;
            _data = other._data;

            other._size = 0;
            other._capacity = 0;
            other._data = nullptr;
        }
        // We need to copy the elements using the current allocator.
        else {
            if (_capacity < other._capacity) {
                deallocate();

                _capacity = other._capacity;
                allocate();
            }

            _size = other._size;
            move_elements(other._data);
        }

        return *this;
    }

    /**
     * Destructor.
     *
     */
    ~Vector() {
        if constexpr (!avoid_init()) {
            destroy_elements();
        }
        deallocate();
    }

    /**
     * Reallocate the vector to a new capacity. The new capacity must be greater
     * than or equal to the current size of the vector and greater than or equal
     * to 0.
     *
     * @param new_capacity The new capacity of the vector.
     */
    void realloc(size_type new_capacity) {
        if (new_capacity < 0 || new_capacity < _size) {
            throw std::length_error(
                "Vector: new_capacity < 0 or new_capacity < _size");
        }

        if (new_capacity == _capacity) {
            return;
        }

        T *new_data;
        allocate_ptr(new_capacity, &new_data);

        for (size_type i = 0; i < _size; ++i) {
            construct_element(new_data + i, std::move(_data[i]));

            if constexpr (!avoid_init()) {
                destroy_element(_data + i);
            }
        }

        deallocate();

        _data = new_data;
        _capacity = new_capacity;
    }

    /**
     * Resize the vector to a new size. The new size must be greater than or
     * equal to 0. If the vector has a ReallocPolicy and the new size is greater
     * than the current capacity, the vector will be reallocated using the
     * policy. Otherwise, a std::length_error will be thrown.
     *
     * @param new_size The new size of the vector.
     */
    void resize(size_type new_size) {
        resize_prepare(new_size);
        resize_unsafe(new_size);
    }

    /**
     * Resize the vector to a new size and initialize all new elements to a
     * given value. The new size must be greater than or equal to 0. If the
     * vector has a ReallocPolicy and the new size is greater than the current
     * capacity, the vector will be reallocated using the policy. Otherwise, a
     * std::length_error will be thrown.
     *
     * @param new_size The new size of the vector.
     * @param value The value to initialize all new elements of the vector.
     */
    void resize(size_type new_size, const T &value) {
        resize_prepare(new_size);
        resize_unsafe(new_size, value);
    }

    /**
     * Resize the vector without checking if the new size is valid or if the
     * new size is greater than the current capacity.
     *
     * @param new_size The new size of the vector.
     */
    void resize_unsafe(size_type new_size) {
        if constexpr (!avoid_init()) {
            if (new_size > _size) {
                construct_elements_range(_size, new_size, T());
            }
            else if (new_size < _size) {
                destroy_elements_range(new_size, _size);
            }
        }

        _size = new_size;
    }

    /**
     * Resize the vector without checking if the new size is valid or if the
     * new size is greater than the current capacity. Initialize all new
     * elements to a given value.
     *
     * @param new_size The new size of the vector.
     * @param value The value to initialize all new elements of the vector.
     */
    void resize_unsafe(size_type new_size, const T &value) {
        if (new_size > _size) {
            construct_elements_range(_size, new_size, value);
        }
        else if (new_size < _size) {
            if constexpr (!avoid_init()) {
                destroy_elements_range(new_size, _size);
            }
        }

        _size = new_size;
    }

    /**
     * Erases all elements from the container. After this call, size() returns zero.
     * Does not affect the capacity of the vector.
     *
     */
    void clear() {
        if constexpr (!avoid_init()) {
            destroy_elements_range(0, _size);
        }

        _size = 0;
    }

    /**
     * Get the size of the vector.
     *
     * @return The size of the vector.
     */
    size_type size() const { return _size; }

    /**
     * Get the capacity of the vector.
     *
     * @return The capacity of the vector.
     */
    size_type capacity() const { return _capacity; }

    /**
     * Check if the vector is empty.
     *
     * @return True if the vector is empty, false otherwise.
     */
    bool empty() const { return _size == 0; }

    /**
     * Get a pointer to the raw data of the vector.
     *
     * @return A pointer to the raw data of the vector.
     */
    T *data() { return _data; }

    /**
     * Access operator.
     *
     * @return A reference to the element at the given index.
     */
    T &operator[](size_type idx) { return _data[idx]; }

    /**
     * Access operator.
     *
     * @return A const reference to the element at the given index.
     */
    const T &operator[](size_type idx) const { return _data[idx]; }

    /**
     * Add a value to the end of the vector. If the size is greater than the
     * capacity and the vector has a ReallocPolicy, the vector will be
     * reallocated using the policy. Otherwise, a std::length_error will be
     * thrown.
     *
     * @param value The value to add to the end of the vector.
     */
    void push_back(const T &value) {
        resize_prepare(_size + 1);
        push_back_unsafe(value);
    }

    /**
     * Move a value to the end of the vector. If the size is greater than the
     * capacity and the vector has a ReallocPolicy, the vector will be
     * reallocated using the policy. Otherwise, a std::length_error will be
     * thrown.
     *
     * @param value The value to move to the end of the vector.
     */
    void push_back(T &&value) {
        resize_prepare(_size + 1);
        push_back_unsafe(std::move(value));
    }

    /**
     * Add a value to the end of the vector without boundary checking.
     *
     * @param value The value to add to the end of the vector.
     */
    void push_back_unsafe(const T &value) {
        construct_element(_data + _size, value);
        ++_size;
    }

    /**
     * Move a value to the end of the vector without boundary checking.
     *
     * @param value The value to move to the end of the vector.
     */
    void push_back_unsafe(T &&value) {
        construct_element(_data + _size, std::move(value));
        ++_size;
    }

    /**
     * Reference to the last element of the vector.
     *
     * @return Reference to the last element of the vector.
     */
    T &back() { return _data[_size - 1]; }

    /**
     * Const reference to the last element of the vector.
     *
     * @return const T& Const reference to the last element of the vector.
     */
    const T &back() const { return _data[_size - 1]; }

    /**
     * Reference to the first element of the vector.
     *
     * @return Reference to the first element of the vector.
     */
    T &front() { return _data[0]; }

    /**
     * Const reference to the first element of the vector.
     *
     * @return const T& Const reference to the first element of the vector.
     */
    const T &front() const { return _data[0]; }

    /**
     * Swap the contents of the vector with another vector.
     */
    void swap(Vector<T> &other) {
        std::swap(_size, other._size);
        std::swap(_capacity, other._capacity);
        std::swap(_data, other._data);

        if constexpr (alloc_traits::propagate_on_container_swap::value) {
            std::swap(_alloc, other._alloc);
        }
    }

    /**
     * Iterator to the beginning of the vector.
     *
     * @return Iterator to the beginning of the vector.
     */
    Iterator begin() { return Iterator(_data); }

    /**
     * Constant Iterator to the beginning of the vector.
     *
     * @return Constant Iterator to the beginning of the vector.
     */
    const Iterator begin() const { return Iterator(_data); }

    /**
     * Iterator to the end of the vector.
     *
     * @return Iterator to the end of the vector.
     */
    Iterator end() { return Iterator(_data + _size); }

    /**
     * Constant Iterator to the end of the vector.
     *
     * @return Constant Iterator to the end of the vector.
     */
    const Iterator end() const { return Iterator(_data + _size); }

    /**
     * Get the allocator used by the vector.
     *
     * @return The allocator used by the vector.
     */
    allocator_type get_allocator() const {
        return _alloc;
    }
private:
    /**
     * Check if the vector should avoid calling the default constructor and
     * destructor when resizing the vector. I.e., check if the elements stored
     * in the vector are both standard layout and trivial.
     * 
     * @return True if the vector should avoid calling the default constructor
     * and destructor, false otherwise.
     */
    static constexpr bool avoid_init() {
        return std::is_standard_layout_v<T> && std::is_trivial_v<T>;
    }

    /**
     * Allocate memory into ptr.
     *
     * @param capacity The size of the allocation in elements.
     * @param ptr Pointer to the allocated memory.
     */
    void allocate_ptr(size_type size, T **ptr) {
        if (size == 0) {
            *ptr = nullptr;
        }
        else {
            *ptr = _alloc.allocate(size);
        }
    }

    /**
     * Deallocate memory from ptr.
     *
     * @param ptr Pointer to the memory to deallocate.
     */
    void deallocate_ptr(T **ptr) {
        _alloc.deallocate(*ptr, _capacity);
        *ptr = nullptr;
    }

    /**
     * Allocate _capacity memory for the vector.
     *
     */
    void allocate() {
        allocate_ptr(_capacity, &_data);
    }

    /**
     * Deallocate memory for the vector.
     *
     */
    void deallocate() {
        deallocate_ptr(&_data);
    }

    /**
     * Construct an element in the given pointer.
     *
     * @tparam Args The types of the arguments to construct the element.
     * @param p The pointer to the element.
     * @param args The arguments to construct the element.
     */
    template<typename... Args>
    void construct_element(T* p, Args&&... args) {
        alloc_traits::construct(_alloc, std::to_address(p), std::forward<Args>(args)...);
    }

    /**
     * Destroy an element in the given pointer.
     *
     * @param p The pointer to the element.
     */
    void destroy_element(T* p) {
        alloc_traits::destroy(_alloc, std::to_address(p));
    }

    /**
     * Initialize elements in the range [first, limit) with a given value.
     *
     * @param first The first element to initialize.
     * @param limit The limit of the range.
     * @param value The value to initialize the elements.
     */
    void construct_elements_range(size_type first, size_type limit, const T &value) {
        for (size_type i = first; i < limit; ++i) {
            construct_element(_data + i, value);
        }
    }

    /**
     * Destroy elements in the range [first, limit).
     *
     * @param first The first element to destroy.
     * @param limit The limit of the range.
     */
    void destroy_elements_range(size_type first, size_type limit) {
        for (size_type i = first; i < limit; ++i) {
            destroy_element(_data + i);
        }
    }

    /**
     * Copy elements from src to _data in the range [first, limit).
     *
     * @param first The first element to copy.
     * @param limit The limit of the range.
     * @param src Pointer to the source vector.
     */
    void copy_elements_range(size_type first, size_type limit, const T *src) {
        for (size_type i = first; i < limit; ++i) {
            construct_element(_data + i, src[i]);
        }
    }

    /**
     * Move elements from src to _data in the range [first, limit).
     *
     * @param first The first element to move.
     * @param limit The limit of the range.
     * @param src Pointer to the source vector.
     * @param dst Pointer to the destination vector.
     */
    void move_elements_range(size_type first, size_type limit, T *src) {
        for (size_type i = first; i < limit; ++i) {
            construct_element(_data + i, std::move(src[i]));
        }
    }

    /**
     * Initialize _size elements of the vector with a given value.
     *
     * @param value The value to initialize all elements of the vector.
     */
    void construct_elements(const T &value) {
        construct_elements_range(0, _size, value);
    }

    /**
     * Destroy _size elements of the vector.
     *
     */
    void destroy_elements() { destroy_elements_range(0, _size); }

    /**
     * Copy _size elements from src to dst.
     *
     * @param src Pointer to the source raw vector.
     * @param dst Pointer to the destination raw vector.
     */
    void copy_elements(const T *src) {
        copy_elements_range(0, _size, src);
    }

    /**
     * Move _size elements from src to dst.
     *
     * @param src Pointer to the source raw vector.
     * @param dst Pointer to the destination raw vector.
     */
    void move_elements(T *src) {
        move_elements_range(0, _size, src);
    }

    /**
     * Check if the new size is valid. If the new size is less than 0, a
     * std::length_error is thrown. If the new size is greater than the
     * current capacity and the vector has a ReallocPolicy, the vector is
     * reallocated using the policy. Otherwise, a std::length_error is
     * thrown.
     *
     * @param new_size The new size of the vector.
     */
    void resize_prepare(size_type new_size) {
        if (new_size < 0) {
            throw std::length_error("Vector: new_size < 0");
        }

        if (new_size > _capacity) {
            if constexpr (ReallocPolicy != nullptr) {
                realloc(ReallocPolicy(_capacity, new_size));
            }
            else {
                throw std::length_error(
                    "Vector: new_size > _capacity and no ReallocPolicy");
            }
        }
    }

    allocator_type _alloc;
    size_type _size;
    size_type _capacity;
    T *_data;
};

} // namespace theseus