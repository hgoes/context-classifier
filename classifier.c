#include "classifier.h"

int main(int argc,char** argv) {
  /*
  classifier_set_t cls;
  parse_classifier_set("classifiers/movement.json",&cls);
  int class;
  double res;
  double vec[] = { 75.9375, 0.0, 75.9375, 35.4375, 237.9375, 602.4375, -6.75, 108.0, 834.75, -51.75, 60.75, 951.75, 0.36993867985709139 };
  printf("Evaluating...\n");
  evaluate_classifier(cls.rules,vec,&class,&res);
  printf("Class %d (%f)\n",class,res);
  */
  pid_t audio_id;
  pid_t movement_id;
  int running = 1;
#ifdef CPU_RUNTIME
  struct tms time_start,time_end;
  struct timespec real_time_start,real_time_end;
  times(&time_start);
  clock_gettime(CLOCK_MONOTONIC,&real_time_start);
#endif
  classifier_set_t cls_acc,cls_aud;
  void exit_handler(int signal) {
    running = 0;
  }

  signal(SIGINT,exit_handler);
  plugin_t* accel_plugin = get_acceleration_replayer_plugin("RandData500S");
  if(parse_classifier_set("classifiers/movement.json",&cls_acc,accel_plugin) != 0) {
    fprintf(stderr,"Failed to parse movement classifiers\n");
    return -1;
  }
  //print_classifier_set(&cls_acc);
  /*if(parse_classifier_set("classifiers/audio.json",&cls_aud) != 0) {
    fprintf(stderr,"Failed to parse audio classifiers\n");
    return -1;
    }*/
  int sock = create_broadcast_socket();
  void callback(char* class,double raw,char* ground_truth,void* user_data) {
    struct timeval tv;
    int battery_level = read_battery_level();
    gettimeofday(&tv,NULL);
    printf("%ld\t%d\t%s\t%f\t%d",tv.tv_sec,tv.tv_usec,class,raw,battery_level);
    if(ground_truth == NULL) {
      printf("\n");
    } else {
      printf("\t%s\n",ground_truth);
    }
    send_broadcast_packet(sock,tv.tv_sec,tv.tv_usec,class,(int)(raw*255.0),9);
  }

  //audio_id = dispatch_plugin(get_audio_plugin(),cls_aud.rules,callback_audio,&aud_tp,&running);
  movement_id = dispatch_plugin(accel_plugin,cls_acc.rules,callback,NULL,&running,cls_acc.semantics);

  //waitpid(audio_id,NULL,0);
  waitpid(movement_id,NULL,0);
#ifdef CPU_RUNTIME
  times(&time_end);
  clock_gettime(CLOCK_MONOTONIC,&real_time_end);
  double rtime = 0;
  rtime += time_end.tms_utime - time_start.tms_utime;
  rtime += time_end.tms_stime - time_start.tms_stime;
  rtime += time_end.tms_cutime - time_start.tms_cutime;
  rtime += time_end.tms_cstime - time_start.tms_cstime;
  rtime /= sysconf(_SC_CLK_TCK);
  printf("%f seconds in CPU time\n",rtime);
  double rtime2 = 0;
  rtime2 += real_time_end.tv_sec - real_time_start.tv_sec;
  rtime2 += ((double)(real_time_end.tv_nsec - real_time_start.tv_nsec)) / 1000000000.0;
  printf("%f seconds in real time\n",rtime2);
  printf("%f%% load\n",100.0*rtime/rtime2);
#endif
  return 0;
}
