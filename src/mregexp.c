#include "reef.h"

#include <setjmp.h>

#define MAX_SPLIT    10000
#define INSTRUCT_LEN (sizeof(Instruct))

#define DIE(reo, message)                                               \
    do {                                                                \
        (reo)->error = merr_raise(MERR_ASSERT, "%s on %d %s",           \
                                  (message), (int)((reo)->pos - (reo)->source), (reo)->pos - 1); \
        longjmp((reo)->kaboom, 1);                                      \
    } while (0)

enum {
    I_END,

    I_BOL, I_EOL, I_CHAR, I_ANY, I_ANYNL = 5,
    I_LPAR, I_RPAR, I_CCLASS, I_NCCLASS, I_SPLIT = 10,
    I_JUMP_ABS, I_JUMP_REL, I_REF, I_PLA, I_NLA = 15,
    I_WORD, I_NWORD, I_SPLITO
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
    uint32_t unum;              /* repeat times, or, parenthese serial number */
    int32_t rrnum;
} Instruct;

typedef struct {
    Instruct *pc;
    const char *sp;
} Road;

typedef struct {
    const char *sp;
    const char *ep;
} Resub;


struct _MRE {
    MBUF bcode;
    uint32_t icount;            /* instruct count */
    uint32_t nsub;              /* parenthese count */

    MLIST *cclist;              /* character class */
    MLIST *sublist;

    const char *source;         /* Nul-terminated source code buffer */
    const char *pos;            /* Current source position */
    const char *pend;           /* Current match end */

    Token tok;

    MERR* error;
	jmp_buf kaboom;
};

#include "_mregexp_tok.c"
#include "_mregexp_util.c"
#include "_mregexp_instruct.c"

static uint32_t _parse_statement(MRE *reo);

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
        case TOK_R_CJK: _add_ranges_cjk(reo, rlist); break;
        case TOK_R_NCJK: _add_ranges_CJK(reo, rlist); break;
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
            //DIE(reo, "unexpect token in character class");
        }
        firstchar = false;
    }

    if (prevc != 0) _addrange(reo, rlist, prevc, prevc);
    if (dash == true) _addrange(reo, rlist, '-', '-');

    mlist_append(reo->cclist, rlist);

    return _emit_cclass(reo, rlist, negative);
}

static uint32_t _parse_repeat(MRE *reo, uint32_t lastcount)
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
            } else DIE(reo, "unknown repeat");
        }
        if (tok != TOK_CLOSE_CURLY) DIE(reo, "unmatch repeat");

        if (!comma) {
            min = max = numx;
        } else {
            if (numx > 0 && min >= numx) DIE(reo, "invalid repeat range");
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

    return _emit_repeat(reo, min, max, lastcount, greedy);
}

static uint32_t _parse_lpar(MRE *reo, uint8_t tok)
{
    Instruct *pc;
    uint32_t icount = 0;
    uint32_t pos = 0;

    switch (tok) {
    case TOK_OPEN_PAREN:
        pos = _instruct_count(reo);
        icount = _emit(reo, I_LPAR, &pc);
        pc->unum = reo->nsub++;
        break;
    case TOK_NC:
        icount = _emit(reo, I_LPAR, NULL);
        break;
    case TOK_PLA:
        pos = _instruct_count(reo);
        icount = _emit(reo, I_PLA, &pc);
        break;
    case TOK_NLA:
        pos = _instruct_count(reo);
        icount = _emit(reo, I_NLA, &pc);
        break;
    default:
        DIE(reo, "tok type error");
    }

    uint32_t alen, blen;
    alen = _parse_statement(reo);
    while (reo->tok.type == TOK_OR) {
        blen = _parse_statement(reo);

        _emit_split(reo, alen, blen);

        alen += blen;
        alen += 2;
    }

    if (reo->tok.type != TOK_CLOSE_PAREN) DIE(reo, "unmatch `)`");

    icount += alen;

    switch (tok) {
    case TOK_OPEN_PAREN:
    {
        Instruct *oldpc = _pc_absolute(reo, pos);
        icount += _emit(reo, I_RPAR, &pc);
        pc->unum = oldpc->unum;
        break;
    }
    case TOK_NC:
        icount += _emit(reo, I_RPAR, NULL);
        break;
    case TOK_PLA:
    case TOK_NLA:
        icount += _emit(reo, I_END, NULL);
        pc = _pc_absolute(reo, pos);
        pc->b = alen + 2;
        break;
    }

    return icount;
}

static uint32_t _parse_statement(MRE *reo)
{
    uint8_t tok = TOK_EOF;
    uint32_t icount = 0, lastcount = 0;
    Instruct *pc;
    MLIST *rlist;

    while ((tok = _tok_next(reo, false)) != TOK_OR && tok != TOK_EOF && tok != TOK_CLOSE_PAREN) {
        mlist_init(&rlist, NULL);
        switch (tok) {
        case TOK_BOL:  lastcount = _emit(reo, I_BOL, NULL); break;
        case TOK_EOL:  lastcount = _emit(reo, I_EOL, NULL); break;
        case TOK_CHAR: lastcount =  _emit(reo, I_CHAR, &pc); pc->c = reo->tok.c; break;
        case TOK_ANY:  lastcount = _emit(reo, I_ANY, NULL); break;

        case TOK_R_DICIMAL: _add_ranges_d(reo, rlist); break;
        case TOK_R_NDICIMAL: _add_ranges_D(reo, rlist); break;
        case TOK_R_WHITESPACE: _add_ranges_s(reo, rlist); break;
        case TOK_R_NWHITESPACE: _add_ranges_S(reo, rlist); break;
        case TOK_R_WORD: _add_ranges_w(reo, rlist); break;
        case TOK_R_NWORD: _add_ranges_W(reo, rlist); break;
        case TOK_R_CJK: _add_ranges_cjk(reo, rlist); break;
        case TOK_R_NCJK: _add_ranges_CJK(reo, rlist); break;

        case TOK_CCLASS:  lastcount = _parse_cclass(reo, false); break;
        case TOK_NCCLASS: lastcount = _parse_cclass(reo, true); break;
        case TOK_REPEAT:  lastcount = _parse_repeat(reo, lastcount); break;
        case TOK_REF: lastcount = _emit(reo, I_REF, &pc); pc->unum = reo->tok.c; break;
        case TOK_WORD: lastcount = _emit(reo, I_WORD, NULL); break;
        case TOK_NWORD: lastcount = _emit(reo, I_NWORD, NULL); break;
        case TOK_OPEN_PAREN:
        case TOK_NC:
        case TOK_PLA:
        case TOK_NLA:
            lastcount = _parse_lpar(reo, tok);
            break;
        default: DIE(reo, "unexpect token");
        }

        if (mlist_length(rlist) > 0) {
            mlist_append(reo->cclist, rlist);
            lastcount = _emit_cclass(reo, rlist, false);
        } else mlist_destroy(&rlist);

        icount += lastcount;
    }

    return icount;
}

/*
 * "Win(?!95|98|NT).*in"
 * =====================
 *   0: split 27
 *   1:     lpar 0
 *   2:         char 'W'
 *   3:         char 'i'
 *   4:         char 'n'
 *   5:         nla 17
 *   6:             split 14
 *   7:                 split 11
 *   8:                     char '9'
 *   9:                     char '5'
 *  10:                     jump relative 13 2147483647
 *  11:                 char '9'
 *  12:                 char '8'
 *  13:                 jump relative 16 2147483647
 *  14:             char 'N'
 *  15:             char 'T'
 *  16:             end
 *  17:         split 22
 *  18:             any
 *  19:             split 22
 *  20:                 any
 *  21:                 jump relative 19 2147483645
 *  22:             jump relative 23 0
 *  23:         char 'i'
 *  24:         char 'n'
 *  25:     rpar 0
 *  26:     end
 *  27: anynl
 *  28: jump absolute 0
 */
static MERR* _compile(MRE *reo, const char *pattern)
{
    Instruct *pc;

    MERR_NOT_NULLB(reo, pattern);

    reo->pos = reo->source = pattern;

    if (setjmp(reo->kaboom)) {
        return merr_pass(reo->error);
    }

    reo->icount = _emit(reo, I_SPLIT, NULL);
    reo->icount += _emit(reo, I_LPAR, NULL);

    uint32_t alen, blen;
    alen = _parse_statement(reo);
    while (reo->tok.type == TOK_OR) {
        blen = _parse_statement(reo);

        _emit_split(reo, alen, blen);

        alen += blen;
        alen += 2;
    }

    reo->icount += alen;

    reo->icount += _emit(reo, I_RPAR, NULL);
    reo->icount += _emit(reo, I_END, NULL);
    pc = _pc_absolute(reo, 0);
    pc->b = reo->icount;

    reo->icount += _emit(reo, I_ANYNL, NULL);
    reo->icount += _emit(reo, I_JUMP_ABS, &pc);
    pc->b = 0;

    if (reo->icount * INSTRUCT_LEN != reo->bcode.len) {
        //printf("%d %d \n", reo->icount, reo->bcode.len / INSTRUCT_LEN);
        DIE(reo, "instruct counter error");
    }

    return MERR_OK;
}

static bool _sub_get(MLIST *slist, uint32_t index, const char **sp, const char **ep)
{
    Resub *sub;
    mlist_get(slist, index, (void**)&sub);

    if (sub->ep <= sub->sp) return false;

    *sp = sub->sp;
    *ep = sub->ep;

    return true;
}

static void _reset_rrnum(MRE *reo)
{
    Instruct *pc = (Instruct *)reo->bcode.buf;

    for (uint32_t i = 0; i < reo->icount; i++) {
        if (pc->op_code == I_JUMP_REL) {
            pc->rrnum = pc->unum;
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

    if (t->pc < istart || t->pc >= iend) DIE(reo, "instruct overflow!!");
}

static bool _execute(MRE *reo, Instruct *start_pc, const char *string, bool igcase)
{
    Road roads[MAX_SPLIT];
    int nroad;

    Rune c;
    Instruct *pc, *pc0;           /* program counter */
    const char *sp = string;      /* string pointer */
    const char *bol = string;
    MLIST *slist;

    reo->pend = string;
    mlist_init(&slist, free);

    if (!start_pc) pc0 = (Instruct *)reo->bcode.buf;
    else pc0 = start_pc;

    if (!start_pc) _reset_rrnum(reo);

    memset(roads, 0x0, sizeof(roads));
    _newroad(reo, &roads[0], pc0, 0, sp);
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
                reo->pend = sp;
                mlist_append(reo->sublist, slist);
                return true;
            case I_BOL:
                if (sp == bol) {
                    pc = pc + 1;
                    continue;
                } else if (sp > bol && _isnewline(sp[-1])) {
                    pc = pc + 1;
                    continue;
                }
                goto river;
            case I_EOL:
                if (*sp == 0) {
                    pc = pc + 1;
                    continue;
                } else if (_isnewline(*sp)) {
                    sp += 1;
                    pc = pc + 1;
                    continue;
                }
                goto river;
            case I_CHAR:
            {
                Rune d;
                sp += chartorune(&c, sp);
                if (igcase) {
                    c = _canon(c);
                    d = _canon(pc->c);
                } else d = pc->c;

                if (c == d) {
                    pc = pc + 1;
                    continue;
                }
                goto river;
            }
            case I_ANY:
                sp += chartorune(&c, sp);
                if (c && !_isnewline(c)) {
                    pc = pc + 1;
                    continue;
                } else goto river;
            case I_ANYNL:
                sp += chartorune(&c, sp);
                if (c) {
                    pc = pc + 1;
                    continue;
                } else goto river;
            case I_LPAR:
            {
                int len = mlist_length(slist);
                if (len > pc->unum) {
                    Resub *sub;
                    mlist_get(slist, pc->unum, (void**)&sub);
                    if (!sub) {
                        sub = mos_calloc(1, sizeof(Resub));
                        mlist_set(slist, pc->unum, sub);
                    }
                    sub->sp = sp;
                    sub->ep = NULL;
                    goto lpardone;
                } else if (len < pc->unum) {
                    for (int i = len; i < pc->unum; i++) {
                        mlist_append(slist, NULL);
                    }
                }
                Resub *sub = mos_calloc(1, sizeof(Resub));
                sub->sp = sp;
                sub->ep = NULL;
                mlist_append(slist, (void*)sub);

            lpardone:
                pc = pc + 1;
                continue;
            }
            case I_RPAR:
            {
                Resub *sub;
                MERR *err;
                err = mlist_get(slist, pc->unum, (void**)&sub);
                JUMP_NOK(err, river);
                sub->ep = sp;

                pc = pc + 1;
                continue;
            }
            case I_CCLASS:
                sp += chartorune(&c, sp);
                if (c && _inrange(pc->rlist, c, igcase)) {
                    pc = pc + 1;
                    continue;
                } else goto river;
            case I_NCCLASS:
                sp += chartorune(&c, sp);
                if (c == 0 || _inrange(pc->rlist, c, igcase)) {
                    goto river;
                } else {
                    pc = pc + 1;
                    continue;
                }
            case I_SPLIT:
                if (nroad >= MAX_SPLIT) DIE(reo, "backtrace overflow!");
                _newroad(reo, &roads[nroad++], pc, pc->b, sp);
                pc = pc + 1;
                continue;
            case I_SPLITO:
                if (nroad >= MAX_SPLIT) DIE(reo, "backtrace overflow!");
                _newroad(reo, &roads[nroad++], pc, 1, sp);
                pc = pc + pc->b;
                continue;
            case I_JUMP_REL:
                if (pc->rrnum >= 0) {
                    if (*sp == 0 && pc->b < 0) {
                        /* don't REPEAT_BACK on EOL */
                        pc = pc + 1;
                        continue;
                    }
                    pc->rrnum -= 1;
                    pc = _pc_relative(reo, pc, pc->b);
                } else {
                    pc = pc + 1;
                }
                continue;
            case I_JUMP_ABS:
                pc = _pc_absolute(reo, pc->b);
                continue;
            case I_REF:
            {
                const char *ps, *pe;

                if (!_sub_get(slist, pc->unum, &ps, &pe)) goto river;

                int i = pe - ps;
                if (i > 0) {
                    if (_strcompare(sp, ps, i, igcase)) goto river;
                    sp += i;
                    pc = pc + 1;
                    continue;
                } else goto river;
            }
            case I_PLA:
                if (!_execute(reo, pc + 1, sp, igcase)) {
                    goto river;
                } else {
                    pc = _pc_relative(reo, pc, pc->b);
                    continue;
                }
            case I_NLA:
                if (_execute(reo, pc + 1, sp, igcase)) {
                    goto river;
                } else {
                    pc = _pc_relative(reo, pc, pc->b);
                    continue;
                }
            case I_WORD:
            {
                int i = sp > bol && _iswordchar(sp[-1]);
                i ^= _iswordchar(sp[0]);
                if (!i) goto river;
                pc = pc + 1;
                continue;
            }
            case I_NWORD:
            {
                int i = sp > bol && _iswordchar(sp[-1]);
                i ^= _iswordchar(sp[0]);
                if (i) goto river;
                pc = pc + 1;
                continue;
            }
            default:
                goto river;
            }
        }

    river:
        if (!start_pc) _reset_rrnum(reo);
    }

    /* all roads don't reach I_END */
    //reo->nmatch = sp - 1;
    //printf("%ld %s don't match\n", sp - string, reo->nmatch);

    mlist_destroy(&slist);
    return false;
}

MRE* mre_init()
{
    MRE *reo = mos_calloc(1, sizeof(MRE));

    mbuf_init(&reo->bcode, 0);
    mlist_init(&reo->cclist, mlist_free);
    mlist_init(&reo->sublist, mlist_free);

    reo->icount = 0;
    reo->nsub = 1;
    merr_destroy(&reo->error);

    return reo;
}

MERR* mre_compile(MRE *reo, const char *pattern)
{
    MERR *err;

    MERR_NOT_NULLB(reo, pattern);

    mbuf_clear(&reo->bcode);
    mlist_clear(reo->cclist);
    mlist_clear(reo->sublist);

    reo->icount = 0;
    reo->nsub = 1;
    merr_destroy(&reo->error);

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
        printf("% 5zd: ", icount);
        if (pc->op_code == I_RPAR) padnum--;
        for (int i = 0; i < padnum; i++) printf("    ");
        switch (pc->op_code) {
        case I_END: puts("end"); if (padnum > 0) padnum--; break;
		case I_BOL: puts("bol"); break;
		case I_EOL: puts("eol"); break;
        case I_CHAR: printf(pc->c >= 32 && pc->c < 127 ? "char '%c'\n" : "char U+%04X\n", pc->c); break;
        case I_ANY: puts("any"); break;
        case I_ANYNL: puts("anynl"); break;
        case I_LPAR: printf("lpar %d\n", pc->unum); padnum++; break;
        case I_RPAR: printf("rpar %d\n", pc->unum); break;
        case I_CCLASS: puts("cclass"); break;
        case I_NCCLASS: puts("ncclass"); break;
        case I_SPLIT: printf("split %zu\n", pc->b + icount); padnum++; break;
        case I_SPLITO: printf("splito %zu\n", pc->b + icount); padnum++; break;
        case I_JUMP_REL: printf("jump relative %zu %d\n", pc->b + icount, pc->unum); if (padnum > 0) padnum--; break;
        case I_JUMP_ABS: printf("jump absolute %u\n", pc->b); if (padnum > 0) padnum--; break;
        case I_REF: printf("ref %d\n", pc->unum); break;
        case I_PLA:
            printf("pla %zu\n", pc->b + icount);
            padnum++; break;
        case I_NLA: printf("nla %zu\n", pc->b + icount); padnum++; break;
        case I_WORD: puts("word"); break;
        case I_NWORD: puts("noword"); break;
        }

        icount++; /* pc - (Instruct *)reo->bcode.buf */
        pc++;
        len -= INSTRUCT_LEN;
    }

}

bool mre_match(MRE *reo, const char *string, bool igcase)
{
    if (!reo || !string || *string == 0 || reo->bcode.len <= 0) return false;

    mlist_clear(reo->sublist);

    return _execute(reo, NULL, string, igcase);
}

uint32_t mre_match_all(MRE *reo, const char *string, bool igcase)
{
    if (!reo || !string || *string == 0 || reo->bcode.len <= 0) return 0;

    mlist_clear(reo->sublist);

    uint32_t count = 0;

    reo->pend = string;
    while (_execute(reo, NULL, reo->pend, igcase)) {
        count++;
    }

    return count;
}

uint32_t mre_match_count(MRE *reo)
{
    if (!reo || !reo->sublist) return 0;

    return mlist_length(reo->sublist);
}

uint32_t mre_sub_count(MRE *reo, uint32_t matchsn)
{
    if (mre_match_count(reo) < matchsn + 1) return 0;

    MLIST *slist;
    MERR *err;

    err = mlist_get(reo->sublist, matchsn, (void**)&slist);
    JUMP_NOK(err, error);

    return mlist_length(slist);

error:
    return 0;
}

bool mre_sub_get(MRE *reo, uint32_t matchsn, uint32_t index, const char **sp, const char **ep)
{
    if (mre_sub_count(reo, matchsn) < index + 1) return false;

    MLIST *slist;
    mlist_get(reo->sublist, matchsn, (void**)&slist);

    return _sub_get(slist, index, sp, ep);
}

void mre_destroy(MRE **reo)
{
    if (!reo) return;

    mbuf_clear(&(*reo)->bcode);
    mlist_destroy(&(*reo)->cclist);
    mlist_destroy(&(*reo)->sublist);

    mos_free(*reo);
}
