/*
 * Copyright Ionescu Matei-Stefan - 323CA - 2022-2023
 */

#ifndef PRIORITY_QUEUE_H_
#define PRIORITY_QUEUE_H_

#include "threads_handler.h"

typedef struct thread_t thread_t;

typedef struct node_t {
	thread_t *thread;
	struct node_t *next;
} node_t;

typedef struct priority_queue_t {
	node_t *head;
} priority_queue_t;


priority_queue_t *pq_create();

void pq_push(priority_queue_t *queue, thread_t *thread);

thread_t *pq_pop(priority_queue_t *queue);

thread_t *pq_peek(priority_queue_t *queue);

void pq_free(priority_queue_t **queue);

thread_t *pq_remove_node(priority_queue_t *queue, node_t *node);

#endif /* PRIORITY_QUEUE_H_ */
