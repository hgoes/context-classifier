#ifndef __CLASSIFIER_H__
#define __CLASSIFIER_H__

#include "config.h"

#include "audio.h"
#include "rule.h"
#include "acceleration.h"
#include "acceleration-replayer.h"
#include "performance.h"
#include "broadcaster.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/times.h>
#include "classifier_set.h"
#include "battery.h"
#include <signal.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <getopt.h>

typedef struct {
  int show_help:1;
  int cpu_runtime:1;
} classifier_options;

#endif
