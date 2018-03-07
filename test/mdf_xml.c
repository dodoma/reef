#include "reef.h"

void test_file()
{
    MDF *node;
    MERR *err;

    mdf_init(&node);
    err = mdf_xml_import_file(node, "data/test.xml1");
    TRACE_NOK_MT(err);
    mdf_xml_export_file(node, "data/test.out1");
    mdf_destroy(&node);
    MTEST_ASSERT_FILE_EQ("data/test.out1", "data/test.res1");

    mdf_init(&node);
    err = mdf_xml_import_file(node, "data/test.xml2");
    TRACE_NOK_MT(err);
    mdf_xml_export_file(node, "data/test.out2");
    mdf_destroy(&node);
    MTEST_ASSERT_FILE_EQ("data/test.out2", "data/test.res2");

    mdf_init(&node);
    err = mdf_xml_import_file(node, "data/test.xml3");
    TRACE_NOK_MT(err);
    mdf_xml_export_file(node, "data/test.out3");
    mdf_destroy(&node);
    MTEST_ASSERT_FILE_EQ("data/test.out3", "data/test.res3");

    mdf_init(&node);
    err = mdf_xml_import_file(node, "data/test.xml4");
    TRACE_NOK_MT(err);
    mdf_xml_export_file(node, "data/test.out4");
    mdf_destroy(&node);
    MTEST_ASSERT_FILE_EQ("data/test.out4", "data/test.res4");
}

void suite_file()
{
    mtest_add_test(test_file, "xml file");
}

int main()
{
    mtc_mt_init("-", "main", MTC_DEBUG);

    mtest_add_suite(suite_file, "file");

    return mtest_run();
}
