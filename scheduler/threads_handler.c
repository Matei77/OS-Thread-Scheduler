/*
 * Copyright Ionescu Matei-Stefan - 323CA - 2022-2023
 */

#include "threads_handler.h"
#include "priority_queue.h"
#include "utils.h"

#include <stdlib.h>
#include <semaphore.h>


scheduler_t *create_scheduler(unsigned int time_quantum, unsigned int io) {
	scheduler_t *new_scheduler = malloc(sizeof(scheduler_t));
	DIE(new_scheduler == NULL, "new_scheduler malloc");

	new_scheduler->time_quantum = time_quantum;
	new_scheduler->io = io;

	new_scheduler->ready_threads = pq_create();
	new_scheduler->waiting_threads = pq_create();
	new_scheduler->terminated_threads = pq_create();

	new_scheduler->running_thread = NULL;

	new_scheduler->threads_nr = 0;

	return new_scheduler;
}

void destroy_scheduler(scheduler_t **scheduler) {
	pq_free(&(*scheduler)->ready_threads);
	pq_free(&(*scheduler)->waiting_threads);
	pq_free(&(*scheduler)->terminated_threads);

	free((*scheduler)->running_thread);

	free(*scheduler);
	*scheduler = NULL;
}

thread_t *create_thread(so_handler *function, unsigned int priority) {

	int rc;

	thread_t *new_thread = malloc(sizeof(thread_t));
	new_thread->function = function;
	new_thread->priority = priority;
	new_thread->state = NEW;
	new_thread->thread_id = 0;
	new_thread->used_time = 0;

	rc = sem_init(&new_thread->th_running, 0, 0);
	DIE(rc != 0, "sem_init() failed.");

	return new_thread;
}

void *start_thread(void *args) {
	thread_t *thread = ((start_args_t*)args)->thread;
	scheduler_t *scheduler = ((start_args_t*)args)->scheduler;

	int rc = sem_wait(&thread->th_running);
	DIE(rc != 0, "sem_wait() failed.");

	printf("function ran by thread: %ld\n", thread->thread_id);
	thread->function(thread->priority);

	thread->state = TERMINATED;
	update_scheduler(scheduler);

	free(args);

	return NULL;
}

void update_scheduler(scheduler_t *scheduler) {
	if (scheduler->running_thread == NULL) {
		if (pq_peek(scheduler->ready_threads) != NULL) {

			thread_t *next_thread = pq_pop(scheduler->ready_threads);
			scheduler->running_thread = next_thread;

			scheduler->running_thread->state = RUNNING;
			sem_post(&scheduler->running_thread->th_running);

		} else {
			scheduler->running_thread = NULL;
		}
	} else if (scheduler->running_thread->state == TERMINATED) {
		pq_push(scheduler->terminated_threads, scheduler->running_thread);
		if (pq_peek(scheduler->ready_threads) != NULL) {

			thread_t *next_thread = pq_pop(scheduler->ready_threads);
			scheduler->running_thread = next_thread;

			scheduler->running_thread->state = RUNNING;
			sem_post(&scheduler->running_thread->th_running);
			
		} else {
			scheduler->running_thread = NULL;
		}
	}
}


