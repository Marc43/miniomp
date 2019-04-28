#include "libminiomp.h"
#include <unistd.h>

// This file implements the PARALLEL construct

// Declaration of array for storing pthread identifier from pthread_create function
pthread_t *miniomp_threads;

// Global variable for parallel descriptor
miniomp_parallel_t *miniomp_parallel;

// Declaration of per-thread specific key
pthread_key_t miniomp_specifickey;

typedef struct {
	int error_code;
} thread_exit_status;

thread_exit_status* status;

pthread_mutex_t taskcounter_mutex = PTHREAD_MUTEX_INITIALIZER;

int generating;

taskgroup_t* taskgroups;
int taskgroup_pointer;

void *worker(void *args) {

//  volatile int i = 0;
//	while(i==0);

	int id = ((miniomp_parallel_t *) args)->id;

	//global identifier with key
	pthread_setspecific(miniomp_specifickey, &((miniomp_parallel_t*) args)->id); 

	if (id == 0) {
		((miniomp_parallel_t *) args)->fn(((miniomp_parallel_t*) args)->fn_data);
		generating = 0;
		__sync_synchronize();
	}

	while(generating || !is_empty(miniomp_taskqueue)) {
		__sync_synchronize();
	
		miniomp_task_t* task = dequeue(miniomp_taskqueue);

		if (task != NULL) {
			task->fn(task->data);
			__sync_fetch_and_sub(&taskgroups[task->taskgroup].counter, 1);
		}
	}

	pthread_exit(NULL);
  return(NULL);
}

void
GOMP_parallel (void (*fn) (void *), void *data, unsigned num_threads, unsigned int flags) {
  if(!num_threads) num_threads = omp_get_num_threads();
  printf("Starting a parallel region using %d threads\n", num_threads);

	miniomp_parallel = (miniomp_parallel_t*) malloc(num_threads * sizeof(miniomp_parallel_t));
	miniomp_threads  = (pthread_t*)	 malloc(num_threads * sizeof(pthread_t));
	status = (thread_exit_status*) malloc(num_threads * sizeof(thread_exit_status));

  taskgroups = (taskgroup_t*) malloc(MAXTASKGROUP_REGIONS * sizeof(taskgroup_t)); //2^32 - 1 tasks per taskgroup..
	taskgroup_pointer = 1;

	taskgroups[0].counter = 0;
	taskgroups[0].occupied = 1; //Never should be set to 0..

	for (int i=1; i < MAXTASKGROUP_REGIONS; i++) {
		taskgroups[i].counter = 0;
		taskgroups[i].occupied = 0;
	}

	generating = 1; //Starting to generate tasks..

  pthread_key_create(&miniomp_specifickey, NULL);
		
  miniomp_taskqueue = init_task_queue(MAXELEMENTS_TQ);

	/* 
		Thread creation, we assign de first one (0) as the master.
	*/
	for (int i=0; i<num_threads; i++) {

		miniomp_parallel[i].id = i;
		miniomp_parallel[i].fn 				= (void*)fn;
		miniomp_parallel[i].fn_data 	= data;

		pthread_create(&miniomp_threads[i], NULL, &worker, (void *) &miniomp_parallel[i]);
	}

	for (int i=0; i<num_threads; i++) {
		pthread_join(miniomp_threads[i], (void**) &status[i]);
	}


	}
