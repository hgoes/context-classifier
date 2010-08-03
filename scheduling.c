#include "scheduling.h"

void scheduler_init(scheduler_t* sched) {
  sched->last_context = NULL;
  sched->repeat_count = 0;
}

int scheduler_get_rate(scheduler_t* sched) {
  return sched->repeat_count;
}

void scheduler_add_context(scheduler_t* sched,char* ctx) {
  if(sched->last_context == ctx) { // Maybe do a strcmp here? don't really know
    if(sched->repeat_count < INT_MAX) {
      sched->repeat_count++;
    }
  } else {
    sched->last_context = ctx;
    sched->repeat_count = 0;
  }
}
