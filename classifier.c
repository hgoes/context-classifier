#include "classifier.h"

void usage() {
  printf("Usage: classifier [-hcb] [--help] [--cpu-runtime] [-udp-broadcast]\n");
  printf("\n");
  printf(" -h,--help\t\tShow this help\n");
  printf(" -c,--cpu-runtime\tPrint runtime statistics upon exit\n");
  printf(" -b,--udp-broadcast\tBroadcast classification results via UDP\n");
}

plugin_t* load_plugin(char* options) {
  if(strncmp("acceleration",options,12) == 0) {
    return get_acceleration_plugin();
  }
  if(strncmp("replay",options,6) == 0) {
    switch(options[6]) {
    case '\0':
      fprintf(stderr,"replay plugin requires two parameters (none given)\n");
      return NULL;
    case ':':
      if(strncmp("acceleration",&options[7],12) == 0) {
        switch(options[19]) {
        case '\0':
          fprintf(stderr,"replay plugin requires two parameters (one given)\n");
          return NULL;
        case ':':
          return get_acceleration_replayer_plugin(&options[20]);
        }
      }
      break;
    }
  }
  fprintf(stderr,"Failed to load plugin for string %s\n",options);
  return NULL;
}

int parse_options(int argc,char** argv,classifier_options* opts) {
  static struct option long_options[] = {
    { "cpu-runtime",0,NULL,0 },
    { "udp-broadcast",0,NULL,0 },
    { "help",0,NULL,0 },
    { NULL,0,NULL,0 }
  };
  int option_index;
  int c;

  //Set defaults
  opts->cpu_runtime = 0;
  opts->udp_broadcast = 0;
  opts->show_help = 0;

  while( (c = getopt_long(argc,argv,"chb",long_options,&option_index)) != -1) {
    switch(c) {
    case 0:
      switch(option_index) {
      case 0:
        opts->cpu_runtime = 1;
        break;
      case 1:
        opts->udp_broadcast = 1;
        break;
      case 2:
        opts->show_help = 1;
        break;
      }
      break;
    case 'c':
      opts->cpu_runtime = 1;
      break;
    case 'h':
      opts->show_help = 1;
      break;
    case 'b':
      opts->udp_broadcast = 1;
      break;
    default:
      return -1;
    }
  }
  opts->length = (argc - optind) / 2;
  if(opts->length > 0) {
    opts->plugins = calloc(opts->length,sizeof(char*));
    opts->classifiers = calloc(opts->length,sizeof(char*));
    for(c = 0; c<opts->length; c++) {
      opts->plugins[c] = strdup(argv[optind+c*2]);
      opts->classifiers[c] = strdup(argv[optind+c*2+1]);
    }
  }
  return 0;
}

int main(int argc,char** argv) {
  classifier_options opts;
  if(parse_options(argc,argv,&opts) != 0) {
    usage();
    exit(-1);
  }
  if(opts.show_help) {
    usage();
    exit(0);
  }
  int running = 1;
  struct tms time_start,time_end;
  struct timespec real_time_start,real_time_end;
  int sock;
  if(opts.cpu_runtime) {
    times(&time_start);
    clock_gettime(CLOCK_MONOTONIC,&real_time_start);
  }


  void exit_handler(int signal) {
    running = 0;
  }

  signal(SIGINT,exit_handler);

  if(opts.udp_broadcast) {
    sock = create_broadcast_socket();
  }

  void callback(char* class,double raw,char* ground_truth,void* user_data) {
    struct timeval tv;
    int battery_level = read_battery_level();
    gettimeofday(&tv,NULL);
    printf("%ld\t%d\t%s\t%f\t%d",tv.tv_sec,tv.tv_usec,class,raw,battery_level);
    if(ground_truth == NULL) {
      printf("\n");
    } else {
      printf("\t%s\n",ground_truth);
    }
    if(opts.udp_broadcast) {
      send_broadcast_packet(sock,tv.tv_sec,tv.tv_usec,class,(int)(raw*255.0),9);
    }
  }

  int i;
  for(i=0;i<opts.length;i++) {
    plugin_t* plugin = load_plugin(opts.plugins[i]);
    classifier_set_t cls;
    if(parse_classifier_set(opts.classifiers[i],&cls,plugin) != 0) {
      fprintf(stderr,"Failed to parse classifier %s\n",opts.classifiers[i]);
      exit(-1);
    }
    
  }

  pid_t* pids = calloc(opts.length,sizeof(pid_t));
  for(i=0;i<opts.length;i++) {
    plugin_t* plugin = load_plugin(opts.plugins[i]);
    classifier_set_t cls;
    if(parse_classifier_set(opts.classifiers[i],&cls,plugin) != 0) {
      fprintf(stderr,"Failed to parse classifier %s\n",opts.classifiers[i]);
      exit(-1);
    }
    pids[i] = dispatch_plugin(plugin,cls.rules,callback,NULL,&running,cls.semantics);
  }

  for(i=0;i<opts.length;i++) {
    waitpid(pids[i],NULL,0);
  }

  if(opts.cpu_runtime) {
    times(&time_end);
    clock_gettime(CLOCK_MONOTONIC,&real_time_end);
    double rtime = 0;
    rtime += time_end.tms_utime - time_start.tms_utime;
    rtime += time_end.tms_stime - time_start.tms_stime;
    rtime += time_end.tms_cutime - time_start.tms_cutime;
    rtime += time_end.tms_cstime - time_start.tms_cstime;
    rtime /= sysconf(_SC_CLK_TCK);
    printf("%f seconds in CPU time\n",rtime);
    double rtime2 = 0;
    rtime2 += real_time_end.tv_sec - real_time_start.tv_sec;
    rtime2 += ((double)(real_time_end.tv_nsec - real_time_start.tv_nsec)) / 1000000000.0;
    printf("%f seconds in real time\n",rtime2);
    printf("%f%% load\n",100.0*rtime/rtime2);
  }
  return 0;
}
