/*
 * file:        qthread.h
 * description: interface definitions for CS7600 Pthreads assignment
 *
 * Peter Desnoyers, Northeastern CCIS, 2013
 * $Id: $
 */
#ifndef __QTHREAD_H__
#define __QTHREAD_H__

#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>

/* Definitions of qthread_mutex_t and qthread_cond_t are going
 * to have to go here, because of some poor decision-making on
 * the part of the POSIX folks.
 */

struct qthread {
	 int thread_id;
     void* thread_stack;
     void* current_sp;
     int isDetached;
     int time_to_wake_up;
     struct qthread *next;
};
typedef struct qthread* qthread_t;

struct queue_list {
	qthread_t front;
	qthread_t rear;
};
typedef struct queue_list* queue_t;
 
struct qthread_mutex {
    int lock;
    queue_t wait_q;
};
typedef struct qthread_mutex qthread_mutex_t;

struct qthread_cond {
    int place_holder;
};
typedef struct qthread_cond qthread_cond_t;

/* function pointer w/ signature 'void *f(void*)'
 */
typedef void *(*qthread_func_ptr_t)(void*);  

/* You are free to change these type definitions, but the ones
 * provided can work fairly well.
 */
typedef int qthread_attr_t;       /* need to support detachstate */
typedef void qthread_mutexattr_t; /* no mutex attributes needed */
typedef void qthread_condattr_t;  /* no cond attributes needed */

/* See 'man pthread_cancel' - a cancelled thread returns a distinctive
 * return value.
 */
#define QTHREAD_CANCELLED ((void*)2)

int  qthread_yield(void);
int  qthread_attr_init(qthread_attr_t *attr);
int  qthread_attr_setdetachstate(qthread_attr_t *attr, int detachstate);
int  qthread_create(qthread_t *thread, qthread_attr_t *attr,
                    qthread_func_ptr_t start, void *arg);
int  qthread_join(qthread_t thread, void **retval);
void qthread_exit(void *val);
int  qthread_cancel(qthread_t thread);
int qthread_mutex_init(qthread_mutex_t *mutex, qthread_mutexattr_t *attr);
int qthread_mutex_destroy(qthread_mutex_t *mutex);
int qthread_mutex_lock(qthread_mutex_t *mutex);
int qthread_mutex_unlock(qthread_mutex_t *mutex);

int qthread_cond_init(qthread_cond_t *cond, qthread_condattr_t *attr);
int qthread_cond_destroy(qthread_cond_t *cond);
int qthread_cond_wait(qthread_cond_t *cond, qthread_mutex_t *mutex);
int qthread_cond_signal(qthread_cond_t *cond);
int qthread_cond_broadcast(qthread_cond_t *cond);
qthread_t get_new_node(int data);
void print_threads(qthread_t head);
void print_q(queue_t Q);
qthread_t add_thread_to_list(qthread_t head, qthread_t thread);
qthread_t dequeue(queue_t *queue_name);
void enqueue(queue_t *queue_name, qthread_t new_node);



/* POSIX replacement API. Not general, but enough to run a
 * multi-threaded webserver. 
 */
int     qthread_usleep(long int usecs);
ssize_t qthread_read(int fd, void *buf, size_t len);
struct sockaddr;
int     qthread_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
ssize_t qthread_write(int fd, const void *buf, size_t len);

#endif /* __QTHREAD_H__ */
