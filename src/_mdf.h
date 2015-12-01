struct _MDF {
    char *name;
    MDF_TYPE type;

    union {
        int64_t n;
        float f;
        char *s;
    } val;

    MHASH *table;

    MDF *prev;
    MDF *next;

    MDF *parent;
    MDF *child;
    MDF *last_child;
};

typedef enum {
    R_UNKNOWN  = 0,
    R_OBJECT,                    /* under object construct */
    R_ARRAY                      /* under array construct */
} JSON_ROLE;

typedef enum {
    A_BAD, A_LOOP, A_NEWLINE,
    A_BARE, A_NUMBARE, A_UNBARE,
    /* " ' " ' */
    A_QUOTE, A_UNQUOTE,
    /* \"\/bfnrtu */
    A_ESCAPE, A_UNESCAPE,
    /* { } [ ] , : */
    A_OBJ, A_UNOBJ, A_ARRAY, A_UNARRAY, A_PAIR, A_VALUE,

    A_UTF8_2, A_UTF8_3, A_UTF8_4, A_UTF_CONTINUE
} JSON_ACTION;

void _mdf_drop_child_node(MDF *pnode, MDF *cnode);
void _mdf_append_child_node(MDF *pnode, MDF *newnode, int current_childnum);
