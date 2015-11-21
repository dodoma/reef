#include "reef.h"

void test_basic()
{
    mtimer_start();

    mtimer_stop("empty");

    mtimer_start();

    unsigned int a = 0;
    for (int i = 0; i < 100000000; i++) {
        a += i;
    }

    mtimer_stop("add digits %ul", a);
}

void test_recusive()
{
    mtimer_start();

    unsigned int a = 0;
    for (int i = 0; i < 100000000; i++) {
        a += i;
    }

    mtimer_start();
    unsigned int b = 0;
    for (int i = 0; i < 100000000; i++) {
        b += i;
    }
    mtimer_stop("add b digit %ul", b);

    mtimer_stop("add a, b digit %ul %ul", a, b);
}

void suite_basic()
{
    mtest_add_test(test_basic, "basic");
}

void suite_recusive()
{
    mtest_add_test(test_recusive, "recusive timer");
}

int main()
{
    mtc_init("test.log", MTC_DEBUG);

    mtest_add_suite(suite_basic, "basic");
    mtest_add_suite(suite_recusive, "recusive");

    return mtest_run();
}
