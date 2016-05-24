#ifndef __MHTTP_H__
#define __MHTTP_H__

/*
 * mhttp, http tools, http 协议相关的工具函数
 */
__BEGIN_DECLS

/*
 * 类似 decodeURIComponent 功能，处理http url 传参
 * 修改输入参数 s
 */
char* mhttp_url_unescape(char *s, int buflen, char esc_char);


__END_DECLS
#endif
