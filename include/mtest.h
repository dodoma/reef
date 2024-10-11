#ifndef __MTEST_H__
#define __MTEST_H__

/*
 * ***attention*** NOT TRHEAD SAFE
 *
 * mtest, test tools
 * check test/demo.c for usage
 */

__BEGIN_DECLS

#define MTEST_ASSERT(expr) mtest_assert_run((int)(expr),                \
                                            #expr,                      \
                                            __func__, __FILE__, __LINE__)

#define MTEST_ASSERT_STR_EQ(fst, snd)                       \
    mtest_assert_run(strcmp(fst, snd) == 0,                 \
                     "strcmp( " #fst ", " #snd " ) == 0",   \
                     __func__, __FILE__, __LINE__)

#define MTEST_ASSERT_STR_NEQ(fst, snd)                      \
    mtest_assert_run(strcmp(fst, snd) != 0,                 \
                     "strcmp( " #fst ", " #snd " ) != 0",   \
                     __func__, __FILE__, __LINE__)

#define MTEST_ASSERT_FILE_EQ(src, dst)                                  \
    do {                                                                \
        char __command[PATH_MAX];                                       \
        snprintf(__command, sizeof(__command), "cmp %s %s", src, dst);  \
        mtest_assert_run(system(__command) == 0,                        \
                         "system( " #src ", " #dst " ) == 0",           \
                         __func__, __FILE__, __LINE__);                 \
    } while(0)


void mtest_assert_run(int result, const char *expr,
                      const char *func, const char *file, int line);
void mtest_add_test(void (*func)(void), const char *funcname);
void mtest_add_suite(void (*func)(void), const char *suitename);
int  mtest_run(void);

__END_DECLS
#endif
