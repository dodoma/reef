#include "reef.h"

#include "_http_tls.c"

static MERR* _connect_to(const char *host, int hostlen, int port, int *rfd, struct _tls *ssl)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return merr_raise(MERR_ASSERT, "socket() failure");

    *rfd = fd;

    char lhost[256] = {0};
    snprintf(lhost, sizeof(lhost), "%.*s", hostlen, host);
    struct in_addr ia;
    int rv = inet_pton(AF_INET, lhost, &ia);
    if (rv <= 0) {
        struct hostent *he = gethostbyname(lhost);
        if (!he) return merr_raise(MERR_ASSERT, "gethostbyname %s failure", lhost);
        ia.s_addr = *( (in_addr_t *) (he->h_addr_list[0]) );
    }

    struct sockaddr_in srvsa;
    srvsa.sin_family = AF_INET;
    srvsa.sin_port = htons(port);
    srvsa.sin_addr.s_addr = ia.s_addr;

    /* 限时连接 */
    fd_set fdset;
    long arg = fcntl(fd, F_GETFL, 0);
    if (fcntl(fd, F_SETFL, arg | O_NONBLOCK) != 0) return merr_raise(MERR_ASSERT, "set NONBLOCK failure");

    struct timeval tv;
    tv.tv_sec = MHTTP_TIMEOUT;
    tv.tv_usec = 0;
    rv = connect(fd, (struct sockaddr *)&srvsa, sizeof(srvsa));
    if (rv < 0) {
        if (errno == EINPROGRESS) {
            FD_ZERO(&fdset);
            FD_SET(fd, &fdset);
            if (select(fd + 1, NULL, &fdset, NULL, &tv) > 0) {
                int valopt;
                socklen_t slen = sizeof(int);
                getsockopt(fd, SOL_SOCKET, SO_ERROR, (void*)&valopt, &slen);
                if (valopt != 0) return merr_raise(MERR_ASSERT, "connect %s failure", lhost);
            } else return merr_raise(MERR_ASSERT, "connect %s failure", lhost);
        } else return merr_raise(MERR_ASSERT, "connect %s failure", lhost);
    }

    fcntl(fd, F_SETFL, arg);
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char*)&tv, sizeof(tv));

    /* Secure Socket Layer */
    if (ssl) {
        ssl->ssl_fd.fd = fd;
        mbedtls_ssl_set_bio(&ssl->ssl, &ssl->ssl_fd, mbedtls_net_send, mbedtls_net_recv, mbedtls_net_recv_timeout);
        while ((rv = mbedtls_ssl_handshake(&ssl->ssl)) != 0) {
            if (rv != MBEDTLS_ERR_SSL_WANT_READ && rv != MBEDTLS_ERR_SSL_WANT_WRITE)
                return merr_raise(MERR_ASSERT, "ssl handshake error %d", rv);

            /* TODO verify */
        }
    }

    return MERR_OK;
}

static MERR* _send_to(int fd, const unsigned char *buf, size_t len, struct _tls *ssl)
{
    MSG_DUMP("send:", buf, len);

    size_t c = 0;
    int rv = 0;
    while (c < len) {

        if (!ssl) rv = send(fd, buf + c, len - c, MSG_NOSIGNAL);
        else rv = mbedtls_ssl_write(&ssl->ssl, buf + c, len - c);

        if (rv <= 0) return merr_raise(MERR_ASSERT, "send() return %d %s", rv, strerror(errno));

        c += rv;
    }

    return MERR_OK;
}

static int _recv_from(int fd, unsigned char *buf, size_t maxlen, struct _tls *ssl)
{
    if (!ssl) return recv(fd, buf, maxlen, MSG_NOSIGNAL);
    else return mbedtls_ssl_read(&ssl->ssl, buf, maxlen);
}

bool mhttp_parse_url(const char *url, bool *secure,
                     char **host, int *hostlen, int *port,
                     char **scriptname, int *scriptlen,
                     char **requesturi)
{
    if (!url) return false;

    char *s = (char*)url;
    int lport = 0, hlen = 0, slen = 0;

    if (!strncasecmp(s, "https://", 8)) {
        *secure = true;
        lport = 443;
        s += 8;
    } else if (!strncasecmp(s, "http://", 7)) {
        *secure = false;
        lport = 80;
        s += 7;
    } else {
        return false;
    }

    while (*s == '/') s++;

    /* host */
    if (host) *host = s;
    while (*s && *s != '/' && *s != ':') {
        hlen++;
        s++;
    }

    /* port */
    if (*s == ':') {
        lport = 0;
        s++;

        while (*s && *s >= '0' && *s <= '9') {
            lport = lport * 10 + (*s - '0');
            s++;
        }
    }

    /* request uri */
    while (*s == '/') s++;

    if (scriptname) *scriptname = s;
    if (requesturi) *requesturi = s;

    /* script length */
    while (*s && *s != '?') {
        slen++;
        s++;
    }

    if (port) *port = lport;
    if (hostlen) *hostlen = hlen;
    if (scriptlen) *scriptlen = slen;

    return true;
}

char* mhttp_url_escape(const char *s)
{
    if (!s) return NULL;

    static char *escs = " $&+,/:;=?@ \"<>#%{}|\\^~[]`'";

    MSTR astr; mstr_init(&astr);

    while (*s) {
        uint8_t c = *s;
        if (strchr(escs, c) || c < 32 || c > 126) {
            mstr_appendc(&astr, '%');
            mstr_appendc(&astr, "0123456789ABCDEF"[c / 16]);
            mstr_appendc(&astr, "0123456789ABCDEF"[c % 16]);
        } else {
            mstr_appendc(&astr, c);
        }

        s++;
    }

    return astr.buf;
}

char* mhttp_url_unescape(char *s, size_t buflen, char esc_char)
{
    size_t i = 0, o = 0;

    if (s == NULL) return s;

    while (i < buflen) {
        if (s[i] == esc_char && (i+2 < buflen) && isxdigit((int)s[i+1]) && isxdigit((int)s[i+2])) {
            uint8_t num;
            num = (s[i+1] >= 'A') ? ((s[i+1] & 0xdf) - 'A') + 10 : (s[i+1] - '0');
            num *= 16;
            num += (s[i+2] >= 'A') ? ((s[i+2] & 0xdf) - 'A') + 10 : (s[i+2] - '0');
            s[o++] = num;
            i+=3;
        } else {
            s[o++] = s[i++];
        }
    }

    if (i && o) s[o] = '\0';

    return s;
}

#include "_http_parse.c"

MERR* mhttp_get(const char *url, MDF *headernode, MDF *rnode, MHTTP_ONBODY_FUNC body_callback, void *arg)
{
    MERR *err;

    MERR_NOT_NULLB(url, rnode);

    mdf_clear(rnode);
    mdf_set_pointer(rnode, "_tmp.callback_arg", arg);

    char *host, *scriptname, *requesturi;
    int port, hostlen, scriptlen;
    struct _tls *ssl = NULL;
    bool secure;

    if (!mhttp_parse_url(url, &secure, &host, &hostlen, &port, &scriptname, &scriptlen, &requesturi))
        return merr_raise(MERR_ASSERT, "not http url %s", url);

    if (secure) {
        char hostname[hostlen + 1];
        memset(hostname, 0x0, sizeof(hostname));
        strncpy(hostname, host, hostlen);
        err = _tls_new(&ssl, hostname, MHTTP_TIMEOUT * 1000);
        if (err) return merr_pass(err);
    }

    int fd = 0;
    MSTR str; mstr_init(&str);
    mstr_appendf(&str,
                 "GET /%s HTTP/1.1\r\n"
                 "HOST: %.*s\r\n"
                 "User-Agent: reef\r\n",
                 requesturi, hostlen, host);
    MDF *cnode = mdf_node_child(headernode);
    while (cnode) {
        mstr_appendf(&str, "%s: %s\r\n", mdf_get_name(cnode, NULL), mdf_get_value(cnode, NULL, NULL));
        cnode = mdf_node_next(cnode);
    }
    mstr_append(&str, "Accept: */*\r\n\r\n");

#define RETURN(ret)                             \
    do {                                        \
        if (secure) _tls_free(ssl);             \
        if (fd > 0) close(fd);                  \
        mstr_clear(&str);                       \
        return (ret);                           \
    } while (0)

    err = _connect_to(host, hostlen, port, &fd, ssl);
    if (err) RETURN(merr_pass(err));

    //mtc_mt_dbg("send %s", str.buf);
    err = _send_to(fd, (unsigned char*)str.buf, str.len, ssl);
    if (err) RETURN(merr_pass(err));

    unsigned char buf[MHTTP_BUFLEN];
    memset(buf, 0x0, MHTTP_BUFLEN);
    int rv = 0, len = 0;
    bool end = false;
    while ((rv = _recv_from(fd, buf + len, MHTTP_BUFLEN - len, ssl)) > 0) {
        mtc_mt_noise("received %d bytes", rv);
        //MSG_DUMP("receive:", buf + len, rv);

        buf[len + rv] = '\0';
        err = _parse_response(buf, len + rv, &len, &end, rnode, body_callback);
        if (err) RETURN(merr_pass(err));

        if (end) RETURN(MERR_OK);

        memset(buf + len, 0x0, MHTTP_BUFLEN - len);
    }

    if (rv <= 0) RETURN(merr_raise(MERR_ASSERT, "receive failure"));

    mtc_mt_warn("never reach here");
    RETURN(MERR_OK);

#undef RETURN
}

MERR* mhttp_post(const char *url, const char *content_type, MDF *headernode, const char *payload,
                 MDF *rnode, MHTTP_ONBODY_FUNC body_callback, void *arg)
{
    MERR *err;

    MERR_NOT_NULLB(url, rnode);

    mdf_clear(rnode);
    mdf_set_pointer(rnode, "_tmp.callback_arg", arg);

    char *host, *scriptname, *requesturi;
    int port, hostlen, scriptlen;
    struct _tls *ssl = NULL;
    bool secure;

    if (!mhttp_parse_url(url, &secure, &host, &hostlen, &port, &scriptname, &scriptlen, &requesturi))
        return merr_raise(MERR_ASSERT, "not http url %s", url);

    if (!content_type) content_type = "application/x-www-form-urlencoded";

    if (secure) {
        char hostname[hostlen + 1];
        memset(hostname, 0x0, sizeof(hostname));
        strncpy(hostname, host, hostlen);
        err = _tls_new(&ssl, hostname, MHTTP_TIMEOUT * 1000);
        if (err) return merr_pass(err);
    }

    int fd = 0;
    MSTR str; mstr_init(&str);
    mstr_appendf(&str,
                 "POST /%s HTTP/1.1\r\n"
                 "HOST: %.*s\r\n"
                 "User-Agent: reef\r\n",
                 requesturi, hostlen, host);
    MDF *cnode = mdf_node_child(headernode);
    while (cnode) {
        mstr_appendf(&str, "%s: %s\r\n", mdf_get_name(cnode, NULL), mdf_get_value(cnode, NULL, NULL));
        cnode = mdf_node_next(cnode);
    }
    mstr_appendf(&str,
                 "Accept: */*\r\n"
                 "Content-Type: %s\r\n"
                 "Content-Length: %zd\r\n\r\n"
                 "%s",
                 content_type, strlen(payload), payload);

#define RETURN(ret)                             \
    do {                                        \
        if (secure) _tls_free(ssl);             \
        if (fd > 0) close(fd);                  \
        mstr_clear(&str);                       \
        return (ret);                           \
    } while (0)

    err = _connect_to(host, hostlen, port, &fd, ssl);
    if (err) RETURN(merr_pass(err));

    //mtc_mt_dbg("send %s", str.buf);
    err = _send_to(fd, (unsigned char*)str.buf, str.len, ssl);
    if (err) RETURN(merr_pass(err));

    unsigned char buf[MHTTP_BUFLEN];
    memset(buf, 0x0, MHTTP_BUFLEN);
    int rv = 0, len = 0;
    bool end = false;
    while ((rv = _recv_from(fd, buf + len, MHTTP_BUFLEN - len, ssl)) > 0) {
        mtc_mt_noise("received %d bytes", rv);
        //MSG_DUMP("receive:", buf + len, rv);

        buf[len + rv] = '\0';
        err = _parse_response(buf, len + rv, &len, &end, rnode, body_callback);
        if (err) RETURN(merr_pass(err));

        if (end) RETURN(MERR_OK);

        memset(buf + len, 0x0, MHTTP_BUFLEN - len);
    }

    if (rv <= 0) RETURN(merr_raise(MERR_ASSERT, "receive failure %d %s", rv, strerror(errno)));

    mtc_mt_warn("never reach here");
    RETURN(MERR_OK);

#undef RETURN
}

MERR* mhttp_post_dnode(const char *url, MDF *headernode, MDF *dnode,
                       MDF *rnode, MHTTP_ONBODY_FUNC body_callback, void *arg, bool useJSON)
{
    MERR *err;

    if (useJSON) {
        char *payload = mdf_json_export_string(dnode);

        err = mhttp_post(url, "application/json", headernode, payload, rnode, body_callback, arg);

        free(payload);

        return merr_pass(err);
    } else {
        MSTR astr, *payload;
        mstr_init(&astr);
        payload = &astr;

        MDF *cnode = mdf_node_child(dnode);
        while (cnode) {
            char *key = mdf_get_name(cnode, NULL);
            char *val = mdf_get_value_stringfy(cnode, NULL, NULL);

            if (key && val) {
                if (astr.len > 0) mstr_appendc(payload, '&');
                mstr_appendf(payload, "%s=%s", key, val);

                free(val);
            }

            cnode = mdf_node_next(cnode);
        }

        err = mhttp_post(url, "application/x-www-form-urlencoded",
                         headernode, payload->buf, rnode, body_callback, arg);

        mstr_clear(&astr);

        return merr_pass(err);
    }

    return MERR_OK;
}

/*
--------------------------63e31c
ccefa02565..Content-Disposition:
 form-data; name="aaaa"....bbbb.
.--------------------------63e31
cccefa02565..Content-Disposition
: form-data; name="nnn"; filenam
e="EveningReflections.jpg"..Cont
ent-Type: image/jpeg....
*/
static size_t _formdata_kv_length(char *key, char *val, int boundary_length)
{
    size_t len = 0;

    if (!key || !val) return 0;

    if (val[0] != '@') {
        len += 2 + boundary_length + 2; /* --boundary\r\n */
        len += strlen("Content-Disposition: form-data; name=");
        len += 2 + strlen(key) + 4;     /* "key"\r\n\r\n */

        len += strlen(val) + 2;         /* val\r\n */
    } else {
        /* TODO JPEG, PNG, DAT, BMP... */

        val++;              /* skip @ */

        struct stat fs;
        if (stat(val, &fs) == -1) {
            mtc_mt_warn("can't open file %s", val+1);
            return 0;
        }

        char *filename = strrchr(val, '/');
        if (filename) filename++;
        else filename = val;

        len += 2 + boundary_length + 2;
        len += strlen("Content-Disposition: form-data; name=");
        len += 2 + strlen(key) + strlen("; filename=");
        len += 2 + strlen(filename) + 2;                /* "filename"\r\n */
        len += strlen("Content-Type: image/jpeg") + 4;  /* Content-Type: image/jpeg\r\n\r\n */

        len += fs.st_size + 2;                          /* fileconent\r\n */
    }

    return len;
}

static MERR* _formdata_kv_send(int fd, char *key, char *val, const char *boundary, struct _tls *ssl)
{
    if (!key || !val) return MERR_OK;

    MSTR str; mstr_init(&str);
    MERR *err;

    unsigned char buf[MHTTP_BUFLEN];
    memset(buf, 0x0, MHTTP_BUFLEN);

#define RETURN(ret)                             \
    do {                                        \
        mstr_clear(&str);                       \
        return (ret);                           \
    } while (0)

    if (val[0] != '@') {
        mstr_appendf(&str, "--%s\r\n", boundary);
        mstr_appendf(&str, "Content-Disposition: form-data; name=\"%s\"\r\n\r\n", key);
        mstr_appendf(&str, "%s\r\n", val);
        err = _send_to(fd, (unsigned char*)str.buf, str.len, ssl);
        if (err) RETURN(merr_pass(err));
    } else {
        /* TODO JPEG, PNG, DAT, BMP... */

        val++;              /* skip @ */

        char *filename = strrchr(val, '/');
        if (filename) filename++;
        else filename = val;

        mstr_appendf(&str, "--%s\r\n", boundary);
        mstr_appendf(&str, "Content-Disposition: form-data; name=\"%s\"; filename=\"%s\"\r\n",
                     key, filename);
        mstr_appendf(&str, "Content-Type: image/jpeg\r\n\r\n");
        err = _send_to(fd, (unsigned char*)str.buf, str.len, ssl);
        if (err) RETURN(merr_pass(err));

        FILE *fp = fopen(val, "r");
        if (fp) {
            size_t len = 0;
            while ((len = fread(buf, 1, MHTTP_BUFLEN, fp)) > 0) {
                err = _send_to(fd, buf, len, ssl);
                if (err) RETURN(merr_pass(err));
            }
        } else return merr_raise(MERR_ASSERT, "unable to open %s", val);

        fclose(fp);

        _send_to(fd, (unsigned char*)"\r\n", 2, ssl);
    }

    RETURN(MERR_OK);

#undef RETURN
}

static size_t _payload_length_mdf(MDF *node, int boundary_length)
{
    size_t len = 0;

    MDF *cnode = mdf_node_child(node);
    while (cnode) {
        char *key = mdf_get_name(cnode, NULL);
        char *val = mdf_get_value(cnode, NULL, NULL);

        len += _formdata_kv_length(key, val, boundary_length);

        cnode = mdf_node_next(cnode);
    }

    len += 2 + boundary_length + 2 + 2; /* --boundary--\r\n */

    return len;
}

static size_t _payload_lengthvf(int variable_count, va_list ap, int boundary_length)
{
    size_t len = 0;

    for (int i = 0; i < variable_count; i++) {
        char *key = va_arg(ap, char*);
        char *val = va_arg(ap, char*);

        len += _formdata_kv_length(key, val, boundary_length);
    }

    len += 2 + boundary_length + 2 + 2; /* --boundary--\r\n */

    return len;
}

static MERR* _send_payload_mdf(int fd, const char *boundary, MDF *node, struct _tls *ssl)
{
    MERR *err;

    MDF *cnode = mdf_node_child(node);
    while (cnode) {
        /* TODO val = mdf_get_value_stringfy() */
        char *key = mdf_get_name(cnode, NULL);
        char *val = mdf_get_value(cnode, NULL, NULL);

        err = _formdata_kv_send(fd, key, val, boundary, ssl);
        if (err) return merr_pass(err);

        cnode = mdf_node_next(cnode);
    }

    MSTR str; mstr_init(&str);
    mstr_appendf(&str, "--%s--\r\n", boundary);
    err = _send_to(fd, (unsigned char*)str.buf, str.len, ssl);
    mstr_clear(&str);

    return merr_pass(err);
}

static MERR* _send_payloadvf(int fd, const char *boundary, int variable_count, va_list ap, struct _tls *ssl)
{
    MERR *err;

    for (int i = 0; i < variable_count; i++) {
        char *key = va_arg(ap, char*);
        char *val = va_arg(ap, char*);

        err = _formdata_kv_send(fd, key, val, boundary, ssl);
        if (err) return merr_pass(err);
    }

    MSTR str; mstr_init(&str);
    mstr_appendf(&str, "--%s--\r\n", boundary);
    err = _send_to(fd, (unsigned char*)str.buf, str.len, ssl);
    mstr_clear(&str);

    return merr_pass(err);
}

MERR* mhttp_post_with_file(const char *url, MDF *dnode, MDF *rnode,
                           MHTTP_ONBODY_FUNC body_callback, void *arg)
{
    MERR *err;

    MERR_NOT_NULLC(url, dnode, rnode);

    char boundary[41];
    memset(boundary, '-', sizeof(boundary));
    mstr_rand_hexstring(boundary+24, 16);
    boundary[40] = '\0';

    mdf_clear(rnode);
    mdf_set_pointer(rnode, "_tmp.callback_arg", arg);

    char *host, *scriptname, *requesturi;
    int port, hostlen, scriptlen;
    struct _tls *ssl = NULL;
    bool secure;

    if (!mhttp_parse_url(url, &secure, &host, &hostlen, &port, &scriptname, &scriptlen, &requesturi))
        return merr_raise(MERR_ASSERT, "not http url %s", url);

    if (secure) {
        char hostname[hostlen + 1];
        memset(hostname, 0x0, sizeof(hostname));
        strncpy(hostname, host, hostlen);
        err = _tls_new(&ssl, hostname, MHTTP_TIMEOUT * 1000);
        if (err) return merr_pass(err);
    }

    int fd = 0;
    MSTR str; mstr_init(&str);
    mstr_appendf(&str,
                 "POST /%s HTTP/1.1\r\n"
                 "HOST: %.*s\r\n"
                 "User-Agent: reef\r\n"
                 "Accept: */*\r\n"
                 "Content-Length: %zd\r\n"
                 //"Expect: 100-continue\r\n"
                 "Content-Type: multipart/form-data; boundary=%s\r\n\r\n",
                 requesturi, hostlen, host, _payload_length_mdf(dnode, strlen(boundary)), boundary);

#define RETURN(ret)                             \
    do {                                        \
        if (secure) _tls_free(ssl);             \
        if (fd > 0) close(fd);                  \
        mstr_clear(&str);                       \
        return (ret);                           \
    } while (0)

    err = _connect_to(host, hostlen, port, &fd, ssl);
    if (err) RETURN(merr_pass(err));

    err = _send_to(fd, (unsigned char*)str.buf, str.len, ssl);
    if (err) RETURN(merr_pass(err));

    //rv = recv(fd, buf, MHTTP_BUFLEN, MSG_NOSIGNAL);

    err = _send_payload_mdf(fd, boundary, dnode, ssl);
    if (err) RETURN(merr_pass(err));

    unsigned char buf[MHTTP_BUFLEN];
    memset(buf, 0x0, MHTTP_BUFLEN);
    int rv = 0, len = 0;
    bool end = false;
    while ((rv = _recv_from(fd, buf + len, MHTTP_BUFLEN - len, ssl)) > 0) {
        mtc_mt_noise("received %d bytes", rv);
        //MSG_DUMP("receive:", buf + len, rv);

        buf[len + rv] = '\0';
        err = _parse_response(buf, len + rv, &len, &end, rnode, body_callback);
        if (err) RETURN(merr_pass(err));

        if (end) RETURN(MERR_OK);

        memset(buf + len, 0x0, MHTTP_BUFLEN - len);
    }

    if (rv <= 0) RETURN(merr_raise(MERR_ASSERT, "receive failure"));

    mtc_mt_warn("never reach here");
    RETURN(MERR_OK);

#undef RETURN
}

MERR* mhttp_post_with_filef(const char *url, MDF *rnode, MHTTP_ONBODY_FUNC body_callback, void *arg,
                            int variable_count, ...)
{
    MERR *err;

    MERR_NOT_NULLB(url, rnode);

    if (variable_count <= 0) return merr_raise(MERR_ASSERT, "variable count %d error", variable_count);

    char boundary[41];
    memset(boundary, '-', sizeof(boundary));
    mstr_rand_hexstring(boundary+24, 16);
    boundary[40] = '\0';

    mdf_clear(rnode);
    mdf_set_pointer(rnode, "_tmp.callback_arg", arg);

    char *host, *scriptname, *requesturi;
    int port, hostlen, scriptlen;
    struct _tls *ssl = NULL;
    bool secure;

    if (!mhttp_parse_url(url, &secure, &host, &hostlen, &port, &scriptname, &scriptlen, &requesturi))
        return merr_raise(MERR_ASSERT, "not http url %s", url);

    va_list apa, apb;
    va_start(apa, variable_count);
    va_copy(apb, apa);

    if (secure) {
        char hostname[hostlen + 1];
        memset(hostname, 0x0, sizeof(hostname));
        strncpy(hostname, host, hostlen);
        err = _tls_new(&ssl, hostname, MHTTP_TIMEOUT * 1000);
        if (err) return merr_pass(err);
    }

    int fd = 0;
    MSTR str; mstr_init(&str);
    mstr_appendf(&str,
                 "POST /%s HTTP/1.1\r\n"
                 "HOST: %.*s\r\n"
                 "User-Agent: reef\r\n"
                 "Accept: */*\r\n"
                 "Content-Length: %zd\r\n"
                 //"Expect: 100-continue\r\n"
                 "Content-Type: multipart/form-data; boundary=%s\r\n\r\n",
                 requesturi, hostlen, host,
                 _payload_lengthvf(variable_count, apa, strlen(boundary)),
                 boundary);

#define RETURN(ret)                             \
    do {                                        \
        va_end(apa);                            \
        if (secure) _tls_free(ssl);             \
        if (fd > 0) close(fd);                  \
        mstr_clear(&str);                       \
        return (ret);                           \
    } while (0)

    err = _connect_to(host, hostlen, port, &fd, ssl);
    if (err) RETURN(merr_pass(err));

    err = _send_to(fd, (unsigned char*)str.buf, str.len, ssl);
    if (err) RETURN(merr_pass(err));

    //rv = recv(fd, buf, MHTTP_BUFLEN, MSG_NOSIGNAL);

    err = _send_payloadvf(fd, boundary, variable_count, apb, ssl);
    if (err) RETURN(merr_pass(err));

    unsigned char buf[MHTTP_BUFLEN];
    memset(buf, 0x0, MHTTP_BUFLEN);
    int rv = 0, len = 0;
    bool end = false;
    while ((rv = _recv_from(fd, buf + len, MHTTP_BUFLEN - len, ssl)) > 0) {
        mtc_mt_noise("received %d bytes", rv);
        //MSG_DUMP("receive:", buf + len, rv);

        buf[len + rv] = '\0';
        err = _parse_response(buf, len + rv, &len, &end, rnode, body_callback);
        if (err) RETURN(merr_pass(err));

        if (end) RETURN(MERR_OK);

        memset(buf + len, 0x0, MHTTP_BUFLEN - len);
    }

    if (rv <= 0) RETURN(merr_raise(MERR_ASSERT, "receive failure"));

    mtc_mt_warn("never reach here");
    RETURN(MERR_OK);

#undef RETURN
}
