
#include "qthread.h"
#include "greatest.h"
#include <assert.h>
#include <sys/time.h>

static double gettime(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec/1.0e6;
}

TEST main_sleep_alone(void)
{
    double now = gettime();
    qthread_usleep(10000);
    double later = gettime();
    // we slept at least as long
    ASSERT(later - now >= (10000/1.0e6));
    // we didn't sleep that much extra
    ASSERT(later - now <= (14000/1.0e6));
    PASS();
}

static void* sleep_thread(void *p)
{
    int sleep_time_us = (int) p;
    double now = gettime();
    qthread_usleep(sleep_time_us);
    double later = gettime();
    double sleep_time_s = sleep_time_us / 1.0e6;
    // we slept at least as long
    assert(later - now >= sleep_time_s);
    // we didn't sleep that much extra
    assert(later - now <= sleep_time_s + (5000/1.0e6));
    return NULL;
}

#define NUM_THREADS 10

static qthread_t threads[NUM_THREADS];

TEST many_sleepers(void)
{
    qthread_attr_t attr;
    qthread_attr_init(&attr);
    size_t i;
    for (i = 0; i < NUM_THREADS; i++)
    {
        qthread_create(&threads[i], &attr, sleep_thread,
                (void *)(i * 10000));

    }
    double start = gettime();
    for (i = 0; i < NUM_THREADS; i++)
    {
        qthread_join(threads[i], NULL);
    }
    double end = gettime();
    // make sure some time actually passed
    ASSERT (end - start > (10000/1.0e6));
    PASS();
}

SUITE(suite)
{
    RUN_TEST(main_sleep_alone);
    RUN_TEST(many_sleepers);
}

GREATEST_MAIN_DEFS();

int main(int argc, char** argv)
{
    GREATEST_MAIN_BEGIN();
    RUN_SUITE(suite);
    GREATEST_MAIN_END();
}

