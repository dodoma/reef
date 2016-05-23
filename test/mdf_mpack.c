#include "reef.h"

void test_short()
{
    MDF *node, *nodeb;
    unsigned char buf[10240];
    char *stra, *strb;
    size_t lena, lenb;
    MERR *err;

    mdf_init(&node);
    err = mdf_json_import_string(node, "{bdofe: {'c': null},'f': 'z'}");
    MTEST_ASSERT(err == MERR_OK);

    lena = mdf_mpack_serialize(node, buf, 10240);
    MTEST_ASSERT(lena > 0);

    mdf_init(&nodeb);
    lenb = mdf_mpack_deserialize(nodeb, buf, lena);
    MTEST_ASSERT(lena == lenb);

    stra = mdf_json_export_string(node);
    strb = mdf_json_export_string(nodeb);
    MTEST_ASSERT_STR_EQ(stra, strb);

    mos_free(stra);
    mos_free(strb);

    MDF *nodea = mdf_get_node(node, "bdofe");
    lena = mdf_mpack_serialize(nodea, buf, 10240);
    MTEST_ASSERT(lena > 0);

    mdf_destroy(&nodeb);
    mdf_init(&nodeb);
    lenb = mdf_mpack_deserialize(nodeb, buf, lena);
    MTEST_ASSERT(lena == lenb);

    stra = mdf_json_export_string(nodea);
    strb = mdf_json_export_string(nodeb);
    MTEST_ASSERT_STR_EQ(stra, strb);

    mos_free(stra);
    mos_free(strb);

    mdf_destroy(&node);
    mdf_destroy(&nodeb);
}

void test_file()
{
    MDF *node, *nodeb;
    unsigned char buf[102400];
    char *stra, *strb;
    size_t lena, lenb;
    MERR *err;

    mdf_init(&node);
    err = mdf_json_import_file(node, "mdf.json");
    MTEST_ASSERT(err == MERR_OK);

    lena = mdf_mpack_serialize(node, buf, 102400);
    MTEST_ASSERT(lena > 0);

    mdf_init(&nodeb);
    lenb = mdf_mpack_deserialize(nodeb, buf, lena);
    MTEST_ASSERT(lena == lenb);

    stra = mdf_json_export_string(node);
    strb = mdf_json_export_string(nodeb);
    MTEST_ASSERT_STR_EQ(stra, strb);

    mos_free(stra);
    mos_free(strb);

    mdf_destroy(&node);
    mdf_destroy(&nodeb);
}

#define NODE_NUM 10000

void test_long()
{
    MDF *node, *nodeb;
    unsigned char buf[102400];
    char **pstr, str[101];
    char *stra, *strb;
    size_t lena, lenb;

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
    lena = mdf_mpack_serialize(node, buf, 102400);
    MTEST_ASSERT(lena > 0);

    mdf_init(&nodeb);
    lenb = mdf_mpack_deserialize(nodeb, buf, lena);
    MTEST_ASSERT(lena == lenb);

    stra = mdf_json_export_string(node);
    strb = mdf_json_export_string(nodeb);
    MTEST_ASSERT_STR_EQ(stra, strb);

    mos_free(stra);
    mos_free(strb);

    for (int i = 0; i < NODE_NUM; i++) mos_free(pstr[i]);
    mos_free(pstr);

    mdf_destroy(&node);
    mdf_destroy(&nodeb);
}

void suite_basic()
{
    mtest_add_test(test_short, "short message pack");
    mtest_add_test(test_file, "short file message pack");
    mtest_add_test(test_long, "long message pack");
}

int main()
{
    mtc_init("-", MTC_DEBUG);

    mtest_add_suite(suite_basic, "basic");

    return mtest_run();
}
