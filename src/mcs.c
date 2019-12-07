#include "reef.h"

#include <setjmp.h>

#define MAX_STACK_LEN 50

#define INSTRUCT_LEN (sizeof(struct instruct))

#define DIE(sme, message)                                               \
    do {                                                                \
        (sme)->error = merr_raise(MERR_ASSERT, "%s on %s:%d %s",        \
                                  (message), (sme)->fname, (sme)->lineno, (sme)->pos); \
        longjmp((sme)->kaboom, 1);                                      \
    } while (0)

#define DIER(sme, pc, message)                                          \
    do {                                                                \
        (sme)->error = merr_raise(MERR_ASSERT, "%s on %s:%d %d %.*s",   \
                                  (message), (sme)->fname, (pc)->lineno, \
                                  (pc)->op, (int)(pc)->len, (pc)->u.s); \
        longjmp((sme)->kaboomr, 1);                                     \
    } while (0)

struct _MCS {
    MBUF bcode;
    MDF *dnode;                 /* parse time dataset */
    const char *path;           /* template load path */
    uint32_t icount;            /* instruction count */

    const char *source;
    const char *pos;
    const char *fname;
    uint32_t lineno;

    uint32_t eachs[MAX_STACK_LEN]; /* 编译时的each位置 */

    MERR *error;
	jmp_buf kaboom;
	jmp_buf kaboomr;
};

struct token {
    uint8_t type;
    const char *s;
    size_t len;
};

struct instruct {
    uint32_t id;                /* 每个指令有个编号(因为mbuf可以动态变化，目前只能根据ID动态查找指令) */
    uint32_t lineno;
    uint8_t op;
    union {
        const char *s;
        const void *p;
    } u;
    size_t len;

    uint32_t pca;               /* 一般用来存储需要跳转的指令位置 */
};

#include "_mcs_token.c"
#include "_mcs_instruct.c"

static void _parse_program(MCS *sme, const char *str, size_t *xlen, const char *fname, uint32_t *lineno)
{
    char shortkey[256];
    uint8_t type = TOK_EOF;
    struct token tok;
    struct instruct *pc;
    MERR *err;

    while ((type = _tok_next(sme, &tok)) != TOK_EOF) {
        switch (type) {
        case TOK_LITERAL:
            _emit(sme, I_LITERAL, &pc);
            pc->u.s = tok.s;
            pc->len = tok.len;
            break;
        case TOK_VAR:
            /* <?cs var:Page.Name ?>  */
            /* <?cs var:url_escape(url) ?> */
            _emit(sme, I_VAR, &pc);
            pc->u.s = tok.s;
            pc->len = tok.len;
            break;
        case TOK_NAME:
            /* <?cs name:Page.Name ?> */
            _emit(sme, I_NAME, &pc);
            pc->u.s = tok.s;
            pc->len = tok.len;
            break;
        case TOK_INCLUDE:
        {
            /* <?cs include: content.cs ?> */
            /* <?cs include:$Include.content ?> */
            MCS *smex = NULL;
            char filename[PATH_MAX];
            snprintf(shortkey, sizeof(shortkey), "%.*s", (int)tok.len, tok.s);

            if (shortkey[0] != '$') snprintf(filename, sizeof(filename), "%s/%s", sme->path, shortkey);
            else {
                char *p = shortkey; p++;
                snprintf(filename, sizeof(filename), "%s/%s", sme->path, mdf_get_value(sme->dnode, p, NULL));
            }

            err = mcs_parse_file(filename, sme->path, sme->dnode, &smex);
            if (err != MERR_OK) DIE(sme, "parse include");

            _emit(sme, I_INCLUDE, &pc);
            pc->u.p = (void*)smex;
        }
        break;
        case TOK_EACH:
        {
            /* <?cs each:item = Page.Menu ?> */
            _emit(sme, I_EACH, &pc);
            pc->u.s = tok.s;
            pc->len = tok.len;

            int pos = 0;
            for (; pos < MAX_STACK_LEN; pos++) {
                if (sme->eachs[pos] == 0) {
                    sme->eachs[pos] = pc->id;
                    break;
                }
            }
            if (pos >= MAX_STACK_LEN) DIE(sme, "stack overflow");
        }
        break;
        case TOK_EACH_CLOSE:
        {
            /* <?cs /each ?> */
            _emit(sme, I_EACH_CLOSE, &pc);

            int pos = MAX_STACK_LEN - 1;
            for (; pos >= 0; pos--) {
                if (sme->eachs[pos] != 0) {
                    struct instruct *eachpc = _instruct_find(sme, sme->eachs[pos]);
                    if (eachpc->pca != 0) DIE(sme, "instruct error");

                    eachpc->pca = pc->id;

                    sme->eachs[pos] = 0;
                    break;
                }
            }
            if (pos < 0) DIE(sme, "each mismatch");
        }
        break;
        case TOK_COMMENT:
            break;
        default:
            DIE(sme, "unexpect token");
        }
    }

    for (int i = 0; i < MAX_STACK_LEN; i++) {
        if (sme->eachs[i] != 0) DIE(sme, "each mismatch");
    }

    _emit(sme, I_END, NULL);
}

static MERR* _execute_program(MCS *sme, MDF *node, void *rock, MCS_PRINTF mprintf);
static MERR* _execute_piece_of_code(MCS *sme, struct instruct *pc, MDF *node, uint8_t endop,
                                    void *rock, MCS_PRINTF mprintf)
{
    char shortkey[256];
    MERR *err;

    while (((char*)pc - sme->bcode.buf) < sme->bcode.len) {
        /* endop maybe I_END, I_xxx_CLOSE */
        if (pc->op == endop) return MERR_OK;

        if (pc->op > I_LITERAL && pc->len > 0)
            snprintf(shortkey, sizeof(shortkey), "%.*s", (int)pc->len, pc->u.s);

        switch (pc->op) {
        case I_LITERAL:
            mprintf(rock, "%.*s", pc->len, pc->u.s);
            break;
        case I_VAR:
            mprintf(rock, "%s", mdf_get_value(node, shortkey, ""));
            break;
        case I_NAME:
            mprintf(rock, "%s", mdf_get_name(node, shortkey));
            break;
        case I_INCLUDE:
            err = _execute_program((MCS*)pc->u.p, node, rock, mprintf);
            if (err != MERR_OK) return merr_pass(err);
            break;
        case I_EACH:
        {
            /* item = Page.Menu */
            char *keyname, *keyvalue;
            if (!mstr_break(shortkey, '=', &keyname, &keyvalue)) DIER(sme, pc, "illegal each");

            struct instruct *lpc = pc;
            MDF *cnode = mdf_get_child(node, keyvalue);
            while (cnode) {
                mdf_copy(node, keyname, cnode, true);
                pc = lpc + 1;
                err = _execute_piece_of_code(sme, pc, node, I_EACH_CLOSE, rock, mprintf);
                if (err != MERR_OK) return merr_pass(err);

                cnode = mdf_node_next(cnode);
            }
            mdf_remove(node, keyname);

            pc = _instruct_find(sme, lpc->pca);
        }
        break;
        default:
            DIER(sme, pc, "unknown instruct");
        }
        pc++;
    }

    return merr_raise(MERR_ASSERT, "program overflow");
}

static MERR* _execute_program(MCS *sme, MDF *node, void *rock, MCS_PRINTF mprintf)
{
    MERR *err;

    if (setjmp(sme->kaboomr)) {
        return merr_pass(sme->error);
    }

    struct instruct *pc = (struct instruct*)sme->bcode.buf;

    err = _execute_piece_of_code(sme, pc, node, I_END, rock, mprintf);

    return merr_pass(err);
}

static MERR* _parse_string(MCS *sme, const char *str, size_t *xlen, const char *fname, uint32_t *lineno, MDF *node)
{
    if (setjmp(sme->kaboom)) {
        return merr_pass(sme->error);
    }

    sme->dnode = node;

    sme->pos = sme->source = str;
    sme->fname = strdup(fname);
    sme->lineno = *lineno;
    memset(sme->eachs, 0x0, sizeof(sme->eachs));

    _parse_program(sme, str, xlen, fname, lineno);

    if (sme->icount * INSTRUCT_LEN != sme->bcode.len) {
        DIE(sme, "instruct counter error");
    }

    return MERR_OK;
}

MERR* mcs_parse_string(const char *str, const char *path, MDF *node, MCS **tpl)
{
    uint32_t lineno;
    MERR *err;

    MERR_NOT_NULLB(tpl, str);
    *tpl = NULL;

    lineno = 1;

    MCS *ltpl = mos_calloc(1, sizeof(MCS));
    mbuf_init(&ltpl->bcode, 0);
    if (!path) ltpl->path = "./";
    else ltpl->path = path;
    ltpl->icount = 0;
    ltpl->error = MERR_OK;

    err = _parse_string(ltpl, str, NULL, "<string>", &lineno, node);
    if (err) return merr_pass(err);

    *tpl = ltpl;

    return MERR_OK;
}

MERR* mcs_parse_file(const char *fname, const char *path, MDF *node, MCS **tpl)
{
    struct stat fs;
    uint32_t lineno;
    char *buf;
    FILE *fp;
    MERR *err;

    MERR_NOT_NULLB(tpl, fname);
    *tpl = NULL;

    lineno = 1;

    if (stat(fname, &fs) == -1)
        return merr_raise(MERR_OPENFILE, "stat %s failure", fname);

    if (fs.st_size < 0 || fs.st_size > INT32_MAX)
        return merr_raise(MERR_ASSERT, "file size error %ld", (long int)fs.st_size);

    /* buf freed in mcs_destroy */
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

    MCS *ltpl = mos_calloc(1, sizeof(MCS));
    mbuf_init(&ltpl->bcode, 0);
    if (!path) ltpl->path = "./";
    else ltpl->path = path;
    ltpl->icount = 0;
    ltpl->error = MERR_OK;

    err = _parse_string(ltpl, buf, NULL, fname, &lineno, node);
    if (err != MERR_OK) return merr_pass(err);

    *tpl = ltpl;

    return MERR_OK;
}

MERR* mcs_rend(MCS *tpl, MDF *node, const char *fname)
{
    FILE *fp;
    MERR *err;

    MERR_NOT_NULLC(tpl, node, fname);

    MDF_TRACE_MT(node);

    if (!fname || !strcmp(fname, "-")) fp = stdout;
    else fp = fopen(fname, "w");
    if (!fp) return merr_raise(MERR_OPENFILE, "open %s for write failure", fname);

    err = _execute_program(tpl, node, fp, (MCS_PRINTF)fprintf);
    if (err != MERR_OK) return merr_pass(err);

    if (fname && strcmp(fname, "-")) fclose(fp);

    return MERR_OK;
}

void  mcs_destroy(MCS **tpl)
{
    if (!tpl || !*tpl) return;

    MCS *sme = *tpl;
    struct instruct *pc = (struct instruct*)sme->bcode.buf;
    while (pc->op != I_END) {
        if (pc->op == I_INCLUDE) {
            mcs_destroy((MCS **)&(pc->u.p));
        }
        pc++;
    }

    mbuf_clear(&sme->bcode);
    if (strcmp(sme->fname, "<string>")) mos_free(sme->source);
    mos_free(sme->fname);

    mos_free(*tpl);
}
