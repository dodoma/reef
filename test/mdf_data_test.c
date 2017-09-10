#include "reef.h"

void test_basic()
{
    MDF *anode, *bnode, *cnode;
    MERR *err;

    mdf_init(&anode);
    mdf_init(&bnode);
    mdf_init(&cnode);

    err = mdf_json_import_file(anode, "data/config_test.json");
    TRACE_NOK(err);
    err = mdf_json_import_file(bnode, "data/data_test.json");
    TRACE_NOK(err);

    mdf_data_rend(anode, bnode, cnode);
    mdf_json_export_file(cnode, "-");

    mdf_destroy(&anode);
    mdf_destroy(&bnode);
    mdf_destroy(&cnode);
}

void suite_rend()
{
    mtest_add_test(test_basic, "no output");
}

int main()
{
    mtc_init("-", MTC_DEBUG);

    mtest_add_suite(suite_rend, "rend");

    return mtest_run();
}
