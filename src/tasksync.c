#include "libminiomp.h"

// Called when encountering taskwait and taskgroup constructs

void
GOMP_taskwait (void)
{
		__sync_synchronize();
		while(taskgroups[taskgroup_pointer].counter != 0) {
			__sync_synchronize();
		}
}

void
GOMP_taskgroup_start (void)
{
//	while (taskgroups[(taskgroup_pointer++ % MAXTASKGROUP_REGIONS)].occupied);
//	taskgroups[taskgroup_pointer].occupied = 1;
	taskgroup_pointer = 1;
}

void
GOMP_taskgroup_end (void)
{
	__sync_synchronize();
	while(taskgroups[taskgroup_pointer].counter != 0) {
		__sync_synchronize();
	}
	taskgroup_pointer = 0;
}
