#include "reef.h"
#include "_mdf.h"

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

    if (inited) return;
    inited = true;

    /*
     * object
     */
    GO_WHITESPACE_LOOP(go_plain);
    go_plain['{'] = A_OBJECT;
    go_plain['['] = A_ARRAY;

    GO_WHITESPACE_LOOP(go_object);
    GO_ALPHA_SET(go_object, A_PAIR_L_RAW);
    go_object['"'] = A_PAIR_L_QUOTE;
    go_object['\''] = A_PAIR_L_QUOTE;

    GO_ALPHA_SET(go_pair_l_raw, A_LOOP);
    go_pair_l_raw[' '] = A_UNPAIR_L_RAW;
    go_pair_l_raw['\t'] = A_UNPAIR_L_RAW;
    go_pair_l_raw['\n'] = A_UNPAIR_L_RAW_NEWLINE;
    go_pair_l_raw[':'] = A_UNPAIR_L_PAIR;

    GO_ALL_SET(go_pair_l_quote, A_LOOP);
    GO_UTF8_SET(go_pair_l_quote);
    go_pair_l_quote['"'] = A_UNPAIR_L_QUOTE;
    go_pair_l_quote['\''] = A_UNPAIR_L_QUOTE;

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

    GO_ALL_SET(go_pair_r_quote, A_LOOP);
    GO_UTF8_SET(go_pair_r_quote);
    go_pair_r_quote['"'] = A_UNPAIR_R_QUOTE;
    go_pair_r_quote['\''] = A_UNPAIR_R_QUOTE;

    GO_DIGIT_SET(go_pair_r_number, A_LOOP);
    go_pair_r_number['.'] = A_PAIR_R_FLOAT;
    go_pair_r_number[' '] = A_UNPAIR_R_NUMBER;
    go_pair_r_number['\t'] = A_UNPAIR_R_NUMBER;
    go_pair_r_number['\n'] = A_UNPAIR_R_NUMBER_NEWLINE;
    go_pair_r_number['}'] = A_UNPAIR_R_NUMBER_BRACE;
    go_pair_r_number[','] = A_UNPAIR_R_NUMBER_COMMA;

    GO_WHITESPACE_LOOP(go_unpair_r);
    go_unpair_r[','] = A_PAIR_NEXT;
    go_unpair_r['}'] = A_UNOBJECT;

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

    GO_ALPHA_SET(go_value_raw, A_LOOP);
    go_value_raw['.'] = A_UNVALUE_RAW;
    go_value_raw[' '] = A_UNVALUE_RAW;
    go_value_raw['\n'] = A_UNVALUE_RAW_NEWLINE;
    go_value_raw['\t'] = A_UNVALUE_RAW;
    go_value_raw[']'] = A_UNVALUE_RAW_SQUARE;
    go_value_raw[','] = A_UNVALUE_RAW_COMMA;

    GO_ALL_SET(go_value_quote, A_LOOP);
    GO_UTF8_SET(go_value_quote);
    go_value_quote['"'] = A_UNVALUE_QUOTE;
    go_value_quote['\''] = A_UNVALUE_QUOTE;

    GO_DIGIT_SET(go_value_number, A_LOOP);
    go_value_number['.'] = A_VALUE_FLOAT;
    go_value_number[' '] = A_UNVALUE_NUMBER;
    go_value_number['\t'] = A_UNVALUE_NUMBER;
    go_value_number['\n'] = A_UNVALUE_NUMBER_NEWLINE;
    go_value_number[']'] = A_UNVALUE_NUMBER_SQUARE;
    go_value_number[','] = A_UNVALUE_NUMBER_COMMA;

    GO_WHITESPACE_LOOP(go_unvalue);
    go_unvalue[','] = A_VALUE_NEXT;
    go_unvalue[']'] = A_UNARRAY;

    /*
     * misc
     */
    GO_UTF8_CONTINUE_SET(go_utf8_continue);
}

static inline void _add_pair_fixtype(MDF *node, char *name, char *value,
                                     int namelen, int valuelen,
                                     MDF_TYPE nodetype, int nodenum)
{
    MDF *xnode;

    mdf_init(&xnode);
    xnode->name = strndup(name, namelen);
    xnode->type = nodetype;

    if (nodetype == MDF_TYPE_STRING) {
        xnode->val.s = strndup(value, valuelen);
    } else if (nodetype == MDF_TYPE_INT) {
        char *s = strndup(value, valuelen);
        xnode->val.n = strtoll(s, NULL, 10);
        mos_free(s);
    } else if (nodetype == MDF_TYPE_FLOAT) {
        char *s = strndup(value, valuelen);
        xnode->val.f = strtof(s, NULL);
        mos_free(s);
    }

    _mdf_append_child_node(node, xnode, nodenum);
}

static inline MERR* _add_pair_unknowntype(MDF *node, char *name, char *value,
                                          int namelen, int valuelen, int nodenum)
{
    MDF *xnode;

    mdf_init(&xnode);
    xnode->name = strndup(name, namelen);

    if (!strncmp("true", value, valuelen)) {
        xnode->type = MDF_TYPE_BOOL;
        xnode->val.n = 1;
    } else if (!strncmp("false", value, valuelen)) {
        xnode->type = MDF_TYPE_BOOL;
        xnode->val.n = 0;
    } else if (!strncmp("null", value, valuelen)) {
        xnode->type = MDF_TYPE_NULL;
        xnode->val.n = 0;
    } else {
        mdf_destroy(&xnode);
        return merr_raise(MERR_ASSERT, "unexpect value %c", *value);
    }

    _mdf_append_child_node(node, xnode, nodenum);

    return MERR_OK;
}

static inline void _add_value_fixtype(MDF *node, char *value, int valuelen,
                                      MDF_TYPE nodetype, int nodenum)
{
    char arrayindex[64] = {0};
    MDF *xnode;

    snprintf(arrayindex, sizeof(arrayindex), "%d", nodenum);

    mdf_init(&xnode);
    xnode->name = strdup(arrayindex);
    xnode->type = nodetype;

    if (nodetype == MDF_TYPE_STRING) {
        xnode->val.s = strndup(value, valuelen);
    } else if (nodetype == MDF_TYPE_INT) {
        char *s = strndup(value, valuelen);
        xnode->val.n = strtoll(s, NULL, 10);
        mos_free(s);
    } else if (nodetype == MDF_TYPE_FLOAT) {
        char *s = strndup(value, valuelen);
        xnode->val.f = strtof(s, NULL);
        mos_free(s);
    }

    _mdf_append_child_node(node, xnode, nodenum);
}

static inline MERR* _add_value_unknowntype(MDF *node, char *value, int valuelen,
                                           int nodenum)
{
    char arrayindex[64] = {0};
    MDF *xnode;

    snprintf(arrayindex, sizeof(arrayindex), "%d", nodenum);

    mdf_init(&xnode);
    xnode->name = strdup(arrayindex);

    if (!strncmp("true", value, valuelen)) {
        xnode->type = MDF_TYPE_BOOL;
        xnode->val.n = 1;
    } else if (!strncmp("false", value, valuelen)) {
        xnode->type = MDF_TYPE_BOOL;
        xnode->val.n = 0;
    } else if (!strncmp("null", value, valuelen)) {
        xnode->type = MDF_TYPE_NULL;
        xnode->val.n = 0;
    } else {
        mdf_destroy(&xnode);
        return merr_raise(MERR_ASSERT, "unexpect value");
    }

    _mdf_append_child_node(node, xnode, nodenum);

    return MERR_OK;
}

static MERR* _import_json(MDF *node, const char *str,
                          size_t *xlen, const char *fname, int *lineno)
{
    char *pos;
    int8_t *go, *go_nearby;

    char *name, *value, quotechar, arrayindex[64];
    int namelen, valuelen, utf8_remain, nodenum;
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
    nodetype = MDF_TYPE_UNKNOWN;
    quotechar = '\0';

    while (*pos != '\0') {
        switch (go[*(const uint8_t*)pos]) {
        case A_LOOP:
            break;
        case A_NEWLINE:
            *lineno = *lineno + 1;
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

        case A_PAIR_R_FLOAT:
            nodetype = MDF_TYPE_FLOAT;
            go = go_pair_r_number;
            break;

        case A_PAIR_R_OBJECT:
        case A_PAIR_R_ARRAY:
        {
            size_t childlen;

            mdf_init(&xnode);
            xnode->name = strndup(name, namelen);

            err = _import_json(xnode, pos, &childlen, fname, lineno);
            if (err) return merr_pass(err);
            pos += childlen;

            _mdf_append_child_node(node, xnode, nodenum);

            AFTER_NODE_APPEND();
            go = go_unpair_r;
            break;
        }

        case A_UNPAIR_R_RAW_NEWLINE:
            *lineno = *lineno + 1;
        case A_UNPAIR_R_RAW:
            if (value) valuelen = pos - value;
            else goto bad_char;

            err = _add_pair_unknowntype(node, name, value, namelen, valuelen, nodenum);
            if (err) return merr_pass(err);

            AFTER_NODE_APPEND();
            go = go_unpair_r;
            break;

        case A_UNPAIR_R_RAW_BRACE:
            if (value) valuelen = pos - value;
            else goto bad_char;

            err = _add_pair_unknowntype(node, name, value, namelen, valuelen, nodenum);
            if (err) return merr_pass(err);

            AFTER_NODE_APPEND();
            goto action_unobj;

        case A_UNPAIR_R_RAW_COMMA:
            if (value) valuelen = pos - value;
            else goto bad_char;

            err = _add_pair_unknowntype(node, name, value, namelen, valuelen, nodenum);
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

        case A_VALUE_FLOAT:
            nodetype = MDF_TYPE_FLOAT;
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

            err = _import_json(xnode, pos, &childlen, fname, lineno);
            if (err) return merr_pass(err);
            pos += childlen;

            _mdf_append_child_node(node, xnode, nodenum);

            AFTER_NODE_APPEND();
            go = go_unvalue;
        }
            break;

        case A_UNVALUE_RAW_NEWLINE:
            *lineno = *lineno + 1;
        case A_UNVALUE_RAW:
            if (value) valuelen = pos - value;
            else goto bad_char;

            err = _add_value_unknowntype(node, value, valuelen, nodenum);
            if (err) return merr_pass(err);

            AFTER_NODE_APPEND();
            go = go_unvalue;
            break;

        case A_UNVALUE_RAW_SQUARE:
            if (value) valuelen = pos - value;
            else goto bad_char;

            err = _add_value_unknowntype(node, value, valuelen, nodenum);
            if (err) return merr_pass(err);

            AFTER_NODE_APPEND();
            goto action_unarray;

        case A_UNVALUE_RAW_COMMA:
            if (value) valuelen = pos - value;
            else goto bad_char;

            err = _add_value_unknowntype(node, value, valuelen, nodenum);
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
            return merr_raise(MERR_ASSERT, "unexpect token %c in line %d of %s",
                              *pos, *lineno, fname);
        }

        pos++;
    }

    return merr_raise(MERR_ASSERT, "illgal json string");
}


/*
 * level 参数用来控制输出格式，-1为单行输出，否则会以换行和缩进控制输出
 */
static void _export_json_string(MDF *node, void *rock, MDF_PRINTF mprintf, int level)
{
    MDF_TYPE pnodetype;

#define PAD_SPACE() for (int _i = 1; _i < level; _i++) mprintf(rock, "  ");
#define NEWLINE() if (level > 0) mprintf(rock, "\n");

    if (node->parent) pnodetype = node->parent->type;
    else pnodetype = MDF_TYPE_UNKNOWN;

    level = level < 0 ? -1 : level + 1;

    while (node) {
        if (pnodetype == MDF_TYPE_OBJECT) {
            PAD_SPACE(); mprintf(rock, "\"%s\": ", node->name);
        } else if (pnodetype == MDF_TYPE_ARRAY) {
            PAD_SPACE();
        }

        switch(node->type) {
        case MDF_TYPE_OBJECT:
            mprintf(rock, "{"); NEWLINE();
            if (node->child) {
                _export_json_string(node->child, rock, mprintf, level);
            }
            PAD_SPACE(); mprintf(rock, "}");
            break;
        case MDF_TYPE_ARRAY:
            mprintf(rock, "["); NEWLINE();
            if (node->child) {
                _export_json_string(node->child, rock, mprintf, level);
            }
            PAD_SPACE(); mprintf(rock, "]");
            break;
        case MDF_TYPE_STRING:
            mprintf(rock, "\"%s\"", node->val.s);
            break;
        case MDF_TYPE_INT:
            mprintf(rock, "%ld", node->val.n);
            break;
        case MDF_TYPE_FLOAT:
            mprintf(rock, "%.5f", node->val.f);
            break;
        case MDF_TYPE_BOOL:
            if (node->val.n != 0) mprintf(rock, "true");
            else mprintf(rock, "false");
            break;
        case MDF_TYPE_NULL:
            mprintf(rock, "null");
            break;
        default:
            break;
        }

        if (node->next) mprintf(rock, ", ");

        NEWLINE();

        node = node->next;
    }
}

MERR* mdf_import_json_string(MDF *node, const char *str)
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

MERR* mdf_import_json_file(MDF *node, const char *fname)
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
        return merr_raise(MERR_ASSERT, "file size error %ld", fs.st_size);

    buf = mos_calloc(1, fs.st_size + 1);

    fp = fopen(fname, "r");
    if (!fp) return merr_raise(MERR_OPENFILE, "open %s for read failure", fname);

    if (fread(buf, 1, fs.st_size, fp) != fs.st_size) {
        fclose(fp);
        return merr_raise(MERR_ASSERT, "read file failure %ld", fs.st_size);
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

char* mdf_export_json_string(MDF *node)
{
    MSTR astr;

    if (!node) return NULL;

    mstr_init(&astr);
    _export_json_string(node, &astr, (MDF_PRINTF)mstr_appendf, -1);

    return astr.buf;
}

MERR* mdf_export_json_file(MDF *node, const char *fname)
{
    FILE *fp;

    fp = fopen(fname, "w");
    if (!fp) return merr_raise(MERR_OPENFILE, "open %s for write failure", fname);

    _export_json_string(node, fp, (MDF_PRINTF)fprintf, 0);

    fclose(fp);

    return MERR_OK;
}