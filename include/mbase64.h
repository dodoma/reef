#ifndef __MBASE64_H__
#define __MBASE64_H__

/*
 * mbase64, base64 encode, decode
 */
__BEGIN_DECLS

/*
 * out_len >= (in_len + 2) / 3 * 4 + 1, return actual len
 * out_len 设为以上公式长度编码时，会被填满（不够 4 的用 '=' 填充），不会浪费空间。
 */
int mb64_encode(const unsigned char *in, size_t in_len, char *out, size_t out_len);

/*
 * out_len >= in_len / 4 * 3, return acture len
 * 解码出来的字符串长度最多为以上公式，具体多少，得用返回值进行修正。
 */
int mb64_decode(const char *in, size_t in_len, unsigned char *out, size_t out_len);

__END_DECLS
#endif
