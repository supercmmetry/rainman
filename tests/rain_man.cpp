#include "gtest/gtest.h"
#include <memmgr.h>
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

    mgr->wipe();

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

    mgr->wipe();

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

    child1->wipe();

    ASSERT_EQ(mgr->get_alloc_size(), 0);

    child2->r_malloc<char>(100);

    ASSERT_EQ(mgr->get_alloc_count(), 1);

    child1->r_malloc<char>(100);
    child1_1 = child1->create_child_mgr();

    child1_1->r_malloc<char>(100);

    ASSERT_EQ(mgr->get_alloc_count(), 3);
    ASSERT_EQ(child1->get_alloc_count(), 2);

    child2->wipe();

    ASSERT_EQ(mgr->get_alloc_count(), 2);
    ASSERT_EQ(child1->get_alloc_count(), 2);

}
