#include "reef.h"

void test_set()
{
    MDF *node;
    MERR *err;

    err = mdf_init(&node);
    MTEST_ASSERT(err == MERR_OK);

    err = mdf_set_value(node, "a", "va");
    MTEST_ASSERT(err == MERR_OK);
    MTEST_ASSERT_STR_EQ("va", mdf_get_value(node, "a", NULL));

    err = mdf_set_value(node, "a", "va");
    MTEST_ASSERT(err == MERR_OK);
    MTEST_ASSERT_STR_EQ("va", mdf_get_value(node, "a", NULL));

    err = mdf_set_value(node, "aa", "vaa");
    MTEST_ASSERT(err == MERR_OK);
    MTEST_ASSERT_STR_EQ("vaa", mdf_get_value(node, "aa", NULL));

    err = mdf_set_value(node, "aa.bb", "vaa.bb");
    MTEST_ASSERT(err == MERR_OK);
    MTEST_ASSERT(mdf_get_value(node, "aa", NULL) == NULL);
    MTEST_ASSERT_STR_EQ("vaa.bb", mdf_get_value(node, "aa.bb", NULL));

    err = mdf_set_value(node, "aa.bb.cc", "vaa.bb.cc");
    MTEST_ASSERT(err == MERR_OK);
    MTEST_ASSERT_STR_EQ("vaa.bb.cc", mdf_get_value(node, "aa.bb.cc", NULL));

    err = mdf_set_value(node, "bb", NULL);
    MTEST_ASSERT(err == MERR_OK);
    MTEST_ASSERT(mdf_get_node(node, "bb") != NULL);
    MTEST_ASSERT(mdf_get_value(node, "bb", NULL) == NULL);

    MTEST_ASSERT(mdf_get_node(node, "unexist") == NULL);
    MTEST_ASSERT(mdf_get_value(node, "unexist", NULL) == NULL);

    err = mdf_set_value(node, "[0]", "indexa");
    MTEST_ASSERT(err == MERR_OK);
    MTEST_ASSERT_STR_EQ("indexa", mdf_get_value(node, "[0]", NULL));
    MTEST_ASSERT_STR_EQ("indexa", mdf_get_value(node, "a", NULL));

    err = mdf_set_value(node, "aa[0]", "index1");
    MTEST_ASSERT(err == MERR_OK);
    MTEST_ASSERT_STR_EQ("index1", mdf_get_value(node, "aa.bb", NULL));

    err = mdf_set_value(node, "aa[0].cc", "index2");
    MTEST_ASSERT(err == MERR_OK);
    MTEST_ASSERT_STR_EQ("index2", mdf_get_value(node, "aa.bb.cc", NULL));

    err = mdf_set_value(node, "aa[2].cc", "index3");
    MTEST_ASSERT(err == MERR_OK);
    MTEST_ASSERT_STR_EQ("index3", mdf_get_value(node, "aa[2].cc", NULL));
    err = mdf_set_value(node, "aa[-1].cc", "index4");
    MTEST_ASSERT(err == MERR_OK);
    MTEST_ASSERT_STR_EQ("index4", mdf_get_value(node, "aa[2].cc", NULL));
    err = mdf_set_value(node, "aa.bb[-1]", "index5");
    MTEST_ASSERT(err == MERR_OK);
    MTEST_ASSERT_STR_EQ("index5", mdf_get_value(node, "aa.bb.cc", NULL));

    MTEST_ASSERT(mdf_get_node(node, "aa[1]") != NULL);
    MTEST_ASSERT(mdf_get_value(node, "aa[1]", NULL) == NULL);

    mdf_destroy(&node);
}

void test_other_type()
{
    MDF *node;
    MERR *err;
    char buf[204800];
    FILE *fp;
    size_t len;

    fp = fopen("1.jpg", "r");
    MTEST_ASSERT(fp != NULL);
    len = fread(buf, 1, 204800, fp);
    fclose(fp);

    mdf_init(&node);

    err = mdf_set_int_value(node, "namea", 100);
    MTEST_ASSERT(err == MERR_OK);
    err = mdf_set_int64_value(node, "nameb", 50000000000);
    MTEST_ASSERT(err == MERR_OK);
    err = mdf_set_float_value(node, "namec", 30000.003);
    MTEST_ASSERT(err == MERR_OK);

    err = mdf_set_bool_value(node, "named", false);
    MTEST_ASSERT(err == MERR_OK);
    err = mdf_set_binary(node, "namee", (unsigned char*)buf, len);
    MTEST_ASSERT(err == MERR_OK);


    MTEST_ASSERT(mdf_get_int_value(node, "namea", 0) == 100);
    MTEST_ASSERT(mdf_get_int64_value(node, "nameb", 0) == 50000000000);
    MTEST_ASSERT(mdf_get_float_value(node, "namec", 0) > 30000.003 - 1e-3);
    MTEST_ASSERT(mdf_get_float_value(node, "namec", 0) < 30000.003 + 1e-3);
    MTEST_ASSERT(mdf_get_bool_value(node, "named", true) == false);
    size_t len_mem = 0;
    unsigned char *mem = mdf_get_binary(node, "namee", &len_mem);
    MTEST_ASSERT(len == len_mem);

    char *strval;

    strval = mdf_get_value_stringfy(node, "namea", NULL);
    MTEST_ASSERT_STR_EQ("100", strval);
    mos_free(strval);

    strval = mdf_get_value_stringfy(node, "nameb", NULL);
    MTEST_ASSERT_STR_EQ("50000000000", strval);
    mos_free(strval);

    strval = mdf_get_value_stringfy(node, "namec", NULL);
    MTEST_ASSERT(strstr(strval, "30000") != NULL);
    mos_free(strval);

    strval = mdf_get_value_stringfy(node, "named", NULL);
    MTEST_ASSERT_STR_EQ("0", strval);
    mos_free(strval);


    fp = fopen("2.jpg", "w");
    MTEST_ASSERT(fp != NULL);
    len = fwrite(mem, 1, len_mem, fp);
    fclose(fp);

    MTEST_ASSERT(mdf_path_exist(node, "nameb") == true);
    MTEST_ASSERT(mdf_path_exist(node, "unexist") == false);

    MTEST_ASSERT(mdf_get_or_create_node(node, "unexist") != NULL);

    MTEST_ASSERT(mdf_node_child_count(node, NULL) == 6);

    MDF *bnode = mdf_get_node(node, "namea");

    MTEST_ASSERT_STR_EQ("namea", mdf_node_name(bnode));
    MTEST_ASSERT(mdf_node_value(bnode) == NULL);

    mdf_destroy(&node);
}

#define NODE_NUM 10000000

void test_huge_nodes()
{
    char **pstr, str[101];
    MDF *node, *nodeb;

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

    for (int i = 0; i < NODE_NUM; i++) {
        MTEST_ASSERT_STR_EQ(mdf_get_value(node, pstr[i], ""), pstr[i]);
    }

    /* 横向拷贝及删除 */
    mdf_init(&nodeb);
    mdf_copy(nodeb, NULL, node);
    MTEST_ASSERT(mdf_node_child_count(nodeb, NULL) == mdf_node_child_count(node, NULL));

    for (int i = 0; i < NODE_NUM; i++) {
        MTEST_ASSERT_STR_EQ(mdf_get_value(node, pstr[i], ""),
                            mdf_get_value(nodeb, pstr[i], ""));
    }

    for (int i = 0; i < NODE_NUM; i++) {
        mdf_remove(nodeb, pstr[i]);
        MTEST_ASSERT(mdf_get_value(nodeb, pstr[i], NULL) == NULL);
    }

    mdf_destroy(&node);
    mdf_destroy(&nodeb);

    mdf_init(&node);
    mdf_init(&nodeb);

    MDF *cnode;
    cnode = node;
    /* 深度拷贝及删除 */
    for (int i = 0; i < 100; i++) {
        mdf_set_value(cnode, pstr[i], pstr[i]);
        cnode = mdf_get_node(cnode, pstr[i]);
    }

    mdf_copy(nodeb, "copy_key", node);

    cnode = node;
    MDF *cnodeb = mdf_get_node(nodeb, "copy_key");
    for (int i = 0; i < 100; i++) {
        cnode = mdf_node_child(cnode);
        cnodeb = mdf_node_child(cnodeb);
    }

    MTEST_ASSERT_STR_EQ(mdf_node_value(cnode), mdf_node_value(cnodeb));

    for (int i = 0; i < NODE_NUM; i++) mos_free(pstr[i]);
    mos_free(pstr);

    mdf_destroy(&node);
    mdf_destroy(&nodeb);
}

void suite_basic()
{
    mtest_add_test(test_set, "basic set and get");
    mtest_add_test(test_other_type, "other types set and get");
    mtest_add_test(test_huge_nodes, "huge nodes");
}

void test_change()
{
    MDF *node;
    MERR *err;

    mdf_init(&node);

    mdf_set_int_value(node, "a", 100);
    err = mdf_set_type(node, "a", MDF_TYPE_STRING);
    MTEST_ASSERT(err != MERR_OK);
    //TRACE_NOK(err);

    mdf_set_value(node, "a", "100");
    MTEST_ASSERT_STR_EQ(mdf_get_value(node, "a", NULL), "100");
    mdf_set_type(node, "a", MDF_TYPE_INT);
    MTEST_ASSERT(mdf_get_value(node, "a", NULL) == NULL);
    MTEST_ASSERT(mdf_get_int_value(node, "a", 0) == 100);

    mdf_set_value(node, "a", "100.3");
    mdf_set_type(node, "a", MDF_TYPE_FLOAT);
    MTEST_ASSERT(mdf_get_float_value(node, "a", 0) > 100);

    /* string node exist, true after bool type change */
    mdf_set_value(node, "a", "0");
    mdf_set_type(node, "a", MDF_TYPE_BOOL);
    MTEST_ASSERT(mdf_get_bool_value(node, "a", false) == true);

    mdf_destroy(&node);
}

void suite_change()
{
    mtest_add_test(test_change, "misc change");
}

int main()
{
    mtc_init("-", MTC_DEBUG);

    mtest_add_suite(suite_basic, "basic");
    mtest_add_suite(suite_change, "change node");

    return mtest_run();
}
