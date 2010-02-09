#include "classifier_set.h"

void print_classifier_set(classifier_set_t* cls) {
  printf("type: %s\n",cls->type);
  rule_list_t* cur = cls->rules;
  while(cur != NULL) {
    printf("{\n");
    int i;
    for(i=0;i<cur->nrules;i++) {
      print_rule(&cur->rules[i]);
    }
    printf("[");
    for(i=0;i<cur->nmembers;i++) {
      printf("%f -> %d  ",cur->avgs[i],cur->ress[i]);
    }
    printf("]\n");
    cur = cur->next;
    printf("}\n");
  }
}

static int load_array(json_t* obj,const char* field,int len,double* trg) {
  json_t* arr = json_object_get(obj,field);
  if(!arr || !json_is_array(arr) || json_array_size(arr)!=len) {
    fprintf(stderr,"\"%s\" field non existent or not an array of size %d.\n",field,len);
    return -1;
  }
  int c;
  for(c=0;c<len;c++) {
    json_t* entr = json_array_get(arr,c);
    if(!json_is_real(entr)) {
      fprintf(stderr,"all entries of \"%s\" field must be reals.\n",field);
      return -1;
    }
    trg[c] = json_real_value(entr);
  }
  return 0;
}

static int parse_rule(json_t* obj,rule_t* rule,int dims) {
  init_rule(rule,dims);
  if(!json_is_object(obj)) {
    fprintf(stderr,"all rules must be JSON objects.\n");
    goto dealloc_rule;
  }

  if(load_array(obj,"sigma",dims*dims,rule->covar) < 0) {
    goto dealloc_rule;
  }
  if(load_array(obj,"mean",dims,rule->vmean) < 0) {
    goto dealloc_rule;
  } 
  if(load_array(obj,"consequence",dims+1,rule->rvec) < 0) {
    goto dealloc_rule;
  }
  return 0;
 dealloc_rule:
  destroy_rule(rule);
  return -1;
}

int parse_classifier_set(const char* fp,classifier_set_t* cls) {
  json_t *json;
  json_error_t error;

  json = json_load_file(fp, &error);
  if(!json) {
    fprintf(stderr,"JSON parsing error in line %d: %s\n",error.line,error.text);
    return -1;
  }
  
  if(!json_is_object(json)) {
    goto out_dec;
  }
  
  json_t* type = json_object_get(json,"type");
  if(!type || !json_is_string(type)) {
    fprintf(stderr,"\"type\" field not existent or not a string.\n");
    goto out_dec;
  }
  cls->type = malloc(strlen(json_string_value(type))+1);
  strcpy(cls->type,json_string_value(type));
  
  json_t* dimension = json_object_get(json,"dimension");
  if(!dimension || !json_is_integer(dimension)) {
    fprintf(stderr,"\"dimension\" field not existent or not an integer.\n");
    goto out_free_type;
  }
  int dims = json_integer_value(dimension);

  json_t* classifier = json_object_get(json,"classifier");
  if(!classifier || !json_is_array(classifier)) {
    fprintf(stderr,"\"classifier\" field not existent or not an array.\n");
    goto out_free_type;
  }

  cls->rules = NULL;
  rule_list_t* last_list = NULL;

  unsigned int i;
  for(i=0;i<json_array_size(classifier);i++) {
    json_t* cur_cls = json_array_get(classifier,i);
    if(!json_is_object(cur_cls)) {
      fprintf(stderr,"all classifiers must be JSON objects.\n");
      goto out_free_rules;
    }
    if(last_list == NULL) {
      last_list = cls->rules = malloc(sizeof(rule_list_t));
    } else {
      last_list->next = malloc(sizeof(rule_list_t));
      last_list = last_list->next;
    }
    last_list->next = NULL;
    last_list->rules = NULL;
    last_list->avgs = NULL;
    last_list->ress = NULL;
    json_t* rules = json_object_get(cur_cls,"rules");
    if(!rules || !json_is_array(rules)) {
      fprintf(stderr,"\"rules\" field not existent or not an array.\n");
      goto out_free_rules;
    }
    last_list->nrules = json_array_size(rules);
    last_list->rules = calloc(sizeof(rule_t),json_array_size(rules));
    unsigned int j;
    for(j=0;j<json_array_size(rules);j++) {
      if(parse_rule(json_array_get(rules,j),&last_list->rules[j],dims) < 0) {
        // Destroy rules allocated so far
        unsigned int k;
        for(k=j;k!=0;k--) {
          destroy_rule(&last_list->rules[k-1]);
        }
        last_list->rules = NULL;
        goto out_free_rules;
      }
    }
    json_t* mapping = json_object_get(cur_cls,"mapping");
    if(!mapping || !json_is_array(mapping)) {
      fprintf(stderr,"\"mapping\" field not existent or not an array.\n");
      goto out_free_rules;
    }
    last_list->nmembers = json_array_size(mapping);
    last_list->avgs = calloc(sizeof(double),json_array_size(mapping));
    last_list->ress = calloc(sizeof(int),json_array_size(mapping));
    for(j=0;j<json_array_size(mapping);j++) {
      json_t* entr = json_array_get(mapping,j);
      if(!json_is_object(entr)) {
        fprintf(stderr,"all elements of \"mapping\" field must be objects.\n");
        goto out_free_rules;
      }
      json_t* value = json_object_get(entr,"value");
      if(!value || !json_is_real(value)) {
        fprintf(stderr,"\"value\" field not existent or not a real.\n");
        goto out_free_rules;
      }
      last_list->avgs[j] = json_real_value(value);
      json_t* class = json_object_get(entr,"class");
      if(!class || !json_is_integer(class)) {
        fprintf(stderr,"\"class\" field not existent or not an integer.\n");
        goto out_free_rules;
      }
      last_list->ress[j] = json_integer_value(class);
    }
  }

  json_decref(json);
  return 0;

 out_free_rules:
  while(cls->rules!=NULL) {
    if(cls->rules->rules != NULL) {
      int c;
      for(c=0;c<cls->rules->nrules;c++) {
        destroy_rule(&(cls->rules->rules[c]));
      }
    }
    if(cls->rules->avgs != NULL) {
      free(cls->rules->avgs);
    }
    if(cls->rules->ress != NULL) {
      free(cls->rules->ress);
    }
    rule_list_t* next = cls->rules->next;
    free(cls->rules);
    cls->rules = next;
  }
 out_free_type:
  free(cls->type);
 out_dec:
  json_decref(json);
  return -1;
}
