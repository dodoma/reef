#ifndef __MSTR_H__
#define __MSTR_H__

/*
 * mstring, string tools
 * 字符串操作追求方便，故，我们建议在栈空间直接使用字符串函数，
 * 在此，不提供堆空间的 MSTR* mstr_init() 方法。
 */
__BEGIN_DECLS

/*
 * ======================
 * basic string functions
 * ======================
 */
void mstr_init(MSTR *astr);
void mstr_append(MSTR *astr, const char *buf);
void mstr_appendc(MSTR *astr, char c);
void mstr_appendn(MSTR *astr, const char *buf, size_t len);
void mstr_appendf(MSTR *astr, const char *fmt, ...) ATTRIBUTE_PRINTF(2,3);
void mstr_set(MSTR *astr, const char *buf);
void mstr_clear(MSTR *astr);


/*
 * ========================
 * advance string functions
 * ========================
 */

/*
 * 返回最多 len 个随机字母(a ~ z)组成的字符串
 * 调用者需确保 s 的内存长度 >= maxlen + 1
 */
void mstr_rand_string(char *s, size_t maxlen);
/*
 * 返回由 len 个随机字母(a ~ z)组成的字符串
 * 调用者需确保 s 的内存长度 >= len + 1
 */
void mstr_rand_string_fixlen(char *s, size_t len);

/*
 * 以 sep 为分割符，将 str 分割为最多为 max 个元素的列表。
 * alist 为新申请的列表，调用者请自行释放。
 */
MERR* mstr_array_split(MLIST **alist, const char *sin, const char *sep, int max);

__END_DECLS
#endif
