#ifndef __MDF_AUX_H__
#define __MDF_AUX_H__

/*
 * mdf auxiliary functions - mdf 辅助工具
 */
__BEGIN_DECLS

/*
 * {
 *   "0": "returnback",
 *   "1": "1",
 *   "2": "last_reader",
 *   "3": "oDn-E0lIXPQVuR3BmUtTnWYZ9t1g",
 *   "4": "last_date",
 *   "5": "1552366918",
 *   "6": "read_num",
 *   "7": "5",
 *   "8": "cover",
 *   "9": "https://mbox.net.cn/s/img/book2.jpg",
 *   "10": "author",
 *   "11": "zhangsan",
 *   "12": "title",
 *   "13": "鱼翅与花椒",
 *   "14": "desc",
 *   "15": "这是一本描写xxx的书"
 * }
 * ===>
 * {
 *   "returnback": "1",
 *   "last_reader": "oDn-E0lIXPQVuR3BmUtTnWYZ9t1g",
 *   "last_date": "1552366918",
 *   "read_num": "5",
 *   "cover": "https://mbox.net.cn/s/img/book2.jpg",
 *   "author": "zhangsan",
 *   "title": "鱼翅与花椒",
 *   "desc": "这是一本描写xxx的书",
 *  }
 */

void mdf_aux_list2map(MDF *node, const char *path);

/*
 * {"path": "/home/ml/tmp"}
 * ===>
 * {"path": "/home/ml/tmp/"}
 */
void mdf_makesure_endwithc(MDF *node, const char *key, char c);


/*
 * 数组操作：
 *
 * 本欲提供 mdf_create_array_node(xxx)
 * 但为降低复杂度，建议使用以下方案
 * ["sda1", "sda2"]
 * ================
 *   MDF *snode = mdf_get_or_create_node(qe->nodeout, "usbstick");
 *   while (tnode) {
 *       MDF *cnode = mdf_insert_node(snode, NULL, -1);
 *       mdf_set_value(cnode, NULL, tnode->name);
 *
 *       tnode = tnode->next;
 *   }
 *   mdf_object_2_array(snode, NULL);
 *
 *
 * [{"name": "sda1"}, {"name": "sda2"}]
 * ====================================
 *   MDF *snode = mdf_get_or_create_node(qe->nodeout, "usbstick");
 *   while (tnode) {
 *       MDF *cnode = mdf_insert_node(snode, NULL, -1);
 *       mdf_set_value(cnode, "name", tnode->name);
 *
 *       tnode = tnode->next;
 *   }
 *   mdf_object_2_array(snode, NULL);
 *
 */

__END_DECLS
#endif
