/**
 * @file
 * @addtogroup sensors Sensor-Implementations
 * Implements the acceleration sensor of the OpenMoko Freerunner phone.
 * @{
 */
#ifndef __ACCELERATION_H__
#define __ACCELERATION_H__

#include "config.h"
#include "performance.h"
#include "plugin.h"

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

typedef struct {
  accel_sensor s1,s2;
} accel_sensors;

/**
 * Open an acceleration sensor for reading.
 * The sensor is given by its file in the linux input-subsystem.
 */
int open_accel_sensor(accel_sensor* sens,const char* dev);
/**
 * Extract one measurement from a acceleration sensor.
 * Waits until at least one measurement for every dimension is available.
 */
int fetch_entry(accel_sensor* sens, int32_t* x,int32_t* y,int32_t* z);
/**
 * Fetch multiple measurements and calculate a feature vector.
 */
int fetch_accel_sample(int n,accel_sensor* sens,double* vec,const int* semantics);
/**
 * Returns a plugin for the two acceleration sensors of the OpenMoko Freerunner phone.
 */
plugin_t* get_acceleration_plugin();

#endif
/**
 * @}
 */
