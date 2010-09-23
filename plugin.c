#include "plugin.h"

void free_plugin(plugin_t* plugin) {
  plugin->destructor(plugin->user_data);
  free(plugin);
}

pid_t dispatch_plugin(const plugin_t* plugin,rule_list_t* rules,classification_cb_t cb,void* cb_data,int* running,const int* semantics,int scheduling) {
  pid_t res = fork();
  if(res == 0) {
    char* class;
    double* vec = calloc(plugin->feature_vector_size+1,sizeof(double));
    char* eval_str;
    double raw;
    char* ground_truth;
    scheduler_t sched;
    int idle_counter = 0;
    if(scheduling) {
      scheduler_init(&sched);
    }
    asprintf(&eval_str,"%s evaluation",plugin->name);
    vec[semantics[0]] = 0.0;
    while(*running) {
      if(scheduling) {
        if(idle_counter >= scheduler_get_rate(&sched) || idle_counter >= 20) {
          idle_counter = 0;
        } else {
          plugin->skipper(plugin->user_data);
          idle_counter++;
          continue;
        }
      }
      if(plugin->callback(plugin->user_data,vec,&ground_truth,semantics) == 0) {
        MEASURED(eval_str,
                 { rules = evaluate_classifier(rules,vec,&class,&raw); });
        vec[semantics[0]] = raw;
        if(scheduling) {
          scheduler_add_context(&sched,class);
        }
        cb(class,raw,ground_truth,cb_data);
      } else {
        fprintf(stderr,"WARNING: plugin %s failed to produce feature vector\n",plugin->name);
      }
    }
    free(vec);
    free(eval_str);
    exit(0);
  }
  return res;
}
