static void _makesure_file(struct rfc2388 *r)
{
    if (!r->filename || r->fp) return;

    strcpy(r->fname, "/var/tmp/mcgi_upload.XXXXXX");
    int fd = mkstemp(r->fname);
    if (fd < 0) {
        mtc_warn("unable to create file %s", r->fname);
        return;
    }

    r->fp = fdopen(fd, "w+");
    if (!r->fp) mtc_warn("unable to open file %s", r->fname);
}

static bool _rfc2388_is_boundary(struct rfc2388 *r, int len, const char *boundary, int bl, bool *end)
{
    char *s = (char*)r->pos;

    *end = false;

    if (s[len] != '\n') return false;

    if (s[len - 1] == '\r') len--;

    if (bl + 2 == len && s[0] == '-' && s[1] == '-' && !strncmp(s + 2, boundary, bl)) return true;
    if (bl + 4 == len && s[0] == '-' && s[1] == '-' && !strncmp(s + 2, boundary, bl) &&
        s[len-1] == '-' && s[len-2] == '-') {
        *end = true;
        return true;
    }

    return false;
}

static bool _rfc2388_is_content_dispotion(struct rfc2388 *r, int len)
{
    char *key = "content-disposition";
    int keylen = 19;

    if (r->pos[len] != '\n') return false;

    if (len > keylen && !strncasecmp((char*)r->pos, key, keylen)) return true;
    else return false;
}

static bool _rfc2388_is_content_type(struct rfc2388 *r, int len)
{
    char *key = "content-type";
    int keylen = 12;

    if (r->pos[len] != '\n') return false;

    if (len > keylen && !strncasecmp((char*)r->pos, key, keylen)) return true;
    else return false;
}

static bool _rfc2388_is_content_encoding(struct rfc2388 *r, int len)
{
    char *key = "content-encoding";
    int keylen = 16;

    if (r->pos[len] != '\n') return false;

    if (len > keylen && !strncasecmp((char*)r->pos, key, keylen)) return true;
    else return false;
}

static bool _rfc2388_is_empty_line(struct rfc2388 *r, int len)
{
    if (len > 2) return false;

    if (r->pos[len] != '\n') return false;

    return true;
}

/* 返回写入字节数 */
static int _rfc2388_fill(struct rfc2388 *r, int len, const char *boundary, int boundarylen)
{
#define FILL_VALUE(buf, filllen)                            \
    do {                                                    \
        if (r->filename) {                                  \
            _makesure_file(r);                              \
            fseek(r->fp, 0, SEEK_END);                      \
            fwrite((buf), 1, (filllen), r->fp);             \
        } else mstr_appendn(r->value, (buf), (filllen));    \
    } while (0)

    if (!r->name) {
        mtc_warn("can't fill to empty variable");
        return 0;
    }

    if (len == r->remain) {
        /* 只是个没有换行的断包 */
        FILL_VALUE(r->pos, len);
        return len;
    } else {
        /* 看看换行后面是不是 boundary */
        int rlen = len;
        if (rlen > 0 && r->pos[len - 1] == '\r') rlen--;

        FILL_VALUE(r->pos, rlen);

        if (r->remain < boundarylen) {
            /* 暂时还不能确定 */
            return rlen;
        } else {
            struct rfc2388 lr;
            lr.pos = r->pos;
            lr.pos += len + 1;
            char *p = memchr(lr.pos, '\n', r->remain - len - 1);
            bool end = false;
            if (!p || !_rfc2388_is_boundary(&lr, p - lr.pos, boundary, boundarylen, &end)) {
                /* 此 [\r]\n 必须写入 */
                if (rlen < len) FILL_VALUE("\r\n", 2);
                else FILL_VALUE("\n", 1);
            }
            return len + 1;
        }
    }
}

void _rfc2388_store(MCGI *ses)
{
    struct rfc2388 *r = ses->r;
    MDF *node = ses->data;

    if (r->name) {
        bool isarray = false;
        size_t len = strlen(r->name);
        if (len >= 3 && r->name[len - 1] == ']' && r->name[len - 2] == '[') {
            r->name[len - 2] = '\0';
            isarray = true;
        }

        if (r->fp) {
            MCGI_UPLOAD_FUNC callback;
            MLIST_ITERATE(ses->upcallbacks, callback) {
                char *savename = callback(r->name, r->filename, r->fname, r->fp);
                if (savename) {
                    if (isarray) {
                        MDF *qnode = mdf_get_or_create_node(node, "UPLOAD");
                        mdf_set_valuef(qnode, "%s.%d=%s", r->name, mdf_child_count(qnode, r->name), savename);
                        mdf_object_2_array(qnode, r->name);
                    } else mdf_set_valuef(node, "UPLOAD.%s=%s", r->name, savename);
                }
            }

            MCGI_UPFILE *ufp = mos_calloc(1, sizeof(MCGI_UPFILE));
            ufp->name = strdup(r->name);
            ufp->filename = r->filename ? strdup(r->filename) : NULL;
            ufp->fname = strdup(r->fname);
            ufp->fp = r->fp;
            mlist_append(ses->files, ufp);

            /* can't link a unlinked file, so, unlink last */
            unlink(r->fname);
        } else {
            if (isarray) {
                MDF *qnode = mdf_get_node(node, "QUERY");
                mdf_set_valuef(qnode, "%s.%d=%s", r->name, mdf_child_count(qnode, r->name), r->value->buf);
                mdf_object_2_array(qnode, r->name);
            } else mdf_set_valuef(node, "QUERY.%s=%s", r->name, r->value->buf);
        }
    }

    mos_free(r->name);
    mos_free(r->filename);
    memset(r->fname, 0x0, sizeof(r->fname));
    mos_free(r->type);
    mstr_clear(r->value);
    r->fp = NULL;
}

int _rfc2388_line(MCGI *ses, const char *boundary, int boundarylen)
{
    struct rfc2388 *r = ses->r;

    int len = 0;
    char *p = memchr(r->pos, '\n', r->remain);
    if (p) {
        len = p - r->pos;
        //mtc_noise("rfc2388 line %.*s", len, r->pos);
        mtc_noise("rfc2388 line %d bytes", len);
    } else {
        /* 没有换行 */
        if (!r->headering) {
            /* 填充变量过程中 */
            if (r->remain >= boundarylen + 2) {
                /* 长度有boundary+\r\n，那肯定是变量内容，直接写入，然后接着读入数据 */
                _rfc2388_fill(r, r->remain, boundary, boundarylen);
                return r->remain;
            } else {
                /* 很短的数据，可能是部分boundary，接着读数据 */
                return 0;
            }
        } else {
            /* 填充头部信息时都没找到换行，只能接着读数据了 */
            return 0;
        }
    }

    bool end = false;
    if (_rfc2388_is_boundary(r, len, boundary, boundarylen, &end)) {
        r->headering = true;

        _rfc2388_store(ses);

        if (end) return r->remain;  /* 最后一个boundary */
        else return len + 1;
    } else if (r->headering && _rfc2388_is_content_dispotion(r, len)) {
        TURNCATE_LINE(r->pos, len);

        char *p = strchr(r->pos, ':');
        if (p && *(++p)) {
            MLIST *alist;
            char *item;
            mstr_array_split(&alist, p, ";", MAX_TOKEN);
            MLIST_ITERATE(alist, item) {
                item = mstr_strip_space(item);
                if (!strncmp(item, "name", 4)) HEADER_ATTR(item, r->name);
                else if (!strncmp(item, "filename", 8)) HEADER_ATTR(item, r->filename);
            }
            mlist_destroy(&alist);
        }

        return len + 1;
    } else if (r->headering && _rfc2388_is_content_type(r, len)) {
        TURNCATE_LINE(r->pos, len);

        HEADER_VALUE(r->pos, r->type);

        return len + 1;
    } else if (r->headering && _rfc2388_is_content_encoding(r, len)) {
        TURNCATE_LINE(r->pos, len);

        char *code;
        HEADER_VALUE(r->pos, code);
        if (!code || (strcmp(code, "7bit") && strcmp(code, "8bit") && strcmp(code, "binary"))) {
            mtc_warn("form data encoding %s not supported", code);
            mos_free(code);
        }

        return len + 1;
    } else if (r->headering && _rfc2388_is_empty_line(r, len)) {
        r->headering = false;

        return len + 1;
    } else {
        /* 有换行，写入该行 */
        return _rfc2388_fill(r, len, boundary, boundarylen);
    }
}
