#include "libminiomp.h"
//#include "intrinsic.h"

// Library constructor and desctructor
void init_miniomp(void) __attribute__((constructor));
void fini_miniomp(void) __attribute__((destructor));

// Function to parse OMP_NUM_THREADS environment variable
void parse_env(void);

void
init_miniomp(void) {
  printf ("mini-omp is being initialized\n");
  // Parse OMP_NUM_THREADS environment variable to initialize nthreads_var internal control variable
  parse_env();
  // Initialize Pthread data structures 
  // Initialize Pthread thread-specific data, useful for example to store the OpenMP thread identifier
  // Initialize OpenMP default lock and default barrier
  // Initialize OpenMP workdescriptors for for and single 
  // Initialize OpenMP task queue for task and taskloop
}

void
fini_miniomp(void) {
  // free structures allocated during library initialization
  printf ("mini-omp is finalized\n");
}
