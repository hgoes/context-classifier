#include "classifier.h"

void audio_thread(int* running,rule_list_t* rules) {
  audio_system sys;
  int res = init_audio_system(&sys);
  if(res < 0) {
    fprintf(stderr,"Error initializing audio system: %s\n",audio_error(res));
    return;
  }
  double vec[7];
  vec[6] = 0;
  while(*running) {
    res = fetch_audio_sample(&sys,vec);
    if(res != 0) {
      fprintf(stderr,"Error reading audio data: %s\n",audio_error(res));
    } else {
      fprintf(stdout,"Audio vec: [%f %f %f %f %f %f %f]\n",vec[0],vec[1],vec[2],vec[3],vec[4],vec[5],vec[6]);
      int class;
      MEASURED("audio evaluation",{ rules = evaluate_classifier(rules,vec,&class,&vec[6]); });
      fprintf(stdout,"Audio class: %d\n",class);
    }
  }
  destroy_audio_system(&sys);
}

void movement_thread(int* running,rule_list_t* rules) {
  accel_sensor fd1,fd2;
  open_accel_sensor(&fd1,"/dev/input/event2");
  open_accel_sensor(&fd2,"/dev/input/event3");
  double vec[13];
  vec[12] = 0;
  while(*running) {
    int res;
    res = fetch_movement_sample(&fd1,vec);
    res = res != 0 ? res : fetch_movement_sample(&fd2,&vec[6]);
    if(res != 0) {
      //fprintf(stderr,"Error reading movement data: %d\n",res);
    } else {
      fprintf(stdout,"Movement vec: [%f %f %f %f %f %f %f %f %f %f %f %f %f]\n",
              vec[0],vec[1],vec[2],vec[3],vec[4],vec[5],vec[6],vec[7],vec[8],vec[9],vec[10],vec[11],vec[12]);
      MEASURED("movement evaluation",{
	  rules = evaluate_classifier(rules,vec,&res,&vec[12]);
	});
      fprintf(stdout,"Movement class: %d\n",res);
    }
  }
}

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
  if(parse_classifier_set("classifiers/movement.json",&cls_acc) != 0) {
    fprintf(stderr,"Failed to parse movement classifiers\n");
    return -1;
  }
  if(parse_classifier_set("classifiers/audio.json",&cls_aud) != 0) {
    fprintf(stderr,"Failed to parse audio classifiers\n");
    return -1;
  }
  audio_id = fork();
  if(audio_id == 0) {
    audio_thread(&running,cls_aud.rules);
    return 0;
  } else {
    movement_id = fork();
    if(movement_id == 0) {
      //movement_thread(&running,cls_acc.rules);
      printf("Movement thread finished!\n");
      return 0;
    }
  }
  waitpid(audio_id,NULL,0);
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
