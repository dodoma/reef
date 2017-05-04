#include "reef.h"

static void _dump_res(MRE *reo)
{
    for (int i = 0; i < mre_sub_count(reo); i ++) {
        const char *sp, *ep;

        if (mre_sub_get(reo, i, &sp, &ep)) {
            printf("%d match: %.*s\n", i, (int)(ep - sp), sp);
        } else {
            printf("sub %d don't match\n", i);
        }
    }
}

void test_basic()
{
    MERR *err;

    MRE *reo = mre_init();

#if 1
    err = mre_compile(reo, "ab|cc|newman|foo|bar|jia|yi|bing");
    //mtc_dbg(" ");
    //mre_dump(reo);
    TRACE_NOK(err);
    MTEST_ASSERT(mre_match(reo, "a9eicnewmansdfo", false) == true);

    err = mre_compile(reo, "ab|cca[\\d][\\D][\\s][\\S][\\w][\\W][abcdefg]");
    TRACE_NOK(err);
    //mtc_dbg(" ");
    //mre_dump(reo);
    MTEST_ASSERT(mre_match(reo, "cca2m (x$f", false) == true);

    err = mre_compile(reo, "[a][ry][nm][opq][b[e][ao(f][e-h][xj-m][x-zr][mnq-u][q-ux-zab][-ab][ab-][-a-m][a-m-*]");
    TRACE_NOK(err);
    //mtc_dbg(" ");
    //mre_dump(reo);
    MTEST_ASSERT(mre_match(reo, "armp[(ekzsy-am*", false) == true);

    err = mre_compile(reo, "[^a][^ry][^nm][^opq][^b[e][^ao(f][^e-h][^xj-m][^x-zr][^mnq-u][^q-ux-zab][^-ab][^ab-][^-a-m][^a-m-*][ab[^][^ab(?=]");
    TRACE_NOK(err);
    //mtc_dbg(" ");
    //mre_dump(reo);
    MTEST_ASSERT(mre_match(reo, "bxosabcymac+++z^1", false) == true);

    err = mre_compile(reo, "ab{2,10}$");
    TRACE_NOK(err);
    //mtc_dbg(" ");
    //mre_dump(reo);
    MTEST_ASSERT(mre_match(reo, "abbbbbb", false) == true);

    err = mre_compile(reo, "ab?c?d*e*f*g+h+ij{1}k{2}l{4}m{1,}n{3,}o{1,3}p{1,10}q{1,2}r{0,1}s{0}$");
    TRACE_NOK(err);
    //mtc_dbg(" ");
    //mre_dump(reo);
    MTEST_ASSERT(mre_match(reo, "acefffffghhhhijkkllllmnnnnnnoppppppppppqqr", false) == true);

    err = mre_compile(reo, "<div>.*</div>");
    TRACE_NOK(err);
    //mtc_dbg(" ");
    //mre_dump(reo);
    MTEST_ASSERT(mre_match(reo, "aa<div>test1</div>bb<div>test2</div>cc", false) == true);

    err = mre_compile(reo, "^(ab|cd) newman$");
    TRACE_NOK(err);
    //mtc_dbg(" ");
    //mre_dump(reo);
    MTEST_ASSERT(mre_match(reo, "cd newman", false) == true);

    err = mre_compile(reo, "(ab|cd)+$");
    TRACE_NOK(err);
    //mtc_dbg(" ");
    //mre_dump(reo);
    MTEST_ASSERT(mre_match(reo, "abababab", false) == true);

    err = mre_compile(reo, "foo(?=bad|good)");
    TRACE_NOK(err);
    //mtc_dbg(" ");
    //mre_dump(reo);
    MTEST_ASSERT(mre_match(reo, "foogood", false) == true);

    err = mre_compile(reo, "foo(?!bad|good)");
    TRACE_NOK(err);
    //mtc_dbg(" ");
    //mre_dump(reo);
    MTEST_ASSERT(mre_match(reo, "foofine", false) == true);

    err = mre_compile(reo, "foo(bad|good) (?:\\|) (yellow|red) \\1 \\2");
    TRACE_NOK(err);
    //mtc_dbg(" ");
    //mre_dump(reo);
    MTEST_ASSERT(mre_match(reo, "foogood | red good red", false) == true);

    err = mre_compile(reo, "foo(?=bad|good)");
    TRACE_NOK(err);
    //mtc_dbg(" ");
    //mre_dump(reo);
    MTEST_ASSERT(mre_match(reo, "fooBaD", true) == true);

    err = mre_compile(reo, "foo(?=bad|good).* ^newline");
    TRACE_NOK(err);
    //mtc_dbg(" ");
    //mre_dump(reo);
    MTEST_ASSERT(mre_match(reo, "fooBaD \nnewline", true) == true);

    err = mre_compile(reo, "foo(good)(fine) (goo(gle)) \\1 \\2 \\3 \\4");
    TRACE_NOK(err);
    //mtc_dbg(" ");
    //mre_dump(reo);
    MTEST_ASSERT(mre_match(reo, "womane shif dvfoogoodfine google good fine google gle sdoeoX", true) == true);
    //_dump_res(reo);

    err = mre_compile(reo, "\\bis\\b.*is\\b");
    TRACE_NOK(err);
    //mtc_dbg(" ");
    //mre_dump(reo);
    MTEST_ASSERT(mre_match(reo, "he is his history", false) == true);

    err = mre_compile(reo, "\\Bis\\B.*");
    TRACE_NOK(err);
    //mtc_dbg(" ");
    //mre_dump(reo);
    MTEST_ASSERT(mre_match(reo, "he is his history", false) == true);
#endif


    mre_destroy(&reo);
}

void test_example()
{
    MERR *err;

    MRE *reo = mre_init();

#if 1
    err = mre_compile(reo, "\\b([a-z]+) \\1\\b");
    //mtc_dbg(" ");
    //mre_dump(reo);
    TRACE_NOK(err);
    MTEST_ASSERT(mre_match(reo, "when i believe believe  you are are my sunsine.", true) == true);
    //_dump_res(reo);

    err = mre_compile(reo, "(\\w+)://([^/:]+)(:\\d*)?([^# ]*)");
    //mtc_dbg(" ");
    //mre_dump(reo);
    TRACE_NOK(err);
    MTEST_ASSERT(mre_match(reo, "http://www.google.com:8080/unexit", true) == true);
    //_dump_res(reo);

    err = mre_compile(reo, "^(?:Chapter|Section) [1-9][0-9]{0,1}$");
    //mtc_dbg(" ");
    //mre_dump(reo);
    TRACE_NOK(err);
    MTEST_ASSERT(mre_match(reo, "chapter 1", true) == true);
    //_dump_res(reo);

    err = mre_compile(reo, "Windows(?=95|98|NT).* power");
    //mtc_dbg(" ");
    //mre_dump(reo);
    TRACE_NOK(err);
    MTEST_ASSERT(mre_match(reo, "Windows98 power", false) == true);
    //_dump_res(reo);

    err = mre_compile(reo, "^\\s*$");
    //mtc_dbg(" ");
    //mre_dump(reo);
    TRACE_NOK(err);
    MTEST_ASSERT(mre_match(reo, "   \t", false) == true);
    //_dump_res(reo);

    err = mre_compile(reo, "\\d{2}-\\d{5}");
    //mtc_dbg(" ");
    //mre_dump(reo);
    TRACE_NOK(err);
    MTEST_ASSERT(mre_match(reo, "My Id: 39-50598", false) == true);
    //_dump_res(reo);

    /*
     * greedy match
     */
    //err = mre_compile(reo, "<\\s*(\\S+)(\\s[^>]*)?>([\\s\\S]*)<\\s*/\\1\\s*>");
    /*
     * non-greddy match
     */
    err = mre_compile(reo, "<\\s*(\\S+)(\\s[^>]*)?>([\\s\\S]*?)<\\s*/\\1\\s*>");
    //mtc_dbg(" ");
    //mre_dump(reo);
    TRACE_NOK(err);
    MTEST_ASSERT(mre_match(reo, "aa<div>test1</div>bb<div>test2</div>cc", false) == true);
    _dump_res(reo);
#endif

    mre_destroy(&reo);
}

void suite_basic()
{
    mtest_add_test(test_basic, "basic");
    mtest_add_test(test_example, "example");
}

int main()
{
    mtc_init("-", MTC_DEBUG);

    mtest_add_suite(suite_basic, "basic");

    return mtest_run();
}
