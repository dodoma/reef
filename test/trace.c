#include "reef.h"

static void* _write_log(void *arg)
{
    int cnum = *(int*)arg;

    for (int i = 0; i < 1000000; i++)
        mtc_dbg("i am log from thread %d", cnum);
}

void test_basic()
{
    MTEST_ASSERT(mtc_init("/unexist/x.log", MTC_NOISE) == MERR_OPENFILE);
    MTEST_ASSERT(mtc_init("/usr/x.log", MTC_NOISE) == MERR_OPENFILE);

    MTEST_ASSERT(mtc_dbg("hello %s", "trace") == false);

    MTEST_ASSERT(mtc_init("x.log", MTC_NOISE) == 0);

    MTEST_ASSERT(mtc_dbg("") == true);
    MTEST_ASSERT(mtc_dbg("hello") == true);
    MTEST_ASSERT(mtc_dbg("hello %s", "trace") == true);
    MTEST_ASSERT(mtc_err("hello %s error", "trace") == true);
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
