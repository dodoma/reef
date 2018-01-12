#define TURNCATE_LINE(pos, len)                                         \
    do {                                                                \
        *((pos) + (len)) = '\0';                                        \
        if ((len) > 1 && *((pos) + (len) - 1) == '\r') *((pos) + (len) - 1) = '\0'; \
    } while (0)

#define HEADER_VALUE(string, key)                                       \
    do {                                                                \
        char *_rsv_p = strchr((string), ':');                           \
        if (_rsv_p && *(++_rsv_p)) {                                    \
            while (*_rsv_p && isspace(*_rsv_p)) _rsv_p++;               \
            char *_rsv_q = _rsv_p;                                      \
            while (*_rsv_q && !isspace(*_rsv_q) && *_rsv_q != ';') _rsv_q++; \
            if (_rsv_q > _rsv_p) {                                      \
                int _rsv_len = _rsv_q - _rsv_p;                         \
                key = mos_calloc(1, (_rsv_len + 1));                    \
                memcpy(key, _rsv_p, (_rsv_len));                        \
                key[_rsv_len] = '\0';                                   \
            } else key = NULL;                                          \
        } else key = NULL;                                              \
    } while (0)

#define HEADER_ATTR(string, key)                \
    do {                                        \
        char *_rsv_p = strchr((string), '=');   \
        if (_rsv_p && *(++_rsv_p)) {            \
            _rsv_p = mstr_strip_space(_rsv_p);  \
            _rsv_p = mstr_strip(_rsv_p, '"');   \
            if (_rsv_p) key = strdup(_rsv_p);   \
            else key = NULL;                    \
        }                                       \
    } while (0)

char *_env_vars[] = {
    "CONTENT_TYPE",
    "CONTENT_LENGTH",
    "DOCUMENT_ROOT",
    "GATEWAY_INTERFACE",

    "HTTP_COOKIE",
    "HTTP_HOST",
    "HTTP_REFERER",
    "HTTP_USER_AGENT",

    "PATH",
    "PATH_INFO",
    "PATH_TRANSLATED",

    "QUERY_STRING",
    "REQUEST_METHOD",
    "REQUEST_URI",
    "SCRIPT_FILENAME",
    "SCRIPT_NAME",

    "REDIRECT_QUERY_STRING",
    "REDIRECT_STATUS",
    "REDIRECT_URL",

    "REMOTE_ADDR",
    "REMOTE_HOST",
    "REMOTE_IDENT",
    "REMOTE_PORT",
    "REMOTE_USER",
    "REMOTE_GROUP",

    "SERVER_ADDR",
    "SERVER_ADMIN",
    "SERVER_NAME",
    "SERVER_PORT",
    "SERVER_ROOT",
    "SERVER_PROTOCOL",
    "SERVER_SOFTWARE",

    "HTTPS",
    /* TODO https vars */
    NULL
};

MERR* _parse_env(MCGI *ses)
{
    MDF *node = ses->data;
    int x = 0;

    while (_env_vars[x]) {
        char *s = getenv(_env_vars[x]);

        mtc_mt_noise("enviorment %s %s", _env_vars[x], s);

        mdf_set_valuef(node, "CGI.%s=%s", _env_vars[x], s);

        x++;
    }

    char *method = mdf_get_value(node, "CGI.REQUEST_METHOD", "GET");
    if (!strcmp(method, "POST")) ses->method = MCGI_METHOD_POST;
    else if (!strcmp(method, "PUT")) ses->method = MCGI_METHOD_PUT;
    else ses->method = MCGI_METHOD_GET;

    mdf_set_type(node, "CGI.CONTENT_LENGTH", MDF_TYPE_INT);
    mdf_set_type(node, "CGI.REMOTE_PORT", MDF_TYPE_INT);
    mdf_set_type(node, "CGI.SERVER_PORT", MDF_TYPE_INT);

    return MERR_OK;
}

MERR* _parse_http_headers(MCGI *ses, char **envp)
{
    if (!envp) return MERR_OK;

    MDF *node = ses->data;

    while (*envp) {
        char *s = strdup(*envp);
        char *k, *v;

        mtc_mt_noise("parse %s", s);

        k = s;
        v = strchr(s, '=');

        if (s && strlen(s) > 5 && !strncmp (s, "HTTP_", 5) && v) {
            k += 5;
            *v++ = '\0';

            mdf_set_valuef(node, "HTTP.%s=%s", k, v);
        }

        free(s);
        envp++;
    }

    return MERR_OK;
}


MERR* _parse_cookie(MCGI *ses)
{
    MDF *node = ses->data;
    char *cookie = mdf_get_value(node, "HTTP.COOKIE", NULL);
    MERR *err;

    if (!cookie) return MERR_OK;

    mtc_mt_noise("parse cookie %s", cookie);

    MLIST *alist;
    err = mstr_array_split(&alist, cookie, ";", MAX_TOKEN);
    if (err) return merr_pass(err);

    char *item;
    MLIST_ITERATE(alist, item) {
        item = mstr_strip_space(item);

        mdf_set_valuef(node, "HTTP.COOKIE.%s", item);
    }
    mlist_destroy(&alist);

    return MERR_OK;
}

MERR* _parse_content_type(MCGI *ses)
{
    MDF *node = ses->data;
    char *contenttype = mdf_get_value(node, "CGI.CONTENT_TYPE", NULL);
    MERR *err;

    if (!contenttype) return MERR_OK;

    mtc_mt_noise("parse contenttype %s", contenttype);

    MLIST *alist;
    err = mstr_array_split(&alist, contenttype, ";", MAX_TOKEN);
    if (err) return merr_pass(err);

    char *item;
    MLIST_ITERATE(alist, item) {
        item = mstr_strip_space(item);

        if (_moon_i == 0) mdf_set_value(node, "CGI.CONTENT_TYPE.type", item);
        else mdf_set_valuef(node, "CGI.CONTENT_TYPE.%s", item);
    }
    mlist_destroy(&alist);

    return MERR_OK;
}

MERR* _parse_query(MCGI *ses)
{
    MDF *node = ses->data;
    char *query = mdf_get_value(node, "CGI.QUERY_STRING", NULL);
    MERR *err;

    if (!query) return MERR_OK;

    mtc_mt_noise("parse query %s", query);

    MLIST *alist;
    err = mstr_array_split(&alist, query, "&", MAX_TOKEN);
    if (err) return merr_pass(err);

    char *item;
    MLIST_ITERATE(alist, item) {
        item = mstr_strip_space(item);
        item = mhttp_url_unescape(item, strlen(item), '%');

        mdf_set_valuef(node, "QUERY.%s", item);
    }
    mlist_destroy(&alist);

    char *s = mdf_get_value(node, "QUERY._reqtype", "json");
    if (!strcmp(s, "json")) {
        ses->reqtype = MCGI_REQ_JSON;
    } else if (!strcmp(s, "html")) {
        ses->reqtype = MCGI_REQ_HTML;
    } else if (!strcmp(s, "image")) {
        ses->reqtype = MCGI_REQ_IMAGE;
    } else ses->reqtype = MCGI_REQ_JSON;

    return MERR_OK;
}


MERR* _parse_payload_post_form(MCGI *ses)
{
    MDF *node = ses->data;
    MERR *err;

    int len = mdf_get_int_value(node, "CGI.CONTENT_LENGTH", 0);
    if (len <= 0) return MERR_OK;
    if (len > MAX_POST_LEN) return merr_raise(MERR_ASSERT, "post length overflow %d %d", len, MAX_POST_LEN);

    char *buf = mos_calloc(1, len + 1);
    size_t readed = 0, rv = 0;

#define RETURN(ret)                             \
    do {                                        \
        mos_free(buf);                          \
        return ret;                             \
    } while (0)

    while (readed < len) {
        rv = fread(buf + readed, 1, len - readed, stdin);
        if (rv <= 0) break;
        readed += rv;
    }
    buf[len] = '\0';

    if (readed != len) RETURN(merr_raise(MERR_ASSERT, "Short readed on CGI POST input %zu %d", readed, len));

    mtc_mt_noise("parse post form %d %s", len, buf);

    MLIST *alist;
    err = mstr_array_split(&alist, buf, "&", MAX_TOKEN);
    if (err) RETURN(merr_pass(err));

    char *item;
    MLIST_ITERATE(alist, item) {
        item = mstr_strip_space(item);
        item = mhttp_url_unescape(item, strlen(item), '%');

        mdf_set_valuef(node, "QUERY.%s", item);
    }
    mlist_destroy(&alist);

    RETURN(MERR_OK);

#undef RETURN
}

#include "_rfc2388.c"

MERR* _parse_payload_rfc2388(MCGI *ses)
{
    MDF *node = ses->data;
    struct rfc2388 *r = ses->r;

    char *boundary = mdf_get_value(node, "CGI.CONTENT_TYPE.boundary", NULL);
    size_t total_to_get = mdf_get_int64_value(node, "CGI.CONTENT_LENGTH", 0);
    int boundarylen = boundary ? strlen(boundary) : 0;

    if (total_to_get <= 0 || !boundary) return MERR_OK;

    mtc_mt_noise("parse payload rfc2388 %s", boundary);

    r->buf = mos_calloc(1, MAX_BUF_LEN);
    r->remain = 0;
    size_t readed = 0;

    while (readed < total_to_get) {
        int seglen = (total_to_get - readed) > MAX_BUF_LEN ? MAX_BUF_LEN : (total_to_get - readed);

        memset(r->buf + r->remain, 0x0, seglen - r->remain);
        int rv = fread(r->buf + r->remain, 1, seglen - r->remain, stdin);
        if (rv <= 0) break;

        readed += rv;
        r->remain += rv;
        r->pos = (char*)r->buf;

        mtc_mt_dbg("readed %d bytes", rv);

        // char zstra[rv*2+1];
        // mstr_bin2str((uint8_t*)r->pos, rv, zstra);
        // mtc_mt_dbg("%s", zstra);

        int len = 0;
        while (r->remain > 0 && (len = _rfc2388_line(ses, boundary, boundarylen)) > 0) {
            r->pos += len;
            r->remain -= len;
        }

        if (r->remain > 0) memmove(r->buf, r->pos, r->remain);
    }

    if (readed != total_to_get) return merr_raise(MERR_ASSERT, "Short POST read %zu %zu", readed, total_to_get);

    return MERR_OK;
}

MERR* _parse_payload_json(MCGI *ses)
{
    MDF *node = ses->data;
    MERR *err;

    int len = mdf_get_int_value(node, "CGI.CONTENT_LENGTH", 0);
    if (len <= 0) return MERR_OK;
    if (len > MAX_POST_LEN) return merr_raise(MERR_ASSERT, "post length overflow %d %d", len, MAX_POST_LEN);

    char *buf = mos_calloc(1, len + 1);
    size_t readed = 0, rv = 0;

#define RETURN(ret)                             \
    do {                                        \
        mos_free(buf);                          \
        return ret;                             \
    } while (0)

    while (readed < len) {
        rv = fread(buf + readed, 1, len - readed, stdin);
        if (rv <= 0) break;
        readed += rv;
    }
    buf[len] = '\0';

    if (readed != len) RETURN(merr_raise(MERR_ASSERT, "Short readed on CGI POST input %zu %d", readed, len));

    //buf = mhttp_url_unescape(buf, len, '%');
    mtc_mt_noise("parse payload json %d", len);

    MDF *onode = mdf_get_or_create_node(node, "QUERY");
    err = mdf_json_import_string(onode, buf);
    if (err) RETURN(merr_pass(err));

    RETURN(MERR_OK);

#undef RETURN
}

MERR* _parse_payload_xml(MCGI *ses)
{
    MDF *node = ses->data;

    return MERR_OK;
}

MERR* _parse_payload_put(MCGI *ses)
{
    MDF *node = ses->data;

    return MERR_OK;
}
