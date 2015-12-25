#include "reef.h"

struct _MERR {
    MERR_CODE code;
    const char *func;
    const char *file;
    int lineno;

    char desc[1024];

    struct _MERR *next;
};

MERR* merr_raisef(const char *func, const char *file, int lineno,
                  MERR_CODE code, const char *fmt, ...)
{
    va_list ap;
    MERR *err;

    err = mos_calloc(1, sizeof(MERR));

    err->code = code;
    err->func = func;
    err->file = file;
    err->lineno = lineno;

    va_start(ap, fmt);
    vsnprintf(err->desc, sizeof(err->desc), fmt, ap);
    va_end(ap);

    err->next = NULL;

    return err;
}

MERR* merr_pass_raw(const char *func, const char *file, int lineno, MERR *error)
{
    MERR *err;

    if (error == MERR_OK) return error;

    err = mos_calloc(1, sizeof(MERR));

    err->code = MERR_PASS;
    err->func = func;
    err->file = file;
    err->lineno = lineno;

    err->next = error;

    return err;
}

uint32_t merr_meanful_code(MERR *error)
{
    MERR *err;

    if (!error) return 0;

    err = error;
    while (err) {
        if (err->code != MERR_PASS) return err->code;

        err = err->next;
    }

    return 0;
}

bool merr_match(MERR *error, MERR_CODE code)
{
    MERR *err;

    if (!error || code < MERR_NOMEM || code > MERR_PASS) return false;

    err = error;
    while (err) {
        if (err->code == code) return true;

        err = err->next;
    }

    return false;
}

void merr_traceback(MERR *error, MSTR *astr)
{
    int errnum;
    MERR *err;

    if (!error || !astr) return;

    err = error;

    mstr_append(astr, "Traceback:\n");

    errnum = 0;
    while (err) {
        errnum++;

        for (int i = 0; i < errnum; i++) mstr_append(astr, "  ");

        mstr_appendf(astr, "%s:%d %s(): %s\n",
                     err->file, err->lineno, err->func, merr_code_2_str(err->code));

        if (err->desc[0]) {
            for (int i = 0; i < errnum; i++) mstr_append(astr, "  ");
            mstr_appendf(astr, "%s\n", err->desc);
        }

        err = err->next;
    }
}

void merr_destroy(MERR **error)
{
    MERR *err, *next;

    if (!error || !*error) return;

    err = *error;
    while (err) {
        next = err->next;

        mos_free(err);

        err = next;
    }

    *error = MERR_OK;
}

const char* merr_code_2_str(MERR_CODE code)
{
    switch (code) {
    case MERR_NOMEM:
        return "out of memory";
    case MERR_ASSERT:
        return "assert error";
    case MERR_EMPTY:
        return "object empty";
    case MERR_OPENFILE:
        return "open file failure";
    case MERR_NEXIST:
        return "object not exist";
    case MERR_PASS:
        return "internal pass";
    default:
        return "unknown error";
    }
}
