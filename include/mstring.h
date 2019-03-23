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
 * raw string functions
 * ======================
 */
/* strip will modify input string. make sure s is modifiable (not const) */
char* mstr_strip(char *s, char n);
char* mstr_strip_space(char *s);
char* mstr_repchr(char *s, char from, char to);
char* mstr_ndup(const char *s, size_t n);
/*
 * json string 中的
 *   1. \" 转换成 " (一般存到内存中时，不用存'\'，否则与人沟通时会有问题)
 */
char* mstr_ndup_json_string(const char *s, size_t n);


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
void mstr_appendvf(MSTR *astr, const char *fmt, va_list ap);
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
/* 返回最多 len 个随机单词[A-Za-z0-9]组成的字符串 */
void mstr_rand_word(char *s, size_t maxlen);
/* 返回最多 len 个随机数字[0-9]组成的字符串 */
void mstr_rand_digit(char *s, size_t maxlen);
/* 返回最多 len 个随机16进制数字[0-9a-f]组成的字符串 */
void mstr_rand_hexstring(char *s, size_t maxlen);

/*
 * 返回由 len 个随机字母(a ~ z)组成的字符串
 * 调用者需确保 s 的内存长度 >= len + 1
 */
void mstr_rand_string_fixlen(char *s, size_t len);
/* 返回由 len 个随机单词[A-Za-z0-9]组成的字符串 */
void mstr_rand_word_fixlen(char *s, size_t len);
/* 返回由 len 个随机数字[0-9]组成的字符串 */
void mstr_rand_digit_fixlen(char *s, size_t len);
/* 返回由 len 个随机16进制数字[0-9]组成的字符串 */
void mstr_rand_hexstring_fixlen(char *s, size_t len);


/*
 * 以 sep 为分割符，将 str 分割为最多为 max 个元素的列表。
 * alist 为新申请的列表，调用者请自行释放。
 */
MERR* mstr_array_split(MLIST **alist, const char *sin, const char *sep, int max);

/*
 * 大小写转换，修改原字符串
 */
char* mstr_tolower(char *s);
char* mstr_toupper(char *s);

/*
 * 将一片二进制数组，转换成16进制字符串（一般用于网络包调试）
 * 例如：uint8_t hexin[4] = {12,5,6,15}
 *      将会转换成 "0c05060f"，串后会补 '\0'
 * charout 的内存长度必须 >= inlen * 2 + 1
 *
 * what a fuck: after hours work, we can do it use sprintf(charout[i], "%x", hexin[j])
 *
 */
void mstr_bin2hexstr(uint8_t *hexin, unsigned int inlen, char *charout);

/*
 * 将一片二进制数组，不可打印部分((<31 || > 127) && !=9 && != 10) 转换成16进制字符串，
 * 保留可打印部分不变
 * charout 的内存长度必须 >= inlen * 2 + 1
 */
void mstr_bin2str(uint8_t *hexin, unsigned int inlen, char *charout);

/*
 * 将一个16进制字符串转换成二进制数组（通常 uint8_t 数组）, hexout 的长度必须为 inlen/2
 * charin: '0123456789abcdef'
 * hexout: usually uint8_t*
 */
void mstr_hexstr2bin(const unsigned char *charin, unsigned int inlen, uint8_t *hexout);


/*
 * 比较2个版本号，如 2.0.32, 2.0.21
 * 返回 1: a > b, 0: a = b, -1: a < b
 */
int mstr_version_compare(char *a, char *b);


__END_DECLS
#endif
