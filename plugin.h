#ifndef __PLUGIN_H__
#define __PLUGIN_H__

#define _GNU_SOURCE

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "rule.h"
#include "performance.h"

typedef int (*feature_getter_t)(void* user_data,double* result);

typedef void (*classification_cb_t)(int class,double raw,void* user_data);

typedef void (*plugin_destructor_t)(void* user_data);

typedef struct {
  char* name;
  void* user_data;
  int feature_vector_size;
  feature_getter_t callback;
  plugin_destructor_t destructor;
} plugin_t;

void free_plugin(plugin_t* plugin);
pid_t dispatch_plugin(const plugin_t* plugin,rule_list_t* rules,classification_cb_t cb,void* cb_data,int* running);

#endif
