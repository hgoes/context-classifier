#ifndef __PERFORMANCE_H__
#define __PERFORMANCE_H__

#include "config.h"

#ifdef PERFORMANCE_OUTPUT

#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>

#define MEASURED(name,code) {\
  struct timeval tp;\
  double performance_measurement_start,\
    performance_measurement_end;\
  gettimeofday(&tp,NULL);\
  performance_measurement_start = ((double)tp.tv_sec) + ((double)tp.tv_usec)/1e6;\
  code \
  gettimeofday(&tp,NULL);\
  performance_measurement_end = ((double)tp.tv_sec) + ((double)tp.tv_usec)/1e6;\
  fprintf(stderr,"%s: %fs\n",name,performance_measurement_end-performance_measurement_start);\
}

#else

#define MEASURED(name,code) code

#endif

#endif
