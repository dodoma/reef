
/*
 * ATTENTION: 乘热使用 pc，若夹杂着其他 _emitxxx 后再使用，bcode.buf 已经被 realloc 改变了值
 */
static uint32_t _emit(MRE *reo, uint8_t op, Instruct **pc)
{
    Instruct inst;

    memset(&inst, 0x0, INSTRUCT_LEN);

    inst.op_code = op;
    mbuf_append(&reo->bcode, &inst, INSTRUCT_LEN);

    if (pc) *pc = (Instruct *)(reo->bcode.buf + reo->bcode.len - INSTRUCT_LEN);

    return 1;
}

static uint32_t _emit_split(MRE *reo, uint32_t alen, uint32_t blen)
{
    Instruct inst;
    uint32_t icount = 0;

    memset(&inst, 0x0, INSTRUCT_LEN);
    inst.op_code = I_JUMP_REL;
    inst.b = blen + 1;
    mbuf_insert(&reo->bcode, reo->bcode.len - ((blen) * INSTRUCT_LEN), &inst, INSTRUCT_LEN);
    icount++;

    memset(&inst, 0x0, INSTRUCT_LEN);
    inst.op_code = I_SPLIT;
    inst.b = alen + 2;
    mbuf_insert(&reo->bcode, reo->bcode.len - ((alen + blen + 1) * INSTRUCT_LEN), &inst, INSTRUCT_LEN);
    icount++;

    return icount;
}

static uint32_t _emit_cclass(MRE *reo, MLIST *rlist, bool negative)
{
    Instruct inst;

    memset(&inst, 0x0, INSTRUCT_LEN);

    inst.op_code = negative ? I_NCCLASS : I_CCLASS;
    inst.rlist = rlist;
    mbuf_append(&reo->bcode, &inst, INSTRUCT_LEN);

    return 1;
}

static uint32_t _emit_repeat(MRE *reo, int min, int max, uint32_t lastcount, bool greedy)
{
    Instruct inst;
    int icount = 0;

    memset(&inst, 0x0, INSTRUCT_LEN);

    if (min > 1 && min == max) {
        /* 上一个组件必须执行 min 次 */
        inst.op_code = I_JUMP_REL;
        inst.b = -lastcount;
        inst.unum = min - 1;    /* 上个指令已经执行了一次，故 -1 */
        inst.unum -= 1;         /* unum 为重复次数，故 -1 */
        mbuf_append(&reo->bcode, &inst, INSTRUCT_LEN);
        icount++;
    } else if (max > 1) {
        /* 上一个组件可以执行 >= min <= max 次 */
        char *previ = reo->bcode.buf + reo->bcode.len - (lastcount * INSTRUCT_LEN);

        if (min > 2) {
            /* REPEAT FIX */
            inst.op_code = I_JUMP_REL;
            inst.b = -lastcount;
            inst.unum = min - 1 - 1 - 1;
            mbuf_append(&reo->bcode, &inst, INSTRUCT_LEN);
            icount++;
        }

        /* REPEAT SPLIT */
        memset(&inst, 0x0, INSTRUCT_LEN);
        inst.op_code = I_SPLIT;
        inst.b = 2 + lastcount;
        mbuf_append(&reo->bcode, &inst, INSTRUCT_LEN);
        icount++;

        /* previous instruct */
        mbuf_append(&reo->bcode, previ, (lastcount * INSTRUCT_LEN));
        icount += lastcount;

        if ((max - min) > 1) {
            /* REPEAT BACK */
            memset(&inst, 0x0, INSTRUCT_LEN);
            inst.op_code = I_JUMP_REL;
            inst.b = -lastcount - 1;
            inst.unum = (max - min) - 1 - 1;
            mbuf_append(&reo->bcode, &inst, INSTRUCT_LEN);
            icount++;
        } else {
            memset(&inst, 0x0, INSTRUCT_LEN);
            inst.op_code = I_JUMP_REL;
            inst.b = 1;
            inst.unum = 0;
            mbuf_append(&reo->bcode, &inst, INSTRUCT_LEN);
            icount++;
        }
    }

    if (min == 0) {
        /* 上一个组件可以不执行 */
        inst.op_code = I_SPLIT;
        inst.b = icount + lastcount + 1;
        mbuf_insert(&reo->bcode, reo->bcode.len - (icount + lastcount) * INSTRUCT_LEN, &inst, INSTRUCT_LEN);
        icount++;

        memset(&inst, 0x0, INSTRUCT_LEN);
        inst.op_code = I_JUMP_REL;
        inst.b = 1;
        mbuf_append(&reo->bcode, &inst, INSTRUCT_LEN);
        icount++;
    }

    return icount;
}
