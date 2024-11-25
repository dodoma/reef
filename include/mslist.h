#ifndef __MSLIST_H__
#define __MSLIST_H__

/*
 * mslist, single linked list
 * 不实现 mslist ，难保正常发挥能写好一个单向链表（水平也就只有这样了）
 * 除了返回单个节点（如搜索），其他所有函数参数和返回结果都是链表头节点
 */
__BEGIN_DECLS

/* when alist is NULL, just same as mslist_new() */
MSLIST* mslist_add(MSLIST *alist, void *data);
MSLIST* mslist_join(MSLIST *alist, MSLIST *blist);
uint32_t mslist_length(MSLIST *alist);
MSLIST* mslist_search(MSLIST *alist, void *key, int (*compare)(void *data, void *key));
MSLIST* mslist_remove(MSLIST *alist, void *key,
                      int (*compare)(void *data, void *key),
                      void (*freeitem)(void *data));
void mslist_iterate(MSLIST *alist, bool (*action)(void *data));
void mslist_free(MSLIST *alist, void (*freeitem)(void *data));

__END_DECLS
#endif  /* __MSLIST_H__ */
