#ifndef __MLIST_H__
#define __MLIST_H__

/*
 * mlist, list data type
 * 列表实现起来大体有: 链表、线性表。
 * 我们选用了动态线性表(dynamic array)，作为moon的列表存储结构。
 * 动态线性表的 append, pop, insert, search等操作的时间复杂度都较好（insert 稍弱）。
 * 在大内存服务器上（是的，web服务器普遍具有较大内存，嵌入式web服务另议），
 * 动态申请一片连续内存，保持上万个元素的动态线性表并不费力。
 * 如果元素过多（超百万个？）且非独立使用服务器，建议使用 mhash 数据结构。
 *
 * dynamic array 类似其他语言的以下数据结构：
 *   std::vector of C++, ArrayList class of Java, list datatype of Python
 *
 * 注意：
 *   1. 列表位置索引从0开始，不是1.
 *
 *   2. 为了支持非指针数组排序（诸如结构体数组）, glibc 的 qsort(), bsearch() 需要的参数为：
 *      连续内存块中，该分片内存的地址，返回的也是该分片内存的地址。
 *        a. The bsearch() function returns a pointer to a matching member of the array.
 *
 *        b. p = (void *) (((const char *) base) + (idx * size));
 *           comparison = (*compare)(key, p);
 *           ...
 *           return (void *) p;
 *
 *      比如，下面的情况下，compare() 的 p 参数为 0x6029e0, 0x6029f0, 0x602a00...
 *      而非 0x400eb8, 0x400ebc, 0x400ec0...
 *
 *        (gdb) p *obj
 *        $4 = {num = 6, max = 256, sorted = false, free = 0, items = 0x6029e0}
 *        (gdb) x/10ag 0x6029e0
 *        0x6029e0:	0x400eb8 <__dso_handle+8>	0x400ebc <__dso_handle+12>
 *        0x6029f0:	0x400ec0 <__dso_handle+16>	0x400ec4 <__dso_handle+20>
 *        0x602a00:	0x400ec8 <__dso_handle+24>	0x6031f0
 *        0x602a10:	0x0	0x0
 *        0x602a20:	0x0	0x0
 *        (gdb) p (char*) 0x6031f0
 *        $5 = 0x6031f0 "fff"
 *
 *      而我们的元素本身即为指针，
 *      所以，我们在使用 mlist 排序、查找(mlist_in(), index(), search(), compare())的参数和返回
 *      需要使用二级指针。
 *      ( mlist_in, mlist_index 其实可以不用二级指针，为了保持和 mlist_search 一致而为之。)
 *
 *    3. 插入和取回非指针（如数字和字符）比较特殊，请参考 test/list.c 用法
 */
__BEGIN_DECLS

MERR* mlist_init(MLIST **alist, void __F(freeitem)(void*));

MERR* mlist_append(MLIST *alist, void *data);
MERR* mlist_pop(MLIST *alist, void **data);
MERR* mlist_insert(MLIST *alist, int x, void *data);
MERR* mlist_delete(MLIST *alist, int x);
void  mlist_free(void *alist);
void  mlist_clear(MLIST *alist);

MERR* mlist_get(MLIST *alist, int x, void **data);
MERR* mlist_set(MLIST *alist, int x, void *data);
MERR* mlist_cut(MLIST *alist, int x, void **data);

MERR* mlist_reverse(MLIST *alist);
int   mlist_length(MLIST *alist);

/*
 * 返回列表中，key 元素的地址(&alist->items[i])，而非元素本身。
 * 参数 key 为待查找元素的地址，而非元素本身。
 * compare 中的两个参数为 item 的地址，请参考 test/list.c 之 miicompare
 */
void* mlist_in(MLIST *alist, const void *key,
               int __F(compare)(const void*, const void*));
int   mlist_index(MLIST *alist, const void *key,
                  int __F(compare)(const void*, const void*));

void  mlist_destroy(MLIST **alist);

void  mlist_sort(MLIST *alist, int __F(compare)(const void *, const void*));

/*
 * quick search, this will sort the list(change item's position).
 * 返回列表中内存元素的地址，而非内存元素
 */
void* mlist_search(MLIST *alist, const void *key,
                   int __F(compare)(const void*, const void*));


__END_DECLS
#endif
