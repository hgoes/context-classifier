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

/**
 * Initalize a rule. Allocates enough memory for the rule.
 * @param n The vector size of the rule
 */
void init_rule(rule_t* rule,int n);
/**
 * Frees the memory allocated for the rule.
 */
void destroy_rule(rule_t* rule);
/**
 * Prints the rule in a human-readable format on stdout.
 */
void print_rule(rule_t* rule);

/**
 * Applies a rule on a feature vector.
 * @param vec The feature vector
 */
double evaluate_rule(const rule_t* rule,const double* vec);
/**
 * Calculates the weight of a rule.
 * @param vec The feature vector
 */
double weigh_rule(const rule_t* rule,const double* vec);

/**
 * Apply a set of rules on a feature vector.
 * @param nrules The number of rules
 * @param rules The array containing the rules
 * @param vec The feature vector
 */
double evaluate_ruleset(int nrules,const rule_t* rules,const double* vec);

/**
 * Parse a ruleset from a file.
 * @param fn The filename to load from
 * @param nrules Where the number of parsed rules will be stored
 * @param rules Where the parsed rules will be stored
 * @return 0 if the parsing was successful, <0 otherwise
 */
int parse_ruleset(const char* fn,int* nrules,rule_t** rules);

rule_list_t* evaluate_classifier(rule_list_t* rl,const double* vec,int* res_clas,double* res_val);

rule_list_t* rule_list_add(rule_list_t* rl,int nrules,rule_t* rules,int nmembers,double* avgs,int* ress);
#endif
