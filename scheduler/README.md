**Name: Ionescu Matei-Ștefan**  
**Group: 323CAb**

# OS Homework #2 - Thread Scheduler

## Thread Scheduler Library ##

The thread scheduler library implements the folowing functions:

1. `so_init(time_quantum, io)`

	_This function initializes the thread scheduler._

	When the `so_init` function is called, it will check if the chosen
	_time_quantum_ and _io_ valid. If this is the case, and a scheduler
	has not been created yet it will allocate memory and initialize a new
	scheduler. The function returns 0 on success.

2. `so_fork(func, priority)`

	_This function creates a new thread and runs it according to the
	scheduler._

	When the `so_fork` function is called, it will check if the chosen
	function _func_ and the _priority_ are valid arguments. If this is the
	case, it will allocate memory and initialize a new `thread_t` thread.
	Then it will create a new system thread using `pthread_create`. The
	created thread is added to the _ready_threads_ priority queue and its
	_id_ to an array that keeps track of all the threads created. The
	function returns, on success, the _id_ of the created thread;

3. `so_wait(io)`

	_This function make the running thread wait for IO._

	When the `so_wait` function is called, it will check if the chosen _io_ is
	valid. If this is the case, the running thread state will change to
	waiting and it will be rescheduled.

4. `so_signal(io)`

	_This function signals that the threads that were waiting for this io can
	now run._

	When the `so_signal` function is called, it will check if the chosen _io_
	is valid. If this is the case, it will go through the waiting threads
	queue and update the status of the threads that were waiting for this
	particular _io_.

5. `so_exec()`

	_This function simulates the execution of instructions._

	When the `so_exec` fucntion is called, it will increase the time used by
	the thread and update the running thread if necessary.

6. `so_end()`

	_This function destroys the scheduler._

	When the `so_end` function is called, it will `pthread_join` all the
	threads and free the allocated memory for the scheduler and threads.


## Round Robin ##

The library uses the round robin priority algorithm to select the running
thread. The algorithm is implemented in the `update_scheduler` function from
the _threads_handler.c_ file. The function will check the following
situations:
	
1. **There is no thread running**

	In this case the thread with the highest priority from the _ready_threads_
	queue will become the running thread.

2. **The running thread finished its execution**

	In this case the thread will be added to the _terminated_threads_ queue
	and a new thread from the _ready_threads_ queue will take its place.

3. **The running thread is waiting for IO**

	In this case the thread will be added to the _waiting_threads_ queue and
	a new thread from the _ready_threads_ queue will take its place.

4. **The running thread used the maximum amount of time**

	In this case the thread will be added back to the _ready_threads_ queue
	and the thread with the highest priority from the _ready_threads_ queue
	will become the running thread. If the thread that used its time has the
	same priority as the next thread from the _ready_threads_ queue the next
	thread will be run.

5. **The running thread priority is lower than the next ready thread**

	In this case the thread will be added back to the _ready_threads_ queue
	and the thread with the highest priority from the _ready_threads_ queue
	will take its place.
