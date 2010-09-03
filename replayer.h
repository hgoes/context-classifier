#ifndef __REPLAYER_H__
#define __REPLAYER_H__

#include "plugin.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

typedef int (*extractor_t)(char** str,double* vec,struct timeval* tp,char** ground_truth,const int* semantics);

typedef struct {
  FILE* file_handle;
  extractor_t extractor;
  struct timeval offset;
  int line_size;
} replayer_t;

plugin_t* get_replayer_plugin(const char* fn,int vec_size,int line_size,extractor_t extractor,semantics_mapper_t mapper);

#endif
