/*
 * file:        list-impl.c
 * description: helper functions for list/queue operations
 * class:       CS 7600, Fall 2015
 */

//#include "qthread.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <sys/time.h>
#include <fcntl.h>
struct qthread {
     int data;
     struct qthread *next;


    /* your code here */
};
typedef struct qthread* qthread_t;

struct queue_list {
	qthread_t front;
	qthread_t rear;
};
typedef struct queue_list* queue;

/* 
 * enqueue : queue *qthread_t -> void
 * Adds new_node at the end of the queue if it exists,
 * else creates a new queue and makes front and rear point to it.
 */
void enqueue(queue *queue_name, qthread_t new_node) {
	if (*queue_name == NULL) {
		*queue_name = (queue) malloc(sizeof(struct queue_list));
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
 * dequeue : queue -> queue
 * Removes and returns a node from the front of the queue 
 * if it exists, otherwise returns NULL.
 */
qthread_t dequeue(queue *queue_name) {
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
	new_node->data = data;
	new_node->next = NULL;
	return new_node;
}

/* 
 * print_q : queue -> void
 * Prints the given queue Q.
 */
void print_q(queue Q) {
	if (Q == NULL) {
		printf("\nQueue is empty!!!");
	}
	else {
		qthread_t temp = Q->front;
		printf("\nQueue is  -- ");
		while (temp != Q->rear) {
			printf("%d ", temp->data);
			temp = temp->next;
		}
		printf("%d \n", temp->data);
	}
}

/* 
 * main : int -> void
 * The main functino to test functionality of 
 * the list/queue operations.
 */
int main() {
	printf("In main");
	queue Q = NULL;
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
}
