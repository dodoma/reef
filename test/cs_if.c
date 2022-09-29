#include "reef.h"

void test_basic()
{
    MERR *err;


    MCS *tpl;
    MDF *node;
    mdf_init(&node);

    err = mcs_parse_file("test_if.cs", NULL, NULL, &tpl);
    TRACE_NOK(err);

    mcs_dump(tpl);

    mdf_set_value(node, "New.name", "newman");
    mdf_set_value(node, "desc", "一张图片");
    mdf_set_value(node, "images.0", "https://mbox.net.cn/x.jpg");
    mdf_set_value(node, "images.1", "https://mbox.net.cn/y.jpg");
    mdf_set_int_value(node, "Count", 100);

    MDF_TRACE(node);

    err = mcs_rend(tpl, node, "-");
    TRACE_NOK(err);

    mdf_destroy(&node);
    mcs_destroy(&tpl);

    mtc_dbg("%f", mcs_eval_numberic("10/29"));
    mtc_dbg("%f", mcs_eval_numberic("301 / 12"));
    mtc_dbg("%f", mcs_eval_numberic("10 + 10 * 10"));
    mtc_dbg("%f", mcs_eval_numberic("301.1212 / 12.111"));
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
