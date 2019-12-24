#include "reef.h"

struct _MUTREE {
    uint64_t key;
    void *value;
    MUTREE *left, *right;
    int level;
};

static MUTREE sentinel = {0, NULL, &sentinel, &sentinel, 0};

static MUTREE* _make(uint64_t key, void *value)
{
    MUTREE *node = mos_calloc(1, sizeof(MUTREE));
    node->key = key;
    node->value = value;
    node->left = node->right = &sentinel;
    node->level = 1;

    return node;
}

static MUTREE* _skew(MUTREE *node)
{
    if (node->left->level == node->level) {
        MUTREE *save = node;
        node = node->left;
        save->left = node->right;
        node->right = save;
    }

    return node;
}

static MUTREE* _split(MUTREE *node)
{
    if (node->right->right->level == node->level) {
        MUTREE *save = node;
        node = node->right;
        save->right = node->left;
        node->left = save;
        node->level++;
    }

    return node;
}

MUTREE* mutree_insert(MUTREE *node, uint64_t key, void *value)
{
    if (node && node != &sentinel) {
        if (key < node->key) node->left = mutree_insert(node->left, key, value);
        else node->right = mutree_insert(node->right, key, value);

        node = _skew(node);
        node = _split(node);

        return node;
    }

    return _make(key, value);
}

void* mutree_lookup(MUTREE *node, uint64_t key)
{
    if (node) {
        while (node != &sentinel) {
            if (key == node->key) return node->value;
            else if (key < node->key) node = node->left;
            else node = node->right;
        }
    }

    return NULL;
}

void mutree_free(MUTREE *node)
{
    if (node && node != &sentinel) {
        mutree_free(node->left);
        mutree_free(node->right);

        mos_free(node);
    }
}

void mutree_print(MUTREE *node, int level)
{
	if (node->left != &sentinel) mutree_print(node->left, level + 1);

	for (int i = 0; i < level; ++i) printf("    ");

	printf(FMT_UINT64" (%d)\n", node->key, node->level);

	if (node->right != &sentinel) mutree_print(node->right, level + 1);
}
