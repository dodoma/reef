#include "reef.h"

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


#if 0
void mhttp_parse_url()
{
    /*
     * JOIN
     */
    unsigned char cid[16];
    mb64_decode(key, 24, cid, 16);
    mstr_bin2hexstr(cid, 16, client->clientid);

    /* some thing special, on client connected */
    MDF *nodein;
    char *p, *u, *k, *v;

    mdf_init(&nodein);
    mlist_get(alist, 0, (void**)&p);
    u = k = v = NULL;
    while (*p != '\0') {
        if (*p == '/' && !u) {
            u = p + 1;
        } else if (*p == '?' && u) {
            *p = '\0';
            k = p + 1;
            v = NULL;
        } else if (*p == '=' && k && !v) {
            *p = '\0';
            v = p + 1;
        } else if (*p == '&') {
            *p = '\0';
            if (k && v) mdf_set_value(nodein, k, (char*)mhttp_url_unescape(v, strlen(v), '%'));
            k = p + 1;
            v = NULL;
        } else if (*p == ' ') {
            *p = '\0';
            if (u && !mdf_get_value(nodein, "_url", NULL)) mdf_set_value(nodein, "_url", u);
            if (k && v) mdf_set_value(nodein, k, (char*)mhttp_url_unescape(v, strlen(v), '%'));
        }

        p++;
    }

    _broadcast_event(MOC_CMD_CONNECT, client->clientid, client->fd, nodein);
    mdf_destroy(&nodein);
}
#endif
