enum {
    I_END,

    I_LITERAL,
    I_VAR,
    I_NAME,
    I_INCLUDE,
    I_EACH,
    I_EACH_CLOSE,
    I_IF,
    I_ELIF,
    I_ELSE,
    I_IF_CLOSE,
};

static inline uint32_t _genid(MCS *sme)
{
    if (sme->icount++ > 0xEFFFFFFF) DIE(sme, "too many instructions");

    return sme->icount;
}

/* TODO speedup */
static struct instruct* _instruct_find(MCS *sme, uint32_t id)
{
    struct instruct *pc = (struct instruct*)sme->bcode.buf;
    while (pc->op != I_END && pc->id != id) {
        pc++;
    }

    if (pc->id == id) return pc;

    DIE(sme, "lookup instruct failure");
}

/*
 * ATTENTION: 不建议直接使用此处返回的pc, 要用也要乘热使用 pc，
 * 若夹杂着其他 _emitxxx 后再使用，bcode.buf 已经被 realloc 改变了值
 */
static void _emit(MCS *sme, uint8_t op, struct instruct **pc)
{
    struct instruct inst;

    memset(&inst, 0x0, INSTRUCT_LEN);

    inst.id = _genid(sme);
    inst.op = op;
    inst.lineno = sme->lineno;
    inst.u.s = NULL;
    inst.len = 0;
    inst.pca = 0;
    inst.pcb = 0;
    mbuf_append(&sme->bcode, &inst, INSTRUCT_LEN);

    if (pc) *pc = (struct instruct *)(sme->bcode.buf + sme->bcode.len - INSTRUCT_LEN);
}
