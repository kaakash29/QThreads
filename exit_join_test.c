
#include "qthread.h"
#include "greatest.h"

void* returns_1234(void* p)
{
    qthread_exit((void*) 1234);
    return NULL;
}

TEST join_a_thread(void)
{
    qthread_t thread;
    qthread_attr_t attr;
    qthread_attr_init(&attr);
    int *i;
    qthread_create(&thread, &attr, returns_1234, NULL);
	
    int status = qthread_join(thread, (void**)&i);
    ASSERT(status == 0);
    PASS();
}

TEST returns_exit_value(void)
{
    qthread_t thread;
    qthread_attr_t attr;
    qthread_attr_init(&attr);
    qthread_create(&thread, &attr, returns_1234, NULL);
    void* p;
    qthread_join(thread, &p);
    ASSERT((int)p == 1234);
    PASS();
}

void* returns_parameter(void *param)
{
    return param;
}

TEST thread_return_is_exit(void)
{
    qthread_t thread;
    qthread_attr_t attr;
    qthread_attr_init(&attr);
    qthread_create(&thread, &attr, returns_parameter, (void*) 45678);
    void* p;
    qthread_join(thread, &p);
    ASSERT((int)p == 45678);
    PASS();
}

TEST join_after_finish(void)
{
    qthread_t thread;
    qthread_attr_t attr;
    qthread_attr_init(&attr);
    qthread_create(&thread, &attr, returns_parameter, (void*) 17);
    void* p;
    qthread_usleep(1000);
    qthread_join(thread, &p);
    ASSERT((int)p == 17);
    PASS();
}

void* mutex_locker(void *mutex)
{
    qthread_mutex_lock(mutex);
    return (void*)1849;
}

void* thread_waiter(void *thread)
{
    void *p;
    qthread_join(thread, &p);
    return p;
}

TEST many_waiters(void)
{
    qthread_mutex_t mutex;
    qthread_mutex_init(&mutex, NULL);

    qthread_mutex_lock(&mutex);

    qthread_t subject_thread;
    qthread_attr_t attr;
    qthread_attr_init(&attr);
    qthread_create(&subject_thread, &attr, mutex_locker, &mutex);

    qthread_t wait_threads[10];
    size_t i;
    for (i = 0; i < 10; i++)
    {
        qthread_create(&wait_threads[i], &attr, thread_waiter, subject_thread);
        qthread_yield();
    }

    qthread_mutex_unlock(&mutex);

    for (i = 0; i < 10; i++)
    {
        void *p;
        qthread_join(wait_threads[i], &p);
        ASSERT((int) p == 1849);
    }
    PASS();
}

TEST join_detached(void)
{
    qthread_t thread;
    qthread_attr_t attr;
    qthread_attr_init(&attr);
    qthread_attr_setdetachstate(&attr, true);
    qthread_create(&thread, &attr, returns_1234, NULL);
    void *p = NULL;
    int res = qthread_join(thread, &p);
    ASSERT(res == -1);
    ASSERT(p == NULL);
    PASS();
}

TEST join_out_of_order(void)
{
    qthread_t thread1, thread2, thread3;
    qthread_create(&thread1, NULL, returns_parameter, (void*) 4);
    qthread_create(&thread2, NULL, returns_parameter, (void*) 5);
    qthread_create(&thread3, NULL, returns_parameter, (void*) 6);
    qthread_usleep(1000);
    void* p;
    qthread_join(thread1, &p);
    ASSERT((int) p == 4);
    qthread_join(thread3, &p);
    ASSERT((int) p == 6);
    qthread_join(thread2, &p);
    ASSERT((int) p == 5);
    PASS();
}

SUITE(suite)
{
	
    RUN_TEST(join_a_thread);
    RUN_TEST(returns_exit_value);
    RUN_TEST(thread_return_is_exit);
    RUN_TEST(join_after_finish);
    //RUN_TEST(many_waiters);
    RUN_TEST(join_detached);
    RUN_TEST(join_out_of_order);
}

GREATEST_MAIN_DEFS();

int main(int argc, char** argv)
{
    GREATEST_MAIN_BEGIN();
    RUN_SUITE(suite);
    GREATEST_MAIN_END();
}

