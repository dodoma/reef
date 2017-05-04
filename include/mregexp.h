#ifndef __MRE_H__
#define __MRE_H__

/*
 * mregexp, 一个试验版的正则表达式解析匹配器(练手为主)，支持以下功能：
 * ^       Match beginning of a buffer
 * $       Match end of a buffer
 * ()      Grouping and substring capturing, include:
 *         (?:)    no capture
 *         (?=)    positive lookahead
 *         (?!)    negative lookahead
 * |       Or
 * .       any charactor except newline
 * [...]   Match any character from set. Ranges like [a-z] are supported
 * [^...]  Match any character but ones from set
 * ?       Match zero or once
 * *       Match zero or more times, *? for non-greedy
 * +       Match one or more times, +? for non-greedy
 * {m}     Match exactly n times
 * {m,}    Match >= n times, {m,}? for non-greedy
 * {m,n}   Match >= n && <=m times, {m,n}? for non-greedy
 *
 * \b      Match word terminal
 * \B      Match non-word terminal
 * \1,2... Back reference
 *
 * \d      Match decimal digit equal to [0-9]
 * \D      Match non-decimal digit equal to [^0-9]
 * \s      Match whitespace equal to [\f\n\r\t\v]
 * \S      Match non-whitespacep equal to [^\f\n\r\t\v]
 * \w      Match any word charactor, equal to [A-Za-z0-9_]
 * \W      Match any non-word charactor, equal to [^A-Za-z0-9_]
 * \n      Match new line character
 * \r      Match line feed character
 * \f      Match form feed character
 * \v      Match vertical tab character
 * \t      Match horizontal tab character
 * \meta   Match one of the meta character: ^$().[]*+?|\
 *
 * many thanks to:
 * 1. mujs
 * 2. mjs
 * 3. clearsilver
 * 4. [正则表达式-教程](http://www.runoob.com/regexp/regexp-tutorial.html)
 */

__BEGIN_DECLS

MRE*  mre_init();
MERR* mre_compile(MRE *reo, const char *pattern);
bool  mre_match(MRE *reo, const char *string, bool igcase);
void  mre_destroy(MRE **reo);

/* 获取匹配`(...)`的个数 */
uint32_t mre_sub_count(MRE *reo);
/* 获取第几个匹配, index 传 1 取第一个 match sub(0 为整个 match sub string) */
bool mre_sub_get(MRE *reo, uint32_t index, const char **sp, const char **ep);

__END_DECLS
#endif
