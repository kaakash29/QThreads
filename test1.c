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
void *f1(void *arg) { printf("\nHello\n"); //return arg; 
	//qthread_exit(arg); 
	return arg;
}

void test0(void)
{
    qthread_t t;
    int i = 1, j = 1;
    qthread_create(&t, NULL, f1, (void*)i);
    qthread_join(t, (void**)&j);
    assert(i == j);
    printf("\ntest 0 done\n");
}
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
    printf("\ntest 1 done\n");
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
    printf("\nin f2\n");
    qthread_usleep(1000000);
    qthread_mutex_unlock(&m);

    qthread_exit(0); 
    //return 0;
}

void *f3(void *v)
{
    qthread_mutex_lock(&m);
    printf("\nin f3\n");
    qthread_usleep(1000);
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

/* 
 * test3 : int -> void
 * The test function to test functionality of 
 * the list/queue operations.
 */
int test3() {
	queue_t Q = NULL;
	enqueue(&Q, get_new_node(2));
	assert(Q != NULL);
	enqueue(&Q, get_new_node(3));
	enqueue(&Q, get_new_node(4));
	enqueue(&Q, get_new_node(5));
	qthread_t rem_data = NULL;
	rem_data = dequeue(&Q);
	rem_data = dequeue(&Q);
	rem_data = dequeue(&Q);
	rem_data = dequeue(&Q);
	rem_data = dequeue(&Q);
	assert(Q == NULL);
	rem_data = dequeue(&Q);
	rem_data = dequeue(&Q);
	rem_data = dequeue(&Q);
	assert(Q == NULL);
	qthread_t head = NULL;
	assert(head == NULL);
	head = add_thread_to_list(head, get_new_node(5));
	assert(head != NULL);
	head = add_thread_to_list(head, get_new_node(2));
	head = add_thread_to_list(head, get_new_node(4));
	head = add_thread_to_list(head, get_new_node(1));
	head = add_thread_to_list(head, get_new_node(7));
	head = add_thread_to_list(head, get_new_node(7));
	printf("\nTest 3 done\n");
}

/* 
 * test4 : int -> void
 * The test function to test functionality of 
 * the list/queue operations.
 */

int counter = 0;
qthread_mutex_t lock;
int num2 = 0;

void* execute(void* num) {
	printf("\nThe number is: %d", (int) num);
	num2 ++;
	qthread_exit(num);
	//return num;
}

void test4() {
	qthread_t th1 = NULL, th2 = NULL;
	void* arg = (void*) 10;
	qthread_create(&th1, NULL, (execute), arg);
	void* ret = 0;
	
	
	arg = (void*) 40;
	qthread_create(&th2, NULL, (execute), arg);
	printf("\nnum2 before usleep = %d\n", num2);
	qthread_usleep(100);
	printf("\nnum2 after usleep = %d\n", num2);
	qthread_join(th2, &ret);
	printf("\n 1. Now in main : received return value = %d\n", (int) ret);
	qthread_join(th1, &ret);
	printf("\n 2. Now in main : received return value = %d\n", (int) ret);
	printf("\nTest 4 done\n");
}

/* 
 * test5 : int -> void
 * The test function to test functionality of 
 * the list/queue operations.
 */

void* counters_th1() {
	int i;
	qthread_usleep(100);
	qthread_mutex_lock(&lock);
	for (i = 0; i < 200000000; i++) {
		counter ++;
	}
	printf("\nCounter is %d\n", counter);
	qthread_mutex_unlock(&lock);
	return (void*) counter;
}

void* counters_th2() {
	int i;
	qthread_mutex_lock(&lock);
	for (i = 0; i < 200000000; i++) {
		counter ++;
	}
	printf("\nCounter is %d\n", counter);
	qthread_mutex_unlock(&lock);
	return (void*) counter;
}

void test5() {
	qthread_t th1 = NULL, th2 = NULL;
	counter = 0;
	qthread_mutex_init(&lock, NULL);
	qthread_create(&th1, NULL, counters_th1, NULL);
	qthread_create(&th2, NULL, counters_th2, NULL);
	
	void* retval;
	qthread_join(th1, &retval);
	qthread_join(th2, &retval);
	printf("\nTest 5 done\n");
}

/* 
 * test6 : int -> void
 * The test function to test functionality of 
 * the list/queue operations.
 */

void test6() {
	qthread_t th1 = NULL, th2 = NULL;
	qthread_mutex_init(&lock, NULL);
	// this is a detached thread
	qthread_create(&th1, NULL, counters_th1, NULL);
	qthread_create(&th2, (void*) 1, counters_th2, NULL);
	
	void* retval;
	qthread_join(th1, &retval);
	qthread_join(th2, &retval);
	printf("\nTest 6 done\n");
}

/* 
 * test7 : int -> void
 * The test function to test functionality of 
 * the list/queue operations.
 */

void test7() {
	qthread_t th1 = NULL, th2 = NULL;
	qthread_mutex_init(&lock, NULL);
	// this is a detached thread
	qthread_create(&th1, NULL, counters_th1, NULL);
	qthread_create(&th2, (void*) 1, counters_th2, NULL);
	
	void* retval;
	qthread_join(th1, &retval);
	//qthread_join(th2, &retval);
	printf("\nTest 7 done\n");
}

/* 
 * test8 : int -> void
 * The test function to test functionality of 
 * the list/queue operations.
 */

void test8() {
	qthread_t th1 = NULL, th2 = NULL;
	qthread_mutex_init(&lock, NULL);
	// this is a detached thread
	qthread_create(&th1, (void*) 1, counters_th1, NULL);
	qthread_create(&th2, NULL, counters_th2, NULL);
	
	void* retval;
	//qthread_join(th1, &retval);
	qthread_join(th2, &retval);
	printf("\nTest 8 done\n");
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
	qthread_t head;
	int attr = 0;
    test0();
    test1();
    test2();
    //qthread_create(&head, &attr, NULL, NULL);
    test3(); // to check the queue and list functionality
    test4();
    test5();
    test6();
    test7();
    test8();
    
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
