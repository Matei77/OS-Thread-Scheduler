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

	if (time_quantum == 0 || io > SO_MAX_NUM_EVENTS) {
		return -1;
	}

	if (scheduler)
		return -1;

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
	if (priority > SO_MAX_PRIO || !func)
		return INVALID_TID;

	int rc;

	thread_t *thread = create_thread(func, priority);

	start_args_t *args = malloc(sizeof(start_args_t));
	args->thread = thread;
	args->scheduler = scheduler;

	rc = pthread_create(&thread->thread_id, NULL, start_thread, args);
	DIE(rc != 0, "pthred_create() failed.");

	printf("created thread{thread_id = %ld; priority = %d}\n",
		   thread->thread_id, thread->priority);

	pq_push(scheduler->ready_threads, thread);

	if (scheduler->running_thread != NULL) {
		so_exec();
	} else {
		update_scheduler(scheduler);
	}


	scheduler->thread_ids[scheduler->threads_nr++] = thread->thread_id;

	return thread->thread_id;
}

/*
 * waits for an IO device
 * + device index
 * returns: -1 if the device does not exist or 0 on success
 */
DECL_PREFIX int so_wait(unsigned int io) { return 0; }

/*
 * signals an IO device
 * + device index
 * return the number of tasks woke or -1 on error
 */
DECL_PREFIX int so_signal(unsigned int io) { return 0; }

/*
 * does whatever operation
 */
DECL_PREFIX void so_exec(void)
{
	// scheduler->running_thread->used_time++;

	// update_scheduler(scheduler);

	// int rc = sem_wait(&scheduler->running_thread->th_running);
	// DIE(rc != 0, "sem_wait() failed.");
}

/*
 * destroys a scheduler
 */
DECL_PREFIX void so_end(void)
{
	int rc;

	if (scheduler) {

		for (int i = 0; i < scheduler->threads_nr; i++) {
			rc = pthread_join(scheduler->thread_ids[i], NULL);
			DIE(rc != 0, "pthread_join() failed.");
		}

		printf("destroy scheduler.\n");
		destroy_scheduler(&scheduler);
	}
}
