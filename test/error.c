#include "reef.h"

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

    MSTR astr;
    mstr_init(&astr);
    merr_traceback(oerr, &astr);
    //printf("%s\n", astr.buf);
    MTEST_ASSERT(strstr(astr.buf, "i am error") != NULL);
    mstr_clear(&astr);

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
