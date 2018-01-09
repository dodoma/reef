#include "reef.h"

/* TODO If we've seen it before... we force it into a list */

#define MAX_TOKEN    1024      /* max cookie, contenttype, query var variable number */
#define MAX_POST_LEN 524288    /* post application/x-www-form-urlencoded max length */
#define MAX_BUF_LEN  10485760  /* post multipart/form-data buffer length*/

struct _MCGI {
    int method;                 /* GET, POST, PUT */
    int reqtype;                /* html, ajax, image */

    MDF *data;
    MLIST *files;

    struct rfc2388 *r;

    double time_start;
    double time_end;
};

struct rfc2388 {
    char *name;
    char *filename;
    char *type;

    FILE *fp;
    MSTR *value;

    bool headering;

    unsigned char *buf;
    char *pos;
    int remain;
};

#include "_cgi_parse.c"

static void _fclose(void *p)
{
    if (p) fclose(p);
}

MERR* mcgi_init(MCGI **ses, char **envp)
{
    MERR *err;

    MERR_NOT_NULLA(ses);

    MCGI *rses = mos_calloc(1, sizeof(MCGI));
    rses->method = MCGI_METHOD_UNKNOWN;
    rses->reqtype = MCGI_REQ_UNKNOWN;
    rses->time_start = mos_timef();
    rses->time_end = 0;
    mdf_init(&rses->data);
    mlist_init(&rses->files, _fclose);

    struct rfc2388 *r = mos_calloc(1, sizeof(struct rfc2388));
    r->name = NULL;
    r->filename = NULL;
    r->type = NULL;
    r->fp = NULL;
    r->value = mos_calloc(1, sizeof(MSTR));
    r->headering = false;
    r->buf = NULL;
    r->pos = NULL;
    r->remain = 0;
    rses->r = r;

    err = _parse_env(rses);
    if (err) return merr_pass(err);

    err = _parse_http_headers(rses, envp);
    if (err) return merr_pass(err);

    err = _parse_cookie(rses);
    if (err) return merr_pass(err);

    err = _parse_content_type(rses);
    if (err) return merr_pass(err);

    err = _parse_query(rses);
    if (err) return merr_pass(err);

    *ses = rses;

    return MERR_OK;
}

void mcgi_destroy(MCGI **ses)
{
    if (!ses || !*ses) return;

    MCGI *lses = *ses;

    struct rfc2388 *r = lses->r;
    if (r->fp) fclose(r->fp);
    mos_free(r->name);
    mos_free(r->filename);
    mos_free(r->type);
    mstr_clear(r->value);
    mos_free(r->value);
    mos_free(r->buf);
    mos_free(r);

    mdf_destroy(&lses->data);
    mlist_destroy(&lses->files);
    lses->r = NULL;

    *ses = NULL;
}

MERR* mcgi_parse_payload(MCGI *ses)
{
    MERR *err;

    MERR_NOT_NULLA(ses);

    if (ses->method == MCGI_METHOD_POST) {
        char *type = mdf_get_value(ses->data, "CGI.CONTENT_TYPE.type", "unknown");
        if (!strncmp(type, "application/x-www-form-urlencoded", 33)) {
            err = _parse_payload_post_form(ses);
            if (err) return merr_pass(err);
        } else if (!strncmp(type, "multipart/form-data", 19)) {
            err = _parse_payload_rfc2388(ses);
            if (err) return merr_pass(err);
        } else if (!strncmp(type, "application/json", 16)) {
            err = _parse_payload_json(ses);
            if (err) return merr_pass(err);
        } else if (!strncmp(type, "text/xml", 8)) {
            err = _parse_payload_xml(ses);
            if (err) return merr_pass(err);
        }
    } else if (ses->method == MCGI_METHOD_PUT) {
        err = _parse_payload_put(ses);
        if (err) return merr_pass(err);
    }

    //MDF_TRACE_MT(ses->data);

    return MERR_OK;
}

int mcgi_req_type(MCGI *ses)
{
    return ses->reqtype;
}

/* TODO next functions */
void mcgi_redirect(MCGI *ses, const char *url)
{
}

MERR* mcgi_cookie_set(MCGI *ses, const char *name, const char *value,
                      const char *path, const char *domain, int duration)
{
    return MERR_OK;
}

MERR* mcgi_cookie_clear(MCGI *ses, const char *name, const char *domain, const char *path)
{
    return MERR_OK;
}

FILE* mcgi_file(MCGI *ses, const char *name)
{
}
