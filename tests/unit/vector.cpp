#include "../doctest.h"

#include "../../swfa/manual_capacity_vector.h"

static int non_pod_default_constructor_calls;
static int non_pod_constructor_calls;
static int non_pod_copy_constructor_calls;
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

    ~NonPOD() {
        ++non_pod_destructor_calls;
    }
};

TEST_CASE("Test vector constructors and assignments (standard layout and trivial)") {
    constexpr int size = 500;

    SUBCASE("Default constructor and realloc") {
        swfa::ManualCapacityVector<POD> v;

        CHECK(v.size() == 0);
        CHECK(v.capacity() == 0);

        v.realloc(size);
        // We are going out of bounds, but this int is a trivial type.
        // The vector should never initialize a vector of ints (if not specified).
        for (int i = 0; i < size; ++i) {
            v[i]._a = 200;
            v[i]._b = 201;
        }

        // Check that the vector is not initialized in the resize.
        v.resize(size);
        for (int i = 0; i < size; ++i) {
            CHECK(v[i]._a == 200);
            CHECK(v[i]._b == 201);
        }

        v.resize(0);
        // Now it should be initialized.
        v.resize(size, {100, 101});
        for (int i = 0; i < size; ++i) {
            CHECK(v[i]._a == 100);
            CHECK(v[i]._b == 101);
        }
    }

    SUBCASE("Constructor with size (default init)") {
        swfa::ManualCapacityVector<POD> v(size);

        CHECK(v.size() == size);
        CHECK(v.capacity() == size);
    }

    SUBCASE("Constructor with size (value init)") {
        swfa::ManualCapacityVector<POD> v(size, {200, 201});

        CHECK(v.size() == size);
        CHECK(v.capacity() == size);

        for (int i = 0; i < size; ++i) {
            CHECK(v[i]._a == 200);
            CHECK(v[i]._b == 201);
        }
    }

    SUBCASE("Copy constructor") {
        swfa::ManualCapacityVector<POD> v1(size, {200, 201});
        swfa::ManualCapacityVector<POD> v2(v1);

        CHECK(v2.size() == size);
        CHECK(v2.capacity() == size);

        for (int i = 0; i < size; ++i) {
            CHECK(v2[i]._a == 200);
            CHECK(v2[i]._b == 201);
        }
    }

    SUBCASE("Move constructor") {
        swfa::ManualCapacityVector<POD> v1(size, {200, 201});
        swfa::ManualCapacityVector<POD> v2(std::move(v1));

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
        swfa::ManualCapacityVector<POD> v1(size, {200, 201});
        swfa::ManualCapacityVector<POD> v2;

        v2 = v1;

        CHECK(v2.size() == size);
        CHECK(v2.capacity() == size);

        for (int i = 0; i < size; ++i) {
            CHECK(v2[i]._a == 200);
            CHECK(v2[i]._b == 201);
        }
    }

    SUBCASE("Move assignment") {
        swfa::ManualCapacityVector<POD> v1(size, {200, 201});
        swfa::ManualCapacityVector<POD> v2;

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

TEST_CASE("Test vector constructors and assignments (non-trivial)") {
    constexpr int size = 500;

    SUBCASE("Default constructor and realloc") {
        non_pod_default_constructor_calls = 0;
        non_pod_constructor_calls = 0;
        non_pod_copy_constructor_calls = 0;
        non_pod_destructor_calls = 0;

        swfa::ManualCapacityVector<NonPOD> v;
        CHECK(v.size() == 0);
        CHECK(v.capacity() == 0);

        v.realloc(size);
        // Initialize some elements out of bounds.
        for (int i = 0; i < size; ++i) {
            v[i]._a = 200;
        }

        CHECK(non_pod_default_constructor_calls == 0);
        CHECK(non_pod_constructor_calls == 0);
        CHECK(non_pod_copy_constructor_calls == 0);
        CHECK(non_pod_destructor_calls == 0);

        // Check that the vector is initialized in the resize.
        v.resize(size);

        CHECK(non_pod_default_constructor_calls == 1);
        CHECK(non_pod_constructor_calls == 0);
        CHECK(non_pod_copy_constructor_calls == size);
        CHECK(non_pod_destructor_calls == 1);

        for (int i = 0; i < size; ++i) {
            CHECK(v[i]._a == 100);
        }

        v.resize(0);

        CHECK(non_pod_default_constructor_calls == 1);
        CHECK(non_pod_constructor_calls == 0);
        CHECK(non_pod_copy_constructor_calls == size);
        CHECK(non_pod_destructor_calls == size + 1);

        // Now it should be initialized to other value.
        v.resize(size, 200);

        CHECK(non_pod_default_constructor_calls == 1);
        CHECK(non_pod_constructor_calls == 1);
        CHECK(non_pod_copy_constructor_calls == size * 2);
        CHECK(non_pod_destructor_calls == size + 1 + 1);

        for (int i = 0; i < size; ++i) {
            CHECK(v[i]._a == 200);
        }
    }

    SUBCASE("Constructor with size (default init)") {
        non_pod_default_constructor_calls = 0;
        non_pod_constructor_calls = 0;
        non_pod_copy_constructor_calls = 0;
        non_pod_destructor_calls = 0;

        swfa::ManualCapacityVector<NonPOD> v(size);

        CHECK(v.size() == size);
        CHECK(v.capacity() == size);

        CHECK(non_pod_default_constructor_calls == 1);
        CHECK(non_pod_constructor_calls == 0);
        CHECK(non_pod_copy_constructor_calls == size);
        CHECK(non_pod_destructor_calls == 1);

        for (int i = 0; i < size; ++i) {
            CHECK(v[i]._a == 100);
        }
    }

    SUBCASE("Constructor with size (value init)") {
        non_pod_default_constructor_calls = 0;
        non_pod_constructor_calls = 0;
        non_pod_copy_constructor_calls = 0;
        non_pod_destructor_calls = 0;

        swfa::ManualCapacityVector<NonPOD> v(size, 200);

        CHECK(v.size() == size);
        CHECK(v.capacity() == size);

        CHECK(non_pod_default_constructor_calls == 0);
        CHECK(non_pod_constructor_calls == 1);
        CHECK(non_pod_copy_constructor_calls == size);
        CHECK(non_pod_destructor_calls == 1);

        for (int i = 0; i < size; ++i) {
            CHECK(v[i]._a == 200);
        }
    }

    SUBCASE("Copy constructor") {
        non_pod_default_constructor_calls = 0;
        non_pod_constructor_calls = 0;
        non_pod_copy_constructor_calls = 0;
        non_pod_destructor_calls = 0;

        swfa::ManualCapacityVector<NonPOD> v1(size, 200);
        swfa::ManualCapacityVector<NonPOD> v2(v1);

        CHECK(v2.size() == size);
        CHECK(v2.capacity() == size);

        CHECK(non_pod_default_constructor_calls == 0);
        CHECK(non_pod_constructor_calls == 1);
        CHECK(non_pod_copy_constructor_calls == size * 2);
        CHECK(non_pod_destructor_calls == 1);

        for (int i = 0; i < size; ++i) {
            CHECK(v2[i]._a == 200);
        }
    }

    SUBCASE("Move constructor") {
        non_pod_default_constructor_calls = 0;
        non_pod_constructor_calls = 0;
        non_pod_copy_constructor_calls = 0;
        non_pod_destructor_calls = 0;

        swfa::ManualCapacityVector<NonPOD> v1(size, 200);
        swfa::ManualCapacityVector<NonPOD> v2(std::move(v1));

        CHECK(v2.size() == size);
        CHECK(v2.capacity() == size);

        CHECK(non_pod_default_constructor_calls == 0);
        CHECK(non_pod_constructor_calls == 1);
        CHECK(non_pod_copy_constructor_calls == size);
        CHECK(non_pod_destructor_calls == 1);

        for (int i = 0; i < size; ++i) {
            CHECK(v2[i]._a == 200);
        }

        CHECK(v1.size() == 0);
        CHECK(v1.capacity() == 0);
        CHECK(v1.data() == nullptr);
    }

    SUBCASE("Copy assignment") {
        non_pod_default_constructor_calls = 0;
        non_pod_constructor_calls = 0;
        non_pod_copy_constructor_calls = 0;
        non_pod_destructor_calls = 0;

        swfa::ManualCapacityVector<NonPOD> v1(size, 200);
        swfa::ManualCapacityVector<NonPOD> v2(1, 50);

        v2 = v1;

        CHECK(non_pod_default_constructor_calls == 0);
        CHECK(non_pod_constructor_calls == 1 + 1);
        CHECK(non_pod_copy_constructor_calls == size + 1 + size);
        CHECK(non_pod_destructor_calls == 1 + 1 + 1);

        for (int i = 0; i < size; ++i) {
            CHECK(v2[i]._a == 200);
        }
    }

    SUBCASE("Move assignment") {
        non_pod_default_constructor_calls = 0;
        non_pod_constructor_calls = 0;
        non_pod_copy_constructor_calls = 0;
        non_pod_destructor_calls = 0;

        swfa::ManualCapacityVector<NonPOD> v1(size, 200);
        swfa::ManualCapacityVector<NonPOD> v2(1, 50);

        v2 = std::move(v1);

        CHECK(v2.size() == size);
        CHECK(v2.capacity() == size);

        CHECK(non_pod_default_constructor_calls == 0);
        CHECK(non_pod_constructor_calls == 1 + 1);
        CHECK(non_pod_copy_constructor_calls == 1 + size);
        CHECK(non_pod_destructor_calls == 1 + 1 + 1);

        for (int i = 0; i < size; ++i) {
            CHECK(v2[i]._a == 200);
        }
        CHECK(v1.size() == 0);
        CHECK(v1.capacity() == 0);
        CHECK(v1.data() == nullptr);
    }
}

TEST_CASE("Test vector pushback") {
    constexpr int size = 500;

    swfa::ManualCapacityVector<int> v;

    CHECK(v.empty() == true);

    v.realloc(size);

    CHECK(v.empty() == true);

    for (int i = 0; i < size; ++i) {
        v.push_back(i);
    }

    try {
        v.push_back(-1);
        CHECK(false);
    } catch (const std::runtime_error &e) {
        CHECK(true);
    }

    CHECK(v.empty() == false);

    for (int i = 0; i < size; ++i) {
        CHECK(v[i] == i);
    }

    v.resize(0);

    CHECK(v.empty() == true);

    for (int i = 0; i < size; ++i) {
        v.push_back_unsafe(i + size);
    }

    CHECK(v.empty() == false);

    for (int i = 0; i < size; ++i) {
        CHECK(v[i] == i + size);
    }
}

TEST_CASE("Test vector element access") {
    constexpr int size = 500;

    swfa::ManualCapacityVector<int> v(size, 200);

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
}

TEST_CASE("Test vector resize") {
    constexpr int size = 500;

    swfa::ManualCapacityVector<int> v;

    try {
        v.resize(size);
        CHECK(false);
    } catch (const std::length_error &e) {
        CHECK(true);
    }

    v.realloc(size);
    v.resize(size);

    try {
        v.resize(size + 1);
        CHECK(false);
    } catch (const std::length_error &e) {
        CHECK(true);
    }

    // Now the vector is invalid.
    // We can do this because the vector is of trivial type.
    v.resize_unsafe(size * 2);
}

TEST_CASE("Test vector iterator") {
    constexpr int size = 500;

    swfa::ManualCapacityVector<int> v(size, 200);

    int i = 0;
    for (auto it = v.begin(); it != v.end(); ++it) {
        *it = i;
        ++i;
    }

    for (i = 0; i < size; ++i) {
        CHECK(v[i] == i);
    }
}

TEST_CASE("Test vector swap") {
    constexpr int size = 500;

    swfa::ManualCapacityVector<int> v1(size * 2, 200);
    swfa::ManualCapacityVector<int> v2(size, 300);

    v1.swap(v2);

    CHECK(v1.size() == size);
    CHECK(v1.capacity() == size);

    CHECK(v2.size() == size * 2);
    CHECK(v2.capacity() == size * 2);

    for (int i = 0; i < size; ++i) {
        CHECK(v1[i] == 300);
        CHECK(v2[i] == 200);
    }
}