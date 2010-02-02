#ifndef __CLASSIFIER_SET_H__
#define __CLASSIFIER_SET_H__

#include <jansson.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "rule.h"

typedef struct {
  char* type;
  rule_list_t* rules;
} classifier_set_t;

int parse_classifier_set(const char* fp,classifier_set_t* cls);

void print_classifier_set(classifier_set_t* cls);
#endif
