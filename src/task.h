/* This structure describes a "task" to be run by a thread.  */
typedef struct {
    void (*fn)(void *);
    void (*data);
		int taskgroup;
} miniomp_task_t;

//For the taskgroup parameter:
//	If taskgroup == 0: Global counter (first position of the vector)
//  If taskgroup >  0: Local taskgroup indexing vector of taskgroups with the counters 

typedef struct {
		int counter; 									//Number of tasks in the current section
		pthread_mutex_t counterlock; 	//lock to operate over this struct
		int occupied;										//Is currently being used?
} taskgroup_t;

typedef struct {
    int max_elements;
    int count;
    int head;
    int tail;
    int first;
    int lock_queue;
    miniomp_task_t **queue;
    // complete with additional field if needed
} miniomp_taskqueue_t;

extern miniomp_taskqueue_t * miniomp_taskqueue;
#define MAXELEMENTS_TQ 128
#define MAXTASKGROUP_REGIONS 128

extern taskgroup_t* taskgroups;
extern int taskgroup_pointer;
 
miniomp_taskqueue_t* init_task_queue(int max_elements);

// funtions to implement basic management operations on taskqueue
bool is_empty(miniomp_taskqueue_t *task_queue);
bool is_full(miniomp_taskqueue_t *task_queue) ;
bool is_valid(miniomp_task_t *task_descriptor);
bool enqueue(miniomp_taskqueue_t *task_queue, miniomp_task_t *task_descriptor); 
miniomp_task_t* dequeue(miniomp_taskqueue_t *task_queue);
//miniomp_task_t *first(miniomp_taskqueue_t *task_queue); 
