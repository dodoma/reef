#include "reef.h"

void test_basic()
{
    MTREE *tree = NULL;
    MERR *err;

    for (int i = 0; i < 100; i++) {
        char key[20];
        sprintf(key, "%d", i);
        tree = mtree_insert(tree, key, MOS_OFFSET_2_MEM(i));
    }

    //mtree_print(tree, 0);

    MTEST_ASSERT(MOS_MEM_2_OFFSET(mtree_lookup(tree, "20")) == 20);
    MTEST_ASSERT(MOS_MEM_2_OFFSET(mtree_lookup(tree, "200")) == 0);
}

void suite_basic()
{
    mtest_add_test(test_basic, "basic");
    //mtest_add_test(test_example, "example");
}

int main()
{
    mtc_init("-", MTC_DEBUG);

    mtest_add_suite(suite_basic, "basic");

    return mtest_run();
}
