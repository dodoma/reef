#ifndef __MHTTP_EXT_H__
#define __MHTTP_EXT_H__

/*
 * mhttp_ext, extensions for http
 */
__BEGIN_DECLS

bool mhttp_getx(const char *url);
bool mhttp_getxf(const char *fmt, ...) ATTRIBUTE_PRINTF(1, 2);
bool mhttp_postx(const char *url, const char *payload);
bool mhttp_post_dnodex(const char *url, MDF *dnode, bool useJSON);

bool mhttp_get_json(const char *url, MDF *body);
bool mhttp_get_jsonf(MDF *body, const char *fmt, ...) ATTRIBUTE_PRINTF(2, 3);
bool mhttp_download(const char *url, const char *filename);
bool mhttp_downloadf(const char *url, const char *fmt, ...) ATTRIBUTE_PRINTF(2, 3);
bool mhttp_post_file(const char *url, const char *key, const char *filename, MSTR *astr);
bool mhttp_post_with_filex(const char *url, MDF *dnode);


__END_DECLS
#endif
