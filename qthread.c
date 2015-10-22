/*
 * file:        qthread.c
 * description: simple emulation of POSIX threads
 * class:       CS 7600, Fall 2015
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <unistd.h>
#include <errno.h>
#include "qthread.h"

#define LOCK 1
#define UNLOCK 0

#define READ 0
#define WRITE 1

//enum IO = {"Read", "Write"};

/*
 * do_switch is defined in do-switch.s, as the stack frame layout
 * changes with optimization level, making it difficult to do with
 * inline assembler.
 */
extern void do_switch(void **location_for_old_sp, void *new_value);

struct qthread {
	 int thread_id;  /* to store the id of the thread */
	 int finished;   /* to indicate whether the thread has 
					  * finished executing */
	 int sleeping;   /* to indicate whether the thread is sleeping */
	 int lock_queued;/* to indicate whether the thread is queued in 
					  *	some lock */
     void* thread_stack; /* the pointer to the thread stack */
     void* current_sp; /* the pointer to the sp of thread stack */
     int isDetached;   /* to indicate a detached thread */
     double time_to_wake_up;  /* to indicate the time to wake up if 
							   * sleeping */
     void* return_value;  /* to save the return value of the thread */
     struct qthread* to_join; /* to save the pointer of the thread 
                               * to which this thread must join to */
     struct qthread* next; /* to store a pointer to the next thread 
                            * in a queue or a list */
};

/* A queue structure to save the front and 
 * rear of a queue of threads */
struct queue_list {
	qthread_t front;
	qthread_t rear;
};

struct fd_wait_node {
	int fd;
	int io_type; // 0 = read, 1 = write
	queue_t io_wait_q;
	struct fd_wait_node *next;
};
typedef struct fd_wait_node* fd_wait_t;

/* Forward declarations */
qthread_t add_thread_to_list(qthread_t head, qthread_t thread);
qthread_t dequeue(queue_t *queue_name);
void enqueue(queue_t *queue_name, qthread_t new_node);
qthread_t remove_threads_from_list(qthread_t head);

/* A good organization is to keep a pointer to the 'current'
 * (i.e. running) thread, and a list of 'active' threads (not
 * including 'current') which are also ready to run.
 */
/* Note that on startup there's already a thread running thread - we
 * need a placeholder 'struct thread' so that we can switch away from
 * that to another thread and then switch back. 
 */
struct qthread os_thread = {};
struct qthread *current = &os_thread;
queue_t RUNNABLE_Q = NULL;   // the queue of runnable threads
int thread_id = 1;
qthread_t sleeping_threads = NULL;  // to keep track of the threads 
	// which are sleeping now and need to awake up after some time
fd_wait_t io_thread_wait_fds = NULL;  // to store the threads waititng for IO according to fd's

/*
 * setup_stack(stack, function, arg1, arg2) - sets up a stack so that
 * switching to it from 'do_switch' will call 'function' with arguments
 * 'arg1' and 'arg2'. Returns the resulting stack pointer.
 *
 * works fine with functions that take one argument ('arg1') or no
 * arguments, as well - just pass zero for the unused arguments.
 */
void *setup_stack(int *stack, void *func, void *arg1, void *arg2)
{
    int old_bp = (int)stack;	/* top frame - SP = BP */

    *(--stack) = 0x3A3A3A3A;    /* guard zone */
    *(--stack) = 0x3A3A3A3A;
    *(--stack) = 0x3A3A3A3A;

    /* this is the stack frame "calling" 'func'
     */
    *(--stack) = (int)arg2;     /* argument */
    *(--stack) = (int)arg1;     /* argument (reverse order) */
    *(--stack) = 0;             /* fake return address (to 'func') */

    /* this is the stack frame calling 'do_switch'
     */
    *(--stack) = (int)func;     /* return address */
    *(--stack) = old_bp;        /* %ebp */
    *(--stack) = 0;             /* %ebx */
    *(--stack) = 0;             /* %esi */
    *(--stack) = 0;             /* %edi */
    *(--stack) = 0xa5a5a5a5;    /* valid stack flag */

    return stack;
}



/* You'll need to do sub-second arithmetic on time. This is an easy
 * way to do it - it returns the current time as a floating point
 * number. The result is as accurate as the clock usleep() uses, so
 * it's fine for us.
 */
static double gettime(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec/1.0e6;
}

/* 
 * print_q : queue_t -> void
 * Prints the given queue Q.
 */
void print_q(queue_t Q, char* msg) {
	if (Q == NULL) {
		printf("\n%s is empty!!!", msg);
	}
	else {
		qthread_t temp = Q->front;
		printf("\n%s ", msg);
		while (temp != Q->rear) {
			printf("%d--%d", temp->thread_id, temp->lock_queued);
			temp = temp->next;
		}
		printf("%d--%d", temp->thread_id, temp->lock_queued);
	}
}

void print_io(fd_wait_t io, char* msg) {
	if (io == NULL) {
		printf("\n IO Block list is EMPTY !!");
	}
	else {
		fd_wait_t temp = io;
		while (temp != NULL) {
			printf("\n%s, ",msg);
			printf("IO fd = %d", io->fd);
			printf("IO type = %d", io->io_type);
			print_q(io->io_wait_q, "io wait_q");
			temp = temp->next;
		}
	}
}

/*
 * print_threads : qthread_t char* -> void
 * Prints the msg and threads in the list pointed to by head.
 */ 
void print_threads(qthread_t head, char* msg) {
	qthread_t temp = head;
	if (head == NULL) {
		printf("\n%s EMPTY!!", msg);
		return;
	}
	printf("\n%s : Threads in order are  -- ", msg);
	while (temp != NULL) {
		printf("%d--%f ", temp->thread_id, temp->time_to_wake_up);
		temp = temp->next;
	}
	printf("\n");
}

void dummy(void* (*func)(void *), void *arg1) {
	void* retval = func(arg1);
	qthread_exit(retval);
}

void clear_queue_to_runnable(queue_t *queue) {
	qthread_t temp = NULL;
	while (*queue != NULL) {
		temp = dequeue(&(*queue));
		temp->lock_queued = 0;
		enqueue(&RUNNABLE_Q, temp);
	}
}



/* Beware - you cannot use do_switch to switch from a thread to
 * itself. If there are no other active threads (or after a timeout
 * the first scheduled thread is the current one) you should return
 * without switching. (why? because you haven't saved the current
 * stack pointer)
 */

fd_set BLOCKING_FD_SET_READ;
fd_set BLOCKING_FD_SET_WRITE;
//FD_ZERO(&BLOCKING_FD_SET_READ);
//FD_ZERO(&BLOCKING_FD_SET_WRITE);
#define FD_MAXSIZE 1024 // the maximum size of blocking fd 
/* 
 * */
 
fd_wait_t io_unblock_threads(fd_wait_t head) {
	struct timeval t;
	fd_wait_t temp = head, prev;
	t.tv_sec = 1;
	t.tv_usec = 1;
	int fd = -1;
	printf("\nio_unblock_threads called .. \n");
	print_io(head, "IO BLOCKED THREADS : ");
	FD_ZERO(&BLOCKING_FD_SET_WRITE);
	FD_ZERO(&BLOCKING_FD_SET_READ);
	while(temp != NULL) {
		if (temp->io_type == READ)
			FD_SET(temp->fd, &BLOCKING_FD_SET_READ);
		else
			FD_SET(temp->fd, &BLOCKING_FD_SET_WRITE);
		temp = temp->next;
	}
	
	select(FD_MAXSIZE, &BLOCKING_FD_SET_READ, 
	&BLOCKING_FD_SET_WRITE, NULL, &t);
	
	temp = head;
	prev = temp;
	while (temp != NULL) {
		fd = temp->fd;
		
		if (((FD_ISSET(fd, &BLOCKING_FD_SET_READ)) && 
		(temp->io_type == READ)) || 
		((FD_ISSET(fd, &BLOCKING_FD_SET_WRITE)) && 
		(temp->io_type == WRITE))){
			//remove the threads from the fd's thread q and
			//enqueue it to the runnable thread
			clear_queue_to_runnable(&temp->io_wait_q);	
			if (prev == temp) {
				// the first node
				prev = temp->next;
				head = temp->next;
			}
			else {
				// this is not the first node
				prev->next = temp->next;
			}
			//prev = temp;
		}
		else {
			prev = temp;
		}
		temp = temp->next;
	}
	return head;
}

void context_switch(void) {
	qthread_t new_thread = NULL, old_current = NULL;
	printf("\n ----- context_switch called -------- \n");
	print_q(RUNNABLE_Q, "Runnable Q : ");
	while ((RUNNABLE_Q != NULL) || (sleeping_threads != NULL) ||
	(io_thread_wait_fds != NULL)) {
		
		//print_threads(sleeping_threads, "Sleeping threads Original ");
		//printf("\nCurrent time = %f", gettime());
		// to check the sleeping threads list if any of 
		// them are ready to put in runnable queue
		
		//print_threads(sleeping_threads, "Sleeping threads Final ");
		
		print_q(RUNNABLE_Q, "Runnable Q : ");
		new_thread = dequeue(&RUNNABLE_Q);
		
		if (RUNNABLE_Q == NULL) {
			sleeping_threads = remove_threads_from_list(sleeping_threads);
			if (RUNNABLE_Q == NULL) {
				io_thread_wait_fds = io_unblock_threads(io_thread_wait_fds);
			}
		}
		
		if ((new_thread != NULL) && (new_thread->sleeping == 0) && 
		(new_thread->lock_queued == 0) && (new_thread->finished == 0)) {
			if (new_thread != current) {
				printf("\nCurrent thread : %d --- to switch thread: %d\n", 
				current->thread_id, new_thread->thread_id);
				old_current = current;
				current = new_thread;
				do_switch(&old_current->current_sp, new_thread->current_sp);
				break;
			}
			else {
				printf("\nCurrent thread : %d --- to switch thread: %d NOT POSSIBLE---\n", 
				current->thread_id, new_thread->thread_id);
				if ((RUNNABLE_Q != NULL) || (sleeping_threads != NULL)
				|| (io_thread_wait_fds != NULL)) {
					enqueue(&RUNNABLE_Q, new_thread);
				}
			}
		}
		
	}
}

/* qthread_yield - yield to the next runnable thread.
 */
int qthread_yield(void)
{
	enqueue(&RUNNABLE_Q, current);
	context_switch();
    return 0;
}

/* Initialize a thread attribute structure. We're using an 'int' for
 * this, so just set it to zero.
 */
int qthread_attr_init(qthread_attr_t *attr)
{
    *attr = 0;
    return 0;
}

/* The only attribute supported is 'detached' - if it is true a thread
 * will clean up when it exits; otherwise the thread structure hangs
 * around until another thread calls qthread_join() on it.
 */
int qthread_attr_setdetachstate(qthread_attr_t *attr, int detachstate)
{
    *attr = detachstate;
    return 0;
}

qthread_t get_new_thread(qthread_attr_t *attr) {
    qthread_t new_thread = (qthread_t) malloc(sizeof(struct qthread));
    new_thread->thread_id = thread_id++;
    new_thread->thread_stack = NULL;
    new_thread->current_sp = NULL;
    new_thread->isDetached = (attr == NULL? 0 : (int)attr);
    new_thread->time_to_wake_up = 0;
    new_thread->return_value = (void*) 0;
    new_thread->to_join = NULL;
    new_thread->next = NULL;
    new_thread->finished = 0;
    new_thread->sleeping = 0;
    new_thread->lock_queued = 0;
    return new_thread;
}

fd_wait_t get_new_fd_wait_node (int fd, int io_type) {
	fd_wait_t fd_node = (fd_wait_t) malloc (sizeof (struct fd_wait_node));
	fd_node->fd = fd;
	fd_node->io_type = io_type;
	fd_node->io_wait_q = NULL;
	fd_node->next = NULL;
	return fd_node;
}

/* a thread can exit by either returning from its main function or
 * calling qthread_exit(), so you should probably use a dummy start
 * function that calls the real start function and then calls
 * qthread_exit after it returns.
 */

/* qthread_create - create a new thread and add it to the active list
 */
int qthread_create(qthread_t *thread, qthread_attr_t *attr,
                   qthread_func_ptr_t start, void *arg)
{
    qthread_t th = get_new_thread(attr);
    *thread = th;
    int stack_size = (1024 * 1024);
    void* buf = (void*) malloc(stack_size);
	int* top_stack = buf + stack_size;
    (*thread)->current_sp = setup_stack(top_stack, dummy, start, arg);
    (*thread)->thread_stack = buf;
    enqueue(&RUNNABLE_Q, *thread);
    print_q(RUNNABLE_Q, "RUNNABLE Q");
    return 0;
}

/* qthread_exit - sort of like qthread_yield, except we never
 * return. If the thread is joinable you need to save 'val' for a
 * future call to qthread_join; otherwise you can free allocated
 * memory. 
 */
void qthread_exit(void *val)
{
    current->finished = 1;
    // checking whether it is a detached thread, for which 
    // return value is not required and it is not required to join
    if (current->isDetached != 1) {
		current->return_value = val;
		while (current->to_join == NULL) {
			qthread_yield();
		}
		enqueue(&RUNNABLE_Q, current->to_join);
		current->to_join = NULL;
	}
	context_switch();
}

void print_mutex(qthread_mutex_t *mutex, char* msg) {
	printf("\n%s, ",msg);
	printf("mutex lock = %d", mutex->lock);
	if (mutex->owner != NULL) 
		printf("  Owner = %d ", mutex->owner->thread_id);
	print_q(mutex->wait_q, "mutex wait_q");
}

/* qthread_mutex_init/destroy - initialize (destroy) a mutex. Ignore
 * 'attr' - mutexes are non-recursive, non-debugging, and
 * non-any-other-POSIX-feature. 
 */
int qthread_mutex_init(qthread_mutex_t *mutex, qthread_mutexattr_t *attr)
{
	mutex = (qthread_mutex_t *) malloc(sizeof(qthread_mutex_t));
    mutex->lock = UNLOCK;
    mutex->owner = NULL;
    mutex->wait_q = NULL;
    return 0;
}

int qthread_mutex_destroy(qthread_mutex_t *mutex)
{
    //free(mutex);
    return 0;
}

/* qthread_mutex_lock/unlock
 */
int qthread_mutex_lock(qthread_mutex_t *mutex)
{
	print_mutex(mutex, "qthread_mutex_lock start : ");
    while (mutex->lock == LOCK) {
		if ((current->sleeping == 0) && (current->lock_queued == 0)) {
			printf("\nThread : %d will get queued", current->thread_id);
			enqueue(&(mutex->wait_q), current);
			print_q(mutex->wait_q, "mutex->wait_q : ");
			current->lock_queued = 1;
			context_switch();
		}
		else {
			return 0;
		}
	}
	printf("\nThread %d : Lock obtained !!\n", current->thread_id);
	mutex->lock = LOCK;
	mutex->owner = current;
	print_mutex(mutex, "qthread_mutex_lock end : ");
    return 0;
}

int qthread_mutex_unlock(qthread_mutex_t *mutex)
{
	//qthread_t temp = NULL;
	print_mutex(mutex, "qthread_mutex_unlock start : ");
	mutex->lock = UNLOCK;
	mutex->owner = NULL;
	clear_queue_to_runnable(&mutex->wait_q);
	/*while (mutex->wait_q != NULL) {
		temp = dequeue(&mutex->wait_q);
		temp->lock_queued = 0;
		enqueue(&RUNNABLE_Q, temp);
	}*/
	print_mutex(mutex, "qthread_mutex_unlock end : ");
	print_q(RUNNABLE_Q, "RUNNABLE Q : ");
    return 0;
}

/* qthread_cond_init/destroy - initialize a condition variable. Again
 * we ignore 'attr'.
 */
int qthread_cond_init(qthread_cond_t *cond, qthread_condattr_t *attr)
{
    cond = (qthread_cond_t *) malloc (sizeof(qthread_cond_t));
    cond->cond_q = NULL;
    return 0;
}
int qthread_cond_destroy(qthread_cond_t *cond)
{
    //free(cond);
    return 0;
}

/* qthread_cond_wait - unlock the mutex and wait on 'cond' until
 * signalled; lock the mutex again before returning.
 */
int qthread_cond_wait(qthread_cond_t *cond, qthread_mutex_t *mutex)
{
	printf("\n qthread_cond_wait called: ");
	print_mutex(mutex, "qthread_mutex state: ");
	print_q(RUNNABLE_Q, "RUNNABLE Q : ");
    if (mutex->owner == current) {
		qthread_mutex_unlock(mutex);
		current->lock_queued = 1;  // to avoid scheduling
		enqueue(&cond->cond_q, current);
		print_q(cond->cond_q, "cond->cond_q : ");
		context_switch();
	}
    return 0;
}

/* qthread_cond_signal/broadcast - wake one/all threads waiting on a
 * condition variable. Not an error if no threads are waiting.
 */
int qthread_cond_signal(qthread_cond_t *cond)
{
	qthread_t temp = NULL;
	printf("\n qthread_cond_signal called: ");
	print_q(cond->cond_q, "cond->cond_q initial : ");
	temp = dequeue(&cond->cond_q);
	if (temp != NULL) 
	{
		temp->lock_queued = 0;
		enqueue(&RUNNABLE_Q, temp);
	}
    print_q(cond->cond_q, "cond->cond_q final : ");
    print_q(RUNNABLE_Q, "RUNNABLE Q : ");
    return 0;
}

int qthread_cond_broadcast(qthread_cond_t *cond)
{
	printf("\n qthread_cond_signal called: ");
	print_q(cond->cond_q, "cond->cond_q initial : ");
    clear_queue_to_runnable(&cond->cond_q);
    print_q(cond->cond_q, "cond->cond_q final : ");
    print_q(RUNNABLE_Q, "RUNNABLE Q : ");
    return 0;
}

/* POSIX replacement API. These are all the functions (well, the ones
 * used by the sample application) that might block.
 *
 * If there are no runnable threads, your scheduler needs to block
 * waiting for one of these blocking functions to return. You should
 * probably do this using the select() system call, indicating all the
 * file descriptors that threads are blocked on, and with a timeout
 * for the earliest thread waiting in qthread_usleep()
 */

/* qthread_usleep - yield to next runnable thread, making arrangements
 * to be put back on the active list after 'usecs' timeout. 
 */
int qthread_usleep(long int usecs)
{
	if (current->finished == 0) {
	current->time_to_wake_up = gettime() + (usecs/1.0e6);
	printf("\nto add sleeping threads: usecs = %ld for thread = %d", usecs, current->thread_id);
    sleeping_threads = add_thread_to_list(sleeping_threads, current);
    current->sleeping = 1;
    context_switch();
	}
    return 0;
}

void io_block_thread(int sockfd, int io_type) {
	fd_wait_t temp = io_thread_wait_fds;
	fd_wait_t prev = temp;
	while (temp != NULL) {
		if ((temp->fd == sockfd) && (temp->io_type == io_type)) {
			// this is the fd for which the current thread will block
			enqueue(&temp->io_wait_q, current);
			break;
		}
		prev = temp;
		temp = temp->next;
	}
	if (temp == NULL) {
		// the fd is not present, need to create a new one
		fd_wait_t new_fd = get_new_fd_wait_node(sockfd, io_type);
		enqueue(&new_fd->io_wait_q, current);
		if (prev != NULL) {
			// not the first node
			prev->next = new_fd;
		}
		else {
			io_thread_wait_fds = new_fd;
		}
	}
}


/* make sure that the file descriptor is in non-blocking mode, try to
 * read from it, if you get -1 / EAGAIN then add it to the list of
 * file descriptors to go in the big scheduling 'select()' and switch
 * to another thread.
 */
ssize_t qthread_read(int sockfd, void *buf, size_t len)
{
    /* set non-blocking mode every time. If we added some more
     * wrappers we could set non-blocking mode from the beginning, but
     * this is a lot simpler (if less efficient)
     */
    int tmp = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, tmp | O_NONBLOCK);
    
    int result = read(sockfd, buf, len);
	while ((result == -1) || (result == EAGAIN)) {
		io_block_thread(sockfd, READ);
		current->lock_queued = 1;
		context_switch();
		result = read(sockfd, buf, len);
	}
	current->lock_queued = 0;
    return result;
}

/* like read - make sure the descriptor is in non-blocking mode, check
 * if if there's anything there - if so, return it, otherwise save fd
 * and switch to another thread. Note that accept() counts as a 'read'
 * for the select call.
 */
int qthread_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    int tmp = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, tmp | O_NONBLOCK);
    
    int result = read(sockfd, addr, *addrlen);
	while ((result == -1) || (result == EAGAIN)) {
		io_block_thread(sockfd, READ);
		current->lock_queued = 1;
		context_switch();
		result = read(sockfd, addr, *addrlen);
	}
	current->lock_queued = 0;
    //return result;
    return 0;
}

/* Like read, again. Note that this is an output, rather than an input
 * - it can block if the network is slow, although it's not likely to
 * in most of our testing.
 */
ssize_t qthread_write(int sockfd, const void *buf, size_t len)
{
    int tmp = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, tmp | O_NONBLOCK);
    
    int result = write(sockfd, buf, len);
	while ((result == -1) || (result == EAGAIN)) {
		io_block_thread(sockfd, WRITE);
		current->lock_queued = 1;
		context_switch();
		result = write(sockfd, buf, len);
	}
	current->lock_queued = 0;
    return result;
}

/* Wait for a non-detached thread to exit. Returns -1 if the thread
 * does not exist or is not joinable, returns 0 otherwise, and places
 * the exit value (passed to qthread_exit(), or return value from thread
 * function) in *retval.
 */
int qthread_join(qthread_t thread, void **retval)
{
	printf("\n---- join invoked ---\n");
    if ((thread == NULL) || (thread->isDetached == 1))
		return -1;	
	thread->to_join = current;
	context_switch();
	if (retval != NULL) {
		*retval = thread->return_value;
	}
    return 0;
}


qthread_t get_new_node(int data) {
	qthread_t new_node = (qthread_t) malloc(sizeof(struct qthread));
	new_node->time_to_wake_up = data;
	new_node->next = NULL;
	return new_node;
}

/* 
 * enqueue : queue_t *qthread_t -> void
 * Adds new_node at the end of the queue if it exists,
 * else creates a new queue and makes front and rear point to it.
 */
void enqueue(queue_t *queue_name, qthread_t new_node) {
	if (new_node != NULL) {
		if (*queue_name == NULL) {
			*queue_name = (queue_t) malloc(sizeof(struct queue_list));
			(*queue_name)->front = new_node;
			(*queue_name)->rear = new_node;
		}
		else {
			qthread_t last_node = (*queue_name)->rear;
			last_node->next = new_node;
			(*queue_name)->rear = new_node;
		}
	}
}

/* 
 * dequeue : queue_t -> qthread_t
 * Removes and returns a node from the front of the queue 
 * if it exists, otherwise returns NULL.
 */
qthread_t dequeue(queue_t *queue_name) {
	qthread_t removed_node = NULL;
	if (*queue_name != NULL) {
		if ((*queue_name)->front == (*queue_name)->rear) {
			removed_node = (*queue_name)->front;
			*queue_name = NULL;
		}
		else {
			removed_node = (*queue_name)->front;
			(*queue_name)->front = (*queue_name)->front->next;
		} 
		removed_node->next = NULL;
	}
	return removed_node;
}

/* 
 * add_thread_to_list : qthread_t qthread_t -> qthread_t
 * Adds the given thread to the list of threads pointed to by head 
 * and returns the head.
 */
qthread_t add_thread_to_list(qthread_t head, qthread_t thread) {
	if (thread == NULL)
		return head;
		
	if (head == NULL) {
		head = thread;
		thread->next = NULL;
	}
	else {
		// to insert the thread at the correct position according to 
		// time.
		qthread_t temp = head;
		qthread_t prev = head;
		while ((temp != NULL) && 
		(temp->time_to_wake_up <= thread->time_to_wake_up)) {
			prev = temp;
			temp = temp->next;
		}
		if (prev == temp) {
			// to insert as the head node
			thread->next = head;
			head = thread;
		}
		else if (temp == NULL) {
			prev->next = thread;
			thread->next = NULL;
		}
		else if (temp->time_to_wake_up > thread->time_to_wake_up) {
			thread->next = prev->next;
			prev->next = thread;
		}
		else {
			temp->next = thread;
			thread->next = NULL;
		}
	}
	return head;
}

/* remove_threads_from_list
 * Removes threads which are ready to run andputs them in the 
 * Runnable Queue
 */
qthread_t remove_threads_from_list(qthread_t head) {
	qthread_t temp = head;
	qthread_t prev = head;
	while ((temp != NULL) && (temp->time_to_wake_up <= gettime())) {
		// add to runnable queue
		temp->sleeping = 0;
		temp->time_to_wake_up = 0;
		enqueue(&RUNNABLE_Q, temp);
		printf("\nPut thread = %d from SLEEP to RUN", temp->thread_id);
		// remove this thread from this list
		if (prev == temp) {
			// the first node
			prev = temp->next;
			head = temp->next;
		}
		else {
			// this is not the first node
			prev->next = temp->next;
		}
		//prev = temp;
		temp = temp->next;
	}
	return head;
}


/* 
 * test_for_list_operations : int -> void
 * The test function to test functionality of 
 * the list/queue operations.
 */
int test_for_list_operations() {
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
