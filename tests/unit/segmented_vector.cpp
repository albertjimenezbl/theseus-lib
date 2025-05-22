#include "../doctest.h"

#include "../../theseus/segmented_vector.h"

static int non_pod_default_constructor_calls;
static int non_pod_constructor_calls;
static int non_pod_copy_constructor_calls;
static int non_pod_move_constructor_calls;
static int non_pod_destructor_calls;

struct POD {
    int _a;
    int _b;
};

struct NonPOD {
public:
    int _a;

    NonPOD() : _a(100) {
        ++non_pod_default_constructor_calls;
    }

    NonPOD(int a) : _a(a) {
        ++non_pod_constructor_calls;
    }

    NonPOD(const NonPOD &other) : _a(other._a) {
        ++non_pod_copy_constructor_calls;
    }

    NonPOD(NonPOD &&other) : _a(other._a) {
        ++non_pod_move_constructor_calls;
    }

    ~NonPOD() {
        ++non_pod_destructor_calls;
    }
};

template <int SegmentSize>
void test_constructors_and_assignments() {
    CAPTURE(SegmentSize);

    constexpr int size = 20;
    static_assert(SegmentSize < size && SegmentSize > size / 2);

    SUBCASE("Default constructor and realloc") {
        theseus::SegmentedVector<POD, SegmentSize> v;

        CHECK(v.capacity() == 0);
        CHECK(v.size() == 0);
        CHECK(v.empty());

        v.realloc(size);
        CHECK(v.capacity() == 2 * SegmentSize);
        CHECK(v.size() == 0);
        CHECK(v.empty());

        v.resize(size - 10);
        CHECK(v.size() == size - 10);
        CHECK(v.capacity() == 2 * SegmentSize);
        CHECK(!v.empty());

        v.realloc(size - 10);
        CHECK(v.capacity() == SegmentSize);
        CHECK(v.size() == size - 10);

        v.resize_unsafe(0);
        v.realloc(0);
        CHECK(v.capacity() == 0);
        CHECK(v.size() == 0);
        CHECK(v.empty());
    }

    SUBCASE("Constructor with size (default init)") {
        theseus::SegmentedVector<POD, SegmentSize> v(size);

        CHECK(v.size() == size);
        CHECK(v.capacity() == 2 * SegmentSize);

        for (int i = 0; i < size; ++i) {
            CHECK(v[i]._a == 0);
            CHECK(v[i]._b == 0);
        }
    }

    SUBCASE("Constructor with size (value init)") {
        theseus::SegmentedVector<POD, SegmentSize> v(size, POD{200, 201});

        CHECK(v.size() == size);
        CHECK(v.capacity() == 2 * SegmentSize);

        for (int i = 0; i < size; ++i) {
            CHECK(v[i]._a == 200);
            CHECK(v[i]._b == 201);
        }
    }

    SUBCASE("Copy constructor") {
        theseus::SegmentedVector<POD, SegmentSize> v1(size, POD{200, 201});
        theseus::SegmentedVector<POD, SegmentSize> v2(v1);

        CHECK(v2.size() == size);
        CHECK(v2.capacity() == SegmentSize * 2);

        for (int i = 0; i < size; ++i) {
            CHECK(v1[i]._a == 200);
            CHECK(v1[i]._b == 201);
            CHECK(v2[i]._a == 200);
            CHECK(v2[i]._b == 201);
        }

        // Also check empty copy.
        theseus::SegmentedVector<POD, SegmentSize> v3;
        theseus::SegmentedVector<POD, SegmentSize> v4(v3);

        CHECK(v4.size() == 0);
        CHECK(v4.capacity() == 0);
    }

    SUBCASE("Move constructor") {
        theseus::SegmentedVector<POD, SegmentSize> v1(size, POD{200, 201});
        theseus::SegmentedVector<POD, SegmentSize> v2(std::move(v1));

        CHECK(v2.size() == size);
        CHECK(v2.capacity() == SegmentSize * 2);

        for (int i = 0; i < size; ++i) {
            CHECK(v2[i]._a == 200);
            CHECK(v2[i]._b == 201);
        }

        CHECK(v1.size() == 0);
        CHECK(v1.capacity() == 0);
    }

    SUBCASE("Copy assignment") {
        theseus::SegmentedVector<POD, SegmentSize> v1(size, POD{200, 201});
        theseus::SegmentedVector<POD, SegmentSize> v2;

        CHECK(v2.size() == 0);
        CHECK(v2.capacity() == 0);

        v2 = v1;

        CHECK(v2.size() == size);
        CHECK(v2.capacity() == SegmentSize * 2);

        for (int i = 0; i < size; ++i) {
            CHECK(v2[i]._a == 200);
            CHECK(v2[i]._b == 201);
        }
    }

    SUBCASE("Move assignment") {
        theseus::SegmentedVector<POD, SegmentSize> v1(size, POD{200, 201});
        theseus::SegmentedVector<POD, SegmentSize> v2;

        CHECK(v2.size() == 0);
        CHECK(v2.capacity() == 0);

        v2 = std::move(v1);

        CHECK(v2.size() == size);
        CHECK(v2.capacity() == SegmentSize * 2);

        for (int i = 0; i < size; ++i) {
            CHECK(v2[i]._a == 200);
            CHECK(v2[i]._b == 201);
        }

        CHECK(v1.size() == 0);
        CHECK(v1.capacity() == 0);
    }
}

TEST_CASE("Test segmented vector constructors and assignments") {
    test_constructors_and_assignments<16>(); // Power of 2.
    test_constructors_and_assignments<17>(); // Not a power of 2.
}

template <int SegmentSize>
void test_init_policies() {
    CAPTURE(SegmentSize);

    constexpr int size = 20;
    static_assert(SegmentSize < size && SegmentSize > size / 2);

    SUBCASE("POD avoid init") {
        theseus::SegmentedVector<POD, SegmentSize, true> v;
        v.realloc(size);

        // Initialize to something.
        v.resize(size, POD{200, 201});

        v.resize(0);
        // Entries should be the same. We are deliberately going out of bounds.
        for (int i = 0; i < size; ++i) {
            CHECK(v[i]._a == 200);
            CHECK(v[i]._b == 201);
        }

        v.resize(size);
        // Again, the vector should keep the initial state.
        for (int i = 0; i < size; ++i) {
            CHECK(v[i]._a == 200);
            CHECK(v[i]._b == 201);
        }
    }

    SUBCASE("POD do not avoid init") {
        theseus::SegmentedVector<POD, SegmentSize, false> v;
        v.realloc(size);

        // Initialize to something.
        v.resize(size, POD{200, 201});

        v.resize(0);
        // Entries should be the same (no destructor). We are deliberately going
        // out of bounds.
        for (int i = 0; i < size; ++i) {
            CHECK(v[i]._a == 200);
            CHECK(v[i]._b == 201);
        }

        v.resize(size);
        // Now, the vector should be default initialized.
        for (int i = 0; i < size; ++i) {
            CHECK(v[i]._a == 0);
            CHECK(v[i]._b == 0);
        }
    }

    SUBCASE("Non-POD avoid init") {
        non_pod_default_constructor_calls = 0;
        non_pod_constructor_calls = 0;
        non_pod_copy_constructor_calls = 0;
        non_pod_move_constructor_calls = 0;
        non_pod_destructor_calls = 0;

        // Avoid init should be discarded by the vector for non-POD types.
        theseus::SegmentedVector<NonPOD, SegmentSize, true> v;
        v.realloc(size);

        CHECK(non_pod_default_constructor_calls == 0);
        CHECK(non_pod_constructor_calls == 0);
        CHECK(non_pod_copy_constructor_calls == 0);
        CHECK(non_pod_move_constructor_calls == 0);
        CHECK(non_pod_destructor_calls == 0);

        // Initialize to something.
        v.resize(size, NonPOD{200});

        CHECK(non_pod_default_constructor_calls == 0);
        CHECK(non_pod_constructor_calls == 1);
        CHECK(non_pod_copy_constructor_calls == size);
        CHECK(non_pod_move_constructor_calls == 0);
        CHECK(non_pod_destructor_calls == 1);

        v.resize(0);

        CHECK(non_pod_default_constructor_calls == 0);
        CHECK(non_pod_constructor_calls == 1);
        CHECK(non_pod_copy_constructor_calls == size);
        CHECK(non_pod_move_constructor_calls == 0);
        CHECK(non_pod_destructor_calls == size + 1);

        v.resize(size, 200);

        CHECK(non_pod_default_constructor_calls == 0);
        CHECK(non_pod_constructor_calls == 2);
        CHECK(non_pod_copy_constructor_calls == size * 2);
        CHECK(non_pod_move_constructor_calls == 0);
        CHECK(non_pod_destructor_calls == size + 2);
    }

    SUBCASE("Non-POD do not avoid init") {
        non_pod_default_constructor_calls = 0;
        non_pod_constructor_calls = 0;
        non_pod_copy_constructor_calls = 0;
        non_pod_move_constructor_calls = 0;
        non_pod_destructor_calls = 0;

        // Same as previous case.
        theseus::SegmentedVector<NonPOD, SegmentSize> v;
        v.realloc(size);

        CHECK(non_pod_default_constructor_calls == 0);
        CHECK(non_pod_constructor_calls == 0);
        CHECK(non_pod_copy_constructor_calls == 0);
        CHECK(non_pod_move_constructor_calls == 0);
        CHECK(non_pod_destructor_calls == 0);

        // Initialize to something.
        v.resize(size, NonPOD{200});

        CHECK(non_pod_default_constructor_calls == 0);
        CHECK(non_pod_constructor_calls == 1);
        CHECK(non_pod_copy_constructor_calls == size);
        CHECK(non_pod_move_constructor_calls == 0);
        CHECK(non_pod_destructor_calls == 1);

        v.resize(0);

        CHECK(non_pod_default_constructor_calls == 0);
        CHECK(non_pod_constructor_calls == 1);
        CHECK(non_pod_copy_constructor_calls == size);
        CHECK(non_pod_move_constructor_calls == 0);
        CHECK(non_pod_destructor_calls == size + 1);

        v.resize(size, NonPOD{200});

        CHECK(non_pod_default_constructor_calls == 0);
        CHECK(non_pod_constructor_calls == 2);
        CHECK(non_pod_copy_constructor_calls == size * 2);
        CHECK(non_pod_move_constructor_calls == 0);
        CHECK(non_pod_destructor_calls == size + 2);
    }
}

TEST_CASE("Test segmented vector initialization policies") {
    test_init_policies<16>(); // Power of 2.
    test_init_policies<17>(); // Not a power of 2.
}

template <int SegmentSize>
void test_vector_resize() {
    CAPTURE(SegmentSize);

    constexpr int size = 20;
    static_assert(SegmentSize < size && SegmentSize > size / 2);

    SUBCASE("Resize with bound check") {
        theseus::SegmentedVector<POD, SegmentSize, true> v;
        v.realloc(size);

        CHECK(v.capacity() == SegmentSize * 2);

        try {
            v.resize(v.capacity() + 1);
            CHECK(false);
        } catch (const std::length_error &e) {
            CHECK(true);
        }
    }
    SUBCASE("Resize without bound check") {
        theseus::SegmentedVector<POD, SegmentSize, true> v;
        v.realloc(size);

        // This leaves the vector in an invalid state.
        try {
            v.resize_unsafe(v.capacity() + 1);
            CHECK(true);
        } catch (const std::length_error &e) {
            CHECK(false);
        }
        v.resize(0);
    }
}

TEST_CASE("Test segmented vector resize") {
    test_vector_resize<16>(); // Power of 2.
    test_vector_resize<17>(); // Not a power of 2.
}

template <int SegmentSize>
void test_clear() {
    CAPTURE(SegmentSize);

    constexpr int size = 20;
    static_assert(SegmentSize < size && SegmentSize > size / 2);

    non_pod_default_constructor_calls = 0;
    non_pod_constructor_calls = 0;
    non_pod_copy_constructor_calls = 0;
    non_pod_move_constructor_calls = 0;
    non_pod_destructor_calls = 0;

    theseus::SegmentedVector<NonPOD, SegmentSize> v(size, NonPOD{200});
    CHECK(v.size() == size);
    CHECK(v.capacity() == SegmentSize * 2);

    v.clear();

    CHECK(v.size() == 0);
    CHECK(v.capacity() == SegmentSize * 2);

    CHECK(non_pod_default_constructor_calls == 0);
    CHECK(non_pod_constructor_calls == 1);
    CHECK(non_pod_copy_constructor_calls == size);
    CHECK(non_pod_move_constructor_calls == 0);
    CHECK(non_pod_destructor_calls == size + 1);
}

TEST_CASE("Test segmented vector clear") {
    test_clear<16>(); // Power of 2.
    test_clear<17>(); // Not a power of 2.
}

template <int SegmentSize>
void test_emplace_back() {
    CAPTURE(SegmentSize);

    constexpr int size = 20;
    static_assert(SegmentSize < size && SegmentSize > size / 2);

    theseus::SegmentedVector<NonPOD, SegmentSize> v;
    v.realloc(size);

    non_pod_default_constructor_calls = 0;
    non_pod_constructor_calls = 0;
    non_pod_copy_constructor_calls = 0;
    non_pod_move_constructor_calls = 0;
    non_pod_destructor_calls = 0;

    for (int i = 0; i < size; ++i) {
        v.emplace_back(i);
    }

    CHECK(non_pod_default_constructor_calls == 0);
    CHECK(non_pod_constructor_calls == size);
    CHECK(non_pod_copy_constructor_calls == 0);
    CHECK(non_pod_move_constructor_calls == 0);
    CHECK(non_pod_destructor_calls == 0);

    for (int i = 0; i < size; ++i) {
        CHECK(v[i]._a == i);
    }

    v.resize(0);
    CHECK(v.empty() == true);

    CHECK(non_pod_default_constructor_calls == 0);
    CHECK(non_pod_constructor_calls == size);
    CHECK(non_pod_copy_constructor_calls == 0);
    CHECK(non_pod_move_constructor_calls == 0);
    CHECK(non_pod_destructor_calls == size);
}

TEST_CASE("Test segmented vector emplace_back") {
    test_emplace_back<16>(); // Power of 2.
    test_emplace_back<17>(); // Not a power of 2.
}

template <int SegmentSize>
void test_push_pop_back() {
    CAPTURE(SegmentSize);

    constexpr int size = 20;
    static_assert(SegmentSize < size && SegmentSize > size / 2);

    theseus::SegmentedVector<NonPOD, SegmentSize> v;
    v.realloc(size);

    non_pod_default_constructor_calls = 0;
    non_pod_constructor_calls = 0;
    non_pod_copy_constructor_calls = 0;
    non_pod_move_constructor_calls = 0;
    non_pod_destructor_calls = 0;

    for (int i = 0; i < size; ++i) {
        v.push_back(std::move(NonPOD{i}));
    }

    v.pop_back();
    v.pop_back_unsafe();
    v.push_back_unsafe(std::move(NonPOD{size - 2}));
    v.push_back_unsafe(std::move(NonPOD{size - 1}));

    CHECK(non_pod_default_constructor_calls == 0);
    CHECK(non_pod_constructor_calls == size + 2);
    CHECK(non_pod_copy_constructor_calls == 0);
    CHECK(non_pod_move_constructor_calls == size + 2);
    CHECK(non_pod_destructor_calls == size + 4);

    for (int i = 0; i < size; ++i) {
        CHECK(v[i]._a == i);
    }

    v.resize(0);
    CHECK(v.empty() == true);

    CHECK(non_pod_default_constructor_calls == 0);
    CHECK(non_pod_constructor_calls == size + 2);
    CHECK(non_pod_copy_constructor_calls == 0);
    CHECK(non_pod_move_constructor_calls == size + 2);
    CHECK(non_pod_destructor_calls == size * 2 + 4);
}

TEST_CASE("Test segmented vector push_back and pop_back") {
    test_push_pop_back<16>(); // Power of 2.
    test_push_pop_back<17>(); // Not a power of 2.
}

template <int SegmentSize>
void test_element_access() {
    CAPTURE(SegmentSize);

    constexpr int size = 20;
    static_assert(SegmentSize < size && SegmentSize > size / 2);

    theseus::SegmentedVector<int, SegmentSize> v(size, 200);

    CHECK(v.front() == 200);
    CHECK(v.back() == 200);

    for (int i = 0; i < size; ++i) {
        v[i] = i;
    }

    for (int i = 0; i < size; ++i) {
        CHECK(v[i] == i);
    }

    v.front() = -1;
    v.back() = -2;

    CHECK(v.front() == -1);
    CHECK(v[0] == -1);

    CHECK(v.back() == -2);
    CHECK(v[size - 1] == -2);

    try {
        v.at(size * 3) = 1;
        CHECK(false);
    } catch (const std::out_of_range &e) {
        CHECK(true);
    }
}

TEST_CASE("Test segmented vector element access") {
    test_element_access<16>(); // Power of 2.
    test_element_access<17>(); // Not a power of 2.
}

template <int SegmentSize>
void test_iterator() {
    CAPTURE(SegmentSize);

    constexpr int size = 20;
    static_assert(SegmentSize < size && SegmentSize > size / 2);

    SUBCASE("Forward iterator") {
        theseus::SegmentedVector<int, SegmentSize> v(size, 200);

        int i = 0;
        for (auto it = v.begin(); it != v.end(); ++it) {
            CHECK(*it == 200);
            *it = i;
            ++i;
        }
        CHECK(i == size);

        i = 0;
        for (auto it = v.cbegin(); it != v.cend(); ++it) {
            static_assert(std::is_const<std::remove_reference_t<decltype(*it)>>::value,
                          "*it is not const!");
            CHECK(*it == i);
            ++i;
        }
        CHECK(i == size);

        i = 0;
        for (auto val : v) {
            CHECK(val == i);
            ++i;
        }
        CHECK(i == size);
    }

    SUBCASE("Reverse iterator") {
        theseus::SegmentedVector<int, SegmentSize> v(size, 200);

        int i = size - 1;
        for (auto rit = v.rbegin(); rit != v.rend(); ++rit) {
            CHECK(*rit == 200);
            *rit = i;
            --i;
        }
        CHECK(i == -1);

        i = 0;
        for (auto val : v) {
            CHECK(val == i);
            ++i;
        }
        CHECK(i == size);
    }

    SUBCASE("Iterator access") {
        theseus::SegmentedVector<int, SegmentSize> v(size, 200);

        for (int i = 0; i < size; ++i) {
            v[i] = i;
        }

        auto it1 = v.begin() + 5;
        CHECK(*it1 == 5);
        CHECK(it1[10] == 15);

        auto it2 = v.end() - 5;
        CHECK(*it2 == 15);
        CHECK(it2 - it1 == 10);
        CHECK(it1 != it2);
        CHECK(it1 < it2);
        CHECK(it1 <= it2);
        CHECK(it2 > it1);
        CHECK(it2 >= it1);

        v.clear();
        CHECK(v.begin() == v.end());
        CHECK(it1 <= it2);
        CHECK(it2 >= it1);
    }
}


TEST_CASE("Test vector iterator") {
    test_iterator<16>(); // Power of 2.
    test_iterator<17>(); // Not a power of 2.
}

template <int SegmentSize>
void test_swap() {
    CAPTURE(SegmentSize);

    constexpr int size = 20;
    static_assert(SegmentSize < size && SegmentSize > size / 2);

    theseus::SegmentedVector<int, SegmentSize> v1(size * 2, 200);
    theseus::SegmentedVector<int, SegmentSize> v2(size, 300);

    for (int i = 0; i < size; ++i) {
        CHECK(v1[i] == 200);
        CHECK(v2[i] == 300);
    }

    v1.swap(v2);


    CHECK(v1.size() == size);
    CHECK(v1.capacity() == SegmentSize * 2);

    CHECK(v2.size() == size * 2);
    CHECK(v2.capacity() == SegmentSize * 3);

    for (int i = 0; i < size; ++i) {
        CHECK(v1[i] == 300);
        CHECK(v2[i] == 200);
    }

    std::swap(v1, v2);
    CHECK(v1.size() == size * 2);
    CHECK(v1.capacity() == SegmentSize * 3);

    CHECK(v2.size() == size);
    CHECK(v2.capacity() == SegmentSize * 2);
}

TEST_CASE("Test vector swap") {
    test_swap<16>(); // Power of 2.
    test_swap<17>(); // Not a power of 2.
}

template <int SegmentSize>
void test_realloc_policy() {
    constexpr int size = 20;
    static_assert(SegmentSize < size && SegmentSize > size / 2);

    theseus::SegmentedVector<int, SegmentSize> v(size, 200);
    CHECK(v.size() == size);
    CHECK(v.capacity() == SegmentSize * 2);

    auto realloc_policy = []([[maybe_unused]] std::ptrdiff_t capacity,
                                std::ptrdiff_t required_size) -> std::ptrdiff_t {
        return required_size * 2;
    };

    auto old_capacity = v.capacity();

    for (int i = v.size(); i < v.capacity(); ++i) {
        v.push_back(i);
    }

    try {
        v.push_back(100);
        CHECK(false);
    } catch (const std::length_error &e) {
        CHECK(true);
    }

    v.set_realloc_policy(realloc_policy);

    v.push_back(100);
    CHECK(v.size() == old_capacity + 1);
    CHECK(v.capacity() == SegmentSize * 5);
}

TEST_CASE("Test segmented vector realloc policy") {
    test_realloc_policy<16>(); // Power of 2.
    test_realloc_policy<17>(); // Not a power of 2.
}

template <int SegmentSize>
void test_allocator() {
    constexpr int size = 20;
    static_assert(SegmentSize < size && SegmentSize > size / 2);

    theseus::SegmentedVector<NonPOD, SegmentSize, true, std::allocator<NonPOD>> v(size, 200);

    CHECK(v.size() == size);
    CHECK(v.capacity() == SegmentSize * 2);
    CHECK(v.get_allocator() == std::allocator<NonPOD>());
}

TEST_CASE("Test segmented vector allocator") {
    test_allocator<16>(); // Power of 2.
    test_allocator<17>(); // Not a power of 2.
}

#if 0 // Benchmarking tests.

#include <iostream>
#include <chrono>
#include <deque>

#include "../../theseus/vector.h"

template<typename Vec>
std::pair<long long, int> benchmark_push_back(Vec& v, int reps, int size) {
    using namespace std::chrono;

    int dummy = 0;

    auto start = high_resolution_clock::now();
    for (int i = 0; i < reps; ++i) {
        v.clear();
        for (int j = 0; j < size; ++j) {
            v.push_back(j);
        }
        dummy += v[0];
    }
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start).count();

    return {duration, dummy};
}

template<typename Vec>
std::pair<long long, int> benchmark_access(Vec& v, int reps, int size) {
    using namespace std::chrono;

    int dummy = 0;

    auto start = high_resolution_clock::now();
    for (int i = 0; i < reps; ++i) {
        for (int j = 1; j < size; ++j) {
            dummy += v[j];
        }
    }
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start).count();

    return {duration, dummy};
}

template<typename Vec>
std::pair<long long, int> benchmark_iterator(Vec& v, int reps, int size) {
    using namespace std::chrono;

    v.resize(size);

    int dummy = 0;

    auto start = high_resolution_clock::now();
    for (int i = 0; i < reps; ++i) {
        for (auto it = v.begin(); it != v.end(); ++it) {
            dummy += *it;
        }
    }
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start).count();

    v.clear();

    return {duration, dummy};
}

TEST_CASE("Benchmark SegmentedVector against Vector") {
    constexpr int reps = 10000;
    constexpr int size = 1000000;

    struct BenchEntry {
        std::string name;
        std::function<void()> realloc;
        std::function<std::pair<long long, int>()> push_bench;
        std::function<std::pair<long long, int>()> access_bench;
        std::function<std::pair<long long, int>()> iterator_bench;
    };

    theseus::Vector<int, true> v1;
    theseus::SegmentedVector<int, 4096, true> v2;
    theseus::SegmentedVector<int, 4000, true> v3;
    std::deque<int> dq;

    std::vector<BenchEntry> benches = {
        {
            "Vector",
            [&](){ v1.realloc(size); },
            [&](){ return benchmark_push_back(v1, reps, size); },
            [&](){ return benchmark_access(v1, reps, size); },
            [&](){ return benchmark_iterator(v1, reps, size); }
        },
        {
            "SegmentedVector - 2power",
            [&](){ v2.realloc(size); },
            [&](){ return benchmark_push_back(v2, reps, size); },
            [&](){ return benchmark_access(v2, reps, size); },
            [&](){ return benchmark_iterator(v2, reps, size); }
        },
        {
            "SegmentedVector - !2power",
            [&](){ v3.realloc(size); },
            [&](){ return benchmark_push_back(v3, reps, size); },
            [&](){ return benchmark_access(v3, reps, size); },
            [&](){ return benchmark_iterator(v3, reps, size); }
        },
        {
            "Deque",
            [&](){},
            [&](){ return benchmark_push_back(dq, reps, size); },
            [&](){ return benchmark_access(dq, reps, size); },
            [&](){ return benchmark_iterator(dq, reps, size); }
        }
    };

    // Reallocate all vectors.
    for (auto& bench : benches) {
        bench.realloc();
    }

    std::vector<long long> push_times, access_times, iterator_times;

    // Execute the benchmarks. Print the dummy values to avoid compiler optimizations.
    for (auto& bench : benches) {
        auto [push_time, dummy1] = bench.push_bench();
        auto [access_time, dummy2] = bench.access_bench();
        auto [iter_time, dummy3] = bench.iterator_bench();
        push_times.push_back(push_time);
        access_times.push_back(access_time);
        iterator_times.push_back(iter_time);
        std::cerr << dummy1 << " " << dummy2 << " " << dummy3 << "\n";
    }

    // Print execution times.
    for (size_t i = 0; i < benches.size(); ++i) {
        std::cout << "Push back time (" << benches[i].name << "): " << push_times[i] << " ms\n";
        std::cout << "Access time (" << benches[i].name << "): " << access_times[i] << " ms\n";
        std::cout << "Iterator time (" << benches[i].name << "): " << iterator_times[i] << " ms\n";
    }
}

#endif