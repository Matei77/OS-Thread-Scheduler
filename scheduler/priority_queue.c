/*
 * Copyright Ionescu Matei-Stefan - 323CA - 2022-2023
 */

#include "priority_queue.h"
#include "utils.h"

#include <stdlib.h>
#include <string.h>

/* Creates a new node */
node_t *create_new_node(thread_t *thread)
{
	node_t *new_node = (node_t *)malloc(sizeof(node_t));
	DIE(new_node == NULL, "new_node malloc");

	new_node->thread = thread;

	new_node->next = NULL;

	return new_node;
}

/* Creates a new thread priority queue */
priority_queue_t *pq_create(void)
{
	priority_queue_t *pq;

	pq = malloc(sizeof(*pq));
	DIE(pq == NULL, "priority_queue malloc");

	pq->head = NULL;

	return pq;
}

/* Adds a new thread to a priority queue */
void pq_push(priority_queue_t *queue, thread_t *thread)
{
	if (!queue)
		return;


	node_t *new_node = create_new_node(thread);

	if (!queue->head) {
		queue->head = new_node;
		return;
	}

	if (queue->head->thread->priority < thread->priority) {
		new_node->next = queue->head;
		queue->head = new_node;

	} else {
		node_t *it = queue->head;
		while (it->next != NULL &&
			   it->next->thread->priority >= thread->priority) {
			it = it->next;
		}

		new_node->next = it->next;
		it->next = new_node;
	}
}

/* Removes the thread with the highest priority */
thread_t *pq_pop(priority_queue_t *queue)
{
	node_t *temp = queue->head;
	if (temp != NULL) {
		queue->head = queue->head->next;

		thread_t *thread = temp->thread;

		free(temp);

		return thread;
	}
	return NULL;
}

/* Gets the thread with the highest priority without removing it from the queue
 */
thread_t *pq_peek(priority_queue_t *queue)
{
	if (queue->head)
		return queue->head->thread;
	return NULL;
}

void pq_free(priority_queue_t **queue)
{
	node_t *it = (*queue)->head;
	while (it) {
		node_t *temp = it;
		it = it->next;

		sem_destroy(&temp->thread->th_running);

		free(temp->thread);
		free(temp);
	}
	free(*queue);
	*queue = NULL;
}

/* Removes a given node from the priority queue */
thread_t *pq_remove_node(priority_queue_t *queue, node_t *node)
{
	if (queue->head == node) {
		return pq_pop(queue);
	}

	node_t *it = queue->head;
	while (it->next != NULL && it->next != node) {
		it = it->next;
	}

	if (it->next == NULL) {
		return NULL;
	}

	node_t *temp = it->next;
	thread_t *thread = it->next->thread;

	it->next = it->next->next;

	free(temp);

	return thread;
}
