#include "acceleration-replayer.h"

static int read_acceleration_package(char** str,double* vec,struct timeval* tp,char** ground_truth,const int* semantics) {
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
  vec[semantics[1]] = means[0];
  vec[semantics[2]] = means[1];
  vec[semantics[3]] = means[2];
  vec[semantics[4]] = vars[0] / 7.0;
  vec[semantics[5]] = vars[1] / 7.0;
  vec[semantics[6]] = vars[2] / 7.0;
  vec[semantics[7]] = means[3];
  vec[semantics[8]] = means[4];
  vec[semantics[9]] = means[5];
  vec[semantics[10]] = vars[3] / 7.0;
  vec[semantics[11]] = vars[4] / 7.0;
  vec[semantics[12]] = vars[5] / 7.0;

  tp->tv_sec = ts_secs;
  tp->tv_usec = ts_usecs;
  *ground_truth = context;
  return 0;
}

static int acceleration_semantics(const char* term) {
  if(strcmp(term,"mean_x1")==0) return 1;
  if(strcmp(term,"mean_y1")==0) return 2;
  if(strcmp(term,"mean_z1")==0) return 3;
  if(strcmp(term,"var_x1")==0) return 4;
  if(strcmp(term,"var_y1")==0) return 5;
  if(strcmp(term,"var_z1")==0) return 6;
  if(strcmp(term,"mean_x2")==0) return 7;
  if(strcmp(term,"mean_y2")==0) return 8;
  if(strcmp(term,"mean_z2")==0) return 9;
  if(strcmp(term,"var_x2")==0) return 10;
  if(strcmp(term,"var_y2")==0) return 11;
  if(strcmp(term,"var_z2")==0) return 12;
  return -1;
}

plugin_t* get_acceleration_replayer_plugin(const char* fn) {
  return get_replayer_plugin(fn,12,32,read_acceleration_package,acceleration_semantics);
}
