#include "replayer.h"

static int fetch_replayer_sample(replayer_t* replayer,double* arr,char** ground_truth,const int* semantics) {
  char* str[replayer->line_size];
  int i;
  size_t len;
  for(i=0;i<replayer->line_size;i++) {
    str[i] = NULL;
    if(getline(&str[i],&len,replayer->file_handle) < 0) {
      for(;i>=0;i--) {
        free(str[i]);
      }
      return -1;
    }
  }
  struct timeval tp,now;
  int res = replayer->extractor(str,arr,&tp,ground_truth,semantics);
  for(i=0;i<replayer->line_size;i++) {
    free(str[i]);
  }
  if(res != 0) return res;
  gettimeofday(&now,NULL);
  if(replayer->offset.tv_sec == 0 && replayer->offset.tv_usec == 0) {
    // There is no offset set yet, make this first message the one
    timersub(&now,&tp,&(replayer->offset));
  } else {
    timeradd(&tp,&(replayer->offset),&tp);
    if(timercmp(&now,&tp,<)) {
      timersub(&tp,&now,&tp);
      usleep(tp.tv_sec * 1000000 + tp.tv_usec);
    }
  }
  return 0;
}

static void skip_replayer_sample(const replayer_t* replayer) {
  int res;
  int count = 0;
  while(count < replayer->line_size && (res = fgetc(replayer->file_handle)) != EOF) {
    if((unsigned char)res == '\n') {
      count++;
    }
  }
}

static void destroy_replayer(replayer_t* replayer) {
  fclose(replayer->file_handle);
  free(replayer);
}

plugin_t* get_replayer_plugin(const char* fn,int vec_size,int line_size,extractor_t extractor,semantics_mapper_t mapper) {
  FILE* fh = fopen(fn,"r");
  replayer_t* dat = malloc(sizeof(replayer_t));
  plugin_t* res = malloc(sizeof(plugin_t));

  dat->file_handle = fh;
  dat->extractor = extractor;
  dat->offset.tv_sec = 0;
  dat->offset.tv_usec = 0;
  dat->line_size = line_size;

  res->name = "replayer";
  res->user_data = dat;
  res->feature_vector_size = vec_size;
  res->destructor = (plugin_destructor_t)destroy_replayer;
  res->skipper = (feature_skipper_t)skip_replayer_sample;
  res->callback = (feature_getter_t)fetch_replayer_sample;
  res->semantic_mapper = (semantics_mapper_t)mapper;
  return res;
}
