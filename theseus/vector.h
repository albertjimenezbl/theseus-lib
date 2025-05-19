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
 * takes the current capacity (size_type) and the required size (size_type) as
 * arguments and returns the new capacity (size_type). It is assumed that the
 * new capacity provided by the policy is greater or equal to the required size.
 *
 * In case the elements stored in the vector are both standard layout and
 * trivial, the template parameter avoid_init_if_possible can be set to true to
 * avoid calling the default constructor and destructor when resizing the
 * vector. This improves the performance of the vector but the value of the
 * elements is not guaranteed to be the same as the default value of the type.
 *
 * @tparam T The type of the elements stored in the vector.
 * @tparam avoid_init_if_possible If true, the default constructor and
 * destructor are avoided when T is standard layout and trivial. This improves
 * the performance of the vector but the value of the elements is not guaranteed
 * to be the same as the default value of the type. If false, the default
 * constructor and destructor are always called when resizing the vector.
 * @tparam Allocator The type of the allocator used by the vector.
 */

namespace theseus {

template <typename T, bool avoid_init_if_possible = false, typename Allocator = std::allocator<T>>
class Vector {
public:
    /**
     * Iterator for the Vector.
     *
     */
    class Iterator {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using iterator_concept = std::contiguous_iterator_tag;
        using value_type = T;
        using size_type = std::ptrdiff_t;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        /**
         * Default constructor.
         *
         */
        Iterator() = default;

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
         * Pre-decrement operator.
         *
         * @return Reference to the current iterator.
         */
        Iterator &operator--() {
            _ptr--;
            return *this;
        }

        /**
         * Post-decrement operator.
         *
         * @return Copy of the iterator before decrementing.
         */
        Iterator operator--(int) {
            Iterator tmp = *this;
            --_ptr;
            return tmp;
        }

        /**
         * Addition operator: it += n.
         *
         * @param n The number of elements to add.
         * @return A new iterator with the added value.
         */
        Iterator &operator+=(difference_type n) {
            _ptr += n;
            return *this;
        }

        /**
         * Subtraction operator: it -= n.
         *
         * @param n The number of elements to subtract.
         * @return A new iterator with the subtracted value.
         */
        Iterator &operator-=(difference_type n) {
            _ptr -= n;
            return *this;
        }

        /**
         * Addition operator: it + n.
         *
         * @param n The number of elements to add.
         * @return A new iterator with the added value.
         */
        Iterator operator+(difference_type n) const {
            return Iterator(_ptr + n);
        }

        /**
         * Subtraction operator: it - n.
         *
         * @param n The number of elements to subtract.
         * @return A new iterator with the subtracted value.
         */
        Iterator operator-(difference_type n) const {
            return Iterator(_ptr - n);
        }

        /**
         * Addition operator: n + it.
         *
         * @param n The number of elements to add.
         * @param it The iterator to add to.
         * @return A new iterator with the added value.
         */
        friend Iterator operator+(difference_type n, const Iterator& it) {
            return Iterator(it._ptr + n);
        }

        /**
         * Difference operator.
         *
         * @param x The first iterator.
         * @param y The second iterator.
         * @return The difference between the two iterators.
         */
        friend difference_type operator-(const Iterator &x, const Iterator &y) {
            return x._ptr - y._ptr;
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
        T *operator->() const { return _ptr; }

        /**
         * Subscript operator.
         *
         * @param idx The index of the element to access with respect to the
         * iterator.
         * @return Reference to the element at the given index.
         */
        T &operator[] (difference_type idx) const { return _ptr[idx]; }

        /**
         * Equality operator.
         *
         * @param x The first iterator.
         * @param y The second iterator.
         * @return True if the two iterators are equal, false otherwise.
         */
        friend bool operator==(const Iterator &x, const Iterator &y) noexcept {
            return x._ptr == y._ptr;
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
            return x._ptr < y._ptr;
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
        T *_ptr = nullptr;
    };
    static_assert(std::contiguous_iterator<Iterator>);

    using value_type = T;
    using allocator_type = Allocator;
    using size_type = std::ptrdiff_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using alloc_traits = std::allocator_traits<allocator_type>;
    using pointer = typename alloc_traits::pointer;
    using const_pointer = typename alloc_traits::const_pointer;
    using iterator = Iterator;
    using const_iterator = const Iterator;
    using reverse_iterator = std::reverse_iterator<Iterator>;
    using const_reverse_iterator = std::reverse_iterator<const Iterator>;

    using realloc_policy = std::function<size_type(size_type, size_type)>;

    /**
     * Create an empty vector. Both the size and capacity are 0 and there is no
     * reallocation policy.
     *
     */
    Vector() noexcept(noexcept(Allocator())) : Vector(Allocator()) {}

    /**
     * Create an empty vector with a given allocator @p alloc. Both the size and
     * capacity are 0 and there is no reallocation policy.
     *
     * @param alloc The allocator to use.
     */
    explicit Vector(const Allocator &alloc) : _alloc(alloc) {}

    /**
     * Create an vector with a given size @p size and optional allocator @p
     * alloc. The capacity is equal to the size and there is no reallocation
     * policy.
     *
     * @param size The size of the vector.
     * @param alloc The allocator to use.
     */
    Vector(size_type size, const Allocator &alloc = Allocator())
        : _alloc(alloc), _size(size), _capacity(size) {

        if (size < 0) {
            throw std::length_error("Vector: size < 0");
        }

        _data = allocate_ptr(_capacity);

        default_construct_elements(_data, _size);
    }

    /**
     * Create an vector with a given size @p size and initialize all elements to
     * a given value @p value. The capacity is equal to the size and there is no
     * reallocation policy. The constructor takes an optional allocator @p
     * alloc.
     *
     * @param size The size of the vector.
     * @param value The value to copy initialize all elements of the vector.
     * @param alloc The allocator to use (optional).
     */
    Vector(size_type size, const T &value, const Allocator &alloc = Allocator())
        : _alloc(alloc), _size(size), _capacity(size) {

        if (size < 0) {
            throw std::length_error("Vector: size < 0");
        }

        _data = allocate_ptr(_capacity);
        copy_construct_elements<true>(_data, &value, _size);
    }

    /**
     * Copy constructor.
     *
     * @param other Source vector to copy.
     */
    Vector(const Vector &other)
        : _realloc_policy(other._realloc_policy),
          _alloc(alloc_traits::select_on_container_copy_construction(other._alloc)),
          _size(other._size), _capacity(other._capacity) {

        _data = allocate_ptr(_capacity);
        copy_construct_elements<false>(_data, other._data, _size);
    }

    /**
     * Move constructor.
     *
     * @param other Source vector to move.
     */
    Vector(Vector &&other) noexcept
        : _realloc_policy(other._realloc_policy),
          _alloc(std::move(other._alloc)),
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

        destroy_elements(_data, _size);

        constexpr bool propagate_alloc = alloc_traits::propagate_on_container_copy_assignment::value;
        const bool reallocate = (propagate_alloc && _alloc != other._alloc) ||
                                _capacity != other._capacity;

        if (reallocate) {
            deallocate_ptr(&_data);
        }

        if (propagate_alloc) {
            _alloc = other._alloc;
        }

        if (reallocate) {
            _capacity = other._capacity;
            _data = allocate_ptr(_capacity);
        }

        _size = other._size;
        copy_construct_elements<false>(_data, other._data, _size);

        _realloc_policy = other._realloc_policy;

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

        destroy_elements(_data, _size);

        // We can safely move the data.
        if (alloc_traits::propagate_on_container_move_assignment::value ||
            _alloc == other._alloc) {

            deallocate_ptr(&_data);

            _realloc_policy = std::move(other._realloc_policy);
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
            _realloc_policy = other._realloc_policy;

            if (_capacity < other._capacity) {
                deallocate_ptr(&_data);

                _capacity = other._capacity;
                _data = allocate_ptr(_capacity);
            }

            _size = other._size;
            move_construct_elements(_data, other._data, _size);
        }

        return *this;
    }

    /**
     * Destructor.
     *
     */
    ~Vector() {
        destroy_elements(_data, _size);
        deallocate_ptr(&_data);
    }

    /**
     * Reallocate the vector to a new capacity. The new capacity must be greater
     * than or equal to the current size of the vector and greater than or equal
     * to 0.
     *
     * @param new_capacity The new capacity of the vector.
     */
    void realloc(size_type new_capacity) {
        if (new_capacity == _capacity) {
            return;
        }
        else if (new_capacity < 0) {
            throw std::length_error("Vector: new_capacity < 0");
        }
        else if (new_capacity < _size) {
            throw std::length_error("Vector: new_capacity < _size");
        }

        // if constexpr (allocator_implements_reallocate) {
        //     // realloc provided by the allocator.
        //     _alloc.reallocate(_data, _size, new_capacity);
        // }
        // else {

        T *new_data = allocate_ptr(new_capacity);

        move_construct_elements(new_data, _data, _size);

        destroy_elements(_data, _size);
        deallocate_ptr(&_data);

        _data = new_data;
        _capacity = new_capacity;
    }

    /**
     * Resize the vector to a new size. The new size must be greater than or
     * equal to 0. If the vector has a realloc_policy and the new size is greater
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
     * Resize the vector to a new size and copy initialize all new elements to a
     * given value. The new size must be greater than or equal to 0. If the
     * vector has a realloc_policy and the new size is greater than the current
     * capacity, the vector will be reallocated using the policy. Otherwise, a
     * std::length_error will be thrown.
     *
     * @param new_size The new size of the vector.
     * @param value The value to copy initialize all new elements of the vector.
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
        if (new_size > _size) {
            default_construct_elements(_data + _size, new_size - _size);
        }
        else if (new_size < _size) {
            destroy_elements(_data + new_size, _size - new_size);
        }

        _size = new_size;
    }

    /**
     * Resize the vector without checking if the new size is valid or if the
     * new size is greater than the current capacity. Copy initialize all new
     * elements to a given value.
     *
     * @param new_size The new size of the vector.
     * @param value The value to copy initialize all new elements of the vector.
     */
    void resize_unsafe(size_type new_size, const T &value) {
        if (new_size > _size) {
            copy_construct_elements<true>(_data + _size, &value, new_size - _size);
        }
        else if (new_size < _size) {
            destroy_elements(_data + new_size, _size - new_size);
        }

        _size = new_size;
    }

    /**
     * Erases all elements from the container. After this call, size() returns
     * zero. The capacity of the vector is not changed.
     *
     */
    void clear() {
        destroy_elements(_data, _size);
        _size = 0;
    }

    /**
     * Add a value to the end of the vector. If the size is greater than the
     * capacity and the vector has a realloc policy, the vector will be
     * reallocated using the policy. Otherwise, a std::length_error will be
     * thrown.
     *
     * @param value The value to add (copy) to the end of the vector.
     */
    void push_back(const T &value) {
        resize_prepare(_size + 1);
        push_back_unsafe(value);
    }

    /**
     * Move a value to the end of the vector. If the size is greater than the
     * capacity and the vector has a realloc policy, the vector will be
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
        emplace_back_unsafe(value);
    }

    /**
     * Move a value to the end of the vector without boundary checking.
     *
     * @param value The value to move to the end of the vector.
     */
    void push_back_unsafe(T &&value) {
        emplace_back_unsafe(std::move(value));
    }

    /**
     * Construct an element at the end of the vector using the given arguments.
     *
     * @tparam Args The types of the arguments to construct the element.
     * @param args The arguments to construct the element.
     */
    template<class... Args>
    void emplace_back(Args &&... args) {
        resize_prepare(_size + 1);
        emplace_back_unsafe(std::forward<Args>(args)...);
    }

    /**
     * Construct an element at the end of the vector using the given arguments
     * without boundary checking.
     *
     * @tparam Args The types of the arguments to construct the element.
     * @param args The arguments to construct the element.
     */
    template<class... Args>
    void emplace_back_unsafe(Args&&... args) {
        args_construct_element(_data + _size, std::forward<Args>(args)...);
        ++_size;
    }

    /**
     * Set the reallocation policy. The policy must be a callable object that
     * takes the current capacity (size_type) and the required size
     * (size_type) as arguments and returns the new capacity
     * (size_type). It is assumed that the new capacity provided by the
     * policy is greater or equal to the required size.
     *
     * A nullptr policy means that the vector will not be reallocated when the
     * required size is greater than the current capacity.
     *
     * @param policy The reallocation policy to set.
     */
    void set_realloc_policy(realloc_policy policy) { _realloc_policy = policy; }

    /**
     * Get the reallocation policy.
     *
     * @return The reallocation policy.
     */
    realloc_policy get_realloc_policy() const { return _realloc_policy; }

    /**
     * Get the allocator used by the vector.
     *
     * @return The allocator used by the vector.
     */
    allocator_type get_allocator() const {
        return _alloc;
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
     * Access operator.
     *
     * @return A reference to the element at the given index.
     */
    T &operator[](size_type idx) { return _data[idx]; }
    const T &operator[](size_type idx) const { return _data[idx]; }

    /**
     * Access operator with bounds checking.
     *
     * @return A reference to the element at the given index.
     */
    T &at(size_type idx) {
        if (idx >= _size) {
            throw std::out_of_range("Vector: index out of range");
        }
        return _data[idx];
    }
    const T &at(size_type idx) const {
        return const_cast<T &>(const_cast<const Vector *>(this)->at(idx));
    }

    /**
     * Reference to the first element of the vector.
     *
     * @return Reference to the first element of the vector.
     */
    T &front() { return _data[0]; }
    const T &front() const { return _data[0]; }

    /**
     * Reference to the last element of the vector.
     *
     * @return Reference to the last element of the vector.
     */
    T &back() { return _data[_size - 1]; }
    const T &back() const { return _data[_size - 1]; }

    /**
     * Get a pointer to the raw data of the vector.
     *
     * @return A pointer to the raw data of the vector.
     */
    T* data() { return _data; }
    const T* data() const { return _data; }

    /**
     * Iterator to the beginning of the vector.
     *
     * @return Iterator to the beginning of the vector.
     */
    Iterator begin() { return Iterator(_data); }
    const Iterator begin() const { return Iterator(_data); }
    const Iterator cbegin() const noexcept { return Iterator(_data); }

    /**
     * Iterator to the end of the vector.
     *
     * @return Iterator to the end of the vector.
     */
    Iterator end() { return Iterator(_data + _size); }
    const Iterator end() const { return Iterator(_data + _size); }
    const Iterator cend() const noexcept { return Iterator(_data + _size); }

    /**
     * Reverse iterator to the beginning of the vector.
     *
     * @return Reverse iterator to the beginning of the vector.
     */
    reverse_iterator rbegin() { return reverse_iterator(end()); }
    const reverse_iterator rbegin() const { return reverse_iterator(end()); }
    const reverse_iterator crbegin() const noexcept { return reverse_iterator(end()); }

    /**
     * Reverse iterator to the end of the vector.
     *
     * @return Reverse iterator to the end of the vector.
     */
    reverse_iterator rend() { return reverse_iterator(begin()); }
    const reverse_iterator rend() const { return reverse_iterator(begin()); }
    const reverse_iterator crend() const noexcept { return reverse_iterator(begin()); }

    /**
     * Swap the contents of the vector with another vector.
     *
     * @param other The vector to swap with.
     */
    void swap(Vector &other) {
        std::swap(_realloc_policy, other._realloc_policy);

        if constexpr (alloc_traits::propagate_on_container_swap::value) {
            std::swap(_alloc, other._alloc);
        }

        std::swap(_size, other._size);
        std::swap(_capacity, other._capacity);
        std::swap(_data, other._data);
    }

    /**
     * Swap the contents of two vectors.
     *
     * @param x The first vector to swap.
     * @param y The second vector to swap.
     */
    friend void swap(Vector &x, Vector &y) {
        x.swap(y);
    }
private:
    /**
     * Check if the vector should avoid calling the default constructor and
     * destructor when resizing the vector. I.e., check if the elements stored
     * in the vector are both standard layout and trivial.
     */
    static constexpr bool avoid_init = avoid_init_if_possible &&
                                       std::is_standard_layout_v<T> &&
                                       std::is_trivial_v<T>;

    /**
     * Check if the allocator defines the boolean
     * allocator_implements_reallocate. If the allocator defines it and it is
     * true, it is possible to call the reallocate function of the allocator. If
     * allocator_implements_reallocate is not defined or it is false, the
     * allocator does not implement the reallocate function.
     *
     */
    // static constexpr bool allocator_implements_reallocate = [] {
    //     if constexpr (requires {
    //                       typename allocator_type::implements_reallocate::value_type;
    //                   }) {
    //         return allocator_type::implements_reallocate::value;
    //     }
    //     else {
    //         return false;
    //     }
    // }();

    /**
     * Return a pointer to a newly allocated memory of size @p size.
     *
     * @param size The number of elements to allocate.
     */
    T *allocate_ptr(size_type size) {
        T* data = alloc_traits::allocate(_alloc, size);
        if (!data) {
            throw std::bad_alloc();
        }
        return data;
    }

    /**
     * Deallocate memory from @p ptr.
     *
     * @param ptr Pointer to the memory to deallocate.
     */
    void deallocate_ptr(T **ptr) {
        alloc_traits::deallocate(_alloc, *ptr, _capacity);
        *ptr = nullptr;
    }

    /**
     * Construct an element at @p pdst using the given arguments @p args.
     *
     * @tparam Args The types of the arguments to construct the element.
     * @param pdst The pointer to the element.
     * @param args The arguments to construct the element.
     */
    template<class... Args>
    void args_construct_element(T* pdst, Args&&... args) {
        alloc_traits::construct(_alloc, std::to_address(pdst), std::forward<Args>(args)...);
    }

    /**
     * Default construct @p n elements starting at pointer @p pdst.
     *
     * @param pdst The pointer to the first element to initialize.
     * @param n The number of elements to initialize.
     */
    void default_construct_elements(T* pdst, size_type n) {
        if constexpr (avoid_init) {
            return;
        }

        for (T* p = pdst; p < pdst + n; ++p) {
            alloc_traits::construct(_alloc, std::to_address(p));
        }
    }

    /**
     * Copy construct @p n elements starting at destination pointer @p pdst and
     * source pointer @p psrc. If @p single_src is true, then there is a single
     * value to copy @p n times, i.e., @p psrc is not incremented after each
     * copy. Otherwise, @p psrc is incremented after each copy.
     *
     * @tparam single_src True if there is a single value to copy @p n times,
     * false if @p psrc is incremented after each copy.
     * @param pdst The pointer to the first element to initialize.
     * @param psrc The pointer to the first element to copy from.
     * @param n The number of elements to initialize.
     */
    template<bool single_src>
    void copy_construct_elements(T* pdst, const T* psrc, size_type n) {
        for (T* p = pdst; p < pdst + n; ++p) {
            alloc_traits::construct(_alloc, std::to_address(p), *psrc);

            if constexpr (!single_src) {
                ++psrc;
            }
        }
    }

    /**
     * Move construct @p n elements starting at destination pointer @p pdst and
     * source pointer @p psrc.
     *
     * @param pdst The pointer to the first element to initialize.
     * @param psrc The pointer to the first element to move from.
     * @param n The number of elements to initialize.
     */
    void move_construct_elements(T* pdst, T* psrc, size_type n) {
        for (T* p = pdst; p < pdst + n; ++p, ++psrc) {
            alloc_traits::construct(_alloc, std::to_address(p), std::move(*psrc));
        }
    }

    /**
     * Destroy @p n elements starting at pointer @p pdst.
     *
     * @param pdst The pointer to the first element to destroy.
     * @param n The number of elements to destroy.
     */
    void destroy_elements(T* pdst, size_type n) {
        if constexpr (avoid_init) {
            return;
        }

        for (T* p = pdst; p < pdst + n; ++p) {
            alloc_traits::destroy(_alloc, std::to_address(p));
        }
    }

    /**
     * Check if the new size is valid. If the new size is less than 0, a
     * std::length_error is thrown. If the new size is greater than the
     * current capacity and the vector has a realloc_policy, the vector is
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
            if (_realloc_policy) {
                realloc(_realloc_policy(_capacity, new_size));
            }
            else {
                throw std::length_error(
                    "Vector: new_size > _capacity and no realloc policy set");
            }
        }
    }

    realloc_policy _realloc_policy = nullptr;
    allocator_type _alloc;
    size_type _size = 0;
    size_type _capacity = 0;
    T *_data = nullptr;
};

} // namespace theseus