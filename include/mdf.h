#ifndef __MDF_H__
#define __MDF_H__

/*
 * mdf, moon data format
 *
 * 一种方便的数据格式，会让应用开发人员倍感轻松(例如 js 语法并没有十分特别的地方，但js对象的使用实在方便)
 * 为此，moon 设计了 mdf 作为主要数据结构。
 *
 * mdf 可以方便的进行如下操作:
 *   基本操作：
 *     1. init, destroy
 *     2. set value (string, int, float, bool)
 *     3. get value, include key exist judement (support format string)
 *     4. remove, copy
 *     5. iterate child
 *     6. assemble new mdf according data, and config mdf (asmb)
 *   格式转换：
 *     1. 由 mdf 导出 json 字符串 (export string)
 *     2. 由 json 字符串生成 mdf  (import string)
 *     3. 由 mdf 导出 message pack 二进制内存 (serialize)
 *     4. 由 message pack 内存生成  mdf (deserialize)
 *   快速拷贝（TODO）：
 *     1. prepare mdf for copy input (spped serialize)
 *     2. quick mdf
 *
 * 注意：
 *   1. mdf 操作 path 参数的 BNF 定义
 *      PATH := (MDFNAME | NILL)
 *      MDFNAME := (MDF_SUBNAME | MDF_SUBNAME\.MDF_NAME)
 *      MDF_SUBNAME := (RAW_NAME | INDEX_NAME | RAW_NAMEINDEX_NAME)
 *      RAW_NAME := [0-9a-zA-Z_]+
 *      INDEX_NAME := \[[-1-9]+\]+
 *
 *      WS := [\t ]*
 *      NILL := (\"\" | NULL)
 *
 *      例如：
 *      "", NULL
 *      aa, aa.bb.cc
 *      [0], [0][3], [0].aa[3]
 *      aa[0], aa[0][2].bb
 *
 *   2. 除非必须，请不要使用 mdf_set_int/int64/float/bool_value()
 *      因为在我们操作数据时，大都不必关心数据类型，默认使用字符串类型即可。
 *      使用类型带来的坏处是 set 和 get 的操作方法必须一致。
 *      例如，mdf_set_int_value(node, "key", 100);
 *      则必须使用 mdf_get_int_value(node, "key", 0) 才能返回100,
 *      mdf_get_value(), mdf_get_float/bool等都不行，非常麻烦。
 *
 *      在与诸如database, web server等外部程序进行数据交互时，需要明确数据类型时使用上述函数
 *      另外，可以使用 mdf_set_type() 进行字符串类型节点的强制类型转换。
 */
__BEGIN_DECLS

MERR* mdf_init(MDF **node);
void  mdf_destroy(MDF **node);

MERR* mdf_set_value(MDF *node, const char *path, const char *value);
MERR* mdf_set_int_value(MDF *node, const char *path, int value);
MERR* mdf_set_int64_value(MDF *node, const char *path, int64_t value);
MERR* mdf_set_float_value(MDF *node, const char *path, float value);
MERR* mdf_set_bool_value(MDF *node, const char *path, bool value);
MERR* mdf_set_binary(MDF *node, const char *path, const unsigned char *buf, size_t len);
/* 将字符串类型节点 转换成 其他类型 */
MERR* mdf_set_type(MDF *node, const char *path, MDF_TYPE type);

char*   mdf_get_value(MDF *node, const char *path, char *dftvalue);
char*   mdf_get_value_copy(MDF *node, const char *path, char *dftvalue);
int     mdf_get_int_value(MDF *node, const char *path, int dftvalue);
int64_t mdf_get_int64_value(MDF *node, const char *path, int64_t dftvalue);
float   mdf_get_float_value(MDF *node, const char *path, float dftvalue);
bool    mdf_get_bool_value(MDF *node, const char *path, bool dftvalue);
unsigned char* mdf_get_binary(MDF *node, const char *path, size_t *len);
/* 返回节点的值（以字符串的方式），返回内容为新申请内存，使用后请自行释放。*/
char* mdf_get_value_stringfy(MDF *node, const char *path, char *dftvalue);

MERR* mdf_copy(MDF *dst, const char *path, MDF *src);
MERR* mdf_remove(MDF *node, const char *path);

MDF* mdf_get_node(MDF *node, const char *path);
MDF* mdf_get_or_create_node(MDF *node, const char *path);
MDF* mdf_node_next(MDF *node);
MDF* mdf_node_parent(MDF *node);
MDF* mdf_node_child(MDF *node);
MDF* mdf_sort_node(MDF *node, int __F(compare)(const void*, const void*));

bool  mdf_path_exist(MDF *node, const char *path);
int   mdf_node_child_count(MDF *node, const char *path);
char* mdf_node_name(MDF *node);
char* mdf_node_value(MDF *node);

/*
 * 解析 json 格式的字符串 str，存储在已准备好的 node 节点中。
 * 注意：
 *   1. 在解析失败时，node 中可能已经存了部分数据，故，为避免内存泄漏，失败时请自行释放 node 空间。
 */
MERR* mdf_json_import_string(MDF *node, const char *str);
MERR* mdf_json_import_file(MDF *node, const char *fname);

/* 返回字符串为新申请内存，使用后请自行释放 */
char* mdf_json_export_string(MDF *node);
MERR* mdf_json_export_file(MDF *node, const char *fname);

/*
 * 打包 mdf 节点（用 message pack 格式）， 放在已经准备好了的长度为 len 字节的 buf 内存中
 * 返回此次打包已使用的内存字节数
 */
size_t mdf_mpack_serialize(MDF *node, unsigned char *buf, size_t len);

/*
 * 解包 长度为 len 字节的 message pack 格式包，存储在已准备好的 node 节点中
 * 返回此次解包了的字节数
 */
size_t mdf_mpack_deserialize(MDF *node, const unsigned char *buf, size_t len);

__END_DECLS
#endif
