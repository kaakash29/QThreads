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
#include <fcntl.h>
#include <string.h>

/* 0. create and join. Create 1 thread, which don't do anything
 * except return a value.and possibly print something Call
 * qthread_join() to wait for it.
 */
void *f1(void *arg) { printf("\nTEST ::: Hello\n"); 
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
    printf("test 0 OK\n");
}
//----

/* 1. create and join. Create N threads, which don't do anything
 * except return a value. (and possibly print something) Call
 * qthread_join() to wait for each of them.
 */
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
//----


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
    qthread_usleep(1000);
    qthread_mutex_unlock(&m);
    return 0;
}

void *f3(void *v)
{
    qthread_mutex_lock(&m);
    printf("\nin f3\n");
    qthread_usleep(100000);
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
//-----


/* 
 * test : int -> void
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
}

/* test4 : Checks if a thread can join before it has been created 
 * Calls qthread join to join a thread which has not yet been created
 */
int test4(void) {
	qthread_t t1;
	void *retval;
	qthread_join(t1, &retval);
	qthread_create(&t1, NULL, f1, NULL);
	
}
//----


/* test5 : to test that only a single thread will execute using 
 * our library. creates a thread which hogs up the memory and never
 * lets go doesn't let thread2 to start
 */
void * thread1(){
	 while(1){
        printf("Hello!!\n");
	 }
}

void * thread2(){
        while(1){
                printf("How are you?\n");
        }
}

int test5(void) {
	qthread_t t1, t2;
	qthread_create(&t1,NULL,thread1,NULL);
    qthread_create(&t2,NULL,thread2,NULL);
    qthread_join(t1,NULL);
    qthread_join(t2,NULL);	
}
//-------



/* test6 : int -> void
 * The test function to test functionality of 
 * the list/queue operations.
 */
int counter = 0;
qthread_mutex_t lock;
int num2 = 0;

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

void test6() {
	qthread_t th1 = NULL, th2 = NULL;
	counter = 0;
	qthread_mutex_init(&lock, NULL);
	// this is a detached thread
	qthread_create(&th1, NULL, counters_th1, NULL);
	qthread_create(&th2, (void*) 1, counters_th2, NULL);
	
	void* retval;
	qthread_join(th1, &retval);
	qthread_join(th2, &retval);
	printf("\nTest 6 done\n");
}
//-------


/* test7 : int -> void
 * The test function to test functionality of 
 * the list/queue operations.
 */
void test7() {
	qthread_t th1 = NULL, th2 = NULL;
	counter = 0;
	qthread_mutex_init(&lock, NULL);
	// this is a detached thread
	qthread_create(&th1, NULL, counters_th1, NULL);
	qthread_create(&th2, (void*) 1, counters_th2, NULL);
	
	void* retval;
	qthread_join(th1, &retval);
	//qthread_join(th2, &retval);
	printf("\nTest 7 done\n");
}
//-------

/* test8 : int -> void
 * The test function to test functionality of 
 * the list/queue operations.
 */
void test8() {
	qthread_t th1 = NULL, th2 = NULL;
	counter = 0;
	qthread_mutex_init(&lock, NULL);
	// this is a detached thread
	qthread_create(&th1, (void*) 1, counters_th1, NULL);
	qthread_create(&th2, NULL, counters_th2, NULL);
	
	void* retval;
	//qthread_join(th1, &retval);
	qthread_join(th2, &retval);
	printf("\nTest 8 done\n");
}
//-------

/* test9 : int -> void
 * The test function to test functionality of 
 * the list/queue operations.
 */
void* execute(void* num) {
	printf("\nThe number is: %d", (int) num);
	num2 ++;
	qthread_exit(num);
	//return num;
}

void test9() {
	qthread_t th1 = NULL, th2 = NULL;
	void* arg = (void*) 10;
	void* ret = 0;
	
	qthread_create(&th1, NULL, (execute), arg);
	arg = (void*) 40;
	qthread_create(&th2, NULL, (execute), arg);
	
	qthread_usleep(100);
	
	qthread_join(th2, &ret);
	qthread_join(th1, &ret);
}
//----

/* test10 : int -> void
 * The test function to test functionality of 
 * the list/queue operations.
 */
 
void test10() {
	qthread_t th1 = NULL, th2 = NULL;
	qthread_mutex_init(&lock, NULL);
	qthread_create(&th1, NULL, counters_th1, NULL);
	qthread_create(&th2, NULL, counters_th2, NULL);
	
	void* retval;
	qthread_join(th1, &retval);
	qthread_join(th2, &retval);
	printf("\nTest 10 done\n");
}




/* test_detach - tests a detached thread that does nothing but yielding 
 * the control back to the scheduler.
 * */
void* f6(void* v) {
  qthread_yield();
}

void test_detach(void) {
  qthread_t t0;
  qthread_attr_t t0a;
  qthread_attr_setdetachstate(&t0a, 1); 
  qthread_create(&t0, &t0a, f6, NULL);
  qthread_yield();
  qthread_yield();
}

//-------


/* test_join_first - tests the qthread_exit function for a 
 * joinable thread creates a thread to return a value 2
 * */
void* f7(void* inp) {
  qthread_exit((void*)2);
}

void test_join_first(void) {
  qthread_t t0;
  void* output;
  qthread_create(&t0, NULL, f7, NULL);
  qthread_join(t0, &output);
  assert((int)output == 2);
  printf("test_join_first done");
}
//----


/* test_join_zombie : tests whether zombie threads are being created 
 * if parent yields before calling join
 * */
void test_join_zombie(void) {
  qthread_t t0;
  void* output;
  qthread_create(&t0, NULL, f7, NULL);
  qthread_yield();
  qthread_join(t0, &output);
  assert((int)output == 2);
  printf("test_join_zombie done");
}

/* test_sleep_main : tests if calling usleep on the main thread works */
void test_sleep_main(void) {
  qthread_usleep(2000000);
}
//----

/* test_sleep_thread : tests if creating a thread which does nothing 
 * but sleeps works
 * */
void *s2(void *v) {
  qthread_usleep(2000000);
  return 0;
}

void test_sleep_thread(void) {
  qthread_t t0;
  void* output;
  qthread_create(&t0, NULL, s2, NULL);
  qthread_yield();
  qthread_join(t0, &output);
  assert((int)output == 0);
  printf("test_sleep_thread done"); 
}
//----

/* test_sleep_mutex_detached: tests if possible its to acquire a mutex 
 * locked by a detached thread.
 * */
void *s3(void *v) {
  qthread_mutex_lock(&m);
  qthread_usleep(2000000);
  qthread_mutex_unlock(&m);
  qthread_exit((void*)0);
}

void test_sleep_mutex_detached(void) {
  qthread_mutex_init(&m, NULL);
  qthread_t t0;
  void* output;
  qthread_attr_t t0a;
  qthread_attr_setdetachstate(&t0a, 1); 
  qthread_create(&t0, &t0a, s3, NULL);
  qthread_yield();
  qthread_mutex_lock(&m);
  qthread_mutex_unlock(&m);
}
//---

void test_read(void) {
  int fd = open("test1.c", O_RDONLY);
  char buf[50000];
  qthread_read(fd, buf, 50000);
  close(fd);
  return;
}

 /* 4. read/write
 * create a pipe ('int fd[2]; pipe(fd);')
 * create 2 threads:
 * one sleeps and then writes to the pipe
 * one reads from the pipe. [this tests blocking read]
 */
void *r1(void *v) {
  int *fd = (int*) v;
  char buf[50000];
  qthread_read(fd[0], buf, 50000);
  close(fd[0]);
  printf("%s", buf);
  qthread_exit((void*)0);
}

void *w1(void *v) {
  int *fd = (int*) v;
  qthread_usleep(1000000);
  const char* out = "Winter Is Coming.\n";
  qthread_write(fd[1], out, strlen(out));
  close(fd[1]);
  qthread_exit((void*)0);
}

void test_read_write(void) {
  int fd[2]; pipe(fd);
  qthread_t r, w;
  void* output;
  qthread_create(&r, NULL, r1, &fd);
  qthread_yield();
  qthread_create(&w, NULL, w1, &fd);
  qthread_yield();
  qthread_join(r, &output);
  qthread_join(w, &output);
}
//----

/* 3. condvar and sleep.
* 	initialize a condvar and a mutex
* 	create N threads, each of which locks the mutex, increments a
*   global count, and waits on the condvar; after the wait it
*   decrements the count and releases the mutex.
* 	call qthread_yield until count indicates all threads are waiting
* 	call qthread_signal, qthread_yield until count indicates a
*   thread has left. repeat.
*/
//      YET TO WRITE
//----



/***************************************
 * MAIN - CALL ALL TEST FUNCTIONS HERE
 ***************************************/
int main(int argc, char **argv)
{
    test0(); printf("\ntest0 Done !!!");fflush(stdout);
    test1(); printf("\ntest1 Done !!!");fflush(stdout);
    test2(); printf("\ntest2 Done !!!");fflush(stdout);
    test3(); printf("\ntest3 Done !!!");fflush(stdout);
    test4(); printf("\ntest4 Done !!!");fflush(stdout);
    //test5(); printf("test5 Done !!!");fflush(stdout);
    test6(); printf("\ntest6 Done !!!");fflush(stdout);
    test7(); printf("\ntest7 Done !!!");fflush(stdout);
    test8(); printf("\ntest8 Done !!!");fflush(stdout);
    test9(); printf("\ntest9 Done !!!");fflush(stdout);
    test10(); printf("\ntest10 Done !!!");fflush(stdout);
    
    test_detach(); printf("\ntest_detach Done !!!");fflush(stdout);
	test_join_zombie(); printf("\ntest_join_zombie Done !!!");fflush(stdout);
    //test_sleep_main(); printf("test_sleep_main Done !!!");fflush(stdout);
    test_sleep_thread(); printf("\ntest_sleep_thread Done !!!");fflush(stdout);
    test_sleep_mutex_detached(); printf("\ntest_sleep_mutex_detached Done !!!");fflush(stdout);
    test_read(); printf("\ntest_read Done !!!");fflush(stdout);
    //test_read_write(); printf("\ntest_read_write Done !!!");fflush(stdout);
   
    
    printf("\n\n All tests Done !! No crashes encountered !! \n\n");
    return 0;
}
