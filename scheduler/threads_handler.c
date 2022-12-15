/*
 * Copyright Ionescu Matei-Stefan - 323CA - 2022-2023
 */

#include "threads_handler.h"
#include "priority_queue.h"
#include "utils.h"

#include <semaphore.h>
#include <stdlib.h>

/* allocate memory for a new scheduler and initialize it */
scheduler_t *create_scheduler(unsigned int time_quantum, unsigned int io)
{
	scheduler_t *new_scheduler = malloc(sizeof(scheduler_t));
	DIE(new_scheduler == NULL, "new_scheduler malloc() failed.");

	new_scheduler->time_quantum = time_quantum;
	new_scheduler->io = io;

	new_scheduler->ready_threads = pq_create();
	new_scheduler->waiting_threads = pq_create();
	new_scheduler->terminated_threads = pq_create();

	new_scheduler->running_thread = NULL;

	new_scheduler->threads_nr = 0;

	new_scheduler->threads_ids_size = THREADS_INITIAL_SIZE;

	new_scheduler->thread_ids =
		malloc(THREADS_INITIAL_SIZE * sizeof(pthread_t));

	return new_scheduler;
}

/* free the memory allocated for the scheduler */
void destroy_scheduler(scheduler_t **scheduler)
{
	pq_free(&(*scheduler)->ready_threads);
	pq_free(&(*scheduler)->waiting_threads);
	pq_free(&(*scheduler)->terminated_threads);

	free((*scheduler)->running_thread);

	free((*scheduler)->thread_ids);

	free(*scheduler);
	*scheduler = NULL;
}

/* allocate memory for a new thread and initialize it */
thread_t *create_thread(so_handler *function, unsigned int priority)
{
	thread_t *new_thread = malloc(sizeof(thread_t));
	DIE(new_thread == NULL, "new_thread malloc() failed.");

	new_thread->function = function;
	new_thread->priority = priority;
	new_thread->state = NEW;
	new_thread->thread_id = 0;
	new_thread->used_time = 0;
	new_thread->waited_io = SO_MAX_NUM_EVENTS;

	int rc = sem_init(&new_thread->th_running, 0, 0);
	DIE(rc != 0, "sem_init() failed.");

	return new_thread;
}

/* the function that created threads run */
void *start_thread(void *args)
{
	thread_t *thread = ((start_args_t *)args)->thread;
	scheduler_t *scheduler = ((start_args_t *)args)->scheduler;

	/* wait for the thread to be running */
	int rc = sem_wait(&thread->th_running);
	DIE(rc != 0, "sem_wait() failed.");

	/* call the function */
	thread->function(thread->priority);

	/* update thread state and scheduler */
	thread->state = TERMINATED;
	update_scheduler(scheduler);

	free(args);

	return NULL;
}

/* set running thread following the round robin algorithm */
void update_scheduler(scheduler_t *scheduler)
{
	/* there is no thread running */
	if (scheduler->running_thread == NULL) {
		update_running_thread(scheduler);
		return;
	}

	/* thread finished it's execution */
	if (scheduler->running_thread->state == TERMINATED) {
		pq_push(scheduler->terminated_threads, scheduler->running_thread);
		scheduler->running_thread = NULL;

		update_running_thread(scheduler);

		return;
	}

	/* thread is waiting for IO */
	if (scheduler->running_thread->state == WAITING) {
		pq_push(scheduler->waiting_threads, scheduler->running_thread);
		scheduler->running_thread = NULL;

		update_running_thread(scheduler);

		return;
	}

	/* thread reached it's maximum time used */
	if (scheduler->running_thread->used_time == scheduler->time_quantum) {

		preempt_thread(scheduler);

		update_running_thread(scheduler);

		return;
	}

	/* a thread with higher priority has been created and needs to be run */
	if (pq_peek(scheduler->ready_threads) != NULL &&
		scheduler->running_thread->priority <
			pq_peek(scheduler->ready_threads)->priority) {

		preempt_thread(scheduler);

		update_running_thread(scheduler);

		return;
	}

	/* if nothing happed signal the thread to continue */
	sem_post(&scheduler->running_thread->th_running);
}

/* select the next thread from the ready queue */
void update_running_thread(scheduler_t *scheduler)
{
	if (pq_peek(scheduler->ready_threads) != NULL) {

		thread_t *next_thread = pq_pop(scheduler->ready_threads);
		scheduler->running_thread = next_thread;

		scheduler->running_thread->state = RUNNING;

		sem_post(&scheduler->running_thread->th_running);

		return;
	}
	scheduler->running_thread = NULL;
}

/* move the running thread to the ready queue and reset it's used time */
void preempt_thread(scheduler_t *scheduler)
{
	scheduler->running_thread->used_time = 0;
	scheduler->running_thread->state = READY;

	pq_push(scheduler->ready_threads, scheduler->running_thread);

	scheduler->running_thread = NULL;
}