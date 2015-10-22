
#include "qthread.h"
#include "greatest.h"

TEST init_destroy(void)
{
    qthread_cond_t cond;
    qthread_cond_init(&cond, NULL);
    qthread_cond_destroy(&cond);
    PASS();
}

void* signaler(void *cond)
{
    qthread_cond_signal(cond);
    return NULL;
}

TEST signal_wakeup(void)
{
    qthread_cond_t cond;
    qthread_cond_init(&cond, NULL);

    qthread_mutex_t mutex;
    qthread_mutex_init(&mutex, NULL);

    qthread_t signal_thread;
    qthread_attr_t attr;
    qthread_attr_init(&attr);
    qthread_create(&signal_thread, &attr, signaler, &cond);

    qthread_mutex_lock(&mutex);
    qthread_cond_wait(&cond, &mutex);
    PASS();
}

void* waiter(void *cond)
{
    qthread_mutex_t mutex;
    qthread_mutex_init(&mutex, NULL);
    qthread_mutex_lock(&mutex);
    qthread_cond_wait(cond, &mutex);
    return NULL;
}

TEST wakeup_signal(void)
{
    qthread_cond_t cond;
    qthread_cond_init(&cond, NULL);

    qthread_t wait_thread;
    qthread_attr_t attr;
    qthread_attr_init(&attr);
    qthread_create(&wait_thread, &attr, waiter, &cond);

    qthread_yield();

    qthread_cond_signal(&cond);

    void *p;
    qthread_join(wait_thread, &p);
    PASS();
}

qthread_mutex_t broadcast_mutex;

void* broadcast_waiter(void *cond)
{
    qthread_mutex_lock(&broadcast_mutex);
    qthread_cond_wait(cond, &broadcast_mutex);
    qthread_mutex_unlock(&broadcast_mutex);
    return NULL;
}

TEST broadcast_all(void)
{
    qthread_cond_t cond;
    qthread_cond_init(&cond, NULL);
    qthread_mutex_init(&broadcast_mutex, NULL);

    qthread_t wait_threads[10];
    qthread_attr_t attr;
    qthread_attr_init(&attr);
    size_t i;
    for (i = 0; i < 10; i++)
    {
        qthread_create(&wait_threads[i], &attr, broadcast_waiter, &cond);
        qthread_yield();
    }
    qthread_cond_broadcast(&cond);
    for (i = 0; i < 10; i++)
    {
        void *p;
        qthread_join(wait_threads[i], &p);
    }
    PASS();
}

int signal_counts = 0;
qthread_mutex_t signal_few_mutex;

void* signal_waiter(void *cond)
{
    qthread_mutex_lock(&signal_few_mutex);
    qthread_cond_wait(cond, &signal_few_mutex);
    qthread_mutex_unlock(&signal_few_mutex);
    ++signal_counts;
    return NULL;
}

TEST signal_a_few(void)
{
    qthread_cond_t cond;
    qthread_cond_init(&cond, NULL);
    qthread_mutex_init(&signal_few_mutex, NULL);

    qthread_t wait_threads[10];
    qthread_attr_t attr;
    qthread_attr_init(&attr);
    size_t i;
    for (i = 0; i < 10; i++)
    {
        qthread_create(&wait_threads[i], &attr, signal_waiter, &cond);
        qthread_yield();
    }
    qthread_cond_signal(&cond);
    qthread_cond_signal(&cond);
    qthread_cond_signal(&cond);
    qthread_yield();
    ASSERT(signal_counts == 3);
    PASS();
}

SUITE(suite)
{
    RUN_TEST(init_destroy);
    RUN_TEST(signal_wakeup);
    RUN_TEST(wakeup_signal);
    RUN_TEST(broadcast_all);
    RUN_TEST(signal_a_few);
}

GREATEST_MAIN_DEFS();

int main(int argc, char** argv)
{
    GREATEST_MAIN_BEGIN();
    RUN_SUITE(suite);
    GREATEST_MAIN_END();
}

