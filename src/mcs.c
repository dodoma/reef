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

    uint32_t eachs[MAX_STACK_LEN]; /* 用来递归存储each的指令ID，解析到 /each 时，赋值到该指令的 pca */
    uint32_t ifs[MAX_STACK_LEN];   /* 用来递归存储if, elif的指令ID，解析到下一个elif, else时，赋值到该指令的 pcb */

    MLIST *iflist;                 /* 用来平级存储所有的if, elif, else位置，解析到 /if 时，赋值到该指令的pca */

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

    uint32_t pca;               /* 存储需要跳转的指令位置, /each, /if */
    uint32_t pcb;               /* 存储需要跳转的指令位置, elif, else */
};

#include "_mcs_token.c"
#include "_mcs_instruct.c"
#include "_mcs_eval.c"

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
                    if (eachpc->pca != 0) DIE(sme, "each instruct error");

                    eachpc->pca = pc->id;

                    sme->eachs[pos] = 0;
                    break;
                }
            }
            if (pos < 0) DIE(sme, "each mismatch");
        }
        break;
        case TOK_IF:
        {
            /* <?cs if:name == "ml" ?> */
            _emit(sme, I_IF, &pc);
            pc->u.s = tok.s;
            pc->len = tok.len;

            /* expect point to somebody */
            int pos = 0;
            for (; pos < MAX_STACK_LEN; pos++) {
                if (sme->ifs[pos] == 0) {
                    sme->ifs[pos] = pc->id;
                    break;
                }
            }
            if (pos >= MAX_STACK_LEN) DIE(sme, "stack overflow");

            mlist_append(sme->iflist, MOS_OFFSET_2_MEM(pc->id));
        }
        break;
        case TOK_ELIF:
        {
            /* <?cs elif:name == "ml" ?> */
            _emit(sme, I_ELIF, &pc);
            pc->u.s = tok.s;
            pc->len = tok.len;

            /* nearest if, elif point to me */
            int pos = MAX_STACK_LEN - 1;
            for (; pos >= 0; pos--) {
                if (sme->ifs[pos] != 0) {
                    struct instruct *ifpc = _instruct_find(sme, sme->ifs[pos]);
                    if (ifpc->pcb != 0) DIE(sme, "elif instruct error");
                    ifpc->pcb = pc->id;

                    sme->ifs[pos] = 0;
                    break;
                }
            }
            if (pos < 0) DIE(sme, "elif mismatch");

            /* expect point to somebody */
            pos = 0;
            for (; pos < MAX_STACK_LEN; pos++) {
                if (sme->ifs[pos] == 0) {
                    sme->ifs[pos] = pc->id;
                    break;
                }
            }
            if (pos >= MAX_STACK_LEN) DIE(sme, "stack overflow");

            mlist_append(sme->iflist, MOS_OFFSET_2_MEM(pc->id));
        }
        break;
        case TOK_ELSE:
        {
            /* <?cs else ?> */
            _emit(sme, I_ELSE, &pc);

            /* nearest if, elif point to me */
            int pos = MAX_STACK_LEN - 1;
            for (; pos >= 0; pos--) {
                if (sme->ifs[pos] != 0) {
                    struct instruct *ifpc = _instruct_find(sme, sme->ifs[pos]);
                    if (ifpc->pcb != 0) DIE(sme, "else instruct error");
                    ifpc->pcb = pc->id;

                    sme->ifs[pos] = 0;
                    break;
                }
            }
            if (pos < 0) DIE(sme, "if mismatch");

            /* expect point to /if */
            pos = 0;
            for (; pos < MAX_STACK_LEN; pos++) {
                if (sme->ifs[pos] == 0) {
                    sme->ifs[pos] = pc->id;
                    break;
                }
            }
            if (pos >= MAX_STACK_LEN) DIE(sme, "stack overflow");

            mlist_append(sme->iflist, MOS_OFFSET_2_MEM(pc->id));
        }
        break;
        case TOK_IF_CLOSE:
        {
            /* <?cs /if ?> */
            _emit(sme, I_IF_CLOSE, &pc);

            /* match if */
            int pos = MAX_STACK_LEN - 1;
            for (; pos >= 0; pos--) {
                if (sme->ifs[pos] != 0) {
                    sme->ifs[pos] = 0;
                    break;
                }
            }
            if (pos < 0) DIE(sme, "if mismatch");

            /* every if, elif, else point to me */
            uint32_t *id;
            MLIST_ITERATE(sme->iflist, id) {
                struct instruct *ppc = _instruct_find(sme, (uint32_t)MOS_MEM_2_OFFSET(id));
                if (ppc->pca != 0) DIE(sme, "ppc instruct error");
                ppc->pca = pc->id;
            }
            mlist_clear(sme->iflist);
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
        if (sme->ifs[i] != 0) DIE(sme, "if mismatch");
    }

    _emit(sme, I_END, NULL);
}

static MERR* _execute_program(MCS *sme, MDF *node, void *rock, MCS_PRINTF mprintf);
static MERR* _execute_piece_of_code(MCS *sme, struct instruct *pc, MDF *node,
                                    uint8_t endops[], uint8_t endoplen,
                                    void *rock, MCS_PRINTF mprintf)
{
    char shortkey[256];
    MERR *err;

    while (((char*)pc - sme->bcode.buf) < sme->bcode.len) {
        /* endop maybe I_END, I_xxx_CLOSE, I_ELSE, I_ELIF */
        for (int i = 0; i < endoplen; i++) {
            if (endops[i] == pc->op) return MERR_OK;
        }

        if (pc->op > I_LITERAL && pc->len > 0)
            snprintf(shortkey, sizeof(shortkey), "%.*s", (int)pc->len, pc->u.s);

        switch (pc->op) {
        case I_LITERAL:
            mprintf(rock, "%.*s", pc->len, pc->u.s);
            break;
        case I_VAR:
            if (mdf_get_type(node, shortkey) == MDF_TYPE_STRING)
                mprintf(rock, "%s", mdf_get_value(node, shortkey, ""));
            else {
                char *val = mdf_get_value_stringfy(node, shortkey, "");
                mprintf(rock, "%s", val);
                free(val);
            }
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
                uint8_t endops[2] = {I_EACH_CLOSE, I_END};
                err = _execute_piece_of_code(sme, pc, node, endops, 2, rock, mprintf);
                if (err != MERR_OK) return merr_pass(err);

                cnode = mdf_node_next(cnode);
            }
            mdf_remove(node, keyname);

            pc = _instruct_find(sme, lpc->pca);
        }
        break;
        case I_IF:
        case I_ELIF:
        {
            struct instruct *lpc = pc;

            Rune c;
            MSTR astr;
            mstr_init(&astr);
            struct opnode *onode = NULL;
            bool satisfied = false;

            /*
             * 1. 格式预处理 2 + 5 * 3 == 100 转换成 2+5*3==100
             */
            char *pos = shortkey;
            while (*pos != '\0') {
                int len = chartorune(&c, pos);
                if (!isspacerune(c)) mstr_appendn(&astr, pos, len);

                pos += len;
            }
            onode = _eval_expr(astr.buf, 0, NULL, node);
            if (onode) satisfied = _opnode_value_bool(onode);
            else satisfied = false;

            mos_free(onode);
            mstr_clear(&astr);

            if (satisfied == true) {
                /* if */
                pc = lpc + 1;
                uint8_t endops[4] = {I_ELIF, I_ELSE, I_IF_CLOSE, I_END};
                err = _execute_piece_of_code(sme, pc, node, endops, 4, rock, mprintf);
                if (err != MERR_OK) return merr_pass(err);

                pc = _instruct_find(sme, lpc->pca);
            } else if (lpc->pcb != 0) {
                /* elif, or else */
                pc = _instruct_find(sme, lpc->pcb);
            } else {
                /* /if */
                pc = _instruct_find(sme, lpc->pca);
            }

            continue;
        }
        break;
        case I_ELSE:
        {
            struct instruct *lpc = pc;

            pc = lpc + 1;
            uint8_t endops[2] = {I_IF_CLOSE, I_END};
            err = _execute_piece_of_code(sme, pc, node, endops, 2, rock, mprintf);
            if (err != MERR_OK) return merr_pass(err);

            /* /if */
            pc = _instruct_find(sme, lpc->pca);

            continue;
        }
        break;
        case I_IF_CLOSE:
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

    uint8_t endops[1] = {I_END};
    err = _execute_piece_of_code(sme, pc, node, endops, 1, rock, mprintf);

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
    memset(sme->ifs, 0x0, sizeof(sme->ifs));
    mlist_init(&sme->iflist, NULL);

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

void mcs_dump(MCS *tpl)
{
    if (!tpl || tpl->bcode.len <= 0) {
        printf("\n");
        return;
    }

    struct instruct *pc = (struct instruct *)tpl->bcode.buf;
    size_t len = tpl->bcode.len;
    int padnum = 0;
    size_t icount = 0;

    /*
     * *instructs* 2
     */
    while (len > 0) {
        printf("% 5zd: ", icount);
        if (pc->op == I_ELIF || pc->op == I_ELSE || pc->op == I_IF_CLOSE || pc->op == I_EACH_CLOSE) {
            if (padnum > 0) padnum--;
        }
        for (int i = 0; i < padnum; i++) printf("    ");
        switch (pc->op) {
        case I_END: puts("END"); if (padnum > 0) padnum--; break;
		case I_LITERAL: puts("LITERAL"); break;
		case I_VAR: puts("VAR"); break;
		case I_NAME: puts("NAME"); break;
		case I_INCLUDE: puts("INCLUDE"); break;
		case I_EACH: puts("EACH"); padnum++; break;
		case I_EACH_CLOSE: puts("/EACH"); break;
        case I_IF: printf("IF %.*s\n", (int)pc->len, pc->u.s); padnum++; break;
        case I_ELIF: printf("ELIF %.*s\n", (int)pc->len, pc->u.s); padnum++; break;
        case I_ELSE: printf("ELSE\n"); padnum++; break;
		case I_IF_CLOSE: puts("/IF"); break;
        }

        icount++; /* pc - (Instruct *)tpl->bcode.buf */
        pc++;
        len -= INSTRUCT_LEN;
    }
}

MERR* mcs_rend(MCS *tpl, MDF *node, const char *fname)
{
    FILE *fp;
    MERR *err;

    MERR_NOT_NULLC(tpl, node, fname);

    //MDF_TRACE_MT(node);

    if (!fname || !strcmp(fname, "-")) fp = stdout;
    else fp = fopen(fname, "w");
    if (!fp) return merr_raise(MERR_OPENFILE, "open %s for write failure", fname);

    err = _execute_program(tpl, node, fp, (MCS_PRINTF)fprintf);
    if (err != MERR_OK) return merr_pass(err);

    if (fname && strcmp(fname, "-")) fclose(fp);

    return MERR_OK;
}

void mcs_destroy(MCS **tpl)
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
    mlist_destroy(&sme->iflist);

    mos_free(*tpl);
}

double mcs_eval_numberic(char *expr)
{
    if (!expr || !*expr) return 0;

    struct opnode *onode = NULL;
    char *pos = expr;

    /*
     * 格式预处理 2 + 5 * 3 == 100 转换成 2+5*3==100
     */
    Rune c;
    MSTR astr;
    mstr_init(&astr);
    while (*pos != '\0') {
        int len = chartorune(&c, pos);
        if (!isspacerune(c)) mstr_appendn(&astr, pos, len);

        pos += len;
    }

    onode = _eval_expr(astr.buf, 0, NULL, NULL);
    double ret = onode->val.f;

    mos_free(onode);
    mstr_clear(&astr);

    return ret;
}
