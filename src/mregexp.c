#include "reef.h"

#include <setjmp.h>

#define MAX_SPLIT    10000
#define INSTRUCT_LEN (sizeof(Instruct))

enum {
    I_END,

    I_BOL, I_EOL, I_CHAR, I_ANY, I_ANYNL, I_LPAR, I_RPAR, I_CCLASS, I_NCCLASS,
    I_SPLIT, I_JUMP_ABS, I_JUMP_REL
};

typedef struct {
    uint8_t type;
    Rune c;
} Token;

typedef struct {
    uint8_t op_code;
    Rune c;                     /* character on I_CHAR */
    int32_t b;                  /* b location on I_SPLIT, JUMP... */
    MLIST *rlist;               /* character class ranges */
    uint32_t repeat;            /* repeat times */
    int32_t rrnum;
} Instruct;

typedef struct {
    Instruct *pc;
    const char *sp;
} Road;


struct _MRE {
    MBUF bcode;
    uint32_t icount;            /* instruct count */

    MLIST *cclist;              /* character class */

    const char *source;         /* Nul-terminated source code buffer */
    const char *pos;            /* Current position */
    const char *nmatch;

    Token tok;

    const char *error;
	jmp_buf kaboom;
};

static void _die(MRE *reo, const char *message)
{
    reo->error = message;
    longjmp(reo->kaboom, 1);
}

#include "_mregexp_tok.c"
#include "_mregexp_util.c"
#include "_mregexp_instruct.c"

static uint32_t _parse_cclass(MRE *reo, bool negative)
{
    uint8_t tok = TOK_EOF;
    Rune prevc = 0;
    bool firstchar = true;
    bool dash = false;
    MLIST *rlist;

    mlist_init(&rlist, NULL);

    while ((tok = _tok_next(reo, true)) != TOK_CLOSE_BRACKET && tok != TOK_EOF) {
        switch (tok) {
        case TOK_R_DICIMAL: _add_ranges_d(reo, rlist); break;
        case TOK_R_NDICIMAL: _add_ranges_D(reo, rlist); break;
        case TOK_R_WHITESPACE: _add_ranges_s(reo, rlist); break;
        case TOK_R_NWHITESPACE: _add_ranges_S(reo, rlist); break;
        case TOK_R_WORD: _add_ranges_w(reo, rlist); break;
        case TOK_R_NWORD: _add_ranges_W(reo, rlist); break;
        case TOK_CHAR:
            if (reo->tok.c == '-') {
                if (prevc == 0) {
                    if (firstchar) prevc = '-'; /* `-` in position 1*/
                    else dash = true;           /* `-` in position end */
                } else {
                    dash = true;                /* `-` in position middle */
                }
            } else {
                if (prevc == 0) prevc = reo->tok.c;
                else {
                    if (!dash) {
                        _addrange(reo, rlist, prevc, prevc);
                        prevc = reo->tok.c;
                    } else {
                        _addrange(reo, rlist, prevc, reo->tok.c);
                        dash = false;
                        prevc = 0;
                    }
                }
            }
            break;
        default: _addrange(reo, rlist, reo->tok.c, reo->tok.c); break;
            //_die(reo, "unexpect token in character class");
        }
        firstchar = false;
    }

    if (prevc != 0) _addrange(reo, rlist, prevc, prevc);
    if (dash == true) _addrange(reo, rlist, '-', '-');

    mlist_append(reo->cclist, rlist);

    return _emit_cclass(reo, rlist, negative);
}

static uint32_t _parse_repeat(MRE *reo)
{
    uint8_t tok = TOK_EOF;
    int min, max;
    bool greedy = true;

    min = max = 0;

    switch (reo->tok.c) {
    case '?': min = 0, max = 1; break;
    case '*': min = 0, max = INT_MAX; break;
    case '+': min = 1, max = INT_MAX; break;
    case '{':
    {
        int numx = 0;
        bool comma = false;
        while ((tok = _tok_next(reo, false)) == TOK_CHAR) {
            if (reo->tok.c >= '0' && reo->tok.c <= '9') {
                numx = numx * 10 + reo->tok.c - '0';
            } else if (reo->tok.c == ',') {
                comma = true;
                min = numx;
                numx = 0;
            }
        }
        if (tok != TOK_CLOSE_CURLY) _die(reo, "unmatch repeat");

        if (!comma) {
            min = max = numx;
        } else {
            if (numx > 0 && min >= numx) _die(reo, "invalid repeat range");
            if (numx == 0) max = INT_MAX;
            else max = numx;
        }
        break;
    }
    default:
        return 0;
    }

    if (max > INT_MAX) max = INT_MAX;

    if (_accept(reo, '?')) greedy = false;

    return _emit_repeat(reo, min, max, greedy);
}

static uint32_t _parse_statement(MRE *reo)
{
    uint8_t tok = TOK_EOF;
    uint32_t icount = 0;
    Instruct *pc;

    while ((tok = _tok_next(reo, false)) != TOK_OR && tok != TOK_EOF) {
        switch (tok) {
        case TOK_BOL:  icount += _emit(reo, I_BOL, NULL); break;
        case TOK_EOL:  icount += _emit(reo, I_EOL, NULL); break;
        case TOK_CHAR: icount +=  _emit(reo, I_CHAR, &pc); pc->c = reo->tok.c; break;
        case TOK_ANY:  icount += _emit(reo, I_ANY, NULL); break;
        case TOK_CCLASS:  icount += _parse_cclass(reo, false); break;
        case TOK_NCCLASS: icount += _parse_cclass(reo, true); break;
        case TOK_REPEAT:  icount += _parse_repeat(reo); break;
        default: _die(reo, "unexpect token");
        }
    }

    return icount;
}

static MERR* _compile(MRE *reo, const char *pattern)
{
    Instruct *pc;

    MERR_NOT_NULLB(reo, pattern);

    reo->pos = reo->source = pattern;

    if (setjmp(reo->kaboom)) {
        return merr_raise(MERR_ASSERT, "%s on %ld %s", reo->error, reo->pos - reo->source, reo->pos - 1);
    }

    reo->icount = _emit(reo, I_SPLIT, &pc);
    pc->b = 3;
    reo->icount += _emit(reo, I_ANYNL, NULL);
    reo->icount += _emit(reo, I_JUMP_ABS, &pc);
    pc->b = 0;
    reo->icount += _emit(reo, I_LPAR, NULL);

    uint32_t alen, blen;
    alen = _parse_statement(reo);
    while (reo->tok.type == TOK_OR) {
        blen = _parse_statement(reo);

        reo->icount += _emit_split(reo, alen, blen);

        alen += blen;
    }

    reo->icount += alen;

    reo->icount += _emit(reo, I_RPAR, NULL);
    reo->icount += _emit(reo, I_END, NULL);

    if (reo->icount * INSTRUCT_LEN != reo->bcode.len) {
        //printf("%d %d \n", reo->icount, reo->bcode.len / INSTRUCT_LEN);
        _die(reo, "instruct counter error");
    }

    return MERR_OK;
}

static void _reset_rrnum(MRE *reo)
{
    Instruct *pc = (Instruct *)reo->bcode.buf;

    for (uint32_t i = 0; i < reo->icount; i++) {
        if (pc->op_code == I_JUMP_REL) {
            pc->rrnum = pc->repeat;
        }
        pc += 1;
    }
}

static void _newroad(MRE *reo, Road *t, Instruct *pc, int32_t pos, const char *sp)
{
    Instruct *istart = (Instruct *)reo->bcode.buf;
    Instruct *iend = (Instruct *)(reo->bcode.buf + reo->bcode.len);

    t->pc = pc + pos;
    t->sp = sp;

    if (t->pc < istart || t->pc >= iend) _die(reo, "instruct overflow!!");
}

static bool _execute(MRE *reo, const char *string)
{
    Road roads[MAX_SPLIT];
    int nroad;

    Rune c;
    Instruct *pc, *reset_pc;      /* program counter */
    const char *sp = string;      /* string pointer */
    const char *bol = string;

    reo->nmatch = string;
    reset_pc = (Instruct *)reo->bcode.buf;

    _reset_rrnum(reo);

    memset(roads, 0x0, sizeof(roads));
    _newroad(reo, &roads[0], reset_pc, 0, sp);
    nroad = 1;

    /*
     * *instructs* 1
     */
    while (nroad > 0) {
        nroad--;
        pc = roads[nroad].pc;
        sp = roads[nroad].sp;
        while (1) {
            switch (pc->op_code) {
            case I_END:
                return true;
            case I_BOL:
                if (sp == bol) {
                    pc = pc + 1;
                    continue;
                }
            case I_EOL:
                if (*sp == 0) {
                    pc = pc + 1;
                    continue;
                }
                goto river;
            case I_CHAR:
                sp += chartorune(&c, sp);
                if (c == pc->c) {
                    pc = pc + 1;
                    continue;
                }
                goto river;
            case I_ANY:
                sp += chartorune(&c, sp);
                if (c && !_isnewline(c)) {
                    pc = pc + 1;
                    continue;
                }
                goto river;
            case I_ANYNL:
                sp += chartorune(&c, sp);
                if (c) {
                    pc = pc + 1;
                    continue;
                } else goto river;
            case I_LPAR:
                pc = pc + 1;
                continue;
            case I_RPAR:
                pc = pc + 1;
                continue;
            case I_CCLASS:
                sp += chartorune(&c, sp);
                if (c && _inrange(pc->rlist, c)) {
                    pc = pc + 1;
                    continue;
                } else goto river;
            case I_NCCLASS:
                sp += chartorune(&c, sp);
                if (c == 0 || _inrange(pc->rlist, c)) {
                    goto river;
                } else {
                    pc = pc + 1;
                    continue;
                }
            case I_SPLIT:
                if (nroad >= MAX_SPLIT) _die(reo, "backtrace overflow!");
                _newroad(reo, &roads[nroad++], pc, pc->b, sp);
                pc = pc + 1;
                continue;
            case I_JUMP_REL:
                if (pc->rrnum >= 0) {
                    pc->rrnum -= 1;
                    pc = _pc_relative(reo, pc, pc->b);
                } else {
                    pc = pc + 1;
                }
                continue;
            case I_JUMP_ABS:
                pc = _pc_absolute(reo, pc->b);
                continue;
            default:
                goto river;
            }
        }
    river:
        ;
    }

    /* all roads don't reach I_END */
    reo->nmatch = sp - 1;
    printf("%ld %s don't match\n", sp - string, reo->nmatch);

    return false;
}

MRE* mre_init()
{
    MRE *reo = mos_calloc(1, sizeof(MRE));

    mbuf_init(&reo->bcode, 0);
    mlist_init(&reo->cclist, mlist_free);

    reo->icount = 0;
    reo->error = NULL;
    reo->nmatch = NULL;

    return reo;
}

MERR* mre_compile(MRE *reo, const char *pattern)
{
    MERR *err;

    MERR_NOT_NULLB(reo, pattern);

    mbuf_clear(&reo->bcode);
    mlist_destroy(&reo->cclist);
    mlist_init(&reo->cclist, mlist_free);

    reo->icount = 0;
    reo->error = NULL;
    reo->nmatch = NULL;

    err = _compile(reo, pattern);
    if (err) return merr_pass(err);

    return MERR_OK;
}

void mre_dump(MRE *reo)
{
    if (!reo || reo->bcode.len <= 0) {
        printf("\n");
        return;
    }

    Instruct *pc = (Instruct *)reo->bcode.buf;
    size_t len = reo->bcode.len;
    int padnum = 0;
    size_t icount = 0;

    /*
     * *instructs* 2
     */
    while (len > 0) {
        printf("% 5ld: ", icount);
        for (int i = 0; i < padnum; i++) printf("    ");
        switch (pc->op_code) {
        case I_END: puts("end"); break;
		case I_BOL: puts("bol"); break;
		case I_EOL: puts("eol"); break;
        case I_CHAR: printf(pc->c >= 32 && pc->c < 127 ? "char '%c'\n" : "char U+%04X\n", pc->c); break;
        case I_ANY: puts("any"); break;
        case I_ANYNL: puts("anynl"); break;
        case I_LPAR: puts("lpar"); break;
        case I_RPAR: puts("rpar"); break;
        case I_CCLASS: puts("cclass"); break;
        case I_NCCLASS: puts("ncclass"); break;
        case I_SPLIT: printf("split %lu\n", pc->b + icount); padnum++; break;
        case I_JUMP_REL: printf("jump relative %lu %d\n", pc->b + icount, pc->repeat); if (padnum > 0) padnum--; break;
        case I_JUMP_ABS: printf("jump absolute %u\n", pc->b); if (padnum > 0) padnum--; break;
        }

        icount++; /* pc - (Instruct *)reo->bcode.buf */
        pc++;
        len -= INSTRUCT_LEN;
    }

}

bool mre_match(MRE *reo, const char *string)
{
    if (!reo || !string || reo->bcode.len <= 0) return false;

    return _execute(reo, string);
}

void mre_destroy(MRE **reo)
{
    if (!reo) return;

    mbuf_clear(&(*reo)->bcode);
    mlist_destroy(&(*reo)->cclist);

    mos_free(*reo);
}
