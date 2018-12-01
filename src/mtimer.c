#include "reef.h"

#define TIMER_NUM 64

static struct timeval m_tv_s[TIMER_NUM], m_tv_e[TIMER_NUM];
static clock_t m_clock_s[TIMER_NUM], m_clock_e[TIMER_NUM];
static int m_cur_i = 0;

void mtimer_start(void)
{
    if (m_cur_i < TIMER_NUM) {
        gettimeofday(&m_tv_s[m_cur_i], NULL);
        m_clock_s[m_cur_i] = clock();

        m_cur_i++;
    }
}

unsigned long mtimer_stop(const char *fmt, ...)
{
    double duration;
    unsigned long elapsed;
    char msg[1024] = {0};

    if (m_cur_i > 0) {
        m_cur_i--;

        m_clock_e[m_cur_i] = clock();
        gettimeofday(&m_tv_e[m_cur_i], NULL);

        elapsed = (m_tv_e[m_cur_i].tv_sec - m_tv_s[m_cur_i].tv_sec) * 1000000ul +
            (m_tv_e[m_cur_i].tv_usec - m_tv_s[m_cur_i].tv_usec);
        duration = (double)(m_clock_e[m_cur_i] - m_clock_s[m_cur_i]) / CLOCKS_PER_SEC;
    } else return 0;

    if (fmt) {
        va_list ap;
        va_start(ap, fmt);
        vsnprintf(msg, sizeof(msg), fmt, ap);
        va_end(ap);
    }

    mtc_foo("%s : nature time %lu usecs, processor time %0.4fs", msg, elapsed, duration);
    mtc_mt_foo("%s : nature time %lu usecs, processor time %0.4fs", msg, elapsed, duration);

    return elapsed;
}
