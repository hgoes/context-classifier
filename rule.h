#ifndef __RULE_H__
#define __RULE_H__

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

#include "vec.h"
#include "ini_parser.h"

typedef struct {
  int size;
  double* rvec;
  double* covar;
  double* vmean;
} rule_t;

void init_rule(rule_t* rule,int n);
void destroy_rule(rule_t* rule);
void print_rule(rule_t* rule);

double evaluate_rule(const rule_t* rule,const double* vec);
double weigh_rule(const rule_t* rule,const double* vec);

double evaluate_ruleset(int nrules,const rule_t* rules,const double* vec);

int parse_ruleset(const char* fn,int* nrules,rule_t** rules);

#endif
