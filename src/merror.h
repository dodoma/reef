#ifndef __MERROR_H__
#define __MERROR_H__

#include "rheads.h"

/*
 * merror, error handle
 * moon reserved error code: -30999 ~ -30800
 */
__BEGIN_DECLS

#define MERR_OK ((MERR*)0)

struct _MERR {
    MERR_CODE code;
    const char *func;
    const char *file;
    int lineno;

    char desc[1024];

    struct _MERR *next;
};

#define merr_pass(e) merr_pass_raw(__func__, __FILE__, __LINE__, e)
#define merr_raise(code, ...) merr_raisef(__func__, __FILE__, __LINE__, code, __VA_ARGS__)

MERR* merr_raisef(const char *func, const char *file, int lineno,
                  MERR_CODE code, const char *fmt, ...) ATTRIBUTE_PRINTF(5, 6);

MERR* merr_pass_raw(const char *func, const char *file, int lineno, MERR *error);

bool merr_match(MERR *error, MERR_CODE code);
void merr_traceback(MERR *error, MSTR *str);
void merr_destroy(MERR **error);

const char* merr_code_2_str(MERR_CODE code);


__END_DECLS
#endif
