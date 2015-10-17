/*
 * file:        list-impl.c
 * description: helper functions for list/queue operations
 * class:       CS 7600, Fall 2015
 */

#include <stdlib.h>
#include <stdio.h>
#include "qthread.h"

/* 
 * enqueue : queue_t *qthread_t -> void
 * Adds new_node at the end of the queue if it exists,
 * else creates a new queue and makes front and rear point to it.
 */
void enqueue(queue_t *queue_name, qthread_t new_node) {
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
	}
	else {
		// to insert the thread at the correct position according to 
		// time.
		qthread_t temp = head;
		qthread_t prev = head;
		while ((temp->next != NULL) && 
		(temp->time_to_wake_up <= thread->time_to_wake_up)) {
			prev = temp;
			temp = temp->next;
		}
		if (prev == temp) {
			// to insert as the head node
			thread->next = head;
			head = thread;
		}
		else if (temp->time_to_wake_up > thread->time_to_wake_up) {
			thread->next = prev->next;
			prev->next = thread;
		}
		else {
			temp->next = thread;
		}
	}
	return head;
}

/* 
 * append_node : *qthread_t *qthread_t -> *qthread_t
 * Adds new_node at the end of the list starting from head and 
 * returns the head.
 */
/*qthread_t append_node(qthread_t head, qthread_t new_node) {
	if (head == NULL) {
		head = new_node;
	}
	else {
		temp = head;
		while (temp.next != NULL) {
			temp = temp.next;
		}
		temp.next = new_node;
	}
	return head;
}*/

qthread_t get_new_node(int data) {
	qthread_t new_node = (qthread_t) malloc(sizeof(struct qthread));
	new_node->time_to_wake_up = data;
	new_node->next = NULL;
	return new_node;
}

/* 
 * print_q : queue_t -> void
 * Prints the given queue Q.
 */
void print_q(queue_t Q) {
	if (Q == NULL) {
		printf("\nQueue is empty!!!");
	}
	else {
		qthread_t temp = Q->front;
		printf("\nQueue is  -- ");
		while (temp != Q->rear) {
			printf("%d ", temp->time_to_wake_up);
			temp = temp->next;
		}
		printf("%d \n", temp->time_to_wake_up);
	}
}

void print_threads(qthread_t head) {
	qthread_t temp = head;
	printf("\nThreads in order are  -- ");
	while (temp != NULL) {
		printf("%d ", temp->time_to_wake_up);
		temp = temp->next;
	}
	printf("\n");
}

/* 
 * main : int -> void
 * The main functino to test functionality of 
 * the list/queue operations.
 */
int main() {
	printf("In main");
	queue_t Q = NULL;
	enqueue(&Q, get_new_node(2));
	print_q(Q);
	enqueue(&Q, get_new_node(3));
	print_q(Q);
	enqueue(&Q, get_new_node(4));
	print_q(Q);
	enqueue(&Q, get_new_node(5));
	print_q(Q);
	qthread_t rem_data = NULL;
	rem_data = dequeue(&Q);
	print_q(Q);
	rem_data = dequeue(&Q);
	print_q(Q);
	rem_data = dequeue(&Q);
	print_q(Q);
	rem_data = dequeue(&Q);
	print_q(Q);
	rem_data = dequeue(&Q);
	print_q(Q);
	rem_data = dequeue(&Q);
	print_q(Q);
	rem_data = dequeue(&Q);
	print_q(Q);
	rem_data = dequeue(&Q);
	print_q(Q);
	qthread_t head = NULL;
	head = add_thread_to_list(head, get_new_node(5));
	head = add_thread_to_list(head, get_new_node(2));
	head = add_thread_to_list(head, get_new_node(4));
	head = add_thread_to_list(head, get_new_node(1));
	head = add_thread_to_list(head, get_new_node(7));
	head = add_thread_to_list(head, get_new_node(7));
	print_threads(head);
	
}
