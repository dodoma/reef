#ifndef __MDLIST_H__
#define __MDLIST_H__

/*
 * mdlist, double linked list data type
 */
__BEGIN_DECLS

MDLIST* mdlist_new(void *data);
void*   mdlist_data(MDLIST *alist);

MDLIST* mdlist_head(MDLIST *alist);
MDLIST* mdlist_tail(MDLIST *alist);
MDLIST* mdlist_prev(MDLIST *alist);
MDLIST* mdlist_next(MDLIST *alist);

bool mdlist_alone(MDLIST *alist);
size_t  mdlist_length(MDLIST *alist);
MDLIST* mdlist_find(MDLIST *alist, void *key, int __F(compareitem)(MDLIST *blist, void *key));

/*
 * 将 blist 插入到 alist 后面
 */
MDLIST* mdlist_insert(MDLIST *alist, MDLIST *blist);

/*
 * 将 blist 添加到 alist 末尾
 */
MDLIST* mdlist_concat(MDLIST *alist, MDLIST *blist);

/*
 * 将 alist 从链表中删除
 */
MDLIST* mdlist_eject(MDLIST *alist, void __F(freeitem)(void*));

void mdlist_free(MDLIST *alist, void __F(freeitem)(void*));

__END_DECLS
#endif
