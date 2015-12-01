#include "reef.h"
#include "_mdf.h"
#include "_mdf_godef.h"

static MERR* _import_json_string(MDF *node, const char *str,
                                 size_t *xlen, const char *fname, int *lineno)
{
    MDF *xnode;
    int utf8_remain;
    char *name, *value, arrayname[64];
    int64_t valuen;
    float valuef;
    int namelen, valuelen;
    JSON_ROLE whoami;

    MDF_TYPE nodetype;
    int nodenum;
    bool in_obj_name;
    char quotechar;

    char *pos;
    const int8_t *go;

    MERR *err;

#define TOKEN_END()                                                     \
    do {                                                                \
        if (name && namelen == -1 && in_obj_name) {                     \
            namelen = pos - name;                                       \
        }                                                               \
        if (value && valuelen == -1 && !in_obj_name) {                  \
            valuelen = pos - value;                                     \
            if (whoami == R_ARRAY) {                                    \
                snprintf(arrayname, sizeof(arrayname), "%d", nodenum);  \
                name = arrayname;                                       \
                namelen = strlen(arrayname);                            \
            }                                                           \
            if (nodetype == MDF_TYPE_INT) {                             \
                char *_s = value;                                       \
                while (_s < pos) {                                      \
                    if (*_s == '.') {                                   \
                        nodetype = MDF_TYPE_FLOAT;                      \
                        break;                                          \
                    }                                                   \
                    _s++;                                               \
                }                                                       \
                _s = strndup(value, valuelen);                          \
                if (nodetype == MDF_TYPE_INT) {                         \
                    valuen = strtoll(_s, NULL, 10);                     \
                } else {                                                \
                    valuef = strtof(_s, NULL);                          \
                }                                                       \
                mos_free(_s);                                           \
            } else if (nodetype == MDF_TYPE_UNKNOWN) {                  \
                if (!strncmp("true", value, valuelen)) {                \
                    nodetype = MDF_TYPE_BOOL;                           \
                    valuen = 1;                                         \
                } else if (!strncmp("false", value, valuelen)) {        \
                    nodetype = MDF_TYPE_BOOL;                           \
                    valuen = 0;                                         \
                } else if (!strncmp("null", value, valuelen)) {         \
                    nodetype = MDF_TYPE_NULL;                           \
                }                                                       \
            }                                                           \
        }                                                               \
    } while (0)

#define AFTER_NODE_APPEND()                     \
    do {                                        \
        xnode = NULL;                           \
        name = value = NULL;                    \
        valuen = 0;                             \
        valuef = 0.0;                           \
        namelen = valuelen = -1;                \
        nodetype = MDF_TYPE_UNKNOWN;            \
        if (whoami == MDF_TYPE_OBJECT) {        \
            in_obj_name = true;                 \
        }                                       \
    } while (0)

    pos = (char*)str;
    utf8_remain = 0;
    name = value = NULL;
    valuen = 0;
    valuef = 0.0;
    namelen = valuelen = -1;
    xnode = NULL;

    whoami = R_UNKNOWN;
    go = goconstruct;

    nodetype = MDF_TYPE_UNKNOWN;
    nodenum = 0;
    in_obj_name = false;
    quotechar = '\0';

    while (*pos != '\0') {
        switch (go[*(const uint8_t*)pos]) {
        case A_LOOP:
            break;

        case A_NEWLINE:
            *lineno = *lineno + 1;
            TOKEN_END();

            go = goconstruct;
            break;

        case A_NUMBARE:
            nodetype = MDF_TYPE_INT;
            value = pos;
            valuelen = -1;

            go = gobare;
            break;

        case A_BARE:
            nodetype = MDF_TYPE_UNKNOWN;
            if (in_obj_name) {
                name = pos;
                namelen = -1;
            } else {
                value = pos;
                valuelen = -1;
            }

            go = gobare;
            break;

        case A_QUOTE:
            quotechar = *pos;

            if (in_obj_name) {
                name = pos + 1;
                namelen = -1;
            } else {
                nodetype = MDF_TYPE_STRING;
                value = pos + 1;
                valuelen = -1;
            }

            go = gostring;
            break;

        case A_UNBARE:
            TOKEN_END();
            go = goconstruct;
            break;

        case A_VALUE:
            TOKEN_END();
            in_obj_name = false;

            go = goconstruct;
            break;

        case A_OBJ:
            if (whoami == R_UNKNOWN) {
                whoami = R_OBJECT;
                node->type = MDF_TYPE_OBJECT;
                in_obj_name = true;
            } else {
                TOKEN_END();
                if (namelen < 0) goto bad_char;

                size_t childlen = 0;

                mdf_init(&xnode);
                xnode->name = strndup(name, namelen);
                err = _import_json_string(xnode, pos, &childlen, fname, lineno);
                if (err) return merr_pass(err);
                pos += childlen;

                _mdf_append_child_node(node, xnode, nodenum++);

                AFTER_NODE_APPEND();
            }
            break;

        case A_ARRAY:
            if (whoami == R_UNKNOWN) {
                whoami = R_ARRAY;
                node->type = MDF_TYPE_ARRAY;
            } else {
                TOKEN_END();
                if (namelen < 0) goto bad_char;

                size_t childlen = 0;

                mdf_init(&xnode);
                xnode->name = strndup(name, namelen);
                err = _import_json_string(xnode, pos, &childlen, fname, lineno);
                if (err) return merr_pass(err);
                pos += childlen;

                _mdf_append_child_node(node, xnode, nodenum++);

                AFTER_NODE_APPEND();
            }
            break;

        case A_PAIR:
            TOKEN_END();

            if (namelen < 0 || valuelen < 0) break;

            mdf_init(&xnode);
            xnode->type = nodetype;
            xnode->name = strndup(name, namelen);
            if (nodetype == MDF_TYPE_STRING) {
                xnode->val.s = strndup(value, valuelen);
            } else if (nodetype == MDF_TYPE_INT) {
                xnode->val.n = valuen;
            } else if (nodetype == MDF_TYPE_FLOAT) {
                xnode->val.f = valuef;
            } else if (nodetype == MDF_TYPE_BOOL) {
                xnode->val.n = valuen;
            } else if (nodetype == MDF_TYPE_NULL) {
                xnode->val.n = 0;
            }

            _mdf_append_child_node(node, xnode, nodenum++);

            AFTER_NODE_APPEND();

            go = goconstruct;
            break;

        case A_UNOBJ:
        case A_UNARRAY:
            TOKEN_END();

            if (xlen) *xlen = pos - str;

            if (namelen < 0 || valuelen < 0 ||
                name == NULL || value == NULL)
                return MERR_OK;

            mdf_init(&xnode);

            xnode->type = nodetype;
            xnode->name = strndup(name, namelen);
            if (nodetype == MDF_TYPE_STRING) {
                xnode->val.s = strndup(value, valuelen);
            } else if (nodetype == MDF_TYPE_INT) {
                xnode->val.n = valuen;
            } else if (nodetype == MDF_TYPE_FLOAT) {
                xnode->val.f = valuef;
            } else if (nodetype == MDF_TYPE_BOOL) {
                xnode->val.n = valuen;
            } else if (nodetype == MDF_TYPE_NULL) {
                xnode->val.n = 0;
            }

            _mdf_append_child_node(node, xnode, nodenum++);

            return MERR_OK;

        case A_UNQUOTE:
            if (*pos == quotechar) {
                TOKEN_END();
                go = goconstruct;
            }
            break;

        case A_ESCAPE:
            go = goescape;
            break;

        case A_UNESCAPE:
            go = gostring;
            break;

		case A_UTF8_2:
			go = goutf8_continue;
			utf8_remain = 1;
			break;

		case A_UTF8_3:
			go = goutf8_continue;
			utf8_remain = 2;
			break;

		case A_UTF8_4:
			go = goutf8_continue;
			utf8_remain = 3;
			break;

		case A_UTF_CONTINUE:
			if (!--utf8_remain) go = gostring;
			break;

        bad_char:
        case A_BAD:
            return merr_raise(MERR_ASSERT, "unexpect %s in line %d of %s",
                              pos, *lineno, fname);
        }

        pos++;
    }

    return MERR_OK;
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
            mprintf(rock, "%f", node->val.f);
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
    int lineno = 1;

    MERR_NOT_NULLA(node);

    if (!str) return MERR_OK;

    return merr_pass(_import_json_string(node, str, NULL, "<string>", &lineno));
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

    err = _import_json_string(node, buf, NULL, fname, &lineno);
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
