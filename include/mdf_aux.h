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

__END_DECLS
#endif
