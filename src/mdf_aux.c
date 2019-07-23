#include "reef.h"
#include "_mdf.h"

void mdf_aux_list2map(MDF *node, const char *path)
{
    if (!node) return;

    char *k, *v;
    k = v = NULL;

    MDF *tmpnode = mdf_get_or_create_node(node, "__list2map__");

    MDF *cnode = mdf_get_child(node, path);
    while (cnode) {
        if (!k) k = mdf_get_value(cnode, NULL, "k");
        else v = mdf_get_value(cnode, NULL, "v");

        if (v) {
            mdf_set_value(tmpnode, k, v);
            k = v = NULL;
        }

        cnode = mdf_node_next(cnode);
    }

    cnode = mdf_get_node(node, path);
    mdf_clear(cnode);
    mdf_copy(node, path, tmpnode, true);
    mdf_remove_me(tmpnode);
}
