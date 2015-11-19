#include "rheads.h"

#define MAX_SUITE 100
#define MAX_TEST  10000
#define MAX_NAME  256
#define MAX_ERROR_LEN 10240

#define _color(c) printf("%s", c)

typedef struct {
    void (*func)(void);
    char funcname[MAX_NAME];
    char suitename[MAX_NAME];
} mtest_t;

static int m_num_suite = 0, m_num_test = 0, m_num_assert = 0;
static int m_num_suite_pass = 0, m_num_test_pass = 0, m_num_assert_pass = 0;
static int m_num_suite_fail = 0, m_num_test_fail = 0, m_num_assert_fail = 0;

static mtest_t m_tests[MAX_TEST];
static char *m_suite_name = NULL;
static char m_assert_error[MAX_ERROR_LEN] = {0};

static bool m_test_passed = false;

static void _signal(int sig)
{
    m_test_passed = false;

    _color(MCOLOR_RED);
    switch (sig) {
    case SIGFPE:
        printf("Failed! \n\n    Division by Zero\n\n");
        break;
    case SIGILL:
        printf("Failed! \n\n    Illegal Instruction\n\n");
        break;
    case SIGSEGV:
        printf("Failed! \n\n    Segmetation Fault\n\n");
        break;
    default:
        break;
    }
    _color(MCOLOR_RESET);

    puts("    | Stopping Execution.");
    fflush(stdout);
    exit(1);
}

static void _put_summary()
{
    puts("");
    puts("  +---------------------------------------------------+");
    puts("  |                      Summary                      |");
    puts("  +---------++------------+-------------+-------------+");

    printf("  | Suites  ||");
    _color(MCOLOR_YELLOW);  printf(" Total %4d ",  m_num_suite);
    _color(MCOLOR_RESET); putchar('|');
    _color(MCOLOR_GREEN);   printf(" Passed %4d ", m_num_suite_pass);
    _color(MCOLOR_RESET); putchar('|');
    _color(MCOLOR_RED);     printf(" Failed %4d ", m_num_suite_fail);
    _color(MCOLOR_RESET); puts("|");

    printf("  | Tests   ||");
    _color(MCOLOR_YELLOW);  printf(" Total %4d ",  m_num_test);
    _color(MCOLOR_RESET); putchar('|');
    _color(MCOLOR_GREEN);   printf(" Passed %4d ", m_num_test_pass);
    _color(MCOLOR_RESET); putchar('|');
    _color(MCOLOR_RED);     printf(" Failed %4d ", m_num_test_fail);
    _color(MCOLOR_RESET); puts("|");

    printf("  | Asserts ||");
    _color(MCOLOR_YELLOW);  printf(" Total %4d ",  m_num_assert);
    _color(MCOLOR_RESET); putchar('|');
    _color(MCOLOR_GREEN);   printf(" Passed %4d ", m_num_assert_pass);
    _color(MCOLOR_RESET); putchar('|');
    _color(MCOLOR_RED);     printf(" Failed %4d ", m_num_assert_fail);
    _color(MCOLOR_RESET); puts("|");

    puts("  +---------++------------+-------------+-------------+");
    puts("");
}

void mtest_assert_run(int result, const char *expr,
                      const char *func, const char *file, int line)
{
    static int assert_fail_num = 0;

    m_num_assert++;
    m_test_passed = m_test_passed && result;

    if (result) {
        m_num_assert_pass++;
    } else {
        m_num_assert_fail++;
        assert_fail_num++;

        char error[256] = {0};
        snprintf(error, sizeof(error),
                 "        %d. Assert [ %s ] (%s:%d %s())\n",
                 assert_fail_num, expr, file, line, func);
        strcat(m_assert_error, error);
    }
}

void mtest_add_test(void (*func)(void), const char *funcname)
{
    if (m_num_test >= MAX_TEST) {
        printf("ERROR: Exceeded maximum test count of %d\n", MAX_TEST);
        exit(1);
    }

    m_tests[m_num_test].func = func;
    strncpy(m_tests[m_num_test].suitename, m_suite_name, MAX_NAME);
    strncpy(m_tests[m_num_test].funcname, funcname, MAX_NAME);

    m_num_test++;
}

void mtest_add_suite(void (*func)(void), const char *suitename)
{
    if (m_num_suite >= MAX_SUITE) {
        printf("ERROR: Exceeded maximum test suite count of %d\n", MAX_SUITE);
        exit(1);
    }

    m_suite_name = (char*)suitename;

    /*
     * add tests
     */
    func();

    m_num_suite++;
}

int mtest_run()
{
    bool suite_passed;
    clock_t start, end;
    double duration;

    signal(SIGFPE,  _signal);
    signal(SIGILL,  _signal);
    signal(SIGSEGV, _signal);

    puts("");
    puts("    +-----------------------------------------+");
    puts("    | mtest, micro test tool                  |");
    puts("    |                                         |");
    puts("    | stolen from github.com/orangeduck/ptest |");
    puts("    +-----------------------------------------+");

    start = clock();

    suite_passed = true;

    for (int i = 0; i < m_num_test; i++) {
        mtest_t test = m_tests[i];

        suite_passed = true;

        if (i == 0 || strcmp(m_tests[i].suitename, m_tests[i-1].suitename)) {
            printf("\n\n  ====== Suite %s ======\n\n", m_tests[i].suitename);

            m_test_passed = true;

            if (i != 0) {
                if (suite_passed) m_num_suite_pass++;
                else m_num_suite_fail++;
            }
        }

        printf("    | %s ... ", test.funcname);
        fflush(stdout);

        test.func();

        suite_passed = suite_passed && m_test_passed;

        if (m_test_passed) {
            m_num_test_pass++;

            _color(MCOLOR_GREEN);
            puts("Passed!");
            _color(MCOLOR_RESET);
        } else {
            m_num_test_fail++;

            _color(MCOLOR_RED);
            printf("Failed! \n\n%s\n", m_assert_error);
            _color(MCOLOR_RESET);
        }
    }

    if (suite_passed) m_num_suite_pass++;
    else m_num_suite_fail++;

    end = clock();

    _put_summary();

    duration = (double)(end - start) / CLOCKS_PER_SEC;

    printf("    Total Running Time: %0.3fs\n\n", duration);

    if (m_num_suite_fail > 0) return 1;
    else return 0;
}
