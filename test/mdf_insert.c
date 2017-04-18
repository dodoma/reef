#include "reef.h"

void basic_assert(MDF *anode, int position, char *key)
{
    MDF *cnode, *znode;
    mdf_init(&cnode);
    mdf_set_value(cnode, "foo", "111");
    mdf_set_int_value(cnode, "bar", 123);


    MDF *bnode = mdf_insert_node(anode, NULL, position);
    mdf_copy(bnode, NULL, cnode, false);
    znode = mdf_get_node(anode, key);
    //mdf_json_export_file(anode, "-");
    //printf("\n\n");

    MTEST_ASSERT(mdf_equal(znode, cnode) == true);

    mdf_destroy(&cnode);
}

void test_basic()
{
    MERR *err;
    MDF *anode;

    mdf_init(&anode);

    /*
     * 已有0个元素的插入
     */
    basic_assert(anode, 0, "[0]");

    mdf_clear(anode);
    basic_assert(anode, -1, "[0]");

    mdf_clear(anode);
    basic_assert(anode, 10, "[0]");


    /*
     * 已有1个元素的插入
     */
    mdf_clear(anode);
    mdf_set_value(anode, "key_0", "value_0");
    basic_assert(anode, 0, "[0]");

    mdf_clear(anode);
    mdf_set_value(anode, "key_0", "value_0");
    basic_assert(anode, 1, "[1]");

    mdf_clear(anode);
    mdf_set_value(anode, "key_0", "value_0");
    basic_assert(anode, -1, "[1]");

    mdf_clear(anode);
    mdf_set_value(anode, "key_0", "value_0");
    basic_assert(anode, 10, "[1]");


    /*
     * 已有2个元素的插入
     */
    mdf_clear(anode);
    mdf_set_value(anode, "key_0", "value_0");
    mdf_set_value(anode, "key_1", "value_1");
    basic_assert(anode, 0, "[0]");

    mdf_clear(anode);
    mdf_set_value(anode, "key_0", "value_0");
    mdf_set_value(anode, "key_1", "value_1");
    basic_assert(anode, 1, "[1]");

    mdf_clear(anode);
    mdf_set_value(anode, "key_0", "value_0");
    mdf_set_value(anode, "key_1", "value_1");
    basic_assert(anode, 2, "[2]");

    mdf_clear(anode);
    mdf_set_value(anode, "key_0", "value_0");
    mdf_set_value(anode, "key_1", "value_1");
    basic_assert(anode, 3, "[2]");

    mdf_clear(anode);
    mdf_set_value(anode, "key_0", "value_0");
    mdf_set_value(anode, "key_1", "value_1");
    basic_assert(anode, -1, "[2]");


    /*
     * 已有3个元素的插入
     */
    mdf_clear(anode);
    mdf_set_value(anode, "key_0", "value_0");
    mdf_set_value(anode, "key_1", "value_1");
    mdf_set_value(anode, "key_2", "value_2");
    basic_assert(anode, 0, "[0]");

    mdf_clear(anode);
    mdf_set_value(anode, "key_0", "value_0");
    mdf_set_value(anode, "key_1", "value_1");
    mdf_set_value(anode, "key_2", "value_2");
    basic_assert(anode, 1, "[1]");

    mdf_clear(anode);
    mdf_set_value(anode, "key_0", "value_0");
    mdf_set_value(anode, "key_1", "value_1");
    mdf_set_value(anode, "key_2", "value_2");
    basic_assert(anode, 2, "[2]");

    mdf_clear(anode);
    mdf_set_value(anode, "key_0", "value_0");
    mdf_set_value(anode, "key_1", "value_1");
    mdf_set_value(anode, "key_2", "value_2");
    basic_assert(anode, 3, "[3]");

    mdf_clear(anode);
    mdf_set_value(anode, "key_0", "value_0");
    mdf_set_value(anode, "key_1", "value_1");
    mdf_set_value(anode, "key_2", "value_2");
    basic_assert(anode, 4, "[3]");

    mdf_clear(anode);
    mdf_set_value(anode, "key_0", "value_0");
    mdf_set_value(anode, "key_1", "value_1");
    mdf_set_value(anode, "key_2", "value_2");
    basic_assert(anode, -1, "[3]");

    mdf_destroy(&anode);
}

void test_more()
{
    MERR *err;
    MDF *anode;

    mdf_init(&anode);

    int num = 1000;

    /*
     * 已有 num 个元素的插入
     */
    for (int i = 0; i < num; i++) {
        mdf_clear(anode);
        for (int j = 0; j < num; j++) {
            mdf_set_valuef(anode, "key_%d=value_%d", j, j);
        }
        char tok[64];
        snprintf(tok, sizeof(tok), "[%d]", i);
        basic_assert(anode, i, tok);
    }

    mdf_clear(anode);
    for (int j = 0; j < num; j++) {
        mdf_set_valuef(anode, "key_%d=value_%d", j, j);
    }
    char tok[64];
    snprintf(tok, sizeof(tok), "[%d]", num);
    basic_assert(anode, num, tok);

    mdf_clear(anode);
    for (int j = 0; j < num; j++) {
        mdf_set_valuef(anode, "key_%d=value_%d", j, j);
    }
    snprintf(tok, sizeof(tok), "[%d]", num);
    basic_assert(anode, -1, tok);

    mdf_destroy(&anode);
}


void suite_basic()
{
    mtest_add_test(test_basic, "basic insert");
    mtest_add_test(test_more, "hash insert");
}

int main()
{
    mtc_init("-", MTC_DEBUG);

    mtest_add_suite(suite_basic, "basic");

    return mtest_run();
}
