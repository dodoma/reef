#include "reef.h"

struct data_source {
    char *name;
    char *path;
};

struct newtype_t {
    int id;
    char *name;
    char *path;
};

static void _newtype_free(void *data)
{
    struct newtype_t *node = (struct newtype_t*)data;
    free(node->path);
}

static int _newtype_compare(void *data, void *key)
{
    return strcmp(((struct newtype_t*)data)->name, (char*)key);
}

static bool _newtype_dump(void *data)
{
    struct newtype_t *node = (struct newtype_t*)data;

    printf("%d\t%s\t\t\t\t%s\n", node->id, node->name, node->path);

    return true;
}

void test_basic()
{
    MSLIST *seeds = NULL;

    struct data_source sources[] = {
        {"ddd", "张学友/忘记你我做不到/"},
        {"ddd", "张学友/"},
        {"ddd", ""},
        {"国语精选", "张学友/忘记你我做不到/"},
        {"国语精选", "张学友/"},
        {"国语精选", "木马/木马/"},
        {"国语精选", "木马/"},
        {"国语精选", ""},
        {"默认媒体库", "2024-11-25 10:27:06/2024-11-25 10:44:12/"},
        {"默认媒体库", "2024-11-25 10:27:06/"},
        {"默认媒体库", "2024-11-25 10:44:12/张学友/忘记你我做不到/"},
        {"默认媒体库", "2024-11-25 10:44:12/张学友/"},
        {"默认媒体库", "2024-11-25 10:44:12/"},
        {"默认媒体库", "0701/"},
        {"默认媒体库", ""},
        {"ddd", "木马/木马/"},
        {"ddd", "木马/"},
        {"ddd", ""},
        {NULL, NULL}
    };

    for (int i = 0; sources[i].name != NULL; i++) {
        struct newtype_t *node = mos_calloc(1, sizeof(struct newtype_t));
        node->id = i+1;
        node->name = sources[i].name;
        node->path = strdup(sources[i].path);

        seeds = mslist_add(seeds, node);
    }

    mslist_iterate(seeds, _newtype_dump);

    //seeds = mslist_remove(seeds, "ddd", _newtype_compare, _newtype_free);
    seeds = mslist_remove(seeds, "默认媒体库", _newtype_compare, _newtype_free);

    MTEST_ASSERT(mslist_length(seeds) == 11);
    MTEST_ASSERT(mslist_search(seeds, "默认媒体库", _newtype_compare) == NULL);
    MTEST_ASSERT(mslist_search(seeds, "ddd", _newtype_compare) != NULL);

    printf("============ remove ddd ============= %d\n", mslist_length(seeds));
    mslist_iterate(seeds, _newtype_dump);

    mslist_free(seeds, _newtype_free);
}

int main()
{
    mtest_add_suite(test_basic, "basic");
    //mtest_add_suite(suite_search, "search");

    return mtest_run();
}
