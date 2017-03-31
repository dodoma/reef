#include "reef.h"
#include "dlfcn.h"


void basic()
{
    struct mjs *jsm = mjs_create();

    mjs_init_local(jsm, mjs_get_global(jsm));
    mjs_set_ffi_resolver(jsm, dlsym);

    mjs_err_t jserr;
    mjs_val_t res;

    jserr = mjs_exec_file(jsm, "./testp.js", &res);
    if (jserr != MJS_OK) {
        printf("%s\n", mjs_strerror(jsm, jserr));
    }

    mjs_println(jsm, res);
}

static void* _call(void *arg)
{
    int cnum = *(int*)arg;


    MDF *onode;
    mdf_init(&onode);
    mdf_set_int_value(onode, "current_thread", cnum);
    char strbuf[1024];
    size_t len = mdf_json_export_buffer(onode, strbuf, 1024);


    struct mjs *jsm = mjs_create();

    mjs_init_local(jsm, mjs_get_global(jsm));
    mjs_set_ffi_resolver(jsm, dlsym);

    mjs_err_t jserr;
    mjs_val_t res;

    jserr = mjs_exec_file(jsm, "./testo.js", &res);
    if (jserr != MJS_OK) {
        printf("%s\n", mjs_strerror(jsm, jserr));
    }

    mjs_println(jsm, res);

    mjs_val_t n1 = mjs_mk_number(jsm, (double)cnum);
    mjs_val_t s1 = mjs_mk_string(jsm, "hello, string from c", ~0, 0);
    mjs_val_t argo = mjs_mk_object(jsm);
    mjs_set(jsm, argo, "aa", 2, s1);
    mjs_set(jsm, argo, "bb", ~0, n1);

    mjs_val_t arga = mjs_mk_array(jsm);
    mjs_array_push(jsm, arga, mjs_mk_number(jsm, 1));
    mjs_array_push(jsm, arga, mjs_mk_number(jsm, 2));

    mjs_val_t global = mjs_get_global(jsm);
    mjs_val_t sm = mjs_get(jsm, global, "SpireMonitor", ~0);
    mjs_val_t f1 = mjs_get(jsm, sm, "dumpObject", ~0);

    for (int i = 0; i < 10; i++) {

        mjs_val_t s2 = mjs_mk_string(jsm, strbuf, len, 0);
        jserr = mjs_call(jsm, &res, f1, sm, 2, arga, s2);
        if (jserr != MJS_OK) {
            printf("%s\n", mjs_strerror(jsm, jserr));
        }
        //mjs_println(jsm, res);
    }

    mjs_destroy(jsm);

    return NULL;
}

void test_mdf()
{
    pthread_t *threads[10];
    int num[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    for (int i = 0; i < 1; i++) {
        threads[i] = mos_calloc(1, sizeof(pthread_t));
    }

    for (int i = 0; i < 1; i++) {
        pthread_create(threads[i], NULL, _call, (void*)&num[i]);
    }

    for (int i = 0; i < 1; i++) {
        pthread_join(*threads[i], NULL);
    }
}

void suite_basic()
{
    //mtest_add_test(test_mdf, "global mdf object");
    mtest_add_test(basic, "basic");
}

int main()
{
    mtc_init("-", MTC_DEBUG);

    mtest_add_suite(suite_basic, "basic");

    return mtest_run();
}
