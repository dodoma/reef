enum optype {
    OP_ERROR  = (1<<0),
    OP_EXISTS = (1<<1),         /* ? */
    OP_NOT    = (1<<2),         /* ! */
    OP_NAME   = (1<<3),         /* Me.name */
    OP_STRING = (1<<4),         /* "xxx" */
    OP_NUMBER = (1<<5),         /* -123.38 */

    OP_EQUAL  = (1<<7),         /* == */
    OP_NEQUAL = (1<<8),         /* != */
    OP_LT     = (1<<9),         /* < */
    OP_LTE    = (1<<10),        /* <= */
    OP_GT     = (1<<11),        /* > */
    OP_GTE    = (1<<12),        /* >= */
    OP_AND    = (1<<13),        /* && */
    OP_OR     = (1<<14),        /* || */
    OP_ADD    = (1<<15),        /* + */
    OP_SUB    = (1<<16),        /* - */
    OP_MULT   = (1<<17),        /* * */
    OP_DIV    = (1<<18),        /* / */
    OP_MOD    = (1<<19),        /* % */

    OP_LPAREN = (1<<20),        /* ( */
    OP_RPAREN = (1<<21),        /* ) */
};

struct opnode {
    enum optype op;

    union {
        float f;                /* use float to do numberic operation(without int64_t) */
        char *s;
    } val;

    size_t len;                 /* string length */
};

enum optype oporder[] = {
    OP_STRING | OP_NUMBER,
    OP_OR,
    OP_AND,
    OP_EQUAL | OP_NEQUAL,
    OP_GT | OP_GTE | OP_LT | OP_LTE,
    OP_ADD | OP_SUB,
    OP_MULT | OP_DIV | OP_MOD,
    OP_NOT | OP_EXISTS | OP_NAME,
    OP_LPAREN,
    OP_ERROR,
    0
};

enum optype abop = OP_EQUAL | OP_NEQUAL | OP_LT | OP_LTE | OP_GT | OP_GTE | \
    OP_AND | OP_OR | OP_ADD | OP_SUB | OP_MULT | OP_DIV | OP_MOD;

static void _op_dump(struct opnode *node, char *msg, size_t len)
{
    switch (node->op) {
    case OP_ERROR:
        snprintf(msg, len, "ERROR %s", node->val.s);
        break;
    case OP_EXISTS:
        snprintf(msg, len, "EXIST %.*s", (int)node->len, node->val.s);
        break;
    case OP_NOT:
        strncpy(msg, "NOT", len);
        break;
    case OP_NAME:
        snprintf(msg, len, "NAME %.*s", (int)node->len, node->val.s);
        break;
    case OP_STRING:
        snprintf(msg, len, "STRING %.*s", (int)node->len, node->val.s);
        break;
    case OP_NUMBER:
        snprintf(msg, len, "NUMBER %f", node->val.f);
        break;
    case OP_EQUAL:
        strncpy(msg, "EQUAL", len);
        break;
    case OP_NEQUAL:
        strncpy(msg, "NEQUAL", len);
        break;
    case OP_LT:
        strncpy(msg, "LT", len);
        break;
    case OP_LTE:
        strncpy(msg, "LTE", len);
        break;
    case OP_GT:
        strncpy(msg, "GT", len);
        break;
    case OP_GTE:
        strncpy(msg, "GTE", len);
        break;
    case OP_AND:
        strncpy(msg, "AND", len);
        break;
    case OP_OR:
        strncpy(msg, "OR", len);
        break;
    case OP_ADD:
        strncpy(msg, "ADD", len);
        break;
    case OP_SUB:
        strncpy(msg, "SUB", len);
        break;
    case OP_MULT:
        strncpy(msg, "MULT", len);
        break;
    case OP_DIV:
        strncpy(msg, "DIV", len);
        break;
    case OP_MOD:
        strncpy(msg, "MOD", len);
        break;
    case OP_LPAREN:
        strncpy(msg, "LPAREN", len);
        break;
    case OP_RPAREN:
        strncpy(msg, "RPAREN", len);
        break;
    default:
        strncpy(msg, "UNKNOWN", len);
        break;
    }
}

/* 当前字符是否不属于name? */
static inline bool _name_escape(char c)
{
    switch (c) {
    case '?':
    case '!':
    case '"':
    case '-':
    case '=':
    case '<':
    case '>':
    case '&':
    case '|':
    case '+':
    case '*':
    case '/':
    case '%':
    case '(':
    case ')':
        return true;
    default:
        return false;
    }
}

static size_t _name_len(char *str)
{
    size_t len = 0;
    char *pos = str;

    while (*pos != '\0' && !_name_escape(*pos)) {
        pos++;
        len++;
    }

    return len;
}

static size_t _string_len(char *str)
{
    size_t len = 0;
    char *pos = str;

    while (*pos != '\0' && *pos != '"') {
        if (*pos == '\\' && *(pos+1) == '"') {
            pos += 2;
            len += 2;
        } else {
            pos++;
            len++;
        }
    }

    return len;
}

static size_t _digit_len(char *str, struct opnode *node, int sign)
{
    size_t len = 0;
    char *pos = str;

    bool point = false;
    float fval = 0.0;
    float fpart = 0.1;

    while (*pos != '\0' && (isdigit(*pos) || *pos == '.')) {
        if (*pos == '.') point = true;
        else {
            if (!point) {
                fval = (fval * 10) + (*pos - '0');
            } else {
                fval += (*pos - '0') * fpart;
                fpart = fpart * 0.1;
            }
        }

        pos++;
        len++;
    }

    node->val.f = sign * fval;

    return len;
}

static size_t _node_next(char *str, struct opnode **node)
{
    size_t len = 0;
    char *pos = str;
    if (!str || *str == '\0') return 0;

    struct opnode *me = mos_calloc(1, sizeof(struct opnode));
    me->op = OP_ERROR;
    me->val.s = pos;
    me->len = 0;

    Rune c;
    len = chartorune(&c, pos);

    switch (c) {
    case '?':
        pos += 1;
        me->op = OP_EXISTS;
        me->val.s = pos;
        me->len = _name_len(pos);
        len = me->len + 1;      /* for '?' */
        break;
    case '!':
        if (*(pos+1) == '=') {
            me->op = OP_NEQUAL;
            len += 1;
        } else {
            me->op = OP_NOT;
        }
        break;
    case '"':
        pos += 1;
        me->op = OP_STRING;
        me->val.s = pos;
        me->len = _string_len(pos);
        len = me->len + 2;      /* for wrapped '"' */
        break;
    case '=':
        if (*(pos+1) == '=') {
            me->op = OP_EQUAL;
            len += 1;
        }
        break;
    case '<':
        if (*(pos+1) == '=') {
            me->op = OP_LTE;
            len += 1;
        } else me->op = OP_LT;
        break;
    case '>':
        if (*(pos+1) == '=') {
            me->op = OP_GTE;
            len += 1;
        } else me->op = OP_GT;
        break;
    case '&':
        if (*(pos+1) == '&') {
            me->op = OP_AND;
            len += 1;
        }
        break;
    case '|':
        if (*(pos+1) == '|') {
            me->op = OP_OR;
            len += 1;
        }
        break;
    case '+':
        me->op = OP_ADD;
        break;
    case '-':
        if ((*node)->op == OP_NUMBER) me->op = OP_SUB;
        else {
            pos += 1;
            me->op = OP_NUMBER;
            len += _digit_len(pos, me, -1);
        }
        break;
    case '*':
        me->op = OP_MULT;
        break;
    case '/':
        me->op = OP_DIV;
        break;
    case '%':
        me->op = OP_MOD;
        break;
    case '(':
        me->op = OP_LPAREN;
        break;
    case ')':
        me->op = OP_RPAREN;
        break;
    case '0'...'9':
        me->op = OP_NUMBER;
        len = _digit_len(pos, me, 1);
        break;
    default:
        me->op = OP_NAME;
        me->len = _name_len(pos);
        len = me->len;
        break;
    }

    *node = me;

    return len;
}

/* 找出链表中优先级最高的操作 */
static MDLIST* _opnode_urgent(MDLIST *alist)
{
    if (!alist) return NULL;

    int index = 0, max = 0;
    MDLIST *urgent = alist;

    MDLIST *item = mdlist_head(alist);
    while (item) {
        struct opnode *node = mdlist_data(item);

        index = 0;
        while (oporder[index]) {
            if (oporder[index] & node->op) break;
            index++;
        }

        if (oporder[index] && index > max) {
            max = index;
            urgent = item;
        }

        item = mdlist_next(item);
    }

    return urgent;
}

static bool _opnode_equal(struct opnode *nodea, struct opnode *nodeb)
{
    if (!nodea || !nodeb) return false;

    if (nodea->op == OP_STRING) {
        if (nodeb->op == OP_STRING && nodea->len == nodeb->len && !memcmp(nodea->val.s, nodeb->val.s, nodea->len))
            return true;
        return false;
    } else if (nodea->op == OP_NUMBER) {
        if (nodeb->op == OP_NUMBER && nodea->val.f == nodeb->val.f) return true;
        return false;
    }

    return false;
}

static MDLIST* _opnode_eval(MDLIST *alist, MDF *mnode)
{
    if (!alist) return NULL;

    char key[1024] = {0};
    struct opnode *onode = mdlist_data(alist);
    if (onode->len > 0) strncpy(key, onode->val.s, onode->len > sizeof(key) ? sizeof(key) : onode->len);

    MDLIST *prev = mdlist_prev(alist);
    MDLIST *next = mdlist_next(alist);
    struct opnode *nodea = mdlist_data(prev);
    struct opnode *nodeb = mdlist_data(next);
    bool eject = false;

    if (onode->op & abop) {
        eject = true;
        if (!prev || !next) {
            onode->op = OP_ERROR;
            onode->val.s = "expect a,b operation";
            return alist;
        }
    }

    switch (onode->op) {
    case OP_ERROR:
        return alist;
    case OP_EXISTS:
        if (mdf_path_exist(mnode, key)) onode->val.f = 1;
        else onode->val.f = 0;
        break;
    case OP_NOT:
        if (!nodeb) {
            onode->op = OP_ERROR;
            onode->val.s = "expect b operation";
            return alist;
        }
        if (nodeb->val.f != 0) onode->val.f = 1;
        else onode->val.f = 0;
        mdlist_eject(next, free);
        return alist;
    case OP_NAME:
        {
            MDF_TYPE type = mdf_get_type(mnode, key);
            if (type == MDF_TYPE_INT) {
                onode->op = OP_NUMBER;
                onode->val.f = (float)mdf_get_int_value(mnode, key, 0);
            } else if (type == MDF_TYPE_FLOAT) {
                onode->op = OP_NUMBER;
                onode->val.f = mdf_get_float_value(mnode, key, 0.0);
            } else if (type == MDF_TYPE_STRING) {
                onode->op = OP_STRING;
                onode->val.s = mdf_get_value(mnode, key, NULL);
                if (onode->val.s) onode->len = strlen(onode->val.s);
            } else {
                onode->op = OP_NUMBER;
                onode->val.f = 0;
            }
        }
        return alist;
    case OP_STRING:
        return alist;
    case OP_NUMBER:
        /* nothing to do */
        break;

    case OP_EQUAL:
        if (_opnode_equal(nodea, nodeb)) onode->val.f = 1;
        else onode->val.f = 0;
        break;
    case OP_NEQUAL:
        if (_opnode_equal(nodea, nodeb)) onode->val.f = 0;
        else onode->val.f = 1;
        break;
    case OP_LT:
        if (nodea->val.f < nodeb->val.f) onode->val.f = 1;
        else onode->val.f = 0;
        break;
    case OP_LTE:
        if (nodea->val.f <= nodeb->val.f) onode->val.f = 1;
        else onode->val.f = 0;
        break;
    case OP_GT:
        if (nodea->val.f > nodeb->val.f) onode->val.f = 1;
        else onode->val.f = 0;
        break;
    case OP_GTE:
        if (nodea->val.f >= nodeb->val.f) onode->val.f = 1;
        else onode->val.f = 0;
        break;
    case OP_AND:
        if (nodea->val.f == 1 && nodeb->val.f == 1) onode->val.f = 1;
        else onode->val.f = 0;
        break;
    case OP_OR:
        if (nodea->val.f == 1 || nodeb->val.f == 1) onode->val.f = 1;
        else onode->val.f = 0;
        break;
    case OP_ADD:
        onode->val.f = nodea->val.f + nodeb->val.f;
        break;
    case OP_SUB:
        onode->val.f = nodea->val.f - nodeb->val.f;
        break;
    case OP_MULT:
        onode->val.f = nodea->val.f * nodeb->val.f;
        break;
    case OP_DIV:
        if (nodeb->val.f != 0) onode->val.f = nodea->val.f / nodeb->val.f;
        else {
            onode->op = OP_ERROR;
            onode->val.s = "div zero";
            return alist;
        }
        break;
    case OP_MOD:
        onode->val.f = (float)((int)nodea->val.f % (int)nodeb->val.f);
        break;

    case OP_LPAREN:
    case OP_RPAREN:
        break;

    default:
        break;
    }

    onode->op = OP_NUMBER;

    if (eject) {
        mdlist_eject(prev, free);
        mdlist_eject(next, free);
    }

    return alist;
}

/*
 * exp:
 *     121
 *     2+5*3.2==100
 *     Me.0.name=="ml"
 *     Count>100&&?Me.name
 *     Count==100||(Count==200&&?Me.name)
 * eop: end of opration，遇到 0 或者 ')' 后停止
 * len: exp 被当前函数吃掉的长度
 */
static struct opnode* _eval_expr(char *exp, uint32_t eop, size_t *eatlen, MDF *mnode)
{
    //mtc_dbg("eval %s", exp);

    MDLIST *alist = NULL;
    struct opnode *onode = NULL;
    size_t len, sublen = 0;

    char *pos = exp;
    while ((len = _node_next(pos, &onode)) != 0 && (onode->op & eop) == 0) {
        pos += len;
        if (eatlen) *eatlen += len;

        if (onode->op == OP_LPAREN) {
            mos_free(onode);
            onode = _eval_expr(pos, OP_RPAREN, &sublen, mnode);
            pos += sublen;
            if (eatlen) *eatlen += sublen;
        }

        alist = mdlist_concat(alist, mdlist_new((void*)onode));
    }

    if (onode && onode->op == OP_RPAREN) {
        if (eatlen) *eatlen += 1;
        mos_free(onode);
        onode = NULL;
    }

    //MDLIST *dumpitem = mdlist_head(alist);
    //while (dumpitem) {
    //    onode = mdlist_data(dumpitem);
    //
    //    char msg[128];
    //    _op_dump(onode, msg, sizeof(msg));
    //    mtc_dbg("%s", msg);
    //
    //    dumpitem = mdlist_next(dumpitem);
    //}

    /* TODO 大规模使用需要注意效率 */
    alist = _opnode_urgent(alist);
    while (alist && !mdlist_alone(alist)) {
        alist = _opnode_eval(alist, mnode);

        onode = mdlist_data(alist);
        if (onode->op == OP_ERROR) break;

        alist = _opnode_urgent(alist);
    }

    _opnode_eval(alist, mnode);

    onode = mdlist_data(alist);

    mdlist_free(alist, NULL);

    return onode;
}

static bool _opnode_value_bool(struct opnode *node)
{
    if (!node) return false;

    if (node->op == OP_NUMBER) {
        if (node->val.f == 1) return true;
        return false;
    } else if (node->op == OP_STRING) {
        if (node->len > 0) return true;
        return false;
    } else if (node->op == OP_ERROR) {
        return false;
    } else return false;
}
