/*
 * Copyright Ionescu Matei-Stefan - 323CA - 2022-2023
 */

#include "so_scheduler.h"
#include "priority_queue.h"
#include "threads_handler.h"
#include "utils.h"

#include <stdlib.h>
#include <unistd.h>

scheduler_t *scheduler = NULL;

/*
 * creates and initializes scheduler
 * + time quantum for each thread
 * + number of IO devices supported
 * returns: 0 on success or negative on error
 */
DECL_PREFIX int so_init(unsigned int time_quantum, unsigned int io)
{
	/* check if time_quantum and io are valid */
	if (time_quantum == 0 || io > SO_MAX_NUM_EVENTS) {
		return -1;
	}

	/* check if has already been created */
	if (scheduler)
		return -1;

	/* create the scheduler */
	scheduler = create_scheduler(time_quantum, io);

	return 0;
}

/*
 * creates a new so_task_t and runs it according to the scheduler
 * + handler function
 * + priority
 * returns: tid of the new task if successful or INVALID_TID
 */
DECL_PREFIX tid_t so_fork(so_handler *func, unsigned int priority)
{
	/* check if the priority and function are valid */
	if (priority > SO_MAX_PRIO || !func)
		return INVALID_TID;

	/* initialize a new thread_t thread elemnet */
	thread_t *thread = create_thread(func, priority);

	/* initialize the arguments for the start_thread funciton */
	start_args_t *args = malloc(sizeof(start_args_t));
	args->thread = thread;
	args->scheduler = scheduler;

	/* create a new thread */
	int rc = pthread_create(&thread->thread_id, NULL, start_thread, args);
	DIE(rc != 0, "pthred_create() failed.");

	/* set the thread as ready */
	thread->state = READY;
	pq_push(scheduler->ready_threads, thread);

	/* if there is already a thread running consume one action from it */
	if (scheduler->running_thread != NULL) {
		so_exec();
	} else {
		update_scheduler(scheduler);
	}

	/* add the thread to the array of threads */
	scheduler->thread_ids[scheduler->threads_nr++] = thread->thread_id;

	return thread->thread_id;
}

/*
 * waits for an IO device
 * + device index
 * returns: -1 if the device does not exist or 0 on success
 */
DECL_PREFIX int so_wait(unsigned int io)
{
	/* check if io is valid */
	if (io >= scheduler->io)
		return -1;

	/* update the thread state */
	scheduler->running_thread->state = WAITING;
	scheduler->running_thread->waited_io = io;

	so_exec();

	return 0;
}

/*
 * signals an IO device
 * + device index
 * return the number of tasks woke or -1 on error
 */
DECL_PREFIX int so_signal(unsigned int io)
{
	/* check if io is valid */
	if (io >= scheduler->io)
		return -1;

	int nr_tasks_woke = 0;

	/* 
	 * remove the threads that were waiting for the io from the
	 * waiting_threads queue and add them to the ready_threads queue
	 */
	node_t *it = scheduler->waiting_threads->head;
	while (it != NULL) {
		if (it->thread->state == WAITING && it->thread->waited_io == io) {

			node_t *temp = it->next;
			thread_t *woken_thread =
				pq_remove_node(scheduler->waiting_threads, it);

			woken_thread->state = READY;
			woken_thread->used_time = 0;
			woken_thread->waited_io = SO_MAX_NUM_EVENTS;

			pq_push(scheduler->ready_threads, woken_thread);
			nr_tasks_woke++;

			it = temp;
		} else {
			it = it->next;
		}
	}

	so_exec();

	return nr_tasks_woke;
}

/*
 * does whatever operation
 */
DECL_PREFIX void so_exec(void)
{
	/* update time_used for running thead */
	scheduler->running_thread->used_time++;

	thread_t *last_thread = scheduler->running_thread;

	update_scheduler(scheduler);

	/* the thread waits if it was preempted */
	int rc = sem_wait(&last_thread->th_running);
	DIE(rc != 0, "sem_wait() failed.");
}

/*
 * destroys a scheduler
 */
DECL_PREFIX void so_end(void)
{
	if (scheduler) {

		int rc;

		/* join all threads */
		for (int i = 0; i < scheduler->threads_nr; i++) {
			rc = pthread_join(scheduler->thread_ids[i], NULL);
			DIE(rc != 0, "pthread_join() failed.");
		}

		/* destroy the scheduler */
		destroy_scheduler(&scheduler);
	}
}
