#include "classifier.h"

void audio_thread(int nrules,rule_t* rules) {
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
      //fprintf(stdout,"%f %f %f %f %f %f\n",vec[0],vec[1],vec[2],vec[3],vec[4],vec[5]);
      //fprintf(stdout,"%f %f %f\n",vec[0],vec[1],vec[2]);
      double res;
      MEASURED("audio evaluation",{ res = evaluate_ruleset(nrules,rules,vec); });
      //fprintf(stdout,"%f\n",evaluate_ruleset(nrules,rules,vec));
      fprintf(stdout,"Audio: %f\n",res);
    }
  }
  destroy_audio_system(&sys);
}

void movement_thread(int nrules,rule_t* rules) {
  int fd1 = open("/dev/input/event2",O_RDONLY);
  int fd2 = open("/dev/input/event3",O_RDONLY);
  double vec[13];
  vec[12] = 0;
  while(1) {
    int res;
    res = fetch_movement_sample(fd1,vec);
    res = res != 0 ? res : fetch_movement_sample(fd2,&vec[6]);
    if(res != 0) {
      fprintf(stderr,"Error reading movement data: %d\n",res);
    } else {
      double eval_res;
      MEASURED("movement evaluation",{
	  eval_res = evaluate_ruleset(nrules,rules,vec);
	});
      //fprintf(stdout,"%f\n",eval_res);
      //fprintf(stdout,"%f %f %f %f %f %f\n",vec[0],vec[1],vec[2],vec[3],vec[4],vec[5]);
    }
  }
}

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
}
