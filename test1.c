/*
 * file:        test1.c
 * description: test file for qthreads (homework 1)
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include "qthread.h"

/* 1. create and join. Create N threads, which don't do anything
 * except return a value. (and possibly print something) Call
 * qthread_join() to wait for each of them.
 */
void *f1(void *arg) { return arg; }
void test1(void)
{
    qthread_t t[10];
    int i, j;
    for (i = 0; i < 10; i++)
        qthread_create(&t[i], NULL, f1, (void*)i);
    for (i = 0; i < 10; i++) {
        qthread_join(t[i], (void**)&j);
        assert(i == j);
    }
    printf("test 1 OK\n");
}

/* 2. mutex and sleep.
 * initialize a mutex
 * Create thread 1, which locks a mutex and goes to sleep for a
 * second or two using qthread_usleep.
 *   (you can wait for this by calling qthread_yield repeatedly,
 *    waiting for thread 1 to set a global variable)
 * threads 2..N try to lock the mutex
 * after thread 1 wakes up it releases the mutex; 
 * threads 2..N each release the mutex after getting it.
 * run with N=2,3,4
 */
int t1rdy;
qthread_mutex_t m;
void *f2(void *v)
{
    qthread_mutex_lock(&m);
    t1rdy = 1;
    qthread_usleep(1000000);
    qthread_mutex_unlock(&m);

    return 0;
}

void *f3(void *v)
{
    qthread_mutex_lock(&m);
    printf("f3\n");
    qthread_mutex_unlock(&m);
    return 0;
}
    
void test2(void)
{
    qthread_t t0, t[10];
    int i;
    
    qthread_mutex_init(&m, NULL);
    qthread_create(&t0, NULL, f2, NULL);
    while (!t1rdy)
        qthread_yield();
    for (i = 0; i < 4; i++)
        qthread_create(&t[i], NULL, f3, NULL);

    void *val;
    qthread_join(t0, &val);
    for (i = 0; i < 4; i++)
        qthread_join(t[i], &val);
    
    printf("test 2 done\n");
}

int main(int argc, char **argv)
{
    /* Here are some suggested tests to implement. You can either run
     * them one after another in the program (cleaning up threads
     * after each test) or pass a command-line argument indicating
     * which test to run.
     * This may not be enough tests to fully debug your assignment,
     * but it's a good start.
     */

    test1();
    test2();
    
    /* 3. condvar and sleep.
     * initialize a condvar and a mutex
     * create N threads, each of which locks the mutex, increments a
     *   global count, and waits on the condvar; after the wait it
     *   decrements the count and releases the mutex.
     * call qthread_yield until count indicates all threads are waiting
     * call qthread_signal, qthread_yield until count indicates a
     *   thread has left. repeat.
     */

    /* 4. read/write
     * create a pipe ('int fd[2]; pipe(fd);')
     * create 2 threads:
     * one sleeps and then writes to the pipe
     * one reads from the pipe. [this tests blocking read]
     */
}
