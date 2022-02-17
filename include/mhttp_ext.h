#ifndef __MHTTP_EXT_H__
#define __MHTTP_EXT_H__

/*
 * mhttp_ext, extensions for http
 */
__BEGIN_DECLS

void  mhttp_getx(const char *url);
void  mhttp_getxf(const char *fmt, ...) ATTRIBUTE_PRINTF(1, 2);
MERR* mhttp_get_json(const char *url, MDF *body);
MERR* mhttp_get_jsonf(MDF *body, const char *fmt, ...) ATTRIBUTE_PRINTF(2, 3);
MERR* mhttp_download(const char *url, const char *filename);
MERR* mhttp_downloadf(const char *url, const char *fmt, ...) ATTRIBUTE_PRINTF(2, 3);
MERR* mhttp_post_file(const char *url, const char *key, const char *filename, MSTR *astr);


__END_DECLS
#endif