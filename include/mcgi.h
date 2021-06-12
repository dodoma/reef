#ifndef __MCGI_H__
#define __MCGI_H__

/*
 * mcgi, Common Gateway Interface 封装
 *
 * 设置的 Dataset: CGI, HTTP, HTTP.COOKIE, QUERY, UPLOAD
 *
 * 保留的 url 关键字:
 *    _reqtype  *json,html,image,auido     指定请求资源的类型
 *    _script   xxxx                       请求资源的 url， 由 http 服务器 url rewrite 生成
 */
__BEGIN_DECLS

#define MAX_TOKEN    1024      /* max cookie, contenttype, query var variable number */
#define MAX_POST_LEN 5242880   /* post application/x-www-form-urlencoded max length */

#define MCGI_BUFLEN  10485760  /* post multipart/form-data buffer length, 程序全局空间 mos_calloc() */

MERR* mcgi_init(MCGI **ses, char **envp);
MERR* mcgi_parse_payload(MCGI *ses);
void  mcgi_regist_upload_callback(MCGI *ses, MCGI_UPLOAD_FUNC up_callback);

int   mcgi_req_type(MCGI *ses);
void  mcgi_destroy(MCGI **ses);

MDF*  mcgi_get_data(MCGI *ses);

void  mcgi_redirect(MCGI *ses, const char *url);
MERR* mcgi_cookie_set(const char *name, const char *value,
                      const char *path, const char *domain, int duration);
MERR* mcgi_cookie_clear(const char *name, const char *domain, const char *path);


__END_DECLS
#endif
