#include "reef.h"
#include "dlfcn.h"

static void* _call(void *arg)
{
    int cnum = *(int*)arg;


    MDF *onode;
    mdf_init(&onode);
    mdf_set_int_value(onode, "current_thread", cnum);
    char strbuf[1024];
    size_t len = mdf_json_export_buffer(onode, strbuf, 1024);


    struct v7 *jsm = v7_create();

    enum v7_err jserr;
    v7_val_t res;

    jserr = v7_exec_file(jsm, "./testo.js", &res);
    if (jserr != V7_OK) {
        //printf("%s\n", v7_get_parser_error(jsm));
	v7_print_error(stderr, jsm, "Evaluation error", res);
    }
    v7_println(jsm, res);

    v7_val_t n1 = v7_mk_number(jsm, (double)cnum);
    v7_val_t s1 = v7_mk_string(jsm, "hello, string from c", ~0, 0);
    v7_val_t argo = v7_mk_object(jsm);
    v7_set(jsm, argo, "aa", 2, s1);
    v7_set(jsm, argo, "bb", ~0, n1);

    v7_val_t arga = v7_mk_array(jsm);
    v7_array_push(jsm, arga, v7_mk_number(jsm, 1));
    v7_array_push(jsm, arga, v7_mk_number(jsm, 2));

    v7_val_t global = v7_get_global(jsm);
    v7_val_t sm = v7_get(jsm, global, "SpireMonitor", ~0);
    v7_val_t f1 = v7_get(jsm, sm, "dumpObject", ~0);

    for (int i = 0; i < 10; i++) {
        v7_val_t args = v7_mk_array(jsm);
        v7_val_t s2 = v7_mk_string(jsm, strbuf, len, 0);
        v7_array_push(jsm, args, arga);
        v7_array_push(jsm, args, s2);

        jserr = v7_apply(jsm, f1, sm, args, &res);
        if (jserr != V7_OK) {
            printf("%s\n", v7_get_parser_error(jsm));
        }
        v7_println(jsm, res);
    }

    v7_destroy(jsm);

    return NULL;
}

void basic()
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
