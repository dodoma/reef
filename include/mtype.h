#ifndef __MTYPE_H__
#define __MTYPE_H__

/*
 * mtype, moon's data type declaration
 */
__BEGIN_DECLS

/*
 * 类似 MLIST 的操作手法， 将 struct _MSTR 移到 mstring.c 可以起到程序自我保护作用。
 * （因为其他文件中的代码不能访问 MSTR 的内部成员）
 * 比如 mstr_array_split() 中，不能访问 llist->num 变量，必须使用 mlist_length(llist)
 * 但，移到 mstring.c 后，外部便不能声明 MSTR astr, 只能声明 MSTR *astr
 *    否则会提示 error: storage size of 'str' isn't known
 *      the MSTR structure on the header file is just a forward declaration,
 *      when it's included on rheads.h, it only has knowledge of the
 *      forward declaration but no clue of how it's defined (or in this case,
 *      what the size of the struct is). define your struct on the header file instead.
 */
struct _MSTR {
    char *buf;
    size_t len;
    size_t max;
};
typedef struct _MSTR MSTR;

struct _MBUF {
    char *buf;
    size_t len;
    size_t max;
};
typedef struct _MBUF MBUF;

typedef enum {
    MERR_NOMEM = -30999,
    MERR_DB,
    MERR_NET,
    MERR_SYSTEM,
    MERR_ASSERT,
    MERR_EMPTY,
    MERR_OPENFILE,
    MERR_NEXIST,

    MERR_PASS = -30800
} MERR_CODE;
typedef struct _MERR MERR;

typedef struct _MLIST MLIST;
typedef struct _MSLIST MSLIST;
typedef struct _MDLIST MDLIST;
typedef struct _MTREE MTREE;
typedef struct _MUTREE MUTREE;

typedef uint32_t (*MHASH_HASH_FUNC)(const void*);
typedef int (*MHASH_COMP_FUNC)(const void*, const void*);
typedef void (*MHASH_DESTROY_FUNC)(void *key, void *value);
typedef struct _MHASH MHASH;

typedef struct _MD5CTX {
    uint32_t state[4];            /* state (ABCD) */
    uint32_t count[2];            /* number of bits, modulo 2^64 (lsb first) */
    unsigned char buffer[64];     /* input buffer */
} md5_ctx;                        /* 此 md5_ctx 给 md5.c 使用，暂不统一风格 */
typedef struct _MD5CTX MD5CTX;

/*
 * 一个节点只能有一个类型，故此，如下操作：
 *   mdf_set_value(node, "a", "value a");
 *   mdf_set_value(node, "a.x", "value b");
 * 后一个赋值会将 a 节点转换成对象类型(value a 会被释放)。
 */
typedef enum {
    MDF_TYPE_UNKNOWN = 0,       /* 0 */
    MDF_TYPE_OBJECT,            /* 1 */
    MDF_TYPE_ARRAY,             /* 2 */
    MDF_TYPE_STRING,            /* 3 */

    /* TYPE NUMBER */
    MDF_TYPE_INT,               /* 4 */
    MDF_TYPE_DOUBLE,            /* 5 */

    MDF_TYPE_BOOL,              /* 6 */
    MDF_TYPE_BINARY_A,          /* 7 由 mdf 申请和释放的二进制数据 */
    MDF_TYPE_BINARY_B,          /* 8 由 业务 管理的二进制数据 */

    MDF_TYPE_NULL               /* 9 */
} MDF_TYPE;
typedef struct _MDF MDF;
typedef struct _MCS MCS;

typedef void (*MDF_PRINTF)(void *rock, const char *fmt, ...);
typedef void (*MCS_PRINTF)(void *rock, const char *fmt, ...);

typedef enum
{
    MMC_OP_SET = 0,
    MMC_OP_ADD,
    MMC_OP_REPLACE,
    MMC_OP_APPEND,
    MMC_OP_PREPEND,
    MMC_OP_CAS,
    MMC_OP_INC,
    MMC_OP_DEC
} MMC_OP;

typedef struct _MRE MRE;

typedef void (*MHTTP_ONBODY_FUNC)(unsigned char *pos, size_t len, const char *bodytype, void *arg);

enum {
    MCGI_METHOD_UNKNOWN = 0,
    MCGI_METHOD_GET,
    MCGI_METHOD_POST,
    MCGI_METHOD_PUT
};

enum {
    MCGI_REQ_UNKNOWN  = 0,
    MCGI_REQ_HTML,
    MCGI_REQ_JSON,
    MCGI_REQ_IMAGE,
    MCGI_REQ_AUDIO
};

typedef struct _MCGI MCGI;
typedef struct _MCGI_UPFILE MCGI_UPFILE;

/*
 * 回调函数可以 link fname 到另外的地址，然后返回链接到的文件名，这样，文件即可永久创建并赋值到 UPLOAD
 * name         : posted key name (by client)
 * filename     : posted filename header value (by client)
 * fname        : file name on server disk (temprary)
 * return char* : file name on server disk (permanently)
 */
typedef char* (*MCGI_UPLOAD_FUNC)(const char *name, const char *filename, char fname[PATH_MAX], FILE *fp);

__END_DECLS
#endif
