#ifndef __MHTTP_H__
#define __MHTTP_H__

/*
 * mhttp, http 协议相关的工具函数
 */
__BEGIN_DECLS

#define MHTTP_BUFLEN  524288    /* 接收http回包缓冲，函数局部空间 char[] */
#define MHTTP_TIMEOUT 5         /* http 请求超时时间 */

#ifdef DEBUG_MSG
#define MSG_DUMP(pre, p, psize)                                     \
    do {                                                            \
        if ((ssize_t)(psize) > 0) {                                 \
            char zstra[(psize)*2+1];                                \
            mstr_bin2str((uint8_t*)(p), (psize), zstra);            \
            mtc_mt_dbg("%s%zu %s", pre, (size_t)(psize), zstra);    \
        }                                                           \
    } while (0)
#else
#define MSG_DUMP(pre, p, psize)
#endif

/*
 * url: https://anothersite.com:8080/cgi/viki?aaaa=bbbb
 * OUTPUT:
 *     host: anothersite.com:8080/cgi/viki?aaaa=bbbb
 *     hostlen: 15
 *     port: 8080
 *     scriptname: cgi/viki?aaaa=bbbb
 *     scriptlen: 8
 *     requesturi: cgi/viki?aaaa=bbbb
 */
bool mhttp_parse_url(const char *url, bool *secure,
                     char **host, int *hostlen, int *port,
                     char **scriptname, int *scriptlen,
                     char **requesturi);

/*
 * 类似 encodeURIComponent 功能 ，新申请内存，记得释放
 */
char* mhttp_url_escape(const char *s);

/*
 * 类似 decodeURIComponent 功能，处理http url 传参
 * 修改输入参数 s
 */
char* mhttp_url_unescape(char *s, size_t buflen, char esc_char);

/*
 * 设置的 Dataset:
 * {
 *     HEADER {
 *         code: 200,
 *         status: "OK"
 *         Content-Length: 102,
 *         Content-Type: "application/json"
 *         ....
 *     }
 * }
 */
MERR* mhttp_get(const char *url, MDF *headernode, MDF *rnode, MHTTP_ONBODY_FUNC body_callback, void *arg);

/*
 * content_type: default "application/x-www-form-urlencoded"
 */
MERR* mhttp_post(const char *url, const char *content_type, MDF *headernode, const char *payload,
                 MDF *rnode, MHTTP_ONBODY_FUNC body_callback, void *arg);

/*
 * 以可上传文件方式(rfc2388)发送POST请求
 * Content-Type: multipart/form-data; boundary=%s\r\n\r\n
 * 请求参数放在 dnode 中
 */
MERR* mhttp_post_with_file(const char *url, MDF *dnode, MDF *rnode,
                           MHTTP_ONBODY_FUNC body_callback, void *arg);

/*
 * 以可上传文件方式(rfc2388)发送POST请求
 * Content-Type: multipart/form-data; boundary=%s\r\n\r\n
 * 请求参数以参数个数+可变参数方式传入
 */
MERR* mhttp_post_with_filef(const char *url, MDF *rnode, MHTTP_ONBODY_FUNC body_callback, void *arg,
                            int varaible_count, ...);

__END_DECLS
#endif
