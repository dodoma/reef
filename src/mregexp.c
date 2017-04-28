#include "reef.h"

#include <setjmp.h>

#define MAX_SPLIT 10000

enum {
    I_END,

    I_BOL, I_EOL, I_CHAR, I_ANY, I_ANYNL, I_LPAR, I_RPAR, I_CCLASS, I_NCCLASS,
    I_SPLIT, I_INCREMENT, I_JUMP
};

typedef struct {
    uint8_t type;
    Rune c;
} Token;

typedef struct {
    uint8_t op_code;
    Rune c;                     /* character on I_CHAR */
    uint32_t b;                 /* b location on I_SPLIT */
    MLIST *rlist;               /* character class ranges */
} Instruct;

typedef struct {
    Instruct *pc;
    const char *sp;
} Road;


struct _MRE {
    MBUF bcode;

    MLIST *cclist;              /* character class */

    const char *source;         /* Nul-terminated source code buffer */
    const char *pos;            /* Current position */
    const char *nmatch;

    Token tok;

    const char *error;
	jmp_buf kaboom;
};

static bool _isnewline(Rune c)
{
	return c == 0xA || c == 0xD || c == 0x2028 || c == 0x2029;
}

static void _die(MRE *reo, const char *message)
{
    reo->error = message;
    longjmp(reo->kaboom, 1);
}

#include "_mregexp_tok.c"

static uint32_t _emit(MRE *reo, uint8_t op)
{
    Instruct inst;

    memset(&inst, 0x0, sizeof(Instruct));

    inst.op_code = op;
    size_t pos = mbuf_append(&reo->bcode, &inst, sizeof(Instruct));

    return pos / sizeof(Instruct);
}

static uint32_t _emit_char(MRE *reo, Rune c)
{
    Instruct inst;

    memset(&inst, 0x0, sizeof(Instruct));

    inst.op_code = I_CHAR;
    inst.c = c;
    size_t pos = mbuf_append(&reo->bcode, &inst, sizeof(Instruct));

    return pos / sizeof(Instruct);
}

static uint32_t _emit_split(MRE *reo, uint32_t alen, uint32_t blen)
{
    Instruct inst;
    size_t pos;

    size_t len = reo->bcode.len;

    if (blen > 0 && len > blen * sizeof(Instruct)) {
        memset(&inst, 0x0, sizeof(Instruct));
        inst.op_code = I_INCREMENT;
        inst.b = blen + 1;
        mbuf_insert(&reo->bcode, len - blen * sizeof(Instruct), &inst, sizeof(Instruct));
    }

    memset(&inst, 0x0, sizeof(Instruct));
    inst.op_code = I_SPLIT;
    inst.b = alen + 2;
    pos = mbuf_insert(&reo->bcode, blen > 0 ? 4 * sizeof(Instruct) : 0, &inst, sizeof(Instruct));

    return pos / sizeof(Instruct);
}

static uint32_t _emit_jump(MRE *reo, uint32_t b)
{
    Instruct inst;

    memset(&inst, 0x0, sizeof(Instruct));

    inst.op_code = I_JUMP;
    inst.b = b;
    size_t pos = mbuf_append(&reo->bcode, &inst, sizeof(Instruct));

    return pos / sizeof(Instruct);
}

static uint32_t _emit_cclass(MRE *reo, MLIST *rlist, bool negative)
{
    Instruct inst;

    memset(&inst, 0x0, sizeof(Instruct));

    inst.op_code = negative ? I_NCCLASS : I_CCLASS;
    inst.rlist = rlist;
    size_t pos = mbuf_append(&reo->bcode, &inst, sizeof(Instruct));

    return pos / sizeof(Instruct);
}

static bool _inrange(MLIST *rlist, Rune c)
{
    Rune *a, *b;

    for (int i = 0; i < mlist_length(rlist); i += 2) {
        mlist_get(rlist, i, (void**)&a);
        mlist_get(rlist, i+1, (void**)&b);

        if ((Rune)a <= c && c <= (Rune)b) return true;
    }

    return false;
}

static void _addrange(MRE *reo, MLIST *rlist, Rune a, Rune b)
{
    if (a > b) {
        _die(reo, "invalid character class range");
    }

    mlist_append(rlist, MOS_MEM_OFFSET(a));
    mlist_append(rlist, MOS_MEM_OFFSET(b));
}

static void _add_ranges_d(MRE *reo, MLIST *rlist)
{
    _addrange(reo, rlist, '0', '9');
}

static void _add_ranges_D(MRE *reo, MLIST *rlist)
{
	_addrange(reo, rlist, 0, '0'-1);
	_addrange(reo, rlist, '9'+1, 0xFFFF);
}

static void _add_ranges_s(MRE *reo, MLIST *rlist)
{
	_addrange(reo, rlist, 0x9, 0xD);
	_addrange(reo, rlist, 0x20, 0x20);
	_addrange(reo, rlist, 0xA0, 0xA0);
	_addrange(reo, rlist, 0x2028, 0x2029);
	_addrange(reo, rlist, 0xFEFF, 0xFEFF);
}

static void _add_ranges_S(MRE *reo, MLIST *rlist)
{
	_addrange(reo, rlist, 0, 0x9-1);
	_addrange(reo, rlist, 0xD+1, 0x20-1);
	_addrange(reo, rlist, 0x20+1, 0xA0-1);
	_addrange(reo, rlist, 0xA0+1, 0x2028-1);
	_addrange(reo, rlist, 0x2029+1, 0xFEFF-1);
	_addrange(reo, rlist, 0xFEFF+1, 0xFFFF);
}

static void _add_ranges_w(MRE *reo, MLIST *rlist)
{
	_addrange(reo, rlist, '0', '9');
	_addrange(reo, rlist, 'A', 'Z');
	_addrange(reo, rlist, '_', '_');
	_addrange(reo, rlist, 'a', 'z');
}

static void _add_ranges_W(MRE *reo, MLIST *rlist)
{
	_addrange(reo, rlist, 0, '0'-1);
	_addrange(reo, rlist, '9'+1, 'A'-1);
	_addrange(reo, rlist, 'Z'+1, '_'-1);
	_addrange(reo, rlist, '_'+1, 'a'-1);
	_addrange(reo, rlist, 'z'+1, 0xFFFF);
}

static void _parse_cclass(MRE *reo, bool negative)
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

    _emit_cclass(reo, rlist, negative);
}

static uint32_t _parse_statement(MRE *reo)
{
    uint8_t tok = TOK_EOF;
    uint32_t count = 0;

    while ((tok = _tok_next(reo, false)) != TOK_OR && tok != TOK_EOF) {
        switch (tok) {
        case TOK_BOL: _emit(reo, I_BOL); break;
        case TOK_EOL: _emit(reo, I_EOL); break;
        case TOK_CHAR: _emit_char(reo, reo->tok.c); break;
        case TOK_ANY: _emit(reo, I_ANY); break;
        case TOK_CCLASS: _parse_cclass(reo, false); break;
        case TOK_NCCLASS: _parse_cclass(reo, true); break;
        default: _die(reo, "unexpect token");
        }

        count++;
    }

    return count;
}

static MERR* _compile(MRE *reo, const char *pattern)
{
    MERR_NOT_NULLB(reo, pattern);

    reo->pos = reo->source = pattern;

    if (setjmp(reo->kaboom)) {
        return merr_raise(MERR_ASSERT, "%s on %ld %s", reo->error, reo->pos - reo->source, reo->pos - 1);
    }

    _emit(reo, I_ANYNL);
    _emit_jump(reo, 0);
    _emit_split(reo, 1, 0);
    _emit(reo, I_LPAR);

    uint32_t alen, blen;
    alen = _parse_statement(reo);
    while (reo->tok.type == TOK_OR) {
        blen = _parse_statement(reo);

        _emit_split(reo, alen, blen);

        alen += blen;
        alen += 2;              /* for split And jump */
    }

    _emit(reo, I_RPAR);
    _emit(reo, I_END);

    return MERR_OK;
}

static void _newroad(Road *t, Instruct *pc, uint32_t pos, const char *sp)
{
    t->pc = pc + pos;
    t->sp = sp;
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

    memset(roads, 0x0, sizeof(roads));
    _newroad(&roads[0], reset_pc, 0, sp);
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

                _newroad(&roads[nroad], pc, pc->b, sp);
                nroad++;
                pc = pc + 1;
                continue;
            case I_INCREMENT:
                pc = pc + pc->b;
                continue;
            case I_JUMP:
                pc = reset_pc + pc->b;
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
    printf("%ld %s dont match\n", sp - string, reo->nmatch);

    return false;
}

MRE* mre_init()
{
    MRE *reo = mos_calloc(1, sizeof(MRE));

    mbuf_init(&reo->bcode, 0);
    mlist_init(&reo->cclist, mlist_free);

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
    size_t counter = 0;

    /*
     * *instructs* 2
     */
    while (len > 0) {
        printf("% 5ld: ", counter);
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
        case I_SPLIT: printf("split %lu\n", pc->b + counter); padnum++; break;
        case I_INCREMENT: printf("split jump %lu\n", pc->b + counter); padnum--; break;
        case I_JUMP: printf("jump %u\n", pc->b); padnum--; break;
        }

        counter++; /* pc - (Instruct *)reo->bcode.buf */
        pc++;
        len -= sizeof(Instruct);
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
