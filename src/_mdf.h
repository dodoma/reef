#ifndef ___MDF_H__
#define ___MDF_H__

struct _MDF {
    uint32_t namelen;
    uint32_t valuelen;
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

    A_COMMENT_A, A_COMMENT_B, A_UNCOMMENT_B,
    A_UTF8_2, A_UTF8_3, A_UTF8_4, A_UTF_CONTINUE
} JSON_ACTION;

typedef enum {
    F_POSITIVE_FIX_INT = 0,
    F_NEGATIVE_FIX_INT,
    F_INT_8,
    F_INT_16,
    F_INT_32,
    F_INT_64,
    F_UINT_8,
    F_UINT_16,
    F_UINT_32,
    F_UINT_64,
    F_NIL,
    F_FALSE,
    F_TRUE,
    F_FLOAT32,
    F_FLOAT64,
    F_FIX_STR,
    F_STR_8,
    F_STR_16,
    F_STR_32,
    F_BIN_8,
    F_BIN_16,
    F_BIN_32,
    F_FIX_ARRAY,
    F_ARRAY_16,
    F_ARRAY_32,
    F_FIX_MAP,
    F_MAP_16,
    F_MAP_32,
    F_FIX_EXT_1,
    F_FIX_EXT_2,
    F_FIX_EXT_4,
    F_FIX_EXT_8,
    F_FIX_EXT_16,
    F_EXT_8,
    F_EXT_16,
    F_EXT_32,
    F_UNKNOWN
} MPACK_FORMAT;

void _mdf_drop_child_node(MDF *pnode, MDF *cnode);
void _mdf_append_child_node(MDF *pnode, MDF *newnode, int current_childnum);

#endif
