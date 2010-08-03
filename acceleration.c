#include "acceleration.h"

int open_accel_sensor(accel_sensor* sens,const char* dev) {
  sens->fd = open(dev,O_RDONLY);
  sens->last_x = 0;
  sens->last_y = 0;
  sens->last_z = 0;
  return 0;
}

static int read_all(int fd, char *buf, int count) {
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

static void accel_statistics(const int32_t* movement_buf,double* vec) {
  int i;
  double meanx = 0,meany = 0,meanz = 0;
  for(i=0;i<ACCEL_SAMPLE_COUNT;i++) {
    meanx += (double)movement_buf[i*3];
    meany += (double)movement_buf[i*3+1];
    meanz += (double)movement_buf[i*3+2];
  }
  meanx /= ACCEL_SAMPLE_COUNT;
  meany /= ACCEL_SAMPLE_COUNT;
  meanz /= ACCEL_SAMPLE_COUNT;
  double varx = 0,vary = 0,varz = 0;
  for(i=0;i<ACCEL_SAMPLE_COUNT;i++) {
    double t = ((double)movement_buf[i*3]) - meanx;
    varx += t*t;
    t = ((double)movement_buf[i*3+1]) - meany;
    vary += t*t;
    t = ((double)movement_buf[i*3+2]) - meanz;
    varz += t*t;
  }
  varx /= ACCEL_SAMPLE_COUNT;
  vary /= ACCEL_SAMPLE_COUNT;
  varz /= ACCEL_SAMPLE_COUNT;
  vec[0] = varx;
  vec[1] = vary;
  vec[2] = varz;
  vec[3] = meanx;
  vec[4] = meany;
  vec[5] = meanz;
}

int fetch_accel_sample(accel_sensor* sens,double* vec) {
  int32_t movement_buf[ACCEL_SAMPLE_COUNT*3];
  int i;
  MEASURED("movement fetching",{
      for(i=0;i<ACCEL_SAMPLE_COUNT;i++) {
	int res = fetch_entry(sens,&movement_buf[i*3],&movement_buf[i*3+1],&movement_buf[i*3+2]);
        //printf("Raw: [%d %d %d]\n",movement_buf[i*3],movement_buf[i*3+1],movement_buf[i*3+2]);
	if(res < 0) return res;
      }
    });
  MEASURED("movement preprocessing",{
      accel_statistics(movement_buf,vec);
    });
  return 0;
}

void skip_accel_sample(accel_sensor* sens) {
  int32_t tmp;
  int i;
  for(i=0;i<ACCEL_SAMPLE_COUNT;i++) {
    fetch_entry(sens,&tmp,&tmp,&tmp);
  }
}

int fetch_entry(accel_sensor* sens, int32_t* x,int32_t* y,int32_t* z) {
  struct input_event ev;
  do {
    if(fetch_event(sens->fd,&ev)!=0) {
      return -1;
    }
    if(ev.type == 2 || ev.type == 3) {
      switch(ev.code) {
      case 0:
	sens->last_x = ev.value;
	break;
      case 1:
	sens->last_y = ev.value;
	break;
      case 2:
	sens->last_z = ev.value;
	break;
      }
    }
  } while(ev.type!=0 || ev.code!=0);
  *x = sens->last_x;
  *y = sens->last_y;
  *z = sens->last_z;
  return 0;
}

int fetch_acceleration_sample(accel_sensors* sens,double* vec) {
  int res;
  res = fetch_accel_sample(&(sens->s1),vec);
  if(res < 0) return res;
  res = fetch_accel_sample(&(sens->s2),&vec[6]);
  return res;
}

void skip_acceleration_sample(accel_sensors* sens) {
  skip_accel_sample(&(sens->s1));
  skip_accel_sample(&(sens->s2));
}

static void destroy_acceleration_plugin(accel_sensors* sens) {
  close(sens->s1.fd);
  close(sens->s2.fd);
  free(sens);
}

plugin_t* get_acceleration_plugin() {
  plugin_t* res = malloc(sizeof(plugin_t));
  accel_sensors* fds = malloc(sizeof(accel_sensors));
  open_accel_sensor(&(fds->s1),"/dev/input/event3");
  open_accel_sensor(&(fds->s2),"/dev/input/event4");
  res->name = "accelerator";
  res->user_data = fds;
  res->feature_vector_size = 12;
  res->callback = (feature_getter_t)fetch_acceleration_sample;
  res->destructor = (plugin_destructor_t)destroy_acceleration_plugin;
  res->skipper = (feature_skipper_t)skip_acceleration_sample;
  return res;
}
