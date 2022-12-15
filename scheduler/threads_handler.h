/*
 * Copyright Ionescu Matei-Stefan - 323CA - 2022-2023
 */

#ifndef THREADS_HANDLER_H_
#define THREADS_HANDLER_H_

#include "so_scheduler.h"
#include "priority_queue.h"

#include <semaphore.h>

#define MAX_THREADS_NR 1024

typedef struct priority_queue_t priority_queue_t;

typedef enum {
	NEW,
	READY,
	RUNNING,
	WAITING,
	TERMINATED
} thread_state_t;

typedef struct thread_t {

	pthread_t thread_id;

	thread_state_t state;

	unsigned int used_time;
	unsigned int priority;

	unsigned int waited_io;

	sem_t th_running;

	so_handler *function;

} thread_t;


typedef struct scheduler_t {

	priority_queue_t *ready_threads;
	priority_queue_t *waiting_threads;
	priority_queue_t *terminated_threads;

	thread_t *running_thread;

	unsigned int time_quantum;
	unsigned int io;

	pthread_t thread_ids[MAX_THREADS_NR];
	int threads_nr;

} scheduler_t;

typedef struct start_args_t {
	thread_t *thread;
	scheduler_t *scheduler;
	
} start_args_t;

scheduler_t *create_scheduler(unsigned int time_quantum, unsigned int io);

void destroy_scheduler(scheduler_t **scheduler);

thread_t *create_thread(so_handler *function, unsigned int priority);

void *start_thread(void *args);

void update_scheduler(scheduler_t *scheduler);

void update_running_thread(scheduler_t *scheduler);

void preempt_thread(scheduler_t *scheduler);

#endif /* THREADS_HANDLER_H_ */
