#include "reef.h"

void test_string_short()
{
    MDF *node;
    char *str;
    MERR *err;

    mdf_init(&node);
    err = mdf_json_import_string(node, "[1, 2,,]");
    MTEST_ASSERT(err != MERR_OK);
    //TRACE_NOK(err);
    merr_destroy(&err);
    mdf_destroy(&node);

    mdf_init(&node);
    err = mdf_json_import_string(node, "[1, 2, 3]");
    MTEST_ASSERT(err == MERR_OK);
    MTEST_ASSERT(mdf_get_int_value(node, "[0]", 0) == 1);
    str = mdf_get_value_stringfy(node, "[2]", NULL);
    MTEST_ASSERT_STR_EQ(str, "3");
    mos_free(str);
    str = mdf_json_export_string(node);
    MTEST_ASSERT_STR_EQ("[1, 2, 3]", str);

    mos_free(str);
    mdf_destroy(&node);
    mdf_init(&node);

    err = mdf_json_import_string(node, "[[1, 2], [2]]");
    MTEST_ASSERT(err == MERR_OK);
    MTEST_ASSERT(mdf_get_int_value(node, "[0][1]", 0) == 2);

    mdf_destroy(&node);
    mdf_init(&node);

    err = mdf_json_import_string(node, "{'a': 100, 'b': 100.2, c: 'value \"c\"'}");
    MTEST_ASSERT(err == MERR_OK);
    MTEST_ASSERT(mdf_get_int_value(node, "a", 0) == 100);
    MTEST_ASSERT_STR_EQ(mdf_get_value(node, "c", NULL), "value \"c\"");
    str = mdf_json_export_string(node);
    MTEST_ASSERT_STR_EQ("{\"a\": 100, \"b\": 100.20000, \"c\": \"value \"c\"\"}", str);

    mos_free(str);
    mdf_destroy(&node);
    mdf_init(&node);

    err = mdf_json_import_string(node,
                                 "{a: [1, True, 3], b: {ba: {baa: 1}, bb: [1, '2', 3]}}");
    MTEST_ASSERT(err == MERR_OK);
    MTEST_ASSERT(mdf_get_int_value(node, "a[2]", 0) == 3);
    MTEST_ASSERT(mdf_get_int_value(node, "b.ba.baa", 0) == 1);
    MTEST_ASSERT_STR_EQ(mdf_get_value(node, "b.bb[1]", NULL), "2");
    str = mdf_json_export_string(node);
    MTEST_ASSERT_STR_EQ("{\"a\": [1, true, 3], \"b\": {\"ba\": {\"baa\": 1}, \"bb\": [1, \"2\", 3]}}", str);
    mos_free(str);

    str = mdf_json_export_string(mdf_get_node(node, "b.ba"));
    MTEST_ASSERT_STR_EQ("{\"baa\": 1}", str);
    mos_free(str);

    mdf_destroy(&node);
}

#define NODE_NUM 10000

void test_string_long()
{
    char **pstr, str[101], *s;
    MDF *node, *nodeb;
    MERR *err;

    pstr = mos_calloc(NODE_NUM, sizeof(char*));
    for (int i = 0; i < NODE_NUM; i++) {
        //mstr_rand_string(str, 100);
        snprintf(str, sizeof(str), "%d", i);

        pstr[i] = strdup(str);
    }

    mdf_init(&node);
    for (int i = 0; i < NODE_NUM; i++) {
        mdf_set_value(node, pstr[i], pstr[i]);
    }
    s = mdf_json_export_string(node);
    MTEST_ASSERT(s != NULL);

    mdf_init(&nodeb);
    err = mdf_json_import_string(nodeb, s);
    MTEST_ASSERT(err == MERR_OK);
    mos_free(s);

    MTEST_ASSERT(mdf_child_count(node, NULL) == mdf_child_count(nodeb, NULL));
    for (int i = 0; i < NODE_NUM; i++) {
        MTEST_ASSERT_STR_EQ(mdf_get_value(node, pstr[i], NULL),
                            mdf_get_value(nodeb, pstr[i], NULL));
    }

    for (int i = 0; i < NODE_NUM; i++) mos_free(pstr[i]);
    mos_free(pstr);

    mdf_destroy(&node);
    mdf_destroy(&nodeb);
}

void suite_string()
{
    mtest_add_test(test_string_short, "short json string");
    mtest_add_test(test_string_long, "long json string");
}

void test_file()
{
    MDF *node, *nodeb;
    char *stra, *strb;
    MERR *err;

    mdf_init(&node);
    err = mdf_json_import_file(node, "data/mdf.json");
    MTEST_ASSERT(err == MERR_OK);

    err = mdf_json_export_file(node, "data/mdf.json.export");
    MTEST_ASSERT(err == MERR_OK);

    mdf_init(&nodeb);
    err = mdf_json_import_file(nodeb, "data/mdf.json.export");
    MTEST_ASSERT(err == MERR_OK);

    MTEST_ASSERT(mdf_child_count(node, NULL) == mdf_child_count(nodeb, NULL));

    stra = mdf_json_export_string(node);
    strb = mdf_json_export_string(nodeb);
    MTEST_ASSERT_STR_EQ(stra, strb);

    mos_free(stra);
    mos_free(strb);

    mdf_destroy(&node);
    mdf_destroy(&nodeb);
}

void suite_file()
{
    mtest_add_test(test_file, "json file");
}

int main()
{
    mtc_init("-", MTC_DEBUG);

    mtest_add_suite(suite_string, "string");
    mtest_add_suite(suite_file, "file");

    return mtest_run();
}
