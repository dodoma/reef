#include "reef.h"
#include "_mdf.h"

#include <setjmp.h>

#define XML_OPEN     1              /* < */
#define XML_CLOSE    2              /* > */
#define XML_SLASH    4              /* / */
#define XML_SPACE    8              /*   */
#define XML_EQUAL    16             /* = */
#define XML_QUOTE    32             /* " ' */
#define XML_END      64             /* </...> */
#define XML_CDATA_S  128            /* <![CDATA[...]] */
#define XML_CDATA_E  256            /* ]]> */
#define XML_IGNORE_S 512            /* <?...?> */
#define XML_IGNORE_E 1024           /* <?...?> */
#define XML_RAW      2048            /* this is page 1 */
#define XML_EOF      4096
#define XML_ALL      0xFFFFFFFF

jmp_buf kaboom;

static int __tok_current(char *pos, int *lineno)
{
    char c = *pos;
    switch (c) {
    case '<':
        if (*(pos+1) == '/') return XML_END;
        else if (*(pos+1) == '?') return XML_IGNORE_S;
        else if (!strncmp(pos, "<![CDATA[", 9)) return XML_CDATA_S;
        else return XML_OPEN;

    case '>':
        return XML_CLOSE;
    case '/':
        return XML_SLASH;

    case 0x0A:
        *lineno = *lineno + 1;
    case 0x20:
    case '\t':
    case 0x0D:
        return XML_SPACE;

    case '=':
        return XML_EQUAL;
    case '"':
    case '\'':
        return XML_QUOTE;

    case ']':
        if (!strncmp(pos, "]]>", 3)) return XML_CDATA_E;
        else return XML_RAW;

    case '?':
        if (*(pos+1) == '>') return XML_IGNORE_E;
        else return XML_RAW;

    case 0:
        return XML_EOF;
    default:
        return XML_RAW;
    }
}

/* 返回当前指向的token类型，除了 raw 外，移动 pos 指向下一个token */
static int __tok_next(char **pos, int *lineno)
{
    int type = __tok_current(*pos, lineno);

    switch (type) {
    case XML_RAW:
        break;
    case XML_END:               /* </...> */
        *pos += 2;
        break;
    case XML_CDATA_S:           /* <![CDATA[...]]> */
        *pos += 9;
        break;
    case XML_CDATA_E:           /* <![CDATA[...]]> */
        *pos += 3;
        break;
    case XML_IGNORE_S:          /* <?...?> */
        *pos += 2;
        break;
    case XML_IGNORE_E:
        *pos += 2;
        break;
    case XML_EOF:
        *pos += 1;
        break;
    default:
        *pos += 1;
    }

    return type;
}

static int __tok_len(int type, char *pos)
{
    switch (type) {
    case XML_RAW:
    {
        Rune c;
        return chartorune(&c, pos);
    }
    case XML_END:
        return 2;
    case XML_CDATA_S:
        return 9;
    case XML_CDATA_E:
        return 3;
    case XML_IGNORE_S:
        return 2;
    case XML_IGNORE_E:
        return 2;
    case XML_EOF:
        return 1;
    default:
        return 1;
    }
}

/* 忽略接下来的 mask 标签 */
static char* __skip(char *pos, int mask, int *lineno)
{
    int type;

    while ((type = __tok_current(pos, lineno)) &&
           type != XML_EOF &&
           type & mask) {
        pos += __tok_len(type, pos);
    }

    return pos;
}

/* 查找接下来的 mask 标签 */
static char* __expect(char *pos, int mask, int *rmask, bool eat, int *lineno)
{
    int type;

    while ((type = __tok_current(pos, lineno))) {
        if (type & mask) {
            /* 找到了该标签 */
            if (eat) pos += __tok_len(type, pos);
            if (rmask) *rmask = type;

            return pos;
        } else {
            pos += __tok_len(type, pos);
        }
    }

    /* 找不到期待的标签 */
    longjmp(kaboom, mask);
}

/* TODO 属性必须是 <name attrx="foo" attry="bar"> 的形式 */
static char* _import_attribute(MDF *xnode, char *pos, int *lineno)
{
    char *p;
    int type = XML_EOF;
    MSTR key; mstr_init(&key);
    MSTR val; mstr_init(&val);

    pos = __skip(pos, XML_SPACE, lineno);

    /* 判断当前token是不是属性，如果不是，直接返回调用者，继续从当前位置判断 */
    while ((type = __tok_current(pos, lineno)) == XML_RAW) {
        p = pos;
        pos = __expect(pos, XML_EQUAL, NULL, false, lineno);
        mstr_appendf(&key, "@%.*s", (int)(pos - p), p);

        pos = __expect(pos, XML_QUOTE, NULL, true, lineno);
        p = pos;
        pos = __expect(pos, XML_QUOTE, NULL, false, lineno);
        mstr_appendf(&val, "%.*s", (int)(pos - p), p);
        pos += __tok_len(XML_QUOTE, pos);

        pos = __skip(pos, XML_SPACE, lineno);

        mdf_set_value(xnode, key.buf, val.buf);

        mstr_clear(&key);
        mstr_clear(&val);
    }

    return pos;
}

static MERR* _import_xml(MDF *node, const char *str, size_t *xlen, const char *fname, int *lineno)
{
    char *pos, *p;
    int type = XML_EOF;
    MDF *xnode;
    MSTR key; mstr_init(&key);
    MSTR val; mstr_init(&val);
    MERR *err;

    pos = (char*)str;
    pos = __skip(pos, XML_SPACE, lineno);

    int rv = setjmp(kaboom);
    if (rv > 0) {
        return merr_raise(MERR_ASSERT, "找不到期待字符 %d '%c' %d of %s", rv, *pos, *lineno, fname);
    }

    while ((type = __tok_next(&pos, lineno)) != XML_EOF) {
        switch (type) {
        case XML_OPEN:
            /* #text */
            if (node->type == MDF_TYPE_STRING && node->val.s) {
                char *tmps = strdup(node->val.s);
                mdf_set_value(node, "#text", tmps);
                mos_free(tmps);
            }

            p = pos = __skip(pos, XML_SPACE, lineno);
            pos = __expect(pos, XML_ALL ^ XML_RAW, NULL, false, lineno);

            mstr_clear(&key);
            mstr_appendf(&key, "%.*s", (int)(pos - p), p);

            if (!mdf_path_exist(node, key.buf)) {
                /* 第一次赋值 */
                xnode = mdf_get_or_create_node(node, key.buf);
            } else {
                if (mdf_get_type(node, key.buf) == MDF_TYPE_STRING) {
                    /* 第二次赋值，变成数组后追加 */
                    char *val = mdf_get_value_copy(node, key.buf, "");
                    mdf_set_type(node, key.buf, MDF_TYPE_ARRAY);
                    mdf_set_valuef(node, "%s.0=%s", key.buf, val);
                    mos_free(val);
                }

                xnode = mdf_insert_node(node, key.buf, -1);
            }

            pos = _import_attribute(xnode, pos, lineno);

            pos = __skip(pos, XML_SPACE, lineno);
            type = __tok_next(&pos, lineno);
            if (type == XML_CLOSE) {
                pos = __skip(pos, XML_SPACE, lineno);
                size_t llen = 0;
                err = _import_xml(xnode, pos, &llen, fname, lineno);
                if (err != MERR_OK) return merr_pass(err);

                pos += llen;
            } else if (type == XML_SLASH) {
                pos = __expect(pos, XML_CLOSE, NULL, true, lineno);
                pos = __skip(pos, XML_SPACE, lineno);
            } else {
                goto error;
            }
            break;

        case XML_CDATA_S:
            mstr_clear(&val);
            p = pos;
            pos = __expect(pos, XML_CDATA_E, NULL, false, lineno);
            mstr_appendn(&val, p, pos - p);
            mdf_set_value(node, NULL, val.buf);

            pos += __tok_len(XML_CDATA_E, pos);
            break;

        case XML_IGNORE_S:
            pos = __expect(pos, XML_IGNORE_E, NULL, true, lineno);
            break;

        case XML_SPACE:
            break;

        case XML_RAW:
            mstr_clear(&val);
            p = pos;
            pos = __expect(pos, XML_OPEN | XML_END | XML_EOF, &type, false, lineno);
            if (type == XML_OPEN || type == XML_END) {
                /* 忽略 raw data 后面空格部分 */
                size_t len = pos - p;
                while (len > 0 && isspace(*(p + len - 1))) len--;
                mstr_appendn(&val, p, len);
                mdf_set_value(node, NULL, val.buf);
            }
            break;

        case XML_END:
            pos = __expect(pos, XML_CLOSE, NULL, true, lineno);
            goto done;

        default:
            goto error;
        }
    }

done:
    mstr_clear(&key);
    mstr_clear(&val);
    pos = __skip(pos, XML_SPACE, lineno);
    if (xlen) *xlen = pos - str;
    return MERR_OK;

error:
    mstr_clear(&key);
    mstr_clear(&val);
    return merr_raise(MERR_ASSERT, "不被期待的字符 '%c' in line %d of %s", *pos, *lineno, fname);
}

struct xml_outbuf {
    char *buf;
    size_t len;
    size_t max;
};

static void _xml_outbuf_appendf(void *rock, const char *fmt, ...)
{
    va_list ap;
    int len, remain;

    struct xml_outbuf *jbuf = rock;

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
static void _export_xml(MDF *node, void *rock, MDF_PRINTF mprintf, int level, char *pkey, bool root)
{
    MDF *cnode;

#define PAD_SPACE() for (int _i = 0; _i < level; _i++) mprintf(rock, "  ");
#define NEWLINE() if (level >= 0) mprintf(rock, "\n");

    char *key = pkey ? pkey : mdf_get_name(node, NULL);

    switch(node->type) {
    case MDF_TYPE_STRING:
    {
        char *val = mdf_get_value(node, NULL, NULL);
        PAD_SPACE(); mprintf(rock, "<%s>%s</%s>", key, val, key); NEWLINE();
        break;
    }
    case MDF_TYPE_ARRAY:
        level = level < 0 ? -1 : level + 1;
        cnode = node->child;
        while (cnode) {
            _export_xml(cnode, rock, mprintf, level, mdf_get_name(node, NULL), false);

            cnode = cnode->next;
        }
        level = level < 0 ? -1: level - 1;
        break;
    case MDF_TYPE_OBJECT:
        if (!root) {
            PAD_SPACE(); mprintf(rock, "<%s", key);
            level = level < 0 ? -1 : level + 1;

            /* @属性 */
            cnode = node->child;
            while (cnode) {
                char *key = mdf_get_name(cnode, NULL);
                if (*key == '@') {
                    mprintf(rock, " %s=\"%s\"", key+1, mdf_get_value(cnode, NULL, ""));
                }

                cnode = cnode->next;
            }
            mprintf(rock, ">"); NEWLINE();
        }

        /* #text */
        cnode = node->child;
        while (cnode) {
            char *key = mdf_get_name(cnode, NULL);
            if (!strcmp(key, "#text")) {
                mprintf(rock, "%s", mdf_get_value(cnode, NULL, "")); NEWLINE();
            }

            cnode = cnode->next;
        }

        /* childs */
        cnode = node->child;
        while (cnode) {
            char *key = mdf_get_name(cnode, NULL);
            if (*key != '@' && *key != '#') _export_xml(cnode, rock, mprintf, level, NULL, false);

            cnode = cnode->next;
        }

        if (!root) {
            level = level < 0 ? -1: level - 1;
            PAD_SPACE(); mprintf(rock, "</%s>", key); NEWLINE();
        }
        break;
    default:
    {
        char *val = mdf_get_value_stringfy(node, NULL, "");
        PAD_SPACE(); mprintf(rock, "<%s>%s</%s>", key, val, key); NEWLINE();
        mos_free(val);
        break;
    }
    }
}

MERR* mdf_xml_import_string(MDF *node, const char *str)
{
    int lineno;
    MERR *err;

    MERR_NOT_NULLA(node);

    if (!str) return MERR_OK;

    lineno = 1;

    err = _import_xml(node, str, NULL, "<string>", &lineno);
    if (err) return merr_pass(err);

    return MERR_OK;
}

MERR* mdf_xml_import_file(MDF *node, const char *fname)
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

    err = _import_xml(node, buf, NULL, fname, &lineno);
    if (err) {
        mos_free(buf);
        return merr_pass(err);
    }

    mos_free(buf);

    return MERR_OK;
}

char* mdf_xml_export_string(MDF *node)
{
    MSTR astr;

    if (!node) return NULL;

    mstr_init(&astr);
    _export_xml(node, &astr, (MDF_PRINTF)mstr_appendf, -1, NULL, true);

    return astr.buf;
}

char* mdf_xml_export_string_pretty(MDF *node)
{
    MSTR astr;

    if (!node) return NULL;

    mstr_init(&astr);
    _export_xml(node, &astr, (MDF_PRINTF)mstr_appendf, 0, NULL, true);

    return astr.buf;
}

size_t mdf_xml_export_buffer(MDF *node, char *buf, size_t len)
{
    struct xml_outbuf jbuf;

    if (!node) return 0;

    memset(buf, 0x0, len);

    jbuf.buf = buf;
    jbuf.max = len;
    jbuf.len = 0;

    _export_xml(node, &jbuf, (MDF_PRINTF)_xml_outbuf_appendf, -1, NULL, true);

    if (jbuf.len > jbuf.max) return 0;

    return jbuf.len;
}

MERR* mdf_xml_export_file(MDF *node, const char *fname)
{
    FILE *fp;

    if (!node) return MERR_OK;

    if (!fname || !strcmp(fname, "-")) fp = stdout;
    else fp = fopen(fname, "w");
    if (!fp) return merr_raise(MERR_OPENFILE, "open %s for write failure", fname);

    _export_xml(node, fp, (MDF_PRINTF)fprintf, 0, NULL, true);

    if (fname && strcmp(fname, "-")) fclose(fp);

    return MERR_OK;
}
