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

    MERR_PASS = -30888
} MERR_CODE;
typedef struct _MERR MERR;

typedef struct _MLIST MLIST;

__END_DECLS
#endif
