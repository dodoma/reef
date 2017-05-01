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

    size_t len = reo->bcode.len;

    if (len > blen * INSTRUCT_LEN) {
        memset(&inst, 0x0, INSTRUCT_LEN);
        inst.op_code = I_JUMP_REL;
        inst.b = blen + 1;
        mbuf_insert(&reo->bcode, len - blen * INSTRUCT_LEN, &inst, INSTRUCT_LEN);
        icount++;
    }

    memset(&inst, 0x0, INSTRUCT_LEN);
    inst.op_code = I_SPLIT;
    inst.b = alen + 2;
    mbuf_insert(&reo->bcode, 4 * INSTRUCT_LEN, &inst, INSTRUCT_LEN);
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

static uint32_t _emit_repeat(MRE *reo, int min, int max, bool greedy)
{
    Instruct inst;
    int icount = 0;

    memset(&inst, 0x0, INSTRUCT_LEN);

    if (min > 1 && min == max) {
        /* 上一个组件必须执行 min 次 */
        inst.op_code = I_JUMP_REL;
        inst.b = -1;
        inst.repeat = min - 1;    /* 上个指令已经执行了一次，故 -1 */
        inst.repeat -= 1;         /* repeat 为重复次数，故 -1 */
        mbuf_append(&reo->bcode, &inst, INSTRUCT_LEN);
        icount++;
    } else if (max > 1) {
        /* 上一个组件可以执行 >= min <= max 次 */
        /* TODO previ */
        char *previ = reo->bcode.buf + reo->bcode.len - INSTRUCT_LEN;

        if (min > 2) {
            /* REPEAT FIX */
            inst.op_code = I_JUMP_REL;
            inst.b = -1;
            inst.repeat = min - 1 - 1 - 1;
            mbuf_append(&reo->bcode, &inst, INSTRUCT_LEN);
            icount++;
        }

        /* REPEAT SPLIT */
        memset(&inst, 0x0, INSTRUCT_LEN);
        inst.op_code = I_SPLIT;
        inst.b = 3;
        mbuf_append(&reo->bcode, &inst, INSTRUCT_LEN);
        icount++;

        /* previous instruct */
        mbuf_append(&reo->bcode, previ, INSTRUCT_LEN);
        icount++;

        if ((max - min) > 1) {
            /* REPEAT BACK */
            memset(&inst, 0x0, INSTRUCT_LEN);
            inst.op_code = I_JUMP_REL;
            inst.b = -2;
            inst.repeat = (max - min) - 1 - 1;
            mbuf_append(&reo->bcode, &inst, INSTRUCT_LEN);
            icount++;
        } else {
            memset(&inst, 0x0, INSTRUCT_LEN);
            inst.op_code = I_JUMP_REL;
            inst.b = 1;
            inst.repeat = 0;
            mbuf_append(&reo->bcode, &inst, INSTRUCT_LEN);
            icount++;
        }
    }

    if (min == 0) {
        /* 上一个组件可以不执行 */
        /* TODO previ */
        inst.op_code = I_SPLIT;
        inst.b = icount + 2;
        mbuf_insert(&reo->bcode, reo->bcode.len - (icount + 1) * INSTRUCT_LEN, &inst, INSTRUCT_LEN);
        icount++;

        memset(&inst, 0x0, INSTRUCT_LEN);
        inst.op_code = I_JUMP_REL;
        inst.b = 1;
        mbuf_append(&reo->bcode, &inst, INSTRUCT_LEN);
        icount++;
    }

    return icount;
}
