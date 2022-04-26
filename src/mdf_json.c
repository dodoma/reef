#include "reef.h"
#include "_mdf.h"

/*
 * don't care MDF_TYPE_BINARY
 */

static int8_t go_plain[256] = {0};

static int8_t go_object[256] = {0};
static int8_t go_pair_l_raw[256] = {0};
static int8_t go_pair_l_quote[256] = {0};
static int8_t go_unpair_l[256] = {0};
static int8_t go_pair[256] = {0};
static int8_t go_pair_r_raw[256] = {0};
static int8_t go_pair_r_quote[256] = {0};
static int8_t go_pair_r_number[256] = {0};
static int8_t go_unpair_r[256] = {0};

static int8_t go_array[256] = {0};
static int8_t go_value_raw[256] = {0};
static int8_t go_value_quote[256] = {0};
static int8_t go_value_number[256] = {0};
static int8_t go_unvalue[256] = {0};

static int8_t go_data_quote[256] = {0};
static int8_t go_data_number[256] = {0};
static int8_t go_data_reserve[256] = {0};

static int8_t go_comment_a[256] = {0};
static int8_t go_comment_b[256] = {0};
static int8_t go_utf8_continue[256] = {0};

static void _json_machine_init()
{
    static bool inited = false;

#define GO_WHITESPACE_LOOP(go)                  \
    do {                                        \
        go['\t'] = A_LOOP;                      \
        go[' ']  = A_LOOP;                      \
        go['\n'] = A_NEWLINE;                   \
    } while (0)

#define GO_ALPHA_SET(go, action)                \
    do {                                        \
        for (int i = 'a'; i <= 'z'; i++) {      \
            go[i] = action;                     \
        }                                       \
        for (int i = 'A'; i <= 'Z'; i++) {      \
            go[i] = action;                     \
        }                                       \
        go['_'] = action;                       \
    } while (0)

#define GO_DIGIT_SET(go, action)                \
    do {                                        \
        go['-'] = action;                       \
        for (int i = '0'; i <= '9'; i++) {      \
            go[i] = action;                     \
        }                                       \
    } while (0)

#define GO_UTF8_SET(go)                         \
    do {                                        \
        for (int i = 192; i <= 223; i++) {      \
            go[i] = A_UTF8_2;                   \
        }                                       \
        for (int i = 224; i <= 239; i++) {      \
            go[i] = A_UTF8_3;                   \
        }                                       \
        for (int i = 240; i <= 247; i++) {      \
            go[i] = A_UTF8_4;                   \
        }                                       \
    } while (0)

#define GO_UTF8_CONTINUE_SET(go)                \
    do {                                        \
        for (int i = 128; i <= 191; i++) {      \
            go[i] = A_UTF_CONTINUE;             \
        }                                       \
    } while (0)

#define GO_ALL_SET(go, action)                  \
    do {                                        \
        for (int i = ' '; i <= '~'; i++) {      \
            go[i] = action;                     \
        }                                       \
    } while (0)

#define GO_256_SET(go, action)                  \
    do {                                        \
        for (int i = 0; i < 256; i++) {         \
            go[i] = action;                     \
        }                                       \
    } while (0)

    if (inited) return;
    inited = true;

    /*
     * object
     */
    GO_WHITESPACE_LOOP(go_plain);
    go_plain['{'] = A_OBJECT;
    go_plain['['] = A_ARRAY;
    go_plain['"'] = A_DATA_QUOTE;
    go_plain['\''] = A_DATA_QUOTE;
    GO_DIGIT_SET(go_plain, A_DATA_NUMBER);
    GO_ALPHA_SET(go_plain, A_DATA_RESERVE);
    go_plain['/'] = A_COMMENT_A;

    GO_WHITESPACE_LOOP(go_object);
    GO_ALPHA_SET(go_object, A_PAIR_L_RAW);
    GO_DIGIT_SET(go_object, A_PAIR_L_RAW);
    go_object['"'] = A_PAIR_L_QUOTE;
    go_object['\''] = A_PAIR_L_QUOTE;
    go_object['/'] = A_COMMENT_A;
    go_object['}'] = A_UNOBJECT;

    GO_ALPHA_SET(go_pair_l_raw, A_LOOP);
    GO_DIGIT_SET(go_pair_l_raw, A_LOOP);
    go_pair_l_raw[' '] = A_UNPAIR_L_RAW;
    go_pair_l_raw['\t'] = A_UNPAIR_L_RAW;
    go_pair_l_raw['\n'] = A_UNPAIR_L_RAW_NEWLINE;
    go_pair_l_raw[':'] = A_UNPAIR_L_PAIR;

    GO_256_SET(go_pair_l_quote, A_LOOP);
    GO_UTF8_SET(go_pair_l_quote);
    go_pair_l_quote['"'] = A_UNPAIR_L_QUOTE;
    go_pair_l_quote['\''] = A_UNPAIR_L_QUOTE;
    go_pair_l_quote['\\'] = A_ESCAPE;

    GO_WHITESPACE_LOOP(go_unpair_l);
    go_unpair_l[':'] = A_PAIR;

    GO_WHITESPACE_LOOP(go_pair);
    GO_ALPHA_SET(go_pair, A_PAIR_R_RAW);
    go_pair['"'] = A_PAIR_R_QUOTE;
    go_pair['\''] = A_PAIR_R_QUOTE;
    GO_DIGIT_SET(go_pair, A_PAIR_R_NUMBER);
    go_pair['{'] = A_PAIR_R_OBJECT;
    go_pair['['] = A_PAIR_R_ARRAY;

    GO_ALPHA_SET(go_pair_r_raw, A_LOOP);
    go_pair_r_raw['.'] = A_UNPAIR_R_RAW;
    go_pair_r_raw[' '] = A_UNPAIR_R_RAW;
    go_pair_r_raw['\t'] = A_UNPAIR_R_RAW;
    go_pair_r_raw['\n'] = A_UNPAIR_R_RAW_NEWLINE;
    go_pair_r_raw['}'] = A_UNPAIR_R_RAW_BRACE;
    go_pair_r_raw[','] = A_UNPAIR_R_RAW_COMMA;

    GO_256_SET(go_pair_r_quote, A_LOOP);
    GO_UTF8_SET(go_pair_r_quote);
    go_pair_r_quote['"'] = A_UNPAIR_R_QUOTE;
    go_pair_r_quote['\''] = A_UNPAIR_R_QUOTE;
    go_pair_r_quote['\\'] = A_ESCAPE;

    GO_DIGIT_SET(go_pair_r_number, A_LOOP);
    go_pair_r_number['.'] = A_PAIR_R_DOUBLE;
    go_pair_r_number[' '] = A_UNPAIR_R_NUMBER;
    go_pair_r_number['\t'] = A_UNPAIR_R_NUMBER;
    go_pair_r_number['\n'] = A_UNPAIR_R_NUMBER_NEWLINE;
    go_pair_r_number['}'] = A_UNPAIR_R_NUMBER_BRACE;
    go_pair_r_number[','] = A_UNPAIR_R_NUMBER_COMMA;

    GO_WHITESPACE_LOOP(go_unpair_r);
    go_unpair_r[','] = A_PAIR_NEXT;
    go_unpair_r['}'] = A_UNOBJECT;
    go_unpair_r['/'] = A_COMMENT_A;

    /*
     * array
     */
    GO_WHITESPACE_LOOP(go_array);
    GO_ALPHA_SET(go_array, A_VALUE_RAW);
    go_array['"'] = A_VALUE_QUOTE;
    go_array['\''] = A_VALUE_QUOTE;
    GO_DIGIT_SET(go_array, A_VALUE_NUMBER);
    go_array['{'] = A_VALUE_OBJECT;
    go_array['['] = A_VALUE_ARRAY;
    go_array['/'] = A_COMMENT_A;
    go_array[']'] = A_UNARRAY;

    GO_ALPHA_SET(go_value_raw, A_LOOP);
    go_value_raw['.'] = A_UNVALUE_RAW;
    go_value_raw[' '] = A_UNVALUE_RAW;
    go_value_raw['\n'] = A_UNVALUE_RAW_NEWLINE;
    go_value_raw['\t'] = A_UNVALUE_RAW;
    go_value_raw[']'] = A_UNVALUE_RAW_SQUARE;
    go_value_raw[','] = A_UNVALUE_RAW_COMMA;

    GO_256_SET(go_value_quote, A_LOOP);
    GO_UTF8_SET(go_value_quote);
    go_value_quote['"'] = A_UNVALUE_QUOTE;
    go_value_quote['\''] = A_UNVALUE_QUOTE;
    go_value_quote['\\'] = A_ESCAPE;

    GO_DIGIT_SET(go_value_number, A_LOOP);
    go_value_number['.'] = A_VALUE_DOUBLE;
    go_value_number[' '] = A_UNVALUE_NUMBER;
    go_value_number['\t'] = A_UNVALUE_NUMBER;
    go_value_number['\n'] = A_UNVALUE_NUMBER_NEWLINE;
    go_value_number[']'] = A_UNVALUE_NUMBER_SQUARE;
    go_value_number[','] = A_UNVALUE_NUMBER_COMMA;

    GO_WHITESPACE_LOOP(go_unvalue);
    go_unvalue[','] = A_VALUE_NEXT;
    go_unvalue[']'] = A_UNARRAY;
    go_unvalue['/'] = A_COMMENT_A;

    /*
     * data
     */
    GO_ALL_SET(go_data_quote, A_LOOP);
    GO_UTF8_SET(go_data_quote);
    go_data_quote['\\'] = A_ESCAPE;
    go_data_quote['"'] = A_UNDATA_QUOTE;
    go_data_quote['\''] = A_UNDATA_QUOTE;

    GO_ALL_SET(go_data_number, A_BAD);
    GO_DIGIT_SET(go_data_number, A_LOOP);
    go_data_number['.'] = A_DATA_DOUBLE;

    GO_ALL_SET(go_data_reserve, A_BAD);
    GO_ALPHA_SET(go_data_reserve, A_LOOP);

    /*
     * misc
     */
    go_comment_a['/'] = A_COMMENT_B;

    GO_256_SET(go_comment_b, A_LOOP);
    go_comment_b['\n'] = A_UNCOMMENT_B;

    GO_UTF8_CONTINUE_SET(go_utf8_continue);
}

#define CHAR_IN(c, x, y) (c == x || c == y)

static inline void _add_pair_fixtype(MDF *node, char *name, char *value,
                                     int namelen, int valuelen,
                                     MDF_TYPE nodetype, int nodenum)
{
    MDF *xnode;

    mdf_init(&xnode);
    xnode->name = mstr_ndup_json_string(name, namelen);
    xnode->namelen = namelen;
    xnode->type = nodetype;

    if (nodetype == MDF_TYPE_STRING) {
        xnode->val.s = mstr_ndup_json_string(value, valuelen);
        xnode->valuelen = valuelen;
    } else if (nodetype == MDF_TYPE_INT) {
        char *s = mstr_ndup(value, valuelen);
        xnode->val.n = strtoll(s, NULL, 10);
        mos_free(s);
    } else if (nodetype == MDF_TYPE_DOUBLE) {
        char *s = mstr_ndup(value, valuelen);
        xnode->val.f = strtod(s, NULL);
        mos_free(s);
    }

    _mdf_append_child_node(node, xnode, nodenum);
}

static inline MERR* _add_pair_unknowntype(MDF *node, char *name, char *value,
                                          int namelen, int valuelen, int nodenum,
                                          const char *fname, int lineno, int columnno)
{
    MDF *xnode;

    mdf_init(&xnode);
    xnode->name = mstr_ndup_json_string(name, namelen);
    xnode->namelen = namelen;

    if (valuelen == 4) {
        if (CHAR_IN(*value, 't', 'T') && CHAR_IN(*(value+1), 'r', 'R') &&
            CHAR_IN(*(value+2), 'u', 'U') && CHAR_IN(*(value+3), 'e', 'E')) {
            xnode->type = MDF_TYPE_BOOL;
            xnode->val.n = 1;
        } else if (CHAR_IN(*value, 'n', 'N') && CHAR_IN(*(value+1), 'u', 'U') &&
            CHAR_IN(*(value+2), 'l', 'L') && CHAR_IN(*(value+3), 'l', 'L')) {
            xnode->type = MDF_TYPE_NULL;
            xnode->val.n = 0;
        } else goto error;
    } else if (valuelen == 5) {
        if (CHAR_IN(*value, 'f', 'F') && CHAR_IN(*(value+1), 'a', 'A') &&
            CHAR_IN(*(value+2), 'l', 'L') && CHAR_IN(*(value+3), 's', 'S') &&
            CHAR_IN(*(value+4), 'e', 'E')) {
            xnode->type = MDF_TYPE_BOOL;
            xnode->val.n = 0;
        } else goto error;
    } else goto error;

    _mdf_append_child_node(node, xnode, nodenum);

    return MERR_OK;

error:
    mdf_destroy(&xnode);
    return merr_raise(MERR_ASSERT, "unexpect token '%c' near line %d:%d of %s",
                      *value, lineno, columnno, fname);
}

static inline void _add_value_fixtype(MDF *node, char *value, int valuelen,
                                      MDF_TYPE nodetype, int nodenum)
{
    char arrayindex[64] = {0};
    MDF *xnode;

    snprintf(arrayindex, sizeof(arrayindex), "%d", nodenum);

    mdf_init(&xnode);
    xnode->name = strdup(arrayindex);
    xnode->namelen = strlen(arrayindex);
    xnode->type = nodetype;

    if (nodetype == MDF_TYPE_STRING) {
        xnode->val.s = mstr_ndup_json_string(value, valuelen);
        xnode->valuelen = valuelen;
    } else if (nodetype == MDF_TYPE_INT) {
        char *s = mstr_ndup(value, valuelen);
        xnode->val.n = strtoll(s, NULL, 10);
        mos_free(s);
    } else if (nodetype == MDF_TYPE_DOUBLE) {
        char *s = mstr_ndup(value, valuelen);
        xnode->val.f = strtod(s, NULL);
        mos_free(s);
    }

    _mdf_append_child_node(node, xnode, nodenum);
}

static inline MERR* _add_value_unknowntype(MDF *node, char *value, int valuelen,
                                           int nodenum,
                                           const char *fname, int lineno, int columnno)
{
    char arrayindex[64] = {0};
    MDF *xnode;

    snprintf(arrayindex, sizeof(arrayindex), "%d", nodenum);

    mdf_init(&xnode);
    xnode->name = strdup(arrayindex);
    xnode->namelen = strlen(arrayindex);

    if (valuelen == 4) {
        if (CHAR_IN(*value, 't', 'T') && CHAR_IN(*(value+1), 'r', 'R') &&
            CHAR_IN(*(value+2), 'u', 'U') && CHAR_IN(*(value+3), 'e', 'E')) {
            xnode->type = MDF_TYPE_BOOL;
            xnode->val.n = 1;
        } else if (CHAR_IN(*value, 'n', 'N') && CHAR_IN(*(value+1), 'u', 'U') &&
            CHAR_IN(*(value+2), 'l', 'L') && CHAR_IN(*(value+3), 'l', 'L')) {
            xnode->type = MDF_TYPE_NULL;
            xnode->val.n = 0;
        } else goto error;
    } else if (valuelen == 5) {
        if (CHAR_IN(*value, 'f', 'F') && CHAR_IN(*(value+1), 'a', 'A') &&
            CHAR_IN(*(value+2), 'l', 'L') && CHAR_IN(*(value+3), 's', 'S') &&
            CHAR_IN(*(value+4), 'e', 'E')) {
            xnode->type = MDF_TYPE_BOOL;
            xnode->val.n = 0;
        } else goto error;
    } else goto error;

    _mdf_append_child_node(node, xnode, nodenum);

    return MERR_OK;

error:
    mdf_destroy(&xnode);
    return merr_raise(MERR_ASSERT, "unexpect token '%c' near line %d:%d of %s",
                      *value, lineno, columnno, fname);
}

static inline MERR* _set_value_fixtype(MDF *node, char *value, int valuelen, MDF_TYPE nodetype)
{
    if (node->type == MDF_TYPE_STRING || node->type == MDF_TYPE_BINARY_A)
        mos_free(node->val.s);
    if (node->type == MDF_TYPE_OBJECT || node->type == MDF_TYPE_ARRAY) {
        mdf_destroy(&node->child);
        node->last_child = NULL;
    }
    node->type = nodetype;

    if (nodetype == MDF_TYPE_STRING) {
        node->val.s = mstr_ndup_json_string(value, valuelen);
        node->valuelen = valuelen;
    } else if (nodetype == MDF_TYPE_INT) {
        char *s = mstr_ndup(value, valuelen);
        node->val.n = strtoll(s, NULL, 10);
        mos_free(s);
    } else if (nodetype == MDF_TYPE_DOUBLE) {
        char *s = mstr_ndup(value, valuelen);
        node->val.f = strtod(s, NULL);
        mos_free(s);
    } else return merr_raise(MERR_ASSERT, "unsupport type %d", nodetype);

    return MERR_OK;
}

static inline MERR* _set_value_unknowntype(MDF *node, char *value, int valuelen)
{
    if (node->type == MDF_TYPE_STRING || node->type == MDF_TYPE_BINARY_A)
        mos_free(node->val.s);
    if (node->type == MDF_TYPE_OBJECT || node->type == MDF_TYPE_ARRAY) {
        mdf_destroy(&node->child);
        node->last_child = NULL;
    }

    if (valuelen == 4) {
        if (CHAR_IN(*value, 't', 'T') && CHAR_IN(*(value+1), 'r', 'R') &&
            CHAR_IN(*(value+2), 'u', 'U') && CHAR_IN(*(value+3), 'e', 'E')) {
            node->type = MDF_TYPE_BOOL;
            node->val.n = 1;
        } else if (CHAR_IN(*value, 'n', 'N') && CHAR_IN(*(value+1), 'u', 'U') &&
            CHAR_IN(*(value+2), 'l', 'L') && CHAR_IN(*(value+3), 'l', 'L')) {
            node->type = MDF_TYPE_NULL;
            node->val.n = 0;
        } else goto error;
    } else if (valuelen == 5) {
        if (CHAR_IN(*value, 'f', 'F') && CHAR_IN(*(value+1), 'a', 'A') &&
            CHAR_IN(*(value+2), 'l', 'L') && CHAR_IN(*(value+3), 's', 'S') &&
            CHAR_IN(*(value+4), 'e', 'E')) {
            node->type = MDF_TYPE_BOOL;
            node->val.n = 0;
        } else goto error;
    } else goto error;

    return MERR_OK;

error:
    return merr_raise(MERR_ASSERT, "unknown value %.*s", valuelen, value);
}

static MERR* _import_json(MDF *node, const char *str,
                          size_t *xlen, const char *fname, int *lineno)
{
    char *pos;
    int8_t *go, *go_nearby;

    char *name, *value, quotechar, arrayindex[64];
    int namelen, valuelen, utf8_remain, nodenum, columnno;
    MDF_TYPE nodetype;
    MDF *xnode;

    MERR *err;

#define AFTER_NODE_APPEND()                     \
    do {                                        \
        name = value = NULL;                    \
        namelen = valuelen = -1;                \
        nodetype = MDF_TYPE_UNKNOWN;            \
        xnode = NULL;                           \
        nodenum++;                              \
    } while (0)

    if (!str || !*str) return MERR_OK;

    _json_machine_init();

    go = go_plain;
    go_nearby = NULL;
    pos = (char*)str;

    name = value = NULL;
    namelen = valuelen = -1;
    utf8_remain = 0;
    nodenum = 0;
    columnno = 1;
    nodetype = MDF_TYPE_UNKNOWN;
    quotechar = '\0';

    while (*pos != '\0') {
        switch (go[*(const uint8_t*)pos]) {
        case A_LOOP:
            break;
        case A_ESCAPE:
            pos++;
            break;
        case A_NEWLINE:
            *lineno = *lineno + 1;
            columnno = 0;
            break;

            /*
             * object begin
             */
        case A_OBJECT:
            node->type = MDF_TYPE_OBJECT;
            go = go_object;
            break;

        case A_PAIR_L_RAW:
            name = pos;
            namelen = -1;
            go = go_pair_l_raw;
            break;

        case A_PAIR_L_QUOTE:
            quotechar = *pos;
            name = pos + 1;
            namelen = -1;
            go = go_pair_l_quote;
            break;

        case A_UNPAIR_L_RAW_NEWLINE:
            *lineno = *lineno + 1;
            columnno = 0;
        case A_UNPAIR_L_RAW:
            if (name) namelen = pos - name;
            go = go_unpair_l;
            break;

        case A_UNPAIR_L_QUOTE:
            if (*pos == quotechar) {
                if (name) namelen = pos - name;
                go = go_unpair_l;
            }
            break;

        case A_UNPAIR_L_PAIR:
            if (name) namelen = pos - name;
            goto action_pair;

        action_pair:
        case A_PAIR:
            go = go_pair;
            break;

        case A_PAIR_R_RAW:
            nodetype = MDF_TYPE_UNKNOWN;
            value = pos;
            valuelen = -1;
            go = go_pair_r_raw;
            break;

        case A_PAIR_R_QUOTE:
            quotechar = *pos;
            nodetype = MDF_TYPE_STRING;
            value = pos + 1;
            valuelen = -1;
            go = go_pair_r_quote;
            break;

        case A_PAIR_R_NUMBER:
            nodetype = MDF_TYPE_INT;
            value = pos;
            valuelen = -1;
            go = go_pair_r_number;
            break;

        case A_PAIR_R_DOUBLE:
            nodetype = MDF_TYPE_DOUBLE;
            go = go_pair_r_number;
            break;

        case A_PAIR_R_OBJECT:
        case A_PAIR_R_ARRAY:
        {
            size_t childlen;

            mdf_init(&xnode);
            xnode->name = mstr_ndup_json_string(name, namelen);
            xnode->namelen = namelen;

            err = _import_json(xnode, pos, &childlen, fname, lineno);
            if (err) return merr_pass(err);
            pos += childlen;
            columnno += childlen;

            _mdf_append_child_node(node, xnode, nodenum);

            AFTER_NODE_APPEND();
            go = go_unpair_r;
            break;
        }

        case A_UNPAIR_R_RAW_NEWLINE:
            *lineno = *lineno + 1;
            columnno = 0;
        case A_UNPAIR_R_RAW:
            if (value) valuelen = pos - value;
            else goto bad_char;

            err = _add_pair_unknowntype(node, name, value, namelen, valuelen, nodenum,
                                        fname, *lineno, columnno);
            if (err) return merr_pass(err);

            AFTER_NODE_APPEND();
            go = go_unpair_r;
            break;

        case A_UNPAIR_R_RAW_BRACE:
            if (value) valuelen = pos - value;
            else goto bad_char;

            err = _add_pair_unknowntype(node, name, value, namelen, valuelen, nodenum,
                                        fname, *lineno, columnno);
            if (err) return merr_pass(err);

            AFTER_NODE_APPEND();
            goto action_unobj;

        case A_UNPAIR_R_RAW_COMMA:
            if (value) valuelen = pos - value;
            else goto bad_char;

            err = _add_pair_unknowntype(node, name, value, namelen, valuelen, nodenum,
                                        fname, *lineno, columnno);
            if (err) return merr_pass(err);

            AFTER_NODE_APPEND();
            goto action_pair_next;

        case A_UNPAIR_R_QUOTE:
            if (*pos == quotechar) {
                if (value) valuelen = pos - value;
                else goto bad_char;

                _add_pair_fixtype(node, name, value, namelen, valuelen,
                                  nodetype, nodenum);

                AFTER_NODE_APPEND();
                go = go_unpair_r;
            }
            break;

        case A_UNPAIR_R_NUMBER_NEWLINE:
            *lineno = *lineno + 1;
            columnno = 0;
        case A_UNPAIR_R_NUMBER:
            if (value) valuelen = pos - value;
            else goto bad_char;

            _add_pair_fixtype(node, name, value, namelen, valuelen, nodetype, nodenum);

            AFTER_NODE_APPEND();
            go = go_unpair_r;
            break;

        case A_UNPAIR_R_NUMBER_BRACE:
            if (value) valuelen = pos - value;
            else goto bad_char;

            _add_pair_fixtype(node, name, value, namelen, valuelen, nodetype, nodenum);

            AFTER_NODE_APPEND();
            goto action_unobj;

        case A_UNPAIR_R_NUMBER_COMMA:
            if (value) valuelen = pos - value;
            else goto bad_char;

            _add_pair_fixtype(node, name, value, namelen, valuelen, nodetype, nodenum);

            AFTER_NODE_APPEND();
            goto action_pair_next;

        action_pair_next:
        case A_PAIR_NEXT:
            go = go_object;
            break;

        action_unobj:
        case A_UNOBJECT:
            if (xlen) *xlen = pos - str;
            return MERR_OK;
            /*
             * object end
             */

            /*
             * array begin
             */
        case A_ARRAY:
            node->type = MDF_TYPE_ARRAY;
            go = go_array;
            break;

        case A_VALUE_RAW:
            nodetype = MDF_TYPE_UNKNOWN;
            value = pos;
            valuelen = -1;
            go = go_value_raw;
            break;

        case A_VALUE_QUOTE:
            quotechar = *pos;
            nodetype = MDF_TYPE_STRING;
            value = pos + 1;
            valuelen = -1;
            go = go_value_quote;
            break;

        case A_VALUE_NUMBER:
            nodetype = MDF_TYPE_INT;
            value = pos;
            valuelen = -1;
            go = go_value_number;
            break;

        case A_VALUE_DOUBLE:
            nodetype = MDF_TYPE_DOUBLE;
            go = go_value_number;
            break;

        case A_VALUE_OBJECT:
        case A_VALUE_ARRAY:
        {
            size_t childlen;

            mdf_init(&xnode);
            memset(arrayindex, 0x0, sizeof(arrayindex));
            snprintf(arrayindex, sizeof(arrayindex), "%d", nodenum);
            xnode->name = strdup(arrayindex);
            xnode->namelen = strlen(arrayindex);

            err = _import_json(xnode, pos, &childlen, fname, lineno);
            if (err) return merr_pass(err);
            pos += childlen;
            columnno += childlen;

            _mdf_append_child_node(node, xnode, nodenum);

            AFTER_NODE_APPEND();
            go = go_unvalue;
        }
            break;

        case A_UNVALUE_RAW_NEWLINE:
            *lineno = *lineno + 1;
            columnno = 0;
        case A_UNVALUE_RAW:
            if (value) valuelen = pos - value;
            else goto bad_char;

            err = _add_value_unknowntype(node, value, valuelen, nodenum,
                                         fname, *lineno, columnno);
            if (err) return merr_pass(err);

            AFTER_NODE_APPEND();
            go = go_unvalue;
            break;

        case A_UNVALUE_RAW_SQUARE:
            if (value) valuelen = pos - value;
            else goto bad_char;

            err = _add_value_unknowntype(node, value, valuelen, nodenum,
                                         fname, *lineno, columnno);
            if (err) return merr_pass(err);

            AFTER_NODE_APPEND();
            goto action_unarray;

        case A_UNVALUE_RAW_COMMA:
            if (value) valuelen = pos - value;
            else goto bad_char;

            err = _add_value_unknowntype(node, value, valuelen, nodenum,
                                         fname, *lineno, columnno);
            if (err) return merr_pass(err);

            AFTER_NODE_APPEND();
            goto action_value_next;

        case A_UNVALUE_QUOTE:
            if (*pos == quotechar) {
                if (value) valuelen = pos - value;
                else goto bad_char;

                _add_value_fixtype(node, value, valuelen, nodetype, nodenum);

                AFTER_NODE_APPEND();
                go = go_unvalue;
            }
            break;

        case A_UNVALUE_NUMBER_NEWLINE:
            *lineno = *lineno + 1;
            columnno =0;
        case A_UNVALUE_NUMBER:
            if (value) valuelen = pos - value;
            else goto bad_char;

            _add_value_fixtype(node, value, valuelen, nodetype, nodenum);

            AFTER_NODE_APPEND();
            go = go_unvalue;
            break;

        case A_UNVALUE_NUMBER_SQUARE:
            if (value) valuelen = pos - value;
            else goto bad_char;

            _add_value_fixtype(node, value, valuelen, nodetype, nodenum);

            AFTER_NODE_APPEND();
            goto action_unarray;

        case A_UNVALUE_NUMBER_COMMA:
            if (value) valuelen = pos - value;
            else goto bad_char;

            _add_value_fixtype(node, value, valuelen, nodetype, nodenum);

            AFTER_NODE_APPEND();
            goto action_value_next;

        action_value_next:
        case A_VALUE_NEXT:
            go = go_array;
            break;

        action_unarray:
        case A_UNARRAY:
            if (xlen) *xlen = pos - str;
            return MERR_OK;
            /*
             * array end
             */

            /*
             * data begin
             */
        case A_DATA_QUOTE:
            quotechar = *pos;
            nodetype = MDF_TYPE_STRING;
            value = pos + 1;
            valuelen = -1;
            go = go_data_quote;
            break;

        case A_DATA_NUMBER:
            nodetype = MDF_TYPE_INT;
            value = pos;
            valuelen = -1;
            go = go_data_number;
            break;

        case A_DATA_DOUBLE:
            nodetype = MDF_TYPE_DOUBLE;
            go = go_data_number;
            break;

        case A_DATA_RESERVE:
            value = pos;
            valuelen = -1;
            go = go_data_reserve;
            break;

        case A_UNDATA_QUOTE:
            if (value) valuelen = pos - value;
            else goto bad_char;

            if (*pos == quotechar) {
                err = _set_value_fixtype(node, value, valuelen, nodetype);
                return merr_pass(err);
            }
            break;
            /*
             * data end
             */

        case A_COMMENT_A:
            go_nearby = go;
            go = go_comment_a;
            break;

        case A_COMMENT_B:
            go = go_comment_b;
            break;

        case A_UNCOMMENT_B:
            go = go_nearby;
            break;

        case A_UTF8_2:
            go_nearby = go;
            go = go_utf8_continue;
            utf8_remain = 1;
            break;

        case A_UTF8_3:
            go_nearby = go;
            go = go_utf8_continue;
            utf8_remain = 2;
            break;

        case A_UTF8_4:
            go_nearby = go;
            go = go_utf8_continue;
            utf8_remain = 3;
            break;

        case A_UTF_CONTINUE:
            if (!--utf8_remain) go = go_nearby;
            break;

        bad_char:
        case A_BAD:
        default:
            return merr_raise(MERR_ASSERT, "unexpect token '%c' in line %d:%d of %s",
                              *pos, *lineno, columnno, fname);
        }

        pos++;
        columnno++;
    }

    if (go == go_data_number) {
        if (value) valuelen = pos - value;
        else return merr_raise(MERR_ASSERT, "illgal json string");

        err = _set_value_fixtype(node, value, valuelen, nodetype);
        return merr_pass(err);
    } else if (go == go_data_reserve) {
        if (value) valuelen = pos - value;
        else return merr_raise(MERR_ASSERT, "illgal json string");

        err = _set_value_unknowntype(node, value, valuelen);
        return merr_pass(err);
    }

    return merr_raise(MERR_ASSERT, "illgal json string");
}

struct json_outbuf {
    char *buf;
    size_t len;
    size_t max;
};

static void _json_outbuf_appendf(void *rock, const char *fmt, ...)
{
    va_list ap;
    int len, remain;

    struct json_outbuf *jbuf = rock;

    if (!rock || !fmt) return;

    remain = jbuf->max - jbuf->len;
    if (remain <= 0) return;

    va_start(ap, fmt);
    len = vsnprintf(jbuf->buf + jbuf->len, remain, fmt, ap);
    va_end(ap);

    if (len >= remain) jbuf->len = jbuf->max + 1;
    else jbuf->len += len;
}

/*
 * level 参数用来控制输出格式，-1为单行输出，否则会以换行和缩进控制输出
 */
static void _export_json_string(MDF *node, void *rock, MDF_PRINTF mprintf, int level, MDF *nnode)
{
    MDF *cnode;

#define PAD_SPACE() for (int _i = 0; _i < level; _i++) mprintf(rock, "  ");
#define NEWLINE() if (level >= 0) mprintf(rock, "\n");

    switch(node->type) {
    case MDF_TYPE_OBJECT:
        mprintf(rock, "{"); NEWLINE();

        level = level < 0 ? -1 : level + 1;
        cnode = node->child;
        while (cnode) {
            /* TODO escape " in name */
            PAD_SPACE(); mprintf(rock, "\"%s\": ", cnode->name);
            _export_json_string(cnode, rock, mprintf, level, cnode->next);

            cnode = cnode->next;
        }

        level = level < 0 ? -1: level - 1;
        PAD_SPACE(); mprintf(rock, "}");
        break;
    case MDF_TYPE_ARRAY:
        mprintf(rock, "["); NEWLINE();

        level = level < 0 ? -1 : level + 1;
        cnode = node->child;
        while (cnode) {
            PAD_SPACE();
            _export_json_string(cnode, rock, mprintf, level, cnode->next);

            cnode = cnode->next;
        }

        level = level < 0 ? -1: level - 1;
        PAD_SPACE(); mprintf(rock, "]");
        break;
    case MDF_TYPE_STRING:
        if (node->val.s) {
            mprintf(rock, "\"");
            char *s = node->val.s;
            while (*s) {
                if (*s == '"') mprintf(rock, "\\\"");
                else if (*s == '\r') mprintf(rock, "\\r");
                else if (*s == '\n') mprintf(rock, "\\n");
                else mprintf(rock, "%c", *s);
                s++;
            }
            mprintf(rock, "\"");
        } else mprintf(rock, "null");
        break;
    case MDF_TYPE_INT:
        mprintf(rock, "%ld", node->val.n);
        break;
    case MDF_TYPE_DOUBLE:
        mprintf(rock, "%.6f", node->val.f);
        break;
    case MDF_TYPE_BOOL:
        if (node->val.n != 0) mprintf(rock, "true");
        else mprintf(rock, "false");
        break;
    case MDF_TYPE_NULL:
        mprintf(rock, "null");
        break;
    default:
        /* null for MDF_TYPE_UNKNOWN */
        mprintf(rock, "null");
        break;
    }

    if (nnode) mprintf(rock, ", ");
    NEWLINE();
}

MERR* mdf_json_import_string(MDF *node, const char *str)
{
    int lineno;
    MERR *err;

    MERR_NOT_NULLA(node);

    if (!str) return MERR_OK;

    lineno = 1;

    err = _import_json(node, str, NULL, "<string>", &lineno);
    if (err) return merr_pass(err);

    return MERR_OK;
}

MERR* mdf_json_import_file(MDF *node, const char *fname)
{
    struct stat fs;
    int lineno;
    char *buf;
    FILE *fp;
    MERR *err;

    MERR_NOT_NULLB(node, fname);

    lineno = 1;

    if (stat(fname, &fs) == -1)
        return merr_raise(MERR_OPENFILE, "stat %s failure", fname);

    if (fs.st_size < 0 || fs.st_size > INT32_MAX)
        return merr_raise(MERR_ASSERT, "file size error %ld", (long int)fs.st_size);

    buf = mos_calloc(1, fs.st_size + 1);

    fp = fopen(fname, "r");
    if (!fp) {
        mos_free(buf);
        return merr_raise(MERR_OPENFILE, "open %s for read failure", fname);
    }

    if (fread(buf, 1, fs.st_size, fp) != fs.st_size) {
        fclose(fp);
        mos_free(buf);
        return merr_raise(MERR_ASSERT, "read file failure %ld", (long int)fs.st_size);
    }
    buf[fs.st_size] = '\0';
    fclose(fp);

    err = _import_json(node, buf, NULL, fname, &lineno);
    if (err) {
        mos_free(buf);
        return merr_pass(err);
    }

    mos_free(buf);

    return MERR_OK;
}

char* mdf_json_export_string(MDF *node)
{
    MSTR astr;

    if (!node) return NULL;

    mstr_init(&astr);
    _export_json_string(node, &astr, (MDF_PRINTF)mstr_appendf, -1, NULL);

    return astr.buf;
}

char* mdf_json_export_string_pretty(MDF *node)
{
    MSTR astr;

    if (!node) return NULL;

    mstr_init(&astr);
    _export_json_string(node, &astr, (MDF_PRINTF)mstr_appendf, 0, NULL);

    return astr.buf;
}

size_t mdf_json_export_buffer(MDF *node, char *buf, size_t len)
{
    struct json_outbuf jbuf;

    if (!node) return 0;

    memset(buf, 0x0, len);

    jbuf.buf = buf;
    jbuf.max = len;
    jbuf.len = 0;

    _export_json_string(node, &jbuf, (MDF_PRINTF)_json_outbuf_appendf, -1, NULL);

    if (jbuf.len > jbuf.max) return 0;

    return jbuf.len;
}

MERR* mdf_json_export_file(MDF *node, const char *fname)
{
    FILE *fp;

    if (!node) return MERR_OK;

    if (!fname || !strcmp(fname, "-")) fp = stdout;
    else fp = fopen(fname, "w");
    if (!fp) return merr_raise(MERR_OPENFILE, "open %s for write failure", fname);

    _export_json_string(node, fp, (MDF_PRINTF)fprintf, 0, NULL);

    if (fname && strcmp(fname, "-")) fclose(fp);

    return MERR_OK;
}
