#include "reef.h"

int main()
{
    MDF *node;
    MERR *err;

    mtc_init("test.log", MTC_DEBUG);

    mdf_init(&node);

    err = mdf_set_value(node, "aa", "111");
    TRACE_NOK(err);
    err = mdf_set_value(node, "aa.bb.cc1", "c111");
    TRACE_NOK(err);
    err = mdf_set_value(node, "aa.bb.cc2", "c222");
    TRACE_NOK(err);
    err = mdf_set_value(node, "aa.bb.cc3", "c333");
    TRACE_NOK(err);

    printf("%s\n", mdf_get_value(node, "aa", ""));

    printf("%s\n", mdf_get_value(node, "aa.bb.cc3", ""));

    printf("%s\n", mdf_get_value(node, "[0]", ""));

    printf("%s\n", mdf_get_value(node, "aa[-1][1]", ""));

    printf("%s\n", mdf_get_value(node, "aa.bb[-1]", ""));

    printf("%s\n", mdf_get_value(node, "[0][-1].cc2", "ddd"));

    printf("%s\n", mdf_get_value(node, "[0][-1].[5]", "ddd"));

    mdf_set_value(node, "[0][-1].[5][5]", "xxxxx");
    MDF *xnode = mdf_get_node(node, "[0][-1].[5][5]");
    printf("%s: %s\n", mdf_node_name(xnode), mdf_node_value(xnode));

    printf("%s\n", mdf_get_value(node, "aa.bb.cc3", ""));
    mdf_remove(node, "aa.bb.cc3");
    printf("%s\n", mdf_get_value(node, "aa.bb.cc3", "xxxxxx"));

    MDF *newnode;
    mdf_init(&newnode);
    mdf_copy(newnode, NULL, node);
    mdf_destroy(&newnode);

#if 0
    mdf_get_value(node, "aa", NULL);
    printf("\n\n");
    mdf_get_value(node, "aa.bb.cc", NULL);
    printf("\n\n");
    mdf_get_value(node, "[0]", NULL);
    printf("\n\n");
    mdf_get_value(node, "[-10][3]", NULL);
    printf("\n\n");
    mdf_get_value(node, "[0].aa[3]", NULL);
    printf("\n\n");
    mdf_get_value(node, ".a[0]", NULL);
    printf("\n\n");
    mdf_get_value(node, "aa[0][2].bb.aa[0][2].bb.aa[0][2].bb", NULL);
    printf("\n\n");
    mdf_get_value(node, "aa[0][2]dd.[10].cc.bb", NULL);
    printf("\n\n");
#endif

    mdf_destroy(&node);

    return 0;
}
