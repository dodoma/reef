#include "reef.h"

static int _compare_key(const void *a, const void *b)
{
    MDF *anode = *(MDF**)a;
    MDF *bnode = *(MDF**)b;

    return strcmp(mdf_get_name(anode, NULL), mdf_get_name(bnode, NULL));
}

static int _compare(const void *a, const void *b)
{
    MDF *anode = *(MDF**)a;
    MDF *bnode = *(MDF**)b;

    return mdf_get_int_value(anode, "id", 0) - mdf_get_int_value(bnode, "id", 0);
}

void test_basic()
{
    MERR *err;
    MDF *anode, *bnode, *cnode;

    mdf_init(&anode);
    mdf_init(&bnode);
    mdf_init(&cnode);

    err = mdf_json_import_file(anode, "data/sort-1.json");
    TRACE_NOK(err);
    mdf_sort_node(anode, _compare);

    mdf_clear(anode);
    mdf_json_import_file(anode, "data/sort-2.json");
    mdf_sort_node(anode, _compare);
    MDF *xnode, *ynode;
    xnode = mdf_node_child(anode);
    ynode = mdf_node_next(xnode);

    MTEST_ASSERT(mdf_get_int_value(xnode, "id", 1) <= mdf_get_int_value(ynode, "id", 0));

    mdf_clear(anode);
    mdf_json_import_file(anode, "data/sort-3.json");
    mdf_sort_node(anode, _compare);
    xnode = mdf_node_child(anode);
    ynode = mdf_node_next(xnode);
    while (xnode && ynode) {
        MTEST_ASSERT(mdf_get_int_value(xnode, "id", 1) <= mdf_get_int_value(ynode, "id", 0));

        xnode = mdf_node_next(xnode);
        ynode = mdf_node_next(xnode);
    }


    mdf_clear(anode);
    mdf_json_import_file(anode, "data/sort-n.json");
    mdf_sort_node(anode, _compare);
    xnode = mdf_node_child(anode);
    ynode = mdf_node_next(xnode);
    while (xnode && ynode) {
        MTEST_ASSERT(mdf_get_int_value(xnode, "id", 1) <= mdf_get_int_value(ynode, "id", 0));

        xnode = mdf_node_next(xnode);
        ynode = mdf_node_next(xnode);
    }

    mdf_sort_node(anode, _compare_key);
    xnode = mdf_node_child(anode);
    ynode = mdf_node_next(xnode);
    while (xnode && ynode) {
        char *xname = mdf_get_name(xnode, NULL);
        char *yname = mdf_get_name(ynode, NULL);

        MTEST_ASSERT(*xname <= *yname);

        xnode = mdf_node_next(xnode);
        ynode = mdf_node_next(xnode);
    }


    mdf_destroy(&anode);
    mdf_destroy(&bnode);
    mdf_destroy(&cnode);
}

void suite_basic()
{
    mtest_add_test(test_basic, "basic sort");
}

int main()
{
    mtc_init("-", MTC_DEBUG);

    mtest_add_suite(suite_basic, "basic");

    return mtest_run();
}
