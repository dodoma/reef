#ifndef __MBASE64_H__
#define __MBASE64_H__

/*
 * mbase64, base64 encode, decode
 */
__BEGIN_DECLS

int mb64_encode(const unsigned char *in, size_t in_len, char *out, size_t out_len);
int mb64_decode(const char *in, size_t in_len, unsigned char *out, size_t out_len);

__END_DECLS
#endif
