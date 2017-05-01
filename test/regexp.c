#include "reef.h"

void test_basic()
{
    MERR *err;

    MRE *reo = mre_init();
#if 0
    err = mre_compile(reo, "ab|cc|newman|foo|bar|jia|yi|bing");
    TRACE_NOK(err);

    MTEST_ASSERT(mre_match(reo, "a9eicnewmansdfo") == true);

    err = mre_compile(reo, "ab|cca[\\d][\\D][\\s][\\S][\\w][\\W][abcdefg]");
    TRACE_NOK(err);
    mtc_dbg(" ");
    mre_dump(reo);
    MTEST_ASSERT(mre_match(reo, "cca2m (x$f") == true);

    err = mre_compile(reo, "[a][ry][nm][opq][b[e][ao(f][e-h][xj-m][x-zr][mnq-u][q-ux-zab][-ab][ab-][-a-m][a-m-*]");
    TRACE_NOK(err);
    mtc_dbg(" ");
    mre_dump(reo);
    MTEST_ASSERT(mre_match(reo, "armp[(ekzsy-am*") == true);

    err = mre_compile(reo, "[^a][^ry][^nm][^opq][^b[e][^ao(f][^e-h][^xj-m][^x-zr][^mnq-u][^q-ux-zab][^-ab][^ab-][^-a-m][^a-m-*][ab[^][^ab(?=]");
    TRACE_NOK(err);
    mtc_dbg(" ");
    mre_dump(reo);
    MTEST_ASSERT(mre_match(reo, "bxosabcymac+++z^1") == true);

    err = mre_compile(reo, "ab{2,10}$");
    TRACE_NOK(err);
    mtc_dbg(" ");
    mre_dump(reo);
    MTEST_ASSERT(mre_match(reo, "abbbbbb") == true);
#endif

    err = mre_compile(reo, "ab?c?d*e*f*g+h+ij{1}k{2}l{4}m{1,}n{3,}o{1,3}p{1,10}q{1,2}r{0,1}s{0}$");
    TRACE_NOK(err);
    mtc_dbg(" ");
    mre_dump(reo);
    MTEST_ASSERT(mre_match(reo, "acefffffghhhhijkkllllmnnnnnnoppppppppppqqr") == true);

    mre_destroy(&reo);
}

void suite_basic()
{
    mtest_add_test(test_basic, "basic");
}

int main()
{
    mtc_init("-", MTC_DEBUG);

    mtest_add_suite(suite_basic, "basic");

    return mtest_run();
}
