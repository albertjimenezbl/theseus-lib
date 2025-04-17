#pragma once

/**
 * An array indexed by diagonals of the Dynamic Programming matrix.
 * 0 is the main diagonal (the diagonal of the {0, 0} cell), positive values are
 * above the main diagonal and negative values are below the main diagonal.
 * A wavefront can never have less than 1 diagonals: at least the main diagonal
 * must be present.
 *
 * @tparam T The type of the elements stored in the wavefront.
 */

namespace theseus {

template <typename T>
class Wavefront {
public:
    using value_type = T;
    using diag_type = ptrdiff_t;
    using size_type = ptrdiff_t;

    /**
     * Construct a wavefront with the given minimum and maximum diagonals.
     * @p min_diag must be less than or equal to 0 and @p max_diag must be
     * greater than or equal to 0, otherwise an exception is thrown.
     *
     * @param min_diag The minimum diagonal (<= 0).
     * @param max_diag The maximum diagonal (>= 0).
     */
    Wavefront(diag_type min_diag, diag_type max_diag)
        : _min_diag(min_diag),
          _max_diag(max_diag),
          _data(nullptr),
          _middle(nullptr) {

        alloc();

        if constexpr (!avoid_init()) {
            init_elements(T());
        }
    }

    /**
     * Construct a wavefront with the given minimum and maximum diagonals and
     * initialize all the elements with the given value. @p min_diag must be
     * less than or equal to 0 and @p max_diag must be greater than or equal to
     * 0, otherwise an exception is thrown.
     *
     * @param min_diag The minimum diagonal (<= 0).
     * @param max_diag The maximum diagonal (>= 0).
     * @param value The value to initialize all the elements.
     */
    Wavefront(diag_type min_diag, diag_type max_diag, const T& value)
        : _min_diag(min_diag),
          _max_diag(max_diag),
          _data(nullptr),
          _middle(nullptr) {

        alloc();
        init_elements(value);
    }

    /**
     * Copy constructor.
     *
     * @param other Source wavefront to copy.
     */
    Wavefront(const Wavefront<T>& other)
        : _min_diag(other._min_diag),
          _max_diag(other._max_diag),
          _data(nullptr),
          _middle(nullptr) {

        alloc();
        copy_elements(other._data, _data);
    }

    /**
     * Move constructor.
     *
     * @param other Source vector to move.
     */
    Wavefront(Wavefront<T>&& other) noexcept
        : _min_diag(other._min_diag),
          _max_diag(other._max_diag),
          _data(other._data),
          _middle(other._middle) {

        other._min_diag = 0;
        other._max_diag = -1;
        other._data = nullptr;
        other._middle = nullptr;
    }

    /**
     * Copy assignment operator. The source wavefront is left in an invalid
     * state.
     *
     * @param other Source wavefront to copy.
     * @return Reference to the current wavefront.
     */
    Wavefront<T>& operator=(const Wavefront<T>& other) {
        if (this == &other) {
            return *this;
        }

        destruct_elements();

        const auto old_size = size();

        _min_diag = other._min_diag;
        _max_diag = other._max_diag;

        if (old_size != other.size()) {
            destruct();
            alloc();
        }

        _middle = &_data[-_min_diag];

        copy_elements(other._data, _data);

        return *this;
    }

    /**
     * Move assignment operator. The source wavefront is left in an invalid
     * state.
     *
     * @param other Source wavefront to move.
     * @return Reference to the current wavefront.
     */
    Wavefront<T>& operator=(Wavefront<T>&& other) noexcept {
        if (this == &other) {
            return *this;
        }

        destruct_elements();
        destruct();

        _min_diag = other._min_diag;
        _max_diag = other._max_diag;
        _data = other._data;
        _middle = other._middle;

        other._min_diag = 0;
        other._max_diag = -1;
        other._data = nullptr;
        other._middle = nullptr;

        return *this;
    }

    /**
     * Destructor.
     *
     */
    ~Wavefront() {
        destruct_elements();
        destruct();
    }

    /**
     * Get the element at the given diagonal.
     *
     * @param diag The diagonal.
     * @return Reference to the element at the given diagonal.
     */
    T& operator[](diag_type diag) { return _middle[diag]; }

    /**
     * Get the element at the given diagonal.
     *
     * @param diag The diagonal.
     * @return Constant reference to the element at the given diagonal.
     */
    const T& operator[](diag_type diag) const { return _middle[diag]; }

    /**
     * Return the minimum diagonal of the wavefront.
     *
     * @return The minimum diagonal of the wavefront.
     */
    diag_type min_diag() const { return _min_diag; }

    /**
     * Return the maximum diagonal of the wavefront.
     *
     * @return The maximum diagonal of the wavefront.
     */
    diag_type max_diag() const { return _max_diag; }

    /**
     * Return the total number of diagonals of the wavefront.
     *
     * @return The total number of diagonals of the wavefront.
     */
    size_type size() const { return _max_diag + (-_min_diag) + 1; }

    /**
     * Check if a given diagonal is within the bounds of the wavefront.
     *
     * @param diag The diagonal to check.
     * @return True if the diagonal is within the bounds of the wavefront,
     * false otherwise.
     */
    bool in_bounds(diag_type diag) const {
        return diag >= _min_diag && diag <= _max_diag;
    }

    /**
     * Swap the contents of this wavefront with another wavefront.
     *
     * @param other The other wavefront to swap with.
     */
    void swap(Wavefront<T>& other) {
        std::swap(_min_diag, other._min_diag);
        std::swap(_max_diag, other._max_diag);
        std::swap(_data, other._data);
        std::swap(_middle, other._middle);
    }

private:
    /**
     * Check if the wavefront should avoid calling the default constructor
     * and destructor.
     *
     * @return True if the wavefront should avoid calling the default
     * constructor and destructor, false otherwise.
     */
    static constexpr bool avoid_init() {
        return std::is_standard_layout_v<T> && std::is_trivial_v<T>;
    }

    /**
     * Allocate memory for the wavefront.
     *
     */
    void alloc() {
        if (_min_diag > 0 || _max_diag < 0) {
            throw std::length_error("Invalid wavefront bounds");
        }

        _data = static_cast<T*>(operator new(sizeof(T) * size()));

        _middle = &_data[-_min_diag];
    }

    void destruct() {
        operator delete(_data);
        _data = nullptr;
        _middle = nullptr;
    }

    /**
     * Initialize the elements of the wavefront with the given value.
     *
     * @param value The value to initialize the elements.
     */
    void init_elements(const T& value) {
        const size_type n = size();
        for (size_type i = 0; i < n; ++i) {
            new (_data + i) T(value);
        }
    }

    /**
     * Destruct the elements of the wavefront.
     */
    void destruct_elements() {
        if constexpr (!avoid_init()) {
            const size_type n = size();
            for (size_type i = 0; i < n; ++i) {
                _data[i].~T();
            }
        }
    }

    /**
     * Copy elements from src to dst. It is assumed that src and dst have
     * max_diag - min_diag + 1 elements.
     *
     * @param src Pointer to the source raw data.
     * @param dst Pointer to the destination raw data.
     */
    void copy_elements(const T* src, T* dst) {
        const size_type n = size();
        for (size_type i = 0; i < n; ++i) {
            new (dst + i) T(src[i]);
        }
    }

    diag_type _min_diag;
    diag_type _max_diag;

    T* _data;
    T* _middle;
};

} // namespace theseus