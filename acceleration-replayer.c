#include "acceleration-replayer.h"

static int read_acceleration_package(char** str,double* vec,struct timeval* tp) {
  double data[8][3];
  long int ts_secs,ts_usecs = 0;
  char ts_fracs[10];
  char context[20];
  int i;
  int fac;
  for(i=0;i<8;i++) {
    int res = sscanf(str[0],"%ld.%9[0123456789] %s %*s %lf %lf %lf",&ts_secs,ts_fracs,context,&data[i][0],&data[i][1],&data[i][2]);
    if(res != 6) {
      context[0] = '\0';
      sscanf(str[0],"%ld.%9[0123456789] %*s %lf %lf %lf",&ts_secs,ts_fracs,&data[i][0],&data[i][1],&data[i][2]);
    }
  }
  fac = 100000;
  for(i=0;i<6;i++) {
    if(ts_fracs[i] == '\0') break;
    ts_usecs += ((int)(ts_fracs[i] - '0'))*fac;
    fac /= 10;
  }
  double sx=0.0,sy=0.0,sz=0.0;
  for(i=0;i<8;i++) {
    sx+=data[i][0];
    sy+=data[i][1];
    sz+=data[i][2];
  }
  sx /= 8.0;
  sy /= 8.0;
  sz /= 8.0;

  double vx=0.0,vy=0.0,vz=0.0;
  for(i=0;i<8;i++) {
    double t;
    t = data[i][0]-sx;
    vx += t*t;
    t = data[i][1]-sy;
    vy += t*t;
    t = data[i][2]-sz;
    vz += t*t;
  }
  vec[0] = vx / 8.0;
  vec[1] = vy / 8.0;
  vec[2] = vz / 8.0;
  vec[3] = sx;
  vec[4] = sy;
  vec[5] = sz;

  tp->tv_sec = ts_secs;
  tp->tv_usec = ts_usecs;
  return 0;
}

plugin_t* get_acceleration_replayer_plugin(const char* fn) {
  return get_replayer_plugin(fn,6,16,read_acceleration_package);
}
