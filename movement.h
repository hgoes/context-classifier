#ifndef __MOVEMENT_H__
#define __MOVEMENT_H__

#include "config.h"
#include "performance.h"

#include <stdio.h>
#include <time.h>
#include <sys/times.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <assert.h>

struct input_event {
  struct timeval time;
  uint16_t type;
  uint16_t code;
  int32_t value;
};

typedef struct {
  int fd;
  int32_t last_x,last_y,last_z;
} accel_sensor;

int open_accel_sensor(accel_sensor* sens,const char* dev);
int fetch_entry(accel_sensor* sens, int32_t* x,int32_t* y,int32_t* z);
int fetch_movement_sample(accel_sensor* sens,double* vec);

#endif
