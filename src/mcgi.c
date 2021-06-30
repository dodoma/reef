#include "reef.h"

struct _MCGI_UPFILE {
    char *name;
    char *filename;
    char *fname;
    FILE *fp;
};

struct _MCGI {
    int method;                 /* GET, POST, PUT */
    int reqtype;                /* html, ajax, image, audio */

    MDF *data;

    MLIST *files;
    MLIST *upcallbacks;

    struct rfc2388 *r;

    double time_start;
    double time_end;

    /* TODO Cache-Control */
};

struct rfc2388 {
    char *name;                 /* posted key name (by client) */
    char *filename;             /* posted filename header value (by client) */
    char *type;
    char fname[PATH_MAX];       /* file name on server disk (temprary) */

    FILE *fp;
    MSTR *value;

    bool headering;

    unsigned char *buf;
    char *pos;
    int remain;
};

#include "_cgi_parse.c"

static void _upfile_free(void *p)
{
    if (!p) return;

    MCGI_UPFILE *ufp = (MCGI_UPFILE*)p;

    mos_free(ufp->name);
    mos_free(ufp->filename);
    mos_free(ufp->fname);
    fclose(ufp->fp);
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
    mlist_init(&rses->files, _upfile_free);
    mlist_init(&rses->upcallbacks, NULL);

    struct rfc2388 *r = mos_calloc(1, sizeof(struct rfc2388));
    r->name = NULL;
    r->filename = NULL;
    r->type = NULL;
    memset(r->fname, 0x0, sizeof(r->fname));
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
    mlist_destroy(&lses->upcallbacks);
    lses->r = NULL;

    *ses = NULL;
}

MDF* mcgi_get_data(MCGI *ses)
{
    if (!ses) return NULL;

    return ses->data;
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

void mcgi_regist_upload_callback(MCGI *ses, MCGI_UPLOAD_FUNC up_callback)
{
    if (!ses || !up_callback) return;

    mlist_append(ses->upcallbacks, up_callback);
}

int mcgi_req_type(MCGI *ses)
{
    if (!ses) return MCGI_REQ_UNKNOWN;
    return ses->reqtype;
}

MERR* mcgi_cookie_set(const char *name, const char *value,
                      const char *path, const char *domain, int duration)
{
    if (!path) path = "/";

    MSTR astr; mstr_init(&astr);

    mstr_appendf(&astr, "Set-Cookie: %s=%s; path=%s", name, value, path);

    if (domain) mstr_appendf(&astr, "; domain=%s", domain);
    if (duration > 0) {
        char my_time[256] = {0};
        time_t exp_date = time(NULL) + (86400 * duration);
        strftime (my_time, 48, "%A, %d-%b-%Y 23:59:59 GMT", gmtime(&exp_date));
        mstr_appendf(&astr, "; expires=%s", my_time);
    }

    mstr_append(&astr, "\r\n");
    printf("%s", astr.buf);
    mstr_clear(&astr);

    return MERR_OK;
}

MERR* mcgi_cookie_clear(const char *name, const char *domain, const char *path)
{
    if (!path) path = "/";

    if (domain) {
        if (domain[0] == '.') domain = domain + 1;
        printf("Set-Cookie: %s=; path=%s; domain=%s; expires=Thursday, 01-Jan-1970 00:00:00 GMT\r\n",
               name, path, domain);
    } else {
        printf("Set-Cookie: %s=; path=%s; expires=Thursday, 01-Jan-1970 00:00:00 GMT\r\n", name, path);
    }

    return MERR_OK;
}

/* TODO next functions */
FILE* mcgi_file(MCGI *ses, const char *name)
{
    return NULL;
}
