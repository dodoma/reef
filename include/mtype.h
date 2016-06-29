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

typedef enum {
    MERR_NOMEM = -30999,
    MERR_ASSERT,
    MERR_EMPTY,
    MERR_OPENFILE,
    MERR_NEXIST,

    MERR_PASS = -30800
} MERR_CODE;
typedef struct _MERR MERR;

typedef struct _MLIST MLIST;

typedef uint32_t (*MHASH_HASH_FUNC)(const void*);
typedef int (*MHASH_COMP_FUNC)(const void*, const void*);
typedef void (*MHASH_DESTROY_FUNC)(void *value);
typedef struct _MHASH MHASH;

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
    MDF_TYPE_FLOAT,             /* 5 */

    MDF_TYPE_BOOL,              /* 6 */
    MDF_TYPE_BINARY,            /* 7 */

    MDF_TYPE_NULL               /* 8 */
} MDF_TYPE;
typedef struct _MDF MDF;

typedef void (*MDF_PRINTF)(void *rock, const char *fmt, ...);

/*
 * MCV moon computer vision
 */
/* channel(or color) per pixel */
typedef enum {
    MCV_CPP_1 = 0x001,
    MCV_CPP_2 = 0x002,
    MCV_CPP_3 = 0x003,
    MCV_CPP_4 = 0x004,
} MCV_CPP;

/* bit per channel(or color) */
typedef enum {
    MCV_BPC_8U =  0x01000,
    MCV_BPC_32S = 0x02000,
    MCV_BPC_32F = 0x04000,
    MCV_BPC_64S = 0x08000,
    MCV_BPC_64F = 0x10000,
} MCV_BPC;

/* matrix data type */
typedef enum {
    MCV_DATA_GRAY = 0x01001,
    MCV_DATA_RGB = 0x01003
} MCV_DATA;


typedef enum {
    MCV_FLAG_NONE = 0,
    MCV_FLAG_TINY_SIGNED,       /* 按有符号整数处理矩阵内容（此类型处理速度比正常计算更快），
                                 * 比如求和操作，可处理小于 1675x1675 的三通道256色图像求和
                                 */
    MCV_FLAG_TINY_UNSIGNED,     /* 按无符号整数处理矩阵内容
                                 * 比如求和操作，可处理小于 2369x2369 的三通道256色图像求和
                                 */
    MCV_FLAG_SIGNED,            /* compute normarlly */
    MCV_FLAG_UNSIGNED           /* compute fabs(x) of elements */
} MCV_FLAG;

/*
 * N north, S south, W west, E east
 */
typedef enum {
    MCV_DIR_NW = 0,
    MCV_DIR_NE,
    MCV_DIR_SW,
    MCV_DIR_SE
} MCV_DIRECTION;

typedef enum {
    MCV_FILE_JPEG = 0,
    MCV_FILE_BMP,
    MCV_FILE_PNG,
    MCV_FILE_RAW
} MCV_FILE_TYPE;

typedef union {
    unsigned char *u8;
    int     *i32;
    float   *f32;
    int64_t *i64;
    double  *f64;
} MCV_MAT_CELL;

typedef struct {
    int type;
    int rows;
    int cols;
    int step;
    MCV_MAT_CELL data;
} MCV_MAT;

typedef struct {
    int type;
    MCV_MAT_CELL data;
} MCV_PIXEL;


typedef struct {
    int x, y;
} MCV_POINT;

typedef struct {
    float x, y;
} MCV_DECIMAL_POINT;

typedef struct {
    int w, h;
} MCV_SIZE;

typedef struct {
    int x, y;
    int w, h;
} MCV_RECT;

__END_DECLS
#endif
