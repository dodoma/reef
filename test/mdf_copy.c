#include "reef.h"

void test_copy()
{
    MDF *anode, *bnode, *cnode, *xnode;

    mdf_init(&anode);
    mdf_init(&bnode);
    mdf_init(&cnode);
    mdf_init(&xnode);

    mdf_json_import_file(anode, "a.json");
    mdf_json_import_file(bnode, "b.json");
    mdf_json_import_file(cnode, "c.json");

    mdf_copy(xnode, "aaa", anode, true);
    mdf_copy(xnode, "bbb", bnode, true);
    MTEST_ASSERT(mdf_equal(cnode, xnode) == true);

    MDF *ynode;
    mdf_init(&ynode);
    mdf_copy(ynode, NULL, anode, true);
    MDF *znode = mdf_node_child(bnode);
    while (znode) {
        mdf_copy(anode, mdf_get_name(znode, NULL), znode, true);

        znode = mdf_node_next(znode);
    }
    znode = mdf_node_child(ynode);
    while (znode) {
        mdf_copy(bnode, mdf_get_name(znode, NULL), znode, true);
        znode = mdf_node_next(znode);
    }
    MTEST_ASSERT(mdf_equal(anode, bnode) == true);
    mdf_destroy(&ynode);

    mdf_set_value(anode, "5ACC9E03.id", "xxxxxx");
    mdf_set_int_value(anode, "5ACC9E03.price", 1000);
    MTEST_ASSERT(mdf_equal(anode, bnode) == false);

    mdf_copy(bnode, "5ACC9E03", mdf_get_node(anode, "5ACC9E03"), false);
    MTEST_ASSERT(mdf_equal(anode, bnode) == false);

    mdf_copy(bnode, "5ACC9E03", mdf_get_node(anode, "5ACC9E03"), true);
    MTEST_ASSERT(mdf_equal(anode, bnode) == true);

    //mdf_json_export_file(anode, "-");
    //mdf_json_export_file(bnode, "-");

    mdf_destroy(&anode);
    mdf_destroy(&bnode);
    mdf_destroy(&cnode);
    mdf_destroy(&xnode);
}

void suite_basic()
{
    mtest_add_test(test_copy, "test mdf copy");
}

int main()
{
    mtest_add_suite(suite_basic, "basic");

    return mtest_run();
}
