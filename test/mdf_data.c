#include "reef.h"

void test_basic()
{
    MDF *anode, *bnode, *cnode, *outnode;
    MERR *err;

    mdf_init(&anode);
    mdf_init(&bnode);
    mdf_init(&cnode);
    mdf_init(&outnode);

    err = mdf_json_import_file(anode, "data/config_000.json");
    TRACE_NOK(err);
    err = mdf_json_import_file(outnode, "data/out_000.json");
    TRACE_NOK(err);

    mdf_data_rend(anode, bnode, cnode);
    //mdf_json_export_file(cnode, "-");
    //mdf_json_export_file(outnode, "-");
    MTEST_ASSERT(mdf_equal(cnode, outnode) == true);

    /* 01 */
    mdf_clear(anode);
    mdf_clear(bnode);
    mdf_clear(outnode);
    mdf_clear(cnode);

    err = mdf_json_import_file(anode, "data/config_01.json");
    TRACE_NOK(err);
    err = mdf_json_import_file(bnode, "data/data_01.json");
    TRACE_NOK(err);
    err = mdf_json_import_file(outnode, "data/out_01.json");
    TRACE_NOK(err);

    mdf_data_rend(anode, bnode, cnode);
    //mdf_json_export_file(cnode, "-");
    //mdf_json_export_file(outnode, "-");
    MTEST_ASSERT(mdf_equal(cnode, outnode) == true);

    /* 02 */
    mdf_clear(anode);
    mdf_clear(bnode);
    mdf_clear(outnode);
    mdf_clear(cnode);

    err = mdf_json_import_file(anode, "data/config_02.json");
    TRACE_NOK(err);
    err = mdf_json_import_file(bnode, "data/data_02.json");
    TRACE_NOK(err);
    err = mdf_json_import_file(outnode, "data/out_02.json");
    TRACE_NOK(err);

    mdf_data_rend(anode, bnode, cnode);
    //mdf_json_export_file(cnode, "-");
    //mdf_json_export_file(outnode, "-");
    MTEST_ASSERT(mdf_equal(cnode, outnode) == true);

    /* 03 */
    mdf_clear(anode);
    mdf_clear(bnode);
    mdf_clear(outnode);
    mdf_clear(cnode);

    err = mdf_json_import_file(anode, "data/config_03.json");
    TRACE_NOK(err);
    err = mdf_json_import_file(bnode, "data/data_03.json");
    TRACE_NOK(err);
    err = mdf_json_import_file(outnode, "data/out_03.json");
    TRACE_NOK(err);

    mdf_data_rend(anode, bnode, cnode);
    //mdf_json_export_file(cnode, "-");
    //mdf_json_export_file(outnode, "-");
    MTEST_ASSERT(mdf_equal(cnode, outnode) == true);

    /* 04 */
    mdf_clear(anode);
    mdf_clear(bnode);
    mdf_clear(outnode);
    mdf_clear(cnode);

    err = mdf_json_import_file(anode, "data/config_04.json");
    TRACE_NOK(err);
    err = mdf_json_import_file(bnode, "data/data_04.json");
    TRACE_NOK(err);
    err = mdf_json_import_file(outnode, "data/out_04.json");
    TRACE_NOK(err);

    mdf_data_rend(anode, bnode, cnode);
    /* 浮点转成字符串时无法精确比较 */
    mdf_remove(cnode, "j");
    //mdf_json_export_file(cnode, "-");
    //mdf_json_export_file(outnode, "-");
    MTEST_ASSERT(mdf_equal(cnode, outnode) == true);

    mdf_destroy(&anode);
    mdf_destroy(&bnode);
    mdf_destroy(&cnode);
    mdf_destroy(&outnode);
}

void test_output()
{
    MDF *anode, *bnode, *cnode, *outnode;
    MERR *err;

    mdf_init(&anode);
    mdf_init(&bnode);
    mdf_init(&cnode);
    mdf_init(&outnode);

    /* 10 */
    mdf_clear(anode);
    mdf_clear(bnode);
    mdf_clear(outnode);
    mdf_clear(cnode);

    err = mdf_json_import_file(anode, "data/config_10.json");
    TRACE_NOK(err);
    err = mdf_json_import_file(bnode, "data/data_10.json");
    TRACE_NOK(err);
    err = mdf_json_import_file(outnode, "data/out_10.json");
    TRACE_NOK(err);

    mdf_data_rend(anode, bnode, cnode);
    //mdf_json_export_file(cnode, "-");
    //mdf_json_export_file(outnode, "-");
    MTEST_ASSERT(mdf_equal(cnode, outnode) == true);


    /* 11 */
    mdf_clear(anode);
    mdf_clear(bnode);
    mdf_clear(outnode);
    mdf_clear(cnode);

    err = mdf_json_import_file(anode, "data/config_11.json");
    TRACE_NOK(err);
    err = mdf_json_import_file(bnode, "data/data_11.json");
    TRACE_NOK(err);
    err = mdf_json_import_file(outnode, "data/out_11.json");
    TRACE_NOK(err);

    mdf_data_rend(anode, bnode, cnode);
    //mdf_json_export_file(cnode, "-");
    //mdf_json_export_file(outnode, "-");
    MTEST_ASSERT(mdf_equal(cnode, outnode) == true);


    /* 12 */
    mdf_clear(anode);
    mdf_clear(bnode);
    mdf_clear(outnode);
    mdf_clear(cnode);

    err = mdf_json_import_file(anode, "data/config_12.json");
    TRACE_NOK(err);
    err = mdf_json_import_file(bnode, "data/data_12.json");
    TRACE_NOK(err);
    err = mdf_json_import_file(outnode, "data/out_12.json");
    TRACE_NOK(err);

    mdf_data_rend(anode, bnode, cnode);
    //mdf_json_export_file(cnode, "-");
    //mdf_json_export_file(outnode, "-");
    MTEST_ASSERT(mdf_equal(cnode, outnode) == true);


    /* 13 */
    mdf_clear(anode);
    mdf_clear(bnode);
    mdf_clear(outnode);
    mdf_clear(cnode);

    err = mdf_json_import_file(anode, "data/config_13.json");
    TRACE_NOK(err);
    err = mdf_json_import_file(bnode, "data/data_13.json");
    TRACE_NOK(err);
    err = mdf_json_import_file(outnode, "data/out_13.json");
    TRACE_NOK(err);

    mdf_data_rend(anode, bnode, cnode);
    //mdf_json_export_file(cnode, "-");
    //mdf_json_export_file(outnode, "-");
    MTEST_ASSERT(mdf_equal(cnode, outnode) == true);


    mdf_destroy(&anode);
    mdf_destroy(&bnode);
    mdf_destroy(&cnode);
    mdf_destroy(&outnode);
}

void test_reserve()
{
    MDF *anode, *bnode, *cnode, *outnode;
    MERR *err;

    mdf_init(&anode);
    mdf_init(&bnode);
    mdf_init(&cnode);
    mdf_init(&outnode);

    /* reserve */
    mdf_clear(anode);
    mdf_clear(bnode);
    mdf_clear(outnode);
    mdf_clear(cnode);

    err = mdf_json_import_file(anode, "data/config_reserve.json");
    TRACE_NOK(err);
    err = mdf_json_import_file(bnode, "data/data_reserve.json");
    TRACE_NOK(err);
    err = mdf_json_import_file(outnode, "data/out_reserve.json");
    TRACE_NOK(err);

    mdf_data_rend(anode, bnode, cnode);
    //mdf_json_export_file(cnode, "-");
    //mdf_json_export_file(outnode, "-");
    MTEST_ASSERT(mdf_equal(cnode, outnode) == true);

    /* array */
    mdf_clear(anode);
    mdf_clear(bnode);
    mdf_clear(outnode);
    mdf_clear(cnode);

    err = mdf_json_import_file(anode, "data/config_array.json");
    TRACE_NOK(err);
    err = mdf_json_import_file(bnode, "data/data_array.json");
    TRACE_NOK(err);
    err = mdf_json_import_file(outnode, "data/out_array.json");
    TRACE_NOK(err);

    mdf_data_rend(anode, bnode, cnode);
    //mdf_json_export_file(cnode, "-");
    //mdf_json_export_file(outnode, "-");
    MTEST_ASSERT(mdf_equal(cnode, outnode) == true);

    mdf_destroy(&anode);
    mdf_destroy(&bnode);
    mdf_destroy(&cnode);
    mdf_destroy(&outnode);
}

void suite_rend()
{
    mtest_add_test(test_basic, "no output");
    mtest_add_test(test_output, "have output");
    mtest_add_test(test_reserve, "reserved keyword");
}

int main()
{
    mtc_init("-", MTC_DEBUG);

    mtest_add_suite(suite_rend, "rend");

    return mtest_run();
}
