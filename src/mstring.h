#ifndef __MSTR_H__
#define __MSTR_H__

/*
 * mstring, string tools
 * 字符串操作追求方便，故，我们建议在栈空间直接使用字符串函数，
 * 在此，不提供堆空间的 MSTR* mstr_init() 方法。
 */
__BEGIN_DECLS

typedef struct {
    char *buf;
    size_t len;
    size_t max;
} MSTR;

void mstr_init(MSTR *str);
void mstr_append(MSTR *str, const char *buf);
void mstr_appendc(MSTR *str, char c);
void mstr_appendn(MSTR *str, const char *buf, size_t len);
void mstr_appendf(MSTR *str, const char *fmt, ...) ATTRIBUTE_PRINTF(2,3);
void mstr_set(MSTR *str, const char *buf);
void mstr_clear(MSTR *str);

__END_DECLS
#endif
