/*
 * A_LOOP: SPACE \t
 * A_NEWLINE: \n
 * A_BARE: _, [a-zA-Z]
 * A_NUMBARE: -, [0-9]
 * A_QUOTE: " '
 * A_OBJ, A_UNOBJ, A_ARRAY, A_UNARRAY, A_PAIR, A_VALUE
 */
static const int8_t goconstruct[256] = {
    /* ascii 0 */
    A_BAD,
    /* ascii 1 ~ 10 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_LOOP, A_NEWLINE,
    /* ascii 11 ~ 20 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* ascii 21 ~ 30 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* ascii 31 ~ 40 */
    A_BAD, A_LOOP, A_BAD, A_QUOTE, A_BAD, A_BAD, A_BAD, A_BAD, A_QUOTE, A_BAD,
    /* ascii 41 ~ 50 */
    A_BAD, A_BAD, A_BAD, A_PAIR, A_NUMBARE, A_BAD, A_BAD, A_NUMBARE, A_NUMBARE, A_NUMBARE,
    /* ascii 51 ~ 60 */
    A_NUMBARE, A_NUMBARE, A_NUMBARE, A_NUMBARE, A_NUMBARE, A_NUMBARE,
    A_NUMBARE, A_VALUE, A_BAD, A_BAD,
    /* ascii 61 ~ 70 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BARE, A_BARE, A_BARE, A_BARE, A_BARE, A_BARE,
    /* ascii 71 ~ 80 */
    A_BARE, A_BARE, A_BARE, A_BARE, A_BARE, A_BARE, A_BARE, A_BARE, A_BARE, A_BARE,
    /* ascii 81 ~ 90 */
    A_BARE, A_BARE, A_BARE, A_BARE, A_BARE, A_BARE, A_BARE, A_BARE, A_BARE, A_BARE,
    /* ascii 91 ~ 100 */
    A_ARRAY, A_BAD, A_UNARRAY, A_BAD, A_BARE, A_BAD, A_BARE, A_BARE, A_BARE, A_BARE,

    /* 101 ~ 110 */
    A_BARE, A_BARE, A_BARE, A_BARE, A_BARE, A_BARE, A_BARE, A_BARE, A_BARE, A_BARE,
    /* 111 ~ 120 */
    A_BARE, A_BARE, A_BARE, A_BARE, A_BARE, A_BARE, A_BARE, A_BARE, A_BARE, A_BARE,
    /* 121 ~ 130 */
    A_BARE, A_BARE, A_OBJ, A_BAD, A_UNOBJ, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* 131 ~ 140 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* 141 ~ 150 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* 151 ~ 160 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* 161 ~ 170 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* 171 ~ 180 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* 181 ~ 190 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* 191 ~ 200 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,

    /* 201 ~ 210 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* 211 ~ 220 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* 221 ~ 230 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* 231 ~ 240 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* 241 ~ 250 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* 251 ~ 255 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD
};

/*
 * A_UNBARE: SPACE \t
 * A_NEWLINE: \n
 * A_LOOP: [_-.], [0-9], [a-zA-Z]
 * A_OBJ, A_UNOBJ, A_ARRAY, A_UNARRAY, A_PAIR, A_VALUE
 */
static const int8_t gobare[256] = {
    /* ascii 0 */
    A_BAD,
    /* ascii 1 ~ 10 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_UNBARE, A_NEWLINE,
    /* ascii 11 ~ 20 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* ascii 21 ~ 30 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* ascii 31 ~ 40 */
    A_BAD, A_UNBARE, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* ascii 41 ~ 50 */
    A_BAD, A_BAD, A_BAD, A_PAIR, A_LOOP, A_LOOP, A_BAD, A_LOOP, A_LOOP, A_LOOP,
    /* ascii 51 ~ 60 */
    A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_VALUE, A_BAD, A_BAD,
    /* ascii 61 ~ 70 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP,
    /* ascii 71 ~ 80 */
    A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP,
    /* ascii 81 ~ 90 */
    A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP,
    /* ascii 91 ~ 100 */
    A_ARRAY, A_BAD, A_UNARRAY, A_BAD, A_LOOP, A_BAD, A_LOOP, A_LOOP, A_LOOP, A_LOOP,

    /* 101 ~ 110 */
    A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP,
    /* 111 ~ 120 */
    A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP,
    /* 121 ~ 130 */
    A_LOOP, A_LOOP, A_OBJ, A_BAD, A_UNOBJ, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* 131 ~ 140 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* 141 ~ 150 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* 151 ~ 160 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* 161 ~ 170 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* 171 ~ 180 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* 181 ~ 190 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* 191 ~ 200 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,

    /* 201 ~ 210 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* 211 ~ 220 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* 221 ~ 230 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* 231 ~ 240 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* 241 ~ 250 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* 251 ~ 255 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD
};

/*
 * A_UNQUOTE: " '
 * A_ESCAPE: \
 * A_LOOP: [32 ... 126]
 * A_UTF8_2: [192 ... 223]
 * A_UTF8_3: [224 ... 239]
 * A_UTF8_4: [240 ... 247]
 */
static const int8_t gostring[256] = {
    /* ascii 0 */
    A_BAD,
    /* ascii 1 ~ 10 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* ascii 11 ~ 20 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* ascii 21 ~ 30 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* ascii 31 ~ 40 */
    A_BAD, A_LOOP, A_LOOP, A_UNQUOTE, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_UNQUOTE, A_LOOP,
    /* ascii 41 ~ 50 */
    A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP,
    /* ascii 51 ~ 60 */
    A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP,
    /* ascii 61 ~ 70 */
    A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP,
    /* ascii 71 ~ 80 */
    A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP,
    /* ascii 81 ~ 90 */
    A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP,
    /* ascii 91 ~ 100 */
    A_LOOP, A_ESCAPE, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP,

    /* 101 ~ 110 */
    A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP,
    /* 111 ~ 120 */
    A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP,
    /* 121 ~ 130 */
    A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_LOOP, A_BAD, A_BAD, A_BAD, A_BAD,
    /* 131 ~ 140 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* 141 ~ 150 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* 151 ~ 160 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* 161 ~ 170 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* 171 ~ 180 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* 181 ~ 190 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* 191 ~ 200 */
    A_BAD, A_UTF8_2, A_UTF8_2, A_UTF8_2, A_UTF8_2, A_UTF8_2,
    A_UTF8_2, A_UTF8_2, A_UTF8_2, A_UTF8_2,

    /* 201 ~ 210 */
    A_UTF8_2, A_UTF8_2, A_UTF8_2, A_UTF8_2, A_UTF8_2, A_UTF8_2,
    A_UTF8_2, A_UTF8_2, A_UTF8_2, A_UTF8_2,
    /* 211 ~ 220 */
    A_UTF8_2, A_UTF8_2, A_UTF8_2, A_UTF8_2, A_UTF8_2, A_UTF8_2,
    A_UTF8_2, A_UTF8_2, A_UTF8_2, A_UTF8_2,
    /* 221 ~ 230 */
    A_UTF8_2, A_UTF8_2, A_UTF8_2, A_UTF8_3, A_UTF8_3, A_UTF8_3,
    A_UTF8_3, A_UTF8_3, A_UTF8_3, A_UTF8_3,
    /* 231 ~ 240 */
    A_UTF8_3, A_UTF8_3, A_UTF8_3, A_UTF8_3, A_UTF8_3, A_UTF8_3,
    A_UTF8_3, A_UTF8_3, A_UTF8_3, A_UTF8_4,
    /* 241 ~ 250 */
    A_UTF8_4, A_UTF8_4, A_UTF8_4, A_UTF8_4, A_UTF8_4, A_UTF8_4,
    A_UTF8_4, A_BAD, A_BAD, A_BAD,
    /* 251 ~ 255 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD
};

/*
 * A_UNESCAPE: ", ', \, /, b, f, n, r, t, u,
 */
static const int8_t goescape[256] = {
    /* ascii 0 */
    A_BAD,
    /* ascii 1 ~ 10 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* ascii 11 ~ 20 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* ascii 21 ~ 30 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* ascii 31 ~ 40 */
    A_BAD, A_BAD, A_BAD, A_UNESCAPE, A_BAD, A_BAD, A_BAD, A_BAD, A_UNESCAPE, A_BAD,
    /* ascii 41 ~ 50 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_UNESCAPE, A_BAD, A_BAD, A_BAD,
    /* ascii 51 ~ 60 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* ascii 61 ~ 70 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* ascii 71 ~ 80 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* ascii 81 ~ 90 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* ascii 91 ~ 100 */
    A_BAD, A_UNESCAPE, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_UNESCAPE, A_BAD, A_BAD,

    /* 101 ~ 110 */
    A_BAD, A_UNESCAPE, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_UNESCAPE,
    /* 111 ~ 120 */
    A_BAD, A_BAD, A_BAD, A_UNESCAPE, A_BAD, A_UNESCAPE, A_UNESCAPE, A_BAD, A_BAD, A_BAD,
    /* 121 ~ 130 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* 131 ~ 140 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* 141 ~ 150 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* 151 ~ 160 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* 161 ~ 170 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* 171 ~ 180 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* 181 ~ 190 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* 191 ~ 200 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,

    /* 201 ~ 210 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* 211 ~ 220 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* 221 ~ 230 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* 231 ~ 240 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* 241 ~ 250 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* 251 ~ 255 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD
};

/*
 * A_UTF_CONTINUE: [128 ... 191]
 */
static const int8_t goutf8_continue[256] = {
    /* ascii 0 */
    A_BAD,
    /* ascii 1 ~ 10 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* ascii 11 ~ 20 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* ascii 21 ~ 30 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* ascii 31 ~ 40 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* ascii 41 ~ 50 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* ascii 51 ~ 60 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* ascii 61 ~ 70 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* ascii 71 ~ 80 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* ascii 81 ~ 90 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* ascii 91 ~ 100 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,

    /* 101 ~ 110 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* 111 ~ 120 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* 121 ~ 130 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_UTF_CONTINUE,
    A_UTF_CONTINUE, A_UTF_CONTINUE,
    /* 131 ~ 140 */
    A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE,
    A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE,
    /* 141 ~ 150 */
    A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE,
    A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE,
    /* 151 ~ 160 */
    A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE,
    A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE,
    /* 161 ~ 170 */
    A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE,
    A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE,
    /* 171 ~ 180 */
    A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE,
    A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE,
    /* 181 ~ 190 */
    A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE,
    A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE, A_UTF_CONTINUE,
    /* 191 ~ 200 */
    A_UTF_CONTINUE, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,

    /* 201 ~ 210 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* 211 ~ 220 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* 221 ~ 230 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* 231 ~ 240 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* 241 ~ 250 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD, A_BAD,
    /* 251 ~ 255 */
    A_BAD, A_BAD, A_BAD, A_BAD, A_BAD
};
