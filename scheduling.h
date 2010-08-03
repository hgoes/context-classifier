#ifndef __SCHEDULING_H__
#define __SCHEDULING_H__

#include <stdlib.h>
#include <limits.h>

typedef struct {
  char* last_context;
  int repeat_count;
} scheduler_t;

void scheduler_init(scheduler_t* sched);
int scheduler_get_rate(scheduler_t* sched);
void scheduler_add_context(scheduler_t* sched,char* ctx);

#endif
