#include "gtest/gtest.h"
#include <memmgr.h>
#include <vector>

class MemoryTest : public testing::Test {
};

TEST(MemoryTest, rain_man_unittest_segv) {
    auto memmgr = new rain_man_mgr;
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

