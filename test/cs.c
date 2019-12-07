#include "reef.h"

void test_basic()
{
    MERR *err;


    MCS *tpl;
    MDF *node;
    mdf_init(&node);

    err = mcs_parse_file("test1.cs", NULL, NULL, &tpl);
    TRACE_NOK(err);

    mdf_set_value(node, "New.name", "hahahahah");
    mdf_set_value(node, "desc", "一张图片");
    mdf_set_value(node, "images.0", "https://mbox.net.cn/x.jpg");
    mdf_set_value(node, "images.1", "https://mbox.net.cn/y.jpg");

    mdf_set_value(node, "users.0.name", "张三");
    mdf_set_value(node, "users.0.score", "84");
    mdf_set_value(node, "users.1.name", "李四");
    mdf_set_value(node, "users.1.score", "87");

    err = mcs_rend(tpl, node, "-");
    TRACE_NOK(err);

    mdf_destroy(&node);
    mcs_destroy(&tpl);
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
