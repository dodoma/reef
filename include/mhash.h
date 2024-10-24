#ifndef __MHASH_H__
#define __MHASH_H__

/*
 * mhash, hash table data type
 */
__BEGIN_DECLS

MERR* mhash_init(MHASH **table, MHASH_HASH_FUNC hash_func,
                 MHASH_COMP_FUNC comp_func, MHASH_DESTROY_FUNC destroy_func);
void  mhash_destroy(MHASH **table);

/*
 * 哈希表中的 key 和 value 直接使用业务端传来的参数赋值（不会另外 malloc + memcpy），
 * 故，请业务端结合 destroy_func 自行确保 key, value 的持续性 和 释放
 */
MERR* mhash_insert(MHASH *table, void *key, void *value);
void* mhash_lookup(MHASH *table, void *key);
bool  mhash_has_key(MHASH *table, void *key);
bool  mhash_remove(MHASH *table, void *key);
void* mhash_next(MHASH *table, void **key);

uint32_t mhash_length(MHASH *table);

uint32_t mhash_str_hash(const void *a);
int      mhash_str_comp(const void *a, const void *b);
void     mhash_str_free(void *key, void *val);

uint32_t mhash_int_hash(const void *a);
int      mhash_int_comp(const void *a, const void *b);

/* 098f6bcd4621d373cade4e832627b4f6 */
void mhash_md5_buf(unsigned char *in, size_t len, unsigned char out[16]);
ssize_t mhash_md5_file(const char *filename, unsigned char out[16]);
ssize_t mhash_md5_file_s(const char *filename, char hexstr[33]);
/* 7d726587934dc8f29e2e42b88a0756be1c47fa64 */
void mhash_sha1_buf(unsigned char *in, size_t len, unsigned char out[20]);
ssize_t mhash_sha1_file(const char *filename, unsigned char out[20]);
ssize_t mhash_sha1_file_s(const char *filename, char hexstr[41]);
/* ecab4881ee80ad3d76bb1da68387428ca752eb885e52621a3129dcf4d9bc4fd4 */
void mhash_sha256_buf(unsigned char *in, size_t len, unsigned char out[32]);
void mhash_hmac_sha256(unsigned char *data, size_t data_len, unsigned char *key, size_t key_len,
                       unsigned char out[32]);

/*
 * 为尽可能少的修改公共代码，做层包裹
 */
void mhash_md5_init(MD5CTX *ctx);
void mhash_md5_update(MD5CTX *ctx, unsigned char *buf, size_t len);
void mhash_md5_final(unsigned char checksum[16], MD5CTX *ctx);

__END_DECLS
#endif
