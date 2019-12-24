#include "reef.h"

struct _MTREE {
    char *key;
    void *value;
    MTREE *left, *right;
    int level;
};

static MTREE sentinel = {"", NULL, &sentinel, &sentinel, 0};

static MTREE* _make(const char *key, void *value)
{
    MTREE *node = mos_calloc(1, sizeof(MTREE));
    node->key = strdup(key);
    node->value = value;
    node->left = node->right = &sentinel;
    node->level = 1;

    return node;
}

static MTREE* _skew(MTREE *node)
{
    if (node->left->level == node->level) {
        MTREE *save = node;
        node = node->left;
        save->left = node->right;
        node->right = save;
    }

    return node;
}

static MTREE* _split(MTREE *node)
{
    if (node->right->right->level == node->level) {
        MTREE *save = node;
        node = node->right;
        save->right = node->left;
        node->left = save;
        node->level++;
    }

    return node;
}

MTREE* mtree_insert(MTREE *node, const char *key, void *value)
{
    if (node && node != &sentinel) {
        int c = strcmp(key, node->key);
        if (c < 0) node->left = mtree_insert(node->left, key, value);
        else node->right = mtree_insert(node->right, key, value);

        node = _skew(node);
        node = _split(node);

        return node;
    }

    return _make(key, value);
}

void* mtree_lookup(MTREE *node, const char *key)
{
    if (node) {
        while (node != &sentinel) {
            int c = strcmp(key, node->key);
            if (c == 0) return node->value;
            else if (c < 0) node = node->left;
            else node = node->right;
        }
    }

    return NULL;
}

void mtree_free(MTREE *node)
{
    if (node && node != &sentinel) {
        mtree_free(node->left);
        mtree_free(node->right);

        mos_free(node->key);
        mos_free(node);
    }
}

void mtree_print(MTREE *node, int level)
{
	if (node->left != &sentinel) mtree_print(node->left, level + 1);

	for (int i = 0; i < level; ++i) printf("    ");

	printf("%s (%d)\n", node->key, node->level);

	if (node->right != &sentinel) mtree_print(node->right, level + 1);
}
