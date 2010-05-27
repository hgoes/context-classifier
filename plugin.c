#include "plugin.h"

void free_plugin(plugin_t* plugin) {
  plugin->destructor(plugin->user_data);
  free(plugin);
}

pid_t dispatch_plugin(const plugin_t* plugin,rule_list_t* rules,classification_cb_t cb,void* cb_data,int* running) {
  pid_t res = fork();
  if(res == 0) {
    char* class;
    double* vec = calloc(plugin->feature_vector_size+1,sizeof(double));
    char* eval_str;
    double raw;
    asprintf(&eval_str,"%s evaluation",plugin->name);
    vec[plugin->feature_vector_size] = 0.0;
    while(*running) {
      if(plugin->callback(plugin->user_data,vec) == 0) {
        MEASURED(eval_str,
                 { rules = evaluate_classifier(rules,vec,&class,&raw); });
        vec[plugin->feature_vector_size] = raw;
        printf("[");
        /*int i;
        for(i = 0; i<=plugin->feature_vector_size; i++) {
          printf("%f",vec[i]);
          if(i!=plugin->feature_vector_size) {
            printf(",");
          }
        }
        printf("] -> %s (%f)\n",class,raw);*/
        printf("%s (%f)\n",class,raw);
        cb(class,raw,cb_data);
      }
    }
    free(vec);
    free(eval_str);
    exit(0);
  }
  return res;
}
