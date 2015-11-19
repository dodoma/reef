#ifndef __MTYPE_H__
#define __MTYPE_H__

/*
 * mtype, moon's data type declaration
 */
__BEGIN_DECLS

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
