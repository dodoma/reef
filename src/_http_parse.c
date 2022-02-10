static int _parse_header_line(char *buf, size_t len, bool *headerend, MDF *node)
{
    char *nl = strstr(buf, "\r\n");
    if (!nl) return -1;

    int rowlen = nl - buf;
    rowlen += 2;
    if (rowlen == 2) {
        /* 头部尾行 */
        *headerend = true;
        return rowlen;
    }

    char *pos = buf;
    char *header = "HTTP/1.1 ";
    int headerlen = strlen(header);
    if (!strncmp(pos, header, headerlen)) {
        /* 头部首行 */
        pos += headerlen;
        int code = 0;
        while (*pos && isspace(*pos)) pos++;
        while (*pos && *pos >= '0' && *pos <= '9') {
            code = code * 10 + (*pos - '0');
            pos++;
        }
        mdf_set_int_value(node, "HEADER.code", code);

        while (*pos && isspace(*pos)) pos++;
        mdf_set_valuef(node, "HEADER.status=%.*s", (int)(nl - pos), pos);
    } else {
        /* 头部 key: value 行 */
        char *p = strchr(pos, ':');
        if (p > pos && (nl - p) > 1) {
            char *q = p;
            q++;
            while (*q && isspace(*q)) q++;
            if (nl > q) mdf_set_valuef(node, "HEADER.%.*s=%.*s", (int)(p - pos), pos, (int)(nl - q), q);
        }
    }

    return rowlen;
}

static int _parse_chunklenline(char *buf, size_t len, int *chunklen)
{
    if (len < 2) return -1;

    char *nl = strstr(buf, "\r\n");
    if (!nl || nl - buf > len - 2) return -1;

    *chunklen = strtol(buf, NULL, 16);

    mtc_mt_noise("new chunk %d", *chunklen);

    return nl - buf + 2;
}

static int _chunkend(char *buf, size_t len)
{
    int rowlen = 0;

    while (rowlen < len && buf[rowlen] == '\r' && buf[rowlen+1] == '\n') rowlen += 2;

    return rowlen;
}

static MERR* _process_chunk(MDF *node, char *pos, size_t remain, bool *end, MHTTP_ONBODY_FUNC body_callback)
{
    int need_to_get = 0, rowlen = 0;
    void *arg = mdf_get_pointer(node, "_tmp.callback_arg");

    while (remain > 0) {
        need_to_get = mdf_get_int_value(node, "_tmp.ntg", 0);
        if (need_to_get <= 0) {
            rowlen = _parse_chunklenline(pos, remain, &need_to_get);
            if (rowlen <= 0) return merr_raise(MERR_ASSERT, "not chunklen line %s", pos);

            pos += rowlen;
            remain -= rowlen;
        }

        if (need_to_get == 0) {
            if (remain > 0) {
                rowlen = _chunkend(pos, remain);
                pos += rowlen;
                remain -= rowlen;
                if (remain > 0) {
                    MSG_DUMP("remain:", pos, remain);
                    return merr_raise(MERR_ASSERT, "unbelieveable, remain %zu", remain);
                }
            }
            *end = true;
            return MERR_OK;
        }

        if (need_to_get >= remain) {
            need_to_get -= remain;
            mdf_set_int_value(node, "_tmp.ntg", need_to_get);
            if (body_callback) body_callback((unsigned char*)pos, remain, "chunked", arg);
            return MERR_OK;
        } else {
            /* 收到了另外一个 chunk 的数据 */
            mdf_set_int_value(node, "_tmp.ntg", 0);
            if (body_callback) body_callback((unsigned char*)pos, need_to_get, "chunked", arg);
            pos += need_to_get;
            remain -= need_to_get;

            rowlen = _chunkend(pos, remain);
            pos += rowlen;
            remain -= rowlen;
        }
    }

    return MERR_OK;
}

static MERR* _process_content(MDF *node, char *pos, size_t remain, bool *end, MHTTP_ONBODY_FUNC body_callback)
{
    char *content_type = mdf_get_value(node, "HEADER.Content-Type", "application/json");
    size_t content_length = mdf_get_int64_value(node, "HEADER.Content-Length", 0);

    int need_to_get = mdf_get_int_value(node, "_tmp.ntg", 0);
    if (need_to_get <= 0) need_to_get = content_length;
    if (need_to_get < 0) return merr_raise(MERR_ASSERT, "content length %d", need_to_get);

    if (remain > need_to_get) return merr_raise(MERR_ASSERT, "unbelieveable, remain %zu", remain);

    if (need_to_get == 0) {
        *end = true;
        return MERR_OK;
    }

    void *arg = mdf_get_pointer(node, "_tmp.callback_arg");
    if (body_callback) body_callback((unsigned char*)pos, remain, content_type, arg);

    if (need_to_get > remain) {
        need_to_get -= remain;
        mdf_set_int_value(node, "_tmp.ntg", need_to_get);
        return MERR_OK;
    } else {
        *end = true;
        return MERR_OK;
    }
}

static MERR* _parse_response(unsigned char *buf, size_t len, int *rlen, bool *end, MDF *node,
                             MHTTP_ONBODY_FUNC body_callback)
{
    int rowlen = 0;
    char *pos = (char*)buf;

    *end = false;
    *rlen = len;

    /*
     * HEADER
     */
    bool headerend = mdf_get_bool_value(node, "_tmp.headerend", false);
    if (!headerend) {
        /* 检查头几个字符 */
        const char *header = "HTTP/1.1 ";
        int headerlen = strlen(header);
        if (len < 12) return MERR_OK;
        else if (strncmp(pos, header, headerlen)) return merr_raise(MERR_ASSERT, "unexpect header %s", pos);
    }

    size_t remain = len;
    while (!headerend && remain > 0) {
        rowlen = _parse_header_line(pos, remain, &headerend, node);
        if (rowlen <= 0) {
            /* 头部信息不全（找不到换行），保留当前数据，继续收包 */
            return MERR_OK;
        }

        pos += rowlen;
        remain -= rowlen;
    }

    if (!headerend) {
        /* 头部信息不全（找不到headerend），保留当前数据，继续收包 */
        return MERR_OK;
    }

    mdf_set_bool_value(node, "_tmp.headerend", true);

    /*
     * BODY
     */
    int code = mdf_get_int_value(node, "HEADER.code", 0);
    if (code >= 100 && code < 200) {
        *end = true;
        return MERR_OK;
    }

    /* 处理到了body部分，不会再有东西剩下 */
    *rlen = 0;

    /* Transfer-Encoding */
    char *t_encoding = mdf_get_value(node, "HEADER.Transfer-Encoding", NULL);
    if (t_encoding && !strcasecmp(t_encoding, "chunked")) {
        /* chunked */
        return merr_pass(_process_chunk(node, pos, remain, end, body_callback));
    } else {
        /* content-length */
        mdf_set_type(node, "HEADER.Content-Length", MDF_TYPE_INT);
        char *content_type = mdf_get_value(node, "HEADER.Content-Type", "application/json");
        size_t content_length = mdf_get_int64_value(node, "HEADER.Content-Length", 0);
        if (content_length <= 0)
            return merr_raise(MERR_ASSERT, "content length %zu", content_length);

        if (!strncmp(content_type, "application/json", 16) ||
            !strncmp(content_type, "application/gzip", 16) ||
            !strncmp(content_type, "text/html", 9) ||
            !strncmp(content_type, "text/xml", 8) ||
            !strncmp(content_type, "text/json", 9) ||
            !strncmp(content_type, "text/plain", 10) ||
            !strncmp(content_type, "image/", 6) ||
            !strncmp(content_type, "audio/", 6)) {
            return merr_pass(_process_content(node, pos, remain, end, body_callback));
        }

        return merr_raise(MERR_ASSERT, "content_type %s not support", content_type);
    }
}
