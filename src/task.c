#include "libminiomp.h"

miniomp_taskqueue_t * miniomp_taskqueue;

// Initializes the task queue
miniomp_taskqueue_t *init_task_queue(int max_elements) {
		miniomp_taskqueue = malloc(sizeof(miniomp_taskqueue_t));	
		miniomp_taskqueue_t* queue = miniomp_taskqueue;
		queue->max_elements = max_elements;
		queue->count = 0;
	  queue->lock_queue = 0;

    queue->queue = (miniomp_task_t**) malloc(max_elements * sizeof(miniomp_task_t*));

		queue->head = queue->tail = 0;

	  return queue;
}

// Checks if the task descriptor is valid
bool is_valid(miniomp_task_t *task_descriptor) {
		return task_descriptor->fn != NULL;
}

// Checks if the task queue is empty
bool is_empty(miniomp_taskqueue_t *task_queue) {
	  return task_queue->count == 0;
}

// Checks if the task queue is full
bool is_full(miniomp_taskqueue_t *task_queue) {
    return task_queue->max_elements == task_queue->count;
}

// Enqueues the task descriptor at the tail of the task queue
bool enqueue(miniomp_taskqueue_t *task_queue, miniomp_task_t *task_descriptor) {
			
    while (!(__sync_bool_compare_and_swap(&(task_queue->lock_queue), 0, 1))) {}

		if (!is_full(task_queue)) {

			task_queue->queue[task_queue->tail] = task_descriptor;
			task_queue->tail = ((task_queue->tail + 1) % task_queue->max_elements);
			task_queue->count++;

			task_queue->lock_queue = 0;

			return true;
	  }
	  task_queue->lock_queue = 0;

    return false;
}


// Dequeue the task descriptor at the head of the task queue
miniomp_task_t *dequeue(miniomp_taskqueue_t *task_queue) { 

		miniomp_task_t* ret = NULL;
			
		while (!(__sync_bool_compare_and_swap(&(task_queue->lock_queue), 0, 1)));

		if (!is_empty(task_queue)) {	
			
			ret = task_queue->queue[task_queue->head];
			task_queue->head = ((task_queue->head + 1) % task_queue->max_elements);
			task_queue->count--;


		}
		task_queue->lock_queue = 0;

    return ret;
}

/*
// Returns the task descriptor at the head of the task queue
miniomp_task_t *first(miniomp_taskqueue_t *task_queue) {
    return NULL;
}
*/

#define GOMP_TASK_FLAG_UNTIED           (1 << 0)
#define GOMP_TASK_FLAG_FINAL            (1 << 1)
#define GOMP_TASK_FLAG_MERGEABLE        (1 << 2)
#define GOMP_TASK_FLAG_DEPEND           (1 << 3)
#define GOMP_TASK_FLAG_PRIORITY         (1 << 4)
#define GOMP_TASK_FLAG_UP               (1 << 8)
#define GOMP_TASK_FLAG_GRAINSIZE        (1 << 9)
#define GOMP_TASK_FLAG_IF               (1 << 10)
#define GOMP_TASK_FLAG_NOGROUP          (1 << 11)

// Called when encountering an explicit task directive. Arguments are:
//      1. void (*fn) (void *): the generated outlined function for the task body
//      2. void *data: the parameters for the outlined function
//      3. void (*cpyfn) (void *, void *): copy function to replace the default memcpy() from 
//                                         function data to each task's data
//      4. long arg_size: specify the size of data
//      5. long arg_align: alignment of the data
//      6. bool if_clause: the value of if_clause. true --> 1, false -->0; default is set to 1 by compiler
//      7. unsigned flags: untied (1) or not (0) 

void
GOMP_task (void (*fn) (void *), void *data, void (*cpyfn) (void *, void *),
           long arg_size, long arg_align, bool if_clause, unsigned flags,
           void **depend, int priority)
{

  miniomp_task_t* to_enqueue = malloc(sizeof(miniomp_task_t));
	to_enqueue->fn = fn;

    if (__builtin_expect (cpyfn != NULL, 0))
        {
				  char *buf =  malloc(sizeof(char) * (arg_size + arg_align - 1));
          char *arg = (char *) (((uintptr_t) buf + arg_align - 1)
                                & ~(uintptr_t) (arg_align - 1));
          cpyfn (arg, data);
					to_enqueue->data = arg;
        }
    else
				{
					char * buf =  malloc(sizeof(char) * (arg_size + arg_align - 1));
					memcpy (buf, data, arg_size);
					to_enqueue->data = buf;
				}

  if (flags & GOMP_TASK_FLAG_NOGROUP) {
		 __sync_fetch_and_add(&taskgroups[0].counter, 1);
		 to_enqueue->taskgroup=0;
	}
  else {
		 __sync_fetch_and_add(&taskgroups[1].counter, 1);
		 to_enqueue->taskgroup=1;
  }

	while (!enqueue(miniomp_taskqueue, to_enqueue));

}
