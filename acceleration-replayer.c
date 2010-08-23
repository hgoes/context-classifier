#include "acceleration-replayer.h"

static int read_acceleration_package(char** str,double* vec,struct timeval* tp,char** ground_truth) {
  double data[8][6];
  long int ts_secs,ts_usecs = 0;
  char ts_fracs[10];
  static char context[20]; //Sorry, sorry, sorry, It's just so easy...
  int i,j;
  int fac;
  for(i=0;i<8;i++) {
    for(j=0;j<2;j++) {
      int res = sscanf(str[i*2+j],"%ld.%9[0123456789] %s %*s %lf %lf %lf",&ts_secs,ts_fracs,context,
                       &data[i][j*3],&data[i][j*3+1],&data[i][j*3+2]);
      if(res != 6) {
        context[0] = '\0';
        sscanf(str[i*2+j],"%ld.%9[0123456789] %*s %lf %lf %lf",&ts_secs,ts_fracs,
               &data[i][j],&data[i][j*3+1],&data[i][j*3+2]);
      }
    }
  }
  fac = 100000;
  for(i=0;i<6;i++) {
    if(ts_fracs[i] == '\0') break;
    ts_usecs += ((int)(ts_fracs[i] - '0'))*fac;
    fac /= 10;
  }
  double means[6];
  for(j=0;j<6;j++) {
    means[j]=0.0;
  }
  for(i=0;i<8;i++) {
    for(j=0;j<6;j++) {
      means[j]+=data[i][j];
    }
  }
  for(j=0;j<6;j++) {
    means[j] /= 8.0;
  }
  double vars[6];
  for(j=0;j<6;j++) {
    vars[j]=0.0;
  }
  for(i=0;i<8;i++) {
    for(j=0;j<6;j++) {
      double t = data[i][j]-means[j];
      vars[j]+=t*t;
    }
  }
  vec[0] = vars[0] / 8.0;
  vec[1] = vars[1] / 8.0;
  vec[2] = vars[2] / 8.0;
  vec[3] = means[0];
  vec[4] = means[1];
  vec[5] = means[2];
  vec[6] = vars[3] / 8.0;
  vec[7] = vars[4] / 8.0;
  vec[8] = vars[5] / 8.0;
  vec[9] = means[3];
  vec[10] = means[4];
  vec[11] = means[5];

  tp->tv_sec = ts_secs;
  tp->tv_usec = ts_usecs;
  *ground_truth = context;
  return 0;
}

plugin_t* get_acceleration_replayer_plugin(const char* fn) {
  return get_replayer_plugin(fn,12,32,read_acceleration_package);
}
