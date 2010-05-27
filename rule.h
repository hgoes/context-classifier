/**
 * @file
 * @addtogroup engine Rule-Engine
 * Implements the rule engine.
 * A rule \f$R\f$ of length \f$n\f$ consists of a result vector (\f$r(R)\f$) of length \f$n+1\f$, a covariance matrix (\f$\Sigma(R)\f$) of size \f$n\cdot n\f$ and a mean vector (\f$m(R)\f$) of length \f$n\f$.
 * @{
 */
#ifndef __RULE_H__
#define __RULE_H__

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

#include "vec.h"
#include "ini_parser.h"
#include "config.h"

/**
 * A single rule.
 */
typedef struct {
  ///Dimension of the rule (\f$n\f$)
  int size;
  ///A result vector of size \f$n+1\f$
  double* rvec;
  ///A covariance matrix of size \f$n\cdot n\f$
  double* covar;
  ///A mean vector of size \f$n\f$
  double* vmean;
} rule_t;

struct rule_list_t_ {
  int nrules;
  rule_t* rules;
  int nmembers;
  double* avgs;
  char** ress;
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
 * Applies a rule \f$R\f$ on a feature vector \f$v\f$.
 * \f[ f(R,v) = r(R)_n + \sum_{i=0}^{n} r(R)_i\cdot v_i \f]
 * @param vec The feature vector
 */
double evaluate_rule(const rule_t* rule,const double* vec);
/**
 * Calculates the weight of a rule.
 * \f[ \mu(R,v) = e^{-\frac12 (v - m(R)) \Sigma^{-1} (v - m(R))^T} \f]
 * @param vec The feature vector
 */
double weigh_rule(const rule_t* rule,const double* vec);

/**
 * Apply a set of rules on a feature vector.
 * \f[ S(R,v) = \frac{\sum_{j=1}^m \mu(R_j,v)\cdot f(R_j,v)}{\sum_{j=1}^m \mu(R_j,v)} \f]
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

rule_list_t* evaluate_classifier(rule_list_t* rl,const double* vec,char** res_clas,double* res_val);

rule_list_t* rule_list_add(rule_list_t* rl,int nrules,rule_t* rules,int nmembers,double* avgs,char** ress);
#endif
/**
 * @}
 */
