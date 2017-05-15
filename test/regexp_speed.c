#include "reef.h"

#define MEM_LEN 104857600

static void _dump_res(MRE *reo, const char *string)
{
    for (int i = 0; i < mre_match_count(reo); i++) {
        for (int j = 0; j < mre_sub_count(reo, i); j++) {
            const char *sp, *ep;

            if (mre_sub_get(reo, i, j, &sp, &ep)) {
                printf("%d's %d match: %d %.*s\n", i, j, (int)(sp - string), (int)(ep - sp), sp);
            } else {
                printf("%d's sub %d don't match\n", i, j);
            }
        }
    }
}

void test_basic()
{
    FILE *fp = fopen("mtent12.txt", "r");
    if (!fp) {
        printf("open mtent12.txt failure, please get it from http://www.gutenberg.org/files/3200/old/mtent12.zip \n");
        return;
    }
    char *buf = mos_calloc(1, MEM_LEN);
    fread(buf, 1, MEM_LEN, fp);

    int match_num = 0;
    MRE *reo = mre_init();
    MERR *err;

#if 1
    mtimer_start();
    err = mre_compile(reo, "Twain");
    TRACE_NOK(err);
    match_num = mre_match_all(reo, buf, false);
    mtimer_stop("Twain %d matches", match_num);
    //_dump_res(reo, buf);
    //mre_dump(reo,);

    mtimer_start();
    err = mre_compile(reo, "Huck[\\w]+");
    TRACE_NOK(err);
    match_num = mre_match_all(reo, buf, false);
    mtimer_stop("Huck[\\w]+ %d matches", match_num);
    //_dump_res(reo, buf);

    mtimer_start();
    err = mre_compile(reo, "[\\w]+ing");
    TRACE_NOK(err);
    match_num = mre_match_all(reo, buf, false);
    mtimer_stop("[\\w]+ing %d matches", match_num);
    //_dump_res(reo, buf);
#endif

    mtimer_start();
    err = mre_compile(reo, "^[^ ]*?Huck");
    TRACE_NOK(err);
    match_num = mre_match_all(reo, buf, false);
    mtimer_stop("^[^ ]*?Twain %d matches", match_num);
    //_dump_res(reo, buf);
    //mre_dump(reo);

#if 1
    mtimer_start();
    err = mre_compile(reo, "Tom|Sawyer|Huckleberry|Finn");
    TRACE_NOK(err);
    match_num = mre_match_all(reo, buf, false);
    mtimer_stop("Tom|Sawyer|Huckleberry|Finn %d matches", match_num);
    //_dump_res(reo, buf);

    mtimer_start();
    err = mre_compile(reo, "(Tom|Sawyer|Huckleberry|Finn).{0,30}river|river.{0,30}(Tom|Sawyer|Huckleberry|Finn)");
    TRACE_NOK(err);
    match_num = mre_match_all(reo, buf, false);
    mtimer_stop("(Tom|Sawyer|Huckleberry|Finn).{0,30}river|river.{0,30}(Tom|Sawyer|Huckleberry|Finn) %d matches", match_num);
    //_dump_res(reo, buf);
#endif

    mre_destroy(&reo);

    mos_free(buf);
}

void test_html()
{
    FILE *fp = fopen("Boost Libraries - 1.40.0.htm", "r");
    if (!fp) {
        printf("open failure, please get it from http://www.boost.org/doc/libs/1_40_0/libs/libraries.htm \n");
        return;
    }

    char *buf = mos_calloc(1, MEM_LEN);
    fread(buf, 1, MEM_LEN, fp);

    int match_num = 0;
    MRE *reo = mre_init();
    MERR *err;

    mtimer_start();
    err = mre_compile(reo, "beman|john|dave");
    TRACE_NOK(err);
    match_num = mre_match_all(reo, buf, true);
    mtimer_stop("beman|john|dave %d matches", match_num);
    //_dump_res(reo, buf);

    mtimer_start();
    err = mre_compile(reo, "<p>.*?</p>");
    TRACE_NOK(err);
    match_num = mre_match_all(reo, buf, true);
    mtimer_stop("<p>.*?</p> %d matches", match_num);
    //_dump_res(reo, buf);

    mtimer_start();
    err = mre_compile(reo, "<a[^>]+href=(\"[^\"]*\"|[\\S]+)[^>]*>");
    TRACE_NOK(err);
    match_num = mre_match_all(reo, buf, true);
    mtimer_stop("<a> %d matches", match_num);
    //_dump_res(reo, buf);

    mtimer_start();
    err = mre_compile(reo, "<h[12345678][^>]*>.*?</h[12345678]>");
    TRACE_NOK(err);
    match_num = mre_match_all(reo, buf, true);
    mtimer_stop("<h> %d matches", match_num);
    //_dump_res(reo, buf);

    mtimer_start();
    err = mre_compile(reo, "<img[^>]+src=(\"[^\"]*\"|[\\S]+)[^>]*>");
    TRACE_NOK(err);
    match_num = mre_match_all(reo, buf, true);
    mtimer_stop("<img> %d matches", match_num);
    //_dump_res(reo, buf);

    mtimer_start();
    err = mre_compile(reo, "<font[^>]+color=(\"[^\"]*\"|[\\S]+)[^>]*>.*?</font>");
    TRACE_NOK(err);
    match_num = mre_match_all(reo, buf, true);
    mtimer_stop("<font> %d matches", match_num);
    //_dump_res(reo, buf);

    mre_destroy(&reo);
    mos_free(buf);
}

int main()
{
    mtc_init("-", MTC_DEBUG);

    test_basic();
    test_html();

    return 0;
}
