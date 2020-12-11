#include "gtest/gtest.h"
#include <memmgr.h>
#include <cache.h>
#include <vector>

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

TEST(MemoryTest, rain_man_wipe) {
    auto mgr = new rainman::memmgr;
    std::vector<int*> ptr_vec;

    for (int i = 0; i < 2000; i++) {
        mgr->r_malloc<int>(20);
    }

    mgr->wipe<int>();

    ASSERT_EQ(mgr->get_alloc_count(), 0);
    ASSERT_EQ(mgr->get_alloc_size(), 0);

    for (int i = 0; i < 10; i++) {
        auto ptr = mgr->r_malloc<int>(20);
        ptr_vec.push_back(ptr);
    }

    for (int i = 0; i < ptr_vec.size(); i++) {
        mgr->r_free(ptr_vec[i]);
    }

    ASSERT_EQ(mgr->get_alloc_size(), 0);
    ASSERT_EQ(mgr->get_alloc_count(), 0);

    mgr->wipe<int>();

    ASSERT_EQ(mgr->get_alloc_count(), 0);
    ASSERT_EQ(mgr->get_alloc_size(), 0);
}

TEST(MemoryTest, rain_man_child_tree_1) {
    auto *mgr = new rainman::memmgr;
    auto child1 = mgr->create_child_mgr();
    auto child2 = mgr->create_child_mgr();
    auto child1_1 = child1->create_child_mgr();

    child1_1->r_malloc<char>(100);

    ASSERT_EQ(child1->get_alloc_count(), 1);
    ASSERT_EQ(mgr->get_alloc_size(), 100);

    child1->wipe<void>();

    ASSERT_EQ(mgr->get_alloc_size(), 0);

    child2->r_malloc<char>(100);

    ASSERT_EQ(mgr->get_alloc_count(), 1);

    child1->r_malloc<char>(100);
    child1_1 = child1->create_child_mgr();

    child1_1->r_malloc<char>(100);

    ASSERT_EQ(mgr->get_alloc_count(), 3);
    ASSERT_EQ(child1->get_alloc_count(), 2);

    child2->wipe<void>();

    ASSERT_EQ(mgr->get_alloc_count(), 2);
    ASSERT_EQ(child1->get_alloc_count(), 2);

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

