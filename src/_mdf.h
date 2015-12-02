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
    A_BAD = 0,
    A_LOOP, A_NEWLINE,


    A_OBJECT,
    A_PAIR_L_RAW, A_PAIR_L_QUOTE,
    A_UNPAIR_L_RAW_NEWLINE, A_UNPAIR_L_RAW, A_UNPAIR_L_QUOTE, A_UNPAIR_L_PAIR,
    A_PAIR,
    A_PAIR_R_RAW, A_PAIR_R_QUOTE, A_PAIR_R_NUMBER, A_PAIR_R_FLOAT,
    A_PAIR_R_OBJECT, A_PAIR_R_ARRAY,
    A_UNPAIR_R_RAW_NEWLINE, A_UNPAIR_R_RAW, A_UNPAIR_R_RAW_BRACE, A_UNPAIR_R_RAW_COMMA,
    A_UNPAIR_R_QUOTE,
    A_UNPAIR_R_NUMBER_NEWLINE, A_UNPAIR_R_NUMBER,
    A_UNPAIR_R_NUMBER_BRACE, A_UNPAIR_R_NUMBER_COMMA,
    A_PAIR_NEXT,
    A_UNOBJECT,


    A_ARRAY,
    A_VALUE_RAW, A_VALUE_QUOTE, A_VALUE_NUMBER, A_VALUE_FLOAT,
    A_VALUE_OBJECT, A_VALUE_ARRAY,
    A_UNVALUE_RAW_NEWLINE, A_UNVALUE_RAW, A_UNVALUE_RAW_SQUARE, A_UNVALUE_RAW_COMMA,
    A_UNVALUE_QUOTE,
    A_UNVALUE_NUMBER_NEWLINE, A_UNVALUE_NUMBER,
    A_UNVALUE_NUMBER_SQUARE, A_UNVALUE_NUMBER_COMMA,
    A_VALUE_NEXT,
    A_UNARRAY,

    A_UTF8_2, A_UTF8_3, A_UTF8_4, A_UTF_CONTINUE
} JSON_ACTION;

void _mdf_drop_child_node(MDF *pnode, MDF *cnode);
void _mdf_append_child_node(MDF *pnode, MDF *newnode, int current_childnum);
