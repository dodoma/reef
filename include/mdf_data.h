#ifndef __MDF_DATA_H__
#define __MDF_DATA_H__

/*
 * merge two node(confignode, datanode), produce a new node(outnode)
 * we call it data render
 * e.g.
 * confignode:
 *    boardid [type=102, require=true] = bid
 *    cardid [type=102, valuenode=true] = 12
 *    video [type=107, value=v] {
 *        restid [type=102]  = rid
 *        destids [type=108, value=dids] {
 *            __arraynode__ [type=102] = __value__
 *        }
 *    }
 *
 * datanode:
 *     bid = 110
 *     v {
 *         rid = 2
 *         title = 闺密争抢土豪现场痛苦
 *         dids {
 *             0 = 21
 *             1 = 22
 *         }
 *     }
 *
 * outnode
 *    boardid [type="102"]  = 110
 *    cardids [type="102"] = 12
 *    video {
 *      restid [type="102"]  = 2
 *      destids {
 *        0 [type="102"]  = 21
 *        1 [type="102"]  = 22
 *      }
 *    }
 *
 * directions in confignode:
 * A: attribute
 *    type
 *      data type(int, string, object, array, etc) of this outnode
 *      refer CnodeType of mcs.h for detail value
 *
 *    value(equal to confignode's value)
 *      outnode's value = hdf_get_value(datanode, value)
 *      value=__value__ for the whole datanode
 *      value=__1stc__.xxx to get value from datanode's first child's xxx
 *          xxx can be NULL, and each value have only one of it at most
 *
 *    require=true
 *      return error if hdf_get_value(datanode, value) == NULL
 *
 *    default
 *      default value if hdf_get_value(datanode, value) == NULL
 *
 *    valuenode=true
 *      outnode's value = hdf_obj_value(confignode)
 *
 *    childtype=__single__
 *      don't iteral datanode, appeared in array node only
 *
 * B: name
 *    __arraynode__
 *      array node's child name MUST be __arraynode__,
 *      and, __arrynode__ just can appeare as arraynode's child name
 *
 *    __datanode__
 *      outnode's name = hdf_obj_name(datanode)
 *
 * C: value
 *    .$.
 *      if ".$." exist in confignode's value, and this node type is array
 *      we will iteral two datanodes before and after ".$."
 * please refer doc/mnl/mcs_data_rend.md for more detail
 */

/*
 * mdf data rend - mdf 数据渲染
 * 多人协作时，我们需要将数据，从一种格式，转换成另外一种格式，以满足对方对数据的组织格式和命名方式的奇葩要求。
 * 例如:
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
 *     boardid: {
 *         __value__: "bid"
 *     },
 *     description: "new board",
 *     cardid: 12,
 *     video: {
 *         __value__: "v",
 *         __require__: true,
 *         restid: {
 *             __value__: "rid"
 *             __type__: "string",
 *         },
 *         destids: {
 *             __type__: "array",
 *             __value__: "dids"
 *             __arraynode__: {
 *                 id: "__nodevalue__"
 *             }
 *         }
 *     }
 * }
 *
 * {
 *  slitid : {
 *      __type__: "array",
 *      __value__: "adgroups.$.spots",
 *      __arraynode__: "spot_id"
 *  }
 * }
 */
void mdf_data_rend(MDF *confignode, MDF *datanode, MDF *outnode);
__END_DECLS
#endif
