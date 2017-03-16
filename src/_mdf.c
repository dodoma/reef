#include "reef.h"
#include "_mdf.h"

#define _FORCE_HASH_AT 10

void _mdf_drop_child_node(MDF *pnode, MDF *cnode)
{
    if (cnode->parent != pnode) return;

    if (cnode->prev == NULL) {
        pnode->child = cnode->next;
        if (pnode->child) pnode->child->prev = NULL;
    } else {
        cnode->prev->next = cnode->next;
        if (cnode->next) cnode->next->prev = cnode->prev;
    }

    if (cnode->next == NULL) {
        pnode->last_child = NULL;
    }

    mhash_remove(pnode->table, cnode->name);

    cnode->next = NULL;
    mdf_destroy(&cnode);
}

void _mdf_append_child_node(MDF *pnode, MDF *newnode, int current_childnum)
{
    if (current_childnum < 0) {
        if (pnode->table) current_childnum = mhash_length(pnode->table);
        else {
            current_childnum = 0;
            for (MDF *cnode = pnode->child; cnode; cnode = cnode->next)
                current_childnum++;
        }
    }

    if (current_childnum >= _FORCE_HASH_AT && pnode->table == NULL) {
        /* new hash table */
        mhash_init(&pnode->table, mhash_str_hash, mhash_str_comp, NULL);

        MDF *cnode = pnode->child;
        while (cnode) {
            mhash_insert(pnode->table, cnode->name, cnode);

            cnode = cnode->next;
        }
    }

    if (pnode->table) {
        /* insert to table */
        mhash_insert(pnode->table, newnode->name, newnode);
    }

    /* append to list */
    newnode->parent = pnode;
    newnode->prev = pnode->last_child;

    if (!pnode->child) pnode->child = newnode;
    if (pnode->last_child) pnode->last_child->next = newnode;

    pnode->last_child = newnode;
}
