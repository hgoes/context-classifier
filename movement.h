#ifndef __MOVEMENT_H__
#define __MOVEMENT_H__

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

int fetch_entry(int fd, uint16_t* x,uint16_t* y,uint16_t* z);

#endif
