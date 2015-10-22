
#include "qthread.h"
#include "greatest.h"
#include <pthread.h>

TEST lock_unlock(void)
{
    qthread_mutex_t mutex;
    qthread_mutex_init(&mutex, NULL);
    qthread_mutex_lock(&mutex);
    qthread_mutex_unlock(&mutex);
    PASS();
}

qthread_mutex_t count_mutex;
int count;
#define COUNT_TO 10000

void* worker(void *p)
{
    size_t i;
    for (i = 0; i < COUNT_TO; i++)
    {
        qthread_mutex_lock(&count_mutex);
        qthread_yield();
        count++;
        qthread_mutex_unlock(&count_mutex);
        qthread_yield();
    }
    return NULL;
}

TEST contended_count(void)
{
    qthread_mutex_init(&count_mutex, NULL);
#define NUM_WORKERS 19
    qthread_t workers[NUM_WORKERS];
    size_t i;
    for (i = 0; i < NUM_WORKERS; i++)
    {
        qthread_create(&workers[i], NULL, worker, NULL);
    }
    void *p;
    for (i = 0; i < NUM_WORKERS; i++)
    {
        qthread_join(workers[i], &p);
    }
    ASSERT(count == NUM_WORKERS * COUNT_TO);
    PASS();
}


TEST init_destroy(void)
{
    qthread_mutex_t mutex;
    qthread_mutex_init(&mutex, NULL);
    qthread_mutex_destroy(&mutex);
    PASS();
}

TEST destroy_locked(void)
{
    qthread_mutex_t mutex;
    qthread_mutex_init(&mutex, NULL);
    qthread_mutex_lock(&mutex);
    qthread_mutex_destroy(&mutex);
    PASS();
}

void* waiter(void *mutex)
{
    qthread_mutex_lock(mutex);
}

TEST destroy_with_waiting(void)
{
    qthread_mutex_t mutex;
    qthread_mutex_init(&mutex, NULL);
    qthread_mutex_lock(&mutex);

    qthread_t wait_thread;
    qthread_attr_t attr;
    qthread_attr_init(&attr);
    qthread_create(&wait_thread, &attr, waiter, &mutex);

    qthread_yield();
    qthread_mutex_destroy(&mutex);
    PASS();
}

SUITE(suite)
{
    RUN_TEST(lock_unlock);
    //RUN_TEST(contended_count);
    RUN_TEST(init_destroy);
    RUN_TEST(destroy_locked);
    RUN_TEST(destroy_with_waiting);
}

GREATEST_MAIN_DEFS();

int main(int argc, char** argv)
{
    GREATEST_MAIN_BEGIN();
    RUN_SUITE(suite);
    GREATEST_MAIN_END();
}

