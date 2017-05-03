enum {
    TOK_EOF,

    TOK_OR,                     /* | */

    TOK_OPEN_PAREN,             /* ( */
    TOK_BOL,                    /* ^ */
    TOK_EOL,                    /* $ */
    TOK_ANY,                    /* . any but \n */
    TOK_CCLASS,                 /* [] chractoer class */
    TOK_NCCLASS,                /* [^] negative character class */
    TOK_REPEAT,                 /* ? * + {m} {m,} {m,n} */
    TOK_NC,                     /* (?: no capture */
    TOK_PLA,                    /* (?= positive lookahead */
    TOK_NLA,                    /* (?! negative lookahead */
    TOK_WORD,                   /* \b */
    TOK_NWORD,                  /* \B */
    TOK_REF,                    /* \1 */

    TOK_R_DICIMAL,               /* \d */
    TOK_R_NDICIMAL,              /* \D */
    TOK_R_WHITESPACE,            /* \s */
    TOK_R_NWHITESPACE,           /* \S */
    TOK_R_WORD,                  /* \w */
    TOK_R_NWORD,                 /* \W */

    TOK_CLOSE_BRACKET,          /* ] */
    TOK_CLOSE_CURLY,            /* } */
    TOK_CLOSE_PAREN,            /* ) */
    TOK_CHAR
};

#define ESCAPES "BbDdSsWw^$\\.*+?()[]{}|"

static bool _tok_long(MRE *reo, char a, char b)
{
    if (reo->tok.c == a && reo->pos[0] == b) {
        reo->pos += 1;
        return true;
    }
    return false;
}

static bool _tok_long3(MRE *reo, char a, char b, char c)
{
    if (reo->tok.c == a && reo->pos[0] == b && reo->pos[1] == c) {
        reo->pos += 2;
        return true;
    }
    return false;
}

static uint8_t _tok_next(MRE *reo, bool restr)
{
    Token *t = &reo->tok;

    t->c = 0;
    t->type = TOK_EOF;

    if (*reo->pos == '\0') return TOK_EOF;

    reo->pos += chartorune(&t->c, (char*)reo->pos);

    if (t->c == '\\') {
        reo->pos += chartorune(&t->c, (char*)reo->pos);
        switch (t->c) {
        case 0: DIE(reo, "unterminate escape sequence");
        case 'f': t->c = '\f'; t->type = TOK_CHAR; return t->type;
        case 'n': t->c = '\n'; t->type = TOK_CHAR; return t->type;
        case 'r': t->c = '\r'; t->type = TOK_CHAR; return t->type;
        case 't': t->c = '\t'; t->type = TOK_CHAR; return t->type;
        case 'v': t->c = '\v'; t->type = TOK_CHAR; return t->type;
        case 'd': t->type = TOK_R_DICIMAL; return t->type;
        case 'D': t->type = TOK_R_NDICIMAL; return t->type;
        case 's': t->type = TOK_R_WHITESPACE; return t->type;
        case 'S': t->type = TOK_R_NWHITESPACE; return t->type;
        case 'w': t->type = TOK_R_WORD; return t->type;
        case 'W': t->type = TOK_R_NWORD; return t->type;
        case 'b': t->c = 'b'; t->type = TOK_WORD; return t->type;
        case 'B': t->c = 'B'; t->type = TOK_NWORD; return t->type;
        }
        if (strchr(ESCAPES, t->c)) {
            t->type = TOK_CHAR;
            return t->type;
        }
        if ((t->c >= 'a' && t->c <= 'z') || (t->c >= 'A' && t->c <= 'Z') || isalpharune(t->c) || t->c == '_') {
            DIE(reo, "invalid escape chracter");
        }
        if (t->c >= '0' && t->c <= '9') {
            t->c -= '0';
            if (*reo->pos >= '0' && *reo->pos <= '9')
                t->c = t->c * 10 + *reo->pos++ - '0';
            t->type = TOK_REF;
            return t->type;
        }
        t->type = TOK_CHAR;
        return t->type;
    }

    if (!restr) {
        /* in non-restrict model(e.g. character class), do following */
        if (_tok_long(reo, '[', '^')) { t->c = '['; t->type = TOK_NCCLASS; return t->type; }
        else if (_tok_long3(reo, '(', '?', '=')) { t->c = '('; t->type = TOK_PLA; return t->type; }
        else if (_tok_long3(reo, '(', '?', '!')) { t->c = '('; t->type = TOK_NLA; return t->type; }
        else if (_tok_long3(reo, '(', '?', ':')) { t->c = '('; t->type = TOK_NC; return t->type; }
    }

    switch (t->c) {
    case 0: t->type = TOK_EOF; return t->type;
    case '(': t->type = TOK_OPEN_PAREN; return t->type;
    case '[': t->type = TOK_CCLASS; return t->type;
    case '{':
    case '?':
    case '*':
    case '+':
        t->type = TOK_REPEAT; return t->type;
    case '^': t->type = TOK_BOL; return t->type;
    case '$': t->type = TOK_EOL; return t->type;
    case '.': t->type = TOK_ANY; return t->type;
    case '}': t->type = TOK_CLOSE_CURLY; return t->type;
    case ']': t->type = TOK_CLOSE_BRACKET; return t->type;
    case ')': t->type = TOK_CLOSE_PAREN; return t->type;
    case '|': t->type = TOK_OR; return t->type;
    }

    t->type = TOK_CHAR;
    return t->type;
}
