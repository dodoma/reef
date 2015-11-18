#include "rheads.h"

void test_error()
{
    MERR *err, *oerr;

    err = merr_raise(MERR_ASSERT, "i am error");
    MTEST_ASSERT(err != MERR_OK);

    oerr = merr_pass(err);
    MTEST_ASSERT(oerr != MERR_OK);

    MTEST_ASSERT(merr_match(oerr, MERR_ASSERT));
    MTEST_ASSERT(merr_match(oerr, MERR_PASS));
    MTEST_ASSERT(!merr_match(oerr, MERR_NOMEM));

    MSTR str;
    mstr_init(&str);
    merr_traceback(oerr, &str);
    //printf("%s\n", str.buf);
    MTEST_ASSERT(strstr(str.buf, "i am error") != NULL);
    mstr_clear(&str);

    merr_destroy(&oerr);
}

void suite_basic()
{
    mtest_add_test(test_error, "errror");
}

int main()
{
    mtest_add_suite(suite_basic, "basic");

    return mtest_run();
}
