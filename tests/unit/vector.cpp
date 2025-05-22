#include "../doctest.h"

#include "../../theseus/vector.h"

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

TEST_CASE("Test vector constructors and assignments") {

    constexpr int size = 500;

    SUBCASE("Default constructor and realloc") {
        theseus::Vector<POD> v;

        CHECK(v.size() == 0);
        CHECK(v.capacity() == 0);

        v.realloc(size);
        CHECK(v.capacity() == size);

        v.resize(size - 2);
        CHECK(v.size() == size - 2);
        CHECK(v.capacity() == size);
    }

    SUBCASE("Constructor with size (default init)") {
        theseus::Vector<POD> v(size);

        CHECK(v.size() == size);
        CHECK(v.capacity() == size);
    }

    SUBCASE("Constructor with size (value init)") {
        theseus::Vector<POD> v(size, POD{200, 201});

        CHECK(v.size() == size);
        CHECK(v.capacity() == size);

        for (int i = 0; i < size; ++i) {
            CHECK(v[i]._a == 200);
            CHECK(v[i]._b == 201);
        }
    }

    SUBCASE("Copy constructor") {
        theseus::Vector<POD> v1(size, POD{200, 201});
        theseus::Vector<POD> v2(v1);

        CHECK(v2.size() == size);
        CHECK(v2.capacity() == size);

        for (int i = 0; i < size; ++i) {
            CHECK(v2[i]._a == 200);
            CHECK(v2[i]._b == 201);
        }
    }

    SUBCASE("Move constructor") {
        theseus::Vector<POD> v1(size, POD{200, 201});
        theseus::Vector<POD> v2(std::move(v1));

        CHECK(v2.size() == size);
        CHECK(v2.capacity() == size);

        for (int i = 0; i < size; ++i) {
            CHECK(v2[i]._a == 200);
            CHECK(v2[i]._b == 201);
        }

        CHECK(v1.size() == 0);
        CHECK(v1.capacity() == 0);
        CHECK(v1.data() == nullptr);
    }

    SUBCASE("Copy assignment") {
        theseus::Vector<POD> v1(size, POD{200, 201});
        theseus::Vector<POD> v2;

        v2 = v1;

        CHECK(v2.size() == size);
        CHECK(v2.capacity() == size);

        for (int i = 0; i < size; ++i) {
            CHECK(v2[i]._a == 200);
            CHECK(v2[i]._b == 201);
        }
    }

    SUBCASE("Move assignment") {
        theseus::Vector<POD> v1(size, POD{200, 201});
        theseus::Vector<POD> v2;

        v2 = std::move(v1);

        CHECK(v2.size() == size);
        CHECK(v2.capacity() == size);

        for (int i = 0; i < size; ++i) {
            CHECK(v2[i]._a == 200);
            CHECK(v2[i]._b == 201);
        }

        CHECK(v1.size() == 0);
        CHECK(v1.capacity() == 0);
        CHECK(v1.data() == nullptr);
    }
}

TEST_CASE("Test vector initialization policies") {
    constexpr int size = 500;

    SUBCASE("POD avoid init") {
        theseus::Vector<POD, true> v;
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
        theseus::Vector<POD, false> v;
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
        theseus::Vector<NonPOD, true> v;
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
        theseus::Vector<NonPOD> v;
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

TEST_CASE("Test vector resize") {
    constexpr int size = 500;

    SUBCASE("Resize with bound check") {
        theseus::Vector<POD, true> v;
        v.realloc(size);

        try {
            v.resize(size + 1);
            CHECK(false);
        } catch (const std::length_error &e) {
            CHECK(true);
        }
    }
    SUBCASE("Resize without bound check") {
        theseus::Vector<POD, true> v;
        v.realloc(size);

        // This leaves the vector in an invalid state.
        try {
            v.resize_unsafe(size + 1);
            CHECK(true);
        } catch (const std::length_error &e) {
            CHECK(false);
        }
        v.resize(0);
    }
}

TEST_CASE("Test vector clear") {
    constexpr int size = 500;

    non_pod_default_constructor_calls = 0;
    non_pod_constructor_calls = 0;
    non_pod_copy_constructor_calls = 0;
    non_pod_move_constructor_calls = 0;
    non_pod_destructor_calls = 0;

    theseus::Vector<NonPOD> v(size, NonPOD{200});
    CHECK(v.size() == size);
    CHECK(v.capacity() == size);

    v.clear();

    CHECK(v.size() == 0);
    CHECK(v.capacity() == size);

    CHECK(non_pod_default_constructor_calls == 0);
    CHECK(non_pod_constructor_calls == 1);
    CHECK(non_pod_copy_constructor_calls == size);
    CHECK(non_pod_move_constructor_calls == 0);
    CHECK(non_pod_destructor_calls == size + 1);
}

TEST_CASE("Test vector emplace_back") {
    constexpr int size = 500;

    theseus::Vector<NonPOD> v;
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

TEST_CASE("Test vector push_back and pop_back") {
    constexpr int size = 500;

    theseus::Vector<NonPOD> v;
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

TEST_CASE("Test vector element access") {
    constexpr int size = 500;

    theseus::Vector<int> v(size, 200);

    CHECK(v.front() == 200);
    CHECK(v.back() == 200);

    for (int i = 0; i < size; ++i) {
        v[i] = i;
    }

    for (int i = 0; i < size; ++i) {
        CHECK(v[i] == i);
    }

    auto *data = v.data();

    for (int i = 0; i < size; ++i) {
        CHECK(data[i] == i);
    }

    v.front() = -1;
    v.back() = -2;

    CHECK(v.front() == -1);
    CHECK(v[0] == -1);

    CHECK(v.back() == -2);
    CHECK(v[size - 1] == -2);

    try {
        v.at(size * 2) = 1;
        CHECK(false);
    } catch (const std::out_of_range &e) {
        CHECK(true);
    }
}

TEST_CASE("Test vector iterator") {
    constexpr int size = 500;

    SUBCASE("Forward iterator") {
        theseus::Vector<int> v(size, 200);

        int i = 0;
        for (auto it = v.begin(); it != v.end(); ++it) {
            CHECK(*it == 200);
            *it = i;
            ++i;
        }

        i = 0;
        for (auto it = v.cbegin(); it != v.cend(); ++it) {
            static_assert(std::is_const<std::remove_reference_t<decltype(*it)>>::value,
                          "*it is not const!");
            CHECK(*it == i);
            ++i;
        }

        i = 0;
        for (auto val : v) {
            CHECK(val == i);
            ++i;
        }
    }

    SUBCASE("Reverse iterator") {
        theseus::Vector<int> v(size, 200);

        int i = size - 1;
        for (auto rit = v.rbegin(); rit != v.rend(); ++rit) {
            CHECK(*rit == 200);
            *rit = i;
            --i;
        }

        i = 0;
        for (auto val : v) {
            CHECK(val == i);
            ++i;
        }
    }

    SUBCASE("Iterator access") {
        theseus::Vector<int> v(size, 200);

        for (int i = 0; i < size; ++i) {
            v[i] = i;
        }

        auto it1 = v.begin() + 100;
        CHECK(*it1 == 100);
        CHECK(it1[10] == 110);

        auto it2 = v.end() - 100;
        CHECK(*it2 == 400);
        CHECK(it2 - it1 == 300);
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

TEST_CASE("Test vector swap") {
    constexpr int size = 500;

    theseus::Vector<int> v1(size * 2, 200);
    theseus::Vector<int> v2(size, 300);

    for (int i = 0; i < size; ++i) {
        CHECK(v1[i] == 200);
        CHECK(v2[i] == 300);
    }

    v1.swap(v2);


    CHECK(v1.size() == size);
    CHECK(v1.capacity() == size);

    CHECK(v2.size() == size * 2);
    CHECK(v2.capacity() == size * 2);

    for (int i = 0; i < size; ++i) {
        CHECK(v1[i] == 300);
        CHECK(v2[i] == 200);
    }

    std::swap(v1, v2);
    CHECK(v1.size() == size * 2);
    CHECK(v1.capacity() == size * 2);

    CHECK(v2.size() == size);
    CHECK(v2.capacity() == size);
}

TEST_CASE("Test vector realloc policy") {
    constexpr int size = 5;

    SUBCASE("No realloc policy") {
        theseus::Vector<int> v(size, 200);

        CHECK(v.size() == size);
        CHECK(v.capacity() == size);

        try {
            v.push_back(100);
            CHECK(false);
        }
        catch (const std::length_error &e) {
            CHECK(true);
        }
    }

    SUBCASE("Realloc policy") {
        theseus::Vector<int, true> v(size, 200);
        CHECK(v.size() == size);
        CHECK(v.capacity() == size);

        auto realloc_policy = []([[maybe_unused]] std::ptrdiff_t capacity,
                                 std::ptrdiff_t required_size) -> std::ptrdiff_t {
            return required_size * 2;
        };

        v.set_realloc_policy(realloc_policy);
        // CHECK(v.get_realloc_policy() == realloc_policy);

        v.push_back(100);
        CHECK(v.size() == size + 1);
        CHECK(v.capacity() == (size + 1) * 2);
    }
}

TEST_CASE("Test vector allocator") {
    // TODO: We may want to extend this test.
    constexpr int size = 500;

    theseus::Vector<NonPOD, true, std::allocator<NonPOD>> v(size, 200);

    CHECK(v.size() == size);
    CHECK(v.capacity() == size);
    CHECK(v.get_allocator() == std::allocator<NonPOD>());
}
