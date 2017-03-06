#include "reef.h"
#include "_mdf.h"

static MDF* _walk_by_name(MDF *node, const char *name, size_t len, bool create)
{
    int childnum;
    char sname[PATH_MAX+1] = {0};
    MDF *rnode;

    childnum = 0;
    len = len > PATH_MAX ? PATH_MAX : len;

    memcpy(sname, name, len);
    sname[len] = '\0';

    if (node->table) {
        rnode = mhash_lookup(node->table, sname);
        if (rnode) goto found;
    } else {
        rnode = node->child;
        while (rnode) {
            if (!strcmp(rnode->name, sname)) goto found;

            childnum++;
            rnode = rnode->next;
        }
    }

    if (create) {
        if (node->type == MDF_TYPE_STRING || node->type == MDF_TYPE_BINARY)
            mos_free(node->val.s);
        node->type = MDF_TYPE_OBJECT;

        mdf_init(&rnode);
        rnode->name = strdup(sname);
        rnode->namelen = strlen(sname);

        _mdf_append_child_node(node, rnode, childnum);

        return rnode;
    }

    return NULL;

found:
    return rnode;
}

static MDF* _walk_by_index(MDF *node, int index, bool create)
{
    char sname[PATH_MAX+1] = {0};
    MDF *cnode, *rnode;
    int childnum;

    childnum = 0;

    if (index < 0) {
        rnode = node->last_child;
        if (rnode) return rnode;
        else {
            index = 0;
            goto create_if_want;
        }
    }

    cnode = node->child;
    while (cnode && childnum < index) {
        cnode = cnode->next;
        childnum++;
    }

    if (cnode) return cnode;

create_if_want:
    if (!create) return NULL;

    if (node->type == MDF_TYPE_STRING || node->type == MDF_TYPE_BINARY)
        mos_free(node->val.s);

    node->type = MDF_TYPE_ARRAY;

    for (int i = childnum; i < index + 1; i++) {
        snprintf(sname, sizeof(sname), "__moon_resolved_index__%d", i);

        mdf_init(&rnode);
        rnode->name = strdup(sname);
        rnode->namelen = strlen(sname);

        _mdf_append_child_node(node, rnode, childnum);

        childnum++;
    }

    return rnode;
}

static MERR* _walk_mdf(MDF *node, const char *path, bool create, MDF **rnode)
{
    char *pos, *oldpos, *start, *end;
    MDF *xnode;
#if __x86_64__
    int64_t index;
#else
    int32_t index;
#endif
    MLIST *indexname_list;
    bool descend;

    if (!path || path[0] == '\0') {
        *rnode = node;
        return MERR_OK;
    }

    mlist_init(&indexname_list, NULL);

    descend = false;
    *rnode = xnode = NULL;
    start = end = NULL;

    pos = (char*)path;
    while (*pos && *pos != '.') {
        switch (*pos) {
        case '[':
            if (descend) goto format_error;
            if (start && !end && pos > start) end = pos;
            descend = true;
            pos++;
            break;
        case ']':
            if (!descend) goto format_error;
            descend = false;
            pos++;
            break;
        default:
            if (descend) {
                /* process INDEX_NAME */
                int sign = 1;
                if (*pos == '-') {
                    sign = -1;
                    pos++;
                }

                oldpos = pos;
                index = 0;
                while (*pos >= '0' && *pos <= '9') {
                    index = (index * 10) + (*pos - '0');

                    pos++;
                }
                index = index * sign;

                if (pos == oldpos) goto format_error;

                mlist_append(indexname_list, (void*)index);
            } else {
                /* process RAW_NAME */
                if (*pos == '_' || *pos == '$' || isalnum(*pos) || *(unsigned char*)pos > 127) {
                    if (mlist_length(indexname_list) > 0) goto format_error;

                    if (!start) start = pos;
                    pos++;
                } else goto format_error;
            }
            break;
        }
    }

    if (start) {
        if (!end) end = pos;
        if (end > start) {
            xnode = _walk_by_name(node, start, end - start, create);
            if (!xnode && !create) {
                /* RAW_NAME not found, and don't create, return NULL */
                mlist_destroy(&indexname_list);
                *rnode = NULL;
                return MERR_OK;
            }
        }
    }

    MLIST_ITERATE(indexname_list, index) {
        if (xnode) xnode = _walk_by_index(xnode, index, create);
        else if (node) xnode = _walk_by_index(node, index, create);

        if (!xnode) break;
    }
    mlist_destroy(&indexname_list);

    if (pos == path) goto format_error;
    else if(pos && *pos == '.' && xnode)
        return merr_pass(_walk_mdf(xnode, pos + 1, create, rnode));
    else *rnode = xnode;

    return MERR_OK;

format_error:
    mlist_destroy(&indexname_list);
    return merr_raise(MERR_ASSERT, "unexpect path %s", pos);
}

static MERR* _copy_mdf(MDF *dst, MDF *src, bool overwrite)
{
    MDF *cnode, *newnode;
    MERR *err;

    if (src->type != MDF_TYPE_ARRAY && src->type != MDF_TYPE_OBJECT) {
        if (dst->type == MDF_TYPE_STRING || dst->type == MDF_TYPE_BINARY) {
            mos_free(dst->val.s);
        } else if (dst->type == MDF_TYPE_OBJECT || dst->type == MDF_TYPE_ARRAY) {
            mdf_destroy(&dst->child);
            dst->last_child = NULL;
        }

        dst->type = src->type;
        if (src->type == MDF_TYPE_STRING && src->val.s) {
            dst->val.s = strdup(src->val.s);
            dst->valuelen = src->valuelen;
        } else if (src->type == MDF_TYPE_BINARY && src->val.s) {
            dst->val.s = mos_calloc(1, src->valuelen);
            memcpy(dst->val.s, src->val.s, src->valuelen);
            dst->valuelen = src->valuelen;
        } else {
            dst->val.n = src->val.n;
            dst->val.f = src->val.f;
        }

        return MERR_OK;
    }

    cnode = src->child;
    while (cnode) {
        MDF *dnode;
        err = _walk_mdf(dst, cnode->name, false, &dnode);
        TRACE_NOK(err);
        if (dnode) {
            if (overwrite) {
                _mdf_drop_child_node(dst, dnode);
            } else goto nextnode;
        }

        mdf_init(&newnode);
        newnode->name = strdup(cnode->name);
        newnode->namelen = strlen(cnode->name);
        newnode->type = cnode->type;
        newnode->val.n = cnode->val.n;
        newnode->val.f = cnode->val.f;
        if (cnode->type == MDF_TYPE_STRING && cnode->val.s) {
            newnode->val.s = strdup(cnode->val.s);
            newnode->valuelen = cnode->valuelen;
        } else if (cnode->type == MDF_TYPE_BINARY && cnode->val.s) {
            newnode->val.s = mos_calloc(1, cnode->valuelen);
            memcpy(newnode->val.s, cnode->val.s, cnode->valuelen);
            newnode->valuelen = cnode->valuelen;
        }
        _mdf_append_child_node(dst, newnode, -1);

        if ((cnode->type == MDF_TYPE_OBJECT || cnode->type == MDF_TYPE_ARRAY) && cnode->child)
            _copy_mdf(newnode, cnode, overwrite);

    nextnode:
        cnode = cnode->next;
    }

    dst->type = src->type;

    return MERR_OK;
}

MERR* mdf_init(MDF **node)
{
    MDF *lnode;

    MERR_NOT_NULLA(node);

    lnode = mos_calloc(1, sizeof(MDF));
    lnode->namelen = lnode->valuelen = 0;
    lnode->name = NULL;
    lnode->type = MDF_TYPE_UNKNOWN;
    lnode->val.s = NULL;
    lnode->val.n = 0;
    lnode->val.f = 0.0;

    lnode->table = NULL;

    lnode->prev = NULL;
    lnode->next = NULL;

    lnode->parent = NULL;
    lnode->child = NULL;
    lnode->last_child = NULL;

    *node = lnode;

    MDF_VALUE_UNKNOWN();

    return MERR_OK;
}

void mdf_destroy(MDF **node)
{
    MDF *lnode, *nnode, *next;

    if (!node || !*node) return;

    lnode = *node;

    if (lnode->child) {
        mdf_destroy(&lnode->child);
        lnode->last_child = NULL;
    }

    nnode = lnode->next;
    while (nnode) {
        next = nnode->next;

        nnode->next = NULL;
        mdf_destroy(&nnode);

        nnode = next;
    }

    if (lnode->table) mhash_destroy(&lnode->table);

    if (lnode->type == MDF_TYPE_STRING ||
        lnode->type == MDF_TYPE_BINARY)
        mos_free(lnode->val.s);

    mos_free(lnode->name);
    mos_free(lnode);

    *node = NULL;
}

void mdf_clear(MDF *node)
{
    if (!node) return;

    if (node->child) {
        mdf_destroy(&node->child);
        node->last_child = NULL;
    }

    MDF *nnode = node->next;
    while (nnode) {
        MDF *next = nnode->next;

        nnode->next = NULL;
        mdf_destroy(&nnode);

        nnode = next;
    }

    if (node->table) mhash_destroy(&node->table);

    if (node->type == MDF_TYPE_STRING || node->type == MDF_TYPE_BINARY)
        mos_free(node->val.s);
    mos_free(node->name);

    node->namelen = node->valuelen = 0;
    node->name = NULL;
    node->type = MDF_TYPE_UNKNOWN;
    node->val.s = NULL;
    node->val.n = 0;
    node->val.f = 0.0;

    node->table = NULL;

    node->prev = NULL;
    node->next = NULL;

    node->parent = NULL;
    node->child = NULL;
    node->last_child = NULL;
}

bool mdf_equal(MDF *anode, MDF *bnode)
{
    float epsilon = anode->type == MDF_TYPE_FLOAT ? 1e-4: 0;

    if (!anode && !bnode) return true;
    if (!anode || !bnode) return false;

    /*
     * mdf_int() 初始化的空节点类型为 MDF_TYPE_UNKNOWN, json 输出使会转换成 MDF_TYPE_NULL
     * 此处，做一个 dirty 兼容
     */
    if (anode->type == MDF_TYPE_UNKNOWN) anode->type = MDF_TYPE_NULL;
    if (bnode->type == MDF_TYPE_UNKNOWN) bnode->type = MDF_TYPE_NULL;

    if (anode->type != bnode->type) return false;
    if (mdf_child_count(anode, NULL) != mdf_child_count(bnode, NULL)) return false;

    switch (anode->type) {
    case MDF_TYPE_STRING:
    case MDF_TYPE_BINARY:
        if (anode->valuelen != bnode->valuelen ||
            (anode->valuelen > 0 && memcmp(anode->val.s, bnode->val.s, anode->valuelen))) {
            return false;
        }
        break;
    case MDF_TYPE_INT:
    case MDF_TYPE_BOOL:
        if (anode->val.n != bnode->val.n) return false;
        break;
    case MDF_TYPE_FLOAT:
        if (fabs((double)(anode->val.f - bnode->val.f)) > epsilon) return false;
        break;
    default:
        break;
    }

    MDF *cnode = anode->child;
    MDF *dnode = bnode->child;
    while (cnode) {
        /* mdf 数组即为特殊key的对象, 故 anode->name 与 bnode->name 不能做相等比较 */
        dnode = mdf_get_node(bnode, cnode->name) == NULL ? dnode: mdf_get_node(bnode, cnode->name);

        if (!mdf_equal(cnode, dnode)) return false;

        cnode = cnode->next;
        dnode = dnode->next;
    }

    return true;
}


MERR* mdf_set_value(MDF *node, const char *path, const char *value)
{
    MDF *anode;
    MERR *err;

    MERR_NOT_NULLA(node);

    err = _walk_mdf(node, path, true, &anode);
    if (err) return merr_pass(err);

    if (anode->type == MDF_TYPE_STRING || anode->type == MDF_TYPE_BINARY)
        mos_free(anode->val.s);
    if (anode->type == MDF_TYPE_OBJECT || anode->type == MDF_TYPE_ARRAY) {
        mdf_destroy(&anode->child);
        anode->last_child = NULL;
    }
    anode->type = MDF_TYPE_STRING;
    if (value) {
        anode->val.s = strdup(value);
        anode->valuelen = strlen(value);
    } else {
        anode->val.s = NULL;
        anode->valuelen = 0;
    }

    return MERR_OK;
}

MERR* mdf_set_int_value(MDF *node, const char *path, int value)
{
    MDF *anode;
    MERR *err;

    MERR_NOT_NULLA(node);

    err = _walk_mdf(node, path, true, &anode);
    if (err) return merr_pass(err);

    if (anode->type == MDF_TYPE_STRING || anode->type == MDF_TYPE_BINARY)
        mos_free(anode->val.s);
    if (anode->type == MDF_TYPE_OBJECT || anode->type == MDF_TYPE_ARRAY) {
        mdf_destroy(&anode->child);
        anode->last_child = NULL;
    }
    anode->type = MDF_TYPE_INT;
    anode->val.n = (int64_t)value;

    return MERR_OK;
}

MERR* mdf_set_int64_value(MDF *node, const char *path, int64_t value)
{
    MDF *anode;
    MERR *err;

    MERR_NOT_NULLA(node);

    err = _walk_mdf(node, path, true, &anode);
    if (err) return merr_pass(err);

    if (anode->type == MDF_TYPE_STRING || anode->type == MDF_TYPE_BINARY)
        mos_free(anode->val.s);
    if (anode->type == MDF_TYPE_OBJECT || anode->type == MDF_TYPE_ARRAY) {
        mdf_destroy(&anode->child);
        anode->last_child = NULL;
    }
    anode->type = MDF_TYPE_INT;
    anode->val.n = value;

    return MERR_OK;
}

MERR* mdf_set_float_value(MDF *node, const char *path, float value)
{
    MDF *anode;
    MERR *err;

    MERR_NOT_NULLA(node);

    err = _walk_mdf(node, path, true, &anode);
    if (err) return merr_pass(err);

    if (anode->type == MDF_TYPE_STRING || anode->type == MDF_TYPE_BINARY)
        mos_free(anode->val.s);
    if (anode->type == MDF_TYPE_OBJECT || anode->type == MDF_TYPE_ARRAY) {
        mdf_destroy(&anode->child);
        anode->last_child = NULL;
    }
    anode->type = MDF_TYPE_FLOAT;
    anode->val.f = value;

    return MERR_OK;
}

MERR* mdf_set_bool_value(MDF *node, const char *path, bool value)
{
    MDF *anode;
    MERR *err;

    MERR_NOT_NULLA(node);

    err = _walk_mdf(node, path, true, &anode);
    if (err) return merr_pass(err);

    if (anode->type == MDF_TYPE_STRING || anode->type == MDF_TYPE_BINARY)
        mos_free(anode->val.s);
    if (anode->type == MDF_TYPE_OBJECT || anode->type == MDF_TYPE_ARRAY) {
        mdf_destroy(&anode->child);
        anode->last_child = NULL;
    }
    anode->type = MDF_TYPE_BOOL;
    anode->val.n = value;

    return MERR_OK;
}

MERR* mdf_set_binary(MDF *node, const char *path, const unsigned char *buf, size_t len)
{
    MDF *anode;
    MERR *err;

    MERR_NOT_NULLA(node);

    err = _walk_mdf(node, path, true, &anode);
    if (err) return merr_pass(err);

    if (anode->type == MDF_TYPE_STRING || anode->type == MDF_TYPE_BINARY)
        mos_free(anode->val.s);
    if (anode->type == MDF_TYPE_OBJECT || anode->type == MDF_TYPE_ARRAY) {
        mdf_destroy(&anode->child);
        anode->last_child = NULL;
    }
    anode->type = MDF_TYPE_BINARY;
    anode->val.s = mos_calloc(1, len);
    memcpy(anode->val.s, buf, len);
    anode->valuelen = len;

    return MERR_OK;
}

MERR* mdf_set_binary_noalloc(MDF *node, const char *path, unsigned char *buf, size_t len)
{
    MDF *anode;
    MERR *err;

    MERR_NOT_NULLA(node);

    err = _walk_mdf(node, path, true, &anode);
    if (err) return merr_pass(err);

    if (anode->type == MDF_TYPE_STRING || anode->type == MDF_TYPE_BINARY)
        mos_free(anode->val.s);
    if (anode->type == MDF_TYPE_OBJECT || anode->type == MDF_TYPE_ARRAY) {
        mdf_destroy(&anode->child);
        anode->last_child = NULL;
    }
    anode->type = MDF_TYPE_BINARY;
    anode->val.s = (char*)buf;
    anode->valuelen = len;

    return MERR_OK;
}

int mdf_add_int_value(MDF *node, const char *path, int val)
{
    if (!node) return 0;

    int ov = mdf_get_int_value(node, path, 0);
    mdf_set_int_value(node, path, ov + val);

    return ov + val;
}

int64_t mdf_add_int64_value(MDF *node, const char *path, int64_t val)
{
    if (!node) return 0;

    int64_t ov = mdf_get_int64_value(node, path, 0);
    mdf_set_int64_value(node, path, ov + val);

    return ov + val;
}

float mdf_add_float_value(MDF *node, const char *path, float val)
{
    if (!node) return 0;

    float ov = mdf_get_float_value(node, path, 0);
    mdf_set_float_value(node, path, ov + val);

    return ov + val;
}

char* mdf_append_string_value(MDF *node, const char *path, char *str)
{
    if (!node) return NULL;

    char *ov = mdf_get_value(node, path, "");
    mdf_set_valuef(node, "%s=%s%s", path, ov, str);

    return mdf_get_value(node, path, NULL);
}

char* mdf_preppend_string_value(MDF *node, const char *path, char *str)
{
    if (!node) return NULL;

    char *ov = mdf_get_value(node, path, "");
    mdf_set_valuef(node, "%s=%s%s", path, str, ov);

    return mdf_get_value(node, path, NULL);
}

void mdf_set_type(MDF *node, const char *path, MDF_TYPE type)
{
    MDF *anode;
    char *s;
    MERR *err;

    if (!node) return;

    err = _walk_mdf(node, path, false, &anode);
    TRACE_NOK(err);

    if (!anode || (anode->type != MDF_TYPE_STRING && anode->type != MDF_TYPE_UNKNOWN)) return;

    s = anode->val.s;

    switch (type) {
    case MDF_TYPE_OBJECT:
    case MDF_TYPE_ARRAY:
        mdf_destroy(&anode->child);
        anode->last_child = NULL;
        break;
    case MDF_TYPE_INT:
        if (anode->val.s) anode->val.n = strtoll(anode->val.s, NULL, 10);
        else anode->val.n = 0;
        break;
    case MDF_TYPE_FLOAT:
        if (anode->val.s) anode->val.f = strtof(anode->val.s, NULL);
        else anode->val.f = 0.0;
        break;
    case MDF_TYPE_BOOL:
        if (anode->val.s) anode->val.n = 1;
        else anode->val.n = 0;
        break;
    case MDF_TYPE_NULL:
        anode->val.n = 0;
        break;
    default:
        break;
    }

    mos_free(s);
    anode->type = type;
}

void mdf_set_digit_type(MDF *node, const char *path, MDF_TYPE type)
{
    MDF *anode;
    MERR *err;

    if (!node) return;

    err = _walk_mdf(node, path, false, &anode);
    TRACE_NOK(err);

    if (!anode || (anode->type != MDF_TYPE_INT &&
                   anode->type != MDF_TYPE_FLOAT &&
                   anode->type != MDF_TYPE_BOOL) ||
        (type != MDF_TYPE_INT &&
         type != MDF_TYPE_FLOAT &&
         type != MDF_TYPE_BOOL)) return;

    if (anode->type != type) {
        anode->type = type;
        switch (type) {
        case MDF_TYPE_INT:
            anode->val.n = (int) anode->val.f;
            break;
        case MDF_TYPE_FLOAT:
            anode->val.f = (float) anode->val.n;
            break;
        case MDF_TYPE_BOOL:
            if (anode->val.n || anode->val.f) anode->val.n = 1;
            else anode->val.n = 0;
            break;
        default:
            break;
        }
    }
}

void mdf_set_type_revert(MDF *node, const char *path)
{
    MERR *err;
    MDF *anode;

    if (!node) return;

    err = _walk_mdf(node, path, false, &anode);
    TRACE_NOK(err);

    if (!anode || anode->type == MDF_TYPE_STRING) return;

    anode->val.s = mdf_get_value_stringfy(anode, NULL, NULL);
    anode->valuelen = anode->val.s ? strlen(anode->val.s) : 0;
    anode->type = MDF_TYPE_STRING;
}

void mdf_object_2_array(MDF *node, const char *path)
{
    MDF *anode;
    MERR *err;

    if (!node) return;

    err = _walk_mdf(node, path, false, &anode);
    TRACE_NOK(err);

    if (!anode || anode->type != MDF_TYPE_OBJECT) return;

    anode->type = MDF_TYPE_ARRAY;
}

void mdf_array_2_object(MDF *node, const char *path)
{
    MDF *anode;
    MERR *err;

    if (!node) return;

    err = _walk_mdf(node, path, false, &anode);
    TRACE_NOK(err);

    if (!anode || anode->type != MDF_TYPE_ARRAY) return;

    anode->type = MDF_TYPE_OBJECT;
}

MDF_TYPE mdf_get_type(MDF *node, const char *path)
{
    MDF *anode;
    MERR *err;

    if (!node) return MDF_TYPE_UNKNOWN;

    err = _walk_mdf(node, path, false, &anode);
    TRACE_NOK(err);

    if (anode) return anode->type;
    else return MDF_TYPE_UNKNOWN;
}

char* mdf_get_name(MDF *node, const char *path)
{
    MDF *anode;
    MERR *err;

    if (!node) return NULL;

    err = _walk_mdf(node, path, false, &anode);
    TRACE_NOK(err);

    if (anode) return anode->name;
    else return NULL;
}

char* mdf_get_value(MDF *node, const char *path, char *dftvalue)
{
    MDF *anode;
    MERR *err;

    if (!node) return dftvalue;

    err = _walk_mdf(node, path, false, &anode);
    TRACE_NOK(err);

    if (anode && anode->type == MDF_TYPE_STRING) return anode->val.s;
    else return dftvalue;
}

char* mdf_get_value_copy(MDF *node, const char *path, char *dftvalue)
{
    MDF *anode;
    MERR *err;

    if (!node) return dftvalue;

    err = _walk_mdf(node, path, false, &anode);
    TRACE_NOK(err);

    if (anode && anode->type == MDF_TYPE_STRING && anode->val.s)
        return strdup(anode->val.s);
    else return dftvalue;
}

int mdf_get_int_value(MDF *node, const char *path, int dftvalue)
{
    MDF *anode;
    MERR *err;

    if (!node) return dftvalue;

    err = _walk_mdf(node, path, false, &anode);
    TRACE_NOK(err);

    if (anode && anode->type == MDF_TYPE_INT) return (int)anode->val.n;
    else return dftvalue;
}

int64_t mdf_get_int64_value(MDF *node, const char *path, int64_t dftvalue)
{
    MDF *anode;
    MERR *err;

    if (!node) return dftvalue;

    err = _walk_mdf(node, path, false, &anode);
    TRACE_NOK(err);

    if (anode && anode->type == MDF_TYPE_INT) return anode->val.n;
    else return dftvalue;
}

float mdf_get_float_value(MDF *node, const char *path, float dftvalue)
{
    MDF *anode;
    MERR *err;

    if (!node) return dftvalue;

    err = _walk_mdf(node, path, false, &anode);
    TRACE_NOK(err);

    if (anode && anode->type == MDF_TYPE_FLOAT) return anode->val.f;
    else return dftvalue;
}

bool mdf_get_bool_value(MDF *node, const char *path, bool dftvalue)
{
    MDF *anode;
    MERR *err;

    if (!node) return dftvalue;

    err = _walk_mdf(node, path, false, &anode);
    TRACE_NOK(err);

    if (anode && anode->type == MDF_TYPE_BOOL) return (bool)anode->val.n;
    else return dftvalue;
}

unsigned char* mdf_get_binary(MDF *node, const char *path, size_t *len)
{
    MDF *anode;
    MERR *err;

    if (!node) return NULL;

    err = _walk_mdf(node, path, false, &anode);
    TRACE_NOK(err);

    if (anode && anode->type == MDF_TYPE_BINARY) {
        *len = anode->valuelen;
        return (unsigned char*)anode->val.s;
    } else {
        *len = 0;
        return NULL;
    }
}

char* mdf_get_value_stringfy(MDF *node, const char *path, char *dftvalue)
{
    char tok[64];
    MDF *anode;
    MERR *err;

    if (!node) return dftvalue;

    err = _walk_mdf(node, path, false, &anode);
    TRACE_NOK(err);

    if (anode) {
        switch(anode->type) {
        case MDF_TYPE_STRING:
            if (anode->val.s) return strdup(anode->val.s);
            else goto dup_default;
        case MDF_TYPE_INT:
            snprintf(tok, sizeof(tok), "%lld", (long long)anode->val.n);
            return strdup(tok);
        case MDF_TYPE_FLOAT:
            snprintf(tok, sizeof(tok), "%f", anode->val.f);
            return strdup(tok);
        case MDF_TYPE_BOOL:
            snprintf(tok, sizeof(tok), "%lld", (long long)anode->val.n);
            return strdup(tok);
        case MDF_TYPE_NULL:
            tok[0] = '0';
            tok[1] = '\0';
            return strdup(tok);
        default:
            goto dup_default;
        }
    }

dup_default:
    if (dftvalue) {
        return strdup(dftvalue);
    } else return NULL;
}


MERR* mdf_copy(MDF *dst, const char *path, MDF *src, bool overwrite)
{
    MDF *anode;
    MERR *err;

    MERR_NOT_NULLB(dst, src);

    err = _walk_mdf(dst, path, true, &anode);
    if (err) return merr_pass(err);

    return merr_pass(_copy_mdf(anode, src, overwrite));
}

MERR* mdf_remove(MDF *node, const char *path)
{
    MDF *pnode, *cnode;
    MERR *err;

    MERR_NOT_NULLA(node);

    if (!path || !*path) return MERR_OK;

    err = _walk_mdf(node, path, false, &cnode);
    if (err) return merr_pass(err);

    if (cnode) pnode = cnode->parent;

    if (cnode && pnode) _mdf_drop_child_node(pnode, cnode);

    return MERR_OK;
}


MDF* mdf_get_node(MDF *node, const char *path)
{
    MDF *anode;
    MERR *err;

    if (!node) return NULL;

    err = _walk_mdf(node, path, false, &anode);
    TRACE_NOK(err);

    return anode;
}

MDF* mdf_get_or_create_node(MDF *node, const char *path)
{
    MDF *anode;
    MERR *err;

    if (!node) return NULL;

    err = _walk_mdf(node, path, true, &anode);
    TRACE_NOK(err);

    return anode;
}

MDF* mdf_get_child(MDF *node, const char *path)
{
    MDF *anode;
    MERR *err;

    if (!node) return NULL;

    err = _walk_mdf(node, path, false, &anode);
    TRACE_NOK(err);

    if (anode) return anode->child;
    else return NULL;
}

MDF* mdf_node_next(MDF *node)
{
    if (!node) return NULL;

    return node->next;
}

MDF* mdf_node_parent(MDF *node)
{
    if (!node) return NULL;

    return node->parent;
}

MDF* mdf_node_child(MDF *node)
{
    if (!node) return NULL;

    return node->child;
}


void mdf_sort_node(MDF *node, int __F(compare)(const void*, const void*))
{
    if (!node || !node->child || !compare) return;

    MLIST *alist;

    mlist_init(&alist, NULL);

    MDF *cnode = node->child;
    while (cnode) {
        mlist_append(alist, cnode);
        cnode = cnode->next;
    }

    mlist_sort(alist, compare);

    MDF *pnode, *xnode;
    MLIST_ITERATE(alist, xnode) {
        xnode->next = NULL;

        if (_moon_i == 0) {
            node->child = pnode = xnode;
        } else {
            pnode->next = xnode;;
        }

        if (_moon_i == mlist_length(alist) - 1) {
            node->last_child = xnode;
        }

        pnode = xnode;
    }

    mlist_destroy(&alist);
}


bool mdf_path_exist(MDF *node, const char *path)
{
    MDF *anode;
    MERR *err;

    if (!node) return false;

    err = _walk_mdf(node, path, false, &anode);
    TRACE_NOK(err);

    if (anode) return true;
    else return false;
}

bool mdf_leaf_node(MDF *node, const char *path)
{
    MDF *anode;
    MERR *err;

    if (!node) return true;

    err = _walk_mdf(node, path, false, &anode);
    TRACE_NOK(err);

    if (!node || !node->child) return true;
    else return false;
}

int mdf_child_count(MDF *node, const char *path)
{
    MDF *anode;
    MERR *err;

    err = _walk_mdf(node, path, false, &anode);
    TRACE_NOK(err);

    if (anode) {
        switch (anode->type) {
        case MDF_TYPE_OBJECT:
        case MDF_TYPE_ARRAY:
            if (anode->table) return mhash_length(anode->table);
            else {
                int num = 0;
                for (MDF *cnode = anode->child; cnode; cnode = cnode->next) num++;
                return num;
            }
            break;
        default:
            return 0;
        }
    } else return 0;
}
