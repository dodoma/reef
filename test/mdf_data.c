#include "reef.h"

void test_basic()
{
    MDF *anode, *bnode, *cnode;

    mdf_init(&anode);
    mdf_init(&bnode);
    mdf_init(&cnode);

    mdf_json_import_file(anode, "mdf.json");
    mdf_json_import_file(cnode, "mdf.json");

    MTEST_ASSERT(mdf_equal(anode, bnode) == false);
    MTEST_ASSERT(mdf_equal(anode, cnode) == true);

    mdf_set_value(anode, "xx", "yy");
    mdf_set_value(cnode, "xx", "yy");
    mdf_set_value(anode, "data.xx", "yy");
    mdf_set_value(cnode, "data.xx", "yy");

    MTEST_ASSERT(mdf_equal(anode, cnode) == true);

    mdf_destroy(&anode);
    mdf_destroy(&bnode);
    mdf_destroy(&cnode);
}

void suite_rend()
{
    mtest_add_test(test_basic, "basic");
}

int main()
{
    mtc_init("-", MTC_DEBUG);

    mtest_add_suite(suite_rend, "rend");

    return mtest_run();
}
