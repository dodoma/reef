#include "reef.h"
#include "_mdf.h"

#if 0
static inline int _node_output_count(MDF *node)
{
    if (!node || !node->child) return 0;

    int count = 0;

    MDF *cnode = mdf_node_child(node);
    while (cnode) {
        char *name = cnode->name;
        int len = cnode->namelen;

        if (name[0] != '_' || name[1] != '-' || name[len-1] != '_' || name[len-2] != '_') count++;
        else if (!strcmp(name, "__arraynode__")) count++;

        cnode = mdf_node_next(cnode);
    }

    return count;
}

static void (*m_cube)(MDF *anode, MDF *bnode, MDF *outnode)[2][5][6];

static void _cube_init()
{
    static bool inited = false;

    if (inited) return;
    inited = true;

    memset(m_cube, 0x0, sizeof(m_cube));

    m_cube[0][0][0] = _callback_empty_string();
    m_cube[0][0][1] = _callback_empty_string();
    m_cube[0][0][2] = _callback_empty_string();
    m_cube[0][0][3] = _callback_empty_string();
    m_cube[0][0][4] = _callback_empty_string();
    m_cube[0][0][5] = _callback_empty_string();

    m_cube[0][1][0] = _callback_();

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
 *    x1y1: 拷贝value，转换成type mdf_set_type()
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
 *    x3y4 ~ x5y4: ---不处理---
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

    MDF *anode, *bnode;

    anode = config_node;
    bnode = data_node;

    _cube_init();

    MDF *cnode = mdf_node_child(anode);
    while (cnode) {
        char *name = mdf_get_name(cnode, NULL);
        MDF_TYPE type = mdf_get_type(cnode, NULL);

        char *key_value = mdf_get_value(cnode, "__value__", NULL);
        char *key_type = mdf_get_value(cnode, "__type__", NULL);
        int x = 0, y = 0;

        if (!key_value || !mdf_path_exist(bnode, name)) y = 0;
        else if (mdf_get_type(bnode, name) == MDF_TYPE_OBJECT) y = 1;
        else if (mdf_get_type(bnode, name) == MDF_TYPE_ARRAY) y = 2;
        else if (mdf_get_type(bnode, name) == MDF_TYPE_STRING) y = 3;
        else y = 4;

        if (!key_type) {
            x = 0;
        } else {
            switch (key_type[0]) {
            case 'a': /* array */
            case 'A':
                x = 1;
                break;
            case 's': /* string */
            case 'S':
                x = 2;
                break;
            case 'i': /* int */
            case 'I':
                x = 3;
                break;
            case 'f': /* float */
            case 'F':
                x = 4;
                break;
            case 'b': /* bool */
            case 'B':
                x = 5;
                break;
            default:
                x = 0;
            }
        }

        if (type != MDF_TYPE_OBJECT) {
            /*
             * A. cnode 为值节点，直接赋值
             */
            mdf_copy(outnode, name, cnode);
        } else {
            /*
             * B. cnode 为对象节点，使用条件魔方
             */
            int z = 0;
            if (_node_output_count(cnode) > 0) z = 1;

            void (*cube_callback)(MDF *anode, MDF *bnode, MDF *outnode);

            cube_callback = m_cube[z][y][x];

            if (cube_callback) cube_callback(anode, bnode, outnode);
        }

    nextnode:
        cnode = mdf_node_next(cnode);
    }
}

#endif
