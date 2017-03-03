#include "reef.h"
#include "_mdf.h"

typedef void (*CALLBACK_FUNC)(MDF *anode, MDF *bnode, MDF *outnode, MDF *vnode, char *name, int x, int y, int z);

CALLBACK_FUNC m_cube[2][5][6];

enum {
    AXIS_X_NULL = 0,
    AXIS_X_ARRAY,
    AXIS_X_STRING,
    AXIS_X_INT,
    AXIS_X_FLOAT,
    AXIS_X_BOOL
};

enum {
    AXIS_Y_NULL = 0,
    AXIS_Y_OBJECT,
    AXIS_Y_ARRAY,
    AXIS_Y_STRING,
    AXIS_Y_IFB
};

enum {
    AXIS_Z_CLEAN = 0,
    AXIS_Z_OUTPUT
};

static void _c_empty_string(MDF *anode, MDF *bnode, MDF *outnode, MDF *vnode, char *name, int x, int y, int z)
{
    MDF *cnode = mdf_get_or_create_node(outnode, name);

    switch (x) {
    case AXIS_X_NULL:
        break;
    case AXIS_X_ARRAY:
        mdf_set_type(cnode, NULL, MDF_TYPE_ARRAY);
        break;
    case AXIS_X_STRING:
        break;
    case AXIS_X_INT:
        mdf_set_type(cnode, NULL, MDF_TYPE_INT);
        break;
    case AXIS_X_FLOAT:
        mdf_set_type(cnode, NULL, MDF_TYPE_FLOAT);
        break;
    case AXIS_X_BOOL:
        mdf_set_type(cnode, NULL, MDF_TYPE_BOOL);
        break;
    default:
        break;
    }
}

static void _c_1_0(MDF *anode, MDF *bnode, MDF *outnode, MDF *vnode, char *name, int x, int y, int z)
{
    switch (x) {
    case AXIS_X_NULL:
        mdf_copy(outnode, name, vnode, true);
        break;
    case AXIS_X_ARRAY:
        mdf_copy(outnode, name, vnode, true);
        mdf_object_2_array(outnode, name);
        break;
    case AXIS_X_BOOL:
        mdf_set_bool_value(outnode, name, true);
        break;
    default:
        break;
    }
}

static void _c_2_0(MDF *anode, MDF *bnode, MDF *outnode, MDF *vnode, char *name, int x, int y, int z)
{
   switch (x) {
    case AXIS_X_NULL:
    case AXIS_X_ARRAY:
        mdf_copy(outnode, name, vnode, true);
        break;
    case AXIS_X_BOOL:
        mdf_set_bool_value(outnode, name, true);
        break;
    default:
        break;
    }
}

static void _c_3_0(MDF *anode, MDF *bnode, MDF *outnode, MDF *vnode, char *name, int x, int y, int z)
{
    switch (x) {
    case AXIS_X_NULL:
    case AXIS_X_STRING:
        mdf_copy(outnode, name, vnode, true);
        break;
    case AXIS_X_ARRAY:
        mdf_copy(outnode, name, vnode, true);
        mdf_set_type(outnode, name, MDF_TYPE_ARRAY);
        break;
    case AXIS_X_INT:
        mdf_copy(outnode, name, vnode, true);
        mdf_set_type(outnode, name, MDF_TYPE_INT);
        break;
    case AXIS_X_FLOAT:
        mdf_copy(outnode, name, vnode, true);
        mdf_set_type(outnode, name, MDF_TYPE_FLOAT);
        break;
    case AXIS_X_BOOL:
        mdf_copy(outnode, name, vnode, true);
        mdf_set_type(outnode, name, MDF_TYPE_BOOL);
        break;
    default:
        break;
    }
}

static void _c_4_0(MDF *anode, MDF *bnode, MDF *outnode, MDF *vnode, char *name, int x, int y, int z)
{
    switch (x) {
    case AXIS_X_NULL:
        mdf_copy(outnode, name, vnode, true);
        break;
    case AXIS_X_STRING:
        mdf_copy(outnode, name, vnode, true);
        mdf_set_type_revert(outnode, name);
        break;
    case AXIS_X_INT:
        mdf_copy(outnode, name, vnode, true);
        mdf_set_digit_type(outnode, name, MDF_TYPE_INT);
        break;
    case AXIS_X_FLOAT:
        mdf_copy(outnode, name, vnode, true);
        mdf_set_digit_type(outnode, name, MDF_TYPE_FLOAT);
        break;
    case AXIS_X_BOOL:
        mdf_copy(outnode, name, vnode, true);
        mdf_set_digit_type(outnode, name, MDF_TYPE_BOOL);
        break;
    default:
        break;
    }
}

static void _c_0_1(MDF *anode, MDF *bnode, MDF *outnode, MDF *vnode, char *name, int x, int y, int z)
{
    MDF *cnode  = mdf_get_or_create_node(outnode, name);

    return mdf_data_rend(anode, bnode, cnode);
}

static void _c_1_1(MDF *anode, MDF *bnode, MDF *outnode, MDF *vnode, char *name, int x, int y, int z)
{
    switch (x) {
    case AXIS_X_NULL:
        mdf_data_rend(anode, vnode, mdf_get_or_create_node(outnode, name));
        break;
    case AXIS_X_ARRAY:
        mdf_data_rend(anode, vnode, mdf_get_or_create_node(outnode, name));
        mdf_object_2_array(outnode, name);
        break;
    case AXIS_X_BOOL:
        mdf_set_bool_value(outnode, name, true);
    default:
        break;
    }
}

static void _c_2_1(MDF *anode, MDF *bnode, MDF *outnode, MDF *vnode, char *name, int x, int y, int z)
{
    int count;
    MDF *cnode;

    MDF *confignode = mdf_get_node(anode, "__arraynode__");
    if (!confignode) return;

    switch (x) {
    case AXIS_X_NULL:
    case AXIS_X_ARRAY:
        count = 0;
        cnode = mdf_node_child(vnode);
        while (cnode) {
            MDF *xnode = mdf_get_or_create_nodef(outnode, "%s[%d]", name, count++);
            mdf_data_rend(confignode, cnode, xnode);

            cnode = mdf_node_next(cnode);
        }
        break;
    case AXIS_X_BOOL:
        mdf_set_bool_value(outnode, name, true);
    default:
        break;
    }
}

static void _c_output_r(MDF *anode, MDF *bnode, MDF *outnode, MDF *vnode, char *name, int x, int y, int z)
{
    MDF *cnode  = mdf_get_or_create_node(outnode, name);

    mdf_data_rend(anode, vnode, cnode);

    switch (x) {
    case AXIS_X_ARRAY:
        mdf_object_2_array(cnode, NULL);
        break;
    default:
        break;
    }
}

static inline bool _is_arraynode(const char *name, uint32_t len)
{
    if (!name || len != 13) return false;

    if (strcmp(name, "__arraynode__")) return false;

    return true;
}

static inline bool _is_config_key(const char *name, uint32_t len)
{
    if (!name || len < 5) return false;

    if (name[0] != '_' || name[1] != '_' || name[len-1] != '_' || name[len-2] != '_') return false;

    if (!strcmp(name, "__arraynode__")) return false;

    return true;
}

static bool _rend_nodevalue(MDF *anode, MDF *bnode, MDF *outnode)
{
    if (!anode || !bnode || !outnode) return false;

    if (anode->type == MDF_TYPE_STRING && anode->valuelen == 13 && !strcmp(anode->val.s, "__nodevalue__")) {
        if (_is_arraynode(anode->name, anode->namelen)) mdf_copy(outnode, NULL, bnode, true);
        else mdf_copy(outnode, anode->name, bnode, true);

        return true;
    }

    return false;
}

static inline char* _key_from_value(MDF *cnode)
{
    if (!cnode || cnode->type != MDF_TYPE_STRING) return NULL;

    if (cnode->valuelen < 7) return NULL;

    char *value = cnode->val.s;

    if (value[0] == '_' && value[1] == '_' && value[2] == 'm' &&
        value[3] == 'r' && value[4] == '_' && value[5] == '_') return value + 6;

    return NULL;
}

static inline int _node_output_count(MDF *node)
{
    if (!node || !node->child) return 0;

    int count = 0;

    MDF *cnode = mdf_node_child(node);
    while (cnode) {
        char *name = cnode->name;
        int len = cnode->namelen;

        if (name[0] != '_' || name[1] != '_' || name[len-1] != '_' || name[len-2] != '_') count++;
        else if (!strcmp(name, "__arraynode__")) count++;

        cnode = mdf_node_next(cnode);
    }

    return count;
}

static void _cube_init()
{
    static bool inited = false;

    if (inited) return;
    inited = true;

    memset(m_cube, 0x0, sizeof(m_cube));

    m_cube[0][0][0] = m_cube[0][0][1] = m_cube[0][0][2] = _c_empty_string;
    m_cube[0][0][3] = m_cube[0][0][4] = m_cube[0][0][5] = _c_empty_string;

    m_cube[0][1][0] = m_cube[0][1][1] = m_cube[0][1][2] = _c_1_0;
    m_cube[0][1][3] = m_cube[0][1][4] = m_cube[0][1][5] = _c_1_0;

    m_cube[0][2][0] = m_cube[0][2][1] = m_cube[0][2][2] = _c_2_0;
    m_cube[0][2][3] = m_cube[0][2][4] = m_cube[0][2][5] = _c_2_0;

    m_cube[0][3][0] = m_cube[0][3][1] = m_cube[0][3][2] = _c_3_0;
    m_cube[0][3][3] = m_cube[0][3][4] = m_cube[0][3][5] = _c_3_0;

    m_cube[0][4][0] = m_cube[0][4][1] = m_cube[0][4][2] = _c_4_0;
    m_cube[0][4][3] = m_cube[0][4][4] = m_cube[0][4][5] = _c_4_0;


    m_cube[1][0][0] = m_cube[1][0][1] = m_cube[1][0][2] = _c_0_1;
    m_cube[1][0][3] = m_cube[1][0][4] = m_cube[1][0][5] = _c_0_1;

    m_cube[1][1][0] = m_cube[1][1][1] = m_cube[1][1][2] = _c_1_1;
    m_cube[1][1][3] = m_cube[1][1][4] = m_cube[1][1][5] = _c_1_1;

    m_cube[1][2][0] = m_cube[1][2][1] = m_cube[1][2][2] = _c_2_1;
    m_cube[1][2][3] = m_cube[1][2][4] = m_cube[1][2][5] = _c_2_1;

    m_cube[1][3][0] = m_cube[1][3][1] = m_cube[1][3][2] = _c_output_r;
    m_cube[1][3][3] = m_cube[1][3][4] = m_cube[1][3][5] = _c_output_r;

    m_cube[1][4][0] = m_cube[1][4][1] = m_cube[1][4][2] = _c_output_r;
    m_cube[1][4][3] = m_cube[1][4][4] = m_cube[1][4][5] = _c_output_r;
}

/*
 * A. config node 为值节点，直接赋值
 *
 * B. config node 为对象节点，使用以下条件魔方
 *   page 1. 无输出子节点
 *     /-------------------------------------------------------------------\
 *     |value\type       | NULL,dft | array | string | int  | float | bool |
 *     |-------------------------------------------------------------------|
 *     |NULL(str,node)   |   x0y0   |  x1y0 | x2y0   | x3y0 | x4y0  | x5y0 |
 *     |-------------------------------------------------------------------|
 *     |object           |   x0y1   |  x1y1 | x2y1   | x3y1 | x4y1  | x5y1 |
 *     |-------------------------------------------------------------------|
 *     |array(raw,.$.)   |   x0y2   |  x1y2 | x2y2   | x3y2 | x4y2  | x5y2 |
 *     |-------------------------------------------------------------------|
 *     |string           |   x0y3   |  x1y3 | x2y3   | x3y3 | x4y3  | x5y3 |
 *     |-------------------------------------------------------------------|
 *     |i,f,b            |   x0y4   |  x1y4 | x2y4   | x3y4 | x4y4  | x5y4 |
 *     |-------------------------------------------------------------------|
 *     |                                                                   |
 *     \-------------------------------------------------------------------/
 *
 *    x0y0 ~ x5y0: 创建空的字符节点，转换成 type mdf_set_type()
 *
 *    x0y1: 拷贝value
 *    x1y1: 拷贝value，转换成type mdf_object_2_array()
 *    x2y1 ~ x4y1: ---不处理---
 *    x5y1: 创建true的输出布尔
 *
 *    x0y2 ~ x1y2: 拷贝value
 *    x2y2 ~ x2y4: ---不处理---
 *    x5y2: 创建true的输出布尔
 *
 *    x0y3: 拷贝value
 *    x1y3 ~ x5y3: 拷贝value对象，转换成type mdf_set_type()
 *
 *    x0y4: 拷贝value
 *    x1y4: ---不处理---
 *    x2y4: 转换成type mdf_set_type_revert()
 *    x3y4 ~ x5y4: 转换成type mdf_set_digit_type()
 *
 *
 *
 *   page 2. 有输出子节点
 *     /-------------------------------------------------------------------\
 *     |value\type       | NULL,dft | array | string | int  | float | bool |
 *     |-------------------------------------------------------------------|
 *     |NULL(str,node)   |   x0y0   |  x1y0 | x2y0   | x3y0 | x4y0  | x5y0 |
 *     |-------------------------------------------------------------------|
 *     |object           |   x0y1   |  x1y1 | x2y1   | x3y1 | x4y1  | x5y1 |
 *     |-------------------------------------------------------------------|
 *     |array(raw,.$.)   |   x0y2   |  x1y2 | x2y2   | x3y2 | x4y2  | x5y2 |
 *     |-------------------------------------------------------------------|
 *     |string           |   x0y3   |  x1y3 | x2y3   | x3y3 | x4y3  | x5y3 |
 *     |-------------------------------------------------------------------|
 *     |i,f,b            |   x0y4   |  x1y4 | x2y4   | x3y4 | x4y4  | x5y4 |
 *     |-------------------------------------------------------------------|
 *     |                                                                   |
 *     \-------------------------------------------------------------------/
 *
 *    x0y0 ~ x5y0: 递归构造输出子节点
 *
 *    x0y1: 更新 data node, 递归构造输出子节点
 *    x1y1: 更新 data node, 递归构造输出子节点, 转换成type mdf_set_type()
 *    x2y1 ~ x4y1: ---不处理---
 *    x5y1: 创建true的输出布尔
 *
 *    x0y2 ~ x1y2: 遍历更新data node, 递归构造输出子节点(__arraynode__, __nodevalue__)
 *    x2y2 ~ x4y2: ---不处理---
 *    x5y2: 创建true的输出布尔
 *
 *    x0y3 ~ x5y3: 更新data node, 递归构造输出节点
 *
 *    x0y4 ~ x5y4: 更新data node, 递归构造输出节点
 */
void mdf_data_rend(MDF *config_node, MDF *data_node, MDF *outnode)
{
    if (!config_node || !outnode) return;

    MDF *anode, *bnode, *vnode;

    anode = config_node;
    bnode = data_node;

    _cube_init();

    MDF *cnode = mdf_node_child(anode);
    while (cnode) {
        char *name = mdf_get_name(cnode, NULL);
        MDF_TYPE type = mdf_get_type(cnode, NULL);

        char *key_value = mdf_get_value(cnode, "__value__", NULL);
        char *key_type = mdf_get_value(cnode, "__type__", NULL);

        if (_rend_nodevalue(cnode, bnode, outnode)) goto nextnode;

        if (!key_value) key_value = _key_from_value(cnode);

        if (key_value) vnode = mdf_get_node(bnode, key_value);
        else vnode = NULL;
        int x = 0, y = 0;

        if (_is_config_key(cnode->name, cnode->namelen)) goto nextnode;

        if (!key_value || !mdf_path_exist(bnode, key_value)) y = AXIS_Y_NULL;
        else if (mdf_get_type(vnode, NULL) == MDF_TYPE_OBJECT) y = AXIS_Y_OBJECT;
        else if (mdf_get_type(vnode, NULL) == MDF_TYPE_ARRAY) y = AXIS_Y_ARRAY;
        else if (mdf_get_type(vnode, NULL) == MDF_TYPE_STRING) y = AXIS_Y_STRING;
        else y = AXIS_Y_IFB;

        if (!key_type) {
            x = AXIS_X_NULL;
        } else {
            switch (key_type[0]) {
            case 'a': /* array */
            case 'A':
                x = AXIS_X_ARRAY;
                break;
            case 's': /* string */
            case 'S':
                x = AXIS_X_STRING;
                break;
            case 'i': /* int */
            case 'I':
                x = AXIS_X_INT;
                break;
            case 'f': /* float */
            case 'F':
                x = AXIS_X_FLOAT;
                break;
            case 'b': /* bool */
            case 'B':
                x = AXIS_X_BOOL;
                break;
            default:
                x = AXIS_X_NULL;
            }
        }

        if (type != MDF_TYPE_OBJECT) {
            /*
             * A. cnode 为值节点，直接赋值
             */
            mdf_copy(outnode, name, vnode ? vnode: cnode, true);
        } else {
            /*
             * B. cnode 为对象节点，使用条件魔方
             */
            int z = AXIS_Z_CLEAN;
            if (_node_output_count(cnode) > 0) z = AXIS_Z_OUTPUT;

            void (*cube_callback)(MDF *anode, MDF *bnode, MDF *outnode, MDF *vnode,
                                  char *name, int x, int y, int z);

            cube_callback = m_cube[z][y][x];

            if (cube_callback) cube_callback(cnode, bnode, outnode, vnode, name, x, y, z);
        }

    nextnode:
        cnode = mdf_node_next(cnode);
    }

    if (!mdf_node_child(anode)) {
        if (_is_config_key(anode->name, anode->namelen)) {
            /*
             * 配置信息节点忽略
             */
        } else if (_is_arraynode(anode->name, anode->namelen)) {
            /*
             * 非对象的数组元素
             */
            if (!_rend_nodevalue(anode, bnode, outnode)) {
                char *key_value = _key_from_value(anode);
                if (key_value) {
                    vnode = mdf_get_node(bnode, key_value);
                    if (vnode) mdf_copy(outnode, NULL, vnode, true);
                } else mdf_copy(outnode, NULL, anode, true);
            }
        } else mdf_copy(outnode, NULL, anode, true);
    }
}
