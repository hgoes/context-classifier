#include "movement.h"

static int read_all(int fd, char *buf, int count)
{
  int n_read = 0;
  while (n_read != count) {
    int result = read(fd, buf + n_read, count - n_read);
    if (result < 0)
      return result;
    else if (result == 0)
      return n_read;
    n_read += result;
  }
  return n_read;
}

static int fetch_event(int fd,struct input_event* ev) {
  int ret = read_all(fd,(char*)ev,sizeof(struct input_event));
  if(ret != sizeof(struct input_event)) {
    return -1;
  } else {
    return 0;
  }
}

static void movement_statistics(const uint16_t* movement_buf,double* vec) {
  int i;
  double meanx = 0,meany = 0,meanz = 0;
  for(i=0;i<MOVEMENT_SAMPLE_COUNT;i++) {
    meanx += (double)movement_buf[i*3];
    meany += (double)movement_buf[i*3+1];
    meanz += (double)movement_buf[i*3+2];
  }
  meanx /= MOVEMENT_SAMPLE_COUNT;
  meany /= MOVEMENT_SAMPLE_COUNT;
  meanz /= MOVEMENT_SAMPLE_COUNT;
  double varx = 0,vary = 0,varz = 0;
  for(i=0;i<MOVEMENT_SAMPLE_COUNT;i++) {
    double t = ((double)movement_buf[i*3]) - meanx;
    varx += t*t;
    t = ((double)movement_buf[i*3+1]) - meany;
    vary += t*t;
    t = ((double)movement_buf[i*3+2]) - meanz;
    varz += t*t;
  }
  varx /= MOVEMENT_SAMPLE_COUNT;
  vary /= MOVEMENT_SAMPLE_COUNT;
  varz /= MOVEMENT_SAMPLE_COUNT;
  vec[0] = varx;
  vec[1] = vary;
  vec[2] = varz;
  vec[3] = meanx;
  vec[4] = meany;
  vec[5] = meanz;
}

int fetch_movement_sample(int fd,double* vec) {
  uint16_t movement_buf[MOVEMENT_SAMPLE_COUNT*3];
  int i;
  MEASURED("movement fetching",{
      for(i=0;i<MOVEMENT_SAMPLE_COUNT;i++) {
	int res = fetch_entry(fd,&movement_buf[i*3],&movement_buf[i*3+1],&movement_buf[i*3+2]);
	if(res < 0) return res;
      }
    });
  MEASURED("movement preprocessing",{
      movement_statistics(movement_buf,vec);
    });
  return 0;
}

int fetch_entry(int fd, uint16_t* x,uint16_t* y,uint16_t* z) {
  struct input_event ev;
  do {
    if(fetch_event(fd,&ev)!=0) {
      return -1;
    }
    if(ev.type == 2 || ev.type == 3) {
      switch(ev.code) {
      case 0:
	*x = ev.value;
	break;
      case 1:
	*y = ev.value;
	break;
      case 2:
	*z = ev.value;
	break;
      }
    }
  } while(ev.type!=0 || ev.code!=0);
  return 0;
}
