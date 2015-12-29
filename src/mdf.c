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

    if (node->type == MDF_TYPE_STRING || MDF_TYPE_BINARY) mos_free(node->val.s);
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
    int64_t index;
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
                if (*pos == '_' || *pos == '$' || isalnum(*pos)) {
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

static MERR* _copy_mdf(MDF *dst, MDF *src)
{
    int childnum;
    MDF *cnode, *newnode;

    childnum = 0;

    cnode = src->child;
    while (cnode) {
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

        _mdf_append_child_node(dst, newnode, childnum);

        _copy_mdf(newnode, cnode);

        childnum++;
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

    return MERR_OK;
}

void mdf_destroy(MDF **node)
{
    MDF *lnode, *cnode, *next;

    if (!node || !*node) return;

    lnode = *node;

    if (lnode->child) {
        mdf_destroy(&lnode->child);
        lnode->last_child = NULL;
    }

    cnode = lnode->next;
    while (cnode) {
        next = cnode->next;

        cnode->next = NULL;
        mdf_destroy(&cnode);

        cnode = next;
    }

    if (lnode->table) mhash_destroy(&lnode->table);

    if (lnode->type == MDF_TYPE_STRING ||
        lnode->type == MDF_TYPE_BINARY)
        mos_free(lnode->val.s);

    mos_free(lnode->name);
    mos_free(lnode);

    *node = NULL;
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

MERR* mdf_set_type(MDF *node, const char *path, MDF_TYPE type)
{
    MDF *anode;
    char *s;
    MERR *err;

    MERR_NOT_NULLA(node);

    err = _walk_mdf(node, path, false, &anode);
    if (err) return merr_pass(err);

    if (!anode || anode->type != MDF_TYPE_STRING)
        return merr_raise(MERR_ASSERT, "node type not string");

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

    return MERR_OK;
}

MERR* mdf_object_2_array(MDF *node, const char *path)
{
    MDF *anode;
    MERR *err;

    MERR_NOT_NULLA(node);

    err = _walk_mdf(node, path, false, &anode);
    if (err) return merr_pass(err);

    if (!anode || anode->type != MDF_TYPE_OBJECT)
        return merr_raise(MERR_ASSERT, "node type not object");

    anode->type = MDF_TYPE_ARRAY;

    return MERR_OK;
}

MERR* mdf_array_2_object(MDF *node, const char *path)
{
    MDF *anode;
    MERR *err;

    MERR_NOT_NULLA(node);

    err = _walk_mdf(node, path, false, &anode);
    if (err) return merr_pass(err);

    if (!anode || anode->type != MDF_TYPE_ARRAY)
        return merr_raise(MERR_ASSERT, "node type not array");

    anode->type = MDF_TYPE_OBJECT;

    return MERR_OK;
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
            snprintf(tok, sizeof(tok), "%ld", anode->val.n);
            return strdup(tok);
        case MDF_TYPE_FLOAT:
            snprintf(tok, sizeof(tok), "%f", anode->val.f);
            return strdup(tok);
        case MDF_TYPE_BOOL:
            snprintf(tok, sizeof(tok), "%ld", anode->val.n);
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


MERR* mdf_copy(MDF *dst, const char *path, MDF *src)
{
    MDF *anode;
    MERR *err;

    MERR_NOT_NULLB(dst, src);

    err = _walk_mdf(dst, path, true, &anode);
    if (err) return merr_pass(err);

    return merr_pass(_copy_mdf(anode, src));
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


MDF* mdf_sort_node(MDF *node, int __F(compare)(const void*, const void*))
{
    /* TODO sort */

    return node;
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
