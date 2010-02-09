#include "classifier.h"

void audio_thread(rule_list_t* rules) {
  audio_system sys;
  int res = init_audio_system(&sys);
  if(res < 0) {
    fprintf(stderr,"Error initializing audio system: %s\n",audio_error(res));
    return;
  }
  double vec[7];
  vec[6] = 0;
  while(1) {
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

void movement_thread(rule_list_t* rules) {
  accel_sensor fd1,fd2;
  open_accel_sensor(&fd1,"/dev/input/event2");
  open_accel_sensor(&fd2,"/dev/input/event3");
  double vec[13];
  vec[12] = 0;
  while(1) {
    int res;
    res = fetch_movement_sample(&fd1,vec);
    res = res != 0 ? res : fetch_movement_sample(&fd2,&vec[6]);
    if(res != 0) {
      fprintf(stderr,"Error reading movement data: %d\n",res);
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

/*
int main(int argc,char** argv) {
  int nrules_audio;
  rule_t* rules_audio;

  int nrules_movement;
  rule_t* rules_movement;
  

  printf("%d\n",parse_ruleset(argv[1],&nrules_audio,&rules_audio));
  printf("%d\n",parse_ruleset(argv[2],&nrules_movement,&rules_movement));
  
  pid_t audioid = fork();
  if(audioid == 0) {
    audio_thread(nrules_audio,rules_audio);
  } else {
    movement_thread(nrules_movement,rules_movement);
  }
  return 0;
  }*/

int main(int argc,char** argv) {
  classifier_set_t cls;
  /*parse_classifier_set("classifiers/audio.json",&cls);
  int class;
  double res;
  double vec[7] = { 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 7.0 };
  evaluate_classifier(cls.rules,vec,&class,&res);
  printf("Class %d (%f)\n",class,res);*/
  if(parse_classifier_set("classifiers/movement.json",&cls) == 0) {
    movement_thread(cls.rules);
  }
  return 0;
}
