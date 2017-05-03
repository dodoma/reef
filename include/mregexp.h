#ifndef __MRE_H__
#define __MRE_H__

/*
 * ^       Match beginning of a buffer
 * $       Match end of a buffer
 * ()      Grouping and substring capturing
 * |       Or
 * .       any charactor except '\n'
 * [...]   Match any character from set. Ranges like [a-z] are supported
 * [^...]  Match any character but ones from set
 * ?       Match zero or once (non-greedy)
 * *       Match zero or more times (greedy)
 * +       Match one or more times (greedy)
 * {m}     Match exactly n times
 * {m,}    Match >= n times
 * {m,n}   Match >= n && <=m times

 * \meta   Match one of the meta character: ^$().[]*+?|\
 *
 * \d      Match decimal digit equal to [0-9]
 * \D      Match non-decimal digit equal to [^0-9]
 * \s      Match whitespace equal to [\f\n\r\t\v]
 * \S      Match non-whitespacep equal to [^\f\n\r\t\v]
 * \w      Match any word charactor, equal to [A-Za-z0-9_]
 * \W      Match any non-word charactor, equal to [^A-Za-z0-9_]
 * \b      Match word terminal
 * \B      Match non-word terminal
 *
 * \n      Match new line character
 * \r      Match line feed character
 * \f      Match form feed character
 * \v      Match vertical tab character
 * \t      Match horizontal tab character
 *
 * \xHH    Match byte with hex value 0xHH, e.g. \x4a
 *
 */
__BEGIN_DECLS

MRE*  mre_init();
MERR* mre_compile(MRE *reo, const char *pattern);
bool  mre_match(MRE *reo, const char *string);
void  mre_destroy(MRE **reo);

/* 获取匹配`(...)`的个数 */
uint32_t mre_sub_count(MRE *reo);
/* 获取第几个匹配, index 传 0 取第一个 match sub */
bool mre_sub_get(MRE *reo, uint32_t index, const char **sp, const char **ep);

__END_DECLS
#endif
