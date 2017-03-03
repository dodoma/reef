#ifndef __MDF_DATA_H__
#define __MDF_DATA_H__

/*
 * mdf data rend - mdf 数据渲染
 *
 * 多人协作时，我们需要将数据，从一种格式，转换成另外一种格式，以满足对方对数据的组织格式和命名方式的奇葩要求。例如:
 * 现有一内部数据(datanode)：
 * {
 *     bid: 110,
 *     v: {
 *         rid = 2,
 *         title = "闺密争抢土豪现场痛苦",
 *         dids: [21, 22]
 *     }
 * }
 * 而，他人需要如下数据(outnode):
 * {
 *     boardid: 110,
 *     description: "new board",
 *     cardid: 12,
 *     video: {
 *         restid: "2"
 *         destids: [{id: 21}, {id: 22}]
 *     }
 * }
 * 此时，我们只需一个配置数据(confignode):
 * {
 *     boardid: "__mr__bid",
 *     description: "new board",
 *     cardid: 12,
 *     video: {
 *         __value__: "v",
 *         restid: {
 *             __value__: "rid"
 *             __type__: "string",
 *         },
 *         destids: {
 *             __value__: "dids"
 *             __arraynode__: {
 *                 id: "__nodevalue__"
 *             }
 *         }
 *     }
 * }
 *
 * 具体用法请参考 test/mdf_data.c
 */
void mdf_data_rend(MDF *confignode, MDF *datanode, MDF *outnode);

__END_DECLS
#endif
