#ifndef __MCGI_H__
#define __MCGI_H__

/*
 * mcgi, Common Gateway Interface 封装
 *
 * 设置的 Dataset: CGI, HTTP, HTTP.COOKIE, QUERY
 *
 * 保留的 url 关键字:
 *    _reqtype  *json, html, image     指定请求资源的类型
 *    _script   xxxx                   请求资源的 url， 由 http 服务器 url rewrite 生成
 */
__BEGIN_DECLS

MERR* mcgi_init(MCGI **ses, char **envp);
MERR* mcgi_parse_payload(MCGI *ses);
int   mcgi_req_type(MCGI *ses);
void  mcgi_destroy(MCGI **ses);

MDF*  mcgi_get_data(MCGI *ses);

void  mcgi_redirect(MCGI *ses, const char *url);
MERR* mcgi_cookie_set(MCGI *ses, const char *name, const char *value,
                      const char *path, const char *domain, int duration);
MERR* mcgi_cookie_clear(MCGI *ses, const char *name, const char *domain, const char *path);


__END_DECLS
#endif
