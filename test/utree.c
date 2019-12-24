#include "reef.h"

void test_basic()
{
    MUTREE *utree = NULL;
    MERR *err;

    for (int i = 0; i < 100; i++) {
        utree = mutree_insert(utree, i, MOS_OFFSET_2_MEM(i));
    }

    //mutree_print(utree, 0);

    MTEST_ASSERT(MOS_MEM_2_OFFSET(mutree_lookup(utree, 20)) == 20);
    MTEST_ASSERT(MOS_MEM_2_OFFSET(mutree_lookup(utree, 200)) == 0);
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
