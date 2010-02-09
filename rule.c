#include "rule.h"

void init_rule(rule_t* rule,int n) {
  rule->size = n;
  rule->rvec = calloc(sizeof(double),n+1);
  rule->covar = calloc(sizeof(double),n*n);
  rule->vmean = calloc(sizeof(double),n);
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
  printf("]\n");
}

double evaluate_rule(const rule_t* rule,const double* vec) {
  return rule->rvec[rule->size] + mult_vec_vec(rule->size-1,rule->rvec,vec);
}

double weigh_rule(const rule_t* rule,const double* vec) {
  int i,j;
  double res = 0;
  int n = rule->size;
  double* t = calloc(sizeof(double),n);
  vec_subtract(n,vec,rule->vmean,t);
  for(i=0;i<n;i++) {
    double tmp = 0;
    for(j=0;j<n;j++) {
      //printf("%lf ",t[j]);
      //printf("TMP: %d %lf\n",j,t[j]);
      tmp+=rule->covar[i*n+j]*t[j];
    }
    //printf("\n");
    res += t[i]*tmp;
  }
  free(t);
  //printf("TMP: %lf\n",exp(-0.5*res));
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

static int increase_sections(const char* name,void* data) {
  if(strcmp(name,"DEFAULT")!=0) {
    state_t* pt = (state_t*)data;
    if(pt->count == 0) {
      if(pt->nrules == -1 || pt->dims == -1) {
	return -3;
      } else {
	pt->rules = calloc(sizeof(rule_t),pt->nrules);
	int i;
	for(i=0;i<pt->nrules;i++) {
	  init_rule(&(pt->rules[i]),pt->dims);
	}
      }
    }
    pt->count++;
  }
  return 0;
}

static int parse_key_value(const char* name,const char* value,void* data) {
  state_t* pt = (state_t*)data;
  if(pt->count == 0) {
    if(strcmp(name,"dimensions")==0) {
      sscanf(value," %d ",&(pt->dims));
    } else if(strcmp(name,"rules")==0) {
      sscanf(value," %d ",&(pt->nrules));
    } else {
      printf("Unknown key %s\n",name);
    }
  } else {
    if(strcmp(name,"sigma")==0) {
      int res = read_doubles(value,pt->rules[pt->count-1].covar,pt->dims*pt->dims);
      if(res < 0) {
	return -2+res;
      }
    } else if(strcmp(name,"mean")==0) {
      int res = read_doubles(value,pt->rules[pt->count-1].vmean,pt->dims);
      if(res < 0) {
	return -2+res;
      }
    } else if(strcmp(name,"consequence")==0) {
      int res = read_doubles(value,pt->rules[pt->count-1].rvec,pt->dims+1);
      if(res < 0) {
	return -2+res;
      }
    } 
  }
  return 0;
}

int parse_ruleset(const char* fn,int* nrules,rule_t** rules) {
  FILE* stream = fopen(fn,"r");
  int res;
  ini_callbacks_t cb;

  cb.section_opened = &increase_sections;
  cb.pair_read = &parse_key_value;

  state_t state;
  state.rules = NULL;
  state.count = 0;
  state.dims = -1;
  state.nrules = -1;

  res = parse_ini_file(&cb,stream,&state);
  if(res != 0) {
    if(state.rules != NULL) {
      free(state.rules);
    }
  } else {
    *rules = state.rules;
    *nrules = state.nrules;
  }
  return res;
}

rule_list_t* evaluate_classifier_rec(rule_list_t* head,rule_list_t* last,rule_list_t* cur,const double* vec,int* res_clas,double* res_val) {
  if(cur == NULL) {
    *res_clas = -1;
    return head;
  } else {
    double eval = evaluate_ruleset(cur->nrules,cur->rules,vec);
    int i;
    for(i=0;i<cur->nmembers;i++) {
      if(fabs(cur->avgs[i] - eval) < FUZZY_VARIANCE) {
	*res_clas = cur->ress[i];
	*res_val  = eval;
	if(last == NULL) {
	  //cur is the first entry
	  return head;
	} else {
	  last->next = cur->next;
	  cur->next = head;
	  return cur;
	}
      }
    }
    return evaluate_classifier_rec(head,cur,cur->next,vec,res_clas,res_val);
  }
}

rule_list_t* evaluate_classifier(rule_list_t* rl,const double* vec,int* res_clas,double* res_val) {
  return evaluate_classifier_rec(rl,NULL,rl,vec,res_clas,res_val);
}

rule_list_t* rule_list_add(rule_list_t* rl,int nrules,rule_t* rules,int nmembers,double* avgs,int* ress) {
  rule_list_t* res = malloc(sizeof(rule_list_t));
  res->nrules = nrules;
  res->rules = rules;
  res->nmembers = nmembers;
  res->avgs = avgs;
  res->ress = ress;
  res->next = rl;
  return res;
}
