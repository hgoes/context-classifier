#ifndef __RULE_H__
#define __RULE_H__

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

#include "vec.h"
#include "ini_parser.h"
#include "config.h"

typedef struct {
  int size;
  double* rvec;
  double* covar;
  double* vmean;
} rule_t;

struct rule_list_t_ {
  int nrules;
  rule_t* rules;
  int nmembers;
  double* avgs;
  int* ress;
  struct rule_list_t_* next;
};

typedef struct rule_list_t_ rule_list_t;

void init_rule(rule_t* rule,int n);
void destroy_rule(rule_t* rule);
void print_rule(rule_t* rule);

double evaluate_rule(const rule_t* rule,const double* vec);
double weigh_rule(const rule_t* rule,const double* vec);

double evaluate_ruleset(int nrules,const rule_t* rules,const double* vec);

int parse_ruleset(const char* fn,int* nrules,rule_t** rules);

rule_list_t* evaluate_classifier(rule_list_t* rl,const double* vec,int* res_clas,double* res_val);

rule_list_t* rule_list_add(rule_list_t* rl,int nrules,rule_t* rules,int nmembers,double* avgs,int* ress);
#endif
