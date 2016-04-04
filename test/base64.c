#include "reef.h"

void test_code()
{
    int inlen;
    unsigned char ina[256], inb[256];
    char outa[1024];
    int rv;

    inlen = 256;

    memset(outa, 0x0, sizeof(outa));
    for (int i = 0; i < inlen; i++) {
        ina[i] = i;
        inb[i] = 0;
    }

    rv = mb64_encode(ina, inlen, outa, 1024);
    rv = mb64_decode(outa, rv, inb, inlen);
    for (int i = 0; i < inlen; i++) {
        MTEST_ASSERT(ina[i] == inb[i]);
    }

    //mtc_dbg("%s", outa);
}

void suite_basic()
{
    mtest_add_test(test_code, "test basic code");
}

int main()
{
    mtc_init("-", MTC_DEBUG);

    mtest_add_suite(suite_basic, "basic");

    return mtest_run();
}
