#include "gtest/gtest.h"
#include <vector>
#include <rainman/rainman.h>

class MemoryTest : public testing::Test {
};

TEST(MemoryTest, rain_man_segv) {
    auto memmgr = new rainman::memmgr;
    std::vector<int*> ptr_vec;
    int *x = memmgr->r_malloc<int>(2000);

    for (int i = 0; i < 2000; i++) {
        x[i] = i + 1;
        auto ptr = memmgr->r_malloc<int>(20);
        ptr_vec.push_back(ptr);
    }

    for (int i = 0; i < ptr_vec.size(); i++) {
        memmgr->r_free(ptr_vec[i]);
    }

    ASSERT_EQ(memmgr->get_alloc_size(), 2000 * sizeof(int));
    ASSERT_EQ(memmgr->get_alloc_count(), 1);

    memmgr->r_free(x);
}

TEST(MemoryTest, rainman_cache_1) {
    remove("cache.rain");
    auto tmp = fopen("cache.rain", "a");
    fclose(tmp);

    auto cache_file = fopen("cache.rain", "rb+");
    auto cache = rainman::cache(cache_file, 128);

    cache.write(1234, 0);
    ASSERT_EQ(cache.read<int>(0), 1234);

    cache.write(3.141f, 0);
    ASSERT_EQ(cache.read<float>(0), 3.141f);
}

TEST(MemoryTest, rainman_cache_2) {
    remove("cache.rain");
    auto tmp = fopen("cache.rain", "a");
    fclose(tmp);

    auto cache_file = fopen("cache.rain", "rb+");
    auto cache = rainman::cache(cache_file, 2);

    auto index1 = cache.allocate<int>(1);
    cache.write(1234, index1);
    ASSERT_EQ(cache.read<int>(index1), 1234);

    auto index2 = cache.allocate<double>(1);
    cache.write(3.141, index2);
    ASSERT_EQ(cache.read<double>(index2), 3.141);

    ASSERT_EQ(index1 + sizeof(int), index2);
}

TEST(MemoryTest, rainman_cache_3) {
    remove("cache.rain");
    auto tmp = fopen("cache.rain", "a");
    fclose(tmp);

    auto cache_file = fopen("cache.rain", "rb+");
    auto cache = rainman::cache(cache_file, 2);

    auto index1 = cache.allocate<int>(1);
    cache.write(1234, index1);
    ASSERT_EQ(cache.read<int>(index1), 1234);

    cache.deallocate(index1);

    auto index2 = cache.allocate<int>(1);
    cache.write(431, index2);
    ASSERT_EQ(cache.read<int>(index2), 431);

    ASSERT_EQ(index1, index2);
}

TEST(MemoryTest, rainman_cache_4) {
    remove("cache.rain");
    auto tmp = fopen("cache.rain", "a");
    fclose(tmp);

    auto cache_file = fopen("cache.rain", "rb+");
    auto cache = rainman::cache(cache_file, 0x10000);

    auto index1 = cache.allocate<int>(1048576);

    for (int i = 0; i < 1048576; i++) {
        cache.write(i, index1 + i * sizeof(int));
    }

    for (int i = 0; i < 1048576; i++) {
        ASSERT_EQ(cache.read<int>(index1 + i * sizeof(int)), i);
    }
}

TEST(MemoryTest, rainman_cache_5) {
    remove("cache.rain");
    auto tmp = fopen("cache.rain", "a");
    fclose(tmp);

    auto cache_file = fopen("cache.rain", "rb+");
    auto cache = rainman::cache(cache_file, 2);

    auto index1 = cache.allocate<uint64_t>(1);
    cache.write(1234, index1);
    ASSERT_EQ(cache.read<uint64_t>(index1), 1234);

    cache.deallocate(index1);

    auto index2 = cache.allocate<int>(1);
    cache.write(431, index2);
    ASSERT_EQ(cache.read<int>(index2), 431);

    ASSERT_EQ(index1, index2);
}

TEST(MemoryTest, rainman_virtual_array_1) {
    remove("cache.rain");
    auto tmp = fopen("cache.rain", "a");
    fclose(tmp);

    auto cache_file = fopen("cache.rain", "rb+");
    auto cache = rainman::cache(cache_file, 0x2000);

    auto arr = rainman::virtual_array<int>(cache, 1048576);

    for (int i = 0; i < 1048576; i++) {
        arr.set(i, i);
    }

    for (int i = 0; i < 1048576; i++) {
        ASSERT_EQ(arr[i], i);
    }
}

TEST(MemoryTest, rainman_pointer_1) {
    auto p = rainman::ptr<int>(20);

    {
        auto z = p;
        z[0] = 12;
    }

    ASSERT_EQ(p[0], 12);
}

TEST(MemoryTest, rainman_pointer_2) {
    auto p = rainman::ptr<int>(20);

    {
        auto z = rainman::ptr<int>(10);
        z[0] = 13;
        z = p;
        *z = 12;
    }

    ASSERT_EQ(p[0], 12);
}

