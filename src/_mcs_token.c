#define _COMMAND_VAR      MSTR_CONSTANT('v', 'a', 'r', ' ', ' ', ' ', ' ', ' ')
#define _COMMAND_NAME     MSTR_CONSTANT('n', 'a', 'm', 'e', ' ', ' ', ' ', ' ')
#define _COMMAND_INCLUDE  MSTR_CONSTANT('i', 'n', 'c', 'l', 'u', 'd', 'e', ' ')
#define _COMMAND_LINCLUDE MSTR_CONSTANT('l', 'i', 'n', 'c', 'l', 'u', 'd', 'e')
#define _COMMAND_SET      MSTR_CONSTANT('s', 'e', 't', ' ', ' ', ' ', ' ', ' ')
#define _COMMAND_IF       MSTR_CONSTANT('i', 'f', ' ', ' ', ' ', ' ', ' ', ' ')
#define _COMMAND_ELSE     MSTR_CONSTANT('e', 'l', 's', 'e', ' ', ' ', ' ', ' ')
#define _COMMAND_ELIF     MSTR_CONSTANT('e', 'l', 'i', 'f', ' ', ' ', ' ', ' ')
#define _COMMAND_ALT      MSTR_CONSTANT('a', 'l', 't', ' ', ' ', ' ', ' ', ' ')
#define _COMMAND_EACH     MSTR_CONSTANT('e', 'a', 'c', 'h', ' ', ' ', ' ', ' ')
#define _COMMAND_DEF      MSTR_CONSTANT('d', 'e', 'f', ' ', ' ', ' ', ' ', ' ')
#define _COMMAND_CALL     MSTR_CONSTANT('c', 'a', 'l', 'l', ' ', ' ', ' ', ' ')

#define _COMMAND_IF_CLOSE   MSTR_CONSTANT('/', 'i', 'f', ' ', ' ', ' ', ' ', ' ')
#define _COMMAND_ALT_CLOSE  MSTR_CONSTANT('/', 'a', 'l', 't', ' ', ' ', ' ', ' ')
#define _COMMAND_EACH_CLOSE MSTR_CONSTANT('/', 'e', 'a', 'c', 'h', ' ', ' ', ' ')
#define _COMMAND_DEF_CLOSE  MSTR_CONSTANT('/', 'd', 'e', 'f', ' ', ' ', ' ', ' ')

/*
 * <?cs var  :   Page.num ?>
 * <?cs else?>
 * INPUT
 *     *sme->pos = 'v'
 * OUTPUT
 *     *bgn = 'v'
 *     *end = ' ' (or ':' if no space)
 *     *sme->pos = 'P'
 */
#define _EXPECT(sme, bgn, end, c)                                       \
    do {                                                                \
        bgn = sme->pos;                                                 \
        end = NULL;                                                     \
        while (*sme->pos && *sme->pos != (c)) {                         \
            if (isspace(*sme->pos)) {                                   \
                if (!end) end = sme->pos;                               \
                if (*sme->pos == '\n') sme->lineno++;                   \
            }                                                           \
            sme->pos++;                                                 \
        }                                                               \
        if (sme->pos == bgn) DIE(sme, "miss command");                  \
        if (*sme->pos != (c)) DIE(sme, "unexpect command terminate");   \
        if (!end) end = sme->pos;                                       \
        /* skip 'c' */                                                  \
        sme->pos += 1;                                                  \
        /* skip space */                                                \
        while (isspace(*sme->pos)) {                                    \
            if (*sme->pos == '\n') sme->lineno++;                       \
            sme->pos++;                                                 \
        }                                                               \
    } while(0)

/*
 * x <?cs var  :   Page.num ?> y
 * INPUT
 *     *sme->pos = '<'
 * OUTPUT
 *     *sme->pos = 'v'
 */
#define _EXPECT_CSBEGIN(sme)                                            \
    do {                                                                \
        if (sme->pos[0] != '<' || sme->pos[1] != '?' || sme->pos[2] != 'c' || sme->pos[3] != 's' || \
            !isspace(sme->pos[4])) DIE(sme, "cs begin expect");         \
        sme->pos += 4;                                                  \
        while (isspace(*sme->pos)) {                                    \
            if (*sme->pos == '\n') sme->lineno++;                       \
            sme->pos++;                                                 \
        }                                                               \
    } while (0)

/*
 * x <?cs var  :   Page.num ?> y
 * INPUT
 *     *sme->pos = 'P'
 * OUTPUT
 *     *bgn = 'P'
 *     end = ' ?>...' (or '?' if no space)
 *     *sme->pos = 'y'
 *
 * x <?cs each:item = Page.Menu  ?> y
 * INPUT
 *     *sme->pos = 'i'
 * OUTPUT
 *     *bgn = 'i'
 *     end =  '  ?>...' (or '?' if no space)
 *     *sme->pos = 'y'
 */
#define _EXPECT_CSEND(sme, bgn, end)                                    \
    do {                                                                \
        bgn = sme->pos;                                                 \
        end = NULL;                                                     \
        while (*sme->pos && (sme->pos[0] != '?' || sme->pos[1] != '>')) { \
            if (isspace(*sme->pos)) {                                   \
                end = sme->pos;                                         \
                if (*sme->pos == '\n') sme->lineno++;                   \
            }                                                           \
            sme->pos++;                                                 \
        }                                                               \
        if (sme->pos == bgn) DIE(sme, "miss parameter");                \
        if (sme->pos[0] != '?' || sme->pos[1] != '>') DIE(sme, "miss close"); \
        if (end && isspace(*end)) {                                     \
            /* end point to last space, move it to these space beginning */ \
            while (isspace(*end)) end--;                                \
            end++;                                                      \
        } else end = sme->pos;                                          \
        /* skip '?>' */                                                 \
        sme->pos += 2;                                                  \
        /* don't skip space  */                                         \
    } while (0)

enum {
    TOK_EOF,
    TOK_COMMENT,
    TOK_LITERAL,
    TOK_VAR,
    TOK_NAME,
    TOK_INCLUDE,
    TOK_EACH,
    TOK_EACH_CLOSE,
    TOK_IF,
    TOK_ELSE,
    TOK_ELIF,
    TOK_IF_CLOSE,
};

static inline bool _tok_cs(MCS *sme)
{
    if (sme->pos[0] == '<' && sme->pos[1] == '?' &&
        sme->pos[2] == 'c' && sme->pos[3] == 's' && isspace(sme->pos[4]))
        return true;
    return false;
}

static uint8_t _tok_next(MCS *sme, struct token *tok)
{
    const char *pbgn, *pend;
    size_t len;
    uint64_t command;

    tok->type = TOK_EOF;
    tok->s = NULL;
    tok->len = 0;

    if (*sme->pos == '\0') return tok->type;

    if (_tok_cs(sme)) {
        _EXPECT_CSBEGIN(sme);

        if (*sme->pos == '/' || !memcmp(sme->pos, "else", 4)) {
            /* command close, or else */
            _EXPECT_CSEND(sme, pbgn, pend);
        } else {
            /* command with : parameters */
            _EXPECT(sme, pbgn, pend, ':');
        }

        len = pend - pbgn;
        if (len <= 8) {
            char s[8];
            memset(s, ' ', sizeof(s));
            for (int i = 0; i < len; i++) {
                s[i] = pbgn[i];
            }

            command = MSTR_CONSTANT(s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7]);
            switch (command) {
            case _COMMAND_VAR:
                _EXPECT_CSEND(sme, pbgn, pend);
                tok->type = TOK_VAR;
                tok->s = pbgn;
                tok->len = pend - pbgn;
                break;
            case _COMMAND_NAME:
                _EXPECT_CSEND(sme, pbgn, pend);
                tok->type = TOK_NAME;
                tok->s = pbgn;
                tok->len = pend - pbgn;
                break;
            case _COMMAND_INCLUDE:
                _EXPECT_CSEND(sme, pbgn, pend);
                tok->type = TOK_INCLUDE;
                tok->s = pbgn;
                tok->len = pend - pbgn;
                break;
            case _COMMAND_EACH:
                _EXPECT_CSEND(sme, pbgn, pend);
                tok->type = TOK_EACH;
                tok->s = pbgn;
                tok->len = pend - pbgn;
                break;
            case _COMMAND_EACH_CLOSE:
                tok->type = TOK_EACH_CLOSE;
                break;
            case _COMMAND_IF:
                _EXPECT_CSEND(sme, pbgn, pend);
                tok->type = TOK_IF;
                tok->s = pbgn;
                tok->len = pend - pbgn;
                break;
            case _COMMAND_ELSE:
                tok->type = TOK_ELSE;
                break;
            case _COMMAND_ELIF:
                _EXPECT_CSEND(sme, pbgn, pend);
                tok->type = TOK_ELIF;
                tok->s = pbgn;
                tok->len = pend - pbgn;
                break;
            case _COMMAND_IF_CLOSE:
                tok->type = TOK_IF_CLOSE;
                break;
            default:
                DIE(sme, "unknown command");
            }
        } else {
            if (*sme->pos != '#') DIE(sme, "unknown command");
            else {
                /* comment command */
                _EXPECT_CSEND(sme, pbgn, pend);

                tok->type = TOK_COMMENT;
                tok->s = pbgn;
                tok->len = pend - pbgn;
            }
        }
    } else {
        tok->type = TOK_LITERAL;
        tok->s = sme->pos;
        while (!_tok_cs(sme)) {
            switch (*sme->pos) {
            case '\n':
                sme->lineno++;
                break;
            case '\0':
                return tok->type;
            default:
                break;
            }

            sme->pos++;
            tok->len++;
        }
    }

    return tok->type;
}
