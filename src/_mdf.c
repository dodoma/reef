#include "reef.h"
#include "_mdf.h"

#define _FORCE_HASH_AT 10

void _mdf_drop_child_node(MDF *pnode, MDF *cnode)
{
    if (cnode->parent != pnode) return;

    MDF *prev = cnode->prev, *next = cnode->next;

    if (prev == NULL) {
        pnode->child = next;

        if (next) next->prev = NULL;
        else pnode->last_child = NULL;
    } else {
        prev->next = next;

        if (next) next->prev = prev;
        else pnode->last_child = prev;
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

void _mdf_insert_child_node(MDF *pnode, MDF *newnode, int current_childnum, int position)
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

    /*
     * insert to list
     * 节点编号为 0, 1, 2, 3...
     * position 为插入位置，如：2 为查入到第三位，即原来索引为2的元素之前
     */
    newnode->parent = pnode;
    newnode->prev = newnode->next = NULL;

    if (position < 0 || position > current_childnum) position = current_childnum;

    int ahead_count = 0;
    MDF *nextnode = pnode->child;
    while (nextnode && ahead_count < position) {
        ahead_count++;
        nextnode = nextnode->next;
    }

    if (ahead_count == current_childnum) {
        /* 插到最后一个 */
        newnode->prev = pnode->last_child;

        if (!pnode->child) pnode->child = newnode;
        if (pnode->last_child) pnode->last_child->next = newnode;

        pnode->last_child = newnode;
    } else if (nextnode) {
        newnode->prev = nextnode->prev;
        newnode->next = nextnode;

        if (nextnode->prev == NULL) {
            pnode->child = newnode;
        } else {
            nextnode->prev->next = newnode;
        }

        nextnode->prev = newnode;
    } else {
        /* TODO unpossible */
        fprintf(stderr, "unbelieve, %d %d %d\n", current_childnum, position, ahead_count);
    }
}
