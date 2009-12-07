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
