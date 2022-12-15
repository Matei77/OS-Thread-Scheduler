/*
 * Copyright Ionescu Matei-Stefan - 323CA - 2022-2023
 */

#ifndef THREADS_HANDLER_H_
#define THREADS_HANDLER_H_

#include "so_scheduler.h"
#include "priority_queue.h"

#include <semaphore.h>


#define THREADS_INITIAL_SIZE 4

typedef struct priority_queue_t priority_queue_t;

typedef enum {
	NEW,
	READY,
	RUNNING,
	WAITING,
	TERMINATED
} thread_state_t;

typedef struct thread_t {

	pthread_t thread_id; /* the id of the thread */

	thread_state_t state; /* the state of the thread */

	unsigned int used_time; /* the time used by the thread */
	unsigned int priority; /* the priority of the thread */

	unsigned int waited_io; /*the io that the thread is waiting for */

	sem_t th_running; /* semaphore that indicates if the thread is running */

	so_handler *function; /* the function ran by the thread */

} thread_t;


typedef struct scheduler_t {

	priority_queue_t *ready_threads; /* priority queue of ready threads */
	priority_queue_t *waiting_threads; /* priority queue of waithing threads */
	priority_queue_t *terminated_threads; /* priority queue of terminated threads */

	thread_t *running_thread; /* the thread that is currently running */

	unsigned int time_quantum; /* the maximum amount of time that the thread can use before being preempted */
	unsigned int io; /* the maiximum number of io devices supported */

	pthread_t *thread_ids; /* array containing the ids of all the threads created */
	int threads_ids_size; /* the size of the allocated thread_ids array */
	int threads_nr; /* the number of threads created */

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
