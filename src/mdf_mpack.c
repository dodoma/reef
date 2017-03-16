#include "reef.h"
#include "_mdf.h"

static inline MPACK_FORMAT _mpack_format(unsigned char *buf)
{
    unsigned char format = *buf;

    if (format < 0x80) return F_POSITIVE_FIX_INT;
    else if (format < 0x90) return F_FIX_MAP;
    else if (format < 0xA0) return F_FIX_ARRAY;
    else if (format < 0xC0) return F_FIX_STR;
    else if (format >= 0xE0) return F_NEGATIVE_FIX_INT;

    switch (format) {
    case 0xC0:
        return F_NIL;
    case 0xC2:
        return F_FALSE;
    case 0xC3:
        return F_TRUE;

    case 0xC4:
        return F_BIN_8;
    case 0xC5:
        return F_BIN_16;
    case 0xC6:
        return F_BIN_32;

    case 0xC7:
        return F_EXT_8;
    case 0xC8:
        return F_EXT_16;
    case 0xC9:
        return F_EXT_32;

    case 0xCA:
        return F_FLOAT32;
    case 0xCB:
        return F_FLOAT64;

    case 0xCC:
        return F_UINT_8;
    case 0xCD:
        return F_UINT_16;
    case 0xCE:
        return F_UINT_32;
    case 0xCF:
        return F_UINT_64;

    case 0xD0:
        return F_INT_8;
    case 0xD1:
        return F_INT_16;
    case 0xD2:
        return F_INT_32;
    case 0xD3:
        return F_INT_64;

    case 0xD4:
        return F_FIX_EXT_1;
    case 0xD5:
        return F_FIX_EXT_2;
    case 0xD6:
        return F_FIX_EXT_4;
    case 0xD7:
        return F_FIX_EXT_8;
    case 0xD8:
        return F_FIX_EXT_16;

    case 0xD9:
        return F_STR_8;
    case 0xDA:
        return F_STR_16;
    case 0xDB:
        return F_STR_32;

    case 0xDC:
        return F_ARRAY_16;
    case 0xDD:
        return F_ARRAY_32;
    case 0xDE:
        return F_MAP_16;
    case 0xDF:
        return F_MAP_32;

    default:
        return F_UNKNOWN;
    }
}

static inline uint8_t _mpack_load_u8(unsigned char *p)
{
    return (uint8_t)p[0];
}

static inline uint16_t _mpack_load_u16(unsigned char *p)
{
    uint16_t val;
    memcpy(&val, p, sizeof(val));
    return val;
}

static inline uint32_t _mpack_load_u32(unsigned char *p)
{
    uint32_t val;
    memcpy(&val, p, sizeof(val));
    return val;
}

static inline uint64_t _mpack_load_u64(unsigned char *p)
{
    uint64_t val;
    memcpy(&val, p, sizeof(val));
    return val;
}

static inline int _mpack_get_fix_int(unsigned char *buf)
{
    return (int8_t)_mpack_load_u8(buf);
}

static inline int64_t _mpack_get_int(unsigned char *buf, size_t *step)
{
    uint8_t type = _mpack_load_u8(buf);

    switch (type) {
    case 0xD0:
        *step = 2;
        return (int8_t)_mpack_load_u8(buf+1);
    case 0xD1:
        *step = 3;
        return (int16_t)_mpack_load_u16(buf+1);
    case 0xD2:
        *step = 5;
        return (int32_t)_mpack_load_u32(buf+1);
    case 0xD3:
        *step = 9;
        return (int64_t)_mpack_load_u64(buf+1);
    }

    return 0;
}

static inline uint _mpack_get_uint(unsigned char *buf, size_t *step)
{
    uint8_t type = _mpack_load_u8(buf);

    switch (type) {
    case 0xCC:
        *step = 2;
        return (uint8_t)_mpack_load_u8(buf+1);
    case 0xCD:
        *step = 3;
        return (uint16_t)_mpack_load_u16(buf+1);
    case 0xCE:
        *step = 5;
        return (uint32_t)_mpack_load_u32(buf+1);
    case 0xCF:
        *step = 9;
        return (uint64_t)_mpack_load_u64(buf+1);
    }

    return 0;
}

static inline float _mpack_get_float(unsigned char *buf, size_t *step)
{
    uint8_t type = _mpack_load_u8(buf);

    switch (type) {
    case 0xCA:
    {
        union {
            float f;
            uint32_t u;
        } v;
        memcpy(&v, buf + 1, sizeof(uint32_t));

        *step = 5;
        return v.f;
    }
    case 0xCB:
    {
        union {
            double d;
            uint64_t u;
        } v;
        memcpy(&v, buf + 1, sizeof(uint64_t));

        *step = 9;
        /* TODO need MDF_TYPE_DOUBLE ? */
        return (float) v.d;
    }
    default:
        break;
    }

    return 0;
}

static inline unsigned char* _mpack_get_str(unsigned char *buf, size_t *step, int *slen)
{
    uint8_t type = _mpack_load_u8(buf);

    switch (type) {
    case 0xD9:
        *slen = _mpack_load_u8(buf+1);
        *step = 2 + *slen;
        return buf + 2;
    case 0xDA:
        *slen = _mpack_load_u16(buf+1);
        *step = 3 + *slen;
        return buf + 3;
    case 0xDB:
        *slen = _mpack_load_u32(buf+1);
        *step = 5 + *slen;
        return buf + 5;
    default:
        *slen = _mpack_load_u8(buf);
        *slen = *slen & 0x1F;
        *step = 1 + *slen;
        return buf + 1;
    }
}

static inline unsigned char* _mpack_get_binary(unsigned char *buf, size_t *step, int *blen)
{
    uint8_t type = _mpack_load_u8(buf);

    switch (type) {
    case 0xC4:
        *blen = _mpack_load_u8(buf+1);
        *step = 2 + *blen;
        return buf + 2;
    case 0xC5:
        *blen = _mpack_load_u16(buf+1);
        *step = 3 + *blen;
        return buf + 3;
    case 0xC6:
        *blen = _mpack_load_u32(buf+1);
        *step = 5 + *blen;
        return buf + 5;
    }
    return NULL;
}

static inline int _mpack_get_array_count(unsigned char *buf, size_t *step)
{
    uint8_t type = _mpack_load_u8(buf);

    switch (type) {
    case 0xDC:
        *step = 3;
        return _mpack_load_u16(buf+1);
    case 0xDD:
        *step = 5;
        return _mpack_load_u32(buf+1);
    default:
        *step = 1;
        return type & 0xF;
    }
}

static inline int _mpack_get_map_count(unsigned char *buf, size_t *step)
{
    uint8_t type = _mpack_load_u8(buf);

    switch (type) {
    case 0xDE:
        *step = 3;
        return _mpack_load_u16(buf+1);
    case 0xDF:
        *step = 5;
        return _mpack_load_u32(buf+1);
    default:
        *step = 1;
        return type & 0xF;
    }
}

static inline size_t _string_serialize(size_t len, char *val, unsigned char *pos)
{
    if (len <= 0x1F) {
        *pos = 0xA0 + len;
        memcpy(pos + 1, val, len);
        return 1 + len;
    } else if (len <= 0xFF) {
        *pos = 0xD9;
        *(uint8_t*)(pos+1) = len;
        memcpy(pos + 2, val, len);
        return 2 + len;
    } else if (len <= 0xFFFF) {
        *pos = 0xDA;
        *(uint16_t*)(pos+1) = len;
        memcpy(pos + 3, val, len);
        return 3 + len;
    } else {
        *pos = 0xDB;
        *(uint32_t*)(pos+1) = len;
        memcpy(pos + 5, val, len);
        return 5 + len;
    }
}

size_t mdf_mpack_serialize(MDF *node, unsigned char *buf, size_t len)
{
    int nodenum;
    unsigned char *pos;
    size_t mylen, step;
    MDF *cnode;

    if (!node || !buf || len <= 0) return 0;

    mylen = step = 0;
    pos = buf;
    nodenum = 0;

    switch (node->type) {
    case MDF_TYPE_OBJECT:
        nodenum = mdf_child_count(node, NULL);
        if (nodenum <= 0xF) {
            *pos = 0x80 + nodenum;
            step = 1;
        } else if (nodenum <= 0xFFFF) {
            *pos = 0xDE;
            *(uint16_t*)(pos+1) = nodenum;
            step = 3;
        } else {
            *pos = 0xDF;
            *(uint32_t*)(pos+1) = nodenum;
            step = 5;
        }

        mylen = step;
        pos += step;
        cnode = node->child;
        while (cnode && mylen < len) {
            step = _string_serialize(cnode->namelen, cnode->name, pos);
            step += mdf_mpack_serialize(cnode, pos + step, len - mylen);
            pos += step;
            mylen += step;
            step = 0;

            cnode = cnode->next;
        }
        break;
    case MDF_TYPE_ARRAY:
        nodenum = mdf_child_count(node, NULL);
        if (nodenum <= 0xF) {
            *pos = 0x90 + nodenum;
            step = 1;
        } else if (nodenum <= 0xFFFF) {
            *pos = 0xDC;
            *(uint16_t*)(pos+1) = nodenum;
            step = 3;
        } else {
            *pos = 0xDD;
            *(uint32_t*)(pos+1) = nodenum;
            step = 5;
        }

        mylen = step;
        pos += step;
        cnode = node->child;
        while (cnode && mylen < len) {
            step = mdf_mpack_serialize(cnode, pos, len - mylen);
            pos += step;
            mylen += step;
            step = 0;

            cnode = cnode->next;
        }
        break;
    case MDF_TYPE_STRING:
        mylen = _string_serialize(node->valuelen, node->val.s, pos);
        break;
    case MDF_TYPE_INT:
        if (node->val.n < 0) {
            if (node->val.n >= -0x20) {
                *pos = (int8_t)node->val.n;
                mylen = 1;
            } else if (node->val.n >= -0xFF) {
                *pos = 0xd0;
                *(int8_t*)(pos+1) = node->val.n;
                mylen = 2;
            } else if (node->val.n >= -0xFFFF) {
                *pos = 0xd1;
                *(int16_t*)(pos+1) = node->val.n;
                mylen = 3;
            } else if (node->val.n >= -0xFFFFFFFF) {
                *pos = 0xd2;
                *(int32_t*)(pos+1) = node->val.n;
                mylen = 5;
            } else {
                *pos = 0xd3;
                *(int64_t*)(pos+1) = node->val.n;
                mylen = 9;
            }
        } else {
            if (node->val.n <= 0x80) {
                *pos = node->val.n;
                mylen = 1;
            } else if (node->val.n <= 0xFF) {
                *pos = 0xCC;
                *(uint8_t*)(pos+1) = node->val.n;
                mylen = 2;
            } else if (node->val.n <= 0xFFFF) {
                *pos = 0xCD;
                *(uint16_t*)(pos+1) = node->val.n;
                mylen = 3;
            } else if (node->val.n <= 0xFFFFFFFF) {
                *pos = 0xCE;
                *(uint32_t*)(pos+1) = node->val.n;
                mylen = 5;
            } else {
                *pos = 0xCF;
                *(uint64_t*)(pos+1) = node->val.n;
                mylen = 9;
            }
        }
        break;
    case MDF_TYPE_FLOAT:
        *pos = 0xCA;
        *(float*)(pos+1) = node->val.f;
        mylen = 5;
        break;
    case MDF_TYPE_BOOL:
        if (node->val.n != 0) {
            *pos = 0xC3;
        } else {
            *pos = 0xC2;
        }
        mylen = 1;
        break;
    case MDF_TYPE_BINARY:
        if (node->valuelen < 0xFF) {
            *pos = 0xC4;
            *(uint8_t*)(pos+1) = node->valuelen;
            memcpy(pos + 2, node->val.s, node->valuelen);
            mylen = 2 + node->valuelen;
        } else if (node->valuelen < 0xFFFF) {
            *pos = 0xC5;
            *(uint16_t*)(pos+1) = node->valuelen;
            memcpy(pos + 3, node->val.s, node->valuelen);
            mylen = 3 + node->valuelen;
        } else {
            *pos = 0xC6;
            *(uint32_t*)(pos+1) = node->valuelen;
            memcpy(pos + 5, node->val.s, node->valuelen);
            mylen = 5 + node->valuelen;
        }
        break;
    case MDF_TYPE_NULL:
        *pos = 0xC0;
        mylen = 1;
        break;
    default:
        /* EXT_1 for MDF_TYPE_UNKNOWN */
        *pos = 0xD4;
        mylen = 3;
    }

    return mylen;
}

size_t mdf_mpack_len(MDF *node)
{
    int nodenum;
    size_t mylen, step;
    MDF *cnode;

    if (!node) return 0;

    mylen = step = 0;
    nodenum = 0;

    switch (node->type) {
    case MDF_TYPE_OBJECT:
        nodenum = mdf_child_count(node, NULL);
        if (nodenum <= 0xF) step = 1;
        else if(nodenum <= 0xFFFF) step = 3;
        else step = 5;

        mylen = step;

        cnode = node->child;
        while (cnode) {
            if (cnode->namelen <= 0x1F) {
                step = 1 + cnode->namelen;
            } else if (cnode->namelen <= 0xFF) {
                step = 2 + cnode->namelen;
            } else if (cnode->namelen <= 0xFFFF) {
                step = 3 + cnode->namelen;
            } else {
                step = 5 + cnode->namelen;
            }

            step += mdf_mpack_len(cnode);
            mylen += step;
            step = 0;

            cnode = cnode->next;
        }
        break;
    case MDF_TYPE_ARRAY:
        nodenum = mdf_child_count(node, NULL);
        if (nodenum <= 0xF) step = 1;
        else if(nodenum <= 0xFFFF) step = 3;
        else step = 5;

        mylen = step;

        cnode = node->child;
        while (cnode) {
            mylen += mdf_mpack_len(cnode);

            cnode = cnode->next;
        }
        break;
    case MDF_TYPE_STRING:
        if (node->valuelen <= 0x1F) {
            mylen = 1 + node->valuelen;
        } else if (node->valuelen <= 0xFF) {
            mylen = 2 + node->valuelen;
        } else if (node->valuelen <= 0xFFFF) {
            mylen = 3 + node->valuelen;
        } else {
            mylen = 5 + node->valuelen;
        }
        break;
    case MDF_TYPE_INT:
        if (node->val.n < 0) {
            if (node->val.n >= -0x20) {
                mylen = 1;
            } else if (node->val.n >= -0xFF) {
                mylen = 2;
            } else if (node->val.n >= -0xFFFF) {
                mylen = 3;
            } else if (node->val.n >= -0xFFFFFFFF) {
                mylen = 5;
            } else {
                mylen = 9;
            }
        } else {
            if (node->val.n <= 0x80) {
                mylen = 1;
            } else if (node->val.n <= 0xFF) {
                mylen = 2;
            } else if (node->val.n <= 0xFFFF) {
                mylen = 3;
            } else if (node->val.n <= 0xFFFFFFFF) {
                mylen = 5;
            } else {
                mylen = 9;
            }
        }
        break;
    case MDF_TYPE_FLOAT:
        mylen = 5;
        break;
    case MDF_TYPE_BOOL:
        mylen = 1;
        break;
    case MDF_TYPE_BINARY:
        if (node->valuelen < 0xFF) {
            mylen = 2 + node->valuelen;
        } else if (node->valuelen < 0xFFFF) {
            mylen = 3 + node->valuelen;
        } else {
            mylen = 5 + node->valuelen;
        }
        break;
    case MDF_TYPE_NULL:
        mylen = 1;
        break;
    default:
        /* MDF_TYPE_UNKNOWN */
        mylen = 3;
        break;
    }

    return mylen;
}

size_t mdf_mpack_deserialize(MDF *node, const unsigned char *buf, size_t len)
{
    if (!node || !buf || len <= 0) return 0;

    MDF_TYPE whoami, nodetype;
    MPACK_FORMAT format;
    unsigned char *pos;
    char *name, *values, *valueb, arrayindex[64];
    int64_t valuen;
    float valuef;
    int nodenum_got, nodenum_need, valued_time, namelen, valuelen;
    size_t step, mylen;
    MDF *xnode;

    pos = (unsigned char*)buf;
    mylen = 0;

    nodenum_got = 0;
    nodenum_need = 1;
    valued_time = 0;
    whoami = MDF_TYPE_UNKNOWN;
    nodetype = MDF_TYPE_UNKNOWN;
    xnode = NULL;
    name = values = valueb = NULL;
    valuen = 0;
    valuef = 0.0;
    namelen = valuelen = -1;
    step = 0;

#define AFTER_NODE_APPENDED()                   \
    do {                                        \
        valued_time = 0;                        \
        name = values = NULL;                   \
        namelen = valuelen = -1;                \
        valuen = 0;                             \
        valuef = 0.0;                           \
        nodetype = MDF_TYPE_UNKNOWN;            \
        nodenum_got++;                          \
        xnode = NULL;                           \
    } while (0)

    while (mylen < len && nodenum_got < nodenum_need) {
        format = _mpack_format(pos);
        switch (format) {
        case F_POSITIVE_FIX_INT:
        case F_NEGATIVE_FIX_INT:
            nodetype = MDF_TYPE_INT;
            step = 1;
            valuen = _mpack_get_fix_int(pos);
            valued_time++;
            break;

        case F_INT_8:
        case F_INT_16:
        case F_INT_32:
        case F_INT_64:
            nodetype = MDF_TYPE_INT;
            valuen = _mpack_get_int(pos, &step);
            valued_time++;
            break;

        case F_UINT_8:
        case F_UINT_16:
        case F_UINT_32:
        case F_UINT_64: /* TODO need MDF_TYPE_UINT64? */
            nodetype = MDF_TYPE_INT;
            valuen = _mpack_get_uint(pos, &step);
            valued_time++;
            break;

        case F_NIL:
            nodetype = MDF_TYPE_NULL;
            step = 1;
            valued_time++;
            break;
        case F_FALSE:
            nodetype = MDF_TYPE_BOOL;
            step = 1;
            valuen = 0;
            valued_time++;
            break;
        case F_TRUE:
            nodetype = MDF_TYPE_BOOL;
            step = 1;
            valuen = 1;
            valued_time++;
            break;

        case F_FLOAT32:
        case F_FLOAT64:
            nodetype = MDF_TYPE_FLOAT;
            valuef = _mpack_get_float(pos, &step);
            valued_time++;
            break;

        case F_FIX_STR:
        case F_STR_8:
        case F_STR_16:
        case F_STR_32:
            nodetype = MDF_TYPE_STRING;
            if (whoami == MDF_TYPE_OBJECT) {
                if (valued_time == 0) name = (char*)_mpack_get_str(pos, &step, &namelen);
                else values = (char*)_mpack_get_str(pos, &step, &valuelen);
            } else values = (char*)_mpack_get_str(pos, &step, &valuelen);
            valued_time++;
            break;

        case F_BIN_8:
        case F_BIN_16:
        case F_BIN_32:
            nodetype = MDF_TYPE_BINARY;
            valueb = (char*)_mpack_get_binary(pos, &step, &valuelen);
            valued_time++;
            break;

        case F_FIX_ARRAY:
        case F_ARRAY_16:
        case F_ARRAY_32:
            if (whoami == MDF_TYPE_UNKNOWN) {
                whoami = MDF_TYPE_ARRAY;
                node->type = MDF_TYPE_ARRAY;
                nodenum_need = _mpack_get_array_count(pos, &step);
                valued_time = 0;
            } else if (whoami == MDF_TYPE_ARRAY) {
                mdf_init(&xnode);
                memset(arrayindex, 0x0, sizeof(arrayindex));
                snprintf(arrayindex, sizeof(arrayindex), "%d", nodenum_got);
                xnode->name = strdup(arrayindex);
                xnode->namelen = strlen(arrayindex);

                step = mdf_mpack_deserialize(xnode, pos, len - mylen);

                _mdf_append_child_node(node, xnode, nodenum_got);

                AFTER_NODE_APPENDED();
            } else if (whoami == MDF_TYPE_OBJECT) {
                mdf_init(&xnode);
                xnode->name = mstr_ndup(name, namelen);
                xnode->namelen = namelen;

                step = mdf_mpack_deserialize(xnode, pos, len - mylen);

                _mdf_append_child_node(node, xnode, nodenum_got);

                AFTER_NODE_APPENDED();
            }
            break;

        case F_FIX_MAP:
        case F_MAP_16:
        case F_MAP_32:
            if (whoami == MDF_TYPE_UNKNOWN) {
                whoami = MDF_TYPE_OBJECT;
                node->type = MDF_TYPE_OBJECT;
                nodenum_need = _mpack_get_map_count(pos, &step);
                valued_time = 0;
            } else if (whoami == MDF_TYPE_ARRAY) {
                mdf_init(&xnode);
                memset(arrayindex, 0x0, sizeof(arrayindex));
                snprintf(arrayindex, sizeof(arrayindex), "%d", nodenum_got);
                xnode->name = strdup(arrayindex);
                xnode->namelen = strlen(arrayindex);

                step = mdf_mpack_deserialize(xnode, pos, len - mylen);

                _mdf_append_child_node(node, xnode, nodenum_got);

                AFTER_NODE_APPENDED();
            } else if (whoami == MDF_TYPE_OBJECT) {
                mdf_init(&xnode);
                xnode->name = mstr_ndup(name, namelen);
                xnode->namelen = namelen;

                step = mdf_mpack_deserialize(xnode, pos, len - mylen);

                _mdf_append_child_node(node, xnode, nodenum_got);

                AFTER_NODE_APPENDED();
            }
            break;

        case F_FIX_EXT_1:
            nodetype = MDF_TYPE_UNKNOWN;
            valued_time = 2;
            step = 3;
            break;

        bad_format:
        case F_FIX_EXT_2:
        case F_FIX_EXT_4:
        case F_FIX_EXT_8:
        case F_FIX_EXT_16:
        case F_EXT_8:
        case F_EXT_16:
        case F_EXT_32:
        default:
            /* TODO error message? */
            printf("fuck error %d\n", *(uint8_t*)pos);
            return mylen;
        }

        if (whoami == MDF_TYPE_OBJECT) {
            if (valued_time == 1) {
                if (nodetype != MDF_TYPE_STRING) goto bad_format;
            } else if (valued_time == 2) {
                mdf_init(&xnode);
                xnode->name = mstr_ndup(name, namelen);
                xnode->namelen = namelen;
                xnode->type = nodetype;

                if (nodetype == MDF_TYPE_STRING) {
                    xnode->val.s = mstr_ndup(values, valuelen);
                    xnode->valuelen = valuelen;
                } else if (nodetype == MDF_TYPE_INT) {
                    xnode->val.n = valuen;
                } else if (nodetype == MDF_TYPE_FLOAT) {
                    xnode->val.f = valuef;
                } else if (nodetype == MDF_TYPE_BOOL) {
                    xnode->val.n = valuen;
                } else if (nodetype == MDF_TYPE_BINARY) {
                    xnode->val.s = mos_calloc(1, valuelen);
                    memcpy(xnode->val.s, valueb, valuelen);
                    xnode->valuelen = valuelen;
                } else if (nodetype == MDF_TYPE_NULL) {
                    xnode->val.n = 0;
                }

                _mdf_append_child_node(node, xnode, nodenum_got);

                AFTER_NODE_APPENDED();
            }
        } else if (whoami == MDF_TYPE_ARRAY) {
            if (valued_time > 0) {
                mdf_init(&xnode);
                memset(arrayindex, 0x0, sizeof(arrayindex));
                snprintf(arrayindex, sizeof(arrayindex), "%d", nodenum_got);
                xnode->name = strdup(arrayindex);
                xnode->namelen = strlen(arrayindex);
                xnode->type = nodetype;

                if (nodetype == MDF_TYPE_STRING) {
                    xnode->val.s = mstr_ndup(values, valuelen);
                    xnode->valuelen = valuelen;
                } else if (nodetype == MDF_TYPE_INT) {
                    xnode->val.n = valuen;
                } else if (nodetype == MDF_TYPE_FLOAT) {
                    xnode->val.f = valuef;
                } else if (nodetype == MDF_TYPE_BOOL) {
                    xnode->val.n = valuen;
                } else if (nodetype == MDF_TYPE_BINARY) {
                    xnode->val.s = mos_calloc(1, valuelen);
                    memcpy(xnode->val.s, valueb, valuelen);
                    xnode->valuelen = valuelen;
                } else if (nodetype == MDF_TYPE_NULL) {
                    xnode->val.n = 0;
                }

                _mdf_append_child_node(node, xnode, nodenum_got);

                AFTER_NODE_APPENDED();
            }
        }

        if (valued_time > 2) goto bad_format;

        pos += step;
        mylen += step;
        step = 0;
    }

    return mylen;
}
