#ifndef __MHTTP_H__
#define __MHTTP_H__

/*
 * mhttp, http 协议相关的工具函数
 */
__BEGIN_DECLS

#define MHTTP_BUFLEN  524288    /* 接收http回包缓冲，函数局部空间 char[] */
#define MHTTP_TIMEOUT 5         /* http 请求超时时间 */

/*
 * 使用 mstr_bin2hexstr() 十六进制格式输出，方便重现故障
 * 此处可以改成 mstr_bin2str() 方便查看字符串
 * 也可以通过 mstr_hexstr2bin("") 之后再 mstr_bin2str() 查看字符串
 */
#define MSG_DUMP(pre, p, psize)                                     \
    do {                                                            \
        if ((ssize_t)(psize) > 0) {                                 \
            char zstra[(psize)*2+1];                                \
            mstr_bin2str((uint8_t*)(p), (psize), zstra);            \
            mtc_dbg("%s%zu %s", pre, (size_t)(psize), zstra);       \
        }                                                           \
    } while (0)

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
 * rnode 里设置的 Dataset:
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
 * POST 请求常用的有三种参数组织方式：
 *
 * 1. application/x-www-form-urlencoded（默认）
 *     char *payload = "client_id=41c986e3334&client_secret=81d9b75b&grant_type=silent";
 * 2. application/json
 *     char *payload = '{"client_id":41c986e3334,"client_secret":"81d9bd5b","grant_type":"silent"}'
 * 3. multipart/form-data; boundary=%s
 *
 * 此函数实现了1，2种方式（用content_type切换），第三种由 mhttp_post_with_file[f]() 实现
 */
MERR* mhttp_post(const char *url, const char *content_type, MDF *headernode, const char *payload,
                 MDF *rnode, MHTTP_ONBODY_FUNC body_callback, void *arg);

/*
 * 非文件内容POST，内部调用mhttp_post()，参数已经在 dnode 中准备好
 */
MERR* mhttp_post_dnode(const char *url, MDF *headernode, MDF *dnode,
                       MDF *rnode, MHTTP_ONBODY_FUNC body_callback, void *arg, bool useJSON);

/*
 * 以可上传文件方式(rfc2388)发送POST请求（表单提交的方式，带或不带文件上传均可）
 * Content-Type: multipart/form-data; boundary=%s\r\n\r\n
 * 请求参数放在 dnode 中
 */
MERR* mhttp_post_with_file(const char *url, MDF *dnode, MDF *rnode,
                           MHTTP_ONBODY_FUNC body_callback, void *arg);

/*
 * 以可上传文件方式(rfc2388)发送POST请求（表单提交的方式，带或不带文件上传均可）
 * Content-Type: multipart/form-data; boundary=%s\r\n\r\n
 * 请求参数以参数个数+可变参数方式传入
 */
MERR* mhttp_post_with_filef(const char *url, MDF *rnode, MHTTP_ONBODY_FUNC body_callback, void *arg,
                            int varaible_count, ...);

__END_DECLS
#endif
