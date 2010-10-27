#include "rule.h"

void init_rule(rule_t* rule,int n) {
  rule->size = n;
  rule->rvec = calloc(sizeof(double),n+1);
  rule->covar = calloc(sizeof(double),n*n);
  rule->vmean = calloc(sizeof(double),n);
  rule->bitvector = 0;
}

void destroy_rule(rule_t* rule) {
  free(rule->rvec);
  free(rule->covar);
  free(rule->vmean);
}

void print_rule(rule_t* rule) {
  int n = rule->size;
  int i;
  printf("Rule [");
  for(i=0;i<n+1;i++) {
    printf("%lf ",rule->rvec[i]);
  }
  printf("] [");
  for(i=0;i<n;i++) {
    printf("%lf ",rule->vmean[i]);
  }
  printf("] [");
  for(i=0;i<n*n;i++) {
    printf("%lf ",rule->covar[i]);
  }
  printf("]");
  if(rule->bitvector == 0) {
    printf("\n");
  } else {
    printf(" (");
    for(i=0;i<n;i++) {
      if((rule->bitvector | (1 << i)) == rule->bitvector) {
        printf("1");
      } else {
        printf("0");
      }
    }
    printf(")\n");
  }
}

double evaluate_rule(const rule_t* rule,const double* vec) {
  return rule->rvec[rule->size] + mult_vec_vec(rule->size,rule->rvec,vec);
}

double weigh_rule(const rule_t* rule,const double* vec) {
  int i,j;
  double res = 0;
  int n = rule->size;
  double* t = calloc(sizeof(double),n);
  vec_subtract(n,vec,rule->vmean,t);
  for(i=0;i<n;i++) {
    if((rule->bitvector | (1 << i)) != rule->bitvector) {
      double tmp = 0;
      for(j=0;j<n;j++) {
        tmp+=rule->covar[i*n+j]*t[j];
      }
      res += t[i]*tmp;
    }
  }
  free(t);
  return exp(-0.5*res);
}

double evaluate_ruleset(int nrules,const rule_t* rules,const double* vec) {
  double r = 0;
  double d = 0;
  int i;
  for(i=0;i<nrules;i++) {
    double w = weigh_rule(&rules[i],vec);
    double e = evaluate_rule(&rules[i],vec);
    //printf("EVAL %d: %lf, %lf\n",i,w,e);
    r += w*e;
    d += w;
  }
  return r/d;
}

typedef struct {
  int count;
  int nrules;
  rule_t* rules;
  int dims;
} state_t;

rule_list_t* evaluate_classifier_rec(rule_list_t* head,rule_list_t* last,rule_list_t* cur,const double* vec,char** res_clas,double* res_val) {
  if(cur == NULL) {
    *res_clas = "ERR";
    *res_val = 0.0;
    return head;
  } else {
    double eval = evaluate_ruleset(cur->nrules,cur->rules,vec);
    int i;
    for(i=0;i<cur->nmembers;i++) {
      if(fabs(cur->avgs[i] - eval) < FUZZY_VARIANCE) {
	*res_clas = cur->ress[i];
	*res_val  = eval;
	if(last != NULL) {
	  //cur is not the first entry
          last->next = cur->next;
	  cur->next = head;
	}
        return cur;
      }
    }
    return evaluate_classifier_rec(head,cur,cur->next,vec,res_clas,res_val);
  }
}

rule_list_t* evaluate_classifier(rule_list_t* rl,const double* vec,char** res_clas,double* res_val) {
  return evaluate_classifier_rec(rl,NULL,rl,vec,res_clas,res_val);
}

rule_list_t* rule_list_add(rule_list_t* rl,int nrules,rule_t* rules,int nmembers,double* avgs,char** ress) {
  rule_list_t* res = malloc(sizeof(rule_list_t));
  res->nrules = nrules;
  res->rules = rules;
  res->nmembers = nmembers;
  res->avgs = avgs;
  res->ress = ress;
  res->next = rl;
  return res;
}
