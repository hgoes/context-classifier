#include "classifier.h"

int main(int argc,char** argv) {
  int nrules;
  rule_t* rules;

  printf("%d\n",parse_ruleset(argv[1],&nrules,&rules));
  
  int i;
  for(i=0;i<nrules;i++) {
    print_rule(&rules[i]);
  }

  audio_system sys;
  int res = init_audio_system(&sys);
  if(res < 0) {
    fprintf(stderr,"%s\n",audio_error(res));
    return -1;
  }
  double vec[7];
  vec[6] = 0;
  while(1) {
    res = fetch_audio_sample(&sys,vec);
    if(res != 0) {
      fprintf(stderr,"Error reading data %s\n",audio_error(res));
    } else {
      //fprintf(stdout,"%f %f %f %f %f %f\n",vec[0],vec[1],vec[2],vec[3],vec[4],vec[5]);
      fprintf(stdout,"%f\n",evaluate_ruleset(nrules,rules,vec));
    }
  }
  destroy_audio_system(&sys);
  return 0;
}
