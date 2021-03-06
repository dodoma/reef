#include "reef.h"

static void* _write_log(void *arg)
{
    int cnum = *(int*)arg;

    char name[64], modulename[64];
    snprintf(name, 64, "./thread_%02d.log", cnum);
    sprintf(modulename, "%05d", cnum);

    mtc_mt_init(name, modulename, MTC_DEBUG);

    for (int i = 0; i < 10000; i++)
        mtc_mt_dbg("i am log from thread %d", cnum);

    return NULL;
}

void test_basic()
{
    MERR *err;
    err = mtc_mt_init("/unexist/x.log", "basic", MTC_NOISE);
    MTEST_ASSERT(merr_match(err, MERR_OPENFILE));
    merr_destroy(&err);

    err = mtc_mt_init("/usr/x.log", "basic", MTC_NOISE);
    MTEST_ASSERT(merr_match(err, MERR_OPENFILE));
    merr_destroy(&err);

    MTEST_ASSERT(mtc_mt_dbg("hello %s", "trace") == false);

    MTEST_ASSERT(mtc_mt_init("x.log", "basic", MTC_NOISE) == MERR_OK);

    MTEST_ASSERT(mtc_mt_dbg(" ") == true);
    MTEST_ASSERT(mtc_mt_dbg("hello") == true);
    MTEST_ASSERT(mtc_mt_dbg("hello %s", "trace") == true);
    MTEST_ASSERT(mtc_mt_err("hello %s error", "trace") == true);
}

void test_mthread()
{
    pthread_t *threads[10];
    int num[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    for (int i = 0; i < 10; i++) {
        threads[i] = mos_calloc(1, sizeof(pthread_t));
    }

    for (int i = 0; i < 10; i++) {
        pthread_create(threads[i], NULL, _write_log, (void*)&num[i]);
    }

    for (int i = 0; i < 10; i++) {
        pthread_join(*threads[i], NULL);
    }
}

void suite_basic()
{
    mtest_add_test(test_basic, "basic");
}

void suite_threadsafe()
{
    mtest_add_test(test_mthread, "multi thread write log");
}

int main()
{
    mtest_add_suite(suite_basic, "basic");
    mtest_add_suite(suite_threadsafe, "thread safe");

    return mtest_run();
}
