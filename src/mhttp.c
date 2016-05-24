#include "reef.h"

char* mhttp_url_unescape(char *s, int buflen, char esc_char)
{
    int i = 0, o = 0;

    if (s == NULL) return s;

    while (i < buflen) {
        if (s[i] == esc_char && (i+2 < buflen) && isxdigit(s[i+1]) && isxdigit(s[i+2])) {
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
