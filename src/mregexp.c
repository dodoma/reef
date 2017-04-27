#include "reef.h"

#include <setjmp.h>

#define MAX_SPLIT 10000

enum {
    I_END,

    I_BOL, I_EOL, I_CHAR, I_ANY, I_ANYNL, I_LPAR, I_RPAR,
    I_SPLIT, I_INCREMENT, I_JUMP
};

typedef struct {
    uint8_t type;
    Rune c;
} Token;

typedef struct {
    uint8_t op_code;
    Rune c;                     /* charector on I_CHAR */
    uint32_t b;                 /* b location on I_SPLIT */
} Instruct;

typedef struct {
    Instruct *pc;
    const char *sp;
} Road;


struct _MRE {
    MBUF bcode;
    const char *source;         /* Nul-terminated source code buffer */
    const char *pos;            /* Current position */

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
    Instruct inst = {.op_code = op, .c = 0, .b = 0};

    size_t pos = mbuf_append(&reo->bcode, &inst, sizeof(Instruct));

    return pos / sizeof(Instruct);
}

static uint32_t _emit_char(MRE *reo, Rune c)
{
    Instruct inst = {.op_code = I_CHAR, .c = c, .b = 0};

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
    Instruct inst = {.op_code = I_JUMP, .c = 0, .b = b};

    size_t pos = mbuf_append(&reo->bcode, &inst, sizeof(Instruct));

    return pos / sizeof(Instruct);
}


static uint32_t _parse_statement(MRE *reo)
{
    uint8_t tok = TOK_EOF;
    uint32_t count = 0;

    tok = _tok_next(reo);
    while (tok != TOK_OR && tok != TOK_EOF) {
        switch (tok) {
        case TOK_BOL:
            _emit(reo, I_BOL);
            break;
        case TOK_EOL:
            _emit(reo, I_EOL);
            break;
        case TOK_CHAR:
            _emit_char(reo, reo->tok.c);
            break;
        default:
            _die(reo, "unexpect token");
        }

        count++;
        tok = _tok_next(reo);
    }

    return count;
}

static MERR* _compile(MRE *reo, const char *pattern)
{
    MERR_NOT_NULLB(reo, pattern);

    reo->pos = reo->source = pattern;

    if (setjmp(reo->kaboom)) {
        return merr_raise(MERR_ASSERT, "%s on %ld %s", reo->error, reo->pos - reo->source, reo->pos);
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
                sp += chartorune(&c, (char*)sp);
                if (c == pc->c) {
                    pc = pc + 1;
                    continue;
                }
                goto river;
            case I_ANY:
                sp += chartorune(&c, (char*)sp);
                if (c && !_isnewline(c)) {
                    pc = pc + 1;
                    continue;
                }
                goto river;
            case I_ANYNL:
                sp += chartorune(&c, (char*)sp);
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

    return false;
}

MRE* mre_init()
{
    MRE *reo = mos_calloc(1, sizeof(MRE));

    mbuf_init(&reo->bcode, 0);

    return reo;
}

MERR* mre_compile(MRE *reo, const char *pattern)
{
    MERR *err;

    MERR_NOT_NULLB(reo, pattern);

    mbuf_clear(&reo->bcode);

    reo->error = NULL;

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

    mos_free(*reo);
}
