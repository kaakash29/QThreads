/* File       : test1.c
 * description: test file for qthreads (homework 1)
 */

//################## INCLUDES FOR TESTS #############################
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include "qthread.h"
#include <fcntl.h>
#include <string.h>


//################## MACROS ########################################
#define SIZE 30


//############### TEST FUNCTIONS ###################################
/* TEST 0. create and join. Create 1 thread, which don't do anything
 * except return a value.and possibly print something Call
 * qthread_join() to wait for it.
 */
void *f1(void *arg) { printf("\nTEST ::: Hello\n"); 
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
//#################################################################
/* TEST1. create and join. Create N threads, which don't do anything
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
}

//###############################################################
/* TEST2. mutex and sleep.
 * initialize a mutex
 * Create thread 1, which locks a mutex and goes to sleep for a
 * second or two using qthread_usleep.
 * you can wait for this by calling qthread_yield repeatedly,
 * waiting for thread 1 to set a global variable)
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
    void *val;
    
    qthread_mutex_init(&m, NULL);
    qthread_create(&t0, NULL, f2, NULL);

    while (!t1rdy)
      qthread_yield();

    for (i = 0; i < 4; i++)
        qthread_create(&t[i], NULL, f3, NULL);

    qthread_join(t0, &val);
    for (i = 0; i < 4; i++)
        qthread_join(t[i], &val);
}


//###################################################################
/* TEST4 : Checks if a thread can join before it has been created 
 * Calls qthread join to join a thread which has not yet been created
 */
int test4(void) {
	qthread_t t1;
	void *retval;
	qthread_join(t1, &retval);
	qthread_create(&t1, NULL, f1, NULL);
	
}

//###################################################################
/* TEST5 : to test that only a single thread will execute using 
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


//####################################################################
/* TEST6 : void -> void
 * Tests locking and unlocking of mutex using counter operations 
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
	qthread_attr_t *attr;
	qthread_attr_init(attr);
	qthread_attr_setdetachstate(attr, 1);
	qthread_t th1 = NULL, th2 = NULL;
	counter = 0;
	qthread_mutex_init(&lock, NULL);
	// this is a detached thread
	qthread_create(&th1, NULL, counters_th1, NULL);
	qthread_create(&th2, attr, counters_th2, NULL);
	
	void* retval;
	qthread_join(th1, &retval);
	qthread_join(th2, &retval);
}


//##################################################################
/* test7 : void -> void
 * Tests the Use of mutex with detached threads.
 */
void test7() {
	qthread_attr_t *attr;
	qthread_attr_init(attr);
	qthread_attr_setdetachstate(attr, 1);
	qthread_t th1 = NULL, th2 = NULL;
	counter = 0;
	qthread_mutex_init(&lock, NULL);
	// this is a detached thread
	qthread_create(&th1, NULL, counters_th1, NULL);
	qthread_create(&th2, attr, counters_th2, NULL);
	
	void* retval;
	qthread_join(th1, &retval);
	//qthread_join(th2, &retval);

}

//###################################################################
/* test8 : void -> void
 * Tests the locking and unlocking of mutexes on detached thread. 
 */
void test8() {
	qthread_attr_t *attr;
	qthread_attr_init(attr);
	qthread_attr_setdetachstate(attr, 1);
	qthread_t th1 = NULL, th2 = NULL;
	counter = 0;
	qthread_mutex_init(&lock, NULL);
	// this is a detached thread
	qthread_create(&th1, attr, counters_th1, NULL);
	qthread_create(&th2, NULL, counters_th2, NULL);
	
	void* retval;
	//qthread_join(th1, &retval);
	qthread_join(th2, &retval);
}

//###################################################################
/* test9 : void -> void
 * Tests mutual exclusion without mutexes or conds..
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

//###################################################################
/* test10 : int -> void
 * Test of counters on joinable threads.
 */
 
void test10() {
	qthread_t th1 = NULL, th2 = NULL;
	qthread_mutex_init(&lock, NULL);
	qthread_create(&th1, NULL, counters_th1, NULL);
	qthread_create(&th2, NULL, counters_th2, NULL);
	
	void* retval;
	qthread_join(th1, &retval);
	qthread_join(th2, &retval);
}

//###################################################################
/* TEST11- tests a detached thread that does nothing but yielding 
 * the control back to the scheduler.
 * */

void* f6(void* v) {
  qthread_yield();
}

void test11(void) {
  qthread_t t0;
  qthread_attr_t t0a;
  qthread_attr_setdetachstate(&t0a, 1); 
  qthread_create(&t0, &t0a, f6, NULL);
  qthread_yield();
  qthread_yield();
}

//###################################################################
/* TEST12 - tests the qthread_exit function for a 
 * joinable thread creates a thread to return a value 2
 * */

void* f7(void* inp) {
  qthread_exit((void*)2);
}

void test12(void) {
  qthread_t t0;
  void* output;
  qthread_create(&t0, NULL, f7, NULL);
  qthread_join(t0, &output);
  assert((int)output == 2);
}

//###################################################################
/* TEST13 : tests whether zombie threads are being created 
 * if parent yields before calling join
 * */
void test13(void) {
  qthread_t t0;
  void* output;
  qthread_create(&t0, NULL, f7, NULL);
  qthread_yield();
  qthread_join(t0, &output);
  assert((int)output == 2);
}

//###################################################################
/* TEST14 : tests if calling usleep on the main thread works 
 * */

void test14(void) {
  qthread_usleep(2000000);
}

//###################################################################-
/* TEST15 : tests if creating a thread which does nothing 
 * but sleeps works
 * */
void *s2(void *v) {
  qthread_usleep(2000000);
  return 0;
}

void test15(void) {
  qthread_t t0;
  void* output;
  qthread_create(&t0, NULL, s2, NULL);
  qthread_yield();
  qthread_join(t0, &output);
  assert((int)output == 0);
}


//###################################################################
/* TEST16: tests if possible to acquire a mutex 
 * locked by a detached thread.
 * */
void *s3(void *v) {
  qthread_mutex_lock(&m);
  qthread_usleep(2000000);
  qthread_mutex_unlock(&m);
  qthread_exit((void*)0);
}

void test16(void) {
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

//###################################################################
/* TEST 17 : Tests if teh read function call works in 
 * non blocking mode in a thread 
 * */
void test17(void) {
  int fd = open("test1.c", O_RDONLY);
  char buf[50000];
  qthread_read(fd, buf, 50000);
  close(fd);
  return;
}


//###################################################################
 /* TEST 18: create a pipe ('int fd[2]; pipe(fd);')
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

void test18(void) {
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

//###################################################################
/* TEST19 : Test a solution for the producer consumer problem using
 * qthreads mutex and locks. 
 */
char buffer[SIZE];
int count = 0, head = 0, tail = 0;
qthread_mutex_t l;
qthread_cond_t notEmpty;
qthread_cond_t notFull;

void* put() {
	int produce = 0;
	while(produce++ < 20) {
		qthread_mutex_lock(&l);
		while (count == SIZE) {
			qthread_cond_wait(&notFull, &l);
		}
		count++;
		printf("\n +++ Produced an item +++ \n");
		buffer[head] = produce;
		head++;
		if (head == SIZE) {
			head = 0;
		}
		qthread_cond_signal(&notEmpty);
		qthread_mutex_unlock(&l);
		qthread_usleep(100000);
	}
}

void* get() {
    int c;
    qthread_mutex_lock(&l);
    while (count == 0) {
        qthread_cond_wait(&notEmpty, &l);
    }
    count--;
    c = buffer[tail];
    printf("\n ---Consumed an item ----\n");
    tail++;
    if (tail == SIZE) {
        tail = 0;
    }
    qthread_cond_signal(&notFull);
    qthread_mutex_unlock(&l);
    return (void*) c;
}

void producer_consumer() {
	qthread_t producer;
	qthread_t consumers[20];
	void* retval;
	int i = 0;
	
	qthread_mutex_init(&l, NULL);
	qthread_cond_init(&notEmpty, NULL);
	qthread_cond_init(&notFull, NULL);
	
		qthread_create(&producer, NULL, put, NULL);
	for (i = 0; i < 20; i++)
		qthread_create(&consumers[i], NULL, get, NULL);
		
		qthread_join(producer, &retval);
		
	for (i = 0; i < 20; i++) {
		qthread_join(consumers[i], &retval);
		printf("\n Return from consumer = %d\n", (int) retval);
	}
	
	qthread_cond_destroy(&notEmpty);
	qthread_cond_destroy(&notFull);
	qthread_mutex_destroy(&l);
}

void test19(){
	producer_consumer();
}

/***************************************
 * MAIN - CALL ALL TEST FUNCTIONS HERE
 ***************************************/
int main(int argc, char **argv)
{
    test0();  printf ("\nPASS : single thread create and join"); fflush(stdout);
    test1();  printf ("\nPASS : multiple thread create and join"); fflush(stdout);
    test2();  printf ("\nPASS : mutex and sleep"); fflush(stdout);
    test4();  printf ("\nPASS : join before creation"); fflush(stdout);
    test6();  printf ("\nPASS : mutex lock and unlock using counters"); fflush(stdout);
    test7();  printf ("\nPASS : Use of mutex with detached threads !!!"); fflush(stdout);
    test8();  printf ("\nPASS : lock and unlock mutex on detached thread"); fflush(stdout);
    test9();  printf ("\nPASS : mutual exclusion without mutexes"); fflush(stdout);
    test10(); printf ("\nPASS : counter on joinable threads !!!"); fflush(stdout);
    test11(); printf ("\nPASS : detached thread that yields only"); fflush(stdout);
    test12(); printf ("\nPASS : qthread_exit function"); fflush(stdout);
    test13(); printf ("\nPASS : zombie threads are being create"); fflush(stdout);
    test14(); printf ("\nPASS : usleep on the main thread"); fflush(stdout);
    test15(); printf ("\nPASS : thread to sleep"); fflush(stdout);
    test16(); printf ("\nPASS : acquire a mutex locked by a detached thread");fflush(stdout);
    test17(); printf ("\nPASS : read function call works"); fflush(stdout);
    test18(); printf ("\nPASS : create a pipe read and write on threads");  fflush(stdout);
    test19(); printf ("\nPASS : dining philosopher's solution"); fflush(stdout);
        
    printf("\n\n");
    return 0;
}
