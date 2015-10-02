/* 
 * file:        philosopher.c
 * description: Dining philosophers, from CS 5600
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#include "qthread.h"

int nforks;
qthread_mutex_t m;
qthread_cond_t C[10];
int fork_in_use[10];
double t0;

static void init_time(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    t0 = tv.tv_sec + tv.tv_usec/1.0e6;
}

/* timestamp - time since start - NOT adjusted for speedup
 */
double timestamp(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    double t1 = tv.tv_sec + tv.tv_usec/1.0e6;
    return (t1-t0);
}

/* sleep_exp(T) - sleep for exp. dist. time with mean 10T msecs
 *                unlocks mutex while sleeping if provided.
 */
void sleep_exp(double T)
{
    double t = -1 * T * log(drand48()); /* sleep time */
    if (t > T*10)
        t = T*10;
    if (t < 0.5)
        t = 0.5;
    qthread_usleep((int)(10000 * t));
}

/* get_forks() method - called before a philospher starts eating.
 *                      'i' identifies the philosopher 0..N-1
 */
void get_forks(int i)
{
    int left = i, right = (i+1) % nforks;
    qthread_mutex_lock(&m);

    printf("DEBUG: %f philosopher %d tries for left fork\n", timestamp(), i);
    if (fork_in_use[left]) 
        qthread_cond_wait(&C[left], &m);
    printf("DEBUG: %f philosopher %d gets left fork\n", timestamp(), i);
    fork_in_use[left] = 1;
    printf("DEBUG: %f philosopher %d tries for right fork\n", timestamp(), i);
    if (fork_in_use[right])
        qthread_cond_wait(&C[right], &m);
    printf("DEBUG: %f philosopher %d gets left fork\n", timestamp(), i);
    fork_in_use[right] = 1;

    qthread_mutex_unlock(&m);
}

/* release_forks()  - called when a philospher is done eating.
 *                    'i' identifies the philosopher 0..N-1
 */
void release_forks(int i)
{
    int left = i, right = (i+1) % nforks;

    qthread_mutex_lock(&m);

    printf("DEBUG: %f philosopher %d puts down both forks\n", timestamp(), i);
    fork_in_use[left] = 0;
    qthread_cond_signal(&C[left]);
    fork_in_use[right] = 0;
    qthread_cond_signal(&C[right]);

    qthread_mutex_unlock(&m);
}

/* the philosopher thread function - create N threads, each of which calls 
 * this function with its philosopher number 0..N-1
 */
void *philosopher_thread(void *context) 
{
    int philosopher_num = (int)context; /* hack... */

    while (1) {
        sleep_exp(4.0);
        get_forks(philosopher_num);
        sleep_exp(2.5);
        release_forks(philosopher_num);
    }
    
    return 0;
}

/* handler - ^C will break out of usleep() below; set flag to stop
 * loop 
 */
static int done;
static void handler(int sig)
{
    signal(SIGINT, SIG_DFL);
    done = 1;
}

/* wait until ^C
 */
void wait_until_done(void)
{
    while (!done)
        qthread_usleep(100000);
}

int main(int argc, char **argv)
{
    int i;
    qthread_t t;

    signal(SIGINT, handler);
    init_time();
    nforks = 4;

    qthread_mutex_init(&m, NULL);
    for (i = 0; i < nforks; i++)
        qthread_cond_init(&C[i], NULL);

    for (i = 0; i < nforks; i++) 
        qthread_create(&t, NULL, philosopher_thread, (void*)i);

    wait_until_done();

    return 0;
}

