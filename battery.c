#include "battery.h"

int read_battery_level() {
  FILE* f = fopen("/sys/class/power_supply/BAT0/state","r");
  if(f==NULL) {
    return -1;
  }
  int level;
  fscanf(f,"%d",&level);
  fclose(f);
  return level;
}
