#include "reef.h"

enum {
    I_END, I_BOL, I_EOL, I_CHAR
};

typedef struct {
    uint8_t op_code;
    char c;
} Instruct;


struct _MRE {
    MBUF bcode;
};

static Instruct* _emit(MRE *reo, uint8_t op)
{
    Instruct inst = {.op_code = op, .c = 0};

    return mbuf_append(&reo->bcode, &inst, sizeof(Instruct));
}

static MERR* _compile(MRE *reo, const char *pattern)
{
    const char *pos = pattern;
    Instruct *inst;
    uint32_t col = 1;

    while (*pos != '\0') {
        switch (*pos) {
        case '^':
            _emit(reo, I_BOL);
            break;
        case '$':
            _emit(reo, I_EOL);
            break;
        default:
            inst = _emit(reo, I_CHAR);
            inst->c = *pos;
            break;
        }
        pos++;
        col++;
    }

    _emit(reo, I_END);

    return MERR_OK;
}

static bool _execute(MRE *reo, const char *string)
{
    Instruct *pc = (Instruct *)reo->bcode.buf; /* program counter */
    const char *sp = string;                   /* string pointer */
    const char *bol = string;

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
            return false;
        case I_CHAR:
            if (*sp == pc->c) {
                sp = sp + 1;
                pc = pc + 1;
                continue;
            }
            return false;
        default:
            return false;
        }
    }
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

    err = _compile(reo, pattern);
    if (err) return merr_pass(err);

    return MERR_OK;
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
